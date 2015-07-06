/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech PCI-1721 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1721 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	1/17/2006			Create by LI.ANG                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
/* #include <linux/config.h> */
#include <linux/module.h>
 

#include "PCI1721.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] =
{
	{       0x13fe, 0x1721,	/* PCI-1721 */
		0x10b5, 0x9054,
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
	
	ret = adv_process_info_add(&privdata->ptr_process_info, 4);
	
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

/* 	if (privdata->ao_running) { */
/* 		adv_fao_stop(device); */
/* 	} */

	
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
static INT32S adv_get_devinfo(adv_device *device, void *arg)
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
		return -EFAULT;
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
	INT32S ret = 0;


	if (device == NULL) {
		return -ENODEV;
	}
     
	switch (cmd) {
	case ADV_AO_DMA_START:
		ret = adv_fao_dma_start(device, (void *) arg);
		break;
	case ADV_AO_STOP:
		ret = adv_fao_stop(device);
		break;
	case ADV_AO_CHECK:
		ret = adv_fao_check(device, (void *) arg);
		break;
	case ADV_DIO_BITS:
		ret = adv_dio_ioctl(device, (VOID *) arg);
		break;
	case ADV_PORTREAD:
		ret = adv_read_port(device, (void *) arg);
		break;
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (void *) arg);
		break;
	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;
	case CLEARFLAG:
		ret = adv_clear_flag(device, (void *) arg);
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (void *) arg);
		break;
	default:
		return -EINVAL;
	}

	return ret;
}


/* PCI-1721 fops */
adv_general_ops dev1721_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.poll           = adv_poll,
};

/* PCI-1721 driver object */
adv_driver pci1721_driver = {
	.driver_name	= "pci1721",
	.devfs_name	= "pci1721",
	.ops		= &dev1721_fops,
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
		KdPrint(KERN_ERR": pci_enable_device failed\n");
		return ret;
	}

	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		KdPrint("Could not kmalloc space for device!");
		return -ENOMEM;
	}
	memset(device, 0, sizeof(adv_device));
	

	/* alloc & initialize the private data structure */
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		return -ENOMEM;
	}

	/* initialize the private data in the device */
	memset(privdata, 0, sizeof(private_data));
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->irq = dev->irq;
	privdata->ioDMAbase = dev->resource[1].start & ~1UL;
	privdata->ioDMAlength = dev->resource[1].end - dev->resource[1].start;
	privdata->iobase = dev->resource[2].start & ~1UL;
	privdata->iolength = dev->resource[2].end - dev->resource[2].start;
	privdata->device_type = dev->device; /* multi-card support for new driver */     
	privdata->board_id = (INT16U) (advInp(privdata, 0x10) & 0x0f);

	/* support multi-card */
	switch (privdata->device_type) {
	case PCI1721:
		privdata->timer_clock = 10;
		advdrv_device_set_devname(device, "pci1721");
		break;
	default:
		return -ENODEV;
	}
	

	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);
	tasklet_init(&privdata->fao_tasklet, fao_tasklet_fn, (PTR_T) device);
	INIT_WORK(&privdata->fao_stop_work, adv_fao_stop_work, (VOID *) device);

	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	pci_set_drvdata(dev, device);


	/* request I/O regions */
 	if (request_region(privdata->ioDMAbase, privdata->ioDMAlength, "PCI-1721") == NULL) {
		kfree(device);
		kfree(privdata);
		KdPrint("Request region failed\n");
		return ret;
	}

	
 	if (request_region(privdata->iobase, privdata->iolength, "PCI-1721") == NULL) {
		release_region(privdata->ioDMAbase, privdata->ioDMAlength);
		kfree(device);
		kfree(privdata);

		KdPrint("Request region failed\n");
		return ret;
	}

	/* request irq */
	ret = request_irq(privdata->irq, pci1721_interrupt_handler, SA_SHIRQ, "adv1721", device);
	if (ret != 0) {
		release_region(privdata->ioDMAbase, privdata->ioDMAlength);
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);

		KdPrint("Request IRQ failed\n");
		return ret;
	}
	

	/* add device into driver list */
	ret = advdrv_add_device(&pci1721_driver, device);
	if (ret) {
		release_region(privdata->ioDMAbase, privdata->ioDMAlength);
		release_region(privdata->iobase, privdata->iolength);
		free_irq(privdata->irq, privdata);
		kfree(device);
		kfree(privdata);

		KdPrint("Add device failed!\n");
		return ret;
	}

	KdPrint("Add a PCI-%x device: ioDMAbase=%xh; iobase=%xh; irq=%d\n; slot=%xh\n", 
		dev->device,
		privdata->ioDMAbase,
		privdata->iobase,
		privdata->irq,
		privdata->pci_slot);
     
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


	KdPrint("Remove a PCI-%x device\n", dev->device);

	device = pci_get_drvdata(dev);
	privdata = (private_data *) device->private_data;

	release_region(privdata->ioDMAbase, privdata->ioDMAlength);
	release_region(privdata->iobase, privdata->iolength);
	free_irq(privdata->irq, privdata);
	
	pci_set_drvdata(dev, NULL);
	advdrv_remove_device(&pci1721_driver, device);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}

/**
 * advdrv_suspend_one - Pnp to suspend a device
 *
 * @dev: Points to the pci_dev device;       
 */
/* static INT32S advdrv_suspend_one(struct pci_dev *dev, u32 state) */
/* { */
/* 	return 0; */
/* } */

/**
 * advdrv_resume_one - Pnp to resume a device
 *
 * @dev: Points to the pci_dev device;       
 */
/* static INT32S advdrv_resume_one(struct pci_dev *dev) */
/* { */

/* 	return 0; */
/* } */


struct pci_driver pci_driver = {
	name:	"pci1721",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
	/* suspend:	advdrv_suspend_one, */
	/* resume:	advdrv_resume_one, */
	id_table:	serial_pci_tbl,
};


/**
 * pci1721_init - The module initialize PCI-1721 driver
 *
 * Enumerate all PCI cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init pci1721_init(void)
{
	INT32S error;
	struct semaphore *sema = NULL;


	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return MemoryAllocateFailed;
	}

	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1721_driver, sema);


	advdrv_register_driver(&pci1721_driver);
	error = pci_module_init(&pci_driver);
	if (error != 0) {
		advdrv_unregister_driver(&pci1721_driver);
		return error;
	}

	return 0;
}

/**
 * pci1721_exit - Exit PCI-1721 driver
 */
static void __exit pci1721_exit(void)
{
	struct semaphore *sema = NULL;


	pci_unregister_driver( &pci_driver );
	advdrv_unregister_driver( &pci1721_driver );
	_ADV_GET_DRIVER_SEMA(pci1721_driver, sema);
     
	kfree(sema);
}

module_init(pci1721_init);
module_exit(pci1721_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1721 device driver module");
MODULE_AUTHOR("Li Ang <li.ang@advantech.com.cn>");
