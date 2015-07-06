/***************************************************************************
 *                                                                         
 *                             ABJ                                         
 *		Copyright 2004 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech PCI-1780 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1780 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	1/17/2006			Create by LI.ANG                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
#include <linux/module.h>
#include "PCI1780.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] =
{
	{       0x13fe, 0x1780,	/* PCI-1780 Rev.A1 01-2 R3 */
		0x13fe, 0xa200,
		0, 0, 0
	},
	{       0x13fe, 0x1780,	/* PCI-1780 Rev.A1 01-2*/
		0x13fe, 0x0000,
		0, 0, 0
	},
	{       0x13fe, 0x1780,	/* PCI-1780 Rev.A1 01-1*/
		0x13fe, 0x1780,
		0, 0, 0
	},
	{0},
};


/**
 * adv_opendevice - The advdrv_core driver calls to open the device.
 *
 * @device: Points to the device object
 */
static INT32S adv_opendevice(adv_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret;
	
	ret = adv_process_info_add(&privdata->ptr_process_info, 10);

	privdata->user_cnt += 1;
	
	return ret;
}

/**
 * adv_closedevice - The advdrv_core driver calls to close the device.
 *
 * @device: Points to the device object
 */
static INT32S adv_closedevice(adv_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret;
	
	ret = adv_process_info_remove(&privdata->ptr_process_info);

	if (privdata->user_cnt <= 1) {
		privdata->user_cnt = 0;
	} else {
		privdata->user_cnt -= 1;
	}	

	if (privdata->user_cnt == 0) {
		advOutpw(privdata, 0x42, 0x00); /* disable interrupt */
		advOutpw(privdata, 0x44, 0x00); /* clear interrupt */
	}

	return ret;
}

/**
 * adv_poll - poll function to support select function of user mode
 *
 * @device: Points to the device object
 */
static INT32U adv_poll(adv_device *device, struct file *fp,
		       struct poll_table_struct *wait)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U mask = 0;
	

	poll_wait(fp, &privdata->event_wait, wait);
	
	if (adv_process_info_isset_event(&privdata->ptr_process_info)) {
		mask = POLLIN | POLLRDNORM;
	}
			
	return mask;
}

/**
 * adv_get_devinfo - get device information
 * 
 * @device: point to the device object
 * @arg: 
 */
static INT32S adv_get_devinfo(adv_device *device, VOID *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_devinfo devinfo;
	

	devinfo.iobase = privdata->iobase;
	devinfo.iolength = privdata->iolength;
	devinfo.pci_slot = privdata->pci_slot;
	devinfo.pci_bus = privdata->pci_bus;
	devinfo.switchID = privdata->board_id;
	devinfo.deviceID = privdata->device_type;
	devinfo.irq = privdata->irq;
	
	if (copy_to_user(arg, &devinfo, sizeof(adv_devinfo))) {
		return EFAULT;
	}

	return 0;
}

/**
 * adv_ioctrl - The advdrv_core driver calls to handle the ioctl commands
 *
 * 
 * @device: Points to the device object
 * @cmd: Io control code from user
 * @arg: Argument from user
 */
static INT32S adv_ioctrl(adv_device *device, INT32U cmd, PTR_T arg)
{
/* 	private_data *privdata = (private_data *) (device->private_data); */
	INT32S ret = 0;


	if (device == NULL) {
		return InvalidDeviceHandle;
	}
	
	switch (cmd) {
	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;
	case ADV_PORTREAD:
		ret = adv_read_port(device, (void *) arg);
		break;
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (void *) arg);
		break;
	case GET_DEVICE_DRIVER_NAME:
		ret = adv_get_device_name(device, (void *) arg);
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (VOID *) arg);
		break;
	default:
		return -ENOSYS;
	}
/* 	adv_process_info_set_error(&privdata->ptr_process_info, ret);	 */
/* 	if (ret) { */
/* 		return -1; */
/* 	} */

	return ret;
}


/* PCI-1780 fops */
adv_general_ops dev1780_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.poll		= adv_poll,
};

