#include "PCI1751.h"
static INT32S adv_set_8255mode(adv_device *device, USHORT usChannel, USHORT usDir)
{
	private_data *privdata = (private_data*)(device->private_data);
	ULONG  diPortCount;
	USHORT usDirCtrl = 0;
	INT32S i, iPort, iGroup;
	INT16U Reg_addr;
	
	diPortCount = privdata->dio_group_num * 3;
	if(usChannel >= diPortCount){
		return -EFAULT;
	}
	if((usDir == 3) || (usDir == 4)){
		if((usChannel%3)!= 2)
			return -EFAULT;
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
    
	for(i = iGroup*3; i < iGroup*3 +3; i++)
	{
		iPort = i%3;
		if(privdata->usDioPortDir[i] == 0) /* DI */
		{
			if(iPort == 0) usDirCtrl |= 0x10;
			else if(iPort == 1) usDirCtrl |= 0x02;
			else if(iPort == 2) usDirCtrl |= 0x09;
	       
		}
		else if(privdata->usDioPortDir[i] == 1) /* DO */
		{
			if(iPort == 0) usDirCtrl &= 0xef;
			else if(iPort == 1) usDirCtrl &= 0xfd;
			else if(iPort == 2) usDirCtrl &= 0xf6;
	   
		}
		else if(privdata->usDioPortDir[i] == 3) /* low nipple is input and
							 * high nipple is output*/
		{
			if(iPort == 2)
			{
				usDirCtrl |= 0x01;
				usDirCtrl &= 0xf7;
			}
	       
		}
		else if(privdata->usDioPortDir[i] == 4) /* low nipple is output and
							 * high nipple is input*/
		{
			if(iPort == 2)
			{
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
INT32S adv_device_set_property(adv_device *device, INT16U nID, void*  pData, INT32S  lDataLength)
{
	private_data *privdata = (private_data*)(device->private_data);
	ULONG  diPortCount ;
	INT32S   ret = 0;
	BYTE* pdata = NULL;
	ULONG i;
	INT32S retVal = 0;
	
	if (pData == NULL) {
		return -EFAULT;
	}
	
	diPortCount = privdata->dio_group_num * 3;
	pdata = (BYTE *)kmalloc(lDataLength * sizeof(BYTE), GFP_KERNEL);
	if (!pdata) {
		return -ENOMEM;
	}

	ret = copy_from_user(pdata, pData, lDataLength * sizeof(BYTE));
	if (ret < 0) {
		kfree(pdata);
		return ret;
	}

	switch (nID) {
	case CFG_CascadeMode:
		if (lDataLength >= sizeof(INT32U)) {
			if(*pdata != 0)
				privdata->IsCascadeMode = 1;
			else 
				privdata->IsCascadeMode = 0;
		} else {
			kfree(pdata);
			return -EFAULT;
		}
	  
		break;
     
	case CFG_DioChannelDirection:
		if(lDataLength >= diPortCount * sizeof(INT8U)) {
			for (i = 0; i < diPortCount; i++) {
				switch (*((INT8U *)pdata +i)) {
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
					ret = -EFAULT;
				}
				
				if (ret) {
					break;
				}
			}
	       
		}
	  
		break;
     
	case CFG_DiInterruptTriggerOnRisingEdge:
		if (pData == NULL || lDataLength < diPortCount*sizeof(INT8U)) {
			ret =  -EFAULT;
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
		
		if ((privdata->device_type == MIC3751)
		    && ((*((INT8U *)pdata + 8) & 0x01) != 0)) {
			privdata->usInterruptSrc |= 0x400;
		}
		
		break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
		if(pData == NULL || lDataLength < diPortCount * sizeof(INT8U)) {     
			ret = -EFAULT;
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
		
		if ((privdata->device_type == MIC3751)
		    && ((*((INT8U *)pdata + 8) & 0x01) != 0)) {
			privdata->usInterruptSrc &= ~0x400;
		}
		break;
		  
	case CFG_InterruptTriggerSource:
		if(pData == NULL || lDataLength < privdata->dio_group_num * sizeof(INT32U)){
			ret = -EFAULT;
			break;
		}
		
		for (i = 0; i < privdata->dio_group_num; i++) {
			if (*((INT32U *)pdata + i) < 4) {
				privdata->usIntSrcConfig[i] = *((INT32U *)pdata + i);
			} else {
				ret = -EFAULT;
				break;
			}
	       	  
		}

		break;

	default:		
		ret = -EFAULT;
		break;
	}

	kfree(pdata);
	return ret;
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
INT32S adv_device_get_property( 
     adv_device *device, 
     INT16U nID, 
     void*  pData,
     INT32S* pDataLength) 
{
	private_data *privdata = (private_data*)(device->private_data);

	void*  cfgData = NULL;
	ULONG  DataLength;		/* user send */
	ULONG  sizeNeed= 0;
	BYTE   data[48];
	ULONG  diPortCount = privdata->dio_group_num * 3;
	int i;
	
	if ( pData == NULL) {
		return -EFAULT;
	}

     
	if(copy_from_user(&DataLength, pDataLength, sizeof(INT32U))){
		return -EFAULT;
	}

	memset(data, 0, sizeof(data));
     
	cfgData = data;
     
	switch ( nID )
	{
	case CFG_BoardID:
	{
		sizeNeed = sizeof(privdata->boardID); 
		memcpy(data, &privdata->boardID, sizeNeed);
	}
	break;

	case CFG_BaseAddress:
	{
		sizeNeed = sizeof(privdata->iobase); 
		memcpy(data, &privdata->iobase, sizeNeed);
	}
	break;
	case CFG_Interrupt:		/* irq number */
	{
		sizeNeed = sizeof(privdata->irq); 
		memcpy(data, &privdata->irq, sizeNeed);
	}
	break;
	case CFG_BusNumber:
	{
		sizeNeed = sizeof(privdata->pci_bus);
		memcpy(data, &privdata->pci_bus, sizeNeed);
	}
	break;
	case CFG_SlotNumber:
	{
		sizeNeed = sizeof(privdata->pci_slot); 
		memcpy(data, &privdata->pci_slot, sizeNeed);
	}
	break;
	case CFG_CascadeMode:
	{
		sizeNeed = sizeof(privdata->IsCascadeMode);
		memcpy(data, &privdata->IsCascadeMode, sizeNeed);
	}
	break;
     
	case CFG_DiChannelCount:
	case CFG_DoChannelCount:
	{
		// pData should be a long *
		sizeNeed = sizeof(INT32U);
		*(INT32U *)data = diPortCount * 8;
	}
	break;
	case CFG_DiPortCount:
	case CFG_DoPortCount:
	{
		// pData should be a long *
		sizeNeed = sizeof(INT32U);
		*(INT32U *)data = diPortCount;
	}
	break;

	case CFG_DiInterruptTriggerOnRisingEdge:
	{
		sizeNeed = diPortCount * sizeof(BYTE);
		if(privdata->usInterruptSrc & 0x04)
			*((BYTE *)cfgData+2) = 0x01;
		if(privdata->usInterruptSrc & 0x40)
			*((BYTE *)cfgData+5) = 0x01;
		if(privdata->usInterruptSrc & 0x400)
			*((BYTE *)cfgData+8) = 0x01;
		
	}
     
	break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
	{
		sizeNeed = diPortCount * sizeof(BYTE);
		if(!(privdata->usInterruptSrc & 0x04))
			*((BYTE *)cfgData+2) = 0x01;
		if(!(privdata->usInterruptSrc & 0x40))
			*((BYTE *)cfgData+5) = 0x01;
		if(!(privdata->usInterruptSrc & 0x400))
			*((BYTE *)cfgData+8) = 0x01;
	}
     
	break;
       

	case CFG_DioChannelDirection:
		sizeNeed = diPortCount * sizeof(INT8U);
		for (i = 0; i < diPortCount; i++) {
			switch(privdata->usDioPortDir[i])
			{
			case 0:
				*(INT8U *)cfgData++ = DIO_ChannelDir_DI;
				break;
			case 1:
				*(INT8U *)cfgData++ = DIO_ChannelDir_DO;
				break;
			case 3:
				*(INT8U *)cfgData++ = DIO_ChannelDir_HighHalf_DO;
				break;
			case 4:
		    		*(INT8U *)cfgData++ = DIO_ChannelDir_LowHalf_DO;
				break;
			}
	       
		}
		break;

	case CFG_InterruptTriggerSource:
		sizeNeed = privdata->dio_group_num * sizeof(INT32U);
		memcpy(data, privdata->usIntSrcConfig, sizeNeed);
		break;
	  
	default:
		return -EFAULT;
	}


	if(copy_to_user(pDataLength, &sizeNeed, sizeof(INT32U))) {
		return -EFAULT;
	}

	if ( DataLength < sizeNeed )
	{
		return -EFAULT;
	}

	if(copy_to_user(pData, data, sizeNeed)){
		return -EFAULT;
	}
	    
	return 0;
}//DeviceGetParam
