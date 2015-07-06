

/* base module includes */
#include <linux/module.h>

#include "PCM3730.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif



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

	return adv_process_info_add(&privdata->ptr_process_info, 4);
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

/************************************************************************
 * Function:	int adv_resetdevice
 * 
 * Description:	The advdrv_core driver calls to reset the device.
 * Parameters:	
	       device 	-Points to the device object
*************************************************************************/
int adv_resetdevice( adv_device *device )
{
     return SUCCESS;

}

/************************************************************************
 * Function:		int adv_mmap
 * 
 * Description:  	The advdrv_core driver calls to map the
 *                      kernel memory to user
 * Parameters:	
              device 	-Points to the device object
 	      vma 	-vm_area_struct 
*************************************************************************/
int adv_mmap(adv_device *device, struct file *file, struct vm_area_struct *vma )
{
     printk("This driver does not surpport mmap\n");
     return SUCCESS;
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


/************************************************************************
 * Function:		 int adv_read
 * 
 * Description:  	The advdrv_core driver calls to read datas from the device.
 * Parameters:	
              device 	-Points to the device object
 	      buf	-points to user buffer.
 	      nbytes 	-the number you want to read
*************************************************************************/
int adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  )
{
     return SUCCESS;
     
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
	kdev_info->switchID = privdata->boardID;
	kdev_info->deviceID = privdata->device_type;

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
	private_data *privdata = (private_data*)(device->private_data);
	PT_DeviceSetParam    lpDeviceSetParam;
	PT_DeviceGetParam    lpDeviceGetParam;
	PT_PortOperation     lpPortOperation;
	PT_EnableEvent       lpEnableEvent;
	PT_CheckEvent        lpCheckEvent;
	int ret = 0;
     
	switch( cmd )
	{
	case ADV_DEVGETPROPERTY:
		if(copy_from_user(&lpDeviceGetParam,(void *)arg,sizeof(PT_DeviceGetParam))){
			ret =  -EFAULT;
			break;
		}

		ret = adv_device_get_property(device, lpDeviceGetParam.nID, 
					      lpDeviceGetParam.pData, lpDeviceGetParam.Length);
		break;

	case ADV_DEVSETPROPERTY:
		if(copy_from_user(&lpDeviceSetParam,(void *)arg,sizeof(PT_DeviceSetParam))){
			ret = -EFAULT;
			break;
		}
		ret = adv_device_set_property( device,
					       lpDeviceSetParam.nID,
					       lpDeviceSetParam.pData,
					       lpDeviceSetParam.Length);
		break;
		
	case ADV_PORTREAD:
		ret = copy_from_user(&lpPortOperation,(void *)arg,sizeof(PT_PortOperation));
		if(ret){
			ret = -EFAULT;
			break;
		}
		
		ret = adv_port_read(device, lpPortOperation.PortNumber,
				    lpPortOperation.DataBuffer,  
				    lpPortOperation.DataLength);

		break;

	case ADV_PORTWRITE:
		ret = copy_from_user(&lpPortOperation,(void *)arg,sizeof(PT_PortOperation));
		if(ret){
			ret = -EFAULT;
			break;
		}
		
		ret = adv_port_write(device, lpPortOperation.PortNumber,
				     lpPortOperation.DataBuffer,
				     lpPortOperation.DataLength);

		break;


	case ADV_ENABLE_EVENT:
		if (copy_from_user(&lpEnableEvent,(void *)arg,sizeof(PT_EnableEvent))){
			ret = -EFAULT;
			break;
		}
		ret = adv_enable_event(device,lpEnableEvent.EventType,
				       lpEnableEvent.Enabled,
				       lpEnableEvent.Count);
		break;
		
	case CHECKEVENT:
		if(copy_from_user(&lpCheckEvent, (PT_CheckEvent *)arg,sizeof(PT_CheckEvent))){
			ret = -EFAULT;
			break;
		}
		adv_check_event(device, (INT32U *)&lpCheckEvent.EventType, 
				lpCheckEvent.Milliseconds);
	  
		if(copy_to_user((PT_CheckEvent *)arg,&lpCheckEvent,sizeof(PT_CheckEvent))){
			ret = -EFAULT;
		}
	  
		break;
				
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (adv_devinfo *) arg);
		break;
		
	default:
		ret = -EFAULT;
		break;
	}

	return ret;
}

/* PCM-3730 fops */
adv_general_ops dev3730_fops = {
     .opendevice	=adv_opendevice,
     .closedevice	=adv_closedevice,
     .read		=adv_read,
     .ioctrl		=adv_ioctrl,
     .resetdevice	=adv_resetdevice,
     .mmap		=adv_mmap,
     .poll		=adv_poll,
};

adv_driver pcm3730_driver = {
	.owner = THIS_MODULE,
	.driver_name		= "pcm3730",
	.devfs_name		= "pcm3730",
	.ops			= &dev3730_fops,
};