/* PCI-1780 driver object */
adv_driver pci1780_driver = {
	.driver_name	= "pci1780",
	.devfs_name	= "pci1780",
	.ops		= &dev1780_fops,
};
 

/**
 * advdrv_init_one - Pnp to initialize the device, and allocate resource for the device.
 *
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

	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		printk("Could not kmalloc space for device!");
		return -ENOBUFS;
	}
	memset(device, 0, sizeof(adv_device));
	

	/* alloc & initialize the private data structure */
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		return -ENOMEM;
	}

	/* Initialize the private data in the device */
	memset(privdata, 0, sizeof(private_data));
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->device_type = dev->subsystem_device; /* multi-card support for new driver */     
	privdata->device_type <<= 16;
	privdata->device_type |= dev->device;
	privdata->irq = dev->irq;
	privdata->iobase = dev->resource[2].start & ~1UL;
	privdata->iolength = dev->resource[2].end - dev->resource[2].start;


	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);
	tasklet_init(&privdata->evt_tasklet, evt_tasklet_fn, (PTR_T) device);


	/* support multi-card */
	switch (privdata->device_type) {
	case PCI1780R1:
	case PCI1780R2:
	case PCI1780R3:
		privdata->device_type = PCI1780R2;
		privdata->board_id = (INT16U) (advInp(privdata, 0x4e) & 0x0f);
		advdrv_device_set_devname(device, "pci1780");
		break;
	}

	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	pci_set_drvdata(dev, device);

	/* request I/O regions */
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1780") == NULL) {
		kfree(device);
		kfree(privdata);
		printk("Request region failed\n");
		return  ret;
	}

	/* request irq */
	ret = request_irq(privdata->irq, pci1780_interrupt_handler,
			  SA_SHIRQ, "adv1780", privdata);
	if (ret != 0) {
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);
		printk("Request IRQ failed\n");
		return ret;
	}


	/* add device into driver list */
	ret = advdrv_add_device(&pci1780_driver, device);
	if (ret) {
		release_region(privdata->iobase, privdata->iolength);
		free_irq(privdata->irq, privdata);
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return ret;
	}

	printk("Add a PCI-%x device: iobase=%xh; slot=%xh; irq=%d\n", 
	       dev->device,
	       privdata->iobase,
	       privdata->pci_slot,
	       privdata->irq);
     
	return 0;
}

/**
 * advdrv_remove_one - Pnp to remove a device, and free resource for the device.
 * 
 * @dev: Points to the pci_dev device; 
 */
static void __devexit advdrv_remove_one(struct pci_dev *dev)
{
	private_data *privdata = NULL; 
	adv_device *device = NULL;


	printk("Remove a PCI-%x device\n", dev->device);

	device = pci_get_drvdata(dev);
	privdata = (private_data *) device->private_data;

	release_region(privdata->iobase, privdata->iolength);
	free_irq(privdata->irq, privdata);
	
	pci_set_drvdata(dev, NULL);
	advdrv_remove_device(&pci1780_driver, device);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}


struct pci_driver pci_driver = {
	name:	"pci1780",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
	id_table:	serial_pci_tbl,
};


/**
 * pci1780_init - The module initialize PCI-1780 driver
 *
 * Enumerate all PCI cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init pci1780_init(void)
{
	INT32S error;
	struct semaphore *sema = NULL;


	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return -ENOMEM;
	}

	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1780_driver, sema);


	advdrv_register_driver(&pci1780_driver);
	error = pci_module_init(&pci_driver);
	if (error != 0) {
		advdrv_unregister_driver(&pci1780_driver);
		return error;
	}

	return 0;
}

/**
 * pci1780_exit - Exit PCI-1780 driver
 */
static void __exit pci1780_exit(void)
{
	struct semaphore *sema = NULL;


	pci_unregister_driver(&pci_driver);
	advdrv_unregister_driver(&pci1780_driver);
	_ADV_GET_DRIVER_SEMA(pci1780_driver, sema);
     
	kfree(sema);
}

module_init(pci1780_init);
module_exit(pci1780_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1780 device driver module");
