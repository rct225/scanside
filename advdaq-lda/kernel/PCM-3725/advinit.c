/***************************************************************************
 *                                                                         
 *                             ABJ                                         
 *		Copyright 2004 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech PCM-3725 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the PCM-3725 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	1/17/2006			Create by LI.ANG                  
 *                                                                         
 *                                                                         
 ***************************************************************************/
#include "pcm3725.h"


#define DRV_NAME "pcm3725"
#define PFX DRV_NAME ":"

#define ADVDEV_ALIGN       32
#define ADVDEV_ALIGN_CONST (ADVDEV_ALIGN - 1)

#define ISA_PCM3725_OFFSET  1

static INT32U padded = 0;



static INT32S adv_opendevice(adv_device *device)
{
	private_data *priv = (private_data *) device->private_data;
	INT32S ret;
	
	ret = adv_process_info_add(&priv->ptr_process_info, 1);
	
	return ret;
}

static INT32S adv_closedevice(adv_device *device)
{
	INT32S ret;
	private_data *priv = (private_data *) device->private_data;
	ret = adv_process_info_remove(&priv->ptr_process_info);
	
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

static INT32S adv_get_devinfo(adv_device *device, adv_devinfo *udev_info)
{
	private_data *priv = (private_data *) device->private_data;
	adv_devinfo kdev_info;
	INT32U ret = 0;
	

	memset((void *) &kdev_info, 0, sizeof(kdev_info));
		
	kdev_info.iobase = priv->iobase;
	kdev_info.iolength = priv->iolength;
	kdev_info.deviceID = priv->device_type;
	kdev_info.irq = priv->irq;
	
	if (copy_to_user(udev_info, &kdev_info, sizeof(adv_devinfo))) {
		ret = -EFAULT;
	}
	
	return ret;
}


static INT32S adv_ioctrl(adv_device *device, INT32U cmd, PTR_T arg)
{
	INT32S ret = 0;
	private_data *priv = (private_data *) device->private_data;

	if (device == NULL)
		return -ENODEV;
	
	switch (cmd) {
	case ADV_PORTREAD:
		ret = adv_read_port(device, (void *) arg);
		break;
	case ADV_PORTWRITE:
		ret = adv_write_port(device, (void *) arg);
		break;
	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (void *)arg);
		break;
	default:
		return -EINVAL;
	}
	adv_process_info_set_error(&priv->ptr_process_info, ret);
	
	return ret;
}

static inline void *advdev_priv(adv_device *dev)
{
	return (void *) ((char *)dev + ((sizeof(*dev) + ADVDEV_ALIGN_CONST) 
			       & ~ADVDEV_ALIGN_CONST));
}

adv_device *alloc_advdev(INT32S sizeof_priv)
{
	void *p;
	adv_device *dev = NULL;
	int alloc_size;
	
	/* ensure 32-byte alignment of both the device and private data */
	alloc_size = (sizeof(*dev) + ADVDEV_ALIGN_CONST) & ~ADVDEV_ALIGN_CONST;
	alloc_size += sizeof_priv + ADVDEV_ALIGN_CONST;
	
	p = kmalloc(alloc_size, GFP_KERNEL);
	if (!p) {
		printk(KERN_ERR PFX "Failed to allocate memory for device.\n");
		return NULL;
	}
	memset(p, 0, alloc_size);
	
	
	dev = (adv_device *)
		(((long)p + ADVDEV_ALIGN_CONST) & ~ADVDEV_ALIGN_CONST);
	padded = (char *)dev - (char *)p;
	
	if (sizeof_priv) {
		dev->private_data = advdev_priv(dev);
	}
	
	return (adv_device *) dev;
}

void free_advdev(adv_device *dev)
{
	kfree((char *)dev - padded);
}
	
adv_device *alloc_pcmdev(INT32S sizeof_priv)
{
	return alloc_advdev(sizeof_priv);
}

static adv_general_ops adv_dev3725_fops = {
	.opendevice = adv_opendevice,
	.closedevice = adv_closedevice,
	.ioctrl = adv_ioctrl,
	.poll = adv_poll
};


static adv_driver pcm3725_adv_driver = {
	.owner = THIS_MODULE,
	.driver_name = DRV_NAME,
	.devfs_name = DRV_NAME,
	.ops = &adv_dev3725_fops,
};
	
