/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1758U Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	advproperty.c
 * Abstract:                                                   
 * 	This file contains routines for device function.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	09/12/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1758U.h"
 
/**
 * adv_set_user_buffer - we get user buffer through this safe way
 *               
 * @buf: Points to the user buffer
 * @len: user buffer len
 * @size: size of data that will be transfered to the user buffer
 * @item: the data to be copy to the user buffer
 */
static INT32S adv_set_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	if (*len < size) {
		return InvalidInputParam;
	}

	if (copy_to_user(buf, item, size)) {			
		return -EFAULT;
	}

	*len = size;

	return 0;
}

/**
 * adv_get_user_buffer - we get user buffer through this safe way
 *               
 * @buf: Points to the user buffer
 * @len: user buffer len
 * @size: size of data that will be received from the user buffer
 * @item: the data to be received
 */
static INT32S adv_get_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	if (*len < size) {
		return -EINVAL;
	}
	
	if (copy_from_user(item, buf, size)) {
		return -EFAULT;
	}

	return 0;
}


INT32S adv_dev_get_property(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceGetParam dev_get_param;
	INT32U length, actual_len = 0, tmp = 0;
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
		return InvalidInputParam;
	}

	switch (dev_get_param.nID) {
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
					  (void *) &(privdata->irq));
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
	case CFG_DioPortType:
		tmp = (INT32U) DIO_PortType_SeperateDIO;
		actual_len = sizeof(tmp);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &tmp);
		break;
	case CFG_DiChannelCount:
		tmp = (INT32U) MAX_DI_CHANNEL;
		actual_len = sizeof(tmp);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &tmp);
		break;
	case CFG_DiDataWidth:
		tmp = (INT32U) DI_DataWidth_Byte;
		actual_len = sizeof(tmp);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &tmp);
		break;
	case CFG_DiPortCount:
		tmp = (INT32U) MAX_DI_PORT;
		actual_len = sizeof(tmp);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &tmp);
		break;
	case CFG_DoDataWidth:
		tmp = (INT32U) DO_DataWidth_Byte;
		actual_len = sizeof(tmp);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &tmp);
		break;
	case CFG_DoChannelCount:
		tmp = (INT32U) MAX_DO_CHANNEL;
		actual_len = sizeof(tmp);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &tmp);
		break;
	case CFG_DoPortCount:
		tmp = (INT32U) MAX_DO_PORT;
		actual_len = sizeof(tmp);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &tmp);
		break;
	case CFG_DiInterruptSupportedChannel:
		tmp = (INT32U) 0xff;
		actual_len = sizeof(tmp);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &tmp);
		break;
	case CFG_DiInterruptTriggerOnRisingEdge:
		actual_len = sizeof(privdata->int_rising_trig);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->int_rising_trig));
		break;
	case CFG_DiInterruptTriggerOnFallingEdge:
		actual_len = sizeof(privdata->int_falling_trig);
		ret = adv_set_user_buffer(buffer, &length, actual_len,
					  (void *) &(privdata->int_falling_trig));
		break;
	default:
		return -EINVAL;
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
	INT32U length, actual_len = 0;
	void *buffer, *tmp_buf;
	INT16U max_port_num;
	INT16U filter_reg[3];
	INT16U i;
	INT32S ret;



	if (copy_from_user(&dev_set_param, arg, sizeof(PT_DeviceSetParam))) {
		return -EFAULT;
	}
	

	length = dev_set_param.Length;
	buffer = dev_set_param.pData;
	memset(filter_reg, 0, sizeof(INT16U) * 3);
	
	  
	tmp_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!tmp_buf) {
		return MemoryAllocateFailed;
	}
	memset(tmp_buf, 0, PAGE_SIZE);


	switch (privdata->device_type) {
	case PCI1758UDI:
		actual_len = sizeof(INT8U) * MAX_DI_PORT;
		max_port_num = MAX_DI_PORT;

		filter_reg[0] = 0x28;
		filter_reg[1] = 0x30;
		filter_reg[2] = 0x32;
		
		break;
	case PCI1758UDO:
		actual_len = sizeof(INT8U) * MAX_DO_PORT;
		max_port_num = MAX_DO_PORT;

		break;
	case PCI1758UDIO:
		actual_len = sizeof(INT8U) * MAX_DIO_PORT;
		max_port_num = MAX_DIO_PORT;

		filter_reg[0] = 0x40;
		filter_reg[1] = 0x52;
		filter_reg[2] = 0x54;

		break;
	default:
		kfree(tmp_buf);
		return BoardIDNotSupported;
	}

	switch (dev_set_param.nID) {
	case CFG_DiInterruptTriggerOnRisingEdge:
		if ((privdata->device_type != PCI1758UDO)
		    && (privdata->device_type != PCI1758UDIO)) {
			kfree(tmp_buf);
			return -EINVAL;
		}

		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		for (i = 0; i < max_port_num; i++) {
			privdata->int_rising_trig[i] = ((INT8U *) tmp_buf)[i];
		}

		break;
	case CFG_DiInterruptTriggerOnFallingEdge:
		if ((privdata->device_type != PCI1758UDI)
		    && (privdata->device_type != PCI1758UDIO)) {
			kfree(tmp_buf);
			return -EINVAL;
		}

		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		for (i = 0; i < max_port_num; i++) {
			privdata->int_falling_trig[i] = ((INT8U *) tmp_buf)[i];
		}
		
		break;
	case CFG_DoPresetValue:
		if ((privdata->device_type != PCI1758UDO)
		    && (privdata->device_type != PCI1758UDIO)) {
			kfree(tmp_buf);			
			return -EINVAL;
		}

		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		
		for (i = 0; i < max_port_num; i++) {
			privdata->do_preset_value[i] = ((INT8U *) tmp_buf)[i];
		}

		break;
	case CFG_DoWatchdogValue:
		if ((privdata->device_type != PCI1758UDO)
		    && (privdata->device_type != PCI1758UDIO)) {
			kfree(tmp_buf);
			return -EINVAL;
		}

		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		for (i = 0; i < max_port_num; i++) {
			privdata->do_watchdog_value[i] = ((INT8U *) tmp_buf)[i];
		}

		break;
	case CFG_WatchdogCounter:
		if ((privdata->device_type != PCI1758UDO)
		    && (privdata->device_type != PCI1758UDIO)) {
			kfree(tmp_buf);
			return -EINVAL;
		}

		actual_len = sizeof(privdata->watchdog_counter);
		
		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		privdata->watchdog_counter = *((INT32U *) tmp_buf);

		break;
	case CFG_DiFilterEnableForAll:
		if ((privdata->device_type != PCI1758UDI)
		    && (privdata->device_type != PCI1758UDIO)) {
			kfree(tmp_buf);
			return -EINVAL;
		}

		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		for (i = 0; i < max_port_num / 2; i++) {
			privdata->di_filter_enable[i] = ((INT16U *) tmp_buf)[i];
			advOutpw(privdata, filter_reg[0] + (i * 2), ((INT16U *) tmp_buf)[i]);
		}

		break;
	case CFG_DiFilterIntervalCounter:
		if ((privdata->device_type != PCI1758UDI)
		    && (privdata->device_type != PCI1758UDIO)) {
			kfree(tmp_buf);
			return -EINVAL;
		}

		ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);
		privdata->di_filter_interval = *((INT32U *) tmp_buf);

		advOutpw(privdata, filter_reg[1],
			 (INT16U) (privdata->di_filter_interval & 0xffff));

		advOutpw(privdata, filter_reg[2],
			 (INT16U) ((privdata->di_filter_interval >> 16) & 0xffff));

		break;
	default:
		/* for digital filter enable single port 0~7 */
		if (dev_set_param.nID >= CFG_DiFilterEnablePort0
		    && dev_set_param.nID <= CFG_DiFilterEnablePort15) {
			if ((privdata->device_type != PCI1758UDI)
			    && (privdata->device_type != PCI1758UDIO)) {
				kfree(tmp_buf);
				return -EINVAL;
			}
			ret = adv_get_user_buffer(buffer, &length, actual_len, tmp_buf);

			i = dev_set_param.nID - CFG_DiFilterEnablePort0;
			privdata->di_filter_enable[i] = *((INT8U *) tmp_buf);

			advOutp(privdata, filter_reg[0] + i, *((INT8U *) tmp_buf));
			
			break;
		}
		kfree(tmp_buf);
		return -EINVAL;
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
