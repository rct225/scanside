/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2007 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech USB-4751 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the USB-4751 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	01/25/2007			Create by zhenyu.zhang                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   
/* base module includes */
#include <linux/module.h>
#include <linux/usb.h> 
#include "USB4751.h"

#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct usb_device_id serial_usb_tbl[] = 
{
     { USB_DEVICE(0x1809, 0x4751) },
     {}, 
};
static INT32S adv_set_boardId(adv_device *device, INT32U boardID)
{
     INT32S retval = 0;
     private_data *privdata = (private_data *) device->private_data;
     
     retval = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			      MAJOR_SYSTEM, 0x40,
			      MINOR_WRITE_SWITCHID, 0,
			    (BYTE *)&boardID, sizeof(INT32U));
     if(retval < 0)
	  return retval ;
     
     return 0;
}


void urb_tasklet_fn(PTR_T arg)
{
	INT32U nEventNumber;
	INT32U i,th;
	EVENTDATA *pEData ;

	private_data *privdata = (private_data *) arg;
	TRANSMEM *ptransem = (TRANSMEM*)privdata->urb_int_transfer_copy;
	
	nEventNumber = ptransem->nEventNum;
	for(i = 0; i < nEventNumber; i++){
		pEData = &ptransem->EData[i];
		if ((pEData->EventType & (1<<ADS_EVT_PORT0_Index))) 	{
			adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
		}

		if ((pEData->EventType & (1<<ADS_EVT_PORT1_Index))) 	{
			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
		}
	}
//	th = adv_process_info_check_event(&privdata->ptr_process_info);
	atomic_set(&privdata->tasklet_processed, 1);
	wake_up_interruptible(&privdata->event_wait);

	return;
}


void urb_int_callback(struct urb *urb, struct pt_regs *regs)
{
	private_data *privdata = (private_data *) urb->context;

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
		return;
	}
	
	if( (*(BYTE *)privdata->urb_int_transfer) && (atomic_dec_and_test(&privdata->tasklet_processed)) ){
		atomic_set(&privdata->tasklet_processed, 0);

		memcpy(privdata->urb_int_transfer_copy, privdata->urb_int_transfer, MAX_EVTBUF_LEN);
		
		tasklet_schedule(&privdata->urb_tasklet);
	}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
	if(privdata->evt_style)	{
		adv_usb_submit_urb(privdata->urb_int,SLAB_ATOMIC);
	}
#endif
	return;
}

/**
 * LE2BE_Dword - 
 *
 * @dwInData: Transfer Little-Endian DWORD data
 * @return : Transfer Big-Endian DWORD data
 */
DWORD LE2BE_DWord(DWORD	dwInData)
{
	INT8U*		pSrcBuf = (INT8U*)&dwInData;
	DWORD		dwOutData;
	INT8U*		pDesBuf = (INT8U*)&dwOutData;

	pDesBuf[0] = pSrcBuf[3]; 
	pDesBuf[1] = pSrcBuf[2]; 
	pDesBuf[2] = pSrcBuf[1]; 
	pDesBuf[3] = pSrcBuf[0]; 

	return dwOutData;
}

/**
 * LE2BE_Word - 
 *
 * @dInData: Transfer Little-Endian WORD data
 * @return : Transfer Big-Endian WORD data
 */
WORD LE2BE_Word(WORD	wInData  )
{
	INT8U*		pSrcBuf = (INT8U*)&wInData;
	WORD		wOutData;
	INT8U*		pDesBuf = (INT8U*)&wOutData;

	pDesBuf[0] = pSrcBuf[1]; 
	pDesBuf[1] = pSrcBuf[0]; 

	return wOutData;
}

INT32S adv_locate_usb_device(adv_usb_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret = 0;
	INT32U tmp;
printk("locate begin --------------------\n");
	if (down_interruptible(privdata->usb_urb_sema)) {
		return -EFAULT;
	}
	
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_DEVICE_OPEN, 0,
				  (INT8U *) &tmp,
				  sizeof(INT32U));

	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_LOCATE, 0,
				  (INT8U *) &tmp,
				  sizeof(INT32U));

	up(privdata->usb_urb_sema);
	
