/***************************************************************************
 *
 *	Copyright 2006 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCI-1758U Device driver for Linux
 *
 * File Name: 
 *	PCI1758U.c
 * Abstract:
 *	This file contains routines for DI/O.
 *
 * Version history
 *      09/12/2006			Create by LI.ANG
 *
 ************************************************************************/
#define __NO_VERSION__
#include "PCI1758U.h"
 

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
	INT16U max_port_num;
	INT8U port_reg[MAX_DO_PORT];
	INT16U i;



	switch (privdata->device_type) {
	case PCI1758UDO:
		max_port_num = MAX_DO_PORT;

		for (i = 0; i < max_port_num; i++) {
			port_reg[i] = 0x00 + i;
		}
		
		break;
	case PCI1758UDIO:
		max_port_num = MAX_DIO_PORT;

		for (i = 0; i < max_port_num; i++) {
			port_reg[i] = 0x08 + i;
		}

		break;
	case PCI1758UDI:
		return -EINVAL;
	default:
		return -ENODEV;
	}

        
	for (i = 0; i < structs->portcount; i++) {
		advOutp(privdata, port_reg[i + structs->portstart], data[i]);
	}


	return 0;
}	


/**
 * adv_do_get - read back latest output data from the specified   
 *              digital output port                             
 *                                                            
 * @device: Points to the device              
 * @structs: copy of argument from user
 * @data: store the data read from previous output data
 */
static INT32S adv_do_get(adv_device *device, cnt_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U max_port_num;
	INT8U port_reg[MAX_DO_PORT];
	INT16U i;


	switch (privdata->device_type) {
	case PCI1758UDO:
		max_port_num = MAX_DO_PORT;

		for (i = 0; i < max_port_num; i++) {
			port_reg[i] = 0x00 + i;
		}
		
		break;
	case PCI1758UDIO:
		max_port_num = MAX_DIO_PORT;

		for (i = 0; i < max_port_num; i++) {
			port_reg[i] = 0x08 + i;
		}

		break;
	case PCI1758UDI:
		return -EINVAL;
	default:
		return -ENODEV;
	}

	/* waitting for CPLD read do watchdog value from
	 * EEPROM and write to the conresponding port */
	mdelay(500);
	
	for (i = 0; i < structs->portcount; i++) {
		data[i] = advInp(privdata, port_reg[i + structs->portstart]);
	}


	return 0;
}	


/**
 * adv_dio_ioctl - dio dispatch function
 *                                                            
 * @device: Points to the device              
 * @arg: argument from user
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
		break;
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
