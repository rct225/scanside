/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1716 Device driver for Linux              
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
 * 	09/21/2006			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "PCM3718.h"
 

INT32S adv_check_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_CheckEvent check_event;
	INT32S event_th;
	
	int i = 0;

	
	if (copy_from_user(&check_event, arg, sizeof(PT_CheckEvent))) {
		return MemoryCopyFailed;
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
		/*	  printk(KERN_ERR "Low Buffer Ready!\n");
			  for(i = 0; i < 8; ++i) {
			    printk(KERN_ERR "privdata->user_buf[%d] = 0x%x\n", i, privdata->user_buf[i]);
			  }*/
				check_event.EventType = ADS_EVT_AI_LOBUFREADY;
				break;
			case 1:
			/*  printk(KERN_ERR "High Buffer Ready!\n");

			  for(i = 0; i < 8; ++i) {
			    printk(KERN_ERR "privdata->user_buf[%d] = 0x%x\n", i, privdata->user_buf[i]);
			  }*/

				check_event.EventType = ADS_EVT_AI_HIBUFREADY;
				break;
			case 2:
				check_event.EventType = ADS_EVT_AI_TERMINATED;
				break;
			case 3:
				check_event.EventType = ADS_EVT_AI_OVERRUN;
				break;
			case 4:
				check_event.EventType = ADS_EVT_AI_INTERRUPT;
				break;
			default:
				break;
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_AI_LOBUFREADY:
			event_th = 0;
			break;
		case ADS_EVT_AI_HIBUFREADY:
			event_th = 1;
			break;
		case ADS_EVT_AI_TERMINATED:
			event_th = 2;
			break;
		case ADS_EVT_AI_OVERRUN:
			event_th = 3;
			break;
		case ADS_EVT_AI_INTERRUPT:
			event_th = 4;
			break;
		default:
			return InvalidEventType;
		}
		check_event.EventType = adv_process_info_check_special_event(ptr, event_th);		
	}
	
	if (copy_to_user(arg, &check_event, sizeof(PT_CheckEvent))) {
		return MemoryCopyFailed;
	}
	
	return 0;
}
		  
  
INT32S adv_enable_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_EnableEvent *lp_enable_event;
	

	lp_enable_event = kmalloc(sizeof(PT_EnableEvent), GFP_KERNEL);
	if (!lp_enable_event) {
		return MemoryAllocateFailed;
	}
     
	if (copy_from_user(lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		return MemoryCopyFailed;
	}

	privdata->evt_cnt = lp_enable_event->Count;

	if (lp_enable_event->Enabled) {
		switch (lp_enable_event->EventType) {
		case ADS_EVT_AI_LOBUFREADY:
			adv_process_info_enable_event(&privdata->ptr_process_info, 0, privdata->evt_cnt);
			break;
		case ADS_EVT_AI_HIBUFREADY:
			adv_process_info_enable_event(&privdata->ptr_process_info, 1, privdata->evt_cnt);
			break;
		case ADS_EVT_AI_TERMINATED:
			adv_process_info_enable_event(&privdata->ptr_process_info, 2, privdata->evt_cnt);
			break;
		case ADS_EVT_AI_OVERRUN:
			adv_process_info_enable_event(&privdata->ptr_process_info, 3, privdata->evt_cnt);
			break;
		case ADS_EVT_AI_INTERRUPT:
			adv_process_info_enable_event(&privdata->ptr_process_info, 4, privdata->evt_cnt);
			break;
		default:
			break;
		}
	} else {
		switch (lp_enable_event->EventType) {
		case ADS_EVT_AI_LOBUFREADY:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			break;
		case ADS_EVT_AI_HIBUFREADY:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			break;
		case ADS_EVT_AI_TERMINATED:
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			break;
		case ADS_EVT_AI_OVERRUN:
			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			break;
		case ADS_EVT_AI_INTERRUPT:
			adv_process_info_disable_event(&privdata->ptr_process_info, 4);
			break;
		default:
			break;
		}
	}
     
	kfree(lp_enable_event);	

	return 0;
}



INT32S adv_clear_flag(adv_device *device, void *arg)
{

	return SUCCESS;
}
