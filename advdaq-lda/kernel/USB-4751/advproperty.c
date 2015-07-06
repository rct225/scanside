/********************************************************************
 *				                                    
 * 	Copyright 2007 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech USB-4751 Device driver for Linux              
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
 * 	01/26/2007			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "USB4751.h"

 
/**
 * adv_set_user_buffer - we get user buffer through this safe way
 *               
 * @buf: points to the user buffer
 * @chan: user buffer len
 * @size: size of data that will be transfered to the user buffer
 * @item: the data to be copy to the user buffer
 */
static INT32S adv_set_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	if (buf != NULL) {
		if (*len < size)
			return -EFAULT;
		if (copy_to_user(buf, item, size)) {
			return -EFAULT;
		}	
	}
	
	*len = size;

	return 0;
}

INT32S adv_dev_get_property(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceGetParam dev_get_param;
	INT32U length, actual_len = 0;
	void *buffer;
	INT32S ret;
     

	if (copy_from_user(&dev_get_param, arg, sizeof(PT_DeviceGetParam))) {
		return -EFAULT;
	}	

	buffer = dev_get_param.pData;

	if (copy_from_user(&length, dev_get_param.Length, sizeof(INT32U))) {
		return -EFAULT;
	}	

	if (length > PAGE_SIZE) {
		printk("buffer length should less than one page size.\n");
		return -EFAULT;
	}

	switch (dev_get_param.nID) {
	case CFG_BoardID:
	case CFG_SwitchID:
		actual_len = sizeof(privdata->board_id);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->board_id));
		break;
		
	default:
		return -EFAULT;
	}

	if (copy_to_user(dev_get_param.Length, &length, sizeof(INT32U))) {
		return -EFAULT;
	}

	return ret;
}


INT32S adv_dev_set_property(adv_device *device, void *arg)
{
	return -EFAULT;
}

INT32S adv_get_err_code(adv_device *device, void * arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U error;


	
	error = adv_process_info_get_error(&privdata->ptr_process_info);
	
	if (copy_to_user((INT32U *) arg, &error, sizeof(INT32U))) {
		return -EFAULT;
	}

	return 0;
}

