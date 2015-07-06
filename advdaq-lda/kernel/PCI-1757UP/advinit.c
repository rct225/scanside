/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1757UP Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1757UP driver module and device. 
Version history
	02/13/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
#include <linux/module.h>

#include "PCI1757UP.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif


static struct pci_device_id serial_pci_tbl[] = 
{
     { 0x13fe, 0x1757,
       PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0
     },
     {0}, 

};

//#define PCI1757UP_DEBUG
/************************************************************************
 * Function:		int adv_opendevice
 * 
 * Description:  	The advdrv_core driver calls to open the device.
 * Parameters:	
	     device 	-Points to the device object
*************************************************************************/
int adv_opendevice( adv_device *device)
{
	INT32S ret;
	private_data  *privdata = NULL;
	
	_ADV_GET_DEVICE_PRIVDATA(device, privdata);
	ret = adv_process_info_add(&privdata->ptr_process_info, 1);
	
	return ret;
}

/************************************************************************
 * Function:		int adv_closedevice
 * 
 * Description:  	The advdrv_core driver calls to close the device.
 * Parameters:	
	    device 	-Points to the device object
*************************************************************************/
int  adv_closedevice( adv_device *device)
{
	INT32S ret = 0;
	private_data *privdata = NULL;
	
	_ADV_GET_DEVICE_PRIVDATA(device, privdata);
	ret = adv_process_info_remove(&privdata->ptr_process_info);
	
	return ret;
}

/************************************************************************
 * Function:	int adv_resetdevice
 * 
 * Description:	The advdrv_core driver calls to reset the device.
 * Parameters:	
	       device 	-Points to the device object
*************************************************************************/
int adv_resetdevice( adv_device *device )
{
     return SUCCESS;

}

/************************************************************************
 * Function:		int adv_mmap
 * 
 * Description:  	The advdrv_core driver calls to map the kernel memory to user
 * Parameters:	
              device 	-Points to the device object
 	      vma 	-vm_area_struct 
*************************************************************************/
int adv_mmap( adv_device *device, struct file *file,  struct vm_area_struct *vma )
{
     printk("This driver does not surpport mmap\n");
     return SUCCESS;
}


/**
 * adv_poll - relative poll or select system call
 */
INT32U adv_poll(adv_device *device, struct file * file, struct poll_table_struct *wait)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32U mask = 0;
	
	poll_wait(file, &privdata->event_wait, wait);
	
	if (adv_process_info_isset_event(&privdata->ptr_process_info))
		mask |= POLLIN | POLLRDNORM;
	
	return mask;
}

/************************************************************************
 * Function:		 int adv_read
 * 
 * Description:  	The advdrv_core driver calls to read datas from the device.
 * Parameters:	
              device 	-Points to the device object
 	      buf	-points to user buffer.
 	      nbytes 	-the number you want to read
*************************************************************************/
int adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  )
{
     return SUCCESS;
}

