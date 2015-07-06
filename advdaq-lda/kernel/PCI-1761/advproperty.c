/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1761 Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	advdevfun.c                                             
 * Abstract:                                                   
 * 	This file contains routines for device function.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	07/20/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1761.h"
 
/**
 * adv_set_user_buffer - we get user buffer through this safe way
 *               
 * buf: Points to the user buffer
 * len: user buffer len
 * size: size of data that will be transfered to the user buffer
 * item: the data to be copy to the user buffer
 */
static INT32S adv_set_user_buffer(VOID *buf, INT32U *len, INT32U size, VOID *item)
{
	if (buf == NULL) {
		if (copy_to_user(len, &size, sizeof(size))) {
			return -EFAULT;
		}		
		return 0;
	}
	
	if (*len < size) {
		return -EINVAL;
	}
	
	if (copy_to_user(buf, item, size)) {
		return -EFAULT;
	}	

	*len = size;

	return 0;
}

/**
 * adv_get_user_buffer - we get user buffer through this safe way
 *               
 * @buf: Points to the user buffer
 * @len: user buffer len
 * @size: size of data that will be received from the user buffer
 * @item: the data to be received
 */
static INT32S adv_get_user_buffer(VOID *buf, INT32U *len, INT32U size, VOID *item)
{
	if (*len < size) {
		return -EINVAL;
	}
	
	if (copy_from_user(item, buf, size)) {
		return -EFAULT;
	}	

	return 0;
}

INT32S adv_dev_get_property(adv_device *device, VOID *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceGetParam dev_get_param;
	INT32U *length;
	INT32U actual_len = 0;
	VOID *buffer;
	INT32S ret;
     

	if (copy_from_user(&dev_get_param, arg, sizeof(PT_DeviceGetParam))) {
		return -EFAULT;
	}	

	buffer = dev_get_param.pData;
	length = dev_get_param.Length;
	
/* 	if (copy_from_user(&length, dev_get_param.Length, sizeof(INT32U))) { */
/* 		return -EFAULT; */
/* 	}	 */


	switch (dev_get_param.nID) {
/* 	case CFG_BoardID: */
/* 		actual_len = sizeof(privdata->board_id); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &(privdata->board_id)); */
/* 		break; */
/* 	case CFG_BaseAddress: */
/* 		actual_len = sizeof(privdata->iobase); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &(privdata->iobase)); */
/* 		break; */
/* 	case CFG_Interrupt: */
/* 		actual_len = sizeof(privdata->irq); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &(privdata->irq)); */
/* 		break; */
/* 	case CFG_BusNumber: */
/* 		actual_len = sizeof(privdata->pci_bus); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &(privdata->pci_bus)); */
/* 		break; */
/* 	case CFG_SlotNumber: */
/* 		actual_len = sizeof(privdata->pci_slot); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &(privdata->pci_slot)); */
/* 		break; */
/* 	case CFG_DioPortType: */
/* 		tmp = (INT32U) DIO_PortType_SeperateDIO; */
/* 		actual_len = sizeof(tmp); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &tmp); */
/* 		break; */
/* 	case CFG_DiChannelCount: */
/* 		tmp = (INT32U) MAX_DI_CHANNEL; */
/* 		actual_len = sizeof(tmp); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &tmp); */
/* 		break; */
/* 	case CFG_DiDataWidth: */
/* 		tmp = (INT32U) DI_DataWidth_Byte; */
/* 		actual_len = sizeof(tmp); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &tmp); */
/* 		break; */
/* 	case CFG_DiPortCount: */
/* 		tmp = (INT32U) MAX_DI_PORT; */
/* 		actual_len = sizeof(tmp); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &tmp); */
/* 		break; */
/* 	case CFG_DoDataWidth: */
/* 		tmp = (INT32U) DO_DataWidth_Byte; */
/* 		actual_len = sizeof(tmp); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &tmp); */
/* 		break; */
/* 	case CFG_DoChannelCount: */
/* 		tmp = (INT32U) MAX_DO_CHANNEL; */
/* 		actual_len = sizeof(tmp); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &tmp); */
/* 		break; */
/* 	case CFG_DoPortCount: */
/* 		tmp = (INT32U) MAX_DO_PORT; */
/* 		actual_len = sizeof(tmp); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &tmp); */
/* 		break; */
/* 	case CFG_DiInterruptSupportedChannel: */
/* 		tmp = (INT32U) 0xff; */
/* 		actual_len = sizeof(tmp); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &tmp); */
/* 		break; */
	case CFG_DiInterruptTriggerOnRisingEdge:
		actual_len = sizeof(privdata->int_trig_mode);

		switch (privdata->device_type) {
		case PCI1761:
		case MIC3761:
		case PCI1763:
			actual_len /= 2; /* one byte */
			break;
		}

		ret = adv_set_user_buffer(buffer, length, actual_len,
					  (VOID *) &privdata->trig_rising);
		break;
	case CFG_DiInterruptTriggerOnFallingEdge:
		actual_len = sizeof(privdata->int_trig_mode);

		switch (privdata->device_type) {
		case PCI1761:
		case MIC3761:
		case PCI1763:
			actual_len /= 2; /* one byte */
			break;
		}

		ret = adv_set_user_buffer(buffer, length, actual_len,
					  (VOID *) &privdata->trig_falling);
		break;
	default:
		return -EINVAL;
	}


	return ret;
}

