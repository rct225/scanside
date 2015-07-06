/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1737 Device driver for Linux              
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
 * 	10/25/2006			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "PCI1737.h"
 

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
				check_event.EventType = ADS_EVT_DI_INTERRUPT16;
				break;
			case 1:
				check_event.EventType = ADS_EVT_DI_INTERRUPT40;
				break;
			case 2:
				check_event.EventType = ADS_EVT_INTERRUPT;
				break;
			default:
				return -EINVAL;
	
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
		case ADS_EVT_INTERRUPT:
			event_th = 2;
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
	PT_EnableEvent *lp_enable_event;
	INT32S ret = -EINVAL;

	lp_enable_event = kmalloc(sizeof(PT_EnableEvent), GFP_KERNEL);
	if (!lp_enable_event) {
		return -ENOMEM;
	}
     
	if (copy_from_user(lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		return -EFAULT;
	}

	privdata->evt_cnt = lp_enable_event->Count;

	if (lp_enable_event->Enabled) {
/* 		if(privdata->usIntSrcConfig[0] == TriggerSource_PC0)			{ */
/* 			privdata->usInterruptSrc &= 0xfc; */
/* 			privdata->usInterruptSrc |= 0x01; */
/* 		}else if (privdata->usIntSrcConfig[0] == TriggerSource_PC0_PC4){ */
/* 			privdata->usInterruptSrc &= 0xfc; */
/* 			privdata->usInterruptSrc |= 0x02; */
/* 		}else if(privdata->usIntSrcConfig[0] == TriggerSource_TMR1){ */
/* 			privdata->usInterruptSrc &= 0xfc; */
/* 			privdata->usInterruptSrc |= 0x03; */
/* 		}else{ */
/* 			privdata->usInterruptSrc &= 0xfc; */
/* 		} */
		  
/* 		if(privdata->usIntSrcConfig[1] == TriggerSource_PC10)	{ */
/* 			privdata->usInterruptSrc &= 0xcf; */
/* 			privdata->usInterruptSrc |= 0x10; */
/* 		}else if (privdata->usIntSrcConfig[1] == TriggerSource_PC10_PC14){ */
/* 			privdata->usInterruptSrc &= 0xcf; */
/* 			privdata->usInterruptSrc |= 0x20; */
/* 		}else if(privdata->usIntSrcConfig[1] == TriggerSource_CNT2){ */
/* 			privdata->usInterruptSrc &= 0xcf; */
/* 			privdata->usInterruptSrc |= 0x30; */
/* 		}else{ */
/* 			privdata->usInterruptSrc &= 0xcf; */
/* 		} */
		  
		/* modify for new event mechanism */
		switch(lp_enable_event->EventType) {
		case ADS_EVT_DI_INTERRUPT16:
			if(privdata->usIntSrcConfig[0] == TriggerSource_PC0 || privdata->usIntSrcConfig[0] == TriggerSource_PC0_PC4) {
				privdata->usInterruptSrc &= 0xfc;
				privdata->usInterruptSrc |= privdata->usIntSrcConfig[0] & 0x03;
				adv_process_info_enable_event(&privdata->ptr_process_info, 0, lp_enable_event->Count);
				ret = 0;
			}
		
			break;
		case ADS_EVT_DI_INTERRUPT40:
			if (privdata->device_type != PCI1739) {
				break;
			}
			if ((privdata->usIntSrcConfig[1] == TriggerSource_PC10) || (privdata->usIntSrcConfig[1] == TriggerSource_PC10_PC14)) {
				privdata->usInterruptSrc &= 0xcf;
				privdata->usInterruptSrc |= (privdata->usIntSrcConfig[1] << 4) & 0x30;
				adv_process_info_enable_event(&privdata->ptr_process_info, 1, lp_enable_event->Count);
				ret = 0;
			}
			
			break;
		case ADS_EVT_INTERRUPT:
			if ((privdata->device_type == PCI1737) && privdata->usIntSrcConfig[0] == TriggerSource_Disable) {
				break;
			}
			
			if ((privdata->device_type == PCI1739) && (privdata->usIntSrcConfig[0] == TriggerSource_Disable) && (privdata->usIntSrcConfig[1] == TriggerSource_Disable)) {
				break;
			}

			privdata->usInterruptSrc &= 0xfc;
			privdata->usInterruptSrc |= privdata->usIntSrcConfig[0] & 0x03;
			privdata->usInterruptSrc &= 0xcf;
			privdata->usInterruptSrc |= (privdata->usIntSrcConfig[1] << 4) & 0x30;

		  	adv_process_info_enable_event(&privdata->ptr_process_info, 2, lp_enable_event->Count);
			ret = 0;
			break;
		default:
			return -EINVAL;
		}
		advOutp(privdata, 0x08, privdata->usInterruptSrc);	  
	} else {
		switch (lp_enable_event->EventType) {
		case ADS_EVT_DI_INTERRUPT16:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			if (((privdata->usIntSrcConfig[0] == TriggerSource_PC0) || (privdata->usIntSrcConfig[0] == TriggerSource_PC0_PC4)) 
			    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 0))){
				privdata->usInterruptSrc &= 0xfc;
			}
			ret = 0;
			break;
		case ADS_EVT_DI_INTERRUPT40:
			if (privdata->device_type != PCI1739) {
				break;
			}
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			if (((privdata->usIntSrcConfig[1] == TriggerSource_PC0) || (privdata->usIntSrcConfig[1] == TriggerSource_PC0_PC4)) 
			    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 1))){
				privdata->usInterruptSrc &= 0xcf;
			}
			ret = 0;
			break;
		case ADS_EVT_INTERRUPT:
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			if (privdata->device_type == PCI1737) {
				privdata->usInterruptSrc &= 0xfc;
			}
			if (privdata->device_type == PCI1739) {
				if (privdata->usIntSrcConfig[0] == TriggerSource_Disable) {
					privdata->usInterruptSrc &= 0xfc;
				}
				
				if (privdata->usIntSrcConfig[1] == TriggerSource_Disable) {
					privdata->usInterruptSrc &= 0xcf;
				}

			}
			ret = 0;
			break;
		default:
			return -EINVAL;
		}

		
		advOutp(privdata, 0x08, privdata->usInterruptSrc);	  
	}
     
	kfree(lp_enable_event);	

	return ret;
}



INT32S adv_clear_flag(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	privdata->overrun_flag = 0;

	return 0;
}
