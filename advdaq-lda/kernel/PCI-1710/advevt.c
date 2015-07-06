/********************************************************************
 *				                                    
 * 	Copyright 2006 ABJ Linux Driver Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1710 Device driver for Linux              
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
 * 	12/12/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1710.h"
 

INT32S adv_check_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_CheckEvent check_event;
	INT32S event_th;
	

	
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
				check_event.EventType = ADS_EVT_AI_INTERRUPT;
				break;
			case 1:
				check_event.EventType = ADS_EVT_AI_LOBUFREADY;
				break;
			case 2:
				check_event.EventType = ADS_EVT_AI_HIBUFREADY;
				break;
			case 3:
				check_event.EventType = ADS_EVT_AI_TERMINATED;
				break;
			case 4:
				check_event.EventType = ADS_EVT_AI_OVERRUN;
				break;
			case 5:
				check_event.EventType = ADS_EVT_TIME_OUT;
				break;
			default:
				return -EINVAL;
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_AI_INTERRUPT:
			event_th = 0;
			break;
		case ADS_EVT_AI_LOBUFREADY:
			event_th = 1;
			break;
		case ADS_EVT_AI_HIBUFREADY:
			event_th = 2;
			break;
		case ADS_EVT_AI_TERMINATED:
			event_th = 3;
			break;
		case ADS_EVT_AI_OVERRUN:
			event_th = 4;
			break;
		case ADS_EVT_TIME_OUT:
			event_th = 5;
			break;
		default:
			return -EINVAL;
		}

		wait_event_interruptible_timeout(privdata->event_wait,
						 adv_process_info_isset_special_event(ptr, event_th) > 0,
						 check_event.Milliseconds * HZ / 1000);

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


     
	if (copy_from_user(&lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		return -EFAULT;
	}

	privdata->evt_cnt = lp_enable_event.Count;

	if (lp_enable_event.Enabled) {
		switch (lp_enable_event.EventType) {
		case ADS_EVT_AI_INTERRUPT: 
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      0,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_AI_LOBUFREADY:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      1,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_AI_HIBUFREADY:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      2,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_AI_TERMINATED:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      3,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_AI_OVERRUN:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      4,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_TIME_OUT:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      5, privdata->evt_cnt);
			break;
		default:
			return -EINVAL;
		}
	} else {
		switch (lp_enable_event.EventType) {
		case ADS_EVT_AI_INTERRUPT:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			break;
		case ADS_EVT_AI_LOBUFREADY:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			break;
		case ADS_EVT_AI_HIBUFREADY:
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			break;
		case ADS_EVT_AI_TERMINATED:
			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			break;
		case ADS_EVT_AI_OVERRUN:
			adv_process_info_disable_event(&privdata->ptr_process_info, 4);
			break;
		case ADS_EVT_TIME_OUT:
			adv_process_info_disable_event(&privdata->ptr_process_info, 5);
			break;
		default:
			return -EINVAL;
		}
	}
     

	return 0;
}



/**
 * adv_clear_flag - clear flags
 *
 * @device: Point to the device object
 */
INT32S adv_clear_flag(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U flag;
	

	if (copy_from_user(&flag, arg, sizeof(INT16U))) {
		return -EFAULT;
	}

	switch (flag) {
	case ADS_EVT_AI_LOBUFREADY:
		privdata->halfready_flag = 2;
		break;
	case ADS_EVT_AI_HIBUFREADY:
		privdata->halfready_flag = 1;
		break;
	case ADS_EVT_AI_OVERRUN:
		privdata->overrun_flag = 0;
		/* privdata->fifo_overrun = 0; */
		break;
	default:
		return -EINVAL;
	}
	
	return 0;
}
