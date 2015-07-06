/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 
	
			Advantech Co., Ltd.

		Advantech USB-4761 Device driver for Linux

File Name: 
	usbdio.c
Abstract:
	This file contains dio operation.

Version history
	03/21/2006			Create by Zhiyong.Xie

************************************************************************/
#include <linux/module.h>
#include "USB4761.h"


INT32S adv_usb_ctrl_transfer(adv_device *device, USB_TRANSFER_INFO *info)
{
	USB_TRANSFER_INFO *trans_info;
	SETUP_PACKET *setup_packet;
	INT8U *buf;
	private_data *privdata = (private_data *)(device->private_data);
	INT32S ret = 0;
	
	trans_info = (USB_TRANSFER_INFO *)kmalloc(sizeof(USB_TRANSFER_INFO), GFP_KERNEL);
	if(!trans_info){
		return -ENOMEM;
	}
	
	ret = copy_from_user(trans_info, info, sizeof(USB_TRANSFER_INFO));
	if(ret){
		kfree(trans_info);
		return ret;
	}
	
	setup_packet = (SETUP_PACKET *)trans_info;
	
	buf = (INT8U *)kmalloc(setup_packet->wLength, GFP_KERNEL);
	if(!buf){
		kfree(trans_info);
		return -ENOMEM;
	}
	
	ret = copy_from_user(buf, trans_info->pBuffer, setup_packet->wLength);
	if(ret){
		kfree(trans_info);
		kfree(buf);
		return ret;
	}

	if (privdata->udev == NULL) {
		return -EFAULT;
	}
	
	if(setup_packet->bmRequestType & 0x80){
		/* In type */
		ret = adv_usb_ctrl_msg(privdata->udev, \
				       usb_rcvctrlpipe(privdata->udev, 0), \
				       setup_packet->bRequest, \
				       setup_packet->bmRequestType,\
				       setup_packet->wValue,\
				       setup_packet->wIndex, buf, \
				       setup_packet->wLength);
		
	} else {
		/* Out type */
		ret = adv_usb_ctrl_msg(privdata->udev, \
				       usb_sndctrlpipe(privdata->udev, 0), \
				       setup_packet->bRequest, \
				       setup_packet->bmRequestType, \
				       setup_packet->wValue, \
				       setup_packet->wIndex, buf, \
				       setup_packet->wLength);
	}
	
	if(ret < 0){
		kfree(trans_info);
		kfree(buf);
		return ret;
	}
	
	ret = 0;
	
	if(setup_packet->bmRequestType & 0x80){
		ret = copy_to_user(trans_info->pBuffer, buf, setup_packet->wLength);
		if (ret != 0) {
			ret = -EFAULT;
		}
	}

	kfree(trans_info);
	kfree(buf);
	return ret;
}
