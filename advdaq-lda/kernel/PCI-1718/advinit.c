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
	07/11/2006		Create by zhiyong.xie

************************************************************************/

/* base module includes */
//#include <linux/config.h>
#include <linux/module.h>

#include "PCI1718.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif



//#define PCI1718_DEBUG
static struct pci_device_id serial_pci_tbl[] = 
{
     { VENDOR_PCI_ID_ADVANTECH, 0x1718,
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
int adv_opendevice( adv_device *device)
{
	
	private_data *privdata = NULL;
     
	_ADV_GET_DEVICE_PRIVDATA(device, privdata);
	
	return adv_process_info_add(&privdata->ptr_process_info, 5);
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
	private_data *privdata = NULL;
     
	_ADV_GET_DEVICE_PRIVDATA(device, privdata);

	if (privdata->fai_running) {
		adv_ai_int_terminate(privdata);
	}

	return adv_process_info_remove(&privdata->ptr_process_info);
	
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
 * Description:  	The advdrv_core driver calls to map the
 *                      kernel memory to user
 * Parameters:	
              device 	-Points to the device object
 	      vma 	-vm_area_struct 
*************************************************************************/
int adv_mmap(adv_device *device, struct file *file, struct vm_area_struct *vma )
{
     printk("This driver does not surpport mmap\n");
     return SUCCESS;
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
int adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  )
{
     return SUCCESS;
     
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
	kdev_info->irq = privdata->irq;
	
	if (copy_to_user(udev_info, kdev_info, sizeof(adv_devinfo)))
		ret = -EFAULT;
	
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
int adv_ioctrl(adv_device *device, unsigned int cmd, unsigned long arg)
{
	private_data *privdata = (private_data*)(device->private_data);
	PT_DeviceSetParam    lpDeviceSetParam;
	PT_DeviceGetParam    lpDeviceGetParam;
	PT_PortOperation     lpPortOperation;
	PT_EnableEvent       lpEnableEvent;
	PT_CheckEvent        lpCheckEvent;
	INT32U error = 0;
	int ret = 0;
     
	switch( cmd )
	{
/* 	case ADV_DEVGETPROPERTY: */
/* 		if(copy_from_user(&lpDeviceGetParam,(void *)arg,sizeof(PT_DeviceGetParam))){ */
/* 			printk("DeviceGetProperty get data  error!\n"); */
/* 			ret = MemoryCopyFailed; */
/* 			break; */
/* 		} */

/* 		ret = adv_device_get_property( device, */
/* 					       lpDeviceGetParam.nID, */
/* 					       lpDeviceGetParam.pData, */
/* 					       lpDeviceGetParam.Length); */
/* 		break; */
		
/* 	case ADV_DEVSETPROPERTY: */

/* 		if(copy_from_user(&lpDeviceSetParam,(void *)arg,sizeof(PT_DeviceSetParam))){ */
/* 			printk("DeviceGetProperty set data  error!\n"); */
	       
/* 			ret = MemoryCopyFailed; */
/* 			break; */
/* 		} */
/* 		ret = adv_device_set_property( device, */
/* 					       lpDeviceSetParam.nID, */
/* 					       lpDeviceSetParam.pData, */
/* 					       lpDeviceSetParam.Length); */
/* 		break; */
		
/* 	case ADV_AIBINARYIN: */
/* 		ret = adv_ai_binary_in(device, (void *) arg); */
/* 		break; */
/* 	case ADV_AICONFIG: */
/* 		ret = adv_ai_config(device, (void *) arg); */
/* 		break; */
/* 	case ADV_MAIBINARYIN: */
/* 		ret = adv_mai_binary_in(device, (void *) arg); */
/* 		break; */
/* 	case ADV_MAICONFIG: */
/* 		ret = adv_mai_config(device, (void *) arg); */
/* 		break; */
/* 	case ADV_AIVOLTIN: */
/* 		ret = adv_ai_voltage_in(device, (void *) arg); */
/* 		break; */
/* 	case ADV_MAIVOLTIN: */
/* 		ret = adv_ai_mvoltage_in(device, (void *) arg); */
/* 		break; */
/* 	case ADV_AISCALE: */
/* 		ret = adv_ai_scale(device, (void *) arg); */
/* 		break; */
/* 	case ADV_AI_INT_START: */
/* 		ret = adv_ai_int_start(device, (void *) arg); */
/* 		break; */
	case ADV_AI_INT_SCAN_START:
		ret = adv_ai_int_scan_start(device, (void *) arg);
		break;
	case ADV_AI_TRANSFER:
		ret = adv_fai_transfer(device, (void *)arg);
		break;
	case FAICHECK:
		ret = adv_ai_check(device, (void *) arg);
		break;
	case CLEARFLAG:
		ret = adv_clear_flag(device, (INT32U)arg);
		break;
	case ADV_AI_STOP:
		ret = adv_ai_int_terminate(privdata);
		break;
/* 	case ADV_DIO_BITS: */
/* 		ret = adv_dio_ioctl(device, (void*)arg); */
/* 		break; */
		
/* 	case ADV_COUNTER: */
/* 		ret = adv_cnt_ioctl(device, (void*)arg); */
/* 		break; */
/* 	case ADV_AOBINARYOUT: */
/* 		ret = adv_ao_binary_out(device, (void *) arg); */
/* 		break; */
/* 	case ADV_AOVOLTOUT: */
/* 		ret = adv_ao_voltage_out(device, (void *) arg); */
/* 		break; */
/* 	case ADV_AOCONFIG: */
/* 		ret = adv_ao_config(device, (void *) arg); */
/* 		break; */
/* 	case ADV_AOSCALE: */
/* 		ret = adv_ao_scale(device, (void *) arg); */
/* 		break; */
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

	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (adv_devinfo *) arg);
		break;
	
	default:
		ret = -EFAULT;
		break;
	}

	return ret;
	
}

/* PCI-1718 fops */
adv_general_ops dev1718_fops =
{
	.opendevice	=adv_opendevice,
	.closedevice	=adv_closedevice,
	.read		=adv_read,
	.ioctrl		=adv_ioctrl,
	.resetdevice	=adv_resetdevice,
	.mmap		=adv_mmap,
	.poll		=adv_poll,
};

/* PCI-1718 driver object */
adv_driver pci1718_driver =
{
     .driver_name		= "pci1718",
     .devfs_name		= "pci1718",
     .ops			= &dev1718_fops,
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

	if ((err = pci_enable_device(dev)) != 0) {
		KdPrint(KERN_ERR  ":pci_enable_device failed\n");
		return err;
	}

	device = (adv_device *)kmalloc(sizeof(adv_device), GFP_KERNEL);
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if((!device) || (!privdata)) {
		kfree(device);
		kfree(privdata);
		PCRIT("Could not kmalloc space for device!");
		return -ENOMEM;
	}

	memset(device, 0, sizeof(adv_device));
	memset(privdata, 0, sizeof(private_data) );

	/* initialize the private_data structure */
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
     	privdata->iobase = dev->resource[2].start;// & ~1UL;
	privdata->iolength = dev->resource[2].end -dev->resource[2].start;
	privdata->irq=dev->irq;
	privdata->sigpid=0;
	privdata->boardID = advInp(privdata, 0x14) & 0x0f;
	privdata->device_type = dev->subsystem_device;
	privdata->timer_clock = 1; /* pacer clock freq in M Hz, default is 10 M Hz */
	privdata->half_fifo_size = 512;	/* total fifo is 1K word */
	privdata->ai_stop = 1;	/* 0 : complete 1 : incomplete */
	spin_lock_init(&privdata->spinlock);
	tasklet_init(&privdata->tasklet, pci1718_tasklet_handler, (PTR_T)privdata);	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	privdata->fai_stop.routine = (void (*)(void *)) adv_ai_int_stop;
	privdata->fai_stop.data = (void *) privdata;
	INIT_LIST_HEAD(&privdata->tq_fai_stop);
	queue_task(&privdata->fai_stop, &privdata->tq_fai_stop);
#else
	INIT_WORK(&privdata->fai_stop, adv_ai_int_stop, privdata);
#endif	
	
	/* for new event mechanism */
	init_waitqueue_head(&privdata->event_wait);
	adv_process_info_header_init(&privdata->ptr_process_info);

	/* inintialize the adv_device structure */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device,privdata->boardID);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);

	if (privdata->device_type == PCI1718HDU)
		advdrv_device_set_devname(device, "pci1718hdu");
	else if (privdata->device_type == PCI1718HGU)
		advdrv_device_set_devname(device, "pci1718hgu");
	
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1718") == NULL) {
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}
	
