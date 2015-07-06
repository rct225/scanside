
/* base module includes */

#include <linux/module.h>

#include "PCM3753I.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif


static struct pci_device_id serial_pci_tbl[] = 
{
     { 0x13fe, 0x3753,
       PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0
     },

     {0}, 
};

/**
 * adv_opendevice - callback of open system call
 * @device - pointer to a adv_device object
 *
 */
INT32S adv_opendevice( adv_device *device)
{
	INT32S ret;
	private_data  *privdata = NULL;
	
	_ADV_GET_DEVICE_PRIVDATA(device, privdata);

	if(privdata->us_exp_board == 1)
		ret = adv_process_info_add(&privdata->ptr_process_info, 12);
	else 
		ret = adv_process_info_add(&privdata->ptr_process_info, 6);

	return ret;
}

/**
 * adv_closedevice - callback of close system call
 * @device - pointer to a adv_device object
 *
 */
INT32S  adv_closedevice( adv_device *device)
{
	INT32S ret = 0;
	private_data *privdata = NULL;
	
	_ADV_GET_DEVICE_PRIVDATA(device, privdata);
	ret = adv_process_info_remove(&privdata->ptr_process_info);
	
	return ret;
}

/**
 * adv_resetdevice - callback of reset system call
 * @device - pointer to a adv_device object
 */
INT32S adv_resetdevice( adv_device *device )
{
	return 0;
}

/**
 * adv_mmap - callback of mmap system call
 * @device - pointer to a adv_device object
 * @vma - point to vm_area_struct
 */
INT32S adv_mmap( adv_device *device ,struct vm_area_struct *vma )
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

/**
 * adv_read - callback of read system call
 * @device - pointer to a adv_device object
 * @vma - point to vm_area_struct
 * @buf - points to user buffer
 * @nbytes - the number you want to read
 */
INT32S adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  )
{
	return 0;
}


/**
 * adv_ioctrl - relating ioctl system call
 * @device - points to adv_device object
 * @cmd - io command
 * @arg - argument
 *
 */
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
		if(ret)
			break;
				
		ret = adv_port_read(device, lpPortOperation.PortNumber,
				    lpPortOperation.DataBuffer, 
				    lpPortOperation.DataLength);

		break;
	
	case ADV_PORTWRITE:
		ret = copy_from_user(&lpPortOperation,(void *)arg,sizeof(PT_PortOperation));
		if(ret)
			break;
			
		ret = adv_port_write(device, lpPortOperation.PortNumber,
				     lpPortOperation.DataBuffer,
				     lpPortOperation.DataLength);

		break;
			
	case ADV_DIO_BITS:
		ret = adv_dio_ioctl(device, (void*)arg);
		break;
	
		
	case ADV_ENABLE_EVENT:
		ret = copy_from_user(&lpEnableEvent,(void *)arg,sizeof(PT_EnableEvent));
		if(ret)
			break;
		
		ret = adv_enable_event(device,lpEnableEvent.EventType,
				       lpEnableEvent.Enabled,
				       lpEnableEvent.Count);
		break;
		
	case CHECKEVENT:
	  	ret = copy_from_user(&lpCheckEvent, (PT_CheckEvent *)arg,sizeof(PT_CheckEvent));
		if(ret)
			break;
		
		ret = adv_check_event(device, (INT32U *)&lpCheckEvent.EventType, lpCheckEvent.Milliseconds);
		if(ret)
			break;
		
		ret = copy_to_user((PT_CheckEvent *)arg,&lpCheckEvent,sizeof(PT_CheckEvent));
		break;

	default:
		ret = -EFAULT;
		break;
	}

	return ret;
}

adv_general_ops dev3753i_fops =
{
     .opendevice	=adv_opendevice,
     .closedevice	=adv_closedevice,
     .read		=adv_read,
     .ioctrl		=adv_ioctrl,
     .resetdevice	=adv_resetdevice,
     .poll		=adv_poll,
};

adv_driver pcm3753i_driver =
{
	.owner = THIS_MODULE,
	.driver_name		= "pcm3753i",
	.devfs_name		= "pcm3753i",
	.ops			= &dev3753i_fops,
};


/**
 * advdrv_init_one - initialize the device
 * @dev - point to a pci_dev device
 * @ent - point to pci_device_id including the device info
 *
 * initialize the device and allocate resource for the device
 */
