/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCM-3718 Device driver for Linux

File Name:
	advevent.c
Abstract:
	Initialize the PCM-3718 driver module and device. 
Version history
	05/25/2006

************************************************************************/	

/* base module includes */
//#include <linux/config.h>
#include <linux/module.h>
#include "PCM3718.h"

INT32S adv_enable_event(adv_device *device,INT32U EventType,INT16U Enabled,INT32U Count)
{
	private_data *privdata = (private_data*)(device->private_data);

	if (Enabled) {
		/* set the interrupt control register */
		switch(EventType){
		case ADS_EVT_AI_LOBUFREADY:
			adv_process_info_enable_event(&privdata->ptr_process_info, 0, Count);
			break;
		case ADS_EVT_AI_HIBUFREADY:
			adv_process_info_enable_event(&privdata->ptr_process_info, 1, Count);
			break;
		case ADS_EVT_AI_TERMINATED:
			adv_process_info_enable_event(&privdata->ptr_process_info, 2, Count);
			break;
		case ADS_EVT_AI_OVERRUN:
			adv_process_info_enable_event(&privdata->ptr_process_info, 3, Count);
			break;
		case ADS_EVT_AI_INTERRUPT:
			adv_process_info_enable_event(&privdata->ptr_process_info, 4, Count);
			break;
		default:
			break;
		}
	} else {
		switch(EventType){
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
		case ADS_EVT_AI_INTERRUPT:
			adv_process_info_disable_event(&privdata->ptr_process_info, 4);
			break;
		default:
			break;
		}

	}
	return 0;
}

INT32U adv_check_event(adv_device *device, INT32U *EventType, 
		       INT32U Milliseconds)
{
	private_data *privdata = (private_data*)(device->private_data);
	adv_process_info_header *p = &privdata->ptr_process_info;
	INT32S event_th = 0;
		
	if (!(*EventType)) {
		event_th = adv_process_info_check_event(p);
				
		if (event_th <= 0) {
			*EventType = 0;
		}else{
			switch (event_th -1) {
			case 0:
				*EventType = ADS_EVT_AI_LOBUFREADY;
				break;
			case 1:
				*EventType = ADS_EVT_AI_HIBUFREADY;
				break;
			case 2:
				*EventType = ADS_EVT_AI_TERMINATED;
				break;
			case 3:
				*EventType = ADS_EVT_AI_OVERRUN;
				break;
			case 4:
				*EventType = ADS_EVT_AI_INTERRUPT;
				break;
			}
			
		}
	  
	} else {
		switch (*EventType) {
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
		case ADS_EVT_AI_INTERRUPT:
			event_th = 4;
			break;

		default:
			event_th = -1;
			break;
		}
		if((event_th >= 0) && (event_th <=3))
			*EventType = adv_process_info_check_special_event(p, event_th);
		else 
			*EventType = 0;
	}
     
	return 0;
     
}

	  
