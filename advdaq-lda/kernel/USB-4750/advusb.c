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
 * 	advusb.c                                             
 * Abstract:                                                   
 * 	This file contains routines for usb.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	03/15/2007			Create by li.ang
 *                                                                  
 ********************************************************************/
  
#include "USB4750.h"

INT32S adv_locate_usb_device(adv_usb_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret = 0;
	INT32U tmp;


	if (down_interruptible(privdata->usb_urb_sema)) {
		return -EFAULT;
	}
	
	if (privdata->udev == NULL) {
		up(privdata->usb_urb_sema);
		return -ENODEV;
	}
	
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_DEVICE_OPEN, 0,
				  (INT8U *) &tmp,
				  sizeof(INT32U));

	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_LOCATE, 0,
				  (INT8U *) &tmp,
				  sizeof(INT32U));

	up(privdata->usb_urb_sema);

	
	return ret;	
}

INT32S adv_stop_locate_usb_device(adv_usb_device *device)
{
	private_data *privdata = (private_data*)device->private_data;
	INT32U tmp;
	INT32S ret = 0;

	
	if (down_interruptible(privdata->usb_urb_sema)) {
		return -EFAULT;
	}
	
	if (privdata->udev == NULL) {
		up(privdata->usb_urb_sema);
		return -ENODEV;
	}

	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_DEVICE_CLOSE, 0,
				  (INT8U *) &tmp,
				  sizeof(INT32U));

	up(privdata->usb_urb_sema);

	return ret;	
}

INT32S adv_usb_ctrl_transfer(adv_usb_device *device, void *arg)
{
	private_data *privdata = (private_data *)(device->private_data);
	USB_TRANSFER_INFO *trans_info;
	SETUP_PACKET *setup_packet;
	INT8U *buf;
	INT32S ret = 0;
	

	trans_info = kmalloc(sizeof(USB_TRANSFER_INFO), GFP_KERNEL);
	if (trans_info == NULL) {
		return -ENOMEM;
	}
	
	ret = copy_from_user(trans_info, arg, sizeof(USB_TRANSFER_INFO));
	if(ret){
		return -EFAULT;
	}
	
	setup_packet = (SETUP_PACKET *) trans_info;
	
	buf = (INT8U *) kmalloc(setup_packet->wLength, GFP_KERNEL);
	if (buf == NULL) {
		return -ENOMEM;
	}

	ret = copy_from_user(buf, trans_info->pBuffer, setup_packet->wLength);
	if (ret) {
		return -EFAULT;
	}
	
	if (privdata->udev == NULL) { /* check udev is very important */
		return -ENODEV;
	}
	
	if (setup_packet->bmRequestType & 0x80) {
		/* In type */
		ret = adv_usb_ctrl_msg(privdata->udev,
				       usb_rcvctrlpipe(privdata->udev, 0),
				       setup_packet->bRequest,
				       setup_packet->bmRequestType,
				       setup_packet->wValue,
				       setup_packet->wIndex, buf,
				       setup_packet->wLength);
	} else {
		/* Out type */
		ret = adv_usb_ctrl_msg(privdata->udev,
				       usb_sndctrlpipe(privdata->udev, 0),
				       setup_packet->bRequest,
				       setup_packet->bmRequestType,
				       setup_packet->wValue,
				       setup_packet->wIndex, buf,
				       setup_packet->wLength);
	}
	
	if (ret < 0) {
		return ret;
	}
	
	if (setup_packet->bmRequestType & 0x80) {
		ret = copy_to_user(trans_info->pBuffer, buf, setup_packet->wLength);
		if (ret) {
			return -EFAULT;
		}
	}

	return 0;
}

VOID urb_callback_fn(struct urb *urb, struct pt_regs *regs)
{
	private_data *privdata = (private_data *) urb->context;
	SW_EVENT_FIFO *event_fifo;
	INT16U event_num;
	

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		printk("urb error!\n");
		return;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
		return;
	}

	event_fifo = (SW_EVENT_FIFO *) (privdata->event_buf);
	event_num = event_fifo->EventCount;		

	/* printk("event num: %d\n", event_num); */
	
	if ((event_num > 0)
	    && (event_num <= MAX_EVENT_NUM)
	    && (atomic_read(&privdata->tasklet_processed))) {
		atomic_set(&privdata->tasklet_processed, 0);
		
		memcpy(privdata->event_buf_clone,
		       privdata->event_buf,
		       MAX_EVTBUF_LEN);
		
		tasklet_schedule(&privdata->urb_tasklet);
	}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
	adv_usb_submit_urb(urb, GFP_ATOMIC);
#endif
	
	return;
}

void urb_tasklet_fn(PTR_T arg)
{
	private_data *privdata = (private_data *) arg;
	SW_EVENT_FIFO *event_fifo;
	EVENT_DATA *event_data;
	INT16U event_num;
	INT16U tmp;
	INT16U i;


	/* if have event */
	event_fifo = (SW_EVENT_FIFO *) (privdata->event_buf_clone);
	event_num = event_fifo->EventCount;		
	event_data = event_fifo->EventDataFifo;


	for (i = 0; i < event_num; i++) {
		if (event_data->EventType > 2) {
			event_data++;
			continue;
		}

		if (event_data->EventType == EVENT_ID_DI0) {
			
			tmp = event_data->PortData[1];
			tmp = ((tmp << 8) & 0xff00) | (event_data->PortData[0]);
			privdata->di_evt_cfg[0].evt_data = tmp;
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       0, 1);
		}

		if (event_data->EventType == EVENT_ID_DI8) {

			tmp = event_data->PortData[1];
			tmp = ((tmp << 8) & 0xff00) | (event_data->PortData[0]);
			privdata->di_evt_cfg[1].evt_data = tmp;
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       1, 1);
		}
			
		event_data++;
	}

	atomic_set(&privdata->tasklet_processed, 1);
	wake_up_interruptible(&privdata->event_wait);

	return;
}
