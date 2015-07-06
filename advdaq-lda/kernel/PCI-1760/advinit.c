/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech PCI-1760 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCI-1760 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	1/17/2006			Create by LI.ANG                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
/* #include <linux/config.h> */
#include <linux/module.h>
 

#include "PCI1760.h"


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct pci_device_id serial_pci_tbl[] = 
{
	{       0x13fe, 0x1760,	/* PCI-1760 */
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, 0
	},
	{0}, 
};

/************************************************************************/
/* Function:		 INT32S adv_opendevice                             */
/*                                                                      */
/* Description:  	The advdrv_core driver calls to open the device.*/
/* Parameters:	device 	-Points to the device object                    */
/************************************************************************/
static INT32S adv_opendevice(adv_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret;
	
	ret = adv_process_info_add(&privdata->ptr_process_info, 4);
	
	return ret;
}

/************************************************************************
 * Function:		 INT32S adv_closedevice
 * 
 * Description:  	The advdrv_core driver calls to close the device.
 * Parameters:	device 	-Points to the device object
 *************************************************************************/
static INT32S adv_closedevice(adv_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret;
	
	ret = adv_process_info_remove(&privdata->ptr_process_info);
	
	return ret;
}

/************************************************************************
 * Function:		 INT32S adv_resetdevice
 * 
 * Description:  	The advdrv_core driver calls to reset the device.
 * Parameters:	device 	-Points to the device object
 *************************************************************************/
static INT32S adv_resetdevice(adv_device *device)
{
	return 0;
}

/************************************************************************
 * Function:		 INT32S adv_mmap
 * 
 * Description:  	The advdrv_core driver calls to map the kernel memory to user
 * Parameters:	device 	-Points to the device object
 				vma 	-vm_area_struct 
*************************************************************************/
static INT32S adv_mmap(adv_device *device, struct file *file, struct vm_area_struct *vma)
{
	return 0;
}

/**
 * adv_poll - relative poll or select system call
 */
static INT32U adv_poll(adv_device *device, struct file *file, struct poll_table_struct *wait)
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

/************************************************************************
 * Function:		 INT32S adv_ioctrl
 * 
 * Description:  	The advdrv_core driver calls to handle the ioctl commands.
 * Parameters:	device 	-Points to the device object
 				cmd	 	-io code.
 				arg		-argument
*************************************************************************/
static INT32S adv_ioctrl(adv_device *device, INT32U cmd, PTR_T arg)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret;

	
	switch (cmd) {
	case EVENT_READ:
		ret = adv_cnt_event_read(device, (void *) arg);
		break;
	case FDI_TRANSFER:
		ret = adv_fdi_transfer(device, (void *) arg);
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
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (void *) arg);
		break;
	default:
		return -EINVAL;
	}
	adv_process_info_set_error(&privdata->ptr_process_info, ret);
	
	if (ret) {
		return -1;
	}

	return 0;
}

/* PCI-1760 fops */
adv_general_ops dev1760_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.resetdevice	= adv_resetdevice,
	.mmap		= adv_mmap,
	.poll		= adv_poll,
};

/* PCI-1760 driver object */
adv_driver pci1760_driver = {
	.driver_name	= "pci1760",
	.devfs_name	= "pci1760",
	.ops		= &dev1760_fops,
};
 


/************************************************************************
 * Function:	INT32S advdrv_init_one
 * 
 * Description: Pnp to initialize the device, and allocate resource for the device.
 * Parameters:	dev	-Points to the pci_dev device
 		ent	-Points to pci_device_id including the device info.
*************************************************************************/
static INT32S __devinit advdrv_init_one(struct pci_dev *dev, const struct pci_device_id *ent)
{     
	private_data *privdata = NULL;
	adv_device *device = NULL;
	INT32S ret;
	INT16U i;
	
     
	if ((ret = pci_enable_device(dev)) != 0) {
		KdPrint(KERN_ERR": pci_enable_device failed\n");
		return ret;
	}

	/* initialize & zero the device structure */
	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		PCRIT("Could not kmalloc space for device!");
		return ENOBUFS;
	}
	memset(device, 0, sizeof(adv_device));
	

	/* alloc & initialize the private data structure */
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		return MemoryAllocateFailed;
	}

	memset(privdata, 0, sizeof(private_data));
	privdata->pci_slot = PCI_SLOT(dev->devfn);
	privdata->pci_bus = dev->bus->number;
	privdata->device_type = dev->device; /* multi-card support for new driver */     
	privdata->irq = dev->irq;
	privdata->iobase = dev->resource[0].start & ~1UL;
	privdata->iolength = dev->resource[0].end - dev->resource[0].start;

	advOutp(privdata, 0x0e, 0x0d); /* command: read board id */
	for (i = 0; i < 10000; i++) {
		if (advInp(privdata, 0x1e) == 0x0d) {
			privdata->board_id = (INT16U) (advInp(privdata, 0x0e) & 0x0f);
			break;
		}
	}		

	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	spin_lock_init(&privdata->spinlock);

	/* request I/O regions */
	if (request_region(privdata->iobase, privdata->iolength, "PCI-1760") == NULL) {
		kfree(device);
		kfree(privdata);
		printk("Request region failed\n");
		return  ret;
	}

	/* request irq */
	ret = request_irq(privdata->irq, pci1760_interrupt_handler, SA_SHIRQ, "adv1760", privdata); 
	if (ret != 0) {
		kfree(device);
		kfree(privdata);
		printk("Request IRQ failed!\n");
		return ret;
	}

	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	_ADV_SET_DEVICE_SLOT(device, privdata->pci_slot);

	advdrv_device_set_devname(device, "pci1760");
	pci_set_drvdata(dev, device);


	/* add device into driver list */
	ret = advdrv_add_device(&pci1760_driver, device);
	if (ret != 0) {
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return ret;
	}

	printk("Add a PCI-%x device: iobase=%xh; irq=%xh; slot=%xh\n", 
	       dev->device,
	       privdata->iobase,
	       privdata->irq,
	       privdata->pci_slot);

	return 0;
}

