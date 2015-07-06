/********************************************************************
 *				                                    
 * 	Copyright 2007 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech USB-4711 Device driver for Linux              
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
  
#include "USB4711.h"

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
				check_event.EventType = ADS_EVT_DEVREMOVED;
				break;
			default:
				break;
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
		case ADS_EVT_DEVREMOVED:
			event_th = 5;
			break;
		default:
			return InvalidEventType;
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
 * 
 * @device - pointer to a special device
 * @EventType - special event type
 * @Enabled - enable or disable
 * @Count - the number of interrupt arouse a special event
 */ 
INT32S adv_enable_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_EnableEvent lp_enable_event;
	INT16U evt_enabled;
	INT16U evt_type;
	INT16U evt_cnt;
	INT32S ret = 0;



	if (copy_from_user(&lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		return -EFAULT;
	}

	evt_enabled = lp_enable_event.Enabled;
	evt_type = lp_enable_event.EventType;
	evt_cnt = lp_enable_event.Count;

	/* process event */
	if (evt_enabled) {
		switch (evt_type) {
		case ADS_EVT_AI_INTERRUPT: 
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      0,
						      evt_cnt);
			break;
		case ADS_EVT_AI_LOBUFREADY:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      1,
						      evt_cnt);
			break;
		case ADS_EVT_AI_HIBUFREADY:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      2,
						      evt_cnt);
			break;
		case ADS_EVT_AI_TERMINATED:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      3,
						      evt_cnt);
			break;
		case ADS_EVT_AI_OVERRUN:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      4,
						      evt_cnt);
			break;
		default:
			ret = -EINVAL;
			return ret;
		}
	} else {
		switch (evt_type) {
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
		default:
			ret = -EINVAL;
			return ret;
		}
	}

	return ret;
}


/**
 * adv_clear_flag - clear flag
 * 
 * @device
 * @
 */ 
INT32S adv_clear_flag(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U flag;
	

	if (copy_from_user(&flag, arg, sizeof(INT32U))) {
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
		privdata->fifo_overrun = 0;
		break;
	default:
		return -EINVAL;
	}
	
	return 0;
}