printk("locate begin end--------------------\n");
	return ret;	
}

INT32S adv_stop_locate_usb_device(adv_usb_device *device)
{
	private_data *privdata = (private_data*)device->private_data;
	INT32U tmp;
	INT32S ret = 0;

printk("locate stop --------------------\n");
	if (down_interruptible(privdata->usb_urb_sema)) {
		return -EFAULT;
	}
	
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_DEVICE_CLOSE, 0,
				  (INT8U *) &tmp,
				  sizeof(INT32U));

	up(privdata->usb_urb_sema);

printk("locate stop end--------------------\n");
	return ret;	
}



INT32S adv_usb_ctrl_transfer(adv_usb_device *device, void *arg)
{
	USB_TRANSFER_INFO trans_info;
	SETUP_PACKET *setup_packet;
	INT8U *buf;
	INT32S ret = 0;
	private_data *privdata = (private_data *)(device->private_data);

	ret = copy_from_user(&trans_info, arg, sizeof(USB_TRANSFER_INFO));
	if(ret){
		return -EFAULT;
	}
	
	setup_packet = (SETUP_PACKET *)&trans_info;
	
	buf = (INT8U *) kmalloc(setup_packet->wLength, GFP_KERNEL);
	if (buf == NULL) {
		return -ENOMEM;
	}
	
	ret = copy_from_user(buf, trans_info.pBuffer, setup_packet->wLength);
	if (ret) {
		return -EFAULT;
	}
	
	if(privdata->udev==NULL){
		return -EFAULT;
	}
	if (setup_packet->bmRequestType & 0x80) {
		/* In type */
		ret = adv_usb_ctrl_msg(privdata->udev,
				       usb_rcvctrlpipe(privdata->udev, 0),
				       setup_packet->bRequest,
				       setup_packet->bmRequestType,
				       setup_packet->wValue,
				       setup_packet->wIndex, buf,
				       setup_packet->wLength);
	} else {
		/* Out type */
		ret = adv_usb_ctrl_msg(privdata->udev,
				       usb_sndctrlpipe(privdata->udev, 0),
				       setup_packet->bRequest,
				       setup_packet->bmRequestType,
				       setup_packet->wValue,
				       setup_packet->wIndex, buf,
				       setup_packet->wLength);
	}
	
	if (ret < 0) {
		return ret;
	}
	
	if (setup_packet->bmRequestType & 0x80) {
		ret = copy_to_user(trans_info.pBuffer, buf, setup_packet->wLength);
		if (ret) {
			return -EFAULT;
		}
	}
	
	return 0;
}

/**
 * adv_opendevice - The advdrv_core driver calls to open the device.
 *
 * @device: Points to the device object
 */
