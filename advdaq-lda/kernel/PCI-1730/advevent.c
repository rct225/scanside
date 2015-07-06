/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1730 Device driver for Linux

File Name:
	advevent.c
Abstract:
	Initialize the PCI-1730 driver module and device. 
Version history
	05/25/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
#include <linux/module.h>
#include "PCI1730.h"

int adv_enable_event(adv_device *device,ULONG EventType,USHORT Enabled,USHORT Count)
{
	BYTE ctrlcode = 0;
	private_data *privdata = (private_data*)(device->private_data);

	if(Enabled){
		if(Count <= 0)
			return -EFAULT;
		
		/* set the interrupt control register */
		switch(EventType){
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				0, Count);
			break;
		case ADS_EVT_DI_INTERRUPT1:
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				1, Count);
			break;
		case ADS_EVT_DI_INTERRUPT16:
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				2, Count);
			break;
		case ADS_EVT_DI_INTERRUPT17:
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				3, Count);
			break;
		default:
			break;
		}

		adv_get_int_control_code(privdata, &ctrlcode);
		advOutp(privdata, 0x08, ctrlcode);

	}else{
		switch(EventType)
		{
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			break;
		case ADS_EVT_DI_INTERRUPT1:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			break;
		case ADS_EVT_DI_INTERRUPT16:
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			break;
		case ADS_EVT_DI_INTERRUPT17:
			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			break;
		default:
			break;
		}

		adv_get_int_control_code(privdata, &ctrlcode);
		advOutp(privdata, 0x08, ctrlcode);
	}
	return 0;
}

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
				*EventType = ADS_EVT_DI_INTERRUPT1;
				break;
			case 2:
				*EventType = ADS_EVT_DI_INTERRUPT16;
				break;
			case 3:
				*EventType = ADS_EVT_DI_INTERRUPT17;
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
		case ADS_EVT_DI_INTERRUPT1:
			event_th = 1;
			break;
		case ADS_EVT_DI_INTERRUPT16:
			//  case ADS_EVT_INTERRUPT_DI0:
			event_th = 2;
			break;
		case ADS_EVT_DI_INTERRUPT17:
			// case ADS_EVT_INTERRUPT_DI1:
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

	  
