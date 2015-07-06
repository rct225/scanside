/********************************************************************
 *				                                    
 * 	Copyright 2007 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech USB-4751 Device driver for Linux              
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
 * 	01/26/2007			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "USB4751.h"

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
	INT32S event_th=0;

	if (copy_from_user(&check_event, arg, sizeof(PT_CheckEvent))) {
		return -EFAULT;
	}

	if (!check_event.EventType) {
		wait_event_interruptible_timeout(privdata->event_wait,
						 adv_process_info_isset_event(ptr) > 0,
						 check_event.Milliseconds * HZ / 1000);

		event_th = adv_process_info_check_event(ptr);

		if (event_th <= 0) {
			check_event.EventType = 0;
		} else {
			switch (event_th - 1) {
			case 0:
				check_event.EventType = ADS_EVT_DI_INTERRUPT16;
				break;
				
			case 1:
				check_event.EventType = ADS_EVT_DI_INTERRUPT40;
				break;

			case 2:
				check_event.EventType = ADS_EVT_DEVREMOVED;
				break;
				
			default:
				return -EFAULT;
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_DI_INTERRUPT16:
			event_th = 0;
			break;
			
		case ADS_EVT_DI_INTERRUPT40:
			event_th = 1;
			break;

		case ADS_EVT_DEVREMOVED:
			event_th = 2;
			break;
			
		default:
			return -EFAULT;
		}
		check_event.EventType = adv_process_info_check_special_event(ptr, event_th);		
	}
	
	if (copy_to_user(arg, &check_event, sizeof(PT_CheckEvent))) {
		return -EFAULT;
	}
	
	return 0;
}
		  
 /**
 * adv_enable_event - enable or disable a special event with count
 * @device - pointer to a special device
 * @EventType - special event type
 * @Enabled - enable or disable
 * @Count - the number of interrupt arouse a special event
 */ 
INT32S adv_enable_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_EnableEvent *lp_enable_event;
	INT32S ret=0;
	int event_enabled = 0;
	unsigned pipe;

	lp_enable_event = kmalloc(sizeof(PT_EnableEvent), GFP_KERNEL);
	if (!lp_enable_event) {
		return -EFAULT;
	}
     
	if (copy_from_user(lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		kfree(lp_enable_event);	
		return -EFAULT;
	}

	privdata->evt_cnt = lp_enable_event->Count;
	privdata->evt_style = lp_enable_event->Enabled;

	if (lp_enable_event->Enabled) {
		if(adv_process_info_isenable_event_all(&privdata->ptr_process_info, 0)){
			event_enabled = 1;
		}
		if(adv_process_info_isenable_event_all(&privdata->ptr_process_info, 1)){
			event_enabled = 1;
		}
		switch (lp_enable_event->EventType) {
			case ADS_EVT_DI_INTERRUPT16:
				adv_process_info_enable_event(&privdata->ptr_process_info, 0, lp_enable_event->Count);
				break;
			case ADS_EVT_DI_INTERRUPT40:
				adv_process_info_enable_event(&privdata->ptr_process_info, 1, lp_enable_event->Count);
				break;
			case ADS_EVT_DEVREMOVED:
				return 0;
		default:
			ret = -EFAULT;
		}
	
        pipe = usb_rcvintpipe(privdata->udev,privdata->int_endpointAddr);
		if(event_enabled==0){
			usb_fill_int_urb(privdata->urb_int, privdata->udev,
			pipe,
		        (void *)privdata->urb_int_transfer, 
			MAX_EVTBUF_LEN,
		        (usb_complete_t)urb_int_callback,
			 privdata, 1);

			//if((ret=adv_usb_submit_urb(privdata->urb_int,SLAB_ATOMIC))!=0){
			ret = adv_usb_submit_urb(privdata->urb_int,GFP_KERNEL);
			if(ret){
				printk("error ret: %d\n", ret);
				return -EIO;
			}
		}
	} else {
		switch (lp_enable_event->EventType) {
			case ADS_EVT_DI_INTERRUPT16:
				adv_process_info_disable_event(&privdata->ptr_process_info, 0);
				break;
			case ADS_EVT_DI_INTERRUPT40:
				adv_process_info_disable_event(&privdata->ptr_process_info, 1);
				break;
			case ADS_EVT_DEVREMOVED:
				adv_process_info_disable_event(&privdata->ptr_process_info, 2);
				break;
		default:
			ret = -EFAULT;
		}
		//usb_unlink_urb(privdata->urb_int);
		if(adv_process_info_isenable_event_all(&privdata->ptr_process_info, 0)){
			event_enabled = 1;
		}
		if(adv_process_info_isenable_event_all(&privdata->ptr_process_info, 1)){
			event_enabled = 1;
		}
		if(event_enabled==0){
			usb_kill_urb(privdata->urb_int);
		}
		privdata->evt_style = lp_enable_event->Enabled;
	}
     
	kfree(lp_enable_event);	

	return ret;
}


