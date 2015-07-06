/***************************************************************************
 *
 *	Copyright 2004 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCI-1720 Device driver for Linux
 *
 * File Name: 
 *	PCI1720.c
 * Abstract:
 *	This file contains routines for general driver core.
 *
 * Version history
 *      03/28/2005			Create by LI.ANG
 *
 ************************************************************************/
#define __NO_VERSION__
#include "PCI1720.h"
 

static INT32S dio_write_port_byte(private_data *privdata, INT16U chan, INT16U data)
{
	INT8U  mask;
	INT16U temp;
	

	mask = 0xFF;
	temp = *((INT16U *)privdata->do_prestate);


	data = (data & mask) | (privdata->do_prestate[chan] & ~(mask));

	if(chan == 0) {
		temp = temp & 0xff00;		// clear low byte data
		data = data | temp; 		
	} else {
		temp = temp & 0x00ff;		// clear high byte data
		data = ((data << 8) & 0xff00) | temp; 		
	}
	
    /* write the desired output state to the hardware */
    advOutpw(privdata, 0x1c, data);

    return 0;
    
	
}

static INT32S dio_write_port_word(private_data *privdata, INT16U chan, INT16U data)
{
	memcpy(privdata->do_prestate, &data, sizeof(INT16U));
	advOutpw(privdata, 0x1c, data);
	return 0;
	
}


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
	INT16U i;
	
	INT32U  chan = structs->portstart;


	if (privdata->device_type != PCI1723) {
		return -EINVAL;
	}


	for (i = 0; i < structs->portcount; i++) {
		privdata->do_prestate[i + structs->portstart] = data[i];
	}


	if (structs->portcount == 1) {
		dio_write_port_byte(privdata, chan, *(data));
	} else if (structs->portcount == 2) {
		dio_write_port_word(privdata, 0, *((INT16U *)data));		
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
	case PCI1723:
		break;
	default:
		return -EINVAL;
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

