/***************************************************************************
 *
 *	Copyright 2004 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech USB-4750 Device driver for Linux
 *
 * File Name: 
 *	advdio.c
 * Abstract:
 *	This file contains routines for general driver core.
 *
 * Version history
 *      03/28/2005			Create by LI.ANG
 *
 ************************************************************************/
#define __NO_VERSION__
#include "USB4750.h"
 

/**
 * adv_dio_enable_event - set port according with event type 
 *
 * @device: Point the device object
 * @structs: copy of argument from user
 * @data: store the data read from previous output state
 *
 * user wanna get real-time DI port state when special event occured, we set
 * DI port start, port count, and port register address in this function.
 */
static INT32S adv_dio_enable_event(adv_device *device, cnt_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U event_type;
	INT16U event_idx = 0;
	INT32U max_portstart;
	INT32U max_portnum;


	switch (privdata->device_type) {
	case USB4750:
		max_portstart = 1;
		max_portnum = 2;
		break;
	default:
		return -EINVAL;
	}

        
	if ((structs->portcount == 0)
	    || (structs->portstart > max_portstart)
	    || (structs->portstart + structs->portcount > max_portnum)) {
		return -EINVAL;
	}

	/* event type */
	event_type = structs->data;

	switch (event_type) {
	case ADS_EVT_DI_INTERRUPT0:
		event_idx = 0;
		break;
	case ADS_EVT_DI_INTERRUPT8:
		event_idx = 1;
		break;		
	default:
		return -EINVAL;
	}
	
	privdata->event_port[event_idx].start = structs->portstart;
	privdata->event_port[event_idx].count = structs->portcount;

	
	return 0;	
}	

/**
 * adv_dio_enable_event - clear port settings according with event type 
 *
 * @device: Point the device object
 * @structs: copy of argument from user
 * @data: store the data read from previous output state
 *
 */
static INT32S adv_dio_disable_event(adv_device *device, cnt_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U event_type;
	INT16U event_idx = 0;
	INT32U max_portstart;
	INT32U max_portnum;


	switch (privdata->device_type) {
	case USB4750:
		max_portstart = 1;
		max_portnum = 2;
		break;
	default:
		return -EINVAL;
	}

	/* event type */
	event_type = structs->data;

	switch (event_type) {
	case ADS_EVT_DI_INTERRUPT0:
		event_idx = 0;
		break;
	case ADS_EVT_DI_INTERRUPT8:
		event_idx = 1;
		break;		
	default:
		return -EINVAL;
	}

	privdata->event_port[event_idx].start = 0;
	privdata->event_port[event_idx].count = 0;
	
	return 0;	
}	

/**
 * adv_dio_get_di_state - get real-time port state according with event type
 *
 * @device: Point the device object
 * @structs: copy of argument from user
 * @data: store the data read from previous output state
 *
 * user get real-time DI port state through this method.
 */
static INT32S adv_dio_get_di_state(adv_device *device, cnt_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U event_type;
	INT16U event_idx = 0;
	INT32U port_start;
	INT32U port_count;
	INT16U i;


        
	/* event type */
	event_type = structs->data;

	switch (event_type) {
	case ADS_EVT_DI_INTERRUPT0:
		event_idx = 0;
		break;
	case ADS_EVT_DI_INTERRUPT8:
		event_idx = 1;
		break;		
	default:
		return -EINVAL;
	}

	port_start = privdata->event_port[event_idx].start;
	port_count = privdata->event_port[event_idx].count;
	
	/* for real-time di state */
	for (i = 0; i < port_count; i++) {
		data[i] = privdata->di_evt_cfg[event_idx].evt_data >> (i * 8);
	}
	
	return 0;	
}	

/**
 * adv_dio_ioctl - dio dispatch function 
 *
 * device: Point the the device object
 * arg: argument from user
 */
INT32S adv_dio_ioctl(adv_device *device, void *arg)
{
	adv_struct structs;
	INT8U *data = NULL;
	INT32S ret = 0;	


	if (copy_from_user(&structs, arg, sizeof(adv_struct))) {
		return -EFAULT;
	}

	if (structs.buffer != NULL) {
		data = kmalloc(structs.n * sizeof(INT8U), GFP_KERNEL);
		if (!data) {
			return -ENOMEM;
		}

		if (copy_from_user(data, structs.buffer, structs.n * sizeof(INT8U))) {
			kfree(data);
			return -EFAULT;
		}
	}

	switch (structs.type) {
	case DIO_ENABLE_EVENT:
		ret = adv_dio_enable_event(device, &structs, data);
		break;
	case DIO_DISABLE_EVENT:
		ret = adv_dio_disable_event(device, &structs, data);
		break;
	case DIO_EVENT_GET_DI_STATE:
		ret = adv_dio_get_di_state(device, &structs, data);
		if (!ret) {
			if (copy_to_user(structs.buffer, data,
					 structs.n * sizeof(INT8U))) {
				kfree(data);
				return -EFAULT;
			}
		}

		break;
	default:
		break;
	}
	
	kfree(data);
	
	return ret;
}

