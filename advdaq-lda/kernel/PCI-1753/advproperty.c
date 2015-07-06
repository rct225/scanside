/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1718 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1718 driver module and device. 
Version history
	05/25/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */

#include <linux/module.h>

#include "PCI1753.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif



/**
 * Set8255Mode - Save the type of DIO port and set relating register
 * @privdata - Device handle
 * @port - port index
 * @mode - 0, 1, 3, 4
 */
INT32S set_8255_mode(private_data *privdata, INT32U port, INT32U mode)
{
	INT8U mode_reg = 0;
	INT32U i;
	INT32U igroup;
	INT32U iport;
	INT16U offset;
	
	if(port >= privdata->max_dio_group * 3)
		return -EFAULT;

	privdata->dio_port_dir[port] = mode;

	igroup = port / 3;

	for(i = igroup*3; i < ((igroup*3) + 3); i++)
	{
		/* Port A, B, or C? */
		iport = i % 3;
		
		/* if set port to input */
		if(privdata->dio_port_dir[i] == 0){
			if (iport == 0) {
				mode_reg  = mode_reg | 0x10;
			} else if (iport == 1) {
				mode_reg = mode_reg | 0x02;
			} else if (iport == 2) {
				mode_reg = mode_reg | 0x09;
			}
		} else if (privdata->dio_port_dir[i] == 1) {
		/* else if set port output */
			if (iport == 0) {
				mode_reg  = mode_reg & 0xef;
			} else if (iport == 1) {
				mode_reg = mode_reg & 0xfd;
			} else if (iport == 2) {
				mode_reg = mode_reg & 0xf6;
			}
		} else if(privdata->dio_port_dir[i] == 3){
		/* else if lower nipple is input and upper nipple is output */
			if(iport == 2) {
				mode_reg = mode_reg | 0x01;
				mode_reg = mode_reg & 0x7f;
			} else {
			  return -EINVAL;
			}
		} else if(privdata->dio_port_dir[i] == 4){
		/* else if low nipple is output and high nipple is input */
			if(iport == 2){
				mode_reg = mode_reg | 0x08;
				mode_reg = mode_reg | 0xfe;
			} else {
			  return -EINVAL;
			}
		}
	}
	
	if(igroup < 4)
		offset = (igroup * 4) + 3;
	else
		/* offset = ((igroup-4) * 4) + 3 + 32; */
		offset = (igroup * 4) + 19; 
	
	advOutp(privdata, offset, mode_reg);
	
	return 0;
}


