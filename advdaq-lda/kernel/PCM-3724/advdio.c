/***************************************************************************
 *
 *	Copyright 2004 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCM-3724 Device driver for Linux
 *
 * File Name: 
 *	PCM3724.c
 * Abstract:
 *	This file contains routines for general driver core.
 *
 * Version history
 *      03/28/2005			Create by LI.ANG
 *
 ************************************************************************/
#define __NO_VERSION__
#include "pcm3724.h"
 

/**
 * adv_do_write - write digital output data to the specified   
 *                digital I/O port                             
 *                                                            
 * @device: Points to the device              
 * @structs: copy of argument from user
 * @data: store the data write to I/O port 
 */
static INT32S adv_do_write(adv_device *device, adv_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT8U  port_reg[MAX_DO_PORT];
	INT16U i;
	

	switch (privdata->device_type) {
	case PCM3724:
		break;
	default:
		return -ENODEV;
	}
        
	for (i = 0; i < structs->portcount; i++) {
		privdata->do_prestate[i + structs->portstart] = data[i];
	}


       return 0;	
}	


/**
 * adv_do_get - get latest output state from the specified   
 *              digital I/O port                             
 *
 * @device: Point the device object
 * @structs: copy of argument from user
 * @data: store the data read from previous output state
 */
static INT32S adv_do_get(adv_device *device, cnt_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U i;


	switch (privdata->device_type) {
	case PCM3724:
		break;
	default:
		return -ENODEV;
	}

	
	for (i = 0; i < structs->portcount; i++) {
		data[i] = privdata->do_prestate[i + structs->portstart];
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
		data = kmalloc(structs.portcount * sizeof(INT8U), GFP_KERNEL);
		if (!data) {
			return -ENOMEM;
		}
		
		if (copy_from_user(data, structs.buffer, structs.portcount * sizeof(INT8U))) {
			kfree(data);
			return -EFAULT;
		}
	}
	
	switch (structs.type) {
	case DO:
		ret = adv_do_write(device, &structs, data);
		break;
	case GETDO:
		ret = adv_do_get(device, &structs, data);
		break;
	default:
		kfree(data);
		return -EINVAL;
	}

	if (!ret) {
		if (copy_to_user(structs.buffer, data, structs.portcount * sizeof(INT8U))) {
			kfree(data);
			return -EFAULT;
		}
	}

	kfree(data);

	return ret;
}