static int adv_set_8255mode(adv_device *device,USHORT usChannel, USHORT usDir)
{
     private_data *privdata = (private_data*)(device->private_data);
     ULONG  diPortCount = 3;
     USHORT usDirCtrl = 0;
     int i, iPort, iGroup;

     USHORT Reg_addr;
     
     if(usChannel >= diPortCount)
     {
	  return -EFAULT;
	  
     }
     if((usDir == 3) || (usDir == 4))
     {
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
      *     D7     HW/SW              0: SW     1: HW
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
     return SUCCESS;
     
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
static long adv_device_set_property(adv_device *device, USHORT nID, void*  pData, INT32S  lDataLength)
{
	private_data *privdata = (private_data*)(device->private_data);
	ULONG  diPortCount = 3;
	long   ret = 0;
	BYTE* pdata = NULL;
	ULONG i;
     
	if (pData == NULL)
		return -EFAULT;
     
	pdata = (BYTE *)kmalloc(lDataLength * sizeof(BYTE), GFP_KERNEL);
	if (!pdata) 
		return -ENOMEM;
     
	if (copy_from_user(pdata, pData, lDataLength * sizeof(BYTE))) {
		kfree(pdata);
		return -EFAULT;
	}
     
	switch (nID) {
	case CFG_DioChannelDirection:
		if ((privdata->ulHWorSW & 0x80) == 1) {
			/* HW */
			printk("Advantech : The Port's Direction Set By SW2!\n");
			ret = -EFAULT;
			break;
		}
	  
		if (lDataLength >= diPortCount * sizeof(INT8U)) {
			INT8U *pDir = (INT8U *)pdata;
			for (i = 0; i < diPortCount; i++, pDir++) {
				switch(*pDir){
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
					break;
				}
				if (ret) {
					break;
				}
			}	       
		}
	  
		break;
	case CFG_DiInterruptTriggerOnRisingEdge:
	 	if (lDataLength < diPortCount * sizeof(INT8U)) {
			ret =  -EFAULT;
			break;
		}

		if ((*(INT32U *)pdata & 0x010000)!= 0)
	      		privdata->usInterruptSrc |= 0x04;
		else 
			privdata->usInterruptSrc &= ~0x04;
		break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
		if (lDataLength < diPortCount * sizeof(INT8U)) {     
			ret = -EFAULT;
			break;
		}
	  
		if ((*(INT32U *)pdata & 0x010000) != 0)
			privdata->usInterruptSrc &= ~0x04;
		else 
			privdata->usInterruptSrc |= 0x04;
		
		break;

	case CFG_InterruptTriggerSource:
		if (lDataLength < sizeof(INT32U)) {
			ret = -EFAULT;
			break;
		}
	  
		if((*((INT32U *)pData) & 0xff) == 0x01)
			privdata->usIntSrcConfig = 0x01;
		else if ((*((INT32U *)pData) & 0xff) == 0x02)
			privdata->usIntSrcConfig = 0x02;
		else if ((*((INT32U *)pData) & 0xff) == 0x0) 
			privdata->usIntSrcConfig = 0x00;
		else {
			ret = -EFAULT;
		}

		break;
	default:
		ret = -EFAULT;
		break;
	}
     
	kfree(pdata);
     
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
static long adv_device_get_property(adv_device *device, USHORT nID, void *pData, INT32S *pDataLength) 
{
	private_data *privdata = (private_data*)(device->private_data);
	void*  cfgData = NULL;
	ULONG  DataLength;		/* user send */
	ULONG  sizeNeed= 0;
	ULONG  diPortCount = 3;
	INT32U *temp = NULL;
	int i;
	INT32S ret = 0;
	
	
	if ((pData == NULL) || (pDataLength == NULL))
		return -EFAULT;
	
	if (copy_from_user(&DataLength, pDataLength, sizeof(ULONG)))
		return -EFAULT;
		
     	switch (nID) {
	case CFG_BoardID:
		sizeNeed = sizeof(privdata->boardID); 
		cfgData = &privdata->boardID;
		break;

	case CFG_BaseAddress:
		sizeNeed = sizeof(privdata->iobase); 
		cfgData = &privdata->iobase;
		break;
	case CFG_Interrupt:		/* irq number */
		sizeNeed = sizeof(privdata->irq); 
		cfgData = &privdata->irq;
		break;
	case CFG_BusNumber:
		sizeNeed = sizeof(privdata->pci_bus);
		cfgData = &privdata->pci_bus;
		break;
	case CFG_SlotNumber:
		sizeNeed = sizeof(privdata->pci_slot); 
		cfgData = &privdata->pci_slot;
		break;
	case CFG_DiPortCount:
	case CFG_DoPortCount:
		// pData should be a LONG *
		sizeNeed = sizeof(long);
		cfgData = &diPortCount;
		break;
	case CFG_DiChannelCount:
	case CFG_DoChannelCount:
		sizeNeed = sizeof(INT32U);
		temp = kmalloc(sizeNeed, GFP_KERNEL);
		if (!temp) {
			ret = -ENOMEM;
			break;
		}
	
		*temp = diPortCount * 8;
		cfgData = temp;
		break;
       	
	case CFG_DiInterruptTriggerOnRisingEdge:
		sizeNeed = diPortCount * sizeof(INT8U);
		temp = kmalloc(sizeNeed, GFP_KERNEL);
		if (!temp) {
			ret = -ENOMEM;
			break;
		}
		
		*(INT32U *)temp = ((privdata->usInterruptSrc & 0x04) >> 2) << 16;
		cfgData = temp;
		break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
		sizeNeed = diPortCount * sizeof(INT8U);
		temp = kmalloc(sizeNeed, GFP_KERNEL);
		if (!temp) {
			ret = -ENOMEM;
			break;
		}
	
		*(INT32U *)temp = (((privdata->usInterruptSrc & 0x04)>>2) ? 0 : 1) <<16;
		cfgData = temp;
		break;
       
	case CFG_DioChannelDirection:
		sizeNeed = diPortCount * sizeof(INT8U);
		temp = kmalloc(sizeNeed, GFP_KERNEL);
		if (!temp) {
			ret = -ENOMEM;
			break;
		}
	
		for (i = 0; i < diPortCount; i++) {
			switch (privdata->usDioPortDir[i]) {
			case 0:
				*((INT8U *)temp + i) = DIO_ChannelDir_DI;
				break;
			case 1:
				*((INT8U *)temp + i) = DIO_ChannelDir_DO;
				break;
			case 3:
				*((INT8U *)temp + i) = DIO_ChannelDir_HighHalf_DO;
				break;
			case 4:
				*((INT8U *)temp + i) = DIO_ChannelDir_LowHalf_DO;
				break;
			default:
				*((INT8U *)temp + i) = DIO_ChannelDir_Invalid;
				break;
			}
	       
		}
	  	cfgData = temp;
		break;
      
	case CFG_InterruptTriggerSource:
		sizeNeed = sizeof(privdata->usIntSrcConfig);
		cfgData = &privdata->usIntSrcConfig;
		break;
	default:
		ret  = -EFAULT;
	}
	
	if(ret)
		goto exit;
	
	if (DataLength < sizeNeed) {
		ret = -EFAULT;
		goto exit;
	}
	
	if (copy_to_user(pDataLength, &sizeNeed, sizeof(ULONG))) {
		ret = -EFAULT;
		goto exit;
	}

	
	if (copy_to_user(pData, cfgData, sizeNeed)) {
		ret = -EFAULT;
		goto exit;
	}

 exit:	
	if(temp)
		kfree(temp);
	
	return ret;
}//DeviceGetParam

int adv_enable_event(adv_device *device,USHORT EventType,USHORT Enabled,USHORT Count)
{
	private_data *privdata = (private_data*)(device->private_data);

	if (Enabled) {
		if (Count <= 0)
			return -EFAULT;
		
		/* set the interrupt control register */
		if(privdata->usIntSrcConfig == 0x01) {
			privdata->usInterruptSrc &= 0xfc;
			privdata->usInterruptSrc |= 0x01;
		} else if (privdata->usIntSrcConfig == 0x02) {
			privdata->usInterruptSrc &= 0xfc;
			privdata->usInterruptSrc |= 0x02;
		    
		} else
			privdata->usInterruptSrc &= 0xfc;
	  
		if(EventType  == ADS_EVT_DI_INTERRUPT16) {
			if ((privdata->usIntSrcConfig != 0x01) 
			    && (privdata->usIntSrcConfig != 0x02)) {
				return -EFAULT;
			}
			adv_process_info_enable_event(&privdata->ptr_process_info, 0, Count);
		}
		
		advOutp(privdata, 32, privdata->usInterruptSrc);	  
	} else {
		if (EventType == ADS_EVT_DI_INTERRUPT16) {
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			if(!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 0))
				privdata->usInterruptSrc &= 0xFC;
		}
	  
		advOutp(privdata, 32, privdata->usInterruptSrc);	 
	  
	}

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
				*EventType = ADS_EVT_DI_INTERRUPT16;
				break;
			default:
				break;
				
			}
			
		}
	  
	} else {
		switch (*EventType) {
		case ADS_EVT_DI_INTERRUPT16:
			event_th = 0;
			break;
		default:
			event_th = -1;
			break;
		}
		if ((event_th >= 0) && (event_th <=1)) {
			*EventType = adv_process_info_check_special_event(p, event_th);
		} else {
			*EventType = 0;
		}
	}
     
	return 0;
}


