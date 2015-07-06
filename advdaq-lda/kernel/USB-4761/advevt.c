#include "USB4761.h"

INT32S adv_set_di_event_mode(adv_device *device, INT32U index)
{
    
	USB_EnableEvent trans_info;
	private_data *privdata = NULL;
	INT32S ret = 0;
     	adv_process_info_header *p = NULL;

	privdata = device->private_data;
	p = &privdata->ptr_process_info;

	trans_info.Enabled = adv_process_info_isenable_event_all(p, index);
	trans_info.EventType = index & 0xff;
	trans_info.EventTrigger = (privdata->us_int_trigger_mode_cfg & (1 << index)) >> index;

	if (privdata->udev == NULL) {
		return -EFAULT;
	}
	
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			       MAJOR_EVENT, 0x40,
			       MINOR_EVENT_ENABLE, 0,
			       (BYTE *)&trans_info, sizeof(trans_info));
	
	if(ret > 0){
		ret = 0;
	}
	
	return ret;
}

INT32S adv_set_di_event_trigger_mode(adv_device *device, INT32U port)
{
     INT32U i;
     
     for(i = ADS_EVT_DI_INTERRUPT0_Index; i <= ADS_EVT_DI_INTERRUPT7_Index; i++){
	     adv_set_di_event_mode(device, i);
     }
    
     return 0;
}

/**
 * check_urb_event - translating the buffer's mean
 * @data - user data with urb
 */
void check_urb_event(PTR_T data)
{
	adv_device *device = (adv_device *)data;
	private_data *privdata = device->private_data;

	INT32U nEventNumber;
	INT32U i;
	PEVENT_DATA pEd;
	atomic_set(&privdata->tasklet_processed, 0);

	nEventNumber = (INT32U)(*(BYTE *)privdata->urb_int_transfer_copy);

	if(nEventNumber){
		for(i = 0; i < nEventNumber; i++){
			pEd = (PEVENT_DATA)(privdata->urb_int_transfer_copy + sizeof(BYTE ) + i*sizeof(EVENT_DATA));
	
			if(pEd->DIEvent > ADS_EVT_DI_INTERRUPT7_Index){
				continue;
			}

			adv_process_info_set_event_all(&privdata->ptr_process_info, pEd->DIEvent, 1);
			privdata->ports_status_event[pEd->DIEvent].status[0] = pEd->DIdata;
		}
	}

	atomic_set(&privdata->tasklet_processed, 1);
	wake_up_interruptible(&privdata->event_wait);


	return;
}



static void urb_int_callback(struct urb *urb, struct pt_regs *regs)
{
	adv_device *device = (adv_device *)urb->context;
	private_data *privdata = device->private_data;
         
	/* if error occur */
	if(urb->status &&
	   !(urb->status == -ENOENT ||
	     urb->status == -ECONNRESET ||
	     urb->status == -ESHUTDOWN))
	{
		return;
	}
	

	//if( (*(BYTE *)privdata->urb_int_transfer) && atomic_dec_and_test(&privdata->tasklet_processed))
	if ( (*(BYTE *)privdata->urb_int_transfer) && (atomic_read(&privdata->tasklet_processed) == 1) ){
		memcpy(privdata->urb_int_transfer_copy, privdata->urb_int_transfer, sizeof(BYTE) + (*(BYTE *)privdata->urb_int_transfer)*sizeof(EVENT_DATA));
		
		tasklet_schedule(&privdata->check_event_tasklet);
	}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,8))
	adv_usb_submit_urb(urb, GFP_KERNEL);
#endif

	return ;
}

/**
 * adv_enable_event - enable event
 * @device -
 * @EventType -
 * @Enabled -
 * @Count
 */
