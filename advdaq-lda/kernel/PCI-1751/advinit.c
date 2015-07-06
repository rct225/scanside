/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1751 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1751 driver module and device. 
Version history
	02/13/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
#include <linux/module.h>

#include "PCI1751.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] = 
{
     { 0x13fe, 0x1751,
       PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0
     },
     { 0x13fe, 0x3751,
       PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0
     },
     {0}, 

};

//#define PCI1751_DEBUG
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
	if (privdata->device_type == PCI1751) {
		ret = adv_process_info_add(&privdata->ptr_process_info, 4);
	} else {
		ret = adv_process_info_add(&privdata->ptr_process_info, 6);
	}
	
	
	return ret;
}

/************************************************************************
 * Function:		int adv_closedevice
 * 
 * Description:  	The advdrv_core driver calls to close the device.
 * Parameters:	
	    device 	-Points to the device object
*************************************************************************/
INT32S  adv_closedevice( adv_device *device)
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
int adv_mmap( adv_device *device, struct file *file, struct vm_area_struct *vma )
{
	printk("This driver does not surpport mmap\n");
	return 0;
}


/**
 * adv_poll - relative poll or select system call
 */
INT32U adv_poll(adv_device *device, struct file *file, struct poll_table_struct *wait)
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
INT32S adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  )
{
     return 0;
}


/**
 * adv_get_devinfo - get device information
 * 
 * @device: point to the device object
 * @dev_info : device info struct
 */
static INT32S adv_get_devinfo(adv_device *device, adv_devinfo *udev_info)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_devinfo *kdev_info;
	INT32U ret = 0;
	
	kdev_info = kmalloc(sizeof(adv_devinfo), GFP_KERNEL);
	if (kdev_info == NULL) 
		return -EFAULT;
		
	kdev_info->iobase = privdata->iobase;
	kdev_info->iolength = privdata->iolength;
	kdev_info->pci_slot = privdata->pci_slot;
	kdev_info->pci_bus = privdata->pci_bus;
	kdev_info->switchID = privdata->boardID;
	kdev_info->deviceID = privdata->device_type;

	if (copy_to_user(udev_info, kdev_info, sizeof(adv_devinfo))){
		ret = -EFAULT;
	}
	
	kfree(kdev_info);
	return ret;
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
     
     
	switch( cmd )
	{
	case ADV_DEVGETPROPERTY:
		ret = copy_from_user(&lpDeviceGetParam,(void *)arg,sizeof(PT_DeviceGetParam));
		if(ret < 0){
			break;
		}

		ret = adv_device_get_property( device,
					       lpDeviceGetParam.nID,
					       lpDeviceGetParam.pData, 
					       lpDeviceGetParam.Length);
		break;
		
	case ADV_DEVSETPROPERTY:
		ret = copy_from_user(&lpDeviceSetParam,(void *)arg,sizeof(PT_DeviceSetParam));
		if(ret < 0){
			break;
		}

		ret = adv_device_set_property( device, 
					       lpDeviceSetParam.nID,
					       lpDeviceSetParam.pData,
					       lpDeviceSetParam.Length);
		break;
		
	case ADV_DIO_BITS:
		ret = adv_dio_ioctl(device, (void*)arg);
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
		if(ret < 0){
			break;
		}
		
			
		ret = adv_port_write(device, lpPortOperation.PortNumber,
				     lpPortOperation.DataBuffer,
				     lpPortOperation.DataLength);

		break;
		
	case ADV_ENABLE_EVENT: 
		ret = copy_from_user(&lpEnableEvent,(void *)arg,sizeof(PT_EnableEvent));
		if(ret < 0){
			break;
		}

		ret = adv_enable_event(device,lpEnableEvent.EventType,
				       lpEnableEvent.Enabled,
				       lpEnableEvent.Count);	
		break;
		
	case CHECKEVENT:
	     	ret = copy_from_user(&lpCheckEvent, (PT_CheckEvent *)arg,sizeof(PT_CheckEvent));
		if(ret < 0){
			break;
		}

		adv_check_event(device, (INT32U *)&lpCheckEvent.EventType, 
				lpCheckEvent.Milliseconds);
	  
		ret = copy_to_user((PT_CheckEvent *)arg,&lpCheckEvent,sizeof(PT_CheckEvent));
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (adv_devinfo *) arg);
		break;

	default:
		ret = -EFAULT;
		break;
	}

	return ret;

}