INT32S adv_device_set_property(adv_device *device, USHORT nID, void *ubuf, INT32U ulen)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT32S  ret = 0;
	BYTE *kbuf = NULL;
	INT32U dio_port_count;
	INT32U i;
	
	dio_port_count = privdata->max_dio_group * 3;
	

	if (ubuf == NULL) {
		return -EFAULT;
	}
	
	kbuf = (BYTE *)kmalloc(ulen * sizeof(BYTE), GFP_KERNEL);
	if (!kbuf) {
		return -ENOMEM;
	}
	
	if (copy_from_user(kbuf, ubuf, ulen * sizeof(BYTE))) {
		kfree(kbuf);
		return -EFAULT;
	}

	switch (nID) {
	case CFG_DioChannelDirection:
		if (ulen < dio_port_count * sizeof(INT8U)) {
			ret = -EFAULT;
			break;
		}
		for (i = 0; i < dio_port_count; i++) {
			switch (*((INT8U *)kbuf +i)) {
			case DIO_ChannelDir_DI:
				ret = set_8255_mode(privdata, i, 0);
				break;
			case DIO_ChannelDir_DO:
				ret = set_8255_mode(privdata, i, 1);
				break;
			case DIO_ChannelDir_HighHalf_DO:
				ret = set_8255_mode(privdata, i, 3);
				break;
			case DIO_ChannelDir_LowHalf_DO:
				ret = set_8255_mode(privdata, i, 4);
				break;
			default:
				ret = -EFAULT;
			}
				
			if (ret) {
				break;
			}
		}
	      
		break;
	case CFG_InterruptTriggerSource:
		if (ulen < privdata->max_dio_group * sizeof(INT32U)) {
			ret = -EFAULT;
			break;
		}
		
		for(i = 0; i < privdata->max_dio_group; i++) {
			switch(* ((INT32U *)kbuf + i)) {
			case 1:
				privdata->int_src_config[i] = 1;
				privdata->int_ctrl_reg[i] = (privdata->int_ctrl_reg[i] & 0xc) | 0x01;
				break;
			case 2:
				privdata->int_src_config[i] = 2;
				privdata->int_ctrl_reg[i] = (privdata->int_ctrl_reg[i] & 0xc) | 0x02;
				break;
			case 0:
			case 3:
				privdata->int_src_config[i] = 0;
				privdata->int_ctrl_reg[i] = (privdata->int_ctrl_reg[i] & 0xc) | 0x00;
				break;
			default:
				ret = -EFAULT;
				break;
			}
		}
		
		break;
  
	case CFG_DiInterruptTriggerOnRisingEdge:
		if (ulen < dio_port_count * sizeof(INT8U)) {
			ret = -EFAULT;
			break;
		}
		
		for (i = 0; i < privdata->max_dio_group; i++) {
			if (*((INT8U *)kbuf + 2 + 3*i) & 0x1) {
				privdata->int_ctrl_reg[i] |= 0x04;
				privdata->int_trigger_signal[2+3*i] = 1;
			} else {
				privdata->int_ctrl_reg[i] &= (~0x04);
				privdata->int_trigger_signal[2+3*i] = 0;
			}
			
		}

		break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
		if (ulen < dio_port_count * sizeof(INT8U)) {
			ret = -EFAULT;
			break;
		}

		for(i = 0; i < privdata->max_dio_group; i++) {
			if(*((INT8U *)kbuf + 2 + 3*i) & 0x1) {
				privdata->int_ctrl_reg[i] &= (~0x04);
				privdata->int_trigger_signal[2+3*i] = 0;
			} else {
				privdata->int_ctrl_reg[i] |= 0x04;
				privdata->int_trigger_signal[2+3*i] = 1;
			}
			
		}
		break;
		 
	case CFG_DiStatusChangeEnabledChannel:
		if (ulen < sizeof(DiEnableChannel)) {
			ret = -EFAULT;
			break;
		}
		for (i = 0; (i < dio_port_count/12) && (i < ulen/sizeof(DiEnableChannel)); i++) {
			if (((DiEnableChannel *)kbuf + i)->port == 1) {
				privdata->cDiStatusChangeEnabledChannel[0] = ((DiEnableChannel *)kbuf + i)->status;
			} else if  (((DiEnableChannel *)kbuf + i)->port == 13) {
				privdata->cDiStatusChangeEnabledChannel[1] = ((DiEnableChannel *)kbuf + i)->status;
			} else {
				ret = -EINVAL;
				break;
			}
		}
		break;
	
	case CFG_DiPatternMatchEnabledChannel:
		if (ulen < sizeof(DiEnableChannel)) {
			ret = -EFAULT;
			break;
		}

		for (i = 0; (i < dio_port_count/12) && (i < ulen/sizeof(DiEnableChannel)); i++) {
			if (((DiEnableChannel *)kbuf + i)->port == 0) {
				privdata->cDiPatternMatchEnabledChannel[0] = ((DiEnableChannel *)kbuf + i)->status;
			} else if  (((DiEnableChannel *)kbuf + i)->port == 12) {
				privdata->cDiPatternMatchEnabledChannel[1] = ((DiEnableChannel *)kbuf + i)->status;
			} else {
				ret = -EINVAL;
				break;
			}
		}
		
		break;
	
	case CFG_DiPatternMatchValue:
		if (ulen < sizeof(DiPortValue)) {
			ret = -EFAULT;
			break;
		}

		for (i = 0; (i < dio_port_count/12) && (i < ulen/sizeof(DiPortValue)); i++) {
			if (((DiPortValue *)kbuf + i)->port == 0) {
				privdata->cDiPatternMatchValue[0] = ((DiPortValue *)kbuf + i)->value;
			} else if  (((DiPortValue *)kbuf + i)->port == 12) {
				privdata->cDiPatternMatchValue[1] = ((DiPortValue *)kbuf + i)->value;
			} else {
				ret = -EINVAL;
				break;
			}
		}
		
		break;

	default:
		ret = -EFAULT;
		break;
	  
	}
     
	kfree(kbuf);
     
	return ret;
}



