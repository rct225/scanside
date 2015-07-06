/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1758U Device driver for Linux              
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
 * 	09/12/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1758U.h"
 

INT32S adv_check_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_CheckEvent check_event;
	INT16U max_chan_num;
	INT16U timeout_evt;
	INT32S event_th = 0;
	INT16U i;

	if (copy_from_user(&check_event, arg, sizeof(PT_CheckEvent))) {
		return -EFAULT;
	}

	if (!check_event.EventType) {
		event_th = adv_process_info_check_event(ptr);
		if (event_th <= 0) {
			check_event.EventType = 0;
		} else {
			switch (privdata->device_type) {
			case PCI1758UDI:
				max_chan_num = MAX_DI_CHANNEL;
				timeout_evt = max_chan_num + 1;
				break;
			case PCI1758UDO:
				max_chan_num = 0;
				timeout_evt = max_chan_num + 1;
				break;
			case PCI1758UDIO:
				max_chan_num = MAX_DIO_CHANNEL;
				timeout_evt = max_chan_num + 1;
				break;
			default:
				return -EINVAL;
			}
			

			if (event_th - 1 == max_chan_num) {
				check_event.EventType = ADS_EVT_WATCHDOG_OVERRUN;
			} else {
				for (i = 0; i < max_chan_num; i++) {
					if (i == event_th - 1) {
						check_event.EventType = ADS_EVT_DI_INTERRUPT0 + i;
					}
				}
			}
		}
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_WATCHDOG_OVERRUN:
			switch (privdata->device_type) {
			case PCI1758UDI:
				return -EINVAL;
			case PCI1758UDO:
				max_chan_num = 0;
				break;
			case PCI1758UDIO:
				max_chan_num = MAX_DIO_CHANNEL;
				break;
			default:
				return -EINVAL;
			}

			event_th = max_chan_num;
			break;

		case ADS_EVT_TIME_OUT:
			break;
		default:				
			switch (privdata->device_type) {
			case PCI1758UDI:
				max_chan_num = MAX_DI_CHANNEL;
				break;
			case PCI1758UDO:
				return -EINVAL;
			case PCI1758UDIO:
				max_chan_num = MAX_DIO_CHANNEL;
				break;
			default:
				return -EINVAL;
			}
			
			if ((check_event.EventType < ADS_EVT_DI_INTERRUPT0)
			    || (check_event.EventType >= ADS_EVT_DI_INTERRUPT0 + max_chan_num)) {
				return -EINVAL;
			}

			
			for (i = 0; i < max_chan_num; i++) {
				if (check_event.EventType == ADS_EVT_DI_INTERRUPT0 + i) {
					event_th = i;
				}
			}
			break;
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
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_EnableEvent lp_enable_event;
	INT16U max_chan_num;
	INT16U max_port_num;
	INT16U event_enabled = 0;
	INT16U timeout_evt = 129;
	INT16U i;
	INT16U chan;
	INT16U port;
	INT16U ris_reg = 0;
	INT16U fal_reg = 0;
	INT16U int_reg;
	INT16U tmp;
	INT8U wd_reg;
	

	if (copy_from_user(&lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		return -EFAULT;
	}


	privdata->evt_cnt = lp_enable_event.Count;


	if (lp_enable_event.Enabled) { /* enable event */
		switch (lp_enable_event.EventType) {
		case ADS_EVT_WATCHDOG_OVERRUN:
			switch (privdata->device_type) {
			case PCI1758UDI:
				return -EINVAL;
			case PCI1758UDO:
				max_chan_num = 0;
				wd_reg = 0x14;
				
				break;
			case PCI1758UDIO:
				max_chan_num = MAX_DIO_CHANNEL;
				wd_reg = 0x38;

				break;
			default:
				return -EINVAL;
			}
			
			/* enable event */
			adv_process_info_enable_event(ptr, max_chan_num, privdata->evt_cnt);
		
			/* enable interrupt */
			advOutpw(privdata, wd_reg, advInpw(privdata, wd_reg) | 0x0002);
			break;
		case ADS_EVT_TIME_OUT:
			break;
		default:
			switch (privdata->device_type) {
			case PCI1758UDI:
				max_chan_num = MAX_DI_CHANNEL;
				max_port_num = MAX_DI_PORT;
				break;
			case PCI1758UDO:
				return -EINVAL;
			case PCI1758UDIO:
				max_chan_num = MAX_DIO_CHANNEL;
				max_port_num = MAX_DIO_PORT;
				break;
			default:
				return -EINVAL;
			}
			
			if ((lp_enable_event.EventType < ADS_EVT_DI_INTERRUPT0)
			    || (lp_enable_event.EventType
				>= ADS_EVT_DI_INTERRUPT0 + max_chan_num)) {
				
				return -EINVAL;
			}


			/* enable event */
			for (i = 0; i < max_chan_num; i++) {
				if (lp_enable_event.EventType == ADS_EVT_DI_INTERRUPT0 + i) {
					adv_process_info_enable_event(ptr, i, privdata->evt_cnt);
				}
			}

			break;
		}

	} else {		/* disable event */
		switch (lp_enable_event.EventType) {
		case ADS_EVT_WATCHDOG_OVERRUN:
			switch (privdata->device_type) {
			case PCI1758UDI:
				return -EINVAL;
			case PCI1758UDO:
				max_chan_num = 0;
				wd_reg = 0x14;

				break;
			case PCI1758UDIO:
				max_chan_num = MAX_DIO_CHANNEL;
				wd_reg = 0x38;

				break;
			default:
				return -EINVAL;
			}

			/* disable event */
			adv_process_info_disable_event(ptr, max_chan_num);

			/* testing any event has enabled before */
			if (adv_process_info_isenable_event_all(ptr, max_chan_num)) {
				event_enabled = 1;
			}

			if (!event_enabled) {
				/* disable interrupt */
				advOutpw(privdata, wd_reg, advInpw(privdata, wd_reg) & 0xfffd);
			}
			break;
		case ADS_EVT_TIME_OUT:
			break;
		default:
			switch (privdata->device_type) {
			case PCI1758UDI:
				max_chan_num = MAX_DI_CHANNEL;
				max_port_num = MAX_DI_PORT;
				int_reg = 0x10;	/* a value which will be add to the
						 * real int_reg addr */
				break;
			case PCI1758UDO:
				return -EINVAL;
			case PCI1758UDIO:
				max_chan_num = MAX_DIO_CHANNEL;
				max_port_num = MAX_DIO_PORT;
				int_reg = 0x00;	/* a value which will be add to the
						 * real int_reg addr */
				break;
			default:
				return -EINVAL;
			}

			if ((lp_enable_event.EventType < ADS_EVT_DI_INTERRUPT0)
			    || (lp_enable_event.EventType
				>= ADS_EVT_DI_INTERRUPT0 + max_chan_num)) {
				return -EINVAL;
			}


			/* disable event */
			for (i = 0; i < max_chan_num; i++) {
				if (lp_enable_event.EventType == ADS_EVT_DI_INTERRUPT0 + i) {
					adv_process_info_disable_event(ptr, i);
					/* testing any event has enabled before */
					if (adv_process_info_isenable_event_all(ptr, i)) {
						event_enabled = 1;
					}
					break;
				}
			}
			
			if (!event_enabled) { /* disable interrupt */
				port = (lp_enable_event.EventType - ADS_EVT_DI_INTERRUPT0)
					/ max_port_num;

				/* we should r/w int reg with word length */
				chan = (lp_enable_event.EventType - ADS_EVT_DI_INTERRUPT0)
					% max_chan_num;
				chan = chan % 16;
				
				switch (port) {
				case 0:
				case 1:
					ris_reg = 0x10;
					fal_reg = 0x12;
					/* OR operation is need, we need to distinguish
					 * 1758UDI & 1758UDIO */
					int_reg |= 0x20; 
					break;
				case 2:
				case 3:
					ris_reg = 0x14;
					fal_reg = 0x16;
					/* OR operation is need, we need to distinguish
					 * 1758UDI & 1758UDIO */
					int_reg |= 0x22;
					break;
				case 4:
				case 5:
					ris_reg = 0x18;
					fal_reg = 0x1a;
					/* OR operation is need, we need to distinguish
					 * 1758UDI & 1758UDIO */
					int_reg |= 0x24;
					break;
				case 6:
				case 7:
					ris_reg = 0x1c;
					fal_reg = 0x1e;
					/* OR operation is need, we need to distinguish
					 * 1758UDI & 1758UDIO */
					int_reg |= 0x26;
					break;
				case 8:
				case 9:
					ris_reg = 0x20;
					fal_reg = 0x22;
					/* no OR operation is need, because only 1758UDI
					 * have more than eight ports  */
					int_reg = 0x38;	
					break;
				case 10:
				case 11:
					ris_reg = 0x24;
					fal_reg = 0x26;
					/* no OR operation is need, because only 1758UDI
					 * have more than eight ports  */
					int_reg = 0x3a;
					break;
				case 12:
				case 13:
					ris_reg = 0x28;
					fal_reg = 0x2a;
					/* no OR operation is need, because only 1758UDI
					 * have more than eight ports  */
					int_reg = 0x3c;
					break;
				case 14:
				case 15:
					ris_reg = 0x2c;
					fal_reg = 0x2e;
					/* no OR operation is need, because only 1758UDI
					 * have more than eight ports  */
					int_reg = 0x3e;
					break;
				default:
					return -EINVAL;
				}

				/* disable interrupt on rising edge trigger */
				tmp = ~(0x0001 << chan);
				tmp &= advInpw(privdata, ris_reg);
				advOutpw(privdata, ris_reg, tmp);

				/* disable interrupt on falling edge trigger*/
				tmp = ~(0x0001 << chan);
				tmp &= advInpw(privdata, fal_reg);
				advOutpw(privdata, fal_reg, tmp);

				/* clear interrupt */
				tmp = 0x0001 << chan;
				tmp |= advInpw(privdata, int_reg);
				advOutpw(privdata, int_reg, tmp);
			}
			break;
		}

	}
	
	return 0;
}



INT32S adv_clear_flag(adv_device *device, void *arg)
{

	return 0;
}