/************************************************************************
 * Function:		 int advdrv_init_one
 * 
 * Description:  	Pnp to initialize the device, and allocate resource for the device.
 * Parameters:	
	     dev	-Points to the pci_dev device
 	     ent 	-Points to pci_device_id including the device info.
*************************************************************************/
static int __devinit
advdrv_init_one(struct adv_isa_device *dev)
{     
	private_data *privdata = NULL;
	adv_device *device = NULL;
	int err;

	device = (adv_device *)kmalloc(sizeof(adv_device), GFP_KERNEL);
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if ((!device) || (!privdata)) {
		kfree(device);
		kfree(privdata);
		PCRIT("Could not kmalloc space for device!");
		return -ENOMEM;
	}
	memset(device, 0, sizeof(adv_device));
	memset(privdata, 0, sizeof(private_data) );
	/* for new event mechanism */
	init_waitqueue_head(&privdata->event_wait);
     
     
	/* get the device type */
     

		privdata->iobase = dev->iobase;// & ~1UL;
		privdata->iolength= 4;
		privdata->boardID= 0;
     
	privdata->irq=dev->irq;
	privdata->sigpid=0;
	privdata->device_type = PCM3730;
	if (request_region(privdata->iobase, privdata->iolength, "PCM-3730" ) == NULL) {
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}

	spin_lock_init(&privdata->spinlock);
	adv_process_info_header_init(&privdata->ptr_process_info);

	advOutp(privdata, 0x10, 0x0f);
	if (request_irq(privdata->irq, pcm3730_interrupt_handler, SA_SHIRQ, "adv3730", privdata)) {
		release_region(privdata->iobase, privdata->iolength);
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}
	

	/* get the device boardID */
	//adv_get_device_boardID(privdata);
	adv_init_device_do_range(privdata);
	adv_init_device_di_range(privdata);
     
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device,privdata->boardID);
	_ADV_SET_DEVICE_IOBASE(device, privdata->iobase);
	_ADV_SET_DEVICE_IRQ(device, privdata->irq);
	    
     
	switch(privdata->device_type)
	{
	case PCM3730:
		advdrv_device_set_devname(device, "pcm3730");
		break;
	default:
		return -1;
	  
	}
	advdrv_add_device( &pcm3730_driver, device );
	printk("Add a Advantech :boardID=%d;iobase=0x%lx;irq=%x;\n",privdata->boardID,privdata->iobase, privdata->irq);
	return SUCCESS;
	
}	
    
/************************************************************************
 * Function:		int advdrv_remove_one
 * 
 * Description:  	Pnp to remove a device, and free resource for the device.
 * Parameters:	
              dev 	-Points to the pci_dev device; 
 			 pci_get_drvdata(dev) points to the private data in adv_device.
*************************************************************************/
static void __devexit advdrv_remove_one(struct adv_isa_device *dev)
{
     private_data *privdata;
     adv_device *device = NULL;
//     ULONG	curpage;
     privdata = NULL;
     device = adv_isa_get_drvdata(dev);
     privdata =(private_data*) device->private_data;
     free_irq(privdata->irq, privdata);
     release_region(privdata->iobase, privdata->iolength);

     advdrv_remove_device(&pcm3730_driver, device);

     //  adv_kfifo_free(privdata->event_queue); /* for new event mechanism */
     kfree(privdata);
     kfree(device);
}
/************************************************************************
 * Function:	int advdrv_suspend_one
 * 
 * Description: Pnp to suspend a device
 * Parameters:	
          	dev 	-Points to the pci_dev device; 
 	        pci_get_drvdata(dev) points to the private data in adv_device.
*************************************************************************/
static int advdrv_suspend_one(struct pci_dev *dev, u32 state)
{
     return SUCCESS;
}
/************************************************************************
 * Function:		 int advdrv_resume_one
 * 
 * Description:  	Pnp to resume a device
 * Parameters:	
             dev 	-Points to the pci_dev device; 
 			pci_get_drvdata(dev) points to the private data in adv_device.
*************************************************************************/
static int advdrv_resume_one(struct pci_dev *dev)
{
     return SUCCESS;
}

struct adv_isa_driver isa_driver = 
{
     name:		 "pcm3730",
     probe:		 advdrv_init_one,
     remove:		 __devexit_p(advdrv_remove_one),
};




static int __init pcm3730_init(void)
{
     int error =0 ;
     struct semaphore *sema = NULL;
     sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
     init_MUTEX(sema);
     _ADV_SET_DRIVER_SEMA(pcm3730_driver, sema);
     adv_isa_register_driver( &isa_driver );
     advdrv_register_driver( &pcm3730_driver );

     return SUCCESS;
	
}


static void __exit pcm3730_exit(void)
{
     struct semaphore *sema = NULL;

     advdrv_unregister_driver( &pcm3730_driver );
     adv_isa_unregister_driver( &isa_driver );
     _ADV_GET_DRIVER_SEMA(pcm3730_driver, sema);
     
     kfree(sema);
}

module_init( pcm3730_init );
module_exit( pcm3730_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech PCM3730 device driver module");
