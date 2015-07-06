/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1721 Device driver for Linux              
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
  
#include "PCI1721.h"
 

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
/* 		wait_event_interruptible_timeout(privdata->event_wait, */
/* 						 adv_process_info_isset_event(ptr) > 0, */
/* 						 check_event.Milliseconds * HZ / 1000); */

		event_th = adv_process_info_check_event(ptr);

		if (event_th <= 0) {
			check_event.EventType = 0;
		} else {
			switch (event_th - 1) {
			case 0:
				check_event.EventType = ADS_EVT_AO_LOBUF_TRANSFERED;
				break;
			case 1:
				check_event.EventType = ADS_EVT_AO_HIBUF_TRANSFERED;
				break;
			case 2:
				check_event.EventType = ADS_EVT_AO_TERMINATED;
				break;
			case 3:
				check_event.EventType = ADS_EVT_AO_UNDERRUN;
				break;
			default:
				break;
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_AO_LOBUF_TRANSFERED:
			event_th = 0;
			break;
		case ADS_EVT_AO_HIBUF_TRANSFERED:
			event_th = 1;
			break;
		case ADS_EVT_AO_TERMINATED:
			event_th = 2;
			break;
		case ADS_EVT_AO_UNDERRUN:
			event_th = 3;
			break;
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
	

     
	if (copy_from_user(&lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		return -EFAULT;
	}

	privdata->evt_cnt = lp_enable_event.Count;

	if (lp_enable_event.Enabled) {
		switch (lp_enable_event.EventType) {
		case ADS_EVT_AO_LOBUF_TRANSFERED:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      0,
						      privdata->evt_cnt);
			privdata->evt_stat[0] = 1;
			break;
		case ADS_EVT_AO_HIBUF_TRANSFERED:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      1,
						      privdata->evt_cnt);
			privdata->evt_stat[1] = 1;
			break;
		case ADS_EVT_AO_TERMINATED:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      2,
						      privdata->evt_cnt);
			privdata->evt_stat[2] = 1;
			break;
		case ADS_EVT_AO_UNDERRUN:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      3,
						      privdata->evt_cnt);
			privdata->evt_stat[3] = 1;
			break;
		default:
			return -EINVAL;
		}
	} else {
		switch (lp_enable_event.EventType) {
		case ADS_EVT_AO_LOBUF_TRANSFERED:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			privdata->evt_stat[0] = 0;
			break;
		case ADS_EVT_AO_HIBUF_TRANSFERED:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			privdata->evt_stat[1] = 0;
			break;
		case ADS_EVT_AO_TERMINATED:
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			privdata->evt_stat[2] = 0;
			break;
		case ADS_EVT_AO_UNDERRUN:
			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			privdata->evt_stat[3] = 0;
			break;
		default:
			return -EINVAL;
		}
	}
     

	return 0;
}


/**
 * adv_clear_flag - clear OVERRUN flag
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
	case ADS_EVT_AO_LOBUF_TRANSFERED:
		privdata->low_buf_flag = 0;
		break;
	case ADS_EVT_AO_HIBUF_TRANSFERED:
		privdata->high_buf_flag = 0;
		break;
	case ADS_EVT_AO_UNDERRUN:
		privdata->underrun_flag = 0;
		break;
	default:
		return -EINVAL;
	}


	return 0;
}

