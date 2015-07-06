#include "PCI1751.h"
INT32S adv_enable_event(adv_device *device,USHORT EventType,USHORT Enabled,USHORT Count)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT32S ret = -EFAULT;
	
	if(Enabled){
		/* set the interrupt control register */
		switch (EventType) {
		case ADS_EVT_DI_INTERRUPT16:
			if ((privdata->usIntSrcConfig[0] == 0x01) || (privdata->usIntSrcConfig[0] == 0x02)) {
				privdata->usInterruptSrc &= 0xffc;
				privdata->usInterruptSrc |= privdata->usIntSrcConfig[0] & 0x03;
				adv_process_info_enable_event(&privdata->ptr_process_info, 0, Count);
				ret = 0;
			} 
			break;
	
		case ADS_EVT_DI_INTERRUPT40:
			if ((privdata->usIntSrcConfig[1] == 0x01) || (privdata->usIntSrcConfig[1] == 0x02)) {
				privdata->usInterruptSrc &= 0xfcf;
				privdata->usInterruptSrc |= ((privdata->usIntSrcConfig[1] << 4) & 0x30) ;
				adv_process_info_enable_event(&privdata->ptr_process_info, 1, Count);
				ret = 0;
			}
			break;
		case ADS_EVT_DI_INTERRUPT64:
			if (privdata->device_type != MIC3751) {
				break;
			}
			
			if ((privdata->usIntSrcConfig[2] == 0x01) 
			    || (privdata->usIntSrcConfig[2] == 0x02)) {
				privdata->usInterruptSrc &= 0xcff;
				privdata->usInterruptSrc |= ((privdata->usIntSrcConfig[2] << 8) & 0x300) ;
				adv_process_info_enable_event(&privdata->ptr_process_info, 2, Count);
				ret = 0;
			} 
			break;


		case ADS_EVT_TERMINATE_CNT0:
			if (privdata->device_type != MIC3751) {
				break;
			}
			
			if (privdata->usIntSrcConfig[0] == 0x03) {
				privdata->usInterruptSrc &= 0xffc;
				privdata->usInterruptSrc |= (privdata->usIntSrcConfig[0] & 0x03);
				/* initiallize counter2  */
				advOutp(privdata, 27, 0x36|(0<<6));
				advOutp(privdata, 24, privdata->cnt_init_data[0]&0xff);
				advOutp(privdata, 24, (privdata->cnt_init_data[0]>>8)&0xff);
				adv_process_info_enable_event(&privdata->ptr_process_info, 3, Count);
				ret = 0;
			} 
			break;
			
		case ADS_EVT_TERMINATE_CNT1:
			if (privdata->device_type != MIC3751) {
				break;
			}
			
			if (privdata->usIntSrcConfig[1] == 0x03) {
				privdata->usInterruptSrc &= 0xfcf;
				privdata->usInterruptSrc |= ((privdata->usIntSrcConfig[1] << 4) & 0x30);
				/* initiallize counter2  */
				advOutp(privdata, 27, 0x36|(1<<6));
				advOutp(privdata, 24 + 1, privdata->cnt_init_data[1]&0xff);
				advOutp(privdata, 24 + 1, (privdata->cnt_init_data[1]>>8)&0xff);
				adv_process_info_enable_event(&privdata->ptr_process_info, 4, Count);
				ret = 0;
			}
			break;
		
		case ADS_EVT_INTERRUPT_TIMER1:
			if (privdata->device_type != PCI1751) {
				break;
			}
			
			if (privdata->usIntSrcConfig[0] == 0x03) {
				privdata->usInterruptSrc &= 0xffc;
				privdata->usInterruptSrc |= privdata->usIntSrcConfig[0] & 0x03;
				if(privdata->IsCascadeMode == 1){
					/* if counter0, 1 cascade */
					advOutp(privdata, 27, 0x36|(0<<6));
					advOutp(privdata, 24, privdata->cnt_init_data[0]&0xff);
					advOutp(privdata, 24, (privdata->cnt_init_data[0]>>8)&0xff);
				}
				advOutp(privdata, 27, 0x36|(1<<6));
				advOutp(privdata, 24+1, (privdata->cnt_init_data[1])&0xff);
				advOutp(privdata, 24+1, (privdata->cnt_init_data[1]>>8)&0xff);
				adv_process_info_enable_event(&privdata->ptr_process_info, 2, Count);
				ret = 0;
			}
			
			break;

		case ADS_EVT_TERMINATE_CNT2:
			if ((privdata->device_type == PCI1751)
			    && (privdata->usIntSrcConfig[1] == 0x03)) {
				privdata->usInterruptSrc &= 0xfcf;
				privdata->usInterruptSrc |= ((privdata->usIntSrcConfig[1] << 4) & 0x30);
				/* initiallize counter2  */
				advOutp(privdata, 27, 0x36|(2<<6));
				advOutp(privdata, 24+2, privdata->cnt_init_data[2]&0xff);
				advOutp(privdata, 24+2, (privdata->cnt_init_data[2]>>8)&0xff);
				adv_process_info_enable_event(&privdata->ptr_process_info, 3, Count);
				ret = 0;
			} else if ((privdata->device_type == MIC3751)
				   && (privdata->usIntSrcConfig[2] == 0x03)) {
				
				privdata->usInterruptSrc &= 0xcff;
				privdata->usInterruptSrc |= ((privdata->usIntSrcConfig[2] << 8) & 0x300);
				/* initiallize counter2  */
				advOutp(privdata, 27, 0x36|(2<<6));
				advOutp(privdata, 24+2, privdata->cnt_init_data[2]&0xff);
				advOutp(privdata, 24+2, (privdata->cnt_init_data[2]>>8)&0xff);
				adv_process_info_enable_event(&privdata->ptr_process_info, 5, Count);
				ret = 0;
			} 			
			break;
		default:
			ret = -EFAULT;
		}
	} else {
		switch (EventType) {
		case ADS_EVT_DI_INTERRUPT16:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			if (((privdata->usIntSrcConfig[0] == 0x01) || (privdata->usIntSrcConfig[0] == 0x02)) 
			    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 0))){
				privdata->usInterruptSrc &= 0xffc;
			}
			ret = 0;
			break;

		case ADS_EVT_DI_INTERRUPT40:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			if (((privdata->usIntSrcConfig[1] == 0x01) || (privdata->usIntSrcConfig[1] == 0x02)) 
			    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 1))){
				privdata->usInterruptSrc &= 0xfcf;
			}
			ret = 0;
			break;
		case ADS_EVT_DI_INTERRUPT64:
			if (privdata->device_type != MIC3751) {
				break;
			}
			
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			if (((privdata->usIntSrcConfig[2] == 0x01) || (privdata->usIntSrcConfig[2] == 0x02)) 
			    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 2))){
				privdata->usInterruptSrc &= 0xcff;
			} 
			ret = 0;
			break;


		case ADS_EVT_TERMINATE_CNT0:
			if (privdata->device_type != MIC3751) {
				break;
			}

			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			if ((privdata->usIntSrcConfig[0] == 0x03) 
			    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 3))) {
				privdata->usInterruptSrc &= 0xffc;
			} 
			ret = 0;
			break;

		case ADS_EVT_INTERRUPT_TIMER1:
			if (privdata->device_type != PCI1751) {
				break;
			}

			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			if ((privdata->usIntSrcConfig[0] == 0x03)
			    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 2))) {
				privdata->usInterruptSrc &= 0xffc;
			} 
			ret = 0;
			break;
	

		case ADS_EVT_TERMINATE_CNT1:
			if (privdata->device_type != MIC3751) {
				break;
			}

			adv_process_info_disable_event(&privdata->ptr_process_info, 4);
			if ((privdata->usIntSrcConfig[1] == 0x03) 
			    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 4))) {
				privdata->usInterruptSrc &= 0xfcf;
			} 
			ret = 0;
			break;


		case ADS_EVT_TERMINATE_CNT2:
			if (privdata->device_type == MIC3751) {
				adv_process_info_disable_event(&privdata->ptr_process_info, 5);
				if ((privdata->usIntSrcConfig[2] == 0x03) 
				    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 5))) {
					privdata->usInterruptSrc &= 0xcff;
				}
			} else {
				adv_process_info_disable_event(&privdata->ptr_process_info, 3);
				if ((privdata->usIntSrcConfig[1] == 0x03) 
				    && (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 3))) {
					privdata->usInterruptSrc &= 0xfcf;
				}
			}
			ret = 0;
			break;
		default:
			ret = -EFAULT;
		}
	}
	advOutp(privdata, 32, privdata->usInterruptSrc);
	if (privdata->device_type == MIC3751) {
		advOutp(privdata, 33, privdata->usInterruptSrc>>8);
	}
	return ret;
}


