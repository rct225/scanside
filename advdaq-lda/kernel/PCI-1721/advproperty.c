/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1721 Device driver for Linux              
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
 * 	05/17/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1721.h"
 
AORANGESET ao_range_set[MAX_AO_RANGE] = {
     {6, 0, 0, 0, 5, 0},
     {6, 0, 0, 0, 10, 0},
     {6, 0, 0, 0, 5, -5},
     {6, 0, 0, 0, 10, -10},
     {6, 0, 1, 0, 20, 0},
     {6, 0, 1, 0, 20, 4},
};

/********************************************************************************
 * Description:  we get user buffer through this safe way
 *               
 * Input:        buf   -Points to the user buffer
 *               chan  -user buffer len
 *               size  -size of data that will be transfered to the user buffer
 *               item  -the data to be copy to the user buffer
 *               
 *               
 * Ouput:        errno                                        
 *               SUCCESS
 ********************************************************************************/
static int adv_set_user_buffer(void *buf, ULONG *len, ULONG size, void *item)
{
	  if (buf != NULL) {
	       if (*len < size)
		    return -InvalidInputParam;
	       if (copy_to_user(buf, item, size)) {
		    printk("copy_to_user failed !\n");
		    return -EFAULT;
	       }	
	  }
	  *len = size;

	  return SUCCESS;
}

/********************************************************************************
 * Description:  we get user buffer through this safe way
 *               
 * Input:        buf   -Points to the user buffer
 *               chan  -user buffer len
 *               size  -size of data that will be received from the user buffer
 *               item  -the data to be received
 *               
 *               
 * Ouput:        errno                                        
 *               SUCCESS
 ********************************************************************************/
static int adv_get_user_buffer(void *buf, ULONG *len, ULONG size, void *item)
{
	  if (buf != NULL) {
	       if (*len < size)
		    return -InvalidInputParam;
	       if (copy_from_user(item, buf, size)) {
		    printk("copy_from_user failed !\n");
		    return -EFAULT;
	       }	
	  }
	  *len = size;

	  return SUCCESS;
}

int adv_dev_get_property(adv_device *device, void *arg)
{
     private_data *privdata = (private_data *) (device->private_data);
     PT_DeviceGetParam dev_get_param;
     ULONG *length, actual_len = 0, tmp = 0;
     void *buffer;
     int ret = 0;
     

#ifdef PCI1721_DEBUG
     printk("DEBUG:BGN:%s()..\n", __FUNCTION__);
#endif

     if (copy_from_user(&dev_get_param, arg, sizeof(PT_DeviceGetParam))) {
	  printk("copy_from_user failed !\n");
	  return -EFAULT;
     }	

     length = dev_get_param.Length;
     buffer = dev_get_param.pData;

     if (*length > PAGE_SIZE) {
	  printk("buffer length should less than one page size.\n");
	  return -InvalidInputParam;
     }

     switch (dev_get_param.nID) {
     case CFG_CardID:
	  actual_len = sizeof(privdata->card_id);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &(privdata->card_id));
	  break;
     case CFG_BoardID:
	  actual_len = sizeof(privdata->board_id);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &(privdata->board_id));
	  break;
     case CFG_BaseAddress:
	  actual_len = sizeof(privdata->iobase);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &(privdata->iobase));
	  break;
     case CFG_Interrupt:
	  actual_len = sizeof(privdata->irq);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &(privdata->irq));
	  break;
     case CFG_BusNumber:
	  actual_len = sizeof(privdata->pci_bus);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &(privdata->pci_bus));
	  break;
     case CFG_SlotNumber:
	  actual_len = sizeof(privdata->pci_slot);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &(privdata->pci_slot));
	  break;
     case CFG_AoPacerRate:
	  actual_len = sizeof(privdata->sample_rate);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &(privdata->sample_rate));
	  break;
     case CFG_AoChanRange:
	  actual_len = sizeof(privdata->AOSettings);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &privdata->AOSettings);
	  break;
     case AO_RANGE_SETTING:
	  actual_len = sizeof(ao_range_set);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) ao_range_set);
	  break;
     case CFG_DioPortType:
	  tmp = (ULONG) DIO_PortType_8255_PortA;
	  actual_len = sizeof(tmp);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &tmp);
	  break;
     case CFG_DiChannelCount:
	  tmp = (ULONG) MAX_DI_CHANNEL;
	  actual_len = sizeof(tmp);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &tmp);
	  break;
     case CFG_DioChannelDirection: {
	  ULONG tmp_a[MAX_DIO_PORT];
	  
	  if (privdata->dio_port_mode & 0x0008) {
	       tmp_a[0] = (ULONG) 0x00;
	  } else {
	       tmp_a[0] = (ULONG) 0xff;
	  }
	  
	  if (privdata->dio_port_mode & 0x0010) {
	       tmp_a[1] = (ULONG) 0x00;
	  } else {
	       tmp_a[1] = (ULONG) 0xff;
	  }
	  
	  actual_len = sizeof(tmp_a);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) tmp_a);
	  break;
     }
     case CFG_DiDataWidth:
	  tmp = (ULONG) DI_DataWidth_Byte;
	  actual_len = sizeof(tmp);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &tmp);
	  break;
     case CFG_DiPortCount:
	  tmp = (ULONG) MAX_DI_PORT;
	  actual_len = sizeof(tmp);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &tmp);
	  break;
     case CFG_DoDataWidth:
	  tmp = (ULONG) DO_DataWidth_Byte;
	  actual_len = sizeof(tmp);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &tmp);
	  break;
     case CFG_DoChannelCount:
	  tmp = (ULONG) MAX_DO_CHANNEL;
	  actual_len = sizeof(tmp);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &tmp);
	  break;
     case CFG_DoPortCount:
	  tmp = (ULONG) MAX_DO_PORT;
	  actual_len = sizeof(tmp);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &tmp);
	  break;
     case CFG_CntrChannelCapability:
	  tmp = (ULONG) (CNTR_ChlCap_EventCounting | CNTR_ChlCap_PulseOut);
	  actual_len = sizeof(tmp);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &tmp);
	  break;
     case CFG_CntrInternalClockFreq:
	  tmp = (ULONG) INTERNAL_CLOCK_RATE;
	  actual_len = sizeof(tmp);
	  ret = adv_set_user_buffer(buffer, length, actual_len,
				     (void *) &tmp);
	  break;
     default:
	  return -ParamNameNotSupported;
     }

     if (ret < 0)
	  return ret;

     if (buffer == NULL)
	  return SUCCESS;