INT32S adv_dev_set_property(adv_device *device, VOID *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceSetParam dev_set_param;
	INT32U length;
	INT32U actual_len = 0;
	VOID *buffer;
	VOID *tmp_buf;
	INT32S ret;
     


	if (copy_from_user(&dev_set_param, arg, sizeof(PT_DeviceSetParam))) {
		return -EFAULT;
	}	

	length = dev_set_param.Length;
	buffer = dev_set_param.pData;
	  
	tmp_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!tmp_buf) {
		return -ENOMEM;
	}
     

	switch (dev_set_param.nID) {
	case CFG_DiInterruptTriggerOnRisingEdge:
		actual_len = sizeof(privdata->int_trig_mode);
		
		switch (privdata->device_type) {
		case PCI1761:
		case MIC3761:
		case PCI1763:
			actual_len /= 2; /* one byte */
			break;
		}

		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);

		switch (privdata->device_type) {
		case PCI1761:
		case MIC3761:
		case PCI1763:
			privdata->int_trig_mode = ~(*((INT8U *) tmp_buf));
			privdata->trig_rising = *((INT8U *) tmp_buf);
			privdata->trig_falling = ~(*((INT8U *) tmp_buf));
			break;
		case PCI1762:
			privdata->int_trig_mode = ~(*((INT16U *) tmp_buf));
			privdata->trig_rising = *((INT16U *) tmp_buf);
			privdata->trig_falling = ~(*((INT16U *) tmp_buf));
			break;
		}
		break;
	case CFG_DiInterruptTriggerOnFallingEdge:
		actual_len = sizeof(privdata->int_trig_mode);

		switch (privdata->device_type) {
		case PCI1761:
		case MIC3761:
		case PCI1763:
			actual_len /= 2; /* one byte */
			break;
		}

		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		switch (privdata->device_type) {
		case PCI1761:
		case MIC3761:
		case PCI1763:
			privdata->int_trig_mode = *((INT8U *) tmp_buf);
			privdata->trig_falling = *((INT8U *) tmp_buf);
			privdata->trig_rising = ~(*((INT8U *) tmp_buf));
			break;
		case PCI1762:
			privdata->int_trig_mode = *((INT16U *) tmp_buf);
			privdata->trig_falling = *((INT16U *) tmp_buf);
			privdata->trig_rising = ~(*((INT16U *) tmp_buf));
			break;
		}
		break;
	default:
		kfree(tmp_buf);
		return ParamNameNotSupported;
	}
     
	kfree(tmp_buf);
	return ret;
}

/* INT32S adv_get_err_code(adv_device *device, void * arg) */
/* { */
/* 	private_data *privdata = (private_data *) (device->private_data); */
/* 	INT32U error; */

	
/* 	error = adv_process_info_get_error(&privdata->ptr_process_info); */
	
/* 	if (copy_to_user((INT32U *) arg, &error, sizeof(INT32U))) { */
/* 		return MemoryCopyFailed; */
/* 	} */

/* 	return SUCCESS; */
/* } */
