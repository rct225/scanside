/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2007 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech USB-4716 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the USB-4716 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	03/15/2007			Create by li.ang
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
/* #include <linux/config.h> */
#include <linux/module.h>
#include <linux/usb.h> 
#include "USB4716.h"
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#include <linux/sched.h>
#else
#include <linux/kthread.h>
#endif


#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct usb_device_id serial_usb_tbl[] = 
{
	{ USB_DEVICE(0x1809, 0x4716) },
	{},
};

static void adv_device_delete(adv_device *device);


/**
 * adv_opendevice - The advdrv_core driver calls to open the device.
 *
 * @device: Points to the device object
 */
static INT32S adv_opendevice(adv_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret = 0;
	INT32U tmp = 0;


	ret = adv_process_info_add(&privdata->ptr_process_info, 6);
	if (ret < 0) {
		return ret;
	}

	adv_process_info_enable_event(&privdata->ptr_process_info,
				      5, /* enable 'remove' event */
				      1);

	if (down_interruptible(privdata->usb_urb_sema)) {
		return -ERESTARTSYS;
	}

	if (privdata->user_cnt == 0) {

		if (privdata->udev == NULL) {
			up(privdata->usb_urb_sema);
			return -ENODEV;
		}
		
		ret = adv_usb_ctrl_msg(privdata->udev,
				       usb_sndctrlpipe(privdata->udev, 0),
				       MAJOR_SYSTEM, 0x40,
				       MINOR_DEVICE_OPEN, 0,
				       (INT8U *) &tmp,
				       sizeof(INT32U));
		if (ret > 0) {
			ret = 0;
		}
	}

     	privdata->user_cnt += 1;
	
	up(privdata->usb_urb_sema);

	
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
	INT32S ret = 0;
	INT32U tmp;


	ret = adv_process_info_remove(&privdata->ptr_process_info);
	if (ret < 0) {
		return ret;
	}

	if (down_interruptible(privdata->usb_urb_sema)) {
		return -ERESTARTSYS;
	}

	if (privdata->user_cnt <= 1) {
		privdata->user_cnt = 0;
	} else {
		privdata->user_cnt -= 1;
	}	
     
	if (privdata->user_cnt == 0) {
	  	/* if usb device has unplugged */
		if (privdata->udev == NULL) {
			up(privdata->usb_urb_sema);
			adv_device_delete(device);

			return 0;
		}

		ret = adv_usb_ctrl_msg(privdata->udev,
				       usb_sndctrlpipe(privdata->udev, 0),
				       MAJOR_SYSTEM,
				       0x40,
				       MINOR_DEVICE_CLOSE,
				       0,
				       (INT8U *) &tmp,
				       sizeof(INT32U));
		if (ret > 0) {
			ret = 0;
		}
	}
	up(privdata->usb_urb_sema);

	return ret;
}

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
static INT32S adv_get_devinfo(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_devinfo devinfo;
	

	devinfo.switchID = privdata->board_id;
	devinfo.deviceID = privdata->device_type;
	devinfo.usb_speed = privdata->usb_speed;
	
	if (copy_to_user(arg, &devinfo, sizeof(adv_devinfo))) {
		return -EFAULT;
	}

	return 0;
}

static INT32S adv_set_boardid(adv_device *device, INT32U id)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret;


	if (id > 0xf) {
		return -EINVAL;
	}
	
	if (privdata->udev == NULL) {
		return -EFAULT;
	}
     
	if (down_interruptible(privdata->usb_urb_sema)) {
		return -ERESTARTSYS;
	}

	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			       MAJOR_SYSTEM, 0x40,
			       MINOR_WRITE_SWITCHID, 0,
			       (INT8U *) &id, sizeof(INT32U));
	if (ret > 0) {
		ret = 0;
	}

	up(privdata->usb_urb_sema);
     
	privdata->board_id = (INT16U) id;
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);

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
static INT32S adv_ioctrl(adv_usb_device *device, INT32U cmd, PTR_T arg)
{
	/* private_data *privdata = (private_data *) (device->private_data); */
	INT32S ret = 0;


	if (device == NULL) {
		return InvalidDeviceHandle;
	}
	
     
	switch (cmd) {
	case ADV_AI_INT_START:
		ret = adv_fai_int_start(device, (void *) arg);
		break;
	case ADV_AI_INT_SCAN_START:
		ret = adv_fai_int_scan_start(device, (void *) arg);
		break;
	case FAICHECK:
		ret = adv_fai_check(device, (void *) arg);
		break;
	case ADV_AI_STOP:
		ret = adv_fai_stop(device);
		break;
	case USB_LOCATE_DEVICE:
		ret = adv_locate_usb_device(device);
		break;
	case USB_SET_DEVICE_BOARDID:
		ret = adv_set_boardid(device, (INT32U) arg);
		break;
	case USB_STOP_LOCATE_DEVICE:
		ret = adv_stop_locate_usb_device(device);
		break;
	case ADV_USB_CTRL_TRANSFER:
		ret = adv_usb_ctrl_transfer(device, (void *) arg);
		break;
	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;
	case CLEARFLAG:
		ret = adv_clear_flag(device, (void *) arg);
		break;
	case ADV_GET_DEVINFO:
		ret = adv_get_devinfo(device, (void *) arg);
		break;
	default:
		return -EINVAL;
	}
	
	return ret;
}


