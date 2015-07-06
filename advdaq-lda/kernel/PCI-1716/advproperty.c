/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1716 Device driver for Linux              
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
 * 	09/26/2006			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "PCI1716.h"

 
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
		if (*len < size)
			return -EFAULT;
		if (copy_to_user(buf, item, size)) {
			return -EFAULT;
		}	
	}
	*len = size;

	return 0;
}

/**
 * adv_get_user_buffer - we get user buffer through this safe way
 *               
 * @buf: points to the user buffer
 * @chan: user buffer len
 * @size: size of data that will be received from the user buffer
 * @item: the data to be received
 */
static INT32S adv_get_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	if (buf != NULL) {
		if (*len < size)
			return -EFAULT;
		if (copy_from_user(item, buf, size)) {
			return -EFAULT;
		}	
	}

	return 0;
}

INT32S adv_dev_get_property(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceGetParam dev_get_param;
	INT32U length, actual_len = 0;
	void *buffer;
	INT32S ret;
     

	if (copy_from_user(&dev_get_param, arg, sizeof(PT_DeviceGetParam))) {
		return -EFAULT;
	}	

	buffer = dev_get_param.pData;

	if (copy_from_user(&length, dev_get_param.Length, sizeof(INT32U))) {
		return -EFAULT;
	}	

	if (length > PAGE_SIZE) {
		printk("buffer length should less than one page size.\n");
		return -EFAULT;
	}

	switch (dev_get_param.nID) {
	case CFG_BoardID:
	case CFG_SwitchID:
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
	case CFG_AiPacerRate:
		actual_len = sizeof(privdata->sample_rate);
		return adv_set_user_buffer(buffer, &length,actual_len,
					  (void *) &(privdata->sample_rate));	
	case CFG_AiChanConfig: 
		actual_len = sizeof(privdata->sd_mask);
		return adv_set_user_buffer(buffer, &length,actual_len,
					  (void *) &(privdata->sd_mask));	

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
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceSetParam dev_set_param;
	INT32U length;
	INT32U actual_len = 0;
	void *buffer;
	void *tmp_buf;
	INT32S ret=0;

	
	if (copy_from_user(&dev_set_param, arg, sizeof(PT_DeviceSetParam))) {
		return -EFAULT;
	}	

	length = dev_set_param.Length;
	buffer = dev_set_param.pData;

	tmp_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!tmp_buf) {
		return -EFAULT;
	}
     

	switch (dev_set_param.nID) {
	case CFG_AiPacerRate:
		actual_len = sizeof(INT32U);
		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		privdata->sample_rate = *((INT32U *) tmp_buf);

		break;
	case CFG_AiChanConfig: 
	{
		INT16U mask;
		INT16U i;

		actual_len = sizeof(INT16U);
		if (length < actual_len) {
			return -EFAULT;
		}
		
		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		privdata->sd_mask = *((INT16U *) tmp_buf);


		/* if user set X channel (even number) to differential,
		 * then we force set X+1 channel (odd number) to differential.
		 *
		 * if user set X channel (even number) to single-end,
		 * then we force set X+1 channel (odd number) to single-end.
		 *
		 * Note!: only the X channel (even number) can be start differential channel */
		for (i = 0; i < (MAX_PHYSICAL_CHANNEL / 2); i++) {
			mask = 0x0001;
			mask <<= (2 * i);
			
			if (privdata->sd_mask & mask) {
				mask <<= 1;
				privdata->sd_mask |= mask;
			} else {
				mask <<= 1;
				privdata->sd_mask &= ~mask;
			}
		}		
	}
	break;
	default:
		kfree(tmp_buf);
		return -EFAULT;
	}
 	
	kfree(tmp_buf);

	return ret;
}

INT32S adv_get_err_code(adv_device *device, void * arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U error;


	
	error = adv_process_info_get_error(&privdata->ptr_process_info);
	
	if (copy_to_user((INT32U *) arg, &error, sizeof(INT32U))) {
		return -EFAULT;
	}

	return 0;
}