INT32S adv_check_event(adv_device *device, INT32U *EventType, 
		       INT32U Milliseconds)
{
	private_data *privdata = (private_data*)(device->private_data);
	adv_process_info_header *p = &privdata->ptr_process_info;
	INT32S event_th = 0;

	if (!(*EventType)) {
		event_th = adv_process_info_check_event(p);
		
		if (event_th <= 0) {
			*EventType = 0;
		} else {
			switch(event_th -1) {
			case 0:
				*EventType = ADS_EVT_DI_INTERRUPT16;
				break;
			case 1:
				*EventType = ADS_EVT_DI_INTERRUPT40;
				break;
			case 2:
				if (privdata->device_type == MIC3751) {
					*EventType = ADS_EVT_DI_INTERRUPT64;
				} else {
					*EventType = ADS_EVT_INTERRUPT_TIMER1;
				}
				
				break;
			case 3:
				if (privdata->device_type == MIC3751) {
					*EventType = ADS_EVT_TERMINATE_CNT0;
				} else {
					*EventType = ADS_EVT_TERMINATE_CNT2;
				}
				
				break;
			case 4:
				if (privdata->device_type == MIC3751) {
					*EventType =ADS_EVT_TERMINATE_CNT1;
				}
				
				break;
			case 5:
				if (privdata->device_type == MIC3751) {
					*EventType =ADS_EVT_TERMINATE_CNT2;
				}
				break;
			}
		}
	} else{
		switch(*EventType)
		{
		case ADS_EVT_DI_INTERRUPT16:
			event_th = 0;
			break;
		case ADS_EVT_INTERRUPT_TIMER1:
			event_th = 1;
			break;
		case ADS_EVT_DI_INTERRUPT40:
			event_th = 2;
			break;
		case ADS_EVT_TERMINATE_CNT2:
			event_th = 3;
			break;
		default:
			event_th = -1;
			break;
		}
		if((event_th >= 0) && (event_th <=1)){
			*EventType = adv_process_info_check_special_event(p, event_th);
		}else{
			*EventType = 0;
		}
		
	}
	
	return 0;
}
