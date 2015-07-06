/***************************************************************************
 *
 *	Copyright 2006 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCI-1716 Device driver for Linux
 *
 * File Name: 
 *	PCI1716.c
 * Abstract:
 *	This file contains routines for general driver core.
 *
 * Version history
 *      09/21/2006			Create by zhenyu.zhang
 *
 ************************************************************************/
#define __NO_VERSION__
#include <PCI1716.h>
 
/**
 * adv_di_read - read digital input data from the specified   
 *               digital I/O port                             
 *                                                            
 * @device: Points to the device
 * @structs: copy of argument from user
 * @data: store the data read from I/O port 
 */
static INT32S adv_di_read(adv_device *device, adv_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT32U max_portstart;
	INT32U max_portnum;
	INT8U  port_reg[MAX_DI_PORT];
	INT16U i;
	

	switch (privdata->device_type) {
	case PCI1716:
		max_portstart = 1;
		max_portnum = 2;
		port_reg[0] = 0x10;
		port_reg[1] = 0x11;
		break;
	default:
		return BoardIDNotSupported;
	}
        
	if ((structs->portcount > max_portnum) || (structs->portstart > max_portstart)) {
		return InvalidPort;
	}

	for (i = 0; i < structs->portcount; i++) {
		data[i] = advInp(privdata, port_reg[i + structs->portstart]);
	}


	return SUCCESS;	
}	

/**
 * adv_do_write - write digital output data to the specified   
 *                digital I/O port                             
 *                                                            
 * @device: Points to the device              
 * @structs: copy of argument from user
 * @data: store the data read from I/O port 
 */
static INT32S adv_do_write(adv_device *device, adv_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT32U max_portstart;
	INT32U max_portnum;
	INT8U  port_reg[MAX_DO_PORT];
	INT16U i;
	

	switch (privdata->device_type) {
	case PCI1716:
		max_portstart = 1;
		max_portnum = 2;
		port_reg[0] = 0x10;
		port_reg[1] = 0x11;

		break;
	default:
		return BoardIDNotSupported;
	}
        
	if ((structs->portcount > max_portnum) || (structs->portstart > max_portstart)) {
		return InvalidPort;
	}

	for (i = 0; i < structs->portcount; i++) {
		advOutp(privdata, port_reg[i + structs->portstart], data[i]);
		privdata->do_prestate[i + structs->portstart] = data[i];
	}


     return SUCCESS;	
}	


/**
 * adv_do_get - get current do port status
 *
 * @device: Point the device object
 * @structs: copy of argument from user
 * @data: store the data read from I/O port 
 * */
static INT32S adv_do_get(adv_device *device, cnt_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U max_portstart;
	INT32U max_portnum;
	INT16U i;


	switch (privdata->device_type) {
	case PCI1716:
		max_portstart = 1;
		max_portnum = 2;
		break;
	default:
		return BoardIDNotSupported;
	}

        
	if ((structs->portcount > max_portnum) || (structs->portstart > max_portstart)) {
		return InvalidPort;
	}
	
	for (i = 0; i < structs->portcount; i++) {
		data[i] = privdata->do_prestate[i + structs->portstart];
	}

	return SUCCESS;	
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
		return MemoryCopyFailed;
	}
     
	if (structs.buffer != NULL) {
		data = kmalloc(structs.portcount * sizeof(INT8U), GFP_KERNEL);
		if (!data) {
			return MemoryAllocateFailed;
		}
		
		if (copy_from_user(data, structs.buffer, structs.portcount * sizeof(INT8U))) {
			kfree(data);
			return MemoryCopyFailed;
		}
	}
	
	switch (structs.type) {
	case DI:
		ret = adv_di_read(device, &structs, data);
		break;
	case DO:
		ret = adv_do_write(device, &structs, data);
		break;
	case GETDO:
		ret = adv_do_get(device, &structs, data);
		break;
	default:
		break;
	}

	if (!ret) {
		if (copy_to_user(structs.buffer, data, structs.portcount * sizeof(INT8U))) {
			kfree(data);
			return MemoryCopyFailed;
		}
	}

	kfree(data);

	return ret;
}

