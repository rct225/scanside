
   

/* base module includes */
/* #include <linux/config.h> */
#include <linux/module.h>
 

#include "PCM3761I.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] = 
{
	{       0x13fe, 0x3761,	
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
	
	ret = adv_process_info_add(&privdata->ptr_process_info, 10);
	
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

static INT32U adv_poll(adv_device *device, struct file *fp,
		       struct poll_table_struct *wait)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U mask = 0;
	

	poll_wait(fp, &privdata->event_wait, wait);
	
	mask = POLLIN | POLLRDNORM;
			
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
	

	devinfo.irq = privdata->irq;
	devinfo.iobase = privdata->iobase;
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
	/* private_data *privdata = (private_data *) device->private_data; */
	INT32S ret;

	
	switch (cmd) {
	case ADV_DIO_BITS:
		ret = adv_dio_ioctl(device, (void *) arg);
		break;
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
	case ADV_DEVGETPROPERTY:
		ret = adv_dev_get_property(device, (void *) arg);
		break;
	case ADV_DEVSETPROPERTY:
		ret = adv_dev_set_property(device, (void *) arg);
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (void *) arg);
		break;
	default:
		return -EINVAL;
	}
	/* adv_process_info_set_error(&privdata->ptr_process_info, ret); */
	
/* 	if (ret) { */
/* 		return -1; */
/* 	} */

	return ret;
}

adv_general_ops dev3761i_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.resetdevice	= adv_resetdevice,
	.mmap		= adv_mmap,
	.poll		= adv_poll,
};

adv_driver pcm3761i_driver = {
	.owner = THIS_MODULE,
	.driver_name	= "pcm3761i",
	.devfs_name	= "pcm3761i",
	.ops		= &dev3761i_fops,
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
	struct semaphore *dio_sema = NULL;
	adv_device *device = NULL;
	INT32S ret;
     

	

	if ((ret = pci_enable_device(dev)) != 0) {
		KdPrint("pci_enable_device failed\n");
		return ret;
	}

	/* allocate urb sema */
	dio_sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (dio_sema == NULL) {
		return -ENOMEM;
	}
	init_MUTEX(dio_sema);

	/* initialize & zero the device structure */
	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		KdPrint("Could not kmalloc space for device!");
		kfree(dio_sema);
		return -ENOMEM;
	}
	memset(device, 0, sizeof(adv_device));
	

	/* alloc & initialize the private data structure */
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		kfree(dio_sema);
		return -ENOMEM;
	}

	memset(privdata, 0, sizeof(private_data));
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->device_type = dev->device; /* multi-card support for new driver */     
	privdata->irq = dev->irq;
	privdata->dio_sema = dio_sema;


	switch (privdata->device_type) {
	case PCM3761I:
		privdata->iobase = dev->resource[0].start & ~1UL;
		privdata->iolength = dev->resource[0].end - dev->resource[0].start;
		break;
		default:
			return -1;
	}
	
	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);

	/* request I/O regions */
	if (request_region(privdata->iobase, privdata->iolength, "PCM-3761I") == NULL) {
		kfree(device);
		kfree(privdata);
		kfree(dio_sema);
		KdPrint("Request region failed\n");
		return -ENXIO;
	}

	/* request irq */
	switch (privdata->device_type) {
	case PCM3761I:
		ret = request_irq(privdata->irq, pcm3761i_interrupt_handler,
				  SA_SHIRQ, "adv3761", privdata); 
		if (ret != 0) {
			release_region(privdata->iobase, privdata->iolength);
			kfree(device);
			kfree(privdata);
			kfree(dio_sema);
			KdPrint("Request IRQ failed\n");
			return ret;
		}
		break;
		default:
			return -1;
	}
	
	/* support multi-card */
	switch (privdata->device_type) {
	case PCM3761I:
		privdata->board_id = (INT16U) (advInp(privdata, 0x02) & 0x0f);
		advdrv_device_set_devname(device, "pcm3761i");
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
	ret = advdrv_add_device(&pcm3761i_driver, device);
	if (ret != 0) {
		release_region(privdata->iobase, privdata->iolength);
		free_irq(privdata->irq, privdata);
		kfree(device);
		kfree(privdata);
		kfree(dio_sema);
		KdPrint("Add device failed!\n");
		return ret;
	}

	printk("Add a PCM-%x device: iobase=%xh; irq=%xh; slot=%xh\n", 
	       dev->device,
	       privdata->iobase,
	       privdata->irq,
	       privdata->pci_slot);

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

	printk("Remove a PCM-%x device\n", dev->device);

	device = pci_get_drvdata(dev);
	privdata = (private_data *) device->private_data;

	free_irq(privdata->irq, privdata);
	release_region(privdata->iobase, privdata->iolength);  
	kfree(privdata->dio_sema);

	pci_set_drvdata(dev, NULL);
	advdrv_remove_device(&pcm3761i_driver, device);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}


struct pci_driver pci_driver = {
	name:	"pcm3761i",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),

	id_table:	serial_pci_tbl,
};


static INT32S __init pcm3761i_init(void)
{
	INT32S error;
	struct semaphore *sema = NULL;


	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return -ENOMEM;
	}
	
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pcm3761i_driver, sema);

	advdrv_register_driver(&pcm3761i_driver);

 	error = pci_module_init(&pci_driver); 
//	error = pci_register_driver(&pci_driver);
	if (error != 0) {
		advdrv_unregister_driver(&pcm3761i_driver);
		return error;
	}


	return 0;
}


static void __exit pcm3761i_exit(void)
{
	struct semaphore *sema = NULL;


	pci_unregister_driver(&pci_driver);
	advdrv_unregister_driver(&pcm3761i_driver);
	_ADV_GET_DRIVER_SEMA(pcm3761i_driver, sema);
     
	kfree(sema);
}

module_init(pcm3761i_init);
module_exit(pcm3761i_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_AUTHOR("Li Ang <li.ang@advantech.com.cn>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCM3761I device driver module");
