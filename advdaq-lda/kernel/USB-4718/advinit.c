/***************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2007 IAG Software Team                  
 *                                                                         
 *			BeiJing R&D Center                        
 *                                                                         
 *			Advantech Co., Ltd.                       
 *                                                                         
 *		Advantech USB-4718 Device driver for Linux        
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	advinit.c                                                 
 * Abstract:                                                         
 * 	Initialize the USB-4718 driver module and device.         
 *                                                                         
 * Version history		                                          
 *	01/25/2007			Create by zhenyu.zhang                  
 *                                                                         
 *                                                                         
 ***************************************************************************/	
   

/* base module includes */
#include <linux/module.h>
#include <linux/usb.h> 
#include "USB4718.h"

#ifndef CONFIG_PCI
#  error "This driver needs to have PCI support."
#endif

static struct usb_device_id serial_usb_tbl[] = 
{
     { USB_DEVICE(0x1809, 0x4718) },
     {}, 

};

/**
 * adv_check_event - check device's event
 * @device - point to a special device
 * @EventType - special event type or NULL
 * @Milliseconds - the dead line of waiting for event
 *
 * 1.EventType == NULL
 *   EventType:
 *       return with whatever event if happened
 *       or NULL when time out
 * 2.EventType is a special event type
 *   EventType:
 *       return 1 if the event happened
 *       or 0 if the event not happened
 */ 
INT32S adv_check_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_CheckEvent check_event;
	INT32S event_th=0;

	if (copy_from_user(&check_event, arg, sizeof(PT_CheckEvent))) {
		return -EFAULT;
	}

	if (!check_event.EventType) {
		wait_event_interruptible_timeout(privdata->event_wait,
						 adv_process_info_isset_event(ptr) > 0,
						 check_event.Milliseconds * HZ / 1000);

		event_th = adv_process_info_check_event(ptr);

		if (event_th <= 0) {
			check_event.EventType = 0;
		} else {
			switch (event_th - 1) {
			case 0:
				check_event.EventType = ADS_EVT_DEVREMOVED;
				break;
								
			default:
				return -EFAULT;
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_DEVREMOVED:
			event_th = 0;
			break;
						
		default:
			return -EFAULT;
		}
		check_event.EventType = adv_process_info_check_special_event(ptr, event_th);		
	}
	
	if (copy_to_user(arg, &check_event, sizeof(PT_CheckEvent))) {
		return -EFAULT;
	}
	
	return 0;
}
		  
 /**
 * adv_enable_event - enable or disable a special event with count
 * @device - pointer to a special device
 * @EventType - special event type
 * @Enabled - enable or disable
 * @Count - the number of interrupt arouse a special event
 */ 
INT32S adv_enable_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_EnableEvent *lp_enable_event;
	INT32S ret=0;
	int event_enabled = 0;

	lp_enable_event = kmalloc(sizeof(PT_EnableEvent), GFP_KERNEL);
	if (!lp_enable_event) {
		return -EFAULT;
	}
     
	if (copy_from_user(lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		kfree(lp_enable_event);	
		return -EFAULT;
	}

	privdata->evt_cnt = lp_enable_event->Count;
	privdata->evt_style = lp_enable_event->Enabled;

	if (lp_enable_event->Enabled) {
		switch (lp_enable_event->EventType) {
			case ADS_EVT_DEVREMOVED:
				adv_process_info_enable_event(&privdata->ptr_process_info, 0, 1);
				break;

			default:
				ret = -EFAULT;
		}
	
	} else {
		switch (lp_enable_event->EventType) {
			case ADS_EVT_DEVREMOVED:
				adv_process_info_disable_event(&privdata->ptr_process_info, 0);
				break;

			default:
				ret = -EFAULT;
		}
	}
     
	kfree(lp_enable_event);	

	return ret;
}



/**
 * adv_device_delete - remove device
 */