#ifdef PCI1721_DEBUG
     printk("DEBUG:END:%s()..\n", __FUNCTION__);
#endif
     return SUCCESS;
}

int adv_dev_set_property(adv_device *device, void *arg)
{
     private_data *privdata = (private_data *) (device->private_data);
     PT_DeviceSetParam dev_set_param;
     ULONG length, actual_len = 0, tmp = 0;
     void *buffer, *tmp_buf;
     int ret = 0;
     

#ifdef PCI1721_DEBUG
     printk("DEBUG:BGN:%s()..\n", __FUNCTION__);
#endif

     if (copy_from_user(&dev_set_param, arg, sizeof(PT_DeviceSetParam))) {
	  printk("copy_from_user failed !\n");
	  return -EFAULT;
     }	

     length = dev_set_param.Length;
     buffer = dev_set_param.pData;

     if (length > PAGE_SIZE) {
	  printk("buffer length should less than one page size.\n");
	  return -InvalidInputParam;
     }
	  
     tmp_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
     if (!tmp_buf)
	  return -ENOMEM;
     

     switch (dev_set_param.nID) {
     case CFG_AoChanRange:
     case AO_RANGE_SETTING: {
	  AORANGESET *ao_range;

	  actual_len = sizeof(privdata->AOSettings);
	  ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
	  if (ret < 0)
	       return ret;
	  
	  ao_range = (AORANGESET *) tmp_buf;
	  ret = adv_ao_range_set(device,
				 ao_range->usChan, ao_range->usAOSource, ao_range->usAOType,
				 ao_range->fAOMax, ao_range->fAOMin);
	  break;
     }
     case CFG_AoPacerRate: {
	  ULONG sample_rate;

	  actual_len = sizeof(ULONG);
	  ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
	  if (ret < 0)
	       return ret;
	  
	  sample_rate = *((ULONG *) tmp_buf);
	  
	  if (sample_rate > 2500000)
	       if ((sample_rate != 5000000) && (sample_rate != 10000000))
		    return -IllegalSpeed;

	  if (sample_rate < 2500000) {
	       privdata->divisor1 = (USHORT) dev_set_param.Reserved[0];
	       privdata->divisor2 = (USHORT) dev_set_param.Reserved[1];

	       advOutpw(privdata, 0x36, 0x0076);
	       advOutpw(privdata, 0x32, privdata->divisor2);
	       advOutpw(privdata, 0x32, (privdata->divisor2 >> 8));

	       advOutpw(privdata, 0x36, 0x00b6);
	       advOutpw(privdata, 0x34, privdata->divisor1);
	       advOutpw(privdata, 0x34, (privdata->divisor1 >> 8));

	       tmp = advInpw(privdata, 0x2a) & 0x001f;
	  } else if (sample_rate == 5000000) {
	       tmp = advInpw(privdata, 0x2a) & 0x001f;
	       tmp |= 0x0020;
	  } else if (sample_rate == 10000000) {
	       tmp = advInpw(privdata, 0x2a) & 0x001f;
	       tmp |= 0x0040;
	  }
	  advOutpw(privdata, 0x2a, tmp);

	  privdata->sample_rate = sample_rate;
	  break;
     }
     case CFG_DioChannelDirection:
	  actual_len = sizeof(ULONG) * MAX_DIO_PORT;
	  ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
	  if (ret < 0)
	       return ret;

	  tmp = *((ULONG *) tmp_buf);
	  if (tmp == 0xff)
	       adv_dio_set_mode(device, 0, 1);
	  else if (tmp == 0x00)
	       adv_dio_set_mode(device, 0, 0);
	  else
	       return -InvalidPort;
	  
	  tmp = *((ULONG *) tmp_buf + 1);
	  if (tmp == 0xff)
	       adv_dio_set_mode(device, 1, 1);
	  else if (tmp == 0x00)
	       adv_dio_set_mode(device, 1, 0);
	  else
	       return -InvalidPort;

	  break;
     default:
	  return -ParamNameNotSupported;
     }
     
     if (ret < 0)
	  return ret;

#ifdef PCI1721_DEBUG
     printk("DEBUG:END:%s()..\n", __FUNCTION__);
#endif
     return SUCCESS;
}

