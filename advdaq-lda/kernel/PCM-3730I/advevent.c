

/* base module includes */
#include <linux/module.h>
#include "PCM3730I.h"

int adv_enable_event(adv_device *device,ULONG EventType,USHORT Enabled,USHORT Count)
{
	private_data *privdata = (private_data*)(device->private_data);

	if(Enabled){
		if(Count <= 0)
			return -EFAULT;
		
		/* set the interrupt control register */
		switch(EventType){
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				0, Count);
			privdata->ctrlcode |= 1;
			break;
		case ADS_EVT_DI_INTERRUPT1:
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				1, Count);
			privdata->ctrlcode |= 2;
			break;
		default:
			break;
		}

	}else{
		switch(EventType)
		{
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			privdata->ctrlcode &= 2;
			break;
		case ADS_EVT_DI_INTERRUPT1:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			privdata->ctrlcode &= 1;
			break;
		default:
			break;
		}

	}

	advOutp(privdata, 0x08, privdata->ctrlcode);
	advOutp(privdata, 0x0c, privdata->event_trigger_mode);

	return 0;
}

INT32U adv_check_event(adv_device *device, INT32U *EventType, 
		       INT32U Milliseconds)
{
	private_data *privdata = (private_data*)(device->private_data);
	adv_process_info_header *p = &privdata->ptr_process_info;
	INT32S event_th = 0;
		
	if(!(*EventType))
	{
		event_th = adv_process_info_check_event(p);
		
		if(event_th <= 0){
			*EventType = 0;
		}else{
			switch(event_th -1)
			{
			case 0:
				*EventType = ADS_EVT_DI_INTERRUPT0;
				break;
			case 1:
				*EventType = ADS_EVT_DI_INTERRUPT1;
				break;
			}
			
		}
	  
	}
	else{
		switch(*EventType)
		{
		case ADS_EVT_DI_INTERRUPT0:
			event_th = 0;
			break;
		case ADS_EVT_DI_INTERRUPT1:
			event_th = 1;
			break;
		default:
			event_th = -1;
			break;
		}
		if((event_th >= 0) && (event_th <=3)){
			*EventType = adv_process_info_check_special_event(p, event_th);
		}else{
			*EventType = 0;
		}
		
	}
     
	return 0;
     
}

	  