/* USB-4716 fops */
adv_general_ops usb4716_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
	.poll           = adv_poll,
};

/* USB-4716 driver object */
adv_driver usb4716_driver = {
	.driver_name	= "usb4716",
	.devfs_name	= "usb4716",
	.ops		= &usb4716_fops,
};
 
/**
 * advdrv_init_one - Pnp to initialize the device, and allocate resource for the device.
 *
 * 
 * @dev: Points to the pci_dev device
 * @ent: Points to pci_device_id including the device info.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static void * advdrv_init_one(struct usb_device *dev, unsigned intf, const struct usb_device_id *id)
{
	private_data *privdata = NULL;
	adv_device *device = NULL;
	struct usb_interface_descriptor *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	struct semaphore *urb_sema = NULL;
	INT32S ret = 0;
	INT32U tmp = 0;
	INT32S i;     


	/* allocate urb sema */
	urb_sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (urb_sema == NULL) {
		return NULL;
	}
	init_MUTEX(urb_sema);
	
	
	/* allocate device structure */
	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		kfree(urb_sema);
		return NULL;
	}
	memset(device, 0, sizeof(adv_device));
	
	/* allocate private data structure */
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (privdata == NULL) {
		kfree(urb_sema);
		kfree(device);
		return NULL;
	}
	memset(privdata, 0, sizeof(private_data));
	
	/* initialize the private data in the device */
	privdata->interface = &(dev->actconfig->interface[intf]);
	privdata->udev = dev;
	privdata->usb_urb_sema = urb_sema;
	privdata->device_type = (id->idProduct << 16) | privdata->udev->descriptor.bcdDevice;

    
	/* get board id */
	if (privdata->udev == NULL) {
		kfree(urb_sema);
		kfree(device);
		kfree(privdata);
		return NULL;
	}
	
	ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
			       MAJOR_SYSTEM, (0x40 | 0x80), MINOR_READ_SWITCHID, 0,
			       &privdata->board_id, sizeof(INT16U));
	if (ret < 0) {
		kfree(urb_sema);
		kfree(device);
		kfree(privdata);
		return NULL;
	}

	/* select pipe */
	tmp = 2;
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			       MAJOR_DIRECT_IO, 0x40, MINOR_DIRECT_WRITE, 0x501,
			       &tmp, sizeof(INT32U));
	if (ret < 0) {
		kfree(urb_sema);
		kfree(device);
		kfree(privdata);
		return NULL;
	}

	/* boot 'start A/D later' function */
	tmp = 1;
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			       MAJOR_DIRECT_IO, 0x40, MINOR_DIRECT_WRITE, 0x502,
			       &tmp, sizeof(INT32U));
	if (ret < 0) {
		kfree(urb_sema);
		kfree(device);
		kfree(privdata);
		return ret;
	}

	/* init queue, spinlock and process info list */
	adv_process_info_header_init(&privdata->ptr_process_info);
	spin_lock_init(&privdata->spinlock);
	init_waitqueue_head(&privdata->event_wait);
	tasklet_init(&privdata->urb_tasklet, urb_tasklet_fn, (PTR_T) privdata);
	INIT_WORK(&privdata->fai_stop_work, adv_fai_stop_work, (VOID *) device);

	
	iface_desc = &(privdata->interface->altsetting[privdata->interface->act_altsetting]);
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		endpoint = &iface_desc->endpoint[i];

		if (!privdata->bulk_endpointAddr &&
		    (endpoint->bEndpointAddress & USB_DIR_IN) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		     == USB_ENDPOINT_XFER_BULK)) {
			privdata->bulk_endpointAddr = endpoint->bEndpointAddress;
			privdata->max_urbbuf_len = 1 << 14; /* 16K Bytes */
		}
	}
	privdata->control_endpointAddr = 0;     

	/* support multi-card */
	switch (privdata->device_type) {
	case USB4716:
		privdata->board_id &= 0x0f;
		advdrv_device_set_devname(device, "usb4716");
		break;
	default:
		tmp = (privdata->device_type >> 16) & 0xffff;
		if (tmp == 0x4716) {
			privdata->board_id &= 0x0f;
			advdrv_device_set_devname(device, "usb4716");
		}
		break;
	}
 
	
	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	usb_set_intfdata(privdata->interface, device);


	/* add device into driver list */
	ret = advdrv_add_device(&usb4716_driver, device);
	if (ret) {
		kfree(urb_sema);
		kfree(device);
		kfree(privdata);
		KdPrint("Add device failed!\n");
		return NULL;
	}

	KdPrint("Add a USB-4716 device: type=%xh; boardID=%xh.\n",
		privdata->device_type, privdata->board_id);

	return device;
}