/************************************************************************
* Function:		int adv_ioctrl
* 
* Description:  	The advdrv_core driver calls to handle the ioctl commands.
* Parameters:	
             device 	-Points to the device object
 	     cmd 	-io code.
 	     arg	-argument
*************************************************************************/
INT32S adv_ioctrl(adv_device *device, unsigned int cmd, unsigned long arg)
{
	PT_DeviceSetParam    lpDeviceSetParam;
	PT_DeviceGetParam    lpDeviceGetParam;
	PT_PortOperation     lpPortOperation;
	PT_EnableEvent       lpEnableEvent;
	PT_CheckEvent        lpCheckEvent;
     
	INT32S ret = 0;
	
     	switch (cmd){
	case ADV_DEVGETPROPERTY:
		if(copy_from_user(&lpDeviceGetParam,(void *)arg,sizeof(PT_DeviceGetParam))){
			ret = -EFAULT;
			break;
		}

		ret = adv_device_get_property(device, lpDeviceGetParam.nID,
					 lpDeviceGetParam.pData, lpDeviceGetParam.Length);
		break;
		
	case ADV_DEVSETPROPERTY:
		if(copy_from_user(&lpDeviceSetParam,(void *)arg,sizeof(PT_DeviceSetParam))){
			ret = -EFAULT;
			break;
			
		}
		ret = adv_device_set_property(device, lpDeviceSetParam.nID,
					 lpDeviceSetParam.pData, lpDeviceSetParam.Length);
		break;
		

	case ADV_PORTREAD:
		ret = copy_from_user(&lpPortOperation,(void *)arg,sizeof(PT_PortOperation));
		if(ret){
			ret = -EFAULT;
			break;
		}
		
		ret = adv_port_read(device, lpPortOperation.PortNumber,
				    lpPortOperation.DataBuffer, 
				    lpPortOperation.DataLength);

		break;

	case ADV_PORTWRITE:
		ret = copy_from_user(&lpPortOperation,(void *)arg,sizeof(PT_PortOperation));
		if(ret){
			ret = -EFAULT;
			break;
		}
		
		ret = adv_port_write(device, lpPortOperation.PortNumber,
				     lpPortOperation.DataBuffer,
				     lpPortOperation.DataLength);

		break;

		
	case ADV_ENABLE_EVENT: 
		if(copy_from_user(&lpEnableEvent,(void *)arg,sizeof(PT_EnableEvent)))
		{
			ret = -EFAULT;
			break;
		
		}

		ret = adv_enable_event(device,lpEnableEvent.EventType,
				       lpEnableEvent.Enabled,
				       lpEnableEvent.Count);	
		break;
		
	case CHECKEVENT:
	  	if(copy_from_user(&lpCheckEvent, (PT_CheckEvent *)arg,sizeof(PT_CheckEvent))){
			ret = -EFAULT;
			break;
		}
		adv_check_event(device, (INT32U *)&lpCheckEvent.EventType, 
				lpCheckEvent.Milliseconds);
	  
		if(copy_to_user((PT_CheckEvent *)arg,&lpCheckEvent,sizeof(PT_CheckEvent))){
			ret = -EFAULT;
		}
	  
		break;

	default:
		ret = -EFAULT;
		break;
	  
	}


	return ret;
}