void adv_device_delete(adv_device *device)
{
	private_data *privdata = NULL;
	privdata =(private_data*) device->private_data;
	
	del_timer(&privdata->usb_timer);
	advdrv_remove_device(&usb4718_driver, device);
	adv_process_info_remove(&privdata->ptr_process_info);
	
	kfree(privdata);
	kfree(device);
}
INT32S adv_dev_get_property(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_DeviceGetParam dev_get_param;
	INT32U length ;
	INT16U buffer[7];
	//INT32S ret;
	length = sizeof(buffer);     

	if(privdata->udev==NULL){
		return -EFAULT;
	}
	if (copy_from_user(&dev_get_param, arg, sizeof(PT_DeviceGetParam))) {
		return MemoryCopyFailed;
	}	

	if (copy_from_user(&length, dev_get_param.Length, sizeof(INT32U))) {
		return MemoryCopyFailed;
	}	
	switch (dev_get_param.nID) {
	case CFG_CascadeMode:
		if(usb4718_readtemp(privdata)){
			return -EFAULT;	
		}
		memcpy(buffer,privdata->usCjcTemperature,length);
		break;
	case CFG_BoardID:
		if(copy_to_user(dev_get_param.pData,&privdata->board_id,sizeof(INT16U))){
		return -EFAULT;
	}	
		return 0;	
	default:
		return -EFAULT;	
	}

	if (copy_to_user(dev_get_param.Length, &length, sizeof(INT32U))) {
		return MemoryCopyFailed;
	}

	if(copy_to_user(dev_get_param.pData, buffer, length)){
		return MemoryCopyFailed;
	}
	 
	return 0;
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
static INT32S adv_set_boardId(adv_device *device, INT32U boardID)
{
     INT32S retval = 0;
     private_data *privdata = (private_data *) device->private_data;
     if(boardID>16)
	 return -EFAULT;
     INT32U bid = LE2BE_DWord(boardID);

     retval = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0), MAJOR_SYSTEM, 0x40,MINOR_WRITE_SWITCHID, 0,(INT8U *)&bid, sizeof(INT32U));
     if(retval < 0)
	  return retval ;
     privdata->board_id = boardID; 
     return 0;
}

static INT32S usb4718_readtemp(private_data* ptr)
{
	private_data *privdata = (private_data *)ptr;

	CJC_TEMP_READ_STRUCT  CjcTempBin;
	INT32U errorcjc=0;
	INT16U				  usCjcTemperature;
	INT32S                 ret;
    	int                   iIteration,i;
	CjcTempBin.ulStatus = 0;
	iIteration = 0;

	if(privdata->udev==NULL){
		return -EFAULT;
	}

	for(i=0;i<5;i++){
		ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
					     MAJOR_AI, 0x40|0x80,
					     MINOR_CJCTEMP_READ, 0,
					     &CjcTempBin,
					     sizeof(CjcTempBin));
		if(ret<0||(0==LE2BE_DWord(CjcTempBin.ulStatus))){
			//continue ;
			return -EFAULT;	
		}

		usCjcTemperature = (USHORT)(LE2BE_DWord(CjcTempBin.ulCjcTemp)&0xffff);
		privdata->usCjcTemperature[privdata->ulCJCIndex%5] = usCjcTemperature;
    		privdata->ulCJCIndex++;
		if (privdata->usCjcTemperature[6]<5) {
			privdata->usCjcTemperature[6]++;
		}
	};
 
	ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
					     MAJOR_AI, 0x40|0x80,
					     MINOR_CJCERROR_READ, 0,
					     &errorcjc,
					     sizeof(errorcjc));
	if(ret<0){
			printk("%s",__FUNCTION__);
			return -EFAULT;	
	}
	privdata->usCjcTemperature[5] = LE2BE_DWord(errorcjc); 
	return 0;
}
/*
int init_usb4718_timer(private_data *privdata)
{
	init_timer(&privdata->usb_timer);
	privdata->usb_timer.function = usb4718_readtemp;
	privdata->usb_timer.data = (unsigned long)privdata;
	privdata->usb_timer.expires = 50000;
	add_timer(&privdata->usb_timer);

	return 0;
}
*/
int adv_locate_usb_device(adv_usb_device *device)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32S ret = 0;
	INT32U tmp;

	if (down_interruptible(privdata->usb_urb_sema)) {
		return -EFAULT;
	}
	
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_DEVICE_OPEN, 0,
				  (INT8U *) &tmp,
				  sizeof(INT32U));
	if(ret<0){
		up(privdata->usb_urb_sema);
		return ret;
	}

	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_LOCATE, 0,
				  (INT8U *) &tmp,
				  sizeof(INT32U));

	up(privdata->usb_urb_sema);
	
	if(ret>=0){
		 ret=0;
	}

	return ret;	
}
int adv_stop_locate_usb_device(adv_usb_device *device)
{
	private_data *privdata = (private_data*)device->private_data;
	INT32U tmp;
	INT32S ret = 0;

	if (down_interruptible(privdata->usb_urb_sema)) {
		return -EFAULT;
	}
	
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				  MAJOR_SYSTEM, 0x40,
				  MINOR_DEVICE_CLOSE, 0,
				  (INT8U *) &tmp,
				  sizeof(INT32U));

	up(privdata->usb_urb_sema);

	if(ret>=0) {
		ret=0;
	}

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
	
	if (ret<0 ) {
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
			return ret ;
		}
	}
	
	privdata->ulAppcount += 1;
	up(privdata->usb_urb_sema);

