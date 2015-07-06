/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1718 Device driver for Linux

File Name:
	advport.c
Abstract:
	the port's operation(read or write)of the PCI-1718 . 
Version history
	01/16/2006		Create by zhiyong.xie

************************************************************************/	
#define __NO_VERSION__
#include "PCI1718.h"

/**
 * adv_port_read - read port's data
 * @device - pointer of struct adv_device which indentified a device
 * @port - start port
 * @databuf - user space buffer
 * @datalen - buffer size of user space
 */
INT32S adv_port_read(adv_device *device, INT32U port, void *databuf, INT32U datalen)
{
	private_data *privdata = (private_data*)(device->private_data);     
	INT32S ret = 0;
	BYTE *buf;
	INT32U i;
	
     	buf=(BYTE *)kmalloc(datalen * sizeof(BYTE), GFP_KERNEL);
	if(!buf)
		return -ENOMEM;
	
	memset(buf, 0, datalen * sizeof(BYTE));
	
	if(port > privdata->iolength){
		kfree(buf);
		return -EFAULT;
	}
     
	if(datalen > privdata->iolength - port)
		datalen = privdata->iolength -port;
	
	switch(datalen)
	{
	case 1:
		*(UCHAR *)buf=advInp(privdata, port);
			
		break;
	case 2:
		*(USHORT *)buf=advInpw(privdata, port);
		break;
	case 4:
		*(ULONG *)buf=advInpdw(privdata, port);
		break;
	default:
		for(i = 0; i < datalen; i++)
		{
			*((BYTE *)buf + i) = advInp(privdata, port + i);
		}
		break;
	}
	
	ret = copy_to_user(databuf, buf, datalen);
          
	kfree(buf);
     	return ret;
}
	  
/**
 * adv_port_write - write data to port
 * @device - a pointer of adv_devcie structure which identifies a devcie
 * @port - start port
 * @databuf - user space buffer
 * @datalen - buffer size
 */
INT32S adv_port_write(adv_device *device, INT32U port, void* databuf, INT32U datalen)
{
	private_data *privdata = (private_data*)(device->private_data);     
	BYTE *buf;
	INT32S ret = 0; 
	INT32U i;
     
	buf=(BYTE *)kmalloc(datalen * sizeof(BYTE),GFP_KERNEL);
	if(!buf)
		return -ENOMEM;
     
	memset(buf, 0, datalen * sizeof(BYTE));
     
	ret = copy_from_user(buf, databuf,datalen);
	if(ret){
		kfree(buf);
		return -EFAULT;
	}
     
	
	if(port > privdata->iolength){
		kfree(buf);
		return -EFAULT;
	}

	if(datalen > privdata->iolength - port)
		datalen = privdata->iolength - port;
	
	switch(datalen)
	{
	case 1:
		advOutp(privdata, port, *(UCHAR*)buf);
		break;
	case 2:
		advOutpw(privdata, port, *(USHORT*)buf);
		break;
	case 4:
		advOutpdw(privdata, port, *(ULONG*)buf);
		break;
	default:
		for(i = 0; i < datalen; i++)
		{
			*((BYTE *)buf + i) = advInp(privdata, port + i);
		}
		break;
	}

	kfree(buf);

	return 0;
}
