/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech PCI-1784 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1784 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	10/23/2006			Create by zhenyu.zhang                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
/* #include <linux/config.h> */
#include <linux/module.h>
 

#include "PCI1784.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] =
{
	{       0x13fe, 0x1784,	/* PCI-1784 */
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
	
	ret = adv_process_info_add(&privdata->ptr_process_info, 25);
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
	advOutpdw(privdata,0x20,0);	
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
		return InvalidDeviceHandle;
     
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
		
	case ADV_GET_ERR_CODE:
		ret = adv_get_err_code(device, (void *) arg);
		break;		
		
	case ADV_GETCOUNTERSTATUS:
		ret = adv_get_counter_status(device,NULL, (void *) arg);
		break;
		
	case ADV_SETCOUNTERSTATUS:
		ret = adv_set_counter_status(device,NULL, (void *) arg);
		break;


	/*case ADV_SETCOUNTERPARAM:
		ret = adv_CntrSetParam(device,(void *) arg);
		break;
	*/	
	case ADV_GETCOUNTERPARAM:
		ret = adv_CntrGetParam(device,(void *) arg);
		break;

	case ADV_DIOSETEVENTCFG:
		ret = adv_DioSetEventConfig(device, (void *) arg);
		break;
		
	case ADV_DIOGETEVENTDATA:
		ret = adv_DioGetEventData(device, (void *) arg);
		break;

	case ADV_CNTRSETEVENTCFG:
		ret = adv_CntrSetEventConfig(device, (void *) arg);
		break;
		
	case ADV_CNTRGETEVENTDATA:
		ret = adv_CntrGetEventData(device, (void *) arg);
		break;


		
	case ADV_PORTREAD:
		ret = adv_read_port(device, (void *) arg);
		break;
		
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (void *) arg);
		break;

	case ADV_SETCOUNTERPARAM:
		ret = adv_set_counter_param(device, (CTL_CNTR_PARAMS *) arg);
		break;
		
	default:
		return InvalidCommandType;
	}
	
	adv_process_info_set_error(&privdata->ptr_process_info, ret);
	
	if (ret) {
		return -1;
	}

	return 0;
}


/* PCI-1784 fops */
adv_general_ops dev1784_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
};

/* PCI-1784 driver object */
adv_driver pci1784_driver = {
	.driver_name	= "pci1784",
	.devfs_name	= "pci1784",
	.ops		= &dev1784_fops,
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
	privdata->device_type = dev->device; /* multi-card support for new driver */     
	privdata->irq = dev->irq;
	switch (privdata->device_type) {
	case PCI1784:
		privdata->iobase = dev->resource[2].start & ~1UL;
		privdata->iolength = dev->resource[2].end - dev->resource[2].start;

		break;
	}


	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);

	/* request I/O regions*/
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1784") == NULL) {
		kfree(device);
		kfree(privdata);
		printk("Request region failed\n");
		return  ret;
	}

	/* request irq*/ 
	ret = request_irq(privdata->irq, pci1784_interrupt_handler, SA_SHIRQ, "adv1784", privdata); 
	if (ret != 0) {
		kfree(device);
		kfree(privdata);
		release_region(privdata->iobase, privdata->iolength);
		printk("Request IRQ failed\n");
		return ret;
	}

	/* support multi-card */
	switch (privdata->device_type) {
	case PCI1784:
		privdata->board_id = (INT16U) (advInpdw(privdata, 0x28) & 0x000f);
		advdrv_device_set_devname(device, "pci1784");
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
	ret = advdrv_add_device(&pci1784_driver, device);

	if (ret) {
		kfree(device);
		kfree(privdata);
		release_region(privdata->iobase, privdata->iolength);
		printk("Add device failed!\n");
		return ret;
	}

	printk("Add a PCI-%x device: iobase=%xh; slot=%xh boardid=%xh\n", 
	       dev->device,
	       privdata->iobase,
	       privdata->pci_slot,privdata->board_id);
     
	return SUCCESS;
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

	advdrv_remove_device(&pci1784_driver, device);
	pci_set_drvdata(dev, NULL);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}

struct pci_driver pci_driver = {
	name:	"pci1784",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
	id_table:	serial_pci_tbl,
};


/**
 * pci1784_init - The module initialize PCI-1784 driver
 *
 * Enumerate all PCI cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init pci1784_init(void)
{
	INT32S error;
	struct semaphore *sema = NULL;

	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return MemoryAllocateFailed;
	}
	
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1784_driver, sema);

	advdrv_register_driver(&pci1784_driver);
	error = pci_module_init(&pci_driver);
	if (error != SUCCESS) {
		advdrv_unregister_driver(&pci1784_driver);
		return error;
	}

	return SUCCESS;
}

/**
 * pci1784_exit - Exit PCI-1784 driver
 */
static void __exit pci1784_exit(void)
{
	struct semaphore *sema = NULL;

	pci_unregister_driver( &pci_driver );
	advdrv_unregister_driver( &pci1784_driver );
	      

	_ADV_GET_DRIVER_SEMA(pci1784_driver, sema);
	kfree(sema);
}


module_init(pci1784_init);
module_exit(pci1784_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1784 device driver module");