static INT32S __devinit
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
	if ((privdata == NULL) || (device == NULL))
	{
		kfree( device );
		kfree(privdata);
		return  -ENOMEM;
	}
   
	memset(device, 0, sizeof(adv_device));
	memset(privdata, 0, sizeof(private_data) );	

	/* Initialize the private data of the device  */
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->iobase = dev->resource[2].start;// & ~1UL;
	privdata->iolength = dev->resource[2].end -dev->resource[2].start;
	privdata->irq=dev->irq;
	privdata->device_type = dev->device; /* multi-card support for new driver */     
	privdata->boardID = advInp(privdata,20);
	
	/* Is expand board available */
	privdata->max_dio_group = 4;
	advOutp(privdata, 53, 0x05);
	if( (advInp(privdata, 53) & 0x07) == 0x02 ){
		advOutp(privdata, 53, 0x02);
		if( (advInp(privdata, 53) & 0x07) == 0x05) {
			privdata->us_exp_board = 1;
			privdata->max_dio_group = 8;
		}
	}
	
	spin_lock_init( &privdata->spinlock );	
	init_waitqueue_head(&privdata->event_wait);
	adv_process_info_header_init(&privdata->ptr_process_info);
	
	if(request_region(privdata->iobase, privdata->iolength, "PCM-3753I" ) == NULL)
	{
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}

	switch (privdata->device_type) {
	case PCM3753I:
			advdrv_device_set_devname(device, "pcm3753i");
		break;
	default: /* this case will never be happened */
		break;
	}

	
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->boardID);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);

	
	/* Initialize kernel data structure */
	pci_set_drvdata(dev, device);
        err = request_irq(privdata->irq, pcm3753i_interrupt_handler, SA_SHIRQ, "adv3753i", privdata);
	if (err) {
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}
	
	/* Add to device list */
	advdrv_add_device(&pcm3753i_driver, device); 

	/* Output Some information */
	printk("Add a Advantech PCM%x device:boardID=%x;iobase=0x%x;irq=%x;slot=%x.\n",dev->device,privdata->boardID,privdata->iobase, privdata->irq, privdata->pci_slot);

	/*advOutp(privdata, 16, 0x0);
	advOutp(privdata, 20, 0x0);
	advOutp(privdata, 24, 0x0);*/
	return 0;
}	
    
/**
 * advdrv_remove_one - callback of pci driver
 * @dev - point to pci_dev struct
 *
 * remove a device and free resource for the device
 */
static void __devexit advdrv_remove_one(struct pci_dev *dev)
{
     private_data *privdata = NULL; 
     adv_device *device = NULL;

     KdPrint("Remove a PCI-%x device\n", dev->device );

     device = pci_get_drvdata(dev);

     /* free resource of device  */
     privdata =(private_data*) device->private_data;
     free_irq(privdata->irq, privdata);
     release_region(privdata->iobase, privdata->iolength);  

     /* remove device from device list */
     advdrv_remove_device(&pcm3753i_driver, device);
    
     pci_set_drvdata(dev, NULL);
     pci_disable_device(dev);

     kfree(privdata);
     kfree(device);
}

/**
 * advdrv_suspend_one - callback of pci driver
 * @dev - pointer to a pci_dev struct
 * @state - reserve
 */
static int advdrv_suspend_one(struct pci_dev *dev, u32 state)
{
     return 0;
}

/**
 * advdrv_resume_one - callback of pci driver
 * @dev - pointer to a pci_dev object
 */
static int advdrv_resume_one(struct pci_dev *dev)
{
     return 0;
}

struct pci_driver pci_driver = 
{
     name:		 "pcm3753i",
     probe:		 advdrv_init_one,
     remove:		 __devexit_p(advdrv_remove_one),
     suspend:	 advdrv_suspend_one,
     resume:		advdrv_resume_one,
     id_table:	 serial_pci_tbl,
};



static INT32S __init pcm3753i_init(void)
{
	INT32S error =0 ;
     	struct semaphore *sema = NULL;

	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if(sema == NULL)
	{
		return -ENOMEM;
	}
     
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pcm3753i_driver, sema);

	advdrv_register_driver( &pcm3753i_driver );
	error =  pci_register_driver(&pci_driver) ;
	if( error < 0 )
	{
		advdrv_unregister_driver( &pcm3753i_driver );
		kfree(sema);
		return error;
	}
		
	return 0;
}


static void __exit pcm3753i_exit(void)
{
	struct semaphore *sema = NULL;
	pci_unregister_driver( &pci_driver );
	advdrv_unregister_driver( &pcm3753i_driver );
     
	_ADV_GET_DRIVER_SEMA(pcm3753i_driver, sema);
	kfree(sema);
}

module_init( pcm3753i_init );
module_exit( pcm3753i_exit );

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCM3753I device driver module");
