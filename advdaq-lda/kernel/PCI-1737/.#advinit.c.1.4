/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech PCI-1737 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1737 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	10/23/2006			Create by zhenyu.zhang                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
#include <linux/module.h>
 

#include "PCI1737.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] =
{
	{       0x13fe, 0x1737,	/* PCI-1737 */
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, 0
	},
	{       0x13fe, 0x1739,	/* PCI-1739 */
		PCI_ANY_ID, PCI_ANY_ID,
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
	
	return ret;
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
	kdev_info->switchID = privdata->board_id;
	kdev_info->deviceID = privdata->device_type;

	if (copy_to_user(udev_info, kdev_info, sizeof(adv_devinfo))){
		ret = -EFAULT;
	}
	
	kfree(kdev_info);
	return ret;
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
	private_data *privdata = (private_data *) (device->private_data);
	INT32S ret = 0;

	if (device == NULL)
		return -EFAULT;
     
	switch (cmd) {
	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;
	case ADV_DEVGETPROPERTY:
		ret = adv_dev_get_property(device, (void *) arg);
		break;
	case ADV_DEVSETPROPERTY:
		ret = adv_dev_set_property(device, (void *) arg);
		break;
	case ADV_PORTREAD:
		ret = adv_read_port(device, (void *) arg);
		break;
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (void *) arg);
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (adv_devinfo *) arg);
		break;
	default:
		return -EFAULT;
	}
	
	adv_process_info_set_error(&privdata->ptr_process_info, ret);
	
	if (ret) {
		return -EFAULT;
	}

	return 0;
}


/* PCI-1737 fops */
adv_general_ops dev1737_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.poll           = adv_poll,
};

