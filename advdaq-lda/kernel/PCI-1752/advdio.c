/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 
	
			Advantech Co., Ltd.

		Advantech PCI-1752 Device driver for Linux

	File Name: 
		advdio.c
	Abstract:
		This file contains routines for general driver core.

	Version history
	07/01/2006			Create by Zhiyong.Xie

************************************************************************/
#define __NO_VERSION__
#include "PCI1752.h"

/**
 * adv_dio_enable_event - enable a special event and which ports to scan
 *                        when the event occured
 * @device - point to a special device
 * @event - which event to enable
 * @scan_start - start ports to scan
 * @scan_count - how many port to scan started from scan_start when
 *               the event occured
 */

INT32S adv_dio_enable_event(adv_device *device, INT32S event, INT32S scan_start, INT32S scan_count)
{
	private_data *privdata = (private_data *)device->private_data;
	INT32S index;
	INT32S ret;
	
	/* set the interrupt control register */
	if ( (scan_start > (privdata->di_port_num -1)) 
	     || ((scan_start + scan_count) > (privdata->di_port_num))
	     || (scan_count == 0))
		return -EFAULT;
	
	switch (event) {
	case ADS_EVT_DI_INTERRUPT0:
		index = 0;
		break;
	case ADS_EVT_DI_INTERRUPT16:
		index = 1;
		break;
	case ADS_EVT_DI_INTERRUPT32:
		index = 2;
		break;
	case ADS_EVT_DI_INTERRUPT48:
		index = 3;
		break;
	default:
		return -EFAULT;
	}

	privdata->event_di_port_start[index] = scan_start;
	privdata->event_di_port_num[index] = scan_count;

	ret = adv_enable_event(device, event, 1, 1);
	return ret;
}

/**
 * adv_dio_disable_event - disable a special event
 * @device - point to a special device
 * @event - which event to disable
 */
INT32S adv_dio_disable_event(adv_device *device, INT32S event)
{
	return adv_enable_event(device, event, 0, 0);
}

/**
 *  adv_dio_get_lastest_event_state - get digital input status when
 *                                    latest event occured
 *  @device - point to a special device
 *  @event - special event type
 *  @pbuf - save the digital input status when latest event occured
 *  @len - size of buf in byte
 */
INT32S adv_dio_get_latest_event_state(adv_device *device, INT32S event, BYTE *pbuf, INT32S len)
{
	private_data *privdata = (private_data *)device->private_data;
	INT32S index;
	INT32S start;
	INT32S count;
	INT8U  *p;
	
	
	switch(event)
	{
	case ADS_EVT_DI_INTERRUPT0:
		index = 0;
		break;
	case ADS_EVT_DI_INTERRUPT16:
		index = 1;
		break;
	case ADS_EVT_DI_INTERRUPT32:
		index = 2;
		break;
	case ADS_EVT_DI_INTERRUPT48:
		index = 3;
		break;
	default:
		return -EFAULT;
	}
	
	start = privdata->event_di_port_start[index];
	count = privdata->event_di_port_num[index];
	
	if (len < count)
		return -EFAULT;
	
	p = &privdata->event_di_status[index][0];
	
	if (copy_to_user(pbuf, (p + start), count))
		return -EFAULT;
		
	return 0;
}

/**
 * adv_dio_ioctl - distribute the operation according the command
 * @device - point to a special devcie
 * @arg - the data pass from user
 */
INT32S adv_dio_ioctl(adv_device *device, void *arg)
{
	INT32S ret = 0;
	adv_struct structs;
	BYTE *data = NULL;
	
	if(copy_from_user(&structs, arg,sizeof(adv_struct))){
		printk("error occured when copy_from_user()..1\n");
		return -EFAULT;
	}
    
	data = kmalloc(structs.portcount*sizeof(BYTE),GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		return ret;
	}
	
	if ((structs.buffer) && copy_from_user(data,structs.buffer, structs.portcount*sizeof(BYTE))) {
		printk("error occured when copy_from_user()..1\n");
		kfree(data);
		return -EFAULT;
	}

	switch (structs.type) {
	case DIO_ENABLE_EVENT:
		ret = adv_dio_enable_event(device, structs.data, structs.portstart, structs.portcount);
		if(ret){
			kfree(data);
			return ret;
		}
		break;

	case DIO_DISABLE_EVENT:
		ret = adv_dio_disable_event(device, structs.data);
		if(ret){
			kfree(data);
			return ret;
		}
		break;
	     
	case DIO_EVENT_GET_DI_STATE:
		ret = adv_dio_get_latest_event_state(device, structs.data, structs.buffer, structs.portcount);
		if(ret){
			kfree(data);
			return ret;
		}
		break;
	}

/* 	if((structs.buffer) && copy_to_user(structs.buffer,data,structs.portcount*sizeof(BYTE))){ */
/* 		kfree(data); */
/* 		return -EFAULT; */
/* 	} */
	
	kfree(data);
     
	return ret;

}