//------------------------------------------------------------------
// Function: Get a configuration data item value.  
//           Each configuration item is distinguish by its name string.
//
// Paramater: lDevHandle, IN, Device hansdle.
//            szParamName, IN, string of configuration item name.
//            pData, IN, Configuration data.
//            lLength, IN, Configuration data length.
//
// Return:    Error code.
//------------------------------------------------------------------
INT32S adv_device_get_property(adv_device *device, USHORT nID, void *ubuf,INT32U *ulen) 
{
	private_data *privdata = (private_data*)(device->private_data);
	void*  kbuf = NULL;
	INT32U len;		/* user send */
	INT32U size_need= 0;
	INT32U dio_port_count;
	INT32U temp;
	BYTE   data[48];
	INT32U i;
	
	if ((ubuf == NULL) || (ulen == NULL)) {
		return -EFAULT;
	}
	
	if (copy_from_user(&len, ulen, sizeof(INT32U))) {
		return -EFAULT;
	}

      	dio_port_count = privdata->max_dio_group * 3;

	switch (nID) {
	case CFG_BoardID:
		size_need = sizeof(privdata->boardID); 
		kbuf = &privdata->boardID;
		break;

	case CFG_BaseAddress:
		size_need = sizeof(privdata->iobase); 
		kbuf = &privdata->iobase;
		break;
	case CFG_Interrupt:		/* irq number */
		size_need = sizeof(privdata->irq); 
		kbuf = &privdata->irq;
		break;
	case CFG_BusNumber:
		size_need = sizeof(privdata->pci_bus);
		kbuf = &privdata->pci_bus;
		break;
	case CFG_SlotNumber:
		size_need = sizeof(privdata->pci_slot); 
		kbuf = &privdata->pci_slot;
		break;
	
	case CFG_DiChannelCount:
	case CFG_DoChannelCount:
		size_need = sizeof(INT32U);
		temp = dio_port_count * 8;
		kbuf = &temp;
		break;
	case CFG_DiPortCount:
	case CFG_DoPortCount:
		size_need = sizeof(INT32U);
		kbuf = &dio_port_count;
		break;

	case CFG_DioChannelDirection:
		size_need = dio_port_count * sizeof(INT8U);
		for (i = 0; i < dio_port_count; i++) {
			switch(privdata->dio_port_dir[i]) {
			case 0:
				data[i] = DIO_ChannelDir_DI;
				break;
			case 1:
				data[i] = DIO_ChannelDir_DO;
				break;
			case 3:
				data[i] = DIO_ChannelDir_HighHalf_DO;
				break;
			case 4:
		    		data[i] = DIO_ChannelDir_LowHalf_DO;
				break;
			}
	       
		}
		kbuf = data;
		break;
	case CFG_InterruptTriggerSource:
		size_need = privdata->max_dio_group * sizeof(INT32U);
		kbuf = privdata->int_src_config;		
		break;

	case CFG_DiInterruptTriggerOnRisingEdge:
		size_need = dio_port_count *sizeof(INT8U);
		kbuf = privdata->int_trigger_signal;
		break;
	case CFG_DiInterruptTriggerOnFallingEdge:
		size_need = dio_port_count *sizeof(INT8U);
		for (i = 0; i < dio_port_count; i++) {
			if ((i + 1)%3) {
				data[i] =(~privdata->int_trigger_signal[i]) & 0x01;
			} else {
				data[i] = 0;
			}
		}
		
		kbuf = data;
		break;
	case CFG_DiStatusChangeEnabledChannel:
		size_need = sizeof(DiEnableChannel);
		((DiEnableChannel *)data)->port = 1;
		((DiEnableChannel *)data)->status = privdata->cDiStatusChangeEnabledChannel[0];
		if(privdata->us_exp_board == 1) {
			size_need = 2 * sizeof(DiEnableChannel);
			((DiEnableChannel *)data + 1)->port = 13;
			((DiEnableChannel *)data + 1)->status = privdata->cDiStatusChangeEnabledChannel[1];
		}

		kbuf = data;
		break;

	case CFG_DiPatternMatchEnabledChannel:
		size_need = sizeof(DiEnableChannel);
		if(privdata->us_exp_board == 1) {
			size_need = 2 * sizeof(DiEnableChannel);
		}
		((DiEnableChannel *)data)->port = 0;
		((DiEnableChannel *)data)->status = privdata->cDiPatternMatchEnabledChannel[0];
		((DiEnableChannel *)data + 1)->port = 12;
		((DiEnableChannel *)data + 1)->status = privdata->cDiPatternMatchEnabledChannel[1];

		kbuf = data;
		break;
	
	case CFG_DiPatternMatchValue:
		size_need = sizeof(DiPortValue);
		if(privdata->us_exp_board == 1) {
			size_need = 2 * sizeof(DiPortValue);
		}
		((DiPortValue *)data)->port = 0;
		((DiPortValue *)data)->value = privdata->cDiPatternMatchValue[0];
		((DiPortValue *)data + 1)->port = 12;
		((DiPortValue *)data + 1)->value = privdata->cDiPatternMatchValue[1];

		kbuf = data;
		break;
		
	default:
		return -EFAULT;
	}

	if (len < size_need) {
		return -EFAULT;
	}

	if (copy_to_user(ulen, &size_need, sizeof(INT32U))) {
		return -EFAULT;
	}

	if (copy_to_user(ubuf, kbuf, size_need)) {
		return -EFAULT;
	}
	    
	return 0;
}
