/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *	Advantech PCI-1710 Series Device driver for Linux
 *		
 * (pci1710, pci1710L, pci1710HG, pci1710HGL, pci1711, pci1711L)
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1710 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	1/17/2006			Create by LI.ANG                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
/* #include <linux/config.h> */
#include <linux/module.h>
 

#include "PCI1710.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] =
{
	{       0x13fe, 0x1710,	/* PCI-1710S */
		0x10b5, 0x9050,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710 */
		0x13fe, 0x0000,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710B */
		0x13fe, 0xb100,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710B2 */
		0x13fe, 0xb200,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710C */
		0x13fe, 0xc100,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710C2 */
		0x13fe, 0xc200,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710L */
		0x13fe, 0x0001,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710LB */
		0x13fe, 0xb101,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710LB2 */
		0x13fe, 0xb201,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710LC */
		0x13fe, 0xc101,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710LC2 */
		0x13fe, 0xc201,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HG */
		0x13fe, 0x0002,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGB */
		0x13fe, 0xb102,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGB2 */
		0x13fe, 0xb202,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGC */
		0x13fe, 0xc102,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGC2 */
		0x13fe, 0xc202,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGL */
		0x13fe, 0x0003,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGLB */
		0x13fe, 0xb103,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGLB2 */
		0x13fe, 0xb203,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGLC */
		0x13fe, 0xc103,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGLC2 */
		0x13fe, 0xc203,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710U */
		0x1000, 0xD100,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710UL */
		0x1000, 0xD101,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGU */
		0x1000, 0xD102,
		0, 0, 0
	},
	{       0x13fe, 0x1710,	/* PCI-1710HGUL */
		0x1000, 0xD103,
		0, 0, 0
	},
	{       0x13fe, 0x1711,	/* PCI-1711S */
		0x10b5, 0x9050,
		0, 0, 0
	},
	{       0x13fe, 0x1711,	/* PCI-1711B */
		0x13fe, 0xb100,
		0, 0, 0
	},
	{       0x13fe, 0x1731,	/* PCI-1711LS */
		0x10b5, 0x9050,
		0, 0, 0
	},
	{       0x13fe, 0x1711,	/* PCI-1711L */
		0x13fe, 0x0001,
		0, 0, 0
	},
	{       0x13fe, 0x1711,	/* PCI-1711LB */
		0x13fe, 0xb101,
		0, 0, 0
	},
	{       0x13fe, 0x1711,	/* PCI-1711U */
		0x1000, 0xc100,
		0, 0, 0
	},
	{       0x13fe, 0x1711,	/* PCI-1711UL */
		0x1000, 0xc101,
		0, 0, 0
	},
	{       0x13fe, 0x1713,	/* PCI-1713 */
		0x10b5, 0x9050,
		0, 0, 0
	},
	{       0x13fe, 0x1713,	/* PCI-1713 */
		0x1000, 0xb100,
		0, 0, 0
	},
	{       0x13fe, 0x3713,	/* MIC-1713 */
		0x10b5, 0x9050,
		0, 0, 0
	},
/* 	{       0x13fe, 0x1741,	/\* PCI-1741U *\/ */
/* 		0x13fe, 0xa100, */
/* 		0, 0, 0 */
/* 	}, */
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
	
	ret = adv_process_info_add(&privdata->ptr_process_info, 6);
	
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
	INT32S ret = 0;


	if (device == NULL) {
		return -ENODEV;
	}

	
	switch (cmd) {
	case ADV_DIO_BITS:
		ret = adv_dio_ioctl(device, (VOID *) arg);
		break;
	case ADV_AI_INT_START:
		ret = adv_fai_int_start(device, (VOID *) arg);
		break;
	case ADV_AI_INT_SCAN_START:
		ret = adv_fai_int_scan_start(device, (VOID *) arg);
		break;
	case ADV_AI_STOP:
		ret = adv_fai_stop(device);	
		break;
	case FAICHECK:
		ret = adv_fai_check(device, (VOID *) arg);
		break;
	case CLEARFLAG:
		ret = adv_clear_flag(device, (VOID *) arg);
		break;
	case ADV_ENABLE_EVENT: 
		ret = adv_enable_event(device, (VOID *) arg);
		break;
	case CHECKEVENT:
		ret = adv_check_event(device, (VOID *) arg);
		break;
	case ADV_PORTREAD:
		ret = adv_read_port(device, (VOID *) arg);
		break;
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (VOID *) arg);
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (VOID *) arg);
		break;
	case ADV_FAI_INT_STATUS:
	case ADV_FAI_INTSCAN_STATUS:
		ret = adv_get_fai_status(device, (void *)arg);
		break;
	default:
		return -EINVAL;
	}

	return ret;
}