/* PCI-1757UP fops */
adv_general_ops dev1757UP_fops =
{
     .opendevice	=adv_opendevice,
     .closedevice	=adv_closedevice,
     .read		=adv_read,
     .ioctrl		=adv_ioctrl,
     .resetdevice	=adv_resetdevice,
     .mmap		=adv_mmap,
     .poll		=adv_poll,
};

/* PCI-1757UP driver object */
adv_driver pci1757UP_driver =
{
     .driver_name		= "pci1757UP",
     .devfs_name		= "pci1757UP",
     .ops			= &dev1757UP_fops,
};

static irqreturn_t pci1757UP_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	private_data   *privdata;
	BYTE cmd;
    
	privdata=( private_data *)dev_id;
	
	spin_lock(&privdata->spinlock);
     
	cmd = advInp(privdata, 32);
	if(!(cmd & 0x08))		/* is pci1757UP interrupt */
	{
		spin_unlock(&privdata->spinlock);
		return IRQ_RETVAL(0);
	}
     
	adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	advOutp(privdata, 32, cmd|0x08);

	wake_up_interruptible(&privdata->event_wait);
	spin_unlock(&privdata->spinlock);
	return IRQ_RETVAL(1);

}

/************************************************************************
 * Function:		 int advdrv_init_one
 * 
 * Description:  	Pnp to initialize the device, and allocate resource for the device.
 * Parameters:	
	     dev	-Points to the pci_dev device
 	     ent 	-Points to pci_device_id including the device info.
*************************************************************************/
static int __devinit
advdrv_init_one(struct pci_dev *dev, const struct pci_device_id *ent)
{     
	private_data *privdata = NULL;
	adv_device *device = NULL;

	INT32S err = 0;
	if ((err = pci_enable_device(dev)) != 0)
	{
		KdPrint(KERN_ERR  ":pci_enable_device failed\n");
		return err;
	}
     
	device = (adv_device *)kmalloc(sizeof(adv_device), GFP_KERNEL);
	privdata = (private_data *)kmalloc(sizeof(private_data), GFP_KERNEL);
	if ((privdata == NULL) || (device == NULL)) {
		kfree(device);
		kfree(privdata);
		return  -EFAULT;
	}
   
	memset(device, 0, sizeof(adv_device));
	memset(privdata, 0, sizeof(private_data) );	

	//Initialize the private data in the device 
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->iobase = dev->resource[2].start;// & ~1UL;
	privdata->iolength = dev->resource[2].end -dev->resource[2].start;
	privdata->ioDMAbase = dev->resource[1].start& ~1UL;
	privdata->ioDMAlength = dev->resource[1].end - dev->resource[1].start;
	privdata->irq=dev->irq;
	privdata->sigpid=0;
	privdata->boardID = advInp(privdata, 36) & 0x0f;
	privdata->ulHWorSW = advInp(privdata,3);
	
	/* initiallize DIO direction */
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
	 *     D7     HW/SW              0: SW     1: HW
	 *     **********************************************/
	if((privdata->ulHWorSW & 0x80) == 1) /* HW */
	{
		if((privdata->ulHWorSW & 0x10) == 1) /* PA */
			privdata->usDioPortDir[0] = 0; /* input */
		else
			privdata->usDioPortDir[0] = 1; /* output */
	  
		if((privdata->ulHWorSW & 0x02) == 1)	/* PB */
			privdata->usDioPortDir[1] = 0; /* input */
		else
			privdata->usDioPortDir[1] = 1; /* output */
	  
		if((privdata->ulHWorSW & 0x01)== 1)
		{
			/* PCH and PCL are all Input */
			if((privdata->ulHWorSW & 0x08) == 1){
				privdata->usDioPortDir[2] = 0; 
			}else{
				/* PCH is output, PCL is Input */
				privdata->usDioPortDir[2] = 3; 
			}
			
	       
		}
		else{
			if((privdata->ulHWorSW & 0x08) == 1)
				privdata->usDioPortDir[2] = 4; /* PCH is input and PCL is output */
			else
				privdata->usDioPortDir[2] = 3; /* PCH and PCL is output */

		}
	}
     	  
	init_waitqueue_head(&privdata->event_wait);
	adv_process_info_header_init(&privdata->ptr_process_info);
	
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1757UP" ) == NULL){
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}
	err = request_irq(privdata->irq, pci1757UP_interrupt_handler, SA_SHIRQ, "adv1757UP", privdata); 
	if (err) {
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}
	
	advdrv_device_set_devname(device, "pci1757up");
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->boardID);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);

	device->config=0;
		

	spin_lock_init( &privdata->spinlock );
	pci_set_drvdata(dev, device);
        
	advdrv_add_device( &pci1757UP_driver, device ); 
	printk("Add a Advantech PCI%x device:boardID=%x;iobase=0x%lx;DMAbase=0x%lx;irq=%x;slot=%x.\n",dev->device,privdata->boardID,privdata->iobase, privdata->ioDMAbase, privdata->irq, privdata->pci_slot  );
	return SUCCESS;
	
}	
    
