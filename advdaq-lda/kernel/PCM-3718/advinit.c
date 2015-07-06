

/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCM-3718 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCM-3718 driver module and device. 
Version history
	07/11/2006		

************************************************************************/

 
#include <linux/module.h>
#include "PCM3718.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif
#define DRV_NAME "pcm3718"
 
/************************************************************************
 * Function:		int adv_opendevice
 * 
 * Description:  	The advdrv_core driver calls to open the device.
 * Parameters:	
	     device 	-Points to the device object
*************************************************************************/
int adv_opendevice( adv_device *device)
{
	
	private_data *privdata = NULL;
     
	_ADV_GET_DEVICE_PRIVDATA(device, privdata);
	
	return adv_process_info_add(&privdata->ptr_process_info, 5);
}

/************************************************************************
 * Function:		int adv_closedevice
 * 
 * Description:  	The advdrv_core driver calls to close the device.
 * Parameters:	
	    device 	-Points to the device object
*************************************************************************/
int  adv_closedevice( adv_device *device)
{
	private_data *privdata = NULL;
     
	_ADV_GET_DEVICE_PRIVDATA(device, privdata);

	return adv_process_info_remove(&privdata->ptr_process_info);
	
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
	kdev_info->irq = privdata->irq;
	
	if (copy_to_user(udev_info, kdev_info, sizeof(adv_devinfo)))
		ret = -EFAULT;
	
	kfree(kdev_info);
	return ret;
}


/************************************************************************
* Function:		int adv_ioctrl
* 
* Description:  	The advdrv_core driver calls to handle the ioctl commands.
* Parameters:	
             device 	-Points to the device object
 	     cmd 	-io code.
 	     arg	-argument
*************************************************************************/
int adv_ioctrl(adv_device *device, unsigned int cmd, unsigned long arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32S ret = 0;


	if (device == NULL)
		return InvalidDeviceHandle;
     
	switch (cmd) {
	case ADV_DEVGETPROPERTY:
		ret = adv_dev_get_property(device, (void *) arg);
		break;
	case ADV_DEVSETPROPERTY:
		ret = adv_dev_set_property(device, (void *) arg);
		break;
	case ADV_GET_ERR_CODE:
		ret = adv_get_err_code(device, (void *) arg);
		break;		
	case ADV_PORTREAD:
		ret = adv_read_port(device, (void *) arg);
		break;
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (void *) arg);
		break;
	case ADV_AI_DMA_START:
		ret = adv_fai_dma_start(device, (void *) arg);
		break;
	case ADV_AI_INT_START:
		ret = adv_fai_int_start(device, (void *) arg);
		break;
	case ADV_AI_INT_SCAN_START:
		ret = adv_fai_int_scan_start(device, (void *) arg);
		break;
	case ADV_AI_TRANSFER:
		ret = adv_fai_transfer(device, (void *)arg);
		break;
	case ADV_AI_STOP:
		ret = adv_fai_stop(device);
		break;
	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;
	case CLEAR_OVERRUN:
		ret = adv_clear_overrun(device);
		break;
	case FAICHECK:
		ret = adv_ai_check(device, (void *) arg);
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



static adv_general_ops adv_dev3718_fops = {
	.opendevice = adv_opendevice,
	.closedevice = adv_closedevice,
	.ioctrl = adv_ioctrl,
	.poll = adv_poll
};


static adv_driver pcm3718_adv_driver = {
	.owner = THIS_MODULE,
	.driver_name = DRV_NAME,
	.devfs_name = DRV_NAME,
	.ops = &adv_dev3718_fops,
};
	
static INT32S  pcm3718_isa_probe(struct adv_isa_device *dev)
{
	private_data *privdata = NULL;
	adv_device *device = NULL;
	int err;
	device = (adv_device *)kmalloc(sizeof(adv_device), GFP_KERNEL);
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if((!device) || (!privdata)) {
		kfree(device);
		kfree(privdata);
		PCRIT("Could not kmalloc space for device!");
		return -ENOMEM;
	}

	memset(device, 0, sizeof(adv_device));
	memset(privdata, 0, sizeof(private_data) );

	/* initialize the private_data structure */
     	privdata->iobase = dev->iobase;
	privdata->irq=dev->irq;	
	privdata->iolength = PCM3718_TOTAL_SIZE;
	privdata->sigpid=0;
	privdata->device_type = PCM3718;
	privdata->timer_clock = 1; /* pacer clock freq in M Hz, default is 10 M Hz */
	privdata->half_fifo_size = 512;	/* total fifo is 1K word */
	privdata->ai_stop = 1;	/* 0 : complete 1 : incomplete */
	privdata->ioDMAbase = 0x03;
	spin_lock_init(&privdata->spinlock);
	tasklet_init(&privdata->tasklet, pcm3718_tasklet_handler, (PTR_T)privdata);	
/*#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	privdata->fai_stop.routine = (void (*)(void *)) adv_ai_int_stop;
	privdata->fai_stop.data = (void *) privdata;
	INIT_LIST_HEAD(&privdata->tq_fai_stop);
	queue_task(&privdata->fai_stop, &privdata->tq_fai_stop);
#else
	INIT_WORK(&privdata->fai_stop, adv_ai_int_stop, privdata);
#endif	
*/	
	/* for new event mechanism */
	init_waitqueue_head(&privdata->event_wait);
	adv_process_info_header_init(&privdata->ptr_process_info);

	/* inintialize the adv_device structure */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);

	 adv_isa_set_drvdata(dev,device);
	/*if (privdata->device_type == PCM3718HDU)
		advdrv_device_set_devname(device, "pcm3718");
	*/
		advdrv_device_set_devname(device, "pcm3718");
	if (request_region(privdata->iobase, privdata->iolength, "PCM-3718") == NULL) {
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}
	
	if (request_irq(privdata->irq, pcm3718_interrupt_handler, SA_SHIRQ, "adv3718", privdata)) {
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}
	
	if (request_dma(privdata->ioDMAbase,"PCM-3718")) {
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}
	pci_set_drvdata(dev, device);

	advdrv_add_device( &pcm3718_adv_driver, device );
	return SUCCESS;	
}

