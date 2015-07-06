/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech USB-4711 Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	advproperty.c                                             
 * Abstract:                                                   
 * 	This file contains routines for device function.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	07/20/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "USB4711.h"

 
/**
 * set_user_buffer - we set user buffer through this safe way
 *
 * @buf: points to the user buffer
 * @len: user buffer len
 * @size: size of data that will be transfered to the user buffer
 * @item: the data to be copy to the user buffer
 */
/* static INT32S set_user_buffer(void *buf, INT32U *len, INT32U size, void *item) */
/* { */
/* 	if (*len < size) { */
/* 		*len = size; */
/* 		return -EINVAL; */
/* 	} */
		
/* 	if (copy_to_user(buf, item, size)) { */
/* 		return -EFAULT; */
/* 	} */

/* 	*len = size; */

/* 	return 0; */
/* } */

/**
 * get_user_buffer - we get user buffer through this safe way
 *
 * @buf: points to the user buffer
 * @len: user buffer len
 * @size: size of data that will be received from the user buffer
 * @item: the data to be received
 */
/* static INT32S get_user_buffer(void *buf, INT32U len, INT32U size, void *item) */
/* { */
/* 	if (len < size) { */
/* 		return -EINVAL; */
/* 	} */

/* 	if (copy_from_user(item, buf, size)) { */
/* 		return -EFAULT; */
/* 	} */

/* 	return 0; */
/* } */


INT32S adv_dev_get_property(adv_device *device, void *arg)
{
	INT32S ret = 0;

	return ret;
}


INT32S adv_dev_set_property(adv_device *device, void *arg)
{
/* 	private_data *privdata = (private_data *) (device->private_data); */
	PT_DeviceSetParam dev_set_param;
	INT32U length;
/* 	INT32U actual_len = 0; */
	INT16U property;
	VOID *buffer;
	VOID *tmp_buf;
/* 	INT32U tmp; */
/* 	INT32S ret; */

	
	if (copy_from_user(&dev_set_param, arg, sizeof(PT_DeviceSetParam))) {
		return -EFAULT;
	}

	length = dev_set_param.Length;
	buffer = dev_set_param.pData;
	property = dev_set_param.nID;

	if (buffer == NULL) {
		return -EINVAL;
	}

	if (length == 0) {
		return -EINVAL;
	}

	tmp_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!tmp_buf) {
		return -ENOMEM;
	}
     
	switch (property) {
/* 	case CFG_BoardID: */
/* 		actual_len = sizeof(INT32U); */

/* 		ret = get_user_buffer(buffer, length, actual_len, tmp_buf); */
/* 		if (ret) { */
/* 			kfree(tmp_buf); */
/* 			return ret; */
/* 		} */
/* 		tmp = *((INT32U *) tmp_buf); */

		
/* 		if (tmp > 0xf) { */
/* 			kfree(tmp_buf); */
/* 			return -EINVAL; */
/* 		} */
		
/* 		if (privdata->udev == NULL) { */
/* 			kfree(tmp_buf); */
/* 			return -EFAULT; */
/* 		} */
     
/* 		if (down_interruptible(privdata->usb_urb_sema)) { */
/* 			kfree(tmp_buf); */
/* 			return -ERESTARTSYS; */
/* 		} */

/* 		ret = adv_usb_ctrl_msg(privdata->udev, */
/* 				       usb_sndctrlpipe(privdata->udev, 0), */
/* 				       MAJOR_SYSTEM, 0x40, */
/* 				       MINOR_WRITE_SWITCHID, 0, */
/* 				       (INT8U *) &privdata->board_id, */
/* 				       sizeof(INT32U)); */
/* 		if (ret < 0) { */
/* 			kfree(tmp_buf); */
/* 			up(privdata->usb_urb_sema); */
/* 			return ret; */
/* 		} */

/* 		up(privdata->usb_urb_sema); */

/* 		privdata->board_id = tmp; */
/* 		_ADV_SET_DEVICE_BOARDID(device, (INT16U) privdata->board_id); */
/* 		break; */
	default:
		kfree(tmp_buf);
		return -EINVAL;
	}
 	
	kfree(tmp_buf);

	return 0;
}

