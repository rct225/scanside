/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1720 Device driver for Linux              
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
  
#include "PCI1720.h"

 
/**
 * set_user_buffer - we set user buffer through this safe way
 *
 * @buf: points to the user buffer
 * @len: user buffer len
 * @size: size of data that will be transfered to the user buffer
 * @item: the data to be copy to the user buffer
 */
static INT32S set_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	if (buf != NULL) {
		if (*len < size) {
			return -EINVAL;
		}
		
		if (copy_to_user(buf, item, size)) {
			return -EFAULT;
		}
	}
	*len = size;

	return 0;
}

/**
 * adv_get_user_buffer - we get user buffer through this safe way
 *
 * @buf: points to the user buffer
 * @len: user buffer len
 * @size: size of data that will be received from the user buffer
 * @item: the data to be received
 */
static INT32S get_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	if (buf != NULL) {
		if (*len < size) {
			return -EINVAL;
		}

		if (copy_from_user(item, buf, size)) {
			return -EFAULT;
		}
	}

	return 0;
}


INT32S adv_dev_get_property(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceGetParam dev_get_param;
	INT32U length;
	INT32U actual_len = 0;
	VOID *buffer;
	INT16U property;
	INT32U *len_ptr;
	INT32S ret;
     

	if (copy_from_user(&dev_get_param, arg, sizeof(PT_DeviceGetParam))) {
		return -EFAULT;
	}


	len_ptr = dev_get_param.Length;
	buffer = dev_get_param.pData;
	property = dev_get_param.nID;


	if (copy_from_user(&length, len_ptr, sizeof(INT32U))) {
		return -EFAULT;
	}

	if (length > PAGE_SIZE) {
		printk("buffer length should less than one page size.\n");
		return -EINVAL;
	}

	switch (property) {
/* 	case CFG_DioPortDirection: */
/* 		actual_len = sizeof(INT32U); */
/* 		ret = set_user_buffer(buffer, &length, actual_len, */
/* 				      (VOID *) privdata->dio_direction); */
/* 		break; */
	default:
		return -EINVAL;
	}

	if (!ret) {
		if (copy_to_user(len_ptr, &length, sizeof(INT32U))) {
			return -EFAULT;
		}
	}
	

	return ret;
}


INT32S adv_dev_set_property(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceSetParam dev_set_param;
	INT32U length;
	INT32U actual_len = 0;
	INT16U property;
	void *buffer;
	void *tmp_buf;
	INT32S ret;
     

	
	if (copy_from_user(&dev_set_param, arg, sizeof(PT_DeviceSetParam))) {
		return -EFAULT;
	}

	length = dev_set_param.Length;
	buffer = dev_set_param.pData;
	property = dev_set_param.nID;

	tmp_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!tmp_buf) {
		return -ENOMEM;
	}
     

	switch (property) {
/* 	case CFG_DioPortDirection: */
/* 		actual_len = sizeof(INT32U); */

/* 		ret = get_user_buffer(buffer, &length, actual_len, tmp_buf); */
/* 		if (!ret) { */
/* 			privdata->dio_direction = *((INT32U *) tmp_buf); */
/* 		} */
		
/* 		break; */
	default:
		kfree(tmp_buf);
		return -EINVAL;
	}
 	
	kfree(tmp_buf);

	return ret;
}