/* PCI-1710 fops */
adv_general_ops dev1710_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.poll		= adv_poll,
};

/* PCI-1710 driver object */
adv_driver pci1710_driver = {
	.owner = THIS_MODULE,
/* 	.mod_name = KBUILD_MODNAME, */
	.driver_name	= "pci1710",
	.devfs_name	= "pci1710",
	.ops		= &dev1710_fops,
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
/* 	struct semaphore *fai_sema = NULL; */
	adv_device *device = NULL;
	INT32S ret;
     
	
	if ((ret = pci_enable_device(dev)) != 0) {
		KdPrint(KERN_ERR": pci_enable_device failed\n");
		return ret;
	}

	/* allocate urb sema */
/* 	fai_sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL); */
/* 	if (fai_sema == NULL) { */
/* 		return -ENOMEM; */
/* 	} */
/* 	init_MUTEX(fai_sema); */

	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		KdPrint("Could not kmalloc space for device!");
/* 		kfree(fai_sema); */
		return -ENOMEM;
	}
	memset(device, 0, sizeof(adv_device));
	

	/* alloc & initialize the private data structure */
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
/* 		kfree(fai_sema); */
		kfree(device);
		return -ENOMEM;
	}


	/* Initialize the private data in the device */
	memset(privdata, 0, sizeof(private_data));
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->irq = dev->irq;
	privdata->iobase = dev->resource[2].start & ~1UL;
	privdata->iolength = dev->resource[2].end - dev->resource[2].start;
	privdata->device_type = dev->subsystem_device; /* multi-card support for new driver */     
	privdata->device_type <<= 16;
	privdata->device_type |= dev->device;
/* 	privdata->fai_sema = fai_sema; */

	printk(KERN_DEBUG "VENDOR:(0x%x) DEVICE:(0x%x) SUBVENDOR:(0x%x) SUBDEVICE:(0x%x)\n",
	       dev->vendor, dev->device, dev->subsystem_vendor,
	       dev->subsystem_device);
	


	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);
	INIT_WORK(&privdata->fai_stop_work, adv_fai_stop_work, (VOID *) device);

	printk(KERN_DEBUG"IOBASE:(0x%x)\n",privdata->iobase);
	
	/* request I/O regions */
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1710") == NULL) {
/* 		kfree(fai_sema); */
		kfree(device);
		kfree(privdata);
		KdPrint("Request region failed\n");
		return  ret;
	}

	/* request irq */
	ret = request_irq(privdata->irq, pci1710_interrupt_handler,
			  SA_SHIRQ, "adv1710", privdata);
	if (ret != 0) {
/* 		kfree(fai_sema); */
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);
		KdPrint("Request IRQ failed\n");
		return ret;
	}

	/* support multi-card */
