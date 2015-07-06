/********************************************************************
 *				                                    
 * 	Copyright 2007 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech USB-4716 Device driver for Linux              
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
  
#include "USB4716.h"

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

	if (privdata->udev == NULL) {
		up(privdata->usb_urb_sema);
		return -ENODEV;
	}

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
	URB_QUEUE *urb_queue = (URB_QUEUE *) urb->context;
	private_data *privdata = (private_data *) urb_queue->privdata;
	adv_user_page *user_page;
	INT16U cache_offset = 0;
	INT16U data_size = 0;
	INT16U wdata;
	INT32S trans_cnt = 0;
	INT32S tmp = 0;

	

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	default:
		return;
	}

/* #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)) */
/* 	if (privdata->fai_running == 0) { */
/* 		return; */
/* 	} */
/* #endif */
	
	trans_cnt = urb->actual_length;
	while (trans_cnt > 0) {
		if (privdata->hungry_page) { /* we got hungry_page, we must fill the bla-
					      * nk area of this page.
					      *
					      * fortunately, we remember the length of f-
					      * illed, the 'load_size', now we can cacul-
					      * ate the size of blank area (data_size).
					      */
			user_page = privdata->hungry_page;
			data_size = user_page->length - privdata->load_size;

			memcpy((VOID *) (user_page->page_addr + user_page->offset
					 + privdata->load_size),
			       urb_queue->kern_cache + cache_offset,
			       data_size);

			privdata->hungry_page = NULL;
		} else {
			user_page = privdata->user_pages + privdata->page_idx;

			tmp = trans_cnt - user_page->length;
			if (tmp < 0) { /* an insufficiency of data, can not fill a whole
					* page. so, we just fill the 'trans_cnt' length.
					*
					* then, we remember the length of filled and th-
					* e pointer of this page (naming 'hungry page').
					*/
				data_size = trans_cnt;
				privdata->hungry_page = user_page;
				privdata->load_size = data_size;
			} else {
				data_size = user_page->length;
			}

			memcpy((VOID *) (user_page->page_addr + user_page->offset),
			       urb_queue->kern_cache + cache_offset,
			       data_size);

			privdata->page_idx += 1;
			privdata->page_idx %= privdata->page_num;
		}

		wdata = *((INT16U *) (urb_queue->kern_cache + cache_offset));
		if (wdata & 0x8000) { /* HW fifo is overrun */
			privdata->fifo_overrun = 1;
		}

		trans_cnt -= data_size;
		cache_offset += data_size;
	}

	privdata->recieve_cnt += urb->actual_length;

	/* we must insure that recieve_cnt will not be overflowed */
	privdata->recieve_cnt %= (privdata->conv_num * sizeof(INT16U));

	tasklet_schedule(&privdata->urb_tasklet);

	/* we must refill the bulk urb, and submit again */
	if (privdata->udev == NULL) {
		return;
	}

	usb_fill_bulk_urb(urb_queue->urb, privdata->udev,
			  usb_rcvbulkpipe(privdata->udev, privdata->bulk_endpointAddr),
			  (INT8U *) urb_queue->kern_cache,
			  urb_queue->urb_buf_len,
			  (usb_complete_t) urb_callback_fn,
			  urb_queue);

	if (privdata->udev == NULL) {
		return;
	}
	
	adv_usb_submit_urb(urb, GFP_ATOMIC);

	return;
}

void urb_tasklet_fn(PTR_T arg)
{
	private_data *privdata = (private_data *) arg;
	INT32U recieved = privdata->recieve_cnt;
	

	recieved /= sizeof(INT16U);
	recieved %= privdata->conv_num;
	if (recieved == 0) {
		recieved = privdata->conv_num;
	}
	
	/* set event */
	if (recieved == privdata->conv_num / 2) { /* low buffer ready */
		privdata->half_ready = 1;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);

		if (privdata->halfready_flag == 2) { /* overrun */
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       4, 1);
			privdata->overrun_flag = 1;
		}
		privdata->halfready_flag = 1;
		privdata->recieved = recieved;
	}
	
	if (recieved == privdata->conv_num) {
		privdata->half_ready = 2;

		if (!privdata->cyclic) { /* terminate */
			privdata->terminated = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);

			/* we schedule 'fai_stop_work' thread to terminate fai */
			if (privdata->fai_running) {
				schedule_work(&privdata->fai_stop_work);
			}

		} else {	/* high buffer ready */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
			
			if (privdata->halfready_flag == 1) { /* overrun */
				adv_process_info_set_event_all(&privdata->ptr_process_info,
							       4, 1);
				privdata->overrun_flag = 1;
			}
			privdata->halfready_flag = 2;
			privdata->recieved = recieved;
		}
	}
	
	if (recieved == privdata->int_cnt) { /* interrupt count */
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}

	wake_up_interruptible(&privdata->event_wait);

	return;
}
