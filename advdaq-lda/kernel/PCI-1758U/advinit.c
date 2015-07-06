/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech PCI-1758U Device Driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1758U driver module and device.         
 *                                                                         
 * Version history		                                          
 *	9/12/2006			Create by LI.ANG
 *	
 ***************************************************************************/	
   

/* base module includes */
/* #include <linux/config.h> */
#include <linux/module.h>
 

#include "PCI1758U.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] = 
{
	{       0x13fe, 0x1758,	/* PCI-1758UDI */
		PCI_ANY_ID, 0xa100,
		0, 0, 0
	},
	{       0x13fe, 0x1758,	/* PCI-1758UDO */
		PCI_ANY_ID, 0xa101,
		0, 0, 0
	},
	{       0x13fe, 0x1758,	/* PCI-1758UDIO */
		PCI_ANY_ID, 0xa102,
		0, 0, 0
	},
	{       0x13fa, 0x1758,	/* PCI-1758UDIO old-version */
		PCI_ANY_ID, 0x0000,
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
	
	ret = adv_process_info_add(&privdata->ptr_process_info, 130);
	
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
 * adv_resetdevice - The advdrv_core driver calls to reset the device.
 * 
 * @device: Points to the device object
 */
static INT32S adv_resetdevice(adv_device *device)
{
	return 0;
}

/**
 * adv_mmap - The advdrv_core driver calls to map the kernel memory to user
 * 
 * @device: Points to the device object
 * @vma: vm_area_struct 
 */
static INT32S adv_mmap(adv_device *device, struct file *fp,
		       struct vm_area_struct *vma)
{
	return 0;
}

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
 * adv_ioctrl - The advdrv_core driver calls to handle the ioctl commands.
 * 
 * @device: Points to the device object
 * @cmd: io code.
 * @arg: argument
 */
static INT32S adv_ioctrl(adv_device *device, INT32U cmd, PTR_T arg)
{
	INT32S ret;

	
	switch (cmd) {
	case ADV_DIO_BITS:
		ret = adv_dio_ioctl(device, (void *) arg);
		break;
/* 	case ADV_COUNTER: */
/* 		ret = adv_cnt_ioctl(device, (void *) arg); */
/* 		break; */
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;
	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
	case CLEARFLAG:
		ret = adv_clear_flag(device, (void *) arg);
		break;
	case ADV_PORTREAD:
		ret = adv_read_port(device, (void *) arg);
		break;
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (void *) arg);
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (void *) arg);
		break;
/* 	case ADV_DEVGETPROPERTY: */
/* 		ret = adv_dev_get_property(device, (void *) arg); */
/* 		break; */
/* 	case ADV_DEVSETPROPERTY: */
/* 		ret = adv_dev_set_property(device, (void *) arg); */
/* 		break; */
	default:
		return -EINVAL;
	}

	return ret;
}

/* PCI-1758U fops */
adv_general_ops dev1758U_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.resetdevice	= adv_resetdevice,
	.mmap		= adv_mmap,
	.poll		= adv_poll,
};