INT32S adv_enable_event(adv_device *device, INT16U event_type, INT16U enable, INT32U count)
{
	private_data *privdata = (private_data *)device->private_data;
	adv_process_info_header *p= &privdata->ptr_process_info;
	INT32S ret = 0;
	INT32U event_enabled = 0;
	INT32U event_index;
	INT32U i;
	
	/* testing any event has enabled before */
	for(i = ADS_EVT_DI_INTERRUPT0_Index; i <= ADS_EVT_DI_INTERRUPT7_Index; i++){
		if(adv_process_info_isenable_event_all(p, i)){
			event_enabled = 1;
			break;
		}
	}

	event_index = event_type - ADS_EVT_DI_INTERRUPT0;
	if(enable){
		switch(event_type)
		{
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_enable_event(p, 0, count);
			break;
		case ADS_EVT_DI_INTERRUPT1:
			adv_process_info_enable_event(p, 1, count);
			break;
		case ADS_EVT_DI_INTERRUPT2:
			adv_process_info_enable_event(p, 2, count);
			break;
		case ADS_EVT_DI_INTERRUPT3:
			adv_process_info_enable_event(p, 3, count);
			break;
		case ADS_EVT_DI_INTERRUPT4:
			adv_process_info_enable_event(p, 4, count);
			break;
		case ADS_EVT_DI_INTERRUPT5:
			adv_process_info_enable_event(p, 5, count);
			break;
		case ADS_EVT_DI_INTERRUPT6:
			adv_process_info_enable_event(p, 6, count);
			break;
		case ADS_EVT_DI_INTERRUPT7:
			adv_process_info_enable_event(p, 7, count);
			break;
		default:
			return -EFAULT;
		}
	
		/* Initillize the urb when first start */
		if(!event_enabled){
			atomic_set(&privdata->tasklet_processed, 1);
			usb_fill_int_urb(privdata->urb_int, privdata->udev,
					 usb_rcvintpipe(privdata->udev, privdata->int_endpointAddr),
					 (void *)privdata->urb_int_transfer, (privdata->urb_int_in_size - sizeof(BYTE))/sizeof(EVENT_DATA)*sizeof(EVENT_DATA) + sizeof(BYTE),
					 (usb_complete_t)urb_int_callback, device, 1);
			
			//tasklet_enable(&privdata->check_event_tasklet);
				
			adv_usb_submit_urb(privdata->urb_int, GFP_KERNEL);
		}

		/* start device di interrupt event  */
		ret = adv_set_di_event_mode(device, event_index);

		if(ret){
			return ret;
		}
		return 0;
	}else{
		adv_process_info_disable_event(p, event_index);
		/* disable EventType di interrupt */
		if (!adv_process_info_isenable_event_all(p, event_index)) {
			ret = adv_set_di_event_mode(device, event_index);
			if(ret){
				return ret;
			}
		}
		
		event_enabled = 0;
		/* testing any event has enabled before */
		for(i = ADS_EVT_DI_INTERRUPT0_Index; i <= ADS_EVT_DI_INTERRUPT7_Index; i++){
			if(adv_process_info_isenable_event_all(p, i)){
				event_enabled = 1;
				break;
			}
		}

		if(!event_enabled){
			usb_kill_urb(privdata->urb_int);
			//	tasklet_disable(&privdata->check_event_tasklet);
		}
		
	}
	return 0;
}


/**
 * adv_check_event - check event
 * @device - device handle
 * @event_type -
 * @milliseconds -
 */
INT32S adv_check_event(adv_device *device, INT32U *event_type, INT32U milli_seconds)
{
	private_data *privdata = (private_data*)(device->private_data);
	adv_process_info_header *p = &privdata->ptr_process_info;
	INT32S event_th = 0;
//	INT32S ret = 0;
	
	if(!(*event_type))
	{
/* 		ret = wait_event_interruptible_timeout(privdata->event_wait, */
/* 						       adv_process_info_isset_event(p) > 0, */
/* 						       milli_seconds * HZ/1000); */
/* 		if(ret < 0){ */
/* 			return -ERESTARTSYS; */
/* 		} */
	
		event_th = adv_process_info_check_event(p);
		
		if(event_th <= 0){
			*event_type = 0;
		}else{
			switch(event_th -1)
			{
			case 0:
				*event_type = ADS_EVT_DI_INTERRUPT0;
				break;
			case 1:
				*event_type = ADS_EVT_DI_INTERRUPT1;
				break;
			case 2:
				*event_type = ADS_EVT_DI_INTERRUPT2;
				break;
			case 3:
				*event_type = ADS_EVT_DI_INTERRUPT3;
				break;
			case 4:
				*event_type = ADS_EVT_DI_INTERRUPT4;
				break;
			case 5:
				*event_type = ADS_EVT_DI_INTERRUPT5;
				break;
			case 6:
				*event_type = ADS_EVT_DI_INTERRUPT6;
				break;
			case 7:
				*event_type = ADS_EVT_DI_INTERRUPT7;
				break;
			case 8:
				*event_type = ADS_EVT_DEVREMOVED;
				break;
			default:
				*event_type = 0;
			}
			
		}
	  
	} else{
		switch(*event_type)
		{
		case ADS_EVT_DI_INTERRUPT0:
			event_th = 0;
			break;
		case ADS_EVT_DI_INTERRUPT1:
			event_th = 1;
			break;
		case ADS_EVT_DI_INTERRUPT2:
			event_th = 2;
			break;
		case ADS_EVT_DI_INTERRUPT3:
			event_th = 3;
			break;
		case ADS_EVT_DI_INTERRUPT4:
			event_th = 4;
			break;
		case ADS_EVT_DI_INTERRUPT5:
			event_th = 5;
			break;
		case ADS_EVT_DI_INTERRUPT6:
			event_th = 6;
			break;
		case ADS_EVT_DI_INTERRUPT7:
			event_th = 7;
			break;
		case ADS_EVT_DEVREMOVED:
			event_th = 8;
			break;
		default:
			event_th = -1;
			break;
		}

		if((event_th >= 0) && (event_th <=7)){
	/* 		ret =wait_event_interruptible_timeout(privdata->event_wait, */
/* 							 adv_process_info_isset_special_event(p, event_th) > 0, */
/* 							 milli_seconds * HZ/1000); */
/* 			if(ret){ */
/* 				return -ERESTARTSYS; */
/* 			} */
			
			*event_type = adv_process_info_check_special_event(p, event_th);
		}else{
			*event_type = 0;
		}
		
	}
	
	return 0;
}
