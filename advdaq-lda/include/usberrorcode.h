/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech USB-4761 Device Driver for Linux

File Name:
	USBErrorCode.h
Abstract:
	Header file of USB device driver. 
Version history
	03/20/2006		Create by zhiyong.xie

************************************************************************/	


#ifndef USB_ERROR_CODE_H
#define USB_ERROR_CODE_H




// ERROR code
#define USBErrorCode			 500						// Dean add
#define USBTransmitFailed		 (USBErrorCode + 1)
#define USBInvalidCtrlCode		 (USBErrorCode + 2)		
#define USBInvalidDataSize		 (USBErrorCode + 3)
#define USBAIChannelBusy		 (USBErrorCode + 4)
#define USBAIDataNotReady		 (USBErrorCode + 5)
#define USBFWUpdateFailed        (USBErrorCode + 6)
#define USBDeviceNotReady      (USBErrorCode + 7)
#define USBOperNotSuccess      (USBErrorCode + 8)
#endif
