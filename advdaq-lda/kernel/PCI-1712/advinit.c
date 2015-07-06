/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech PCI-1712 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1712 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	4/12/2006			Create by zhenyu.zhang                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
#include <linux/module.h>
#include "PCI1712.h"

#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] =
{
	{       0x13fe, 0x1712,	/* PCI-1712 */
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

	privdata->pVaAddrBlock = dma_alloc_coherent(NULL, 128, 
						    &privdata->phyAddrBlock,
						    GFP_KERNEL);
	if (privdata->pVaAddrBlock == NULL) {
		return -ENOMEM;
	}
	
	ret = adv_process_info_add(&privdata->ptr_process_info, 10);
	
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

static void  adShutDown(adv_device *device) 
{
	private_data *privdata = (private_data *) device->private_data;

	
	if (!(privdata->usFAIRunning || privdata->usFAORunning)){
		advOutpw(privdata, 0x06, 0);  //Base+6 the control register
		advOutp(privdata, 0x08, 0);  //Base+8 the Interrupt control register
		advOutp(privdata, 0x09, 0);  //Base+9 the FIFO control register
		advOutpDMAw(privdata, 0x68, 0x0000);  //Disable PCI the Interrupts  //yingsong 11/09/04
	}
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
 * adv_get_devinfo - get device information
 * 
 * @device: point to the device object
 * @arg: 
 */
INT32S adv_get_devinfo(adv_device *device, void *arg)
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
	case ADV_AI_DMA_START:
		ret = adv_fai_dma_start(device, (void *) arg);
		break;
	case ADV_FAO_DMA_START:
		ret = adFAODmaStart(device, (void *) arg);
		break;
	case ADV_FAO_STOP:
		ret = adFAOStop(device, (void *) arg);
		break;
	case ADV_FAO_TERMINATE:
		ret = adFAOTerminate(device, (void *) arg);
		break;
	case ADV_FAI_TERMINATE:
		ret = adFAITerminate(device, (void *) arg);
		break;
	case ADV_FAI_TRANSFER:
		ret = adFAITransfer(device, (void *) arg);
		break;
	case ADV_FAI_CHECK:
		ret = adFAICheck(device, (void *) arg);
		break;
	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;
	case ADV_CLEAR_UNDERRUN:
		ret = adClearUnderrun(device, (void *) arg);
		break;
	case ADV_CLEAR_FLAG:
		ret = adClearFlag(device, (void *) arg);
		break;
	case ADV_PORTREAD:
		ret = adv_read_port(device, (void *) arg);
		break;
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (void *) arg);
		break;
	case ADV_FAO_LOAD:
		ret = adFAOLoad(device, (void *) arg);
		break;
	case ADV_FAO_LOADEX:
		ret = adFAOLoadEx(device, (void *) arg);
		break;
	case ADV_FAO_CHECK:
		ret = adFAOCheck(device, (void *) arg);
		break;
	case ADV_FAO_DMA_CHECK:
		if (copy_to_user((void *) arg, &(privdata->usFAORunning), sizeof(privdata->usFAORunning))) {
			return -EFAULT;
		}
		break;
	case ADV_FAI_DMA_CHECK:
		if (copy_to_user((void *) arg, &(privdata->usFAIRunning), sizeof(privdata->usFAIRunning))) {
			ret = -EFAULT;
		}
		break;

	case ADV_ALLOCATE_DMA_BUFFER:
		ret = adAllocateDmaBuffer(device, (void *) arg);
		break;
	case ADV_FREE_DMA_BUFFER:
		ret = adFreeDmaBuffer(device, (void *) arg);
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (void *) arg);
		break;
	default:
		ret = -EFAULT;
	}
	adv_process_info_set_error(&privdata->ptr_process_info, ret);
	
	if (ret) {
		return ret;
	}

	return 0;
}


/* PCI-1712 fops */
adv_general_ops dev1712_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.poll		=adv_poll,
};

/* PCI-1712 driver object */
adv_driver pci1712_driver = {
	.owner = THIS_MODULE,
/* 	.mod_name = KBUILD_MODNAME, */
	.driver_name	= "pci1712",
	.devfs_name	= "pci1712",
	.ops		= &dev1712_fops,
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
		return ret;
	}

	pci_set_master(dev);

	device = (adv_device *) kzalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		return -ENOBUFS;
	}
/* 	memset(device, 0, sizeof(adv_device)); */
	

	/* alloc & initialize the private data structure */
	privdata = kzalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		return -ENOMEM;
	}

	/* Initialize the private data in the device */