/* PCI-1751 fops */
adv_general_ops dev1751_fops =
{
     .opendevice	=adv_opendevice,
     .closedevice	=adv_closedevice,
     .read		=adv_read,
     .ioctrl		=adv_ioctrl,
     .resetdevice	=adv_resetdevice,
     .mmap		=adv_mmap,
     .poll		=adv_poll,
};

/* PCI-1751 driver object */
adv_driver pci1751_driver =
{
     .driver_name		= "pci1751",
     .devfs_name		= "pci1751",
     .ops			= &dev1751_fops,
};

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

	int err;
	if ((err = pci_enable_device(dev)) != 0)
	{
		KdPrint(KERN_ERR  ":pci_enable_device failed\n");
		return err;
	}
     
	device = (adv_device *)kmalloc(sizeof(adv_device), GFP_KERNEL);
	privdata = (private_data *)kmalloc(sizeof(private_data), GFP_KERNEL);
	if ((privdata == NULL) || (device == NULL))
	{
		kfree( device );
		kfree(privdata);
		return  MemoryAllocateFailed;
	}
   
	memset(device, 0, sizeof(adv_device));
	memset(privdata, 0, sizeof(private_data) );	
     
	//Initialize the private data of the device 
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->iobase = dev->resource[2].start;// & ~1UL;
	privdata->iolength = dev->resource[2].end -dev->resource[2].start;
	privdata->irq = dev->irq;
	privdata->sigpid = 0;
	
	if ( (dev->device == 0x1751) && (dev->subsystem_device != 0x9050)) {
	  privdata->boardID = advInp(privdata, 20) & 0x0f;
	}
	  privdata->device_type = dev->device;
	init_waitqueue_head(&privdata->event_wait);
	adv_process_info_header_init(&privdata->ptr_process_info);
	
	if(request_region(privdata->iobase, privdata->iolength, "PCI-1751" ) == NULL){
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}

	switch (privdata->device_type) {
	case MIC3751:
		advdrv_device_set_devname(device, "mic3751");
		privdata->ioDMAbase = dev->resource[1].start;// & ~1UL;
		privdata->ioDMAlength = dev->resource[1].end -dev->resource[1].start;
		privdata->dio_group_num = 3;
		break;
	default:
		advdrv_device_set_devname(device, "pci1751");
		privdata->dio_group_num = 2;
		break;
	}
	
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->boardID);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);
	
	spin_lock_init( &privdata->spinlock );
	pci_set_drvdata(dev, device);
        
	request_irq(privdata->irq, pci1751_interrupt_handler, SA_SHIRQ, "adv1751", privdata); 
     
	advdrv_add_device( &pci1751_driver, device); 
	if (privdata->device_type == PCI1751) {
		printk("Add a Advantech PCI-");
	} else {
		printk("Add a Advantech MIC-");
	}
	
	printk("%x device:boardID=%x;iobase=0x%lx;DMAbase=0x%x;irq=%x;slot=%x.\n",dev->device, privdata->boardID, privdata->iobase, (INT32U)privdata->ioDMAbase, (INT32U)privdata->irq, privdata->pci_slot  );
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

     advdrv_remove_device(&pci1751_driver, device);
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
     name:		 "pci1751",
     probe:		 advdrv_init_one,
     remove:		 __devexit_p(advdrv_remove_one),
     suspend:	 advdrv_suspend_one,
     resume:		advdrv_resume_one,
     id_table:	 serial_pci_tbl,
};



/************************************************************************
 * static int __init pci1751_init(void)
 * 
 * Description:  The module initialize PCI-1751 driver - Enumerate all PCI cards on the
 * 				 bus, register the driver in Advantech core driver.
 *************************************************************************/
static int __init pci1751_init(void)
{
	int error =0 ;
     	struct semaphore *sema = NULL;

	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if(sema == NULL)
	{
		return -ENOMEM;
	}
     
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1751_driver, sema);

	advdrv_register_driver( &pci1751_driver );
	error =  pci_module_init(&pci_driver) ;
	if( error != 0 )
	{
		advdrv_unregister_driver( &pci1751_driver );
		kfree(sema);
		return error;
	}
	return 0;
}
/************************************************************************
 * static int __init pci1751_init(void)
 * 
 * Description:  Exit  PCI-1751 driver .
 *************************************************************************/
static void __exit pci1751_exit(void)
{
     struct semaphore *sema = NULL;
     pci_unregister_driver( &pci_driver );
     advdrv_unregister_driver( &pci1751_driver );
     
     _ADV_GET_DRIVER_SEMA(pci1751_driver, sema);
     kfree(sema);
}

module_init( pci1751_init );
module_exit( pci1751_exit );

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1751 device driver module");
