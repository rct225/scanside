#include "PCI1753.h"

INT32S adv_dio_set_event_config(adv_device *device, INT32U event_type, INT32U portstart, INT32U portcount)
{
	private_data *privdata = (private_data *)device->private_data;
	INT32U event_th;
	switch (event_type) {
	case ADS_EVT_DI_PATTERNMATCH_PORT0:
		event_th = 0;
		break;
	case ADS_EVT_DI_STATUSCHANGE_PORT1:
		event_th = 1;
		break;
	case ADS_EVT_DI_INTERRUPT16:
		event_th = 2;
		break;
	case ADS_EVT_DI_INTERRUPT40:
		event_th = 3;
		break;
	case ADS_EVT_DI_INTERRUPT64:
		event_th = 4;
		break;
	case ADS_EVT_DI_INTERRUPT88:
		event_th = 5;
		break;
	case ADS_EVT_DI_PATTERNMATCH_PORT12:
		event_th = 6;
		break;
	case ADS_EVT_DI_STATUSCHANGE_PORT13:
		event_th = 7;
		break;
	case ADS_EVT_DI_INTERRUPT112:
		event_th = 8;
		break;
	case ADS_EVT_DI_INTERRUPT136:
		event_th = 9;
		break;
	case ADS_EVT_DI_INTERRUPT160:
		event_th = 10;
		break;
	case ADS_EVT_DI_INTERRUPT184:
		event_th = 11;
		break;     
	default:
		return -EFAULT;
	}
      
	if(event_th > 12) {
		return -EFAULT;
	}
     
	if(portstart > (MAX_DIO_PORT - 1)){
		return -EFAULT;
	}
	
	if((portstart + portcount) > MAX_DIO_PORT){
		return -EFAULT;
	}
	
   	privdata->ports_event_scan[event_th].start = portstart;
	privdata->ports_event_scan[event_th].num = portcount;
	
	return 0;
}

INT32S adv_dio_get_event_data(adv_device *device, INT32U event_type, INT8U *pbuf, INT32U len)
{
	private_data *privdata = (private_data *)device->private_data;
	INT32U event_th;
	INT32U portstart;
	INT32U portcount;
	
	switch (event_type) {
	case ADS_EVT_DI_PATTERNMATCH_PORT0:
		event_th = 0;
		break;
	case ADS_EVT_DI_STATUSCHANGE_PORT1:
		event_th = 1;
		break;
	case ADS_EVT_DI_INTERRUPT16:
		event_th = 2;
		break;
	case ADS_EVT_DI_INTERRUPT40:
		event_th = 3;
		break;
	case ADS_EVT_DI_INTERRUPT64:
		event_th = 4;
		break;
	case ADS_EVT_DI_INTERRUPT88:
		event_th = 5;
		break;
	case ADS_EVT_DI_PATTERNMATCH_PORT12:
		event_th = 6;
		break;
	case ADS_EVT_DI_STATUSCHANGE_PORT13:
		event_th = 7;
		break;
	case ADS_EVT_DI_INTERRUPT112:
		event_th = 8;
		break;
	case ADS_EVT_DI_INTERRUPT136:
		event_th = 9;
		break;
	case ADS_EVT_DI_INTERRUPT160:
		event_th = 10;
		break;
	case ADS_EVT_DI_INTERRUPT184:
		event_th = 11;
		break;     
	default:
		return -EFAULT;
	}
           
	portstart =  privdata->ports_event_scan[event_th].start;
	portcount =  privdata->ports_event_scan[event_th].num;
        
	if(len < portcount){
		return -EFAULT;
	}
	
	memcpy(pbuf, &privdata->ports_status_event[event_th].status[portstart], portcount);
	
	return 0;
}

INT32S adv_dio_ioctl(adv_device *device, void *arg)
{
	INT32S ret = 0;
	adv_struct structs;
	BYTE *data = NULL;

	ret = copy_from_user(&structs, arg, sizeof(adv_struct));
	if(ret){
		return ret;
	}
    
	if(structs.buffer){
		data=kmalloc(structs.portcount*sizeof(BYTE),GFP_KERNEL);
		if(!data){
			return -ENOMEM;
		}
	     
		ret = copy_from_user(data,structs.buffer,structs.portcount*sizeof(BYTE));
		if(ret){
			kfree(data);
			return -ENOMEM;
		}
	}
     
	switch(structs.type){
	case DIO_ENABLE_EVENT:
		ret = adv_dio_set_event_config(device, structs.data, structs.portstart, structs.portcount);
		break;
	case DIO_EVENT_GET_DI_STATE:
		ret = adv_dio_get_event_data(device, structs.data, data, structs.portcount);
		break;
	default:
		ret = -EFAULT;
		break;
	}
	
	if(structs.buffer){
		ret = copy_to_user(structs.buffer, data, structs.portcount*sizeof(BYTE));
		if(ret){
			kfree(data);
			return ret;
		}
	}
	kfree(data);
     
	return ret;
}