static INT32S  pcm3725_isa_probe(struct adv_isa_device *idev)
{
	adv_device *dev = NULL;
	private_data *priv = NULL;
	
	INT32S ret = 0;
	//INT32S i = 0;

	dev = alloc_pcmdev(sizeof(*priv));
	if (!dev) {
		printk(KERN_ERR PFX 
		       "Failed to allocate memory for adv_device and private data.\n");
		return -ENOMEM;
	}
	
	priv = (private_data *) advdev_priv(dev);
	
	/* allocate & initialize the private data structure */
	dev->iobase = idev->iobase;
	dev->irq = idev->irq;

	priv->iobase = dev->iobase;
	priv->iolength = PCM3725_TOTAL_SIZE;
	priv->device_type = PCM3725;
	priv->irq = dev->irq;

/* 	for (i = 0; i < MAX_CONNECTORS; ++i) { */
/* 		advOutp(priv, i * 4 + CFG_REG_OFFSET, 0x9b); */
/* 	} */

	
	adv_process_info_header_init(&priv->ptr_process_info);
	init_waitqueue_head(&priv->event_wait);
	spin_lock_init(&priv->spinlock);
	
	
	
	if (!request_region(priv->iobase, priv->iolength, idev->device_name)) {
		printk    (KERN_ERR PFX 
			   "I/O ports 0x%x - 0x%x already in use.\n",
			   priv->iobase, 
			   priv->iobase + priv->iolength);
		free_advdev(dev);
		return -EBUSY;
	}
	
	ret = request_irq(priv->irq, pcm3725_interrupt_handler,
			  IRQF_DISABLED, idev->device_name, priv);
	if (ret) {
		printk(KERN_ERR PFX "Could not allocate IRQ%d\n", priv->irq);
		release_region(priv->iobase, priv->iolength);
		free_advdev(dev);
		return -EBUSY;
	}
	
	/* _ADV_SET_DEVICE_PRIVDATA(dev, priv); */
	
	/* add device into driver list */
	advdrv_device_set_devname(dev, idev->device_name);
	_ADV_SET_DEVICE_IOBASE(dev, priv->iobase);
	_ADV_SET_DEVICE_IRQ(dev, priv->irq);
	adv_isa_set_drvdata(idev, dev);
	ret = advdrv_add_device(&pcm3725_adv_driver, dev);
	if (ret) {
		printk(KERN_ERR PFX 
		       "Failed to add device into adv_driver object.\n");
		release_region(priv->iobase, priv->iolength);
		free_irq(priv->irq, priv);
		free_advdev(dev);
		return ret;
	}
		
	return 0;
}

static INT32S pcm3725_device_remove(struct adv_isa_device *idev)
{
	/* struct eisa_device *edev; */
	adv_device *dev;
	private_data *priv;
	
	/* edev = to_eisa_device(device); */
	dev = adv_isa_get_drvdata(idev);
	priv = (private_data *)advdev_priv(dev);
	
	release_region(priv->iobase, PCM3725_TOTAL_SIZE);
	free_irq(priv->irq, priv);
	advdrv_remove_device(&pcm3725_adv_driver, dev);
	free_advdev(dev);	
	
	return 0;
}

static struct adv_isa_driver pcm3725_isa_driver = {
		.name = DRV_NAME,
		.probe = pcm3725_isa_probe,
		.remove = pcm3725_device_remove,
};



/**
 * pcm3725 - The module initialize PCM-3725 driver
 * 
 * Loadable module initialization stuff, register the driver in Advantech 
 * core driver.
 */
static INT32S __init pcm3725_init(void)
{
	struct semaphore *sema = NULL;
	INT32S ret = 0;
	
	sema = kmalloc(sizeof(*sema), GFP_KERNEL);
	if (!sema) {
		printk(KERN_ERR PFX 
		       "Failed to allocate memory for semaphore.\n");
		return -ENOMEM;
	}
	
	
	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(pcm3725_adv_driver, sema);
	
	ret = adv_isa_register_driver(&pcm3725_isa_driver);
        ret |= advdrv_register_driver(&pcm3725_adv_driver);

	return ret;
}

/**
 * pcm3725_exit - Exit PCM-3725 driver
 */
static void __exit pcm3725_exit(void)
{
	struct semaphore *sema = NULL;
	
	_ADV_GET_DRIVER_SEMA(pcm3725_adv_driver, sema);
	kfree(sema);
	advdrv_unregister_driver(&pcm3725_adv_driver);
/* 	pcm3725_device_remove(device); */
	
	adv_isa_unregister_driver(&pcm3725_isa_driver);
	
	
	/* release_region(iobase, PCM3725_TOTAL_SIZE); */
/* 	free_irq(irq, priv); */
/* 	kfree(privdata); */
}

module_init(pcm3725_init);
module_exit(pcm3725_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("li.ang <li.ang@advantech.com.cn>");
MODULE_DESCRIPTION("PCM-3725 ISA device driver");
