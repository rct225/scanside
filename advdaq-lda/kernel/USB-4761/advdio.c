#include "USB4761.h"

INT32S adv_dio_set_event_config(adv_device *device, INT32U event_type, INT32U portstart, INT32U portcount)
{
	private_data *privdata = (private_data *)device->private_data;
	INT32U event_index;
//	INT32S ret = 0;
     
     
	event_index = event_type - ADS_EVT_DI_INTERRUPT0;
     
	if((event_index < ADS_EVT_DI_INTERRUPT0_Index) 
	   || (event_index > ADS_EVT_DI_INTERRUPT7_Index)){
		return -EFAULT;
	}
     
	if(portstart > (MAX_DI_PORTS_NUM -1)){
		return -EFAULT;
	}
	
	if((portstart + portcount) > MAX_DI_PORTS_NUM){
		return -EFAULT;
	}
	
   	privdata->ports_event_scan[event_index].start = portstart;
	privdata->ports_event_scan[event_index].num = portcount;
	
	return 0;
}

INT32S adv_dio_get_event_data(adv_device *device, INT32U event_type, INT8U *pbuf, INT32U len)
{
	private_data *privdata = (private_data *)device->private_data;
	INT32U event_index;
	INT32U portstart;
	INT32U portcount;
	
	event_index = event_type - ADS_EVT_DI_INTERRUPT0;

	if((event_index < ADS_EVT_DI_INTERRUPT0_Index) 
	   || (event_index > ADS_EVT_DI_INTERRUPT7_Index)){
		return -EFAULT;
	}
     
	portstart =  privdata->ports_event_scan[event_index].start;
	portcount =  privdata->ports_event_scan[event_index].num;
        
	if(len < portcount){
		return -EFAULT;
	}
	
	memcpy(pbuf, &privdata->ports_status_event[event_index].status[portstart], portcount);
	
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
