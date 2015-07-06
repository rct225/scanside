/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1760 Device driver for Linux              
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
 * 	05/16/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1760.h"
 

INT32S adv_check_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_CheckEvent check_event;
	INT32S event_th = -1;
	
		
	if (copy_from_user(&check_event, arg, sizeof(PT_CheckEvent))) {
		return -ENOMEM;
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
				check_event.EventType = ADS_EVT_DI_PATTERNMATCH_PORT0;
				break;
			case 1:
				check_event.EventType = ADS_EVT_DI_STATUSCHANGE_PORT0;
				break;
			case 2:
				check_event.EventType = ADS_EVT_COUNTERMATCH;
				break;
			case 3:
				check_event.EventType = ADS_EVT_COUNTEROVERFLOW;
				break;
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_DI_PATTERNMATCH_PORT0:
			event_th = 0;
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT0:
			event_th = 1;
			break;
		case ADS_EVT_COUNTERMATCH:
			event_th = 2;
			break;
		case ADS_EVT_COUNTEROVERFLOW:
			event_th = 3;
			break;
		default:
			break;
		}
/* 		wait_event_interruptible_timeout(privdata->event_wait, */
/* 						 adv_process_info_isset_special_event(ptr, event_th) > 0, */
/* 						 check_event.Milliseconds * HZ / 1000);			 */
		
		if ((check_event.EventType >= 0) && (check_event.EventType <= 3)) {
			check_event.EventType = adv_process_info_check_special_event(ptr, event_th);		
		} else {
			check_event.EventType = 0;
		}
		

	}
	
	if (copy_to_user(arg, &check_event, sizeof(PT_CheckEvent))) {
		return -ENOMEM;
	}
	
	return 0;
}
		  
  

INT32S adv_enable_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_EnableEvent *lp_enable_evt;


	lp_enable_evt = kmalloc(sizeof(PT_EnableEvent), GFP_KERNEL);
	if (!lp_enable_evt) {
		return -ENOMEM;
	}
     
	if (copy_from_user(lp_enable_evt, arg, sizeof(PT_EnableEvent))) {
		return -ENOMEM;
	}

     
	if (lp_enable_evt->Enabled) {
		if (!lp_enable_evt->Count) {
			return -EINVAL;
		}

		switch (lp_enable_evt->EventType) {
		case ADS_EVT_DI_PATTERNMATCH_PORT0:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      0,
						      lp_enable_evt->Count);
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT0:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      1,
						      lp_enable_evt->Count);
			break;
		case ADS_EVT_COUNTERMATCH:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      2,
						      lp_enable_evt->Count);
			break;
		case ADS_EVT_COUNTEROVERFLOW:
				adv_process_info_enable_event(&privdata->ptr_process_info,
							      3,
							      lp_enable_evt->Count);

			break;
		default:
			break;
		}
	} else {
		switch (lp_enable_evt->EventType) {
		case ADS_EVT_DI_PATTERNMATCH_PORT0:
			adv_process_info_disable_event(&privdata->ptr_process_info,
						      0);
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT0:
			adv_process_info_disable_event(&privdata->ptr_process_info,
						      1);
			break;
		case ADS_EVT_COUNTERMATCH:
			adv_process_info_disable_event(&privdata->ptr_process_info,
						      2);
			break;
		case ADS_EVT_COUNTEROVERFLOW:
				adv_process_info_disable_event(&privdata->ptr_process_info,
							      3);

			break;
		default:
			break;
		}

	}
     
	kfree(lp_enable_evt);

	return 0;
}



INT32S adv_clear_flag(adv_device *device, void *arg)
{

	return 0;
}