static INT32S adv_opendevice(adv_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret;
	INT32U ulTemp;	
	
	if(down_interruptible(privdata->usb_urb_sema))
		return -EFAULT;
	
	if(privdata->ulAppcount==0){
		ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_DEVICE_OPEN, 0,
				  (INT8U *) &ulTemp,
				  sizeof(INT32U));

		if(ret<0){
			up(privdata->usb_urb_sema);
			return  ret;
		}
	}
	
	privdata->ulAppcount += 1;
	up(privdata->usb_urb_sema);

	ret = adv_process_info_add(&privdata->ptr_process_info, 4);
	adv_process_info_enable_event(&privdata->ptr_process_info, 2, 1);
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
	INT32U ulTemp;
	
	if(down_interruptible(privdata->usb_urb_sema)){
		return -EFAULT;
	}
	
	privdata->ulAppcount -= 1;
	if(privdata->ulAppcount<=0){
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
				       (INT8U *) &ulTemp,
				       sizeof(INT32U));

		if(ret<0){
			up(privdata->usb_urb_sema);
			return  ret;
		}
	}
	
	up(privdata->usb_urb_sema);

	ret = adv_process_info_remove(&privdata->ptr_process_info);
	
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
	private_data *privdata = (private_data *) (device->private_data);
	INT32S ret = 0;

	if (device == NULL)
		return InvalidDeviceHandle;
     
	switch (cmd) {
	case USB_LOCATE_DEVICE:
		ret = adv_locate_usb_device(device);
		break;
		
	case USB_STOP_LOCATE_DEVICE:
		ret = adv_stop_locate_usb_device(device);
		break;

	case USB_SET_DEVICE_BOARDID:
		ret = adv_set_boardId(device, arg);
		if(ret){
			return ret;
		}
		
		privdata->board_id = arg;
		_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
		return 0;
	
	case ADV_USB_CTRL_TRANSFER:
		ret = adv_usb_ctrl_transfer(device, (void *) arg);
		break;
		
	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
		
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;
		
	case ADV_DEVGETPROPERTY:
		ret = adv_dev_get_property(device, (void *) arg);
		break;
		
	case ADV_DEVSETPROPERTY:
		ret = adv_dev_set_property(device, (void *) arg);
		break;
		
	case ADV_GET_ERR_CODE:
		ret = adv_get_err_code(device, (void *) arg);
		break;
		
	default:
		return -EFAULT;
	}
	
	adv_process_info_set_error(&privdata->ptr_process_info, ret);

	return ret;
}


/* USB-4751 fops */
adv_general_ops usb4751_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
};

/* USB-4751 driver object */
adv_driver usb4751_driver = {
	.driver_name	= "usb4751",
	.devfs_name	= "usb4751",
	.ops		= &usb4751_fops,
};
 /**
 * adv_device_delete - remove device
 */
void adv_device_delete(adv_device *device)
{
	private_data *privdata = NULL;
     
	struct semaphore           *urb_sema = NULL;
	privdata =(private_data*) device->private_data;
	
	urb_sema = privdata->usb_urb_sema;
     
	tasklet_kill(&privdata->urb_tasklet);
	
	if(privdata->urb_int){
		usb_kill_urb(privdata->urb_int);
		usb_free_urb(privdata->urb_int);
	}
	
	kfree(privdata->urb_int_transfer);
     	kfree(privdata->urb_int_transfer_copy);
	advdrv_remove_device(&usb4751_driver, device);
	adv_process_info_remove(&privdata->ptr_process_info);
	
	kfree(privdata);
	kfree(device);
	kfree(urb_sema);
}


#if LINUX_VERSION_CODE<KERNEL_VERSION(2,6,0)
/**
 * advdrv_init_one - Pnp to initialize the device, and allocate resource for the device.
 *
 * 
 * @interface: Points to the usb_interface device
 * @id: Points to usb_device_id including the device info.
 */
