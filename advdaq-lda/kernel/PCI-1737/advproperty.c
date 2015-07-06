/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1737 Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	advdevfun.c                                             
 * Abstract:                                                   
 * 	This file contains routines for device function.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	10/26/2006			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "PCI1737.h"

 static int adv_set_8255mode(adv_device *device,USHORT usChannel, USHORT usDir)
{
     private_data *privdata = (private_data*)(device->private_data);
     USHORT usDirCtrl = 0;
     int i, iPort, iGroup;
     USHORT Reg_addr;
     
     if (usChannel >= privdata->port_num) {
	     return -EINVAL;
     }
     
     if((usDir == 3) || (usDir == 4))     {
	  if((usChannel%3)!= 2){
	       return -EINVAL;
	  }
     }
     
     privdata->usDioPortDir[usChannel] = usDir;
     
     iGroup = usChannel/3;
     
     /* **************************************************
      * Input/Output Control
      *     Bit              Description
      *     D0     Port C lower bits  0: output 1: input
      *     D1     Port B             0: output 1: input
      *     D2     Don't care
      *     D3     Port C higher bits 0: output 1: input
      *     D4     Port A             0: output 1: input
      *     D5     Don't Care
      *     D6     Don't Care
      *     D7     Don't Care
      * ***************************************************/
    
     for(i = iGroup*3; i < iGroup*3 +3; i++)     {
	  iPort = i%3;
	  if(privdata->usDioPortDir[i] == 0) 	  {
	       if(iPort == 0) usDirCtrl |= 0x10;
	       else if(iPort == 1) usDirCtrl |= 0x02;
	       else if(iPort == 2) usDirCtrl |= 0x09;
	       
	  }  else if(privdata->usDioPortDir[i] == 1) 	  {
	       if(iPort == 0) usDirCtrl &= 0xef;
	       else if(iPort == 1) usDirCtrl &= 0xfd;
	       else if(iPort == 2) usDirCtrl &= 0xf6;
	  }  else if(privdata->usDioPortDir[i] == 3)   {
	       if(iPort == 2)	       {
		    usDirCtrl |= 0x01;
		    usDirCtrl &= 0xf7;
	       }
	  }  else if(privdata->usDioPortDir[i] == 4)   {
	       if(iPort == 2)	       {
		    usDirCtrl |= 0x08;
		    usDirCtrl &= 0xfe;
	       }
	  }
     }
     /* get the register address */
     Reg_addr = (iGroup * 4) + 3;
     advOutp(privdata, Reg_addr, usDirCtrl);
	 
     return 0;
     
}

/**
 * adv_set_user_buffer - we get user buffer through this safe way
 *               
 * @buf: points to the user buffer
 * @chan: user buffer len
 * @size: size of data that will be transfered to the user buffer
 * @item: the data to be copy to the user buffer
 */
static INT32S adv_set_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	if (buf != NULL) {
		if (*len < size){
			return -EFAULT;
		}
		if (copy_to_user(buf, item, size)) {
			return -EFAULT;
		}	
	}
	*len = size;

	return 0;
}