/* PCI-1737 driver object */
adv_driver pci1737_driver = {
	.driver_name	= "pci1737",
	.devfs_name	= "pci1737",
	.ops		= &dev1737_fops,
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
	INT32U device_type;
	
     
     
	if ((ret = pci_enable_device(dev)) != 0) {
		printk(KERN_ERR": pci_enable_device failed\n");
		return ret;
	}

	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		printk("Could not kmalloc space for device!");
		return -ENOMEM;
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
	privdata->device_type = dev->device; /* multi-card support for new driver */     
	device_type = (dev->device << 16) | dev->subsystem_device;
	switch(device_type) {
	case PCI1737RA101:
	case PCI1739RA101:
		privdata->iobase = dev->resource[0].start & ~1UL;
		privdata->iolength = dev->resource[0].end - dev->resource[0].start;
		break;
	case PCI1737RA102:
	case PCI1739RA102:
		privdata->iobase = dev->resource[2].start & ~1UL;
		privdata->iolength = dev->resource[2].end - dev->resource[2].start;
		break;
	default:
		break;
	}

	privdata->irq = dev->irq;
	switch (privdata->device_type) {
	case PCI1737:
		privdata->port_num = 3;
		privdata->port_group = 1;
		privdata->ulSW = advInp(privdata,3);
		privdata->di_port_cnt = 3;
		privdata->di_channel_cnt = 3 * 8;
		privdata->do_port_cnt = 3;
		privdata->do_channel_cnt = 3 * 8;

		if(privdata->ulSW & 0x10) /* PA */
			privdata->usDioPortDir[0] = 0; /* input */
		else
			privdata->usDioPortDir[0] = 1; /* output */
		  
		if(privdata->ulSW & 0x02)	/* PB */
			privdata->usDioPortDir[1] = 0; /* input */
		else
			privdata->usDioPortDir[1] = 1; /* output */
		  
		if(privdata->ulSW & 0x01){
			/* PCH and PCL are all Input */
			if(privdata->ulSW & 0x08){
				privdata->usDioPortDir[2] = 0; 
			}else{
				/* PCH is output, PCL is Input */
				privdata->usDioPortDir[2] = 3; 
			}
		}else{
			if(privdata->ulSW & 0x08)
				privdata->usDioPortDir[2] = 4; /* PCH is input and PCL is output */
			else
				privdata->usDioPortDir[2] = 3; /* PCH and PCL is output */
		}
		break;
	case PCI1739:
		privdata->port_num = 6;
		privdata->port_group = 2;
		privdata->ulSW = advInp(privdata,3);
		privdata->di_port_cnt = 6;
		privdata->di_channel_cnt = 6 * 8;
		privdata->do_port_cnt = 6;
		privdata->do_channel_cnt = 6 * 8;

		if(privdata->ulSW & 0x10) /* PA */
			privdata->usDioPortDir[0] = 0; /* input */
		else
			privdata->usDioPortDir[0] = 1; /* output */
		  
		if(privdata->ulSW & 0x02)	/* PB */
			privdata->usDioPortDir[1] = 0; /* input */
		else
			privdata->usDioPortDir[1] = 1; /* output */
		  
		if(privdata->ulSW & 0x01){
			/* PCH and PCL are all Input */
			if(privdata->ulSW & 0x08){
				privdata->usDioPortDir[2] = 0; 
			}else{
				/* PCH is output, PCL is Input */
				privdata->usDioPortDir[2] = 3; 
			}
		}else{
			if(privdata->ulSW & 0x08)
				privdata->usDioPortDir[2] = 4; /* PCH is input and PCL is output */
			else
				privdata->usDioPortDir[2] = 3; /* PCH and PCL is output */
		}

		privdata->ulSW = advInp(privdata,7);

		if(privdata->ulSW & 0x10) /* PA1 */
			privdata->usDioPortDir[3] = 0; /* input */
		else
			privdata->usDioPortDir[3] = 1; /* output */
		  
		if((privdata->ulSW & 0x02) == 1)	/* PB1 */
			privdata->usDioPortDir[4] = 0; /* input */
		else
			privdata->usDioPortDir[4] = 1; /* output */
		  
		if(privdata->ulSW & 0x01)		{
			/* PCH and PCL are all Input */
			if(privdata->ulSW & 0x08){
				privdata->usDioPortDir[5] = 0; 
			}else{
				/* PCH is output, PCL is Input */
				privdata->usDioPortDir[5] = 3; 
			}
		}else{
			if(privdata->ulSW & 0x08)
				privdata->usDioPortDir[5] = 4; /* PCH is input and PCL is output */
			else
				privdata->usDioPortDir[5] = 3; /* PCH and PCL is output */
		}
	
		break;
	}

	
	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);

	/* request I/O regions*/
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1737") == NULL) {
		kfree(device);
		kfree(privdata);
		printk("Request region failed\n");
		return  ret;
	}

	/* request irq*/ 
	ret = request_irq(privdata->irq, pci1737_interrupt_handler, SA_SHIRQ, "adv1737", privdata); 
	if (ret != 0) {
		kfree(device);
		kfree(privdata);
		printk("Request IRQ failed\n");
		return ret;
	}

	/* support multi-card */
	switch (privdata->device_type) {
	case PCI1737:
		privdata->board_id = (INT32U) (advInp(privdata, 0x09) & 0x0f);
		advdrv_device_set_devname(device, "pci1737u");
		break;
	case PCI1739:
		privdata->board_id = (INT32U) (advInp(privdata, 0x09) & 0x0f);
		advdrv_device_set_devname(device, "pci1739u");
		break;
	}
	
	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);

	pci_set_drvdata(dev, device);

	/* add device into driver list */
	ret = advdrv_add_device(&pci1737_driver, device);

	if (ret) {
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
        
        free_irq(privdata->irq, privdata);

	release_region(privdata->iobase, privdata->iolength);

	advdrv_remove_device(&pci1737_driver, device);
	pci_set_drvdata(dev, NULL);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}

struct pci_driver pci_driver = {
	name:	"pci1737",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
	id_table:	serial_pci_tbl,
};


/**
 * pci1737_init - The module initialize PCI-1737 driver
 *
 * Enumerate all PCI cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init pci1737_init(void)
{
	INT32S error;
	struct semaphore *sema = NULL;

	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return -ENOMEM;
	}
	
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1737_driver, sema);

	advdrv_register_driver(&pci1737_driver);
	error = pci_module_init(&pci_driver);
	if (error != SUCCESS) {
		advdrv_unregister_driver(&pci1737_driver);
		return error;
	}

	return 0;
}

/**
 * pci1737_exit - Exit PCI-1737 driver
 */
static void __exit pci1737_exit(void)
{
	struct semaphore *sema = NULL;

	pci_unregister_driver( &pci_driver );
	advdrv_unregister_driver( &pci1737_driver );
	      

	_ADV_GET_DRIVER_SEMA(pci1737_driver, sema);
	kfree(sema);
}


module_init(pci1737_init);
module_exit(pci1737_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1737 device driver module");
