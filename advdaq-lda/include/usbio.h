/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech USB Device Driver for Linux

File Name:
	USBIO.h
Abstract:
	Header file of USB device driver. 
Version history
	03/20/2006		Create by zhiyong.xie

************************************************************************/	


#ifndef __ADV_USBPUB_H__
#define __ADV_USBPUB_H__
#include "advdevice.h"
typedef struct __SETUP__PACKET
{
     INT8U bmRequestType;
     INT8U bRequest;
     INT16U wValue;
     INT16U wIndex;
     INT16U wLength;
}SETUP_PACKET, *PSETUP_PACKET;

typedef struct __USB_TRANSFER_INFO
{
	SETUP_PACKET SetupPacket;
	INT8U *pBuffer;
}USB_TRANSFER_INFO, *PUSB_TRANSFER_INFO;

#endif
