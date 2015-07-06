/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 
	
			Advantech Co., Ltd.

		Advantech PCI-1751 Device driver for Linux

	File Name: 
		advdio.c
	Abstract:
		This file contains routines for general driver core.

	Version history
	01/11/2006			Create by Zhiyong.Xie

************************************************************************/
#define __NO_VERSION__
#include "PCI1751.h"

/* static int adv_di_read(adv_device *device, adv_struct *structs, BYTE *data) */
/* { */
/* 	int portstart,portnum; */
/* 	int i; */
	
/* 	private_data *privdata = (private_data*)(device->private_data); */
/* 	portstart = structs->portstart; */
/* 	portnum = structs->portcount; */

/* 	if((portstart > 6) || (portstart < 0)) */
/* 	     return InvalidPort; */
/* 	if(((portstart + portnum) > 6) ||  ((portstart + portnum) < 0)) */
/* 	     return InvalidPort; */
	
/* 	for(i = 0; i < portnum; i++) */
/* 	{ */
/* 	     int port_addr; */
/* 	     port_addr = ((portstart + i)/3)*4 + ((portstart + i)%3); */
/* 	     *(data + i) = advInp(privdata, port_addr); */
/* 	} */
	
/* 	return 0; */
/* }	 */

/* static int adv_do_write(adv_device *device, adv_struct *structs, BYTE *data) */
/* { */
/*      int portstart, portnum; */
/*      int i; */
	
/*      private_data *privdata = (private_data*)(device->private_data); */
		
/*      portstart = structs->portstart; */
/*      portnum = structs->portcount; */
/*      if((portstart > 6) || (portstart < 0)) */
/* 	  return InvalidPort; */
/*      if(((portstart + portnum) > 6) ||  ((portstart + portnum) < 0)) */
/* 	  return InvalidPort; */
	     
/*      for(i = 0; i < portnum; i++) */
/*      { */
/* 	  int port_addr; */
/* 	  port_addr = ((portstart + i)/3)*4 + ((portstart + i)%3); */
/* 	  advOutp(privdata, port_addr, *(data + i)); */
/* 	  privdata->dovalue_save[i + portstart] = *(data + i); */
/*      } */
	
/*      return 0;	 */
/* }	 */

/* static int adv_do_get(adv_device *device, adv_struct *structs, BYTE *data) */
/* { */
/*      int portstart, portnum; */
/*      int i; */
     
/*      private_data *privdata = (private_data*)(device->private_data); */
/*      portstart = structs->portstart; */
/*      portnum = structs->portcount; */

/*      if((portstart > 6) || (portstart < 0)) */
/* 	  return InvalidPort; */
/*      if(((portstart + portnum) > 6) ||  ((portstart + portnum) < 0)) */
/* 	  return InvalidPort; */
	
/*      for(i = 0; i < portnum; i++) */
/*      { */
/* 	  int port_addr; */
/* 	  port_addr = ((portstart + i)/3)*4 + ((portstart + i)%3); */

/* 	  *(data + i) = privdata->dovalue_save[i + portstart]; */
/*      } */
/*      return 0; */
/* } */


INT32S adv_dio_set_event_config(adv_device *device, INT32U event_type, INT32U portstart, INT32U portcount)
{
	private_data *privdata = (private_data *)device->private_data;
	INT32U event_th;
	INT32S ret = 0;
	
	switch (event_type) {
	case ADS_EVT_DI_INTERRUPT16:
		event_th = 0;
		break;
	case ADS_EVT_DI_INTERRUPT40:
		event_th = 1;
		break;
	case ADS_EVT_DI_INTERRUPT64:
		if (privdata->device_type == MIC3751) {
			event_th = 2;
		} else {
			ret = -EFAULT;
		}
		break;
		
	case ADS_EVT_TERMINATE_CNT0:
		if (privdata->device_type == MIC3751) {
			event_th = 3;
		} else {
			ret = -EFAULT;
		}
		break;
		
	case  ADS_EVT_INTERRUPT_TIMER1:
		event_th = 2;
		break;     
	case ADS_EVT_TERMINATE_CNT1:
		if (privdata->device_type == MIC3751) {
			event_th = 4;
		} else {
			ret = -EFAULT;
		}
		break;
	case ADS_EVT_TERMINATE_CNT2:
		if (privdata->device_type == MIC3751) {
			event_th = 5;
		} else {
			event_th = 3;
		}
		
		break;
	default:
		return -EFAULT;
	}
      
	if (ret) {
		return ret;
	}
     
	if(portstart > (MAX_DIO_PORT - 1)){
		return -EFAULT;
	}
	
	if((portstart + portcount) > MAX_DIO_PORT){
		return -EFAULT;
	}
	
   	privdata->ports_event_scan[event_th].start = portstart;
	privdata->ports_event_scan[event_th].num = portcount;
	
	return ret;
}