/************************************************************************
 * Function:		int advdrv_remove_one
 * 
 * Description:  	Pnp to remove a device, and free resource for the device.
 * Parameters:	
              dev 	-Points to the pci_dev device; 
 			 pci_get_drvdata(dev) points to the private data in adv_device.
*************************************************************************/
static void __devexit advdrv_remove_one(struct pci_dev *dev)
{

     private_data *privdata; 
     adv_device *device = NULL;
     privdata = NULL;
     KdPrint("Remove a PCI-%x device\n", dev->device );
     device = pci_get_drvdata(dev);
     privdata =(private_data*) device->private_data;
     free_irq(privdata->irq, privdata);
     release_region(privdata->iobase, privdata->iolength);  

     advdrv_remove_device(&pci1757UP_driver, device);
     pci_set_drvdata(dev, NULL);
     pci_disable_device(dev);

     kfree(privdata);
     kfree(device);
}
/************************************************************************
 * Function:	int advdrv_suspend_one
 * 
 * Description: Pnp to suspend a device
 * Parameters:	
          	dev 	-Points to the pci_dev device; 
 	        pci_get_drvdata(dev) points to the private data in adv_device.
*************************************************************************/
static int advdrv_suspend_one(struct pci_dev *dev, u32 state)
{
     return SUCCESS;
}
/************************************************************************
 * Function:		 int advdrv_resume_one
 * 
 * Description:  	Pnp to resume a device
 * Parameters:	
             dev 	-Points to the pci_dev device; 
 			pci_get_drvdata(dev) points to the private data in adv_device.
*************************************************************************/
static int advdrv_resume_one(struct pci_dev *dev)
{

     return SUCCESS;
}

