/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1761 Device driver for Linux              
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
 * 	07/20/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
#include "PCI1761.h"
 


INT32S adv_enable_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_EnableEvent lp_enable_event;
	INT16U event_enabled = 0;
	INT8U mask = 0;
	INT8U int_reg = 0;
	INT16U i;

     
	if (copy_from_user(&lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		return -EFAULT;
	}

/* 	/\* testing any event has enabled before *\/ */
/* 	for (i = 0; i < MAX_EVENT_NUM; i++) { */
/* 		if (adv_process_info_isenable_event_all(ptr, i)) { */
/* 			event_enabled = 1; */
/* 			break; */
/* 		} */
/* 	} */

	privdata->evt_cnt = lp_enable_event.Count;

	if (lp_enable_event.Enabled) {
		switch (lp_enable_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0: 
		{
			switch (privdata->device_type) {
			case PCI1761:
			case MIC3761:
			case PCI1763:
				mask = 0x01;
				break;
			case PCI1762:
				mask = 0x02;
				mask |= (privdata->int_trig_mode << 2); /* set ID0RF bit */
				int_reg = 0x06;
				break;
			default:
				return -ENODEV;
			}
		}
		adv_process_info_enable_event(&privdata->ptr_process_info,
					      0,
					      privdata->evt_cnt);
		break;
		case ADS_EVT_DI_INTERRUPT1:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			
			mask = 0x02;
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      1,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_DI_INTERRUPT2:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x04;
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      2,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_DI_INTERRUPT3:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x08;
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      3,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_DI_INTERRUPT4:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x10;
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      4,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_DI_INTERRUPT5:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x20;
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      5,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_DI_INTERRUPT6:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x40;
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      6,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_DI_INTERRUPT7:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x80;
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      7,
						      privdata->evt_cnt);
			break;
		case ADS_EVT_DI_INTERRUPT8: /* for pci1762 */
			if (privdata->device_type != PCI1762) {
				return -EINVAL;
			}
			mask = 0x02;
			mask |= ((privdata->int_trig_mode >> 8) << 2); /* set ID8RF bit */
			int_reg = 0x07;
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      8,
						      privdata->evt_cnt); 
			break;
		case ADS_EVT_TIME_OUT:
			adv_process_info_enable_event(&privdata->ptr_process_info,
						      9, privdata->evt_cnt);
			return 0;
		default:
			return -EINVAL;
		}

		switch (privdata->device_type) {
		case PCI1761:	/* note: the hardware manual is not correct, Appendix C
				 *       except to C.2 */
		case PCI1763:
		case MIC3761:
			mask |= advInp(privdata, 0x03);
			advOutp(privdata, 0x03, mask); /* enable specified interrupt */
			advOutp(privdata, 0x04, privdata->int_trig_mode);

			break;
		case PCI1762:
			mask |= advInp(privdata, int_reg);
			advOutp(privdata, int_reg, mask);
			/* printk("reg(0x%.2x): 0x%.2x\n", int_reg, advInp(privdata, int_reg)); */
			break;
		default:
			return -ENODEV;
		}
