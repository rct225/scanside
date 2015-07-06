/********************************************************************
 *				                                    
 * 	Copyright 2007 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech USB-4750 Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	advevt.c                                             
 * Abstract:                                                   
 * 	This file contains routines for events.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	03/15/2007			Create by li.ang
 *                                                                  
 ********************************************************************/
  
#include "USB4750.h"

/**
 * adv_check_event - check device's event
 * @device - point to a special device
 * @EventType - special event type or NULL
 * @Milliseconds - the dead line of waiting for event
 *
 * 1.EventType == NULL
 *   EventType:
 *       return with whatever event if happened
 *       or NULL when time out
 * 2.EventType is a special event type
 *   EventType:
 *       return 1 if the event happened
 *       or 0 if the event not happened
 */ 
INT32S adv_check_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_CheckEvent check_event;
	INT32S event_th = 0;


	if (copy_from_user(&check_event, arg, sizeof(PT_CheckEvent))) {
		return -EFAULT;
	}

	if (!check_event.EventType) {

		event_th = adv_process_info_check_event(ptr);
		
		if (event_th <= 0) {
			check_event.EventType = 0;
		} else {
			switch (event_th - 1) {
			case 0:
				check_event.EventType = ADS_EVT_DI_INTERRUPT0;
				break;
			case 1:
				check_event.EventType = ADS_EVT_DI_INTERRUPT8;
				break;
			case 2:
				check_event.EventType = ADS_EVT_DEVREMOVED;
				break;
			default:
				break;
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			event_th = 0;
			break;
		case ADS_EVT_DI_INTERRUPT8:
			event_th = 1;
			break;
		case ADS_EVT_DEVREMOVED:
			event_th = 2;
			break;
		default:
			break;
		}

		check_event.EventType = adv_process_info_check_special_event(ptr, event_th);
	}
	
	if (copy_to_user(arg, &check_event, sizeof(PT_CheckEvent))) {
		return -EFAULT;
	}
	
	return 0;
}

static INT32S usb_enable_event(adv_device *device, INT16U type, INT16U strobe)
{
	private_data *privdata = (private_data *) (device->private_data);
	USB_EnableEvent usb_enable_evt;
	INT32S ret;
	

	usb_enable_evt.Enabled = strobe;

	switch (type) {
	case ADS_EVT_DI_INTERRUPT0:
		usb_enable_evt.EventType = EVENT_ID_DI0;

		if (privdata->trig_edge & 0x0001) {
			usb_enable_evt.EventTrigger = 1;
		} else {
			usb_enable_evt.EventTrigger = 0;
		}		

		break;
	case ADS_EVT_DI_INTERRUPT8:	
		usb_enable_evt.EventType = EVENT_ID_DI8;

		if (privdata->trig_edge & 0x0100) {
			usb_enable_evt.EventTrigger = 1;
		} else {
			usb_enable_evt.EventTrigger = 0;
		}		

		break;
	default:
		return -EINVAL;
	}
	
	if (down_interruptible(privdata->usb_urb_sema)) {
		return -EFAULT;
	}
	
	if (privdata->udev == NULL) {
		up(privdata->usb_urb_sema);
		return -ENODEV;
	}

	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			       MAJOR_EVENT, 0x40,
			       MINOR_EVENT_ENABLE, 0,
			       (INT8U *) &usb_enable_evt,
			       sizeof(USB_EnableEvent));
	if (ret < 0) {
		up(privdata->usb_urb_sema);
		return ret;
	}

	up(privdata->usb_urb_sema);

	return 0;
}

 /**
 * adv_enable_event - enable or disable a special event with count
 * 
 * @device - pointer to a special device
 * @EventType - special event type
 * @Enabled - enable or disable
 * @Count - the number of interrupt arouse a special event
 */ 
INT32S adv_enable_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_process_info_header *p = &privdata->ptr_process_info;
	PT_EnableEvent *lp_enable_event;
	INT16U event_enabled = 0;
	INT32S ret = 0;
	INT16U i;
	

	lp_enable_event = kmalloc(sizeof(PT_EnableEvent), GFP_KERNEL);
	if (!lp_enable_event) {
		return -ENOMEM;
	}

	if (copy_from_user(lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		kfree(lp_enable_event);
		return -EFAULT;
	}
	
	/* testing any event has enabled before */
	for (i = 0; i < MAX_EVENT_NUM; i++) {
		if (adv_process_info_isenable_event_all(p, i)) {
			event_enabled = 1;
			break;
		}
	}

        /* process event */
	if (lp_enable_event->Enabled) {
		switch (lp_enable_event->EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      0,
						      lp_enable_event->Count);
			break;
		case ADS_EVT_DI_INTERRUPT8:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      1,
						      lp_enable_event->Count);
			break;
		default:
			ret = -EINVAL;
			kfree(lp_enable_event);
			return ret;
		}

		if (!event_enabled) {
			adv_usb_submit_urb(privdata->urb_int, GFP_KERNEL);
		}

		/* communication with device, enable the event */
		usb_enable_event(device,
				 lp_enable_event->EventType,
				 lp_enable_event->Enabled);
	} else {
		switch (lp_enable_event->EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			break;
		case ADS_EVT_DI_INTERRUPT8:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			break;
		default:
			ret = -EINVAL;
			kfree(lp_enable_event);
			return ret;
		}

		event_enabled = 0;
		
		/* testing any event has enabled before */
		for (i = 0; i < MAX_EVENT_NUM; i++) {
			if (adv_process_info_isenable_event_all(p, i)) {
				event_enabled = 1;
				break;
			}
		}

		if (!event_enabled) {
			usb_kill_urb(privdata->urb_int);

			/* communication with device, disable the event */
			usb_enable_event(device,
					 lp_enable_event->EventType,
					 lp_enable_event->Enabled);
		}
	}

	kfree(lp_enable_event);

	
	return ret;
}