INT32S adv_dev_get_property(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceGetParam dev_get_param;
	INT32U length, actual_len = 0;
	void *buffer;
	INT32S ret=0;
    	ULONG i; 
	INT8U trig_edge[6];

	memset(trig_edge, 0, 6);

	if (copy_from_user(&dev_get_param, arg, sizeof(PT_DeviceGetParam))) {
		return -EFAULT;
	}	

	buffer = dev_get_param.pData;

	if (copy_from_user(&length, dev_get_param.Length, sizeof(INT32U))) {
		return -EFAULT;
	}	

	switch (dev_get_param.nID) {
/* 	case CFG_SwitchID: */
/* 		actual_len = sizeof(privdata->device_type); */
/* 		ret = adv_set_user_buffer(buffer, &length, actual_len, */
/* 					  (void *) &(privdata->device_type)); */
/* 		break; */
	case CFG_BoardID:
		actual_len = sizeof(privdata->board_id);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->board_id));
		break;
	case CFG_BaseAddress:
		actual_len = sizeof(privdata->iobase);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->iobase));
		break;
	case CFG_Interrupt:
		actual_len = sizeof(privdata->irq);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
			                (void*)&(privdata->irq));
		break;
	case CFG_BusNumber:
		actual_len = sizeof(privdata->pci_bus);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->pci_bus));
		break;
	case CFG_SlotNumber:
		actual_len = sizeof(privdata->pci_slot);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->pci_slot));
		break;
	case CFG_DiChannelCount:
		actual_len = sizeof(privdata->di_channel_cnt);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->di_channel_cnt));
		break;
	case CFG_DoChannelCount:
		actual_len = sizeof(privdata->do_channel_cnt);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->do_channel_cnt));
		break;
	case CFG_DiPortCount:
		actual_len = sizeof(privdata->di_port_cnt);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->di_port_cnt));
		break;
	case CFG_DoPortCount:
		actual_len = sizeof(privdata->do_port_cnt);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->do_port_cnt));
		break;
		
	case CFG_DiInterruptTriggerOnRisingEdge:
		if (privdata->usInterruptSrc & 0x04) {
			trig_edge[2] = 1;
		}
		
		if (privdata->usInterruptSrc & 0x40) {
			trig_edge[5] = 1;
		}
		
		actual_len = privdata->port_num * sizeof(INT8U);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) trig_edge);
		break;
	case CFG_DiInterruptTriggerOnFallingEdge:
		if (!(privdata->usInterruptSrc & 0x04)) {
			trig_edge[2] = 1;
		}
		
		if (!(privdata->usInterruptSrc & 0x40)) {
			trig_edge[5] = 1;
		}
		
		actual_len = privdata->port_num * sizeof(INT8U);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) trig_edge);
		break;
	case CFG_DioChannelDirection:
		
		actual_len = privdata->port_num * sizeof(INT8U);
		length = actual_len;
		for(i = 0; i < privdata->port_num; i++)	{
			switch(privdata->usDioPortDir[i]){
			case 0:
				*(INT8U *)buffer++ = DIO_ChannelDir_DI;
				break;
			case 1:
				*(INT8U *)buffer++ =DIO_ChannelDir_DO;
				break;
			case 3:
				*(INT8U *)buffer++ = DIO_ChannelDir_HighHalf_DO;
				break;
			case 4:
				*(INT8U *)buffer++ = DIO_ChannelDir_LowHalf_DO;
				break;
			default:
				return -EINVAL;
			}
	       
		}
	  
		break;
	case CFG_InterruptTriggerSource:
		actual_len = privdata->port_group * sizeof(INT32U);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
				  (void *) (privdata->usIntSrcConfig));
		break;

	default:
		return -EFAULT;
	}

	if (copy_to_user(dev_get_param.Length, &length, sizeof(INT32U))) {
		return -EFAULT;
	}

	return ret;
}


