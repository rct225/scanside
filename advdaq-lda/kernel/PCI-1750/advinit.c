/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1750 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1750 driver module and device. 
Version history
	01/11/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
#include <linux/module.h>

#include "PCI1750.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] = 
{
     { 0x13fe, 0x1750,
       PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0
     },
     {0}, 

};


/************************************************************************
 * Function:		int adv_opendevice
 * 
 * Description:  	The advdrv_core driver calls to open the device.
 * Parameters:	
	     device 	-Points to the device object
*************************************************************************/
INT32S adv_opendevice( adv_device *device)
{
	INT32S ret;
	private_data *privdata = NULL;
	
	_ADV_GET_DEVICE_PRIVDATA(device, privdata);
	ret = adv_process_info_add(&privdata->ptr_process_info, 4);
	
	return ret;
}

/************************************************************************
 * Function:		int adv_closedevice
 * 
 * Description:  	The advdrv_core driver calls to close the device.
 * Parameters:	
	    device 	-Points to the device object
*************************************************************************/
INT32S adv_closedevice( adv_device *device)
{
	INT32S ret;
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
INT32S adv_resetdevice( adv_device *device )
{
     return 0;

}

/************************************************************************
 * Function:		int adv_mmap
 * 
 * Description:  	The advdrv_core driver calls to map the kernel memory to user
 * Parameters:	
              device 	-Points to the device object
 	      vma 	-vm_area_struct 
*************************************************************************/
INT32S adv_mmap( adv_device *device, struct file *file, struct vm_area_struct *vma )
{
     printk("This driver does not surpport mmap\n");
     return 0;
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
static long adv_device_set_property(adv_device *device, USHORT nID, void *pData, ULONG  lDataLength)
{
  private_data *privdata = (private_data*)(device->private_data);
  ULONG  diPortCount =2;
  long   ret = 0;
  BYTE* pdata = NULL;
  INT32U i; 
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
  case CFG_DiInterruptTriggerOnRisingEdge:
    if ( (pdata == NULL) || (lDataLength < diPortCount*sizeof(INT8U))){
      ret =  -EFAULT;
      break;
    }
	
    if ((*(INT16U *)pdata & 0x0001) != 0 ) {
      privdata->usInterruptSrc |= 0x04;
    } else {
      privdata->usInterruptSrc &= ~0x04;
    }
		
    if ((*(INT16U *)pdata & 0x0100) != 0) {
      privdata->usInterruptSrc |= 0x40;
    } else {
      privdata->usInterruptSrc &= ~0x40;
    }
		
    break;
	  
  case CFG_DiInterruptTriggerOnFallingEdge:
    if ((pdata == NULL) || (lDataLength < diPortCount*sizeof(INT8U))) {     
      ret = -EFAULT;
      break;
    }
	  
    if ((*(INT16U *)pdata & 0x0001)!= 0) {
      privdata->usInterruptSrc &= ~0x04;
    } else {
      privdata->usInterruptSrc |= 0x04;
    }
		
    if ((*(INT16U *)pdata & 0x0100) != 0) {
      privdata->usInterruptSrc &= ~0x40;
    } else {
      privdata->usInterruptSrc |= 0x40;
    }
		
    break;

  case CFG_InterruptTriggerSource:
    if ((pdata == NULL) || (lDataLength < diPortCount * sizeof(INT32U))){
      ret = -EFAULT;
      break;
    }
		
    for (i = 0; i < diPortCount; i++) {
      if((*((INT32U *)pdata + i) & 0xff) == 0x01) {
	privdata->usIntSrcConfig[i] = 0x01;
      } else if ((*((INT32U *)pdata + i) & 0xff) == 0x02) {
	privdata->usIntSrcConfig[i] = 0x02;
      } else if ((*((INT32U *)pdata + i) & 0xff) == 0x03) {
	privdata->usIntSrcConfig[i] = 0x03;
      } else if ((*((INT32U *)pdata + i) & 0xff) == 0x00){
	privdata->usIntSrcConfig[i] = 0x00;
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
static long adv_device_get_property(adv_device *device, USHORT nID, void *pData,INT32S *pDataLength) 
{
	private_data *privdata = (private_data*)(device->private_data);

	void*  cfgData = NULL;
	ULONG  DataLength;		/* user send */
	ULONG  sizeNeed= 0;
	ULONG  diPortCount =2;
	INT32U temp = 0;
	
	if ((pData == NULL) || (pDataLength == NULL)) {
		return -EFAULT;
	}
	
	if (copy_from_user(&DataLength, pDataLength, sizeof(ULONG))) {
		return -EFAULT;
	}
     
     
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
	case CFG_CascadeMode:
		sizeNeed = sizeof(privdata->IsCascadeMode);
		cfgData = &privdata->IsCascadeMode;
		break;
     
	case CFG_DiChannelCount:
	case CFG_DoChannelCount:
		// pData should be a long *
		sizeNeed = sizeof(INT32U);
		temp = diPortCount * 8;
		cfgData = &temp;
		break;
	case CFG_DiPortCount:
	case CFG_DoPortCount:
		// pData should be a long *
		sizeNeed = sizeof(INT32U);
		temp = diPortCount * 8;
		cfgData = &temp;
		break;
	case CFG_InterruptTriggerSource:
		sizeNeed = 2 * sizeof(INT32U);
		cfgData = privdata->usIntSrcConfig;
		break;
	
	case CFG_DiInterruptTriggerOnRisingEdge:
		sizeNeed = sizeof(INT16U);
		if(privdata->usInterruptSrc & 0x04)
			temp |= 0x0001;
		if(privdata->usInterruptSrc & 0x40)
			temp |= 0x0100;
		cfgData = &temp;
		break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
		sizeNeed = sizeof(INT16U);
		if(!(privdata->usInterruptSrc & 0x04))
			temp |= 0x0001 ;
		if(!(privdata->usInterruptSrc & 0x40))
			temp |= 0x0100;
		cfgData = &temp;
		break;
       
	default:
		return -EFAULT;
	}

	if (DataLength < sizeNeed) {
		return -EFAULT;
	}

	if (copy_to_user(pDataLength, &sizeNeed, sizeof(INT32U))) {
		return -EFAULT;
	}

	if (copy_to_user(pData, cfgData, sizeNeed)) {
		return -EFAULT;
	}

	return 0;
}

/**
 * adv_enable_event - enable/disable event
 * @device - device handle
 * @EventType - Event Type
 * @Enabled - Enabled
 * @Count - count
 */
   
int adv_enable_event(adv_device *device, USHORT EventType, USHORT Enabled, USHORT Count)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT32S ret = 0;
	
 	if(Enabled){
		if(Count <= 0)
			return InvalidEventCount;
		/* set the interrupt control register */
		/* modify according the new event mechanism*/
		switch (EventType) {
		case ADS_EVT_DI_INTERRUPT0: 
			if ((privdata->usIntSrcConfig[0] == 0x01) 
			    || (privdata->usIntSrcConfig[0] == 0x02)) {
				privdata->usInterruptSrc &= 0xfc;
				privdata->usInterruptSrc |= privdata->usIntSrcConfig[0] & 0x03;
				adv_process_info_enable_event(&privdata->ptr_process_info, 0, Count);
			} else {
				printk("Advantech : Please Check Device's Property\n");
				ret = -EFAULT;
			}
			break;
		case ADS_EVT_INTERRUPT_TIMER1: 
			if (privdata->usIntSrcConfig[0] == 0x03) {
				privdata->usInterruptSrc &= 0xfc;
				privdata->usInterruptSrc |= privdata->usIntSrcConfig[0] & 0x03;
				adv_process_info_enable_event(&privdata->ptr_process_info, 1, Count);

				/* Initiallize Timer register */
				if (privdata->IsCascadeMode == 1) {
					/* if counter0, 1 cascade */
					advOutp(privdata, 27, 0x36|(0<<6));
					advOutp(privdata, 24, privdata->cnt_init_data[0]&0xff);
					advOutp(privdata, 24, (privdata->cnt_init_data[0]>>8)&0xff);
				}
				advOutp(privdata, 27, 0x36|(1<<6));
				advOutp(privdata, 24+1, (privdata->cnt_init_data[1])&0xff);
				advOutp(privdata, 24+1, (privdata->cnt_init_data[1]>>8)&0xff);
			} else {
				printk("Advantech : Property Error! Please Check Device's Property\n");
				ret = -EFAULT;
			}
		
			break;
		case ADS_EVT_DI_INTERRUPT8: 
			if ((privdata->usIntSrcConfig[1] == 0x01) 
			    || (privdata->usIntSrcConfig[1] == 0x02)) {
				privdata->usInterruptSrc &= 0xcf;
				privdata->usInterruptSrc |= (privdata->usIntSrcConfig[1] << 4) & 0x30;
				adv_process_info_enable_event(&privdata->ptr_process_info, 2, Count);
			} else {
				printk("Advantech : Please Check Device's Property\n");
				ret = -EFAULT;
			}
			break;
		case ADS_EVT_TERMINATE_CNT2: 
			if (privdata->usIntSrcConfig[1] == 0x03) {
				privdata->usInterruptSrc &= 0xcf;
				privdata->usInterruptSrc |= (privdata->usIntSrcConfig[1] << 4) & 0x30;
				adv_process_info_enable_event(&privdata->ptr_process_info, 3, Count);
				
				/* Initillize Counter Resigter */
				advOutp(privdata, 27, 0x36|(2<<6));
				advOutp(privdata, 24+2, privdata->cnt_init_data[2]&0xff);
				advOutp(privdata, 24+2, (privdata->cnt_init_data[2]>>8)&0xff);

			} else {
				printk("Advantech : Property Error! Please Check Device's Property\n");
				ret = -EFAULT;
			}
		
			break;
		default:
			ret = -EFAULT;
			break;
		}
	
		advOutp(privdata, 32, privdata->usInterruptSrc);	  

	} else {
		switch (EventType) {
		case ADS_EVT_DI_INTERRUPT0: 
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			if (((privdata->usIntSrcConfig[0] == 0x01) || (privdata->usIntSrcConfig[0] == 0x02))
			    && !adv_process_info_isenable_event_all(&privdata->ptr_process_info, 0)) {
				privdata->usInterruptSrc &= 0xFC;
			}
		
			break;
		case ADS_EVT_INTERRUPT_TIMER1: 
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			if ((privdata->usIntSrcConfig[0] == 0x03) 
			    && !adv_process_info_isenable_event_all(&privdata->ptr_process_info, 1)) {
				privdata->usInterruptSrc &= 0xFC;
			}
		
			break;
		case ADS_EVT_DI_INTERRUPT8: 
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			if (((privdata->usIntSrcConfig[1] == 0x01) ||(privdata->usIntSrcConfig[1] == 0x02))
			    && !adv_process_info_isenable_event_all(&privdata->ptr_process_info, 2)) {
				privdata->usInterruptSrc &= 0xCF;
			}
	     	
			break;
			
		case ADS_EVT_TERMINATE_CNT2: 
			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			
			if ((privdata->usIntSrcConfig[1] == 0x03)
			    && !adv_process_info_isenable_event_all(&privdata->ptr_process_info, 3)) {
				privdata->usInterruptSrc &= 0xCF;
			}
			break;
		default:
			ret = -EFAULT;
		}

		advOutp(privdata, 32, privdata->usInterruptSrc);	  	  
	}

	return ret;
}


INT32U adv_check_event(adv_device *device, INT32U *EventType, 
		       INT32U Milliseconds)
{
	private_data *privdata = (private_data*)(device->private_data);
	adv_process_info_header *p = &privdata->ptr_process_info;
	INT32S event_th = 0;
		
	if (!(*EventType)) {
		event_th = adv_process_info_check_event(p);
		
		if(event_th <= 0){
			*EventType = 0;
		}else{
			switch(event_th - 1)
			{
			case 0:
				*EventType = ADS_EVT_DI_INTERRUPT0;
				break;
			case 1:
				*EventType = ADS_EVT_INTERRUPT_TIMER1;
				break;
			case 2:
				*EventType = ADS_EVT_DI_INTERRUPT8;
				break;
			case 3:
				*EventType = ADS_EVT_TERMINATE_CNT2;
				break;
			}
			
		}
	  
	} else {
		switch (*EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			event_th = 0;
			break;
		case ADS_EVT_INTERRUPT_TIMER1:
			event_th = 1;
			break;
		case ADS_EVT_DI_INTERRUPT8:
			event_th = 2;
			break;
		case ADS_EVT_TERMINATE_CNT2:
			event_th = 3;
		default:
			event_th = -1;
			break;
		}
		if ((event_th >= 0) && (event_th <=3))
			*EventType = adv_process_info_check_special_event(p, event_th);
		else
			*EventType = 0;
		
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
int adv_ioctrl(adv_device *device, unsigned int cmd, unsigned long arg)
{
	PT_DeviceSetParam    lpDeviceSetParam;
	PT_DeviceGetParam    lpDeviceGetParam;
	PT_PortOperation     lpPortOperation;
	PT_EnableEvent       lpEnableEvent;
	PT_CheckEvent        lpCheckEvent;
	INT32S ret = 0;
     
     
	switch( cmd )
	{
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
		
		
	case ADV_COUNTER:
		ret = adv_cnt_ioctl(device, (void*)arg);
		break;

	case ADV_PORTREAD:
		ret = copy_from_user(&lpPortOperation,(void *)arg,sizeof(PT_PortOperation));
		if(ret < 0){
			break;
		}
		
		ret = adv_port_read(device, lpPortOperation.PortNumber,
				    lpPortOperation.DataBuffer, 
				    lpPortOperation.DataLength);

		break;

	case ADV_PORTWRITE:
		ret = copy_from_user(&lpPortOperation,(void *)arg,sizeof(PT_PortOperation));
		if(ret != 0){
			ret = -EFAULT;
			break;
		}
		
		ret = adv_port_write(device, lpPortOperation.PortNumber,
				     lpPortOperation.DataBuffer,
				     lpPortOperation.DataLength);

		break;
		
	case ADV_ENABLE_EVENT: 
		if(copy_from_user(&lpEnableEvent,(void *)arg,sizeof(PT_EnableEvent))){
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

/* PCI-1750 fops */
adv_general_ops dev1750_fops =
{
     .opendevice	=adv_opendevice,
     .closedevice	=adv_closedevice,
     .read		=adv_read,
     .ioctrl		=adv_ioctrl,
     .resetdevice	=adv_resetdevice,
     .mmap		=adv_mmap,
     .poll		=adv_poll,
};

/* PCI-1750 driver object */
adv_driver pci1750_driver =
{
     .driver_name		= "pci1750",
     .devfs_name		= "pci1750",
     .ops			= &dev1750_fops,
};

static irqreturn_t pci1750_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	private_data   *privdata;

	unsigned char cmd;

	privdata=( private_data *)dev_id;
	
	spin_lock(&privdata->spinlock);
	cmd = advInp(privdata, 32);
	/* **********************************************************************
	 * Interrupt Control Register (Base + 32)
	 * Bit#          D7   D6   D5   D4   D3   D2   D1   D0
	 * Abbreviation  F1   E1  M11   M10  F0   E0   M01  M00
	 *               |    |    |    |    |     |    |____|__Interrupt Group0
	 *               |    |    |    |    |     |____________Triger edge
	 *               |    |    |    |    |__________________Interrupt Flag Bit
	 *               |    |    |____|_______________________Interrupt Group1
	 *               |    |_________________________________Triger edge
	 *               |______________________________________Interrupt Flag Bit
	 *               F1&F0          Interrupt status
	 *          read  1                Interrupt
	 *                0                No Interrupt
	 *          write 1                Clear Interrupt
	 *                0                Don't care
	 * *********************************************************************
	 * */
	if(!(cmd & 0x88))		/* is pci1750 interrupt */
	{
		spin_unlock(&privdata->spinlock);
		return IRQ_RETVAL(0);
	}
     
	advOutp(privdata, 32, cmd);
	if(cmd & 0x08){		/* is group 0 interrupt */
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
	}
     
	if(cmd & 0x80){		/* is grop 1 interrupt */
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
		adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
	}

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

	INT32S err;
	if ((err = pci_enable_device(dev)) != 0)
	{
		KdPrint(KERN_ERR  ":pci_enable_device failed\n");
		return err;
	}

	device = (adv_device *)kmalloc(sizeof(adv_device), GFP_KERNEL);
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if( (device == NULL) || (privdata == NULL) ){
		return -ENOMEM;
	}
     
	memset(device, 0, sizeof(adv_device));
	memset(privdata, 0, sizeof(privdata));
	
	//Initialize the private data of the device 
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->iobase = dev->resource[2].start;// & ~1UL;
	privdata->iolength = dev->resource[2].end -dev->resource[2].start;
	privdata->irq=dev->irq;
	privdata->sigpid=0;
	privdata->boardID = 0;
	privdata->IsCascadeMode = 1; /* PCI1750 is cascademode
				      * counter0 with counter1*/
	privdata->usIntSrcConfig[0] = 0;
	privdata->usIntSrcConfig[1] = 0;
	init_waitqueue_head(&privdata->event_wait);
	adv_process_info_header_init(&privdata->ptr_process_info);
	
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1750") == NULL) {
		kfree(privdata);
		kfree(device);
		return -EFAULT;
	}
	
	err = request_irq(privdata->irq, pci1750_interrupt_handler, SA_SHIRQ, "adv1750", privdata);
	if (err) {
		kfree(privdata);
		kfree(device);
		return err;
	}
	
	/* inintialize the adv_device structure */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->boardID);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);
	advdrv_device_set_devname(device, "pci1750");
		
	spin_lock_init( &privdata->spinlock );

	pci_set_drvdata(dev, device);
     
	advdrv_add_device(&pci1750_driver, device);
	printk("Add a Advantech PCI%x device:boardID=%x;iobase=0x%lx;DMAbase=0x%lx;irq=%x;slot=%x.\n",dev->device,privdata->boardID,privdata->iobase, privdata->ioDMAbase, (INT32U)privdata->irq, privdata->pci_slot);
	return 0;
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
     
     advdrv_remove_device(&pci1750_driver, device);
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
     return 0;
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

     return 0;
}

struct pci_driver pci_driver = 
{
     name:		 "pci1750",
     probe:		 advdrv_init_one,
     remove:		 __devexit_p(advdrv_remove_one),
     suspend:	 advdrv_suspend_one,
     resume:		advdrv_resume_one,
     id_table:	 serial_pci_tbl,
};



/************************************************************************
 * static int __init pci1750_init(void)
 * 
 * Description:  The module initialize PCI-1750 driver - Enumerate all PCI cards on the
 * 				 bus, register the driver in Advantech core driver.
 *************************************************************************/
static int __init pci1750_init(void)
{
     	int error =0 ;
	struct semaphore *sema = NULL;

	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if(sema == NULL)
	{
		return -ENOMEM;
	}
     
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1750_driver, sema);
         
	advdrv_register_driver( &pci1750_driver );
	error =  pci_module_init(&pci_driver) ;
	if( error != 0 )
	{
		advdrv_unregister_driver( &pci1750_driver );
		kfree(sema);
		return error;
	}
	return SUCCESS;
	
}
/************************************************************************
 * static int __exit pci1750_exit(void)
 * 
 * Description:  Exit  PCI-1750 driver .
 *************************************************************************/
static void __exit pci1750_exit(void)
{
	
     struct semaphore *sema = NULL;
     pci_unregister_driver( &pci_driver );
     advdrv_unregister_driver( &pci1750_driver );
     
     _ADV_GET_DRIVER_SEMA(pci1750_driver, sema);
     kfree(sema);
}

module_init( pci1750_init );
module_exit( pci1750_exit );

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1750 device driver module");