/* 	memset(privdata, 0, sizeof(private_data)); */
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;

	privdata->device_type = dev->subsystem_device; /* multi-card support for new driver */     
	privdata->device_type <<= 16;
	privdata->device_type |= dev->subsystem_vendor;

	privdata->irq = dev->irq;
	privdata->ioDMAbase = dev->resource[1].start & ~1UL;
	privdata->ioDMAlength = dev->resource[1].end - dev->resource[1].start;
	privdata->iobase = dev->resource[2].start & ~1UL;
	privdata->iolength = dev->resource[2].end - dev->resource[2].start;
	

	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);


	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);
	pci_set_drvdata(dev, device);
	privdata->pdev = dev;
	tasklet_init(&privdata->tasklet, pci1712_tasklet_handler, (PTR_T)privdata);

	/* request I/O regions */
	if (request_region(privdata->ioDMAbase, privdata->ioDMAlength, "PCI-1712") == NULL) {
		kfree(device);
		kfree(privdata);
		return  ret;
	}

	if (request_region(privdata->iobase, privdata->iolength, "PCI-1712") == NULL) {
		release_region(privdata->ioDMAbase, privdata->ioDMAlength);
		kfree(device);
		kfree(privdata);
		return  ret;
	}

	/* request irq */
	ret = request_irq(privdata->irq, pci1712_interrupt_handler, SA_SHIRQ, "adv1712", privdata); 
	if (ret != 0) {
		release_region(privdata->ioDMAbase, privdata->ioDMAlength);
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);
		return ret;
	}

	/* support multi-card */
	switch (privdata->device_type) {
	case PCI1712S:
		privdata->board_id = 0;
		//privdata->board_id = (INT16U) (advInp(privdata, 0x20) & 0x0f);
		advdrv_device_set_devname(device, "pci1712");
		break;
	case PCI1712L:
		privdata->board_id = 0;
		//privdata->board_id = (INT16U) (advInp(privdata, 0x20) & 0x0f);
		advdrv_device_set_devname(device, "pci1712L");
		break;

	}

	/* add device into driver list */
	ret = advdrv_add_device(&pci1712_driver, device);
	if (ret) {
		release_region(privdata->ioDMAbase, privdata->ioDMAlength);
		release_region(privdata->iobase, privdata->iolength);
		free_irq(privdata->irq,privdata);
		kfree(device);
		kfree(privdata);
		return ret;
	}

	printk(KERN_DEBUG "Add a PCI-%x device: ioDMAbase=%xh; iobase=%lxh; slot=%xh\n", 
	       dev->device,
	       privdata->ioDMAbase,
	       privdata->iobase,
	       privdata->pci_slot);
     
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



	device = pci_get_drvdata(dev);
	privdata = (private_data *) device->private_data;

	adShutDown(device);
	dma_free_coherent(NULL, 128, privdata->pVaAddrBlock, privdata->phyAddrBlock);

	release_region(privdata->ioDMAbase, privdata->ioDMAlength);
	release_region(privdata->iobase, privdata->iolength);
	free_irq(privdata->irq,privdata);
	pci_set_drvdata(dev, NULL);
	advdrv_remove_device(&pci1712_driver, device);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}

/**
 * advdrv_suspend_one - Pnp to suspend a device
 *
 * @dev: Points to the pci_dev device;       
 */
static INT32S advdrv_suspend_one(struct pci_dev *dev, int state)
{
	return 0;
}

/**
 * advdrv_resume_one - Pnp to resume a device
 *
 * @dev: Points to the pci_dev device;       
 */
static INT32S advdrv_resume_one(struct pci_dev *dev)
{

	return 0;
}


struct pci_driver pci_driver = {
	name:	"pci1712",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
	suspend:	advdrv_suspend_one,
	resume:	advdrv_resume_one,
	id_table:	serial_pci_tbl,
};


/**
 * pci1712_init - The module initialize PCI-1712 driver
 *
 * Enumerate all PCI cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init pci1712_init(void)
{
	INT32S error;
	struct semaphore *sema = NULL;
	sema = kzalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return -ENOMEM;
	}

	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1712_driver, sema);

	advdrv_register_driver(&pci1712_driver);
	error = pci_module_init(&pci_driver);
	if (error != 0) {
		advdrv_unregister_driver(&pci1712_driver);
		return error;
	}
	return 0;
}

/**
 * pci1712_exit - Exit PCI-1712 driver
 */
static void __exit pci1712_exit(void)
{
	struct semaphore *sema = NULL;

	pci_unregister_driver( &pci_driver );
	if(advdrv_unregister_driver( &pci1712_driver ))
		return;
	
	_ADV_GET_DRIVER_SEMA(pci1712_driver, sema);
	kfree(sema);
}

module_init(pci1712_init);
module_exit(pci1712_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1712 device driver module");
