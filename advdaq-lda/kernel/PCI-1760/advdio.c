/***************************************************************************
 *
 *	Copyright 2004 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCI-1760 Device driver for Linux
 *
 * File Name: 
 *	PCI1760.c
 * Abstract:
 *	This file contains routines for general driver core.
 *
 * Version history
 *      03/28/2005			Create by LI.ANG
 *
 ************************************************************************/
#define __NO_VERSION__
#include "PCI1760.h"
 

/**
 * adv_di_counter_reset - reset the value of a specified counter to the preset value
 *                                                            
 * @device: Points to the device              
 * @structs: copy of argument from user
 * @data: counter number
 */
/* static INT32S adv_di_counter_reset(adv_device *device, adv_struct *structs, INT16U *data) */
/* { */
/* 	private_data *privdata = (private_data*)(device->private_data); */
/* 	INT16U counter; */
	


/* 	switch (privdata->device_type) { */
/* 	case PCI1760: */
/* 		break; */
/* 	default: */
/* 		return BoardIDNotSupported; */
/* 	} */
        
/* 	counter = *data; */
/* 	if (counter > 0xffff) { */
/* 		return InvalidCountNumber; */
/* 	}	 */

/* 	command_out(privdata, 0, C_PSEUDO); */
/* 	command_out(privdata, (INT32U) counter, CCO_PRESET); */
	
/* 	return 0;	 */
/* }	 */

/**
 * adv_fdi_transfer - transfer special interrupt status to user
 *                                                            
 * @device: Points to the device              
 * @structs: copy of argument from user
 * @data: pointer of user buffer which recieved interrupt status data
 */
INT32S adv_fdi_transfer(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_FDITransfer *lp_fdi_transfer = (PT_FDITransfer *) arg;
	INT32U tmp = 0;
	

	switch (lp_fdi_transfer->usEventType) {
	case ADS_EVT_DI_PATTERNMATCH_PORT0:
		tmp = privdata->pattern_data;
		break;
	case ADS_EVT_DI_STATUSCHANGE_PORT0:
		tmp = privdata->status_data;
		break;
	case ADS_EVT_COUNTERMATCH:
		tmp = privdata->cnt_match_data;
		break;
	case ADS_EVT_COUNTEROVERFLOW:
		tmp = privdata->cnt_overflow_data;
		break;
	default:
		return -EINVAL;
	}

	if (copy_to_user(lp_fdi_transfer->ulRetData, &tmp, sizeof(INT32U))) {
		return -EFAULT;
	}
 
	return 0;
}


/**
 * adv_di_read - read digital input data from the specified   
 *               digital I/O port                             
 *                                                            
 * @device: Points to the device
 * @structs: copy of argument from user
 * @data: store the data read from I/O port 
 */
/* static INT32S adv_di_read(adv_device *device, adv_struct *structs, INT8U *data) */
/* { */
/* 	private_data *privdata = (private_data*)(device->private_data); */
/* 	INT16U portstart; */
/* 	INT16U portnum; */
/* 	INT16U max_channel; */
		

/* 	portstart = structs->portstart; */
/* 	portnum = structs->portcount; */

/* 	switch (privdata->device_type) { */
/* 	case PCI1760: */
/* 		max_channel = MAX_DI_PORT; */
/* 		break; */
/* 	default: */
/* 		return BoardIDNotSupported; */
/* 	} */

/* 	if ((portnum > max_channel) || (portstart > max_channel)) { */
/* 		return InvalidPort; */
/* 	} */
	
/* 	*data = advInp(privdata, 0x1f); */

/* 	return 0; */
/* } */


/**
 * adv_do_write - write digital output data to the specified   
 *                digital I/O port                             
 *                                                            
 * @device: Points to the device              
 * @structs: copy of argument from user
 * @data: store the data write to I/O port 
 */
/* static INT32S adv_do_write(adv_device *device, adv_struct *structs, INT8U *data) */
/* { */
/* 	private_data *privdata = (private_data*)(device->private_data); */
/* 	INT16U portstart; */
/* 	INT16U portnum; */
/* 	INT16U max_channel; */
		


/* 	portstart = structs->portstart; */
/* 	portnum = structs->portcount; */

/* 	switch (privdata->device_type) { */
/* 	case PCI1760: */
/* 		max_channel = MAX_DO_PORT; */
/* 		break; */
/* 	default: */
/* 		return BoardIDNotSupported; */
/* 	} */

/* 	if ((portnum > max_channel) || (portstart > max_channel)) { */
/* 		return InvalidPort; */
/* 	} */
	
/* 	command_out(privdata, (INT32U) *data, CRO_VALUE); */
	
/* 	privdata->do_prestate = *data; */
     

/* 	return 0; */
/* } */


/**
 * adv_do_get - read back latest output data from the specified   
 *              digital output port                             
 *                                                            
 * @device: Points to the device              
 * @structs: copy of argument from user
 * @data: store the data read from previous output data
 */
/* static INT32S adv_do_get(adv_device *device, cnt_struct *structs, INT16U *data) */
/* { */
/* 	private_data *privdata = (private_data *) (device->private_data); */
/* 	INT16U port, portnum; */


/* 	switch (privdata->device_type) { */
/* 	case PCI1760: */
/* 		break; */
/* 	default: */
/* 		return BoardIDNotSupported; */
/* 	} */

/* 	port = structs->portstart; */
/* 	portnum = structs->portcount; */
/* 	*data = privdata->do_prestate; */

/* 	return 0; */
/* } */


/**
 * adv_dio_ioctl - dio dispatch function
 *                                                            
 * @device: Points to the device              
 * @arg: argument from user
 */
/* INT32S adv_dio_ioctl(adv_device *device, void *arg) */
/* { */
/* 	adv_struct structs; */
/* 	INT8U *data = NULL; */
/* 	INT16U ret = 0;	 */


/* 	if (copy_from_user(&structs, arg, sizeof(adv_struct))) { */
/* 		return MemoryCopyFailed; */
/* 	} */
     
/* 	if (structs.buffer != NULL) { */
/* 		data = kmalloc(structs.portcount * sizeof(INT8U), GFP_KERNEL); */
/* 		if (!data) { */
/* 			return MemoryAllocateFailed; */
/* 		} */
		
/* 		if (copy_from_user(data, structs.buffer, structs.portcount * sizeof(INT8U))) { */
/* 			return MemoryCopyFailed; */
/* 		} */
/* 	} */
	
/* 	switch (structs.type) { */
/* 	case DI: */
/* 		ret = adv_di_read(device, &structs, data); */
/* 		break; */
/* 	case DO: */
/* 		ret = adv_do_write(device, &structs, data); */
/* 		break; */
/* 	case GETDO: */
/* 		ret = adv_do_get(device, &structs, (INT16U *) data); */
/* 		break; */
/*  	case DI_COUNTER_RESET: */
/* 		ret = adv_di_counter_reset(device, NULL, (INT16U *) data); */
/* 		break; */
/* 	case FDI_TRANSFER: */
/* 		ret = adv_fdi_transfer(device, NULL, data); */
/* 		break; */
/* 	default: */
/* 		break; */
/* 	} */

/* 	if (!ret) { */
/* 		if (copy_to_user(structs.buffer, data, structs.portcount * sizeof(INT8U))) { */
/* 			return MemoryCopyFailed; */
/* 		} */
/* 	} */

/* 	return ret; */
/* } */