//	adv_process_info_enable_event(&privdata->ptr_process_info, 0, 1);
	ret = adv_process_info_add(&privdata->ptr_process_info, 4);
	
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
			return ret;
		}
	}
	
	up(privdata->usb_urb_sema);

//	adv_process_info_disable_event(&privdata->ptr_process_info, 0);
	
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
		
	case ADV_DEVGETPROPERTY:
		ret = adv_dev_get_property(device, (void *) arg);
		break;

	case ADV_ENABLE_EVENT:
		ret = adv_enable_event(device, (void *) arg);
		break;
		
	case CHECKEVENT:
		ret = adv_check_event(device, (void *) arg);
		break;

	default:
		return InvalidCommandType;
	}
	
	adv_process_info_set_error(&privdata->ptr_process_info, ret);

	return ret;
}


/* USB-4718 fops */
adv_general_ops usb4718_fops = {
	.opendevice	= adv_opendevice,
	.closedevice	= adv_closedevice,
	.ioctrl		= adv_ioctrl,
};

/* USB-4718 driver object */
adv_driver usb4718_driver = {
	.driver_name	= "usb4718",
	.devfs_name	= "usb4718",
	.ops		= &usb4718_fops,
};
 

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
	INT32S ret=0;
	INT32U tmp=0;
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
	device->interface = &(dev->actconfig->interface[intf]);
	device->udev = dev;
	privdata->usb_urb_sema = kmalloc(sizeof(struct semaphore),GFP_KERNEL);
	privdata->device_type = USB4718; 
	init_MUTEX(privdata->usb_urb_sema);
	privdata->ulAppcount = 0;

	privdata->udev = device->udev;
	privdata->device_type = USB4718;
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
	
     	privdata->board_id = LE2BE_DWord(tmp);
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);

	switch (privdata->device_type) {
	case USB4718:
		advdrv_device_set_devname(device, "usb4718");
		break;
	}

	ret = advdrv_add_device(&usb4718_driver, device);
	if (ret) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return ret;
	}

	printk("Add a USB-4718 device:boardID=%x.\n",privdata->board_id);
     
	return device;
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
	INT32S ret;
	INT32U tmp;
	//CJC_TEMP_READ_STRUCT  CjcTempBin;
	
	device = (adv_device *) kmalloc(sizeof(adv_usb_device), GFP_KERNEL);
	if (device == NULL) {
		printk("Could not kmalloc space for device!");
		return -ENOBUFS;
	}
	
	memset(device, 0, sizeof(adv_usb_device));

	/* alloc & initialize the private data structure */
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	if (!privdata) {
		kfree(device);
		return -ENOMEM;
	}

	/* Initialize the private data in the device */
	memset(privdata, 0, sizeof(private_data));
	device->udev = usb_get_dev(interface_to_usbdev(intf));
	device->interface = intf;
	privdata->usb_urb_sema = kmalloc(sizeof(struct semaphore),GFP_KERNEL);
	init_MUTEX(privdata->usb_urb_sema);
	privdata->ulAppcount = 0;
	privdata->udev = device->udev;
	privdata->device_type = USB4718;

    	privdata->ulCJCIndex = 0;
	adv_process_info_header_init(&privdata->ptr_process_info);
	
	init_waitqueue_head(&privdata->event_wait);

	ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
				     MAJOR_SYSTEM, 0x40|0x80,
				     MINOR_READ_SWITCHID, 0,
				     &tmp,
				     sizeof(INT32U));
	if(ret<0){
		printk("%s",__FUNCTION__);
		return -EFAULT;
	}
	/* link the info into the other structures */
     	privdata->board_id = LE2BE_DWord(tmp);
	_ADV_SET_DEVICE_BOARDID(device, privdata->board_id);
	_ADV_SET_DEVICE_PRIVDATA(device, privdata);

	usb_set_intfdata(intf, device);
	//init_usb4718_timer(privdata);

	/* support multi-card */
	switch (privdata->device_type) {
	case USB4718:
		advdrv_device_set_devname(device, "usb4718");
		break;
	}

	/* add device into driver list */
	ret = advdrv_add_device(&usb4718_driver, device);
	if (ret) {
		kfree(privdata->usb_urb_sema);
		kfree(device);
		kfree(privdata);
		printk("Add device failed!\n");
		return ret;
	}

	printk("Add a USB-4718 device:boardID=%x.\n",privdata->board_id);
     
	return SUCCESS;
}
#endif
#if LINUX_VERSION_CODE<KERNEL_VERSION(2,6,0)
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
	if(privdata->ulAppcount == 0)
		adv_device_delete(device);
	else
		privdata->udev = NULL;

	adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	wake_up_interruptible(&privdata->ptr_process_info,0,1);
	printk("Remove a USB-4718 device\n");
	
}
#else
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
	if(privdata->ulAppcount == 0)
		adv_device_delete(device);
	else
		privdata->udev = NULL;

	usb_set_intfdata(intf, NULL);
	usb_put_dev(device->udev);
	adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);

	printk("Remove a USB-4718 device\n");
}
#endif

