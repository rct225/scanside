/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 
	
			Advantech Co., Ltd.

		Advantech PCI-1718 Device driver for Linux

	File Name: 
		advdio.c
	Abstract:
		This file contains routines for general driver core.

	Version history
	01/11/2006			Create by Zhiyong.Xie

************************************************************************/
#define __NO_VERSION__
#include "PCI1718.h"

static INT32S adv_di_read(adv_device *device, INT32U portstart, 
			  INT32U portnum, BYTE *data)
{
	private_data *privdata = (private_data*)(device->private_data);
     
	if( (portstart > 1) || ((portstart + portnum) > 2) )
	{
		adv_process_info_set_error(&privdata->ptr_process_info, 
					   InvalidPort);
		return -EFAULT;
	}

	switch(portnum)
	{
	case 1:
		*data = advInp(privdata, (portstart? 0xB : 0x3));
		break;
	case 2:
		*data = advInp(privdata, 0x3);
		*(data + 1) = advInp(privdata, 0xB);
		break;
	default:
		break;
	}
	
	return 0;	
}

static INT32S adv_do_write(adv_device *device, INT32U portstart, 
			   INT32U portnum, BYTE *data)
{
	private_data *privdata = (private_data*)(device->private_data);
         
	if( (portstart > 1) || ((portstart + portnum ) > 2) )
	{
		adv_process_info_set_error(&privdata->ptr_process_info, 
					   InvalidPort);
		return -EFAULT;
	}
	
	switch(portnum)
	{
	case 1:
		advOutp(privdata, (portstart? 0xB : 0x3), *data);
		privdata->dovalue_save[0] = *data;
		break;
	case 2:
		advOutp(privdata, 0x3, *data);
		advOutp(privdata, 0xB, *(data + 1));
		privdata->dovalue_save[0] = *data;
		privdata->dovalue_save[1] = *(data + 1);
		break;
	default:
		break;
	}

	return 0;	
}

static INT32S adv_do_get(adv_device *device, INT32U portstart, 
			 INT32U portnum, BYTE *data)
{
	INT32U i;
	
	private_data *privdata = (private_data*)(device->private_data);
     
	if( (portstart > 1) || ((portstart + portnum ) > 2) )
	{
		adv_process_info_set_error(&privdata->ptr_process_info, 
					   InvalidPort);
		return -EFAULT;
	}

	for(i = 0; i < portnum; i++)
	{
		*(data + i) = privdata->dovalue_save[i + portstart];
	}

	return 0;	
}


int adv_dio_ioctl(adv_device *device, void *arg)
{
	int ret = SUCCESS;
	adv_struct structs;
	BYTE *data = NULL;
	private_data *privdata = (private_data*)(device->private_data);

#ifdef PCI1718_DEBUG
	KdPrint("DEBUG:BGN:adv_dio_ioctl()\n");
#endif
	if(copy_from_user(&structs,arg,sizeof(adv_struct))){
		printk("error occured when copy_from_user()..1\n");
		adv_process_info_set_error(&privdata->ptr_process_info, 
					   MemoryCopyFailed);
		return -EFAULT;
	}
    
	data=kmalloc(structs.portcount*sizeof(BYTE),GFP_KERNEL);
	if(!data){
		adv_process_info_set_error(&privdata->ptr_process_info, 
					   MemoryAllocateFailed);
		return -ENOMEM;
	}
	     
	if(copy_from_user(data,structs.buffer,structs.portcount*sizeof(BYTE))){
		printk("error occured when copy_from_user()..1\n");
		kfree(data);
		adv_process_info_set_error(&privdata->ptr_process_info, 
					   MemoryCopyFailed);
		return -EFAULT;
	}

	switch(structs.type){
	case DI:
		ret = adv_di_read(device, structs.portstart, structs.portcount, data);
		if(ret){
			printk("adv_di_read func return error!\n");
			kfree(data);
			return ret;
		}
		break;
	case DO:
		ret = adv_do_write(device, structs.portstart, structs.portcount, data);
		if(ret){
			printk("adv_do_write func return error!\n");
			kfree(data);
			return ret;
		}
		break;
	case GETDO:
		ret = adv_do_get(device, structs.portstart, structs.portcount, data);
		if( ret ){
			printk("adv_do_get func return error!\n");
			kfree(data);
			return ret;
		}
		break;
	default:
		ret = adv_di_read(device, structs.portstart, structs.portcount, data);
		if(!ret){
			printk("adv_di_read func return error!\n");
			kfree(data);
			return ret;
		}
		break;
			
			
	}

	if(copy_to_user(structs.buffer,data,structs.portcount*sizeof(BYTE))){
		kfree(data);
		adv_process_info_set_error(&privdata->ptr_process_info, 
					   MemoryCopyFailed);
		return -EFAULT;
	}
	kfree(data);
     
#ifdef PCI1718_DEBUG
	KdPrint(" DEBUG:END:adv_dio_ioctl()\n");
#endif
	return ret;

}