struct pci_driver pci_driver = 
{
     name:		 "pci1757UP",
     probe:		 advdrv_init_one,
     remove:		 __devexit_p(advdrv_remove_one),
     suspend:	 advdrv_suspend_one,
     resume:		advdrv_resume_one,
     id_table:	 serial_pci_tbl,
};



/************************************************************************
 * static int __init pci1757UP_init(void)
 * 
 * Description:  The module initialize PCI-1757UP driver - Enumerate all PCI cards on the
 * 				 bus, register the driver in Advantech core driver.
 *************************************************************************/
static int __init pci1757UP_init(void)
{
	int error =0 ;
     	struct semaphore *sema = NULL;

	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if(sema == NULL)
	{
		return -ENOMEM;
	}
     
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1757UP_driver, sema);

	advdrv_register_driver( &pci1757UP_driver );
	error =  pci_module_init(&pci_driver) ;
	if( error != 0 )
	{
		advdrv_unregister_driver( &pci1757UP_driver );
		kfree(sema);
		return error;
	}
	return SUCCESS;
	
}
/************************************************************************
 * static int __init pci1757UP_init(void)
 * 
 * Description:  Exit  PCI-1757UP driver .
 *************************************************************************/
static void __exit pci1757UP_exit(void)
{
	struct semaphore *sema = NULL;
	pci_unregister_driver( &pci_driver );
	advdrv_unregister_driver( &pci1757UP_driver );
     
	_ADV_GET_DRIVER_SEMA(pci1757UP_driver, sema);
	kfree(sema);

}

module_init( pci1757UP_init );
module_exit( pci1757UP_exit );

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1757UP device driver module");