struct usb_driver usb_driver = {
     .name = "usb4718",
     .probe = advdrv_init_one,
     .disconnect = advdrv_remove_one,
     .id_table = serial_usb_tbl,
};


/**
 * usb4718_init - The module initialize USB-4718 driver
 *
 * Enumerate all USB cards on the bus, register the driver in Advantech core driver.
 */
static INT32S __init usb4718_init(void)
{
	int error = 0;
	struct semaphore *sema = NULL;
	sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!sema) {
		return MemoryAllocateFailed;
	}

	init_MUTEX(sema);
	
	_ADV_SET_DRIVER_SEMA(usb4718_driver, sema);
	advdrv_register_driver(&usb4718_driver);
	
	error = usb_register(&usb_driver);
	if (error != SUCCESS) {
		advdrv_unregister_driver(&usb4718_driver);
		return error;
	}
	
	return SUCCESS;
}

/**
 * usb4718_exit - Exit USB-4718 driver
 */
static void __exit usb4718_exit(void)
{
	struct semaphore *sema = NULL;

	usb_deregister( &usb_driver );
	
	if(advdrv_unregister_driver( &usb4718_driver )){
		return;
	}
	
	_ADV_GET_DRIVER_SEMA(usb4718_driver, sema);

	printk("exit ok\n");
	
	kfree(sema);
}
module_init( usb4718_init );
module_exit( usb4718_exit );

MODULE_DEVICE_TABLE(usb, serial_usb_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech USB4718 device driver module");

