/********************************************************************
 *				                                   
 * 		Copyright 2004 IAG Software Team,                  
 *                                                                  
 * 			BeiJing R&D Center                         
 *	                                                           
 * 			Advantech Co., Ltd.                        
 *                                                                  
 * 	    Advantech PCI-1720 Device driver for Linux             
 *                                                                  
 *                                                                  
 *                                                                  
 * 	File Name:                                                 
 * 		advport.c                                            
 * 	Abstract:                                                  
 * 		This file contains routines for port io function.       
 *                                                                  
 *                                                                  
 * 	Version history                                            
 * 	01/09/2006			Create by LI.ANG           
 *                                                                  
 ********************************************************************/
#include "PCI1720.h"


/**
  * adv_read_port - Read specified IO port
  *
  * @device: points to the device object
  * @arg: argument from user
  */
INT32S adv_read_port(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_PortOperation port_operation;
	INT32U port_num;
	INT32U data_len;
	INT32U buf  = 0;
	INT16U port_type;
	VOID *user_buf;


	if (copy_from_user(&port_operation, arg, sizeof(PT_PortOperation))) {
		return -EFAULT;
	}

	port_type = (INT16U) port_operation.PortType;
	port_num = port_operation.PortNumber;
	data_len = port_operation.DataLength;
	user_buf = port_operation.DataBuffer;


	if (data_len > sizeof(INT32U)) {
		return -EINVAL;
	}
     
	switch (port_type) {
	case AD_READ_PORT_UCHAR:
		buf = (INT8U) advInp(privdata, port_num);
		break;
	case AD_READ_PORT_USHORT:
		buf = (INT16U) advInpw(privdata, port_num);
		break;
	case AD_READ_PORT_ULONG:
		buf = (INT32U) advInpdw(privdata, port_num);
		break;
	default:
		break;
	}

	if (copy_to_user(user_buf, &buf, data_len)) {
		return -EFAULT;
	}

	if (copy_to_user((void *) arg, &port_operation, sizeof(PT_PortOperation))) {
		return -EFAULT;
	}

	return 0;
}


/**
  * adv_write_port -Write specified IO port
  *
  * @device: points to the device object
  * @arg: argument from user
  */
INT32S adv_write_port(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_PortOperation port_operation;
	INT32U port_num;
	INT32U data_len;
	INT32U buf = 0;
	INT16U port_type;
	VOID *user_buf;
     

	if (copy_from_user(&port_operation, arg, sizeof(PT_PortOperation))) {
		return -EFAULT;
	}

	port_type = (INT16U) port_operation.PortType;
	port_num = port_operation.PortNumber;
	data_len = port_operation.DataLength;
	user_buf = port_operation.DataBuffer;


	if (copy_from_user(&buf, (INT32U *) user_buf, data_len)) {
		return -EFAULT;
	}
     
	if (data_len > sizeof(INT32U)) {
		return -EINVAL;
	}
     
	switch (port_type) {
	case AD_WRITE_PORT_UCHAR:
		advOutp(privdata, port_num, (INT8U) buf);
		break;
	case AD_WRITE_PORT_USHORT:
		advOutpw(privdata, port_num, (INT16U) buf);
		break;
	case AD_WRITE_PORT_ULONG:
		advOutpdw(privdata, port_num, (INT32U) buf);
		break;
	default:
		break;
	}

	if (copy_to_user((void *) arg, &port_operation, sizeof(PT_PortOperation))) {
		return -EFAULT;
	}

	return 0;
}