static INT32S pcm3718_device_remove(struct adv_isa_device *idev)
{
	 private_data *privdata;
	 adv_device *device = NULL;
	 privdata = NULL;
	 device = adv_isa_get_drvdata(idev);
	 privdata =(private_data*) device->private_data;
	 free_irq(privdata->irq, privdata);
	 release_region(privdata->iobase, privdata->iolength);

	 //tasklet_disable(&privdata->tasklet);
	disable_dma(privdata->ioDMAbase);
	free_dma(privdata->ioDMAbase);
	 advdrv_remove_device(&pcm3718_adv_driver, device);

	 kfree(privdata);
	 kfree(device);

	 return 0;
}

static struct adv_isa_driver pcm3718_isa_driver = {
		.name = DRV_NAME,
		.probe = pcm3718_isa_probe,
		.remove = pcm3718_device_remove,
};



/**
 * pcm3718 - The module initialize PCM-3718 driver
 * 
 * Loadable module initialization stuff, register the driver in Advantech 
 * core driver.
 */
static INT32S __init pcm3718_init(void)
{
	struct semaphore *sema = NULL;
	INT32S ret = 0;
struct adv_isa_device *dev;	
	sema = kmalloc(sizeof(*sema), GFP_KERNEL);
	if (!sema) {
		return -ENOMEM;
	}
	
	
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pcm3718_adv_driver, sema);
	
	ret = adv_isa_register_driver(&pcm3718_isa_driver);
        ret |= advdrv_register_driver(&pcm3718_adv_driver);
 
// pcm3718_isa_probe(dev);
	return ret;
}

static void __exit pcm3718_exit(void)
{
	struct semaphore *sema = NULL;
	
	_ADV_GET_DRIVER_SEMA(pcm3718_adv_driver, sema);
	
	kfree(sema);

	advdrv_unregister_driver(&pcm3718_adv_driver);
	
	adv_isa_unregister_driver(&pcm3718_isa_driver);
	
}

module_init(pcm3718_init);
module_exit(pcm3718_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PCM-3718 device driver");