/* 	printk("device_type: 0x%x\n", privdata->device_type); */
	
	switch (privdata->device_type) {
	case PCI1710S:
	case PCI1710:
	case PCI1710B:
	case PCI1710B2:
	case PCI1710C:
	case PCI1710C2:
		advdrv_device_set_devname(device, "pci1710");
		privdata->device_type = PCI1710;
		break;
	case PCI1710U:
		advdrv_device_set_devname(device, "pci1710U");
		privdata->device_type = PCI1710;

		break;
		
	case PCI1710L:
	case PCI1710LB:
	case PCI1710LB2:
	case PCI1710LC:
	case PCI1710LC2:
		advdrv_device_set_devname(device, "pci1710L");
		privdata->device_type = PCI1710L;
		break;
	case PCI1710UL:
		advdrv_device_set_devname(device, "pci1710UL");
		privdata->device_type = PCI1710L;

		break;
		
	case PCI1710HG:
	case PCI1710HGB:
	case PCI1710HGB2:
	case PCI1710HGC:
	case PCI1710HGC2:
		advdrv_device_set_devname(device, "pci1710HG");
		privdata->device_type = PCI1710HG;
		break;
	case PCI1710HGU:
		advdrv_device_set_devname(device, "pci1710HGU");
		privdata->device_type = PCI1710HG;
		break;
		
	case PCI1710HGL:
	case PCI1710HGLB:
	case PCI1710HGLB2:
	case PCI1710HGLC:
	case PCI1710HGLC2:
		advdrv_device_set_devname(device, "pci1710HGL");
		privdata->device_type = PCI1710HGL;
		break;
	case PCI1710HGUL:
		advdrv_device_set_devname(device, "pci1710HGUL");
		privdata->device_type = PCI1710HGL;
		break;
		
	case PCI1711:
	case PCI1711B:
		advdrv_device_set_devname(device, "pci1711");
		privdata->device_type = PCI1711;
		break;
	case PCI1711U:
		advdrv_device_set_devname(device, "pci1711U");
		privdata->device_type = PCI1711;

		break;
		
	case PCI1711LS:
	case PCI1711L:
	case PCI1711LB:
		advdrv_device_set_devname(device, "pci1711L");
		privdata->device_type = PCI1711L;
		break;
	case PCI1711UL:
		advdrv_device_set_devname(device, "pci1711UL");
		privdata->device_type = PCI1711L;
		break;
		
	case PCI1713:
		advdrv_device_set_devname(device, "pci1713");
		break;
	case PCI1713U:
		advdrv_device_set_devname(device, "pci1713U");
                privdata->device_type = PCI1713;
		break;

	case MIC3713:
		advdrv_device_set_devname(device, "mic3713");
		break;

/* 	case PCI1741U: */
/* 		advdrv_device_set_devname(device, "pci1741U"); */
/* 		break; */
	default:
		break;
	}

	if(dev->device == 0x1710) {
		if (dev->subsystem_device >= 0xB100) {
			privdata->board_id = (INT16U) (advInp(privdata, 0x14) & 0x0f);
		} else {
			privdata->board_id = 0;
		}
	} else if(dev->device == 0x1711) {
		if (dev->subsystem_device == 0xC100 || dev->subsystem_device == 0xC101) {
			privdata->board_id = (INT16U) (advInp(privdata, 0x14) & 0x0f);
		} else {
			privdata->board_id = 0;
		}
	} else if(dev->device == 0x1713) {
		if (dev->subsystem_device >= 0xB100) {
			privdata->board_id = (INT16U) (advInp(privdata, 0x14) & 0x0f);
		} else {
			privdata->board_id = 0;
		}
		
	} else if(dev->device == 0x3713) {
		privdata->board_id = (INT16U) (advInp(privdata, 0x2) & 0x0f);
		
	}
	
	
	
	
	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	pci_set_drvdata(dev, device);


	/* add device into driver list */
	ret = advdrv_add_device(&pci1710_driver, device);
	if (ret) {
/* 		kfree(fai_sema); */
		release_region(privdata->iobase, privdata->iolength);
		free_irq(privdata->irq, privdata);
		kfree(device);
		kfree(privdata);
		KdPrint("Add device failed!\n");
		return ret;
	}

	KdPrint("Add a PCI-%x device: board id=%xh; iobase=%xh; irq=%d; slot=%xh\n\n", 
		dev->device,
		privdata->board_id,
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
static VOID __devexit advdrv_remove_one(struct pci_dev *dev)
{
	private_data *privdata = NULL; 
	adv_device *device = NULL;


	KdPrint("Remove a PCI-%x device\n", dev->device);

	device = pci_get_drvdata(dev);
	privdata = (private_data *) device->private_data;

	release_region(privdata->iobase, privdata->iolength);
	free_irq(privdata->irq, privdata);
/* 	kfree(privdata->fai_sema); */
	
	pci_set_drvdata(dev, NULL);
	advdrv_remove_device(&pci1710_driver, device);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}


struct pci_driver pci_driver = {
	name:	"pci1710",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
	id_table:	serial_pci_tbl,
};


/**
 * pci1710_init - The module initialize PCI-1710 driver
 *
 * Enumerate all PCI cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init pci1710_init(VOID)
{
	INT32S error;
	struct semaphore *sema = NULL;


	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return -EFAULT;
	}

	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1710_driver, sema);


	advdrv_register_driver(&pci1710_driver);
	error = pci_module_init(&pci_driver);
	if (error != 0) {
		advdrv_unregister_driver(&pci1710_driver);
		return error;
	}

	return 0;
}

/**
 * pci1710_exit - Exit PCI-1710 driver
 */
static VOID __exit pci1710_exit(VOID)
{
	struct semaphore *sema = NULL;


	pci_unregister_driver( &pci_driver );
	advdrv_unregister_driver( &pci1710_driver );
	_ADV_GET_DRIVER_SEMA(pci1710_driver, sema);
     
	kfree(sema);
}

module_init(pci1710_init);
module_exit(pci1710_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Li Ang <li.ang@advantech.com.cn>");
MODULE_DESCRIPTION("Advantech PCI1710 device driver module");