INT32S adv_dio_get_event_data(adv_device *device, INT32U event_type, INT8U *pbuf, INT32U len)
{
	private_data *privdata = (private_data *)device->private_data;
	INT32U event_th;
	INT32U portstart;
	INT32U portcount;
	INT32S ret = 0;
	
	switch (event_type) {
	case ADS_EVT_DI_INTERRUPT16:
		event_th = 0;
		break;
	case ADS_EVT_DI_INTERRUPT40:
		event_th = 1;
		break;
	case ADS_EVT_DI_INTERRUPT64:
		if (privdata->device_type == MIC3751) {
			event_th = 2;
		} else {
			ret = -EFAULT;
		}
		break;
		
	case ADS_EVT_TERMINATE_CNT0:
		if (privdata->device_type == MIC3751) {
			event_th = 3;
		} else {
			ret = -EFAULT;
		}
		break;
		
	case  ADS_EVT_INTERRUPT_TIMER1:
		event_th = 2;
		break;     
	case ADS_EVT_TERMINATE_CNT1:
		if (privdata->device_type == MIC3751) {
			event_th = 4;
		} else {
			ret = -EFAULT;
		}
		break;
	case ADS_EVT_TERMINATE_CNT2:
		if (privdata->device_type == MIC3751) {
			event_th = 5;
		} else {
			event_th = 3;
		}
		
		break;
	default:
		ret = -EFAULT;
	}
                 
	if (ret) {
		return ret;
	}
	
	portstart =  privdata->ports_event_scan[event_th].start;
	portcount =  privdata->ports_event_scan[event_th].num;
        
	if(len < portcount){
		return -EFAULT;
	}
	
	memcpy(pbuf, &privdata->ports_status_event[event_th].status[portstart], portcount);
	
	return 0;
}

int adv_dio_ioctl(adv_device *device, void *arg)
{
	int ret = SUCCESS;
	adv_struct structs;
	BYTE *data = NULL;

	if(copy_from_user(&structs,arg,sizeof(adv_struct))){
		return -EFAULT;
	}
    
	data=kmalloc(structs.portcount*sizeof(BYTE),GFP_KERNEL);
	if(!data){
		ret = -ENOMEM;
		return ret;
	}
	     
	if(copy_from_user(data,structs.buffer,structs.portcount*sizeof(BYTE))){
		kfree(data);
	  	return -EFAULT;
	}
	switch(structs.type){
/* 	case DI: */
/* 		ret = adv_di_read(device,&structs,data); */
/* 		if(ret){ */
/* 			kfree(data); */
/* 			return ret; */
/* 		} */
/* 		break; */
/* 	case DO: */
/* 		ret = adv_do_write(device,&structs,data); */
/* 		if(ret){ */
/* 			kfree(data); */
/* 			return ret; */
/* 		} */
/* 		break; */
/* 	case GETDO: */
/* 		ret = adv_do_get(device,&structs,data); */
/* 		if(ret){ */
/* 			kfree(data); */
/* 			return ret; */
/* 		} */
/* 		break; */

	case DIO_ENABLE_EVENT:
		ret = adv_dio_set_event_config(device, structs.data, structs.portstart, structs.portcount);
		break;
	case DIO_EVENT_GET_DI_STATE:
		ret = adv_dio_get_event_data(device, structs.data, data, structs.portcount);
		break;

	default:
		ret = -EFAULT;
		kfree(data);
		return ret;
	}
	if(copy_to_user(structs.buffer,data,structs.portcount*sizeof(BYTE))){
		ret = -EFAULT;
	}

	kfree(data);
	return ret;
}

