/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCM-3724 Device driver for Linux              
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
 * 	08/21/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "pcm3724.h"
 

INT32S adv_check_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_CheckEvent check_event;
	INT32S event_th;
	

	/* printk("%s is called...\n", __FUNCTION__); */

	if (copy_from_user(&check_event, arg, sizeof(PT_CheckEvent))) {
		return -EFAULT;
	}

	if (!check_event.EventType) {
/* 		wait_event_interruptible_timeout(privdata->event_wait, */
/* 						 adv_process_info_isset_event(ptr) > 0, */
/* 						 check_event.Milliseconds * HZ / 1000); */

		event_th = adv_process_info_check_event(ptr);

		if (event_th <= 0) {
			check_event.EventType = 0;
		} else {
			switch (event_th - 1) {
			case 0:
				check_event.EventType = ADS_EVT_DI_INTERRUPT0;
				break;
			case 1:
				check_event.EventType = ADS_EVT_DI_INTERRUPT1;
				break;
/* 			case 9: */
/* 				check_event.EventType = ADS_EVT_TIME_OUT; */
/* 				break; */
			default:
				return -EINVAL;
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			event_th = 0;
			break;
		case ADS_EVT_DI_INTERRUPT1:
			event_th = 1;
			break;
/* 		case ADS_EVT_TIME_OUT: */
/* 			event_th = 9; */
/* 			break; */
		default:
			return -EINVAL;
		}

/* 		wait_event_interruptible_timeout(privdata->event_wait, */
/* 						 adv_process_info_isset_special_event(ptr, event_th) > 0, */
/* 						 check_event.Milliseconds * HZ / 1000); */

		check_event.EventType = adv_process_info_check_special_event(ptr, event_th);
	}
	
	if (copy_to_user(arg, &check_event, sizeof(PT_CheckEvent))) {
		return -EFAULT;
	}
	
	return 0;
}
		  
  
INT32S adv_enable_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_EnableEvent lp_enable_event;
	INT16U int_src = 0;
	


/* 	lp_enable_event = kmalloc(sizeof(PT_EnableEvent), GFP_KERNEL); */
/* 	if (!lp_enable_event) { */
/* 		return -ENOMEM; */
/* 	} */
 
	if (copy_from_user(&lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		return -EFAULT;
	}
	
	privdata->evt_cnt = lp_enable_event.Count;
	int_src = privdata->int_src;

	if (lp_enable_event.Enabled) {
		switch (lp_enable_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0: 
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      0,
						      privdata->evt_cnt);
/* 			int_src |= 0x0100; */
			break;
		case ADS_EVT_DI_INTERRUPT1: 
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      1,
						      privdata->evt_cnt);
/* 			int_src |= 0x0100; */
			break;
/* 		case ADS_EVT_TIME_OUT: */
/* 			adv_process_info_enable_event(&privdata->ptr_process_info, */
/* 						      9, privdata->evt_cnt); */
/* 			break; */
		default:
			return -EINVAL;
		}
	} else {
		switch (lp_enable_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0: 
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
/* 			int_src &= 0xfeff; */
			break;
		case ADS_EVT_DI_INTERRUPT1: 
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
/* 			int_src &= 0xfeff; */
			break;
/* 		case ADS_EVT_TIME_OUT: */
/* 			adv_process_info_disable_event(&privdata->ptr_process_info, 9); */
/* 			break; */
		default:
			return -EINVAL;
		}
	}
/* 	advOutpw(privdata, 0x42, int_src); /\* open conresponding interrupt *\/ */
/* 	privdata->int_src = int_src; */

	/* printk("0x42: 0x%x\n", int_src); */
	
/* 	kfree(lp_enable_event);	 */

	return 0;
}



INT32S adv_clear_flag(adv_device *device, void *arg)
{

	return 0;
}