static void * advdrv_init_one(struct usb_device *dev, unsigned intf, const struct usb_device_id *id)
{ 	
	private_data *privdata = NULL;
	adv_usb_device *device = NULL;
	INT32S ret=0,i;
	INT32U tmp=0;
	struct usb_interface_descriptor *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	
	device = (adv_device *) kmalloc(sizeof(adv_usb_device), GFP_KERNEL);
	if (device == NULL) {
		printk("Could not kmalloc space for device!");
		return NULL;
	}
	memset(device, 0, sizeof(adv_usb_device));

	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		return NULL;
	}
	memset(privdata, 0, sizeof(private_data));

	device->interface = &(dev->actconfig->interface[intf]);
	privdata->interface = device->interface;
	device->udev = dev;
	privdata->udev = dev; 
	privdata->usb_urb_sema = kmalloc(sizeof(struct semaphore),GFP_KERNEL);
	privdata->device_type = USB4751; 
	init_MUTEX(privdata->usb_urb_sema);
	privdata->ulAppcount = 0;

	adv_process_info_header_init(&privdata->ptr_process_info);

	tasklet_init(&privdata->urb_tasklet, urb_tasklet_fn, (PTR_T) privdata);

	init_waitqueue_head(&privdata->event_wait);

	ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
				     MAJOR_SYSTEM, 0x40|0x80,
				     MINOR_READ_SWITCHID, 0,
				     &privdata->board_id,
				     sizeof(INT16U));
	if(ret<0){
		printk("%s",__FUNCTION__);
		return NULL;
	}
	
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	
	switch (privdata->device_type) {
	case USB4751:
		advdrv_device_set_devname(device, "usb4751");
		break;
	default:
		return NULL;
	}

	ret = advdrv_add_device(&usb4751_driver, device);
	if (ret) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return NULL;
	}

	iface_desc = &(privdata->interface->altsetting[privdata->interface->act_altsetting]);
	for(i = 0; i < iface_desc->bNumEndpoints; i++){
		endpoint = &iface_desc->endpoint[i];
		
		if(!privdata->int_endpointAddr &&
		   (endpoint->bEndpointAddress & USB_DIR_IN) &&
		   ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		    == USB_ENDPOINT_XFER_INT)){
			privdata->int_endpointAddr = endpoint->bEndpointAddress;
			privdata->urb_int_in_size = endpoint->wMaxPacketSize;
		}
	}

	tasklet_init(&privdata->urb_tasklet, urb_tasklet_fn, (PTR_T) privdata);
	atomic_set(&privdata->tasklet_processed, 1);

	privdata->urb_int_transfer = kmalloc(MAX_EVTBUF_LEN, GFP_KERNEL);
	if (!privdata->urb_int_transfer) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}

	privdata->urb_int_transfer_copy= kmalloc(MAX_EVTBUF_LEN, GFP_KERNEL);
	if (!privdata->urb_int_transfer_copy) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		kfree(privdata->urb_int_transfer);
		return -EFAULT;
	}

	privdata->urb_int = adv_usb_alloc_urb(0, GFP_KERNEL);
	if (!privdata->urb_int) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return -EFAULT;
	}

	printk("Add a USB-4751 device:boardID=%x.\n",privdata->board_id);
     
	return device;
}

/**
 * advdrv_remove_one - Pnp to remove a device, and free resource for the device.
 * 
 * @interface: Points to the pci_dev device; 
 */
static void __devexit advdrv_remove_one(struct usb_device *dev, void *ptr)
{
	private_data *privdata = NULL; 
	adv_usb_device *device = NULL;

	device = (adv_usb_device *)ptr;
	privdata = (private_data *) device->private_data;

	adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
	wake_up_interruptible(&privdata->event_wait);
	if(privdata->ulAppcount == 0)
		adv_device_delete(device);
	else
		privdata->udev = NULL;


	printk("Remove a USB-4751 device\n");	
}

#else
/**
 * advdrv_init_one - Pnp to initialize the device, and allocate resource for the device.
 *
 * 
 * @interface: Points to the usb_interface device
 * @id: Points to usb_device_id including the device info.
 */