#else
static INT32S __devinit advdrv_init_one(struct usb_interface *interface, const struct usb_device_id *id)
{
	private_data *privdata = NULL;
	adv_device *device = NULL;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	struct semaphore *urb_sema = NULL;
	INT32U tmp;
	INT32S ret = 0;
	INT16U i;

	
	/* allocate urb sema */
	urb_sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (urb_sema == NULL) {
		return -ENOMEM;
	}
	init_MUTEX(urb_sema);
	

	/* allocate device structure */
	device = (adv_device *) kmalloc(sizeof(adv_device), GFP_KERNEL);
	if (device == NULL) {
		kfree(urb_sema);
		return -ENOMEM;
	}
	memset(device, 0, sizeof(adv_device));
	
	/* allocate private data structure */
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (privdata == NULL) {
		kfree(urb_sema);
		kfree(device);
		return -ENOMEM;
	}
	memset(privdata, 0, sizeof(private_data));
	
	/* initialize the private data in the device */
	privdata->udev = usb_get_dev(interface_to_usbdev(interface));
	privdata->interface = interface;
	privdata->usb_urb_sema = urb_sema;
	privdata->device_type = (id->idProduct << 16) | privdata->udev->descriptor.bcdDevice;


	/* get board id */
	ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
			       MAJOR_SYSTEM, 0x40|0x80, MINOR_READ_SWITCHID, 0,
			       (INT8U *) &privdata->board_id, sizeof(INT16U));
	if (ret < 0) {
		kfree(urb_sema);
		kfree(device);
		kfree(privdata);
		return ret;
	}


	/* select pipe */
	tmp = 2;
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			       MAJOR_DIRECT_IO, 0x40, MINOR_DIRECT_WRITE, 0x501,
			       &tmp, sizeof(INT32U));
	if (ret < 0) {
		kfree(urb_sema);
		kfree(device);
		kfree(privdata);
		return ret;
	}

	/* boot 'start A/D later' function */
	tmp = 1;
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			       MAJOR_DIRECT_IO, 0x40, MINOR_DIRECT_WRITE, 0x502,
			       &tmp, sizeof(INT32U));
	if (ret < 0) {
		kfree(urb_sema);
		kfree(device);
		kfree(privdata);
		return ret;
	}

	/* init queue, spinlock and process info list */
	adv_process_info_header_init(&privdata->ptr_process_info);
	spin_lock_init(&privdata->spinlock);
	init_waitqueue_head(&privdata->event_wait);
	tasklet_init(&privdata->urb_tasklet, urb_tasklet_fn, (PTR_T) privdata);
	INIT_WORK(&privdata->fai_stop_work, adv_fai_stop_work, (VOID *) device);


	/* probe endpoint */
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		endpoint = &iface_desc->endpoint[i].desc;

		if(!privdata->bulk_endpointAddr &&
		   (endpoint->bEndpointAddress & USB_DIR_IN) &&
		   ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		    == USB_ENDPOINT_XFER_BULK)) {
			privdata->bulk_endpointAddr = endpoint->bEndpointAddress;
			privdata->max_urbbuf_len = 1 << 14; /* 16KB */
		}
	}
	privdata->control_endpointAddr = 0;

	
	/* support multi-card */
	switch (privdata->device_type) {
	case USB4716:
		privdata->board_id &= 0x0f;
		advdrv_device_set_devname(device, "usb4716");
		break;
	default:
		tmp = (privdata->device_type >> 16) & 0xffff;
		if (tmp == 0x4716) {
			privdata->board_id &= 0x0f;
			advdrv_device_set_devname(device, "usb4716");
		}
		break;
	}

	/* link the info into the other structures */
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	usb_set_intfdata(interface, device);


	/* add device into driver list */
	ret = advdrv_add_device(&usb4716_driver, device);
	if (ret) {
		kfree(urb_sema);
		kfree(device);
		kfree(privdata);
		KdPrint("Add device failed!\n");
		return ret;
	}

	KdPrint("Add a USB-4716 device: type=%xh; boardID=%xh.\n",
	       privdata->device_type, privdata->board_id);


	return 0;
}
#endif    
/************************************************************************
 * Function:		int advdrv_remove_one
 *
 * Description:  	Pnp to remove a device, and free resource for the device.
 * Parameters:
              dev 	-Points to the pci_dev device;
 			 pci_get_drvdata(dev) points to the private data in adv_device.
*************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))    
static void advdrv_remove_one(struct usb_device *dev, void *ptr)
{
	adv_device *device = (adv_device *) ptr;
	private_data *privdata = (private_data *) device->private_data;
	

	KdPrint("Device set NULL!\n" );
	adv_process_info_set_event_all(&privdata->ptr_process_info, 5, 1);

	if (privdata->user_cnt == 0) {
		wake_up_interruptible(&privdata->event_wait);
		adv_device_delete(device);
	} else {
		privdata->udev = NULL;
		wake_up_interruptible(&privdata->event_wait);
	}
}

#else
static void __devexit advdrv_remove_one(struct usb_interface *interface)
{
	adv_device *device = usb_get_intfdata(interface);
	private_data *privdata = (private_data *) device->private_data;


	KdPrint("Device set NULL!\n" );
	adv_process_info_set_event_all(&privdata->ptr_process_info, 5, 1);

	if (privdata->user_cnt == 0) {
		wake_up_interruptible(&privdata->event_wait);
		adv_device_delete(device);
	} else {
		privdata->udev = NULL;
		wake_up_interruptible(&privdata->event_wait);
	}

	usb_set_intfdata(interface, NULL);
}
#endif

struct usb_driver usb_driver = {
     .name = "usb4716",
     .probe = advdrv_init_one,
     .disconnect = advdrv_remove_one,
     .id_table = serial_usb_tbl,
};

/**
 * adv_device_delete - remove device
 */
