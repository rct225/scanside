/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1730 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1730 driver module and device. 
Version history
	05/25/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
#include <linux/module.h>

#include "PCI1730.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif

//------------------------------------------------------------------
// Function: Set a configuration data item value.  
//           Each configuration item is distinguish by its name string.
//           Most Configuration item is used to hardware setting, and 
//           had better configured in configuration dialog box for 
//           default value. 
//           Using this function to change its setting if caller need to 
//           change configuration running time.
//           Not every item data is re-writable.
//
// Paramater: lDevHandle, IN, Device hansdle.
//            szParamName, IN, string of configuration item name.
//            pData, IN, Configuration data.
//            lLength, IN, Configuration data length.
//
// Return:    Error code.
//------------------------------------------------------------------
long adv_device_set_property(adv_device *device, INT16U nID, void*  pData, INT32U  lDataLength)
{
	private_data *privdata = (private_data*)(device->private_data);
	ULONG  diPortCount = 0;
	long   retVal = 0;
	BYTE* pdata = NULL;

	BYTE ctrlcode = 0;

	diPortCount = privdata->di_port_num;
	pdata = (BYTE *)kmalloc(lDataLength * sizeof(BYTE), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;
		
	if (copy_from_user(pdata, pData, lDataLength * sizeof(BYTE))) {
		kfree(pdata);
		return -EFAULT;
	}
     
	switch (nID) {
 	case CFG_DiInterruptTriggerOnRisingEdge:
		if (pData == NULL || lDataLength < diPortCount) {
			retVal =  -EFAULT;
			break;
		}
		
		if (diPortCount == 2) {
			privdata->event_trigger_mode = ~(*(INT16U *)pdata);
		} else if (diPortCount == 4) {
			privdata->event_trigger_mode = ~(*(INT32U *)pdata); 	
		} else {
			retVal = -EFAULT;
		}
		break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
		if (pData == NULL || lDataLength < diPortCount) {
			retVal = -EFAULT;
			break;
		}
		
		if (diPortCount == 2) {
			privdata->event_trigger_mode = (*(INT16U *)pdata);
		} else if (diPortCount == 4) {
			privdata->event_trigger_mode = (*(INT32U *)pdata);
		} else {
			retVal = -EFAULT;
		}
		break;
	  
	default:
		retVal = -EFAULT;
		break;
	
		  
	}
     
	switch(privdata->device_type)
	{
	case PCI1730:
	case PCI1733:
		privdata->event_trigger_mode &= 0x00030003;
		break;
	case PCI1734:
		privdata->event_trigger_mode = 0;
		break;
	case PCI1736:
		privdata->event_trigger_mode &= 0x00000003;
		break;
	}
     
     
	if(!adv_get_contorl_code(privdata->device_type, privdata->event_trigger_mode, &ctrlcode)){
		advOutp(privdata, 0x0c, ctrlcode);
	}
     
	kfree(pdata);
     
	return retVal;
     
     
}//DeviceSetParam



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
long adv_device_get_property(adv_device *device, INT16U nID, void*  pData, INT32U* pDataLength) 
{
	private_data *privdata = (private_data*)(device->private_data);
	void*  cfgData = NULL;
	ULONG  DataLength;		/* user send */
	ULONG  sizeNeed= 0;
	BYTE   data[24];
	long   retVal = 0;
	ULONG  diPortCount, doPortCount;
     
	diPortCount = privdata->di_port_num;
	doPortCount = privdata->do_port_num;
     
     
	if(copy_from_user(&DataLength, pDataLength, sizeof(INT32U)))
		return -EFAULT;
     
	memset(data, 0, sizeof(data));
     
	cfgData = data;
     
	switch ( nID ) {
	case CFG_BoardID:
		sizeNeed = sizeof(privdata->boardID);
		memcpy(data, &privdata->boardID, sizeNeed);
		break;

	case CFG_BaseAddress:
		sizeNeed = sizeof(privdata->iobase);
		memcpy(data, &privdata->iobase, sizeNeed);
		break;

	case CFG_Interrupt:		/* irq number */
		sizeNeed = sizeof(privdata->irq);
		memcpy(data, &privdata->irq, sizeNeed);
		break;
	case CFG_BusNumber:
		sizeNeed = sizeof(privdata->pci_bus);
		memcpy(data, &privdata->pci_bus, sizeNeed);
		break;
	case CFG_SlotNumber:
		sizeNeed = sizeof(privdata->pci_slot);
		memcpy(data, &privdata->pci_slot, sizeNeed);
	break;
     
	case CFG_DiChannelCount:
	case CFG_DoChannelCount:
		// pData should be a long *
		sizeNeed = sizeof(INT32U);
		*(INT32U *)data = (nID == CFG_DiChannelCount)?diPortCount*8:doPortCount*8;
	break;
	case CFG_DiPortCount:
	case CFG_DoPortCount:
		// pData should be a long *
		sizeNeed = sizeof(INT32U);
		*(INT32U *)data = (nID == CFG_DiPortCount)?diPortCount:doPortCount;
	break;
     

	case CFG_DiInterruptTriggerOnRisingEdge:
		if (privdata->device_type == PCI1734) {
			return -EFAULT;
		}
		sizeNeed = diPortCount*sizeof(BYTE);
		*(ULONG *)cfgData = (~privdata->event_trigger_mode) & 0x00030003;
	break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
		if (privdata->device_type == PCI1734) {
			return -EFAULT;
		}
		sizeNeed = diPortCount*sizeof(BYTE);
		*(ULONG *)cfgData = privdata->event_trigger_mode;
	break;
	default:
		return -EFAULT;
	}
     
	if(retVal)
		return retVal;
	if(copy_to_user(pDataLength, &sizeNeed, sizeof(ULONG)))
		return -EFAULT;
	
	if ( pData == NULL )
		return -EFAULT;
	
	if ( DataLength < sizeNeed )
		return -EFAULT;
	
	if(copy_to_user(pData, data, sizeNeed))
		return -EFAULT;
	
	return 0;

}//DeviceGetParam