/* PCI-1758U driver object */
adv_driver pci1758U_driver = {
	.driver_name	= "pci1758U",
	.devfs_name	= "pci1758U",
	.ops		= &dev1758U_fops,
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
		KdPrint(KERN_ERR": pci_enable_device failed\n");
		return ret;
	}
	
	/* initialize & zero the device structure */
	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		PCRIT("Could not kmalloc space for device!");
		return -ENOMEM;
	}
	memset(device, 0, sizeof(adv_device));
	
	/* alloc & initialize the private data structure */
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		return -ENOMEM;
	}
	memset(privdata, 0, sizeof(private_data));
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->device_type = dev->subsystem_device; /* multi-card support for new driver */     
	privdata->irq = dev->irq;
	privdata->iobase = dev->resource[2].start & ~1UL;
	privdata->iolength = dev->resource[2].end - dev->resource[2].start;
	
	
	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);

	/* request I/O regions */
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1758U") == NULL) {
		kfree(device);
		kfree(privdata);
		printk("Request region failed\n");
		return -EIO;
	}

	/* request irq */
	ret = request_irq(privdata->irq, pci1758U_interrupt_handler,
			  SA_SHIRQ, "pci1758U", privdata); 
	if (ret != 0) {
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);
		printk("Request IRQ failed\n");
		return ret;
	}

	/* support multi-card */	
	switch (privdata->device_type) {
	case PCI1758UDI:
		privdata->board_id = (INT16U) (advInp(privdata, 0x56) & 0x0f);
		advdrv_device_set_devname(device, "pci1758UDI");
		break;
	case PCI1758UDO:
		privdata->board_id = (INT16U) (advInp(privdata, 0x1c) & 0x0f);
		advdrv_device_set_devname(device, "pci1758UDO");
		break;
	case PCI1758UDIO:
	case PCI1758UDIOOV:
		privdata->device_type = PCI1758UDIO;
		privdata->board_id = (INT16U) (advInp(privdata, 0x3e) & 0x0f);
		advdrv_device_set_devname(device, "pci1758UDIO");
		adv_eeprom_enable(privdata);
		break;
	default:
		break;
	}

	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	pci_set_drvdata(dev, device);

	/* add device into driver list */
	ret = advdrv_add_device(&pci1758U_driver, device);
	if (ret != 0) {
		release_region(privdata->iobase, privdata->iolength);
		free_irq(privdata->irq, privdata);
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return ret;
	}

	printk("Add a PCI-%x device: iobase=%xh; irq=%xh; slot=%xh; card=%s\n", 
	       dev->device,
	       privdata->iobase,
	       privdata->irq,
	       privdata->pci_slot,
	       device->device_name);

	return 0;
}

/**
 * advdrv_remove_one - Pnp to remove a device, and free resource for the device.
 *
 * @dev: Points to the pci_dev device
 * 
 * pci_get_drvdata(dev) get pointer of the private data in adv_device.
 */
static void __devexit advdrv_remove_one(struct pci_dev *dev)
{
	private_data *privdata; 
	adv_device *device = NULL;
	privdata = NULL;

	printk("Remove a PCI-%x device\n", dev->device);

	device = pci_get_drvdata(dev);
	privdata = (private_data *) device->private_data;

	free_irq(privdata->irq, privdata);
	release_region(privdata->iobase, privdata->iolength);  

	pci_set_drvdata(dev, NULL);
	advdrv_remove_device(&pci1758U_driver, device);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}

/**
 * advdrv_suspend_one - Pnp to suspend a device *                                                                
 * 
 * @dev: Points to the pci_dev device
 */
/* static INT32S advdrv_suspend_one(struct pci_dev *dev, u32 state) */
/* { */
/* 	return 0; */
/* } */

/**
 * advdrv_resume_one - Pnp to resume a device
 * 
 * @dev: Points to the pci_dev device
 *
 * pci_get_drvdata(dev) get pointer of the private data in adv_device.
 */
/* static INT32S advdrv_resume_one(struct pci_dev *dev) */
/* { */
/* 	return 0; */
/* } */

struct pci_driver pci_driver = {
	name:	"pci1758U",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
/* 	suspend: advdrv_suspend_one, */
/* 	resume:	advdrv_resume_one, */
	id_table:	serial_pci_tbl,
};

/**
 * pci1761_init - The module initialize PCI-1761 driver, Enumerate all PCI cards on the
 * 		  bus, register the driver in Advantech core driver.
 */
static INT32S __init pci1758U_init(void)
{
	INT32S error;
	struct semaphore *sema = NULL;


	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return -ENOMEM;
	}

	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1758U_driver, sema);

	advdrv_register_driver(&pci1758U_driver);

	error = pci_module_init(&pci_driver);
	if (error != 0) {
		printk("pci module init fail!\n");
		advdrv_unregister_driver(&pci1758U_driver);
		return error;
	}

	return 0;
}

/**
 * pci1761_exit - Exit PCI-1761 driver
 */
static void __exit pci1758U_exit(void)
{
	struct semaphore *sema = NULL;


	pci_unregister_driver(&pci_driver);
	advdrv_unregister_driver(&pci1758U_driver);
	_ADV_GET_DRIVER_SEMA(pci1758U_driver, sema);
     
	kfree(sema);
}

module_init(pci1758U_init);
module_exit(pci1758U_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1758U device driver module");
MODULE_AUTHOR("Li Ang <li.ang@advantech.com.cn>");