static void adv_device_delete(adv_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT16U i;

	KdPrint("Remove a USB-4716 device\n" );

	/* we must guarantee the urb region is released */
	if (privdata->urb_queue != NULL) {
		for (i = 0; i < privdata->urb_num; i++) {
			if (privdata->urb_queue[i].urb != NULL) {
				usb_kill_urb(privdata->urb_queue[i].urb);
				usb_free_urb(privdata->urb_queue[i].urb);
				privdata->urb_queue[i].urb = NULL;
			}

			if (privdata->urb_queue[i].kern_cache != NULL) {
				kfree(privdata->urb_queue[i].kern_cache);
				privdata->urb_queue[i].kern_cache = NULL;
			}
		}
	}
	 
	/* terminate the tasklet */
	tasklet_kill(&privdata->urb_tasklet);

     
	if (privdata->urb_queue != NULL) {
		kfree(privdata->urb_queue);
		privdata->urb_queue = NULL;
	}
	
	if (privdata->user_pages) {
		adv_unmap_user_pages(privdata->user_pages,
				     privdata->page_num,
				     1);
	}

	advdrv_remove_device(&usb4716_driver, device);

   
	kfree(privdata->usb_urb_sema);
	kfree(privdata);
	kfree(device);
}

/**
 * usb4716_init - The module initialize USB-4716 driver
 *
 * Enumerate all PCI cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init usb4716_init(void)
{
	int error = 0;
	struct semaphore *sema = NULL;



	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (sema == NULL) {
		return -ENOMEM;
	}

	init_MUTEX(sema);
	
	_ADV_SET_DRIVER_SEMA(usb4716_driver, sema);
	advdrv_register_driver(&usb4716_driver);
	
	error = usb_register(&usb_driver);
	if (error != 0) {
		advdrv_unregister_driver(&usb4716_driver);
		return error;
	}
	
	return 0;
}

/**
 * pci1716_exit - Exit USB-4716 driver
 */
static void __exit usb4716_exit(void)
{
	struct semaphore *sema = NULL;


	usb_deregister(&usb_driver);
	if (advdrv_unregister_driver(&usb4716_driver)) {
		return;
	}
	
	_ADV_GET_DRIVER_SEMA(usb4716_driver, sema);
	
	kfree(sema);
}

module_init(usb4716_init);
module_exit(usb4716_exit);

MODULE_DEVICE_TABLE(usb, serial_usb_tbl);
MODULE_AUTHOR("Li Ang <li.ang@advantech.com.cn>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech USB4716 device driver module");
