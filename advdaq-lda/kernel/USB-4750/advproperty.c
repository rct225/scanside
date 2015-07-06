/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech USB-4750 Device driver for Linux              
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
  
#include "USB4750.h"

 
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
	if (*len < size) {
		*len = size;
		return -EINVAL;
	}
		
	if (copy_to_user(buf, item, size)) {
		return -EFAULT;
	}

	*len = size;

	return 0;
}

/**
 * get_user_buffer - we get user buffer through this safe way
 *
 * @buf: points to the user buffer
 * @len: user buffer len
 * @size: size of data that will be received from the user buffer
 * @item: the data to be received
 */
static INT32S get_user_buffer(void *buf, INT32U len, INT32U size, void *item)
{
	if (len < size) {
		return -EINVAL;
	}

	if (copy_from_user(item, buf, size)) {
		return -EFAULT;
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
	INT32U tmp = 0;
	INT32U *len_ptr;
	INT32S ret;
     

	if (copy_from_user(&dev_get_param, arg, sizeof(PT_DeviceGetParam))) {
		return -EFAULT;
	}


	len_ptr = dev_get_param.Length;
	buffer = dev_get_param.pData;
	property = dev_get_param.nID;

	if (buffer == NULL) {
		return -EINVAL;
	}
	
	if (copy_from_user(&length, len_ptr, sizeof(INT32U))) {
		return -EFAULT;
	}


	switch (property) {
	case CFG_DiInterruptTriggerOnRisingEdge:
		actual_len = sizeof(INT32U);

		tmp = privdata->trig_edge & 0x0101;
		ret = set_user_buffer(buffer, &length, actual_len,
				      (VOID *) &tmp);
		break;
	case CFG_DiInterruptTriggerOnFallingEdge:
		actual_len = sizeof(INT32U);

		tmp = ~privdata->trig_edge & 0x0101;
		ret = set_user_buffer(buffer, &length, actual_len,
				      (VOID *) &tmp);
		break;
	case CFG_InterruptTriggerSource:
		actual_len = sizeof(INT32U);
		tmp = 3;
		
		if (down_interruptible(privdata->usb_urb_sema)) {
			return -ERESTARTSYS;
		}
	
		if (privdata->udev == NULL) {
			up(privdata->usb_urb_sema);
			return -ENODEV;
		}

		ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				       MAJOR_DIRECT_IO, 0x40,
				       MINOR_DIRECT_EE_READ_TX, 0,
				       (INT8U *) &tmp,
				       sizeof(INT16U));
		if (ret < 0) {
			up(privdata->usb_urb_sema);
			return ret;
		}

		ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
				       MAJOR_DIRECT_IO, 0x40 | 0x80,
				       MINOR_DIRECT_EE_READ_RX, 0,
				       (INT8U *) &tmp,
				       sizeof(INT16U));
		if (ret < 0) {
			up(privdata->usb_urb_sema);
			return ret;
		}
		
		up(privdata->usb_urb_sema);
		
		ret = set_user_buffer(buffer, &length, actual_len, (VOID *) &tmp);

		break;
	case CFG_DoPresetValue:
		actual_len = sizeof(INT32U);
		tmp = 1;
		
		if (down_interruptible(privdata->usb_urb_sema)) {
			return -ERESTARTSYS;
		}
	
		if (privdata->udev == NULL) {
			up(privdata->usb_urb_sema);
			return -ENODEV;
		}

		ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				       MAJOR_DIRECT_IO, 0x40,
				       MINOR_DIRECT_EE_READ_TX, 0,
				       (INT8U *) &tmp,
				       sizeof(INT16U));
		if (ret < 0) {
			up(privdata->usb_urb_sema);
			return ret;
		}

		ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
				       MAJOR_DIRECT_IO, 0x40 | 0x80,
				       MINOR_DIRECT_EE_READ_RX, 0,
				       (INT8U *) &tmp,
				       sizeof(INT16U));
		if (ret < 0) {
			up(privdata->usb_urb_sema);
			return ret;
		}
		
		up(privdata->usb_urb_sema);

		ret = set_user_buffer(buffer, &length, actual_len, (VOID *) &tmp);

		break;
	default:
		return -EINVAL;
	}


	if (copy_to_user(len_ptr, &length, sizeof(INT32U))) {
		return -EFAULT;
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
	VOID *buffer;
	VOID *tmp_buf;
	INT16U i;
	INT32U tmp = 0;
	INT32S ret;
     

	
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
	case CFG_DiInterruptTriggerOnRisingEdge: 
		actual_len = sizeof(privdata->trig_edge);

		ret = get_user_buffer(buffer, length, actual_len, tmp_buf);
		if (ret) {
			kfree(tmp_buf);
			return ret;
		}

		tmp = *((INT16U *) tmp_buf) & 0x0101;
		
		for (i = 0; i < MAX_DI_CHANNEL; i++) {
			if (tmp & (0x0001 << i)) {
				privdata->trig_edge |= 0x0001 << i;
			} else {
				privdata->trig_edge &= ~(0x0001 << i);
			}
		}

		break;
	case CFG_DiInterruptTriggerOnFallingEdge:
		actual_len = sizeof(privdata->trig_edge);

		ret = get_user_buffer(buffer, length, actual_len, tmp_buf);
		if (ret) {
			kfree(tmp_buf);
			return ret;
		}
		tmp = *((INT16U *) tmp_buf) & 0x0101;
		
		for (i = 0; i < MAX_DI_CHANNEL; i++) {
			if (tmp & (0x0001 << i)) {
				privdata->trig_edge &= ~(0x0001 << i);
			} else {
				privdata->trig_edge |= 0x0001 << i;
			}
		}
		
		break;
	case CFG_InterruptTriggerSource:
	{
		EE_WRITE_STRUCT ee_write_data;

		
		actual_len = sizeof(privdata->di_int_mode);
		
		ret = get_user_buffer(buffer, length, actual_len, tmp_buf);
		if (ret) {
			kfree(tmp_buf);
			return ret;
		}
		privdata->di_int_mode = *((INT16U *) tmp_buf) & 0x303;


		ee_write_data.usAddr = 3;
		ee_write_data.usEEData = privdata->di_int_mode;
		
		if (down_interruptible(privdata->usb_urb_sema)) {
			kfree(tmp_buf);
			return -ERESTARTSYS;
		}
	
		if (privdata->udev == NULL) {
			kfree(tmp_buf);
			up(privdata->usb_urb_sema);
			return -ENODEV;
		}

		ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				       MAJOR_DIRECT_IO, 0x40,
				       MINOR_DIRECT_EE_WRITE_TX, 0,
				       (INT8U *) &ee_write_data,
				       sizeof(EE_WRITE_STRUCT));
		if (ret < 0) {
			kfree(tmp_buf);
			up(privdata->usb_urb_sema);
			return ret;
		}

		mdelay(10);	/* waitting for EEProm write period */
	
		up(privdata->usb_urb_sema);
	}
	break;
	case CFG_DoPresetValue:
	{
		EE_WRITE_STRUCT ee_write_data;


		actual_len = sizeof(INT32U);
		
		ret = get_user_buffer(buffer, length, actual_len, tmp_buf);
		if (ret) {
			kfree(tmp_buf);
			return ret;
		}

		tmp = *((INT32U *) tmp_buf);
		if (tmp > 65535) { /* max value is 16-bits width */
			kfree(tmp_buf);
			return -EINVAL;
		}
		
		privdata->do_preset = (INT16U) tmp;
		
		ee_write_data.usAddr = 1;
		ee_write_data.usEEData = privdata->do_preset;
		

		if (down_interruptible(privdata->usb_urb_sema)) {
			kfree(tmp_buf);
			return -ERESTARTSYS;
		}
	
		if (privdata->udev == NULL) {
			kfree(tmp_buf);
			up(privdata->usb_urb_sema);
			return -ENODEV;
		}

		ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				       MAJOR_DIRECT_IO, 0x40,
				       MINOR_DIRECT_EE_WRITE_TX, 0,
				       (INT8U *) &ee_write_data,
				       sizeof(EE_WRITE_STRUCT));
		if (ret < 0) {
			kfree(tmp_buf);
			up(privdata->usb_urb_sema);
			return ret;
		}

		mdelay(10);	/* waitting for EEProm write period */

		up(privdata->usb_urb_sema);
	}
		break;
	default:
		kfree(tmp_buf);
		return -EINVAL;
	}
 	
	kfree(tmp_buf);

	return 0;
}