/**************************************************************************************
 * Function:	 INT32S advdrv_remove_one
 * 
 * Description:  Pnp to remove a device, and free resource for the device.
 * Parameters:	 dev	-Points to the pci_dev device; 
 			pci_get_drvdata(dev) points to the private data in adv_device.
**************************************************************************************/
static void __devexit advdrv_remove_one(struct pci_dev *dev)
{
	private_data *privdata; 
	adv_device *device = NULL;
	privdata = NULL;

	KdPrint("Remove a PCI-%x device\n", dev->device);

	device = pci_get_drvdata(dev);
	privdata = (private_data *) device->private_data;

	free_irq(privdata->irq, privdata);
	release_region(privdata->iobase, privdata->iolength);  

	pci_set_drvdata(dev, NULL);
	advdrv_remove_device(&pci1760_driver, device);

	pci_disable_device(dev);

	kfree(privdata);
	kfree(device);
}

/************************************************************************
 *  Function:		INT32S advdrv_suspend_one                    
 *                                                                
 *  Description:  	Pnp to suspend a device                   
 *  Parameters:	        dev  -Points to the pci_dev device;       
 *************************************************************************/
/* static INT32S advdrv_suspend_one(struct pci_dev *dev, u32 state) */
/* { */
/* 	return 0; */
/* } */

/*****************************************************************************************
 *    Function:      INT32S advdrv_suspend_one
 * 
 * Description:      Pnp to resume a device
 *  Parameters:	     dev  -Points to the pci_dev device; 
 *                	   pci_get_drvdata(dev) points to the private data in adv_device.
 ****************************************************************************************/
/* static INT32S advdrv_resume_one(struct pci_dev *dev) */
/* { */
/* 	return 0; */
/* } */

struct pci_driver pci_driver = {
	name:	"pci1760",
	probe:	advdrv_init_one,
	remove:	__devexit_p(advdrv_remove_one),
/* 	suspend:	advdrv_suspend_one, */
/* 	resume:	advdrv_resume_one, */
	id_table:	serial_pci_tbl,
};

/**************************************************************************************
 * static INT32S __init pci1760_init(void)
 * 
 * Description:  The module initialize PCI-1760 driver, Enumerate all PCI cards on the
 * 		 bus, register the driver in Advantech core driver.
 **************************************************************************************/
static INT32S __init pci1760_init(void)
{
	INT32S error;
	struct semaphore *sema = NULL;


	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return MemoryAllocateFailed;
	}

	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pci1760_driver, sema);

	advdrv_register_driver(&pci1760_driver);

	error = pci_module_init(&pci_driver);
	if (error != 0) {
		advdrv_unregister_driver(&pci1760_driver);
		return error;
	}

	return 0;
}

/************************************************************************
 * static INT32S __exit pci1760_init(void)
 * 
 * Description:  Exit  PCI-1760 driver .
 *************************************************************************/
static void __exit pci1760_exit(void)
{
	struct semaphore *sema = NULL;


	pci_unregister_driver( &pci_driver );
	advdrv_unregister_driver( &pci1760_driver );
	_ADV_GET_DRIVER_SEMA(pci1760_driver, sema);
     
	kfree(sema);
/*      pci_unregister_driver(&pci_driver); */
/*      advdrv_unregister_driver(&pci1760_driver); */
}

module_init(pci1760_init);
module_exit(pci1760_exit);

MODULE_DEVICE_TABLE(pci, serial_pci_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCI1760 device driver module");
MODULE_AUTHOR("Li Ang <li.ang@advantech.com.cn>");