static int __devinit advdrv_init_one(struct usb_interface *intf, const struct usb_device_id *id)
{     
	private_data *privdata = NULL;
	adv_usb_device *device = NULL;
	INT32S ret,i;
	INT32U tmp;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	
	device = (adv_device *) kmalloc(sizeof(adv_usb_device), GFP_KERNEL);
	if (device == NULL) {
		printk("Could not kmalloc space for device!");
		return -ENOBUFS;
	}
	memset(device, 0, sizeof(adv_usb_device));

	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		return -ENOMEM;
	}
	memset(privdata, 0, sizeof(private_data));

	device->udev = usb_get_dev(interface_to_usbdev(intf));
	privdata->udev = device->udev; 
	device->interface = intf;
	privdata->interface = intf;
	privdata->usb_urb_sema = kmalloc(sizeof(struct semaphore),GFP_KERNEL);
	init_MUTEX(privdata->usb_urb_sema);
	privdata->ulAppcount = 0;
	privdata->device_type = USB4751;
	
	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);

	ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
				     MAJOR_SYSTEM, 0x40|0x80,
				     MINOR_READ_SWITCHID, 0,
				     &tmp,
				     sizeof(INT16U));
	if(ret<0){
		printk("%s",__FUNCTION__);
		return -EFAULT;
	}

     	privdata->board_id = tmp;
	
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	usb_set_intfdata(intf, device);

	/* probe endpoint */
	iface_desc = intf->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		endpoint = &iface_desc->endpoint[i].desc;

		if(!privdata->int_endpointAddr &&
		   (endpoint->bEndpointAddress & USB_DIR_IN) &&
		   ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		    == USB_ENDPOINT_XFER_INT)) {
			endpoint->wMaxPacketSize = 64;
			printk("%d endpoint %x\n", i, endpoint->bEndpointAddress);	   
			privdata->int_endpointAddr = endpoint->bEndpointAddress;
			privdata->urb_int_in_size = endpoint->wMaxPacketSize;
	       
		}
     
	}

	tasklet_init(&privdata->urb_tasklet, urb_tasklet_fn, (PTR_T) privdata);
	atomic_set(&privdata->tasklet_processed, 1);

	switch (privdata->device_type) {
	case USB4751:
		advdrv_device_set_devname(device, "usb4751");
		break;
	}

	ret = advdrv_add_device(&usb4751_driver, device);
	if (ret) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return ret;
	}

	privdata->urb_int_transfer = kmalloc(MAX_EVTBUF_LEN, GFP_KERNEL);
	if (!privdata->urb_int_transfer) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		return -EFAULT;
	}

	privdata->urb_int_transfer_copy= kmalloc(MAX_EVTBUF_LEN, GFP_KERNEL);
	if (!privdata->urb_int_transfer_copy) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		kfree(privdata->urb_int_transfer);
		return -EFAULT;
	}
	privdata->urb_int = adv_usb_alloc_urb(0, GFP_KERNEL);
	if (!privdata->urb_int) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return -ENODEV;
	}

	printk("Add a USB-4751 device:boardID=%x.\n",privdata->board_id);
     
	return 0;
}

/**
 * advdrv_remove_one - Pnp to remove a device, and free resource for the device.
 * 
 * @interface: Points to the pci_dev device; 
 */
static void __devexit advdrv_remove_one(struct usb_interface *intf)
{
	private_data *privdata = NULL; 
	adv_usb_device *device = NULL;

	device = (adv_usb_device*)usb_get_intfdata(intf);
	
	privdata = (private_data *) device->private_data;

	usb_set_intfdata(intf, NULL);
	usb_put_dev(device->udev);

	if(privdata->ulAppcount == 0){
		adv_device_delete(device);
	}else{
		privdata->udev = NULL;
	}

	adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
	
	printk("Remove a USB-4751 device\n");
	
}
#endif

struct usb_driver usb_driver = {
     .name = "usb4751",
     .probe = advdrv_init_one,
     .disconnect = advdrv_remove_one,
     .id_table = serial_usb_tbl,
};

/**
 * usb4751_init - The module initialize USB-4751 driver
 *
 * Enumerate all PCI cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init usb4751_init(void)
{
	int error = 0;
	struct semaphore *sema = NULL;
	
	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return MemoryAllocateFailed;
	}

	init_MUTEX(sema);
	_ADV_SET_DRIVER_SEMA(usb4751_driver, sema);
	advdrv_register_driver(&usb4751_driver);

	error = usb_register(&usb_driver);
	if (error) {
		advdrv_unregister_driver(&usb4751_driver);
		return error;
	}
	
	return 0;
}

/**
 * usb4751_exit - Exit USB-4751 driver
 */
static void __exit usb4751_exit(void)
{
	struct semaphore *sema = NULL;

	usb_deregister( &usb_driver );
	
	if(advdrv_unregister_driver( &usb4751_driver )){
		return;
	}
	
	_ADV_GET_DRIVER_SEMA(usb4751_driver, sema);
	
	kfree(sema);
	printk("usb4751 exit ok\n");
}

module_init( usb4751_init );
module_exit( usb4751_exit );

MODULE_DEVICE_TABLE(usb, serial_usb_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech USB4751 device driver module");

