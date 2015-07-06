/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1712 Device driver for Linux              
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
  
#include "PCI1712.h"
 

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
		event_th = adv_process_info_check_event(ptr);

		if (event_th <= 0) {
			check_event.EventType = 0;
		} else {
			switch (event_th - 1) {
			case 0:
				check_event.EventType = ADS_EVT_AI_LOBUFREADY;
				break;
			case 1:
				check_event.EventType = ADS_EVT_AI_HIBUFREADY;
				break;
			case 2:
				check_event.EventType = ADS_EVT_AI_TERMINATED;
				break;
			case 3:
				check_event.EventType = ADS_EVT_AI_OVERRUN;
				break;
			case 4:
				check_event.EventType = ADS_EVT_AO_LOBUF_TRANSFERED;
				break;
			case 5:
				check_event.EventType = ADS_EVT_AO_HIBUF_TRANSFERED;
				break;
			case 6:
				check_event.EventType = ADS_EVT_AO_TERMINATED;
				break;
			case 7:
				check_event.EventType = ADS_EVT_AO_UNDERRUN;
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
		case ADS_EVT_AO_LOBUF_TRANSFERED:
			event_th = 4;
			break;
		case ADS_EVT_AO_HIBUF_TRANSFERED:
			event_th = 5;
			break;
		case ADS_EVT_AO_TERMINATED:
			event_th = 6;
			break;
		case ADS_EVT_AO_UNDERRUN:
			event_th = 7;
			break;
		default:
			return -EINVAL;
		}
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
	INT16U evt_cnt;
	
     
	if (copy_from_user(&lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		return -EFAULT;
	}

	evt_cnt = lp_enable_event.Count;

	if (lp_enable_event.Enabled) {
		switch (lp_enable_event.EventType) {
		case ADS_EVT_AI_LOBUFREADY: 
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      0,
						      evt_cnt);
			break;
		case ADS_EVT_AI_HIBUFREADY: 
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      1,
						      evt_cnt);
			break;

		case ADS_EVT_AI_TERMINATED:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      2,
						      evt_cnt);
			break;
		case ADS_EVT_AI_OVERRUN:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      3,
						      evt_cnt);
			break;
		case ADS_EVT_AO_LOBUF_TRANSFERED:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      4,
						      evt_cnt);
			break;
		case ADS_EVT_AO_HIBUF_TRANSFERED:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      5,
						      evt_cnt);
			break;
		case ADS_EVT_AO_TERMINATED:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      6,
						      evt_cnt);
			break;
		case ADS_EVT_AO_UNDERRUN:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      7,
						      evt_cnt);
			break;
		default:

			return -EINVAL;
		}
	} else {
		switch (lp_enable_event.EventType) {
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

		case ADS_EVT_AO_LOBUF_TRANSFERED:
			adv_process_info_disable_event(&privdata->ptr_process_info, 4);
			break;
		case ADS_EVT_AO_HIBUF_TRANSFERED:
			adv_process_info_disable_event(&privdata->ptr_process_info, 5);
			break;
		case ADS_EVT_AO_TERMINATED:
			adv_process_info_disable_event(&privdata->ptr_process_info, 6);
			break;
		case ADS_EVT_AO_UNDERRUN:
			adv_process_info_disable_event(&privdata->ptr_process_info, 7);
			break;

		default:
			return -EINVAL;
		}
	}
     
	return 0;
}



INT32S adv_clear_flag(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	privdata->overrun_flag = 0;
	return 0;
}