INT32S adv_dev_set_property(adv_device *device, void *arg)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT32S   ret = 0;
	INT8U* pdata = NULL;
	ULONG i;
     

	PT_DeviceSetParam dev_set_param;
       INT16U nID;
       void*  pData;
       INT32U lDataLength;

	if (copy_from_user(&dev_set_param, arg, sizeof(PT_DeviceSetParam))) {
		return -EFAULT;
	}	
	
	lDataLength = dev_set_param.Length;
	pData = dev_set_param.pData;
	nID = dev_set_param.nID;

	pdata = kmalloc(lDataLength * sizeof(*pdata), GFP_KERNEL);
	if(!pdata) {
		return -ENOMEM;
	}
	memcpy(pdata,pData,lDataLength);
	switch(nID) {
	case CFG_DioChannelDirection: {
			INT8U *pDir = pdata;
			if (privdata->port_num > lDataLength/sizeof(*pdata)) {
				return -EINVAL;
			}
			for(i = 0; i < privdata->port_num; i++, pDir++) {
				switch(*pDir) {
				case DIO_ChannelDir_DI:
					ret = adv_set_8255mode(device, (USHORT)i, 0);
				        	
					break;
				case DIO_ChannelDir_DO:
					ret = adv_set_8255mode(device, (USHORT)i, 1);
					break;
				case DIO_ChannelDir_HighHalf_DO:
					ret = adv_set_8255mode(device, (USHORT)i, 3);
					break;
				case DIO_ChannelDir_LowHalf_DO:
					ret = adv_set_8255mode(device, (USHORT)i, 4);
					break;
				default:

					ret = -EINVAL;
				}
				if (ret < 0) {
					break;
				}
				
		    
			}
	       
		}
	  
	break;
     
	case CFG_DiInterruptTriggerOnRisingEdge:
		if(pData == NULL || lDataLength < privdata->port_num * sizeof(INT8U)) {
			ret =  -EINVAL;
			break;
		}
		if((*((INT8U *)pdata + 2) & 0x01)!= 0){
			privdata->usInterruptSrc |= 0x04;
		} else {
			privdata->usInterruptSrc &= ~0x04;
		}
		
		if((*((INT8U *)pdata + 5) & 0x01) != 0) {
			privdata->usInterruptSrc |= 0x40;
		} else {
			privdata->usInterruptSrc &= ~0x40;
		}

/* 		if((*((INT8U *)pdata + 2) & 0x01)!= 0)		{ */
/* 			privdata->usInterruptSrc |= 0x04; */
/* 		} */
/* 		if((*((INT8U *)pdata + 5) & 0x01) != 0)		{ */
/* 			privdata->usInterruptSrc |= 0x40; */
/* 		} */
		
		break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
		if(pData == NULL || lDataLength < privdata->port_num * sizeof(INT8U)) {     
			ret = -EINVAL;
			break;
		}
	  
		if((*((INT8U *)pdata + 2)& 0x01)!= 0) {
			privdata->usInterruptSrc &= ~0x04;
		} else {
			privdata->usInterruptSrc |= 0x04;
		}
		
		if((*((INT8U *)pdata + 5) & 0x01) != 0) {
			privdata->usInterruptSrc &= ~0x40;
		} else {
			privdata->usInterruptSrc |= 0x40;
		}
/* 		if((*(INT8U *)pdata & 0x01)!= 0)		{ */
/* 			privdata->usInterruptSrc &= ~0x04; */
/* 		} */
/* 		if((*((INT8U *)pdata + 1) & 0x01) != 0)		{ */
/* 			privdata->usInterruptSrc &= ~0x40; */
/* 		} */
		
		break;
		  
	case CFG_InterruptTriggerSource:
		if(pData == NULL || lDataLength < privdata->port_group * sizeof(INT32U)){
			ret = -EINVAL;
			break;
		}
		for (i = 0; i < privdata->port_group; i++) {
			if (*((INT32U *)pdata + i) < 3) {
				privdata->usIntSrcConfig[i] = *((INT32U *)pdata + i);
			} else {
				ret = -EINVAL;
				break;
			}
	       	  
		}


/* 		if(lDataLength/sizeof(INT32U) >= 1)		{ */
/* 			if((*((INT32U *)pData) & 0xff) == TriggerSource_PC0) { */
/* 				privdata->usIntSrcConfig[0] = TriggerSource_PC0; */
		    
/* 			}	else if ((*((INT32U *)pData) & 0xff) == TriggerSource_PC0_PC4)			{ */
/* 				privdata->usIntSrcConfig[0] = TriggerSource_PC0_PC4; */
/* 			}	else if((*((INT32U *)pData) & 0xff) == TriggerSource_TMR1)	{ */
/* 				privdata->usIntSrcConfig[0] = TriggerSource_TMR1; */
/* 			}else if((*((INT32U *)pData) & 0xff) == TriggerSource_Disable)	{ */
/* 				privdata->usIntSrcConfig[0] = TriggerSource_Disable; */
/* 			}else{ */
/* 				privdata->usIntSrcConfig[0] = TriggerSource_Disable; */
/* 				ret = -EINVAL; */
/* 				break; */
/* 			} */
	       	  
/* 		} */
/* 		if(lDataLength/sizeof(INT32U) >= 1)		{ */
/* 			if((*((INT32U *)pData + 1) & 0xff) == TriggerSource_PC10)			{ */
/* 				privdata->usIntSrcConfig[1] = TriggerSource_PC10; */
/* 			}	else if ((*((INT32U *)pData + 1) & 0xff) == TriggerSource_PC10_PC14)			{ */
/* 				privdata->usIntSrcConfig[1] = TriggerSource_PC10_PC14; */
/* 			}	else if((*((INT32U *)pData + 1) & 0xff) == TriggerSource_CNT2)		{ */
/* 				privdata->usIntSrcConfig[1] = TriggerSource_CNT2; */
/* 			}	else if((*((INT32U *)pData + 1) & 0xff) == TriggerSource_Disable)	{ */
/* 				privdata->usIntSrcConfig[1] = TriggerSource_Disable; */
/* 			}	else		{ */
/* 				privdata->usIntSrcConfig[1] = TriggerSource_Disable; */
		    
/* 				ret = -EINVAL; */
/* 				break; */
/* 			} */
   	  
/* 		} */
	      
		break;
	default:
		ret = -EINVAL;
		break;
	  
  
	}
     
      
	kfree(pdata);
     
	return ret;
     
}
