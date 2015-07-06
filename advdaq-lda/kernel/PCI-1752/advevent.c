/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1752 Device driver for Linux

File Name:
	advevent.c
Abstract:
	Initialize the PCI-1752 driver module and device. 
Version history
	05/25/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
#include <linux/module.h>
#include "PCI1752.h"
/**
 * adv_enable_event - enable or disable a special event with count
 * @device - pointer to a special device
 * @EventType - special event type
 * @Enabled - enable or disable
 * @Count - the number of interrupt arouse a special event
 */
INT32S adv_enable_event(adv_device *device,ULONG EventType,USHORT Enabled,USHORT Count)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT16U ctlcode = 0;
	
	if(Enabled){
		if(Count <= 0)
			return -EFAULT;
		
                /* set the interrupt control register */
		switch (EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				0, Count);
			ctlcode = advInpw(privdata, 0x08)&0x000f;
			
			if(privdata->event_trigger_mode & 0x01){
				ctlcode |= 0x04;
			}else{
				ctlcode &= 0x0b;
			}
						
			advOutpw(privdata, 0x08, ctlcode|0x02);
			break;
		case ADS_EVT_DI_INTERRUPT16:
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				1, Count);
			ctlcode = advInpw(privdata, 0x0a)&0x000f;
			if(privdata->event_trigger_mode & 0x02){
				ctlcode |= 0x04;
			}else{
				ctlcode &= 0x0b;
			}
			advOutpw(privdata, 0x0a, ctlcode|0x02);
			break;
		case ADS_EVT_DI_INTERRUPT32:
			if (privdata->device_type == PCI1756) {
				return -EINVAL;
			}
			
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				2, Count);
			ctlcode = advInpw(privdata, 0x0c)&0x000f;
			if(privdata->event_trigger_mode & 0x04){
				ctlcode |= 0x04;
			}else{
				ctlcode &= 0x0b;
			}
			advOutpw(privdata, 0x0c, ctlcode|0x02);

			break;
		case ADS_EVT_DI_INTERRUPT48:
			if (privdata->device_type == PCI1756) {
				return -EINVAL;
			}
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				3, Count);
			ctlcode = advInpw(privdata, 0x0e)&0x000f;
			if(privdata->event_trigger_mode & 0x08){
				ctlcode |= 0x04;
			}else{
				ctlcode &= 0x0b;
			}
			advOutpw(privdata, 0x0e, ctlcode|0x02);

			break;
		default:
			return -EINVAL;
		}
	}else{
		switch (EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 0)) {
				ctlcode = advInpw(privdata, 0x08)&0x000f;
				advOutpw(privdata, 0x08, ctlcode & 0x0d);
			}
			    
			break;
		case ADS_EVT_DI_INTERRUPT16:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 1)) {
				ctlcode = advInpw(privdata, 0x0a)&0x000f;
				advOutpw(privdata, 0x0a, ctlcode & 0x0d);
			}
				
			break;
		case ADS_EVT_DI_INTERRUPT32:
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 2)) {
				ctlcode = advInpw(privdata, 0x0c)&0x000f;
				advOutpw(privdata, 0x0c, ctlcode & 0x0d);
			}
			
			break;
		case ADS_EVT_DI_INTERRUPT48:
			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 3)) {
				ctlcode = advInpw(privdata, 0x0e)&0x000f;
				advOutpw(privdata, 0x0e, ctlcode & 0x0d);
			}
			
			break;
		default:
			return -EINVAL;
		}

	}
	return 0;
}
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
INT32U adv_check_event(adv_device *device, INT32U *EventType, 
		       INT32U Milliseconds)
{
	private_data *privdata = (private_data*)(device->private_data);
	adv_process_info_header *p = &privdata->ptr_process_info;
	INT32S event_th = 0;
		
	if(!(*EventType))
	{
		event_th = adv_process_info_check_event(p);
		
		if(event_th <= 0){
			*EventType = 0;
		}else{
			switch(event_th -1)
			{
			case 0:
				*EventType = ADS_EVT_DI_INTERRUPT0;
				break;
			case 1:
				*EventType = ADS_EVT_DI_INTERRUPT16;
				break;
			case 2:
				*EventType = ADS_EVT_DI_INTERRUPT32;
				break;
			case 3:
				*EventType = ADS_EVT_DI_INTERRUPT48;
				break;
			}
			
		}
	  
	}
	else{
		switch(*EventType)
		{
		case ADS_EVT_DI_INTERRUPT0:
			event_th = 0;
			break;
		case ADS_EVT_DI_INTERRUPT16:
			event_th = 1;
			break;
		case ADS_EVT_DI_INTERRUPT32:
			event_th = 2;
			break;
		case ADS_EVT_DI_INTERRUPT48:
			event_th = 3;
			break;
		default:
			event_th = -1;
			break;
		}
		if((event_th >= 0) && (event_th <=3)){
			*EventType = adv_process_info_check_special_event(p, event_th);
		}else{
			
			*EventType = 0;
		}
		
	}
     
	return 0;
     
}

	  
