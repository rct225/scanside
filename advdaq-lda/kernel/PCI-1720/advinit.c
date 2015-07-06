/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech PCI-1720 Series Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1720 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	1/17/2006			Create by LI.ANG                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
/* #include <linux/config.h> */
#include <linux/module.h>
 

#include "PCI1720.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] =
{
	{       0x13fe, 0x1720,	/* PCI-1720 PCI-1720U */
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, 0
	},
	{       0x13fe, 0x1723,	/* PCI-1723 */
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, 0
	},
	{       0x13fe, 0x1724,	/* PCI-1724 */
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, 0
	},
	{       0x13fe, 0x1727,	/* PCI-1727U */
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, 0
	},
	{0},
};

/**
  * adv_opendevice - The advdrv_core driver calls to open the device.                           
  *                                                                      
  * @device: points to the device object                    
  */
static INT32S adv_opendevice(adv_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret;
	

/* 	if(!try_module_get(device->driver->owner)){ */
/* 		return -ENODEV; */
/* 	} */
	

	ret = adv_process_info_add(&privdata->ptr_process_info, 4);
	
	return ret;
}

/**
  * adv_closedevice - The advdrv_core driver calls to close the device.
  * 
  * @device: points to the device object
  */
static INT32S adv_closedevice(adv_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret;
	
/* 	module_put(device->driver->owner); */
	ret = adv_process_info_remove(&privdata->ptr_process_info);
	
	return ret;
}

/**
 * adv_get_devinfo - Get device information
 * 
 * @device: point to the device object
 * @arg: 
 */
static INT32S adv_get_devinfo(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_devinfo devinfo;
	

	devinfo.iobase = privdata->iobase;
	devinfo.irq = privdata->irq;
	devinfo.iolength = privdata->iolength;
	devinfo.pci_slot = privdata->pci_slot;
	devinfo.pci_bus = privdata->pci_bus;
	devinfo.switchID = privdata->board_id;
	devinfo.deviceID = privdata->device_type;

	if (copy_to_user(arg, &devinfo, sizeof(adv_devinfo))) {
		return -EFAULT;
	}

	return 0;
}

/**
  * adv_ioctrl - The advdrv_core driver calls to handle the ioctl commands.
  *
  * @device: Points to the device object
  * @cmd: io code.
  * @arg: argument
  */
static INT32S adv_ioctrl(adv_device *device, INT32U cmd, PTR_T arg)
{
	INT32S ret = 0;


	if (device == NULL) {
		return -ENODEV;
	}
	
     
	switch (cmd) {
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (void *) arg);
		break;
	case ADV_PORTREAD:
		ret = adv_read_port(device, (void *) arg);
		break;
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (void *) arg);
		break;
	case ADV_DIO_BITS:
		ret = adv_dio_ioctl(device, (void *) arg);
		break;
	default:
		return -EINVAL;
	}


	return ret;
}

/* PCI-1720 fops */
adv_general_ops dev1720_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
};

/* PCI-1720 driver object */
adv_driver pci1720_driver = {
/* 	.owner = THIS_MODULE, */
	.driver_name	= "pci1720",
	.devfs_name	= "pci1720",
	.ops		= &dev1720_fops,
};
 

/**
 * advdrv_init_one - Pnp to initialize the device, and allocate resource for the device.
 *
 * @dev: Points to the pci_dev device
 * @ent: Points to pci_device_id including the device info.
 */
static INT32S __devinit advdrv_init_one(struct pci_dev *dev, const struct pci_device_id *ent)
{     
	private_data *privdata = NULL;
	adv_device *device = NULL;
	INT32S ret;
     
     
	if ((ret = pci_enable_device(dev)) != 0) {
		printk(KERN_ERR": pci_enable_device failed\n");
		return ret;
	}

	/* allocate memory for device */
	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		printk("Could not kmalloc space for device!");
		return -ENOMEM;
	}
	memset(device, 0, sizeof(adv_device));
	
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		return -ENOMEM;
	}
	memset(privdata, 0, sizeof(private_data));


	/* initialize the private data in the device */
	privdata->irq = dev->irq;
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->iobase = dev->resource[2].start & ~1UL;
	privdata->iolength = dev->resource[2].end - dev->resource[2].start;
	privdata->device_type = dev->device; /* multi-card support for new driver */     

	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);
	

	/* request I/O regions */
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1720") == NULL) {
		kfree(device);
		kfree(privdata);
		printk("Request region failed\n");
		return  ret;
	}

	/* support multi-card */
	switch (privdata->device_type) {
	case PCI1720:
		privdata->board_id = (INT16U) (advInp(privdata, 0x20) & 0x0f);
		advdrv_device_set_devname(device, "pci1720");
		break;
	case PCI1723:
		privdata->board_id = (INT16U) (advInp(privdata, 0x10) & 0x0f);
		advdrv_device_set_devname(device, "pci1723");
		break;
	case PCI1724:
		privdata->board_id = (INT16U) (advInp(privdata, 0x10) & 0x0f);
		advdrv_device_set_devname(device, "pci1724");
		break;
	case PCI1727:
		privdata->board_id = (INT16U) (advInp(privdata, 0x23) & 0x0f);
		advdrv_device_set_devname(device, "pci1727");
		break;
	default: /* this case will never be happened */
		break;
	}


	/* link the info into the device structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);
	pci_set_drvdata(dev, device);

	/* add device into driver list */
	ret = advdrv_add_device(&pci1720_driver, device);
	if (ret != 0) {
		release_region(privdata->iobase, privdata->iolength);  
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return ret;
	}

	printk("Add a PCI-%x device: iobase=%xh; slot=%xh\n", 
	       dev->device,
	       privdata->iobase,
	       privdata->pci_slot);
     
	
	return 0;
}

/**
  * advdrv_remove_one -Pnp to remove a device, and free resource for the device.
  * 
  * @dev: Points to the pci_dev device;
  * 
  */
static void __devexit advdrv_remove_one(struct pci_dev *dev)
{
	private_data *privdata = NULL; 
	adv_device *device = NULL;


	printk("Remove a PCI-%x device\n", dev->device);

	device = pci_get_drvdata(dev);
	privdata = (private_data *) device->private_data;

	release_region(privdata->iobase, privdata->iolength);  

	pci_set_drvdata(dev, NULL);
	advdrv_remove_device(&pci1720_driver, device);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}


struct pci_driver pci_driver = {
	name:	"pci1720",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
	id_table:	serial_pci_tbl,
};

/**
  * pci1720_init - The module initialize PCI-1720 driver - Enumerate all PCI cards on the
  * 			    bus, register the driver in Advantech core driver.
  */
static INT32S __init pci1720_init(void)
{
	INT32S error;
	struct semaphore *sema = NULL;


	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return -ENOMEM;
	}

	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1720_driver, sema);


	advdrv_register_driver(&pci1720_driver);
	error = pci_module_init(&pci_driver);
	if (error != 0) {
		advdrv_unregister_driver(&pci1720_driver);
		return error;
	}

	return 0;
}

/**
  * pci1720_init -Exit  PCI-1720 driver .
  */
static void __exit pci1720_exit(void)
{
	struct semaphore *sema = NULL;


	pci_unregister_driver( &pci_driver );
	advdrv_unregister_driver( &pci1720_driver );
	_ADV_GET_DRIVER_SEMA(pci1720_driver, sema);
     
	kfree(sema);
}

module_init(pci1720_init);
module_exit(pci1720_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1720 device driver module");