/* 		advOutp(privdata, 0x3, 0x00); */
/* 		advOutp(privdata, 0x4, 0x00); */
/* 		printk("reg[0x3]: 0x%x\n", advInp(privdata, 0x3)); */
/* 		printk("reg[0x4]: 0x%x\n", advInp(privdata, 0x4)); */
		
	} else {
		switch (lp_enable_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0:
		{
			switch (privdata->device_type) {
			case PCI1761:
			case MIC3761:
			case PCI1763:
				mask = 0x01;
				break;
			case PCI1762:
				mask = 0x01; /* set ID0CLR bit */
				int_reg = 0x06;
				break;
			default:
				return -ENODEV;
			}
		}
		adv_process_info_disable_event(&privdata->ptr_process_info, 0);
		break;
		case ADS_EVT_DI_INTERRUPT1:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x02;
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			break;
		case ADS_EVT_DI_INTERRUPT2:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x04;
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			break;
		case ADS_EVT_DI_INTERRUPT3:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x08;
			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			break;
		case ADS_EVT_DI_INTERRUPT4:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x10;
			adv_process_info_disable_event(&privdata->ptr_process_info, 4);
			break;
		case ADS_EVT_DI_INTERRUPT5:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x20;
			adv_process_info_disable_event(&privdata->ptr_process_info, 5);
			break;
		case ADS_EVT_DI_INTERRUPT6:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x40;
			adv_process_info_disable_event(&privdata->ptr_process_info, 6);
			break;
		case ADS_EVT_DI_INTERRUPT7:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			mask = 0x80;
			adv_process_info_disable_event(&privdata->ptr_process_info, 7);
			break;
		case ADS_EVT_DI_INTERRUPT8: /* for pci1762 */
			if (privdata->device_type != PCI1762) {
				return -EINVAL;
			}
			mask = 0x01; /* set ID8CLR bit */
			int_reg = 0x07;
			adv_process_info_disable_event(&privdata->ptr_process_info, 8);
			break;
		case ADS_EVT_TIME_OUT:
			adv_process_info_disable_event(&privdata->ptr_process_info, 9);
			return 0;
		default:
			return -EINVAL;
		}

		/* testing any event has enabled before */
		for (i = 0; i < MAX_EVENT_NUM; i++) {
			if (adv_process_info_isenable_event_all(ptr, i)) {
				event_enabled = 1;
				break;
			}
		}

		if (!event_enabled) {
			switch (privdata->device_type) {
			case PCI1761: /* note: the hardware manual is not correct, Appendix C
				       *       except to C.2 */
			case MIC3761:
			case PCI1763:
				mask = ~mask & advInp(privdata, 0x03);
				advOutp(privdata, 0x03, mask); /* disable specified interrupt */
				advOutp(privdata, 0x05, ~mask); /* clear specified interrupt */
				break;
			case PCI1762:
				advOutp(privdata, int_reg, mask);
				break;
			default:
				return -ENODEV;
			}
		}
	}
     
	return 0;
}

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
		/* printk("event_th: %d\n", event_th); */
		
		if (event_th <= 0) {
			/* check_event.EventType = 0; */
			check_event.EventType = ADS_EVT_TIME_OUT;
		} else {
			switch (event_th - 1) {
			case 0:
				check_event.EventType = ADS_EVT_DI_INTERRUPT0;
				break;
			case 1:
				check_event.EventType = ADS_EVT_DI_INTERRUPT1;
				break;
			case 2:
				check_event.EventType = ADS_EVT_DI_INTERRUPT2;
				break;
			case 3:
				check_event.EventType = ADS_EVT_DI_INTERRUPT3;
				break;
			case 4:
				check_event.EventType = ADS_EVT_DI_INTERRUPT4;
				break;
			case 5:
				check_event.EventType = ADS_EVT_DI_INTERRUPT5;
				break;
			case 6:
				check_event.EventType = ADS_EVT_DI_INTERRUPT6;
				break;
			case 7:
				check_event.EventType = ADS_EVT_DI_INTERRUPT7;
				break;
			case 8:
				check_event.EventType = ADS_EVT_DI_INTERRUPT8; /* for pci1762 */
				break;
/* 			case 9: */
/* 				check_event.EventType = ADS_EVT_TIME_OUT; */
/* 				break; */
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			event_th = 0;
			break;
		case ADS_EVT_DI_INTERRUPT1:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			event_th = 1;
			break;
		case ADS_EVT_DI_INTERRUPT2:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			event_th = 2;
			break;
		case ADS_EVT_DI_INTERRUPT3:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			event_th = 3;
			break;
		case ADS_EVT_DI_INTERRUPT4:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			event_th = 4;
			break;
		case ADS_EVT_DI_INTERRUPT5:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			event_th = 5;
			break;
		case ADS_EVT_DI_INTERRUPT6:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			event_th = 6;
			break;
		case ADS_EVT_DI_INTERRUPT7:
			if (privdata->device_type == PCI1762) {
				return -EINVAL;
			}
			event_th = 7;
			break;
		case ADS_EVT_DI_INTERRUPT8: /* for pci1762 */
			if (privdata->device_type != PCI1762) {
				return -EINVAL;
			}
			event_th = 8;
			break;
		case ADS_EVT_TIME_OUT:
			event_th = 9;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 9, 1);
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


INT32S adv_clear_flag(adv_device *device, void *arg)
{

	return 0;
}
