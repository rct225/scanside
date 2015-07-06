/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *	Advantech PCI-1741U Series Device driver for Linux
 *		
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1741U driver module and device.         
 *                                                                         
 * Version history		                                          
 *	1/17/2006			Create by LI.ANG                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
/* #include <linux/config.h> */
#include <linux/module.h>
 

#include "PCI1741U.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] =
{
	{       0x13fe, 0x1741,	/* PCI-1741U */
		0x13fe, 0xa100,
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
	default:
		return -EINVAL;
	}

	return ret;
}


/* PCI-1741U fops */
adv_general_ops dev1741U_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.poll		= adv_poll,
};

/* PCI-1741U driver object */
adv_driver pci1741U_driver = {
	.driver_name	= "pci1741U",
	.devfs_name	= "pci1741U",
	.ops		= &dev1741U_fops,
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

	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);
	INIT_WORK(&privdata->fai_stop_work, adv_fai_stop_work, (VOID *) device);

	/* request I/O regions */
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1741U") == NULL) {
/* 		kfree(fai_sema); */
		kfree(device);
		kfree(privdata);
		KdPrint("Request region failed\n");
		return ret;
	}

	/* request irq */
	ret = request_irq(privdata->irq, pci1741U_interrupt_handler,
			  SA_SHIRQ, "adv1741U", privdata);
	if (ret != 0) {
/* 		kfree(fai_sema); */
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);
		KdPrint("Request IRQ failed\n");
		return ret;
	}

	/* support multi-card */
	/* printk("device_type: 0x%x\n", privdata->device_type); */
	
	switch (privdata->device_type) {
/* 	case PCI1710S: */
/* 	case PCI1710: */
/* 	case PCI1710B: */
/* 	case PCI1710B2: */
/* 	case PCI1710C: */
/* 	case PCI1710C2: */
/* 		advdrv_device_set_devname(device, "pci1710"); */
/* 		privdata->device_type = PCI1710; */
/* 		break; */
/* 	case PCI1710L: */
/* 	case PCI1710LB: */
/* 	case PCI1710LB2: */
/* 	case PCI1710LC: */
/* 	case PCI1710LC2: */
/* 		advdrv_device_set_devname(device, "pci1710L"); */
/* 		privdata->device_type = PCI1710L; */
/* 		break; */
/* 	case PCI1710HG: */
/* 	case PCI1710HGB: */
/* 	case PCI1710HGB2: */
/* 	case PCI1710HGC: */
/* 	case PCI1710HGC2: */
/* 		advdrv_device_set_devname(device, "pci1710HG"); */
/* 		privdata->device_type = PCI1710HG; */
/* 		break; */
/* 	case PCI1710HGL: */
/* 	case PCI1710HGLB: */
/* 	case PCI1710HGLB2: */
/* 	case PCI1710HGLC: */
/* 	case PCI1710HGLC2: */
/* 		advdrv_device_set_devname(device, "pci1710HGL"); */
/* 		privdata->device_type = PCI1710HGL; */
/* 		break; */
/* 	case PCI1711: */
/* 	case PCI1711B: */
/* 		advdrv_device_set_devname(device, "pci1711"); */
/* 		privdata->device_type = PCI1711; */
/* 		break; */
/* 	case PCI1711LS: */
/* 	case PCI1711L: */
/* 	case PCI1711LB: */
/* 		advdrv_device_set_devname(device, "pci1711L"); */
/* 		privdata->device_type = PCI1711L; */
/* 		break; */
/* 	case PCI1713: */
/* 		advdrv_device_set_devname(device, "pci1713"); */
/* 		break; */
	case PCI1741U:
		advdrv_device_set_devname(device, "pci1741U");
		break;
	default:
		break;
	}
	privdata->board_id = (INT16U) (advInp(privdata, 0x14) & 0x0f);

	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	pci_set_drvdata(dev, device);


	/* add device into driver list */
	ret = advdrv_add_device(&pci1741U_driver, device);
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
	advdrv_remove_device(&pci1741U_driver, device);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}


struct pci_driver pci_driver = {
	name:	"pci1741U",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
	id_table:	serial_pci_tbl,
};


/**
 * pci1741U_init - The module initialize PCI-1741U driver
 *
 * Enumerate all PCI cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init pci1741U_init(VOID)
{
	INT32S error;
	struct semaphore *sema = NULL;


	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return -EFAULT;
	}

	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1741U_driver, sema);


	advdrv_register_driver(&pci1741U_driver);
	error = pci_module_init(&pci_driver);
	if (error != 0) {
		advdrv_unregister_driver(&pci1741U_driver);
		return error;
	}

	return 0;
}

/**
 * pci1741U_exit - Exit PCI-1741U driver
 */
static VOID __exit pci1741U_exit(VOID)
{
	struct semaphore *sema = NULL;


	pci_unregister_driver( &pci_driver );
	advdrv_unregister_driver( &pci1741U_driver );
	_ADV_GET_DRIVER_SEMA(pci1741U_driver, sema);
     
	kfree(sema);
}

module_init(pci1741U_init);
module_exit(pci1741U_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Li Ang <li.ang@advantech.com.cn>");
MODULE_DESCRIPTION("Advantech PCI1741U device driver module");