	if (request_irq(privdata->irq, pci1718_interrupt_handler, SA_SHIRQ, "adv1718", privdata)) {
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}
	
	pci_set_drvdata(dev, device);

	advdrv_add_device( &pci1718_driver, device );
	printk("Add a Advantech PCI%x device:boardID=%d;iobase=0x%lx;irq=%x;slot=%x.\n",dev->device,privdata->boardID,privdata->iobase, privdata->irq, privdata->pci_slot  );
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
	
	tasklet_disable(&privdata->tasklet);

     advdrv_remove_device(&pci1718_driver, device);
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
	name:		 "pci1718",
	probe:		 advdrv_init_one,
	remove:		 __devexit_p(advdrv_remove_one),
	suspend:	 advdrv_suspend_one,
	resume:		advdrv_resume_one,
	id_table:	 serial_pci_tbl,
};



/************************************************************************
 * static int __init pci1718_init(void)
 * 
 * Description:  The module initialize PCI-1718 driver - 
                 Enumerate all PCI cards on the
 * 		 bus, register the driver in Advantech core driver.
 *************************************************************************/
static int __init pci1718_init(void)
{
	int error =0 ;
	struct semaphore *sema = NULL;

	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if(sema == NULL)
	{
		return -ENOMEM;
	}
     
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1718_driver, sema);
     
	advdrv_register_driver( &pci1718_driver );
	error =  pci_module_init(&pci_driver) ;
	if( error != 0 )
	{
		advdrv_unregister_driver( &pci1718_driver );
		kfree(sema);
		return error;
	}
	return SUCCESS;
	
}
/************************************************************************
 * static int __init pci1718_init(void)
 * 
 * Description:  Exit  PCI-1718 driver .
 *************************************************************************/
static void __exit pci1718_exit(void)
{

     struct semaphore *sema = NULL;
     pci_unregister_driver( &pci_driver );
     advdrv_unregister_driver( &pci1718_driver );
     
     _ADV_GET_DRIVER_SEMA(pci1718_driver, sema);
     kfree(sema);
}

module_init( pci1718_init );
module_exit( pci1718_exit );

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1718 device driver module");
