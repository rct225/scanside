/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1780 Device driver for Linux              
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
  
#include "PCI1780.h"
 
/**
 * adv_set_user_buffer - we get user buffer through this safe way
 *               
 * buf: Points to the user buffer
 * len: user buffer len
 * size: size of data that will be transfered to the user buffer
 * item: the data to be copy to the user buffer
 */
static INT32S adv_set_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	if (buf != NULL) {
		if (*len < size)
			return InvalidInputParam;
		if (copy_to_user(buf, item, size)) {
			return MemoryCopyFailed;
		}	
	}
	*len = size;

	return SUCCESS;
}

/**
 * adv_get_user_buffer - we get user buffer through this safe way
 *               
 * @buf: Points to the user buffer
 * @len: user buffer len
 * @size: size of data that will be received from the user buffer
 * @item: the data to be received
 */
static INT32S adv_get_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	if (buf != NULL) {
		if (*len < size)
			return InvalidInputParam;
		if (copy_from_user(item, buf, size)) {
			return MemoryCopyFailed;
		}	
	}

	return SUCCESS;
}

INT32S adv_dev_get_property(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceGetParam dev_get_param;
	INT32U length, actual_len = 0;
/* 	INT32U tmp = 0; */
	void *buffer;
	INT32S ret = 0;
     

	if (copy_from_user(&dev_get_param, arg, sizeof(PT_DeviceGetParam))) {
		return MemoryCopyFailed;
	}	

	buffer = dev_get_param.pData;

	if (copy_from_user(&length, dev_get_param.Length, sizeof(INT32U))) {
		return MemoryCopyFailed;
	}	

	if (length > PAGE_SIZE) {
		printk("buffer length should less than one page size.\n");
		return InvalidInputParam;
	}

	switch (dev_get_param.nID) {
	case CFG_BoardID:
		actual_len = sizeof(privdata->board_id);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->board_id));
		break;
	case CFG_BaseAddress:
		actual_len = sizeof(privdata->iobase);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->iobase));
		break;
	case CFG_Interrupt:
		actual_len = sizeof(privdata->irq);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->irq));
		break;
	case CFG_BusNumber:
		actual_len = sizeof(privdata->pci_bus);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->pci_bus));
		break;
	case CFG_SlotNumber:
		actual_len = sizeof(privdata->pci_slot);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->pci_slot));
		break;
	case CFG_DioPortType:
/* 		tmp = (INT32U) DIO_PortType_SeperateDIO; */
/* 		actual_len = sizeof(tmp); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &tmp); */
		break;
	default:
		return ParamNameNotSupported;
	}

	if (copy_to_user(dev_get_param.Length, &length, sizeof(INT32U))) {
		return MemoryCopyFailed;
	}

	return ret;
}

INT32S adv_dev_set_property(adv_device *device, void *arg)
{
/* 	private_data *privdata = (private_data *) (device->private_data); */
	PT_DeviceSetParam dev_set_param;
	INT32U length, actual_len = 0;
	void *buffer, *tmp_buf;
	INT32S ret;
     


	if (copy_from_user(&dev_set_param, arg, sizeof(PT_DeviceSetParam))) {
		return MemoryCopyFailed;
	}	

	length = dev_set_param.Length;
	buffer = dev_set_param.pData;
	actual_len = sizeof(INT16U);
     
	if (length != actual_len)
		return InvalidInputParam;
	  
	tmp_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!tmp_buf) {
		return MemoryAllocateFailed;
	}
     

	switch (dev_set_param.nID) {
	case CFG_DiInterruptTriggerOnFallingEdge:
		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
/* 		privdata->int_trig_mode |= *((INT16U *) tmp_buf); */
		break;
	default:
		kfree(tmp_buf);
		return ParamNameNotSupported;
	}
     
	kfree(tmp_buf);
	return ret;
}

INT32S adv_get_device_name(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);


	
	if (copy_to_user(arg, &privdata->device_type, sizeof(INT32U))) {
		printk("failed~!\n");
		return MemoryCopyFailed;
	}
	printk("ok!\n");
	return 0;
}

INT32S adv_get_err_code(adv_device *device, void * arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U error;

	
	error = adv_process_info_get_error(&privdata->ptr_process_info);
	
	if (copy_to_user((INT32U *) arg, &error, sizeof(INT32U))) {
		return MemoryCopyFailed;
	}

	return SUCCESS;
}
