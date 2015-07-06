
/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech USB-4761 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the USB-4761 driver module and device. 
Version history
	03/20/2006		Create by zhiyong.xie

************************************************************************/	


#include <linux/usb.h>

/* #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)) */
/* #include <linux/sched.h> */
/* #else */
/* #include <linux/kthread.h> */
/* #endif */

#include "USB4761.h"



static struct usb_device_id serial_usb_tbl[] =
{
     {  USB_DEVICE(VENDOR_USB_ID_ADVANTECH, ADVTECH_USB_PRODUCT_ID) },
     {}
};

/**
 * adv_device_delete - delete device from device list and free memory
 */
static void adv_device_delete(adv_device *device);

/************************************************************************
 * Function:		int adv_opendevice
 * 
 * Description:  	The advdrv_core driver calls to open the device.
 * Parameters:	
	     device 	-Points to the device object
*************************************************************************/

INT32S adv_opendevice( adv_device *device)
{
	INT32S ret = 0;
	INT32U ulTemp;
	private_data *privdata = (private_data *)device->private_data;

	if(down_interruptible(privdata->usb_urb_sema)){
		return -ERESTARTSYS;
	}
		
	if(privdata->ulAppCount == 0){
		
		if (privdata->udev == NULL) {
			up(privdata->usb_urb_sema);
			return -EFAULT;
		}
		
		ret =  adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
					MAJOR_SYSTEM, 0x40,
					MINOR_DEVICE_OPEN, 0,
					(BYTE *)&ulTemp, 4);
		if (ret < 0) {
			up(privdata->usb_urb_sema);
			return ret;
		}
		ret = 0;
	}

     	privdata->ulAppCount += 1;
	up(privdata->usb_urb_sema);
	
	adv_process_info_add(&privdata->ptr_process_info, 9);
	adv_process_info_enable_event(&privdata->ptr_process_info, 8, 1);
     
	return ret;//SUCCESS;
}

/************************************************************************
 * Function:		int adv_closedevice
 *
 * Description:  	The advdrv_core driver calls to close the device.
 * Parameters:
	    device 	-Points to the device object
*************************************************************************/
INT32S  adv_closedevice(adv_device *device)
{
	INT32S ret = 0;
	INT32U ulTemp;
	private_data *privdata = (private_data *)device->private_data;

	adv_process_info_remove(&privdata->ptr_process_info);          
	
	if(down_interruptible(privdata->usb_urb_sema)){
		return -ERESTARTSYS;
	}

	if(privdata->ulAppCount <= 1)
		privdata->ulAppCount = 0;
	else
		privdata->ulAppCount -= 1;
     
	if (privdata->ulAppCount == 0) {
	  	/* if usb device has unplugged */
		if (privdata->udev == NULL) {
			up(privdata->usb_urb_sema);
			adv_device_delete(device);
			return 0;
		}

		ret =  adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
					MAJOR_SYSTEM, 0x40,
					MINOR_DEVICE_CLOSE, 0,
					(BYTE*)&ulTemp, 4);
		if(ret >= 0){
			ret = 0;
		}
	}
	up(privdata->usb_urb_sema);

	return ret;
}

/************************************************************************
 * Function:	int adv_resetdevice
 *
 * Description:	The advdrv_core driver calls to reset the device.
 * Parameters:
	       device 	-Points to the device object
*************************************************************************/
INT32S adv_resetdevice( adv_device *device )
{
     return 0;

}

/************************************************************************
 * Function:		int adv_mmap
 *
 * Description:  	The advdrv_core driver calls to map the kernel memory to user
 * Parameters:
              device 	-Points to the device object
 	      vma 	-vm_area_struct
*************************************************************************/
INT32S adv_mmap( adv_device *device, struct file *file, struct vm_area_struct *vma )
{
     printk("This driver does not surpport mmap\n");
     return 0;
}


/************************************************************************
 * Function:		int adv_poll
 *
 * Description:  	The advdrv_core driver calls to map the kernel memory to user
 * Parameters:
              device 	-Points to the device object
 	      vma 	-vm_area_struct
*************************************************************************/
INT32U adv_poll(adv_device *device, struct file * file, struct poll_table_struct *wait)
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
INT32S adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  )
{
        
     return 0;
}



static INT32S adv_set_boardId(adv_device *device, INT32U boardID)
{
     INT32S retval = 0;
     private_data *privdata = (private_data *) device->private_data;
     
     if (privdata->udev == NULL) {
	     return -EFAULT;
     }
     
     retval = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			      MAJOR_SYSTEM, 0x40,
			      MINOR_WRITE_SWITCHID, 0,
			    (BYTE *)&boardID, sizeof(INT32U));
     if(retval < 0)
	  return retval ;
     
     return 0;
}
/* ************************************************************ */
/* get the usb device data format */
/* ************************************************************ */
/* static int adv_get_usbdata_format(adv_device *device) */
/* { */
/*      GET_USB_HW_INFO_STRUCT HwInfo; */
/*      int ret = 0; */
    
/*      private_data *privdata = (private_data *) device->private_data; */
     
/*      if(down_interruptible(privdata->usb_urb_sema)) */
/* 	  return -ERESTARTSYS; */
    
     
/*      ret = adv_usb_ctrl_in_sync(device, MAJOR_SYSTEM, MINOR_GET_USB_HW_INFO, */
/* 			    (BYTE *)&HwInfo, sizeof(GET_USB_HW_INFO_STRUCT)); */
/*      up(privdata->usb_urb_sema); */
     
/*      if(ret < 0) */
/* 	  return ret; */
/*      privdata->little_endian = HwInfo.LittleEndian; */
/*      privdata->general_DL = HwInfo.GeneralDL; */
/*      return 0; */
     
/* } */


/* static int adv_get_firmware_version(adv_device *device) */
/* { */
/*      int ret = 0; */
    
/*      private_data *privdata = (private_data *) device->private_data; */
     
/*      if(down_interruptible(privdata->usb_urb_sema)) */
/* 	  return -ERESTARTSYS; */
/*      ret = adv_usb_ctrl_in_sync(device, MAJOR_SYSTEM, MINOR_GET_FW_VERSION, */
/* 			    privdata->szFirmwareVersion, 64); */

/*      up(privdata->usb_urb_sema); */
     
/*      if(ret < 0) */
/* 	  return ret; */
     
/*      return 0; */
     
/* } */



/* static int adv_direct_write_EEdata(adv_device *device, USHORT usAddr, ULONG ulData) */
/* { */
/*      int ret = 0; */
    
/*      private_data *privdata = (private_data *) device->private_data; */
     
/*      ULONG lLastError = 0; */
/*      EE_WRITE_STRUCT EEWriteInfo; */
     
/*      EEWriteInfo.ulEEData = ulData; */
/*      EEWriteInfo.usAddr = usAddr; */
     
/*      if(down_interruptible(privdata->usb_urb_sema)) */
/* 	  return -ERESTARTSYS; */
/*      ret = usb_ctrl_out_sync(device, MAJOR_DIRECT_IO, MINOR_DIRECT_EE_WRITE_TX, */
/* 			     (BYTE *)&EEWriteInfo, sizeof(EEWriteInfo)); */
/*      if(ret < 0){ */
/* 	  up(privdata->usb_urb_sema); */
/* 	  return ret; */
/*      } */
     
/*      ret = adv_usb_ctrl_in_sync(device, MAJOR_SYSTEM, MINOR_GET_LAST_ERROR, */
/* 			    (BYTE *)&lLastError, sizeof(lLastError)); */
/*      if(ret < 0){ */
/* 	  up(privdata->usb_urb_sema); */
/* 	  return ret; */
/*      } */
     

/*      up(privdata->usb_urb_sema); */
     
/*      if(lLastError){ */
/* 	  return -EFAULT; */
/*      } */
     
          
/*      return 0; */
     
/* } */


/* static int adv_direct_read_EEdata(adv_device *device, USHORT usAddr, ULONG *pulData) */
/* { */
/*      int ret = 0; */
    
/*      private_data *privdata = (private_data *) device->private_data; */
     
 
/*      EE_READ_STRUCT EEReadInfo; */
     
/*      usAddr = usAddr; */
     
/*      if(down_interruptible(privdata->usb_urb_sema)) */
/* 	  return -ERESTARTSYS; */
/*      ret = usb_ctrl_out_sync(device, MAJOR_DIRECT_IO, MINOR_DIRECT_EE_READ_TX, */
/* 			     (BYTE *)&usAddr, sizeof(usAddr)); */
/*      if(ret < 0){ */
/* 	  up(privdata->usb_urb_sema); */
/* 	  return ret; */
/*      } */
     
/*      ret = adv_usb_ctrl_in_sync(device, MAJOR_DIRECT_IO, MINOR_DIRECT_EE_READ_RX, */
/* 			    (BYTE *)&EEReadInfo, sizeof(EEReadInfo)); */
/*      if(ret < 0){ */
/* 	  up(privdata->usb_urb_sema); */
/* 	  return ret; */
/*      } */
     
/*      up(privdata->usb_urb_sema); */
     
/*      *pulData = EEReadInfo.ulEEData; */
/*      if(EEReadInfo.ulLastError){ */
/* 	  return -EFAULT; */
/*      } */
     
          
/*      return 0; */
     
/* } */
/* **************get do status just like adv_get_do_status()******** */
/* ************* but not get the semaphore ************************ */
static INT32S adv_get_do_status_nosema(adv_device *device, INT16U usChlNum, INT16U *usData )
{
     INT32S ret = 0;
     private_data *privdata = (private_data *)device->private_data;
     
     ULONG lResult = 0;
     USB_DO_READ_TX UsbDoReadTx;
     USB_DO_READ_RX UsbDoReadRx;
     
     if(usChlNum > 1){
	     return -EFAULT;
     }
     
     
     UsbDoReadTx.usSize = sizeof(BYTE);
     UsbDoReadTx.usChannel = usChlNum;
    
     if (privdata->udev == NULL) {
	     return -EFAULT;
     }
     
     ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			    MAJOR_DIO, 0x40, MINOR_DO_READ_TX, 0,
			    (BYTE *)&UsbDoReadTx, sizeof(USB_DO_READ_TX));

     if(ret < 0){
	     return ret;
     }
     
     ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
			    MAJOR_DIO, 0x40|0x80, MINOR_DO_READ_RX, 0,
			    (BYTE *)&UsbDoReadRx, sizeof(USB_DO_READ_RX));
     if(ret < 0){
	  return ret;
     }
     
     
     if(usData){
	     *usData = (INT16U)((UsbDoReadRx.ulData) & 0xff);
     }
     
     lResult = UsbDoReadRx.ulRetCode;
     
     if(UsbDoReadRx.usSize != UsbDoReadTx.usSize){
	     return -EFAULT;
     }
     
     
     if(lResult){
	  return -EFAULT;
     }
          
     return 0;
     
}

/* /\* *********************** *\/ */
/* /\* ready for other use     *\/ */
/* /\* *********************** *\/ */
/* /\* static int adv_set_usb_led(adv_device *device, ULONG ulStatus) *\/ */
/* /\* { *\/ */
/* /\*      ULONG ulLedStatus; *\/ */
/* /\*      int nRet; *\/ */
/* /\*      private_data *privdata = (private_data *)device->private_data; *\/ */
     
/* /\*      //if(device->little_endian) *\/ */
/* /\*      ulLedStatus = ulStatus; *\/ */
/* /\*      if(down_interruptible(privdata->usb_urb_sema)) *\/ */
/* /\* 	  return -ERESTARTSYS; *\/ */
/* /\*      nRet = usb_ctrl_out_sync(device, MAJOR_SYSTEM, MINOR_USBLED, *\/ */
/* /\* 			      (BYTE *)&ulLedStatus, sizeof(ULONG)); *\/ */
/* /\*      up(privdata->usb_urb_sema); *\/ */
/* /\*      if(nRet < 0) *\/ */
/* /\* 	  return nRet; *\/ */
/* /\*      return 0; *\/ */
     
/* /\* } *\/ */
static INT32S adv_locate_usb_device(adv_device *device)
{
	INT32U ulTemp;
	INT32S ret = 0;
	private_data *privdata = (private_data *)device->private_data;
	
	if (privdata->udev == NULL) {
		return -EFAULT;
	}
	
	if(privdata->ulAppCount == 0){
		ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
				       MAJOR_SYSTEM, 0x40, 
				       MINOR_DEVICE_OPEN, 0,
				       (BYTE *)&ulTemp, sizeof(INT32U));
	}
	if(ret < 0){
		return ret;
	}
     
	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			       MAJOR_SYSTEM, 0x40,
			       MINOR_LOCATE, 0,
			       (BYTE *)&ulTemp, sizeof(INT32U));
       
	if(ret < 0){
		return ret;
	}
	
	return 0;
}

static INT32S adv_stop_locate_usb_device(adv_device *device)
{
	INT32U ulTemp;
	INT32S ret = 0;
	private_data *privdata = (private_data *)device->private_data;
     
	if (privdata->udev == NULL) {
		return -EFAULT;
	}
	
	if(privdata->ulAppCount == 0){
		ret =  adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
					MAJOR_SYSTEM, 0x40,
					MINOR_DEVICE_CLOSE, 0,
					(BYTE *)&ulTemp, sizeof(INT32U));
	}
     
     
	if(ret < 0){
		return ret;
	}
	
	return 0;
     
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
        
	private_data *privdata = (private_data *)(device->private_data);
	INT32S ret = 0;
     
	PT_DeviceSetParam    lpDeviceSetParam;
	PT_DeviceGetParam    lpDeviceGetParam;

	PT_EnableEvent       lpEnableEvent;
	PT_CheckEvent        lpCheckEvent;
     
     
	switch( cmd )
	{
	case USB_LOCATE_DEVICE:
		return adv_locate_usb_device(device);
	  
	case USB_STOP_LOCATE_DEVICE:
		return adv_stop_locate_usb_device(device);
		
	case USB_SET_DEVICE_BOARDID:
		ret = adv_set_boardId(device, arg);
		if(ret){
			return ret;
		}
		
		privdata->boardID = arg;
		_ADV_SET_DEVICE_BOARDID(device, privdata->boardID);
		return 0;
		
	case ADV_USB_CTRL_TRANSFER:
		return adv_usb_ctrl_transfer(device, (USB_TRANSFER_INFO *)arg);
	     
	case ADV_DEVGETPROPERTY:
		ret = copy_from_user(&lpDeviceGetParam,(void *)arg,sizeof(PT_DeviceGetParam));
		if(ret < 0){
			break;
		}

		ret = adv_device_get_property( device,
					       lpDeviceGetParam.nID,
					       lpDeviceGetParam.pData, 
					       lpDeviceGetParam.Length);
		break;
		
	case ADV_DEVSETPROPERTY:
		ret = copy_from_user(&lpDeviceSetParam,(void *)arg,sizeof(PT_DeviceSetParam));
		if(ret < 0){
			break;
		}
	
		ret = adv_device_set_property( device, 
					       lpDeviceSetParam.nID,
					       lpDeviceSetParam.pData,
					       lpDeviceSetParam.Length);
		break;

	case ADV_DIO_BITS:
		ret = adv_dio_ioctl(device, (void*)arg);
		break;
		
	case ADV_ENABLE_EVENT:
		ret = copy_from_user(&lpEnableEvent, (void *)arg, sizeof(PT_EnableEvent));
		if(ret < 0){
			break;
		}

		ret = adv_enable_event(device, lpEnableEvent.EventType,
				       lpEnableEvent.Enabled,
				       lpEnableEvent.Count);
		break;
		
	case CHECKEVENT:
	     	ret = copy_from_user(&lpCheckEvent, (PT_CheckEvent *)arg, sizeof(PT_CheckEvent));
		if(ret < 0){
			break;
		}

		ret = adv_check_event(device, (INT32U *)&lpCheckEvent.EventType,
				      lpCheckEvent.Milliseconds);
		if(ret < 0){
			break;
		}
		
		ret = copy_to_user((PT_CheckEvent *)arg,&lpCheckEvent,sizeof(PT_CheckEvent));
		break;
	  

	default:
		ret = -EFAULT;
		break;
	  
	}

	return ret;
}

/* USB-4761 fops */
adv_general_ops usb4761_fops =
{
     .opendevice	= adv_opendevice,
     .closedevice	= adv_closedevice,
     .read		= adv_read,
     .ioctrl		= adv_ioctrl,
     .resetdevice	= adv_resetdevice,
     .mmap		= adv_mmap,
     .poll              = adv_poll,
};

/* USB-4761 driver object */
adv_driver usb4761_driver =
{
     .driver_name		= "usb4761",
     .devfs_name		= "usb4761",
     .ops			= &usb4761_fops,
};

/************************************************************************
 * Function:		 int advdrv_init_one
 * 
 * Description:  	Pnp to initialize the device, and allocate resource for the device.
 * Parameters:	
	     dev	-Points to the pci_dev device
 	     ent 	-Points to pci_device_id including the device info.
*************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static void * advdrv_init_one(struct usb_device *dev, unsigned intf, const struct usb_device_id *id)
{
	private_data *privdata = NULL;
	adv_device *device = NULL;
	
	struct usb_interface_descriptor *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	struct semaphore *urb_sema = NULL;
	struct semaphore *disconnect_sema = NULL;
	struct urb        *urb_int = NULL; 
	
	INT32S i;
	INT32S ret = 0;
	INT32U do_status;
     
	urb_sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	disconnect_sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	device = (adv_device *)kmalloc(sizeof(adv_device), GFP_KERNEL);
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);
	
	if((urb_sema == NULL) || (disconnect_sema == NULL) 
	   || (privdata == NULL) || (device == NULL) ) {
		kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(privdata);
		kfree(device);
		return NULL;
	}
	
	urb_int = adv_usb_alloc_urb(0, GFP_KERNEL);
	if(urb_int == NULL){
		kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(privdata);
		kfree(device);
		return NULL;
	}

	//Initialize the private data in the device
	memset(privdata, 0, sizeof(private_data));
	memset(device, 0, sizeof(adv_device));     
	init_MUTEX(urb_sema);
	init_MUTEX(disconnect_sema);

	_ADV_SET_DEVICE_PRIVDATA(device, privdata);

	privdata->interface = &(dev->actconfig->interface[intf]);
	privdata->udev = dev;
	
	if (privdata->udev == NULL) {
		return -EFAULT;
	}
	ret =  adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
				MAJOR_SYSTEM, 0x40|0x80,
				MINOR_READ_SWITCHID, 0,
				&privdata->boardID, 4);
	if(ret < 0){
		kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(device);
		kfree(privdata);
		usb_free_urb(urb_int);
		return NULL;
	}


	
        /* get Do default value */
	ret = adv_get_do_status_nosema(device, 0, (INT16U *)&do_status);
	if(ret < 0){
		kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(device);
		kfree(privdata);
		usb_free_urb(urb_int);
		return NULL;
	}
//	privdata->dovalue_save[0] = (INT8U)(do_status & 0xff);
	
	iface_desc = &(privdata->interface->altsetting[privdata->interface->act_altsetting]);
	for(i = 0; i < iface_desc->bNumEndpoints; i++)
	{
		endpoint = &iface_desc->endpoint[i];
		
		if(!privdata->int_endpointAddr &&
		   (endpoint->bEndpointAddress & USB_DIR_IN) &&
		   ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		    == USB_ENDPOINT_XFER_INT)){
			privdata->int_endpointAddr = endpoint->bEndpointAddress;
			privdata->urb_int_in_size = endpoint->wMaxPacketSize;
		}
	}

	privdata->control_endpointAddr = 0;
	privdata->usb_urb_sema = urb_sema;
	privdata->disconnect_sema = disconnect_sema;
	privdata->urb_int = urb_int;

	
	privdata->urb_int_transfer = kmalloc(privdata->urb_int_in_size, GFP_KERNEL);
	privdata->urb_int_transfer_copy = kmalloc(privdata->urb_int_in_size, GFP_KERNEL);
	if((privdata->urb_int_transfer == NULL) || (privdata->urb_int_transfer_copy == NULL))
	{
		kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(device);
		kfree(privdata);
		usb_free_urb(urb_int);
		kfree(privdata->urb_int_transfer);
		kfree(privdata->urb_int_transfer_copy);
		return NULL;
	}

	tasklet_init(&privdata->check_event_tasklet, check_urb_event, (PTR_T)device);
	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	atomic_set(&privdata->tasklet_processed, 1);

	/* Transfer data memory of LONG form little-Endian to Big-Endian */
	privdata->boardID = privdata->boardID;
     
	/* BoardID range : 0 ~15 */
	privdata->boardID = privdata->boardID  & 0x0f;
	
	spin_lock_init( &privdata->spinlock);

	device->config=0;
	_ADV_SET_DEVICE_BOARDID(device,privdata->boardID);
	advdrv_device_set_devname(device, "usb4761");
	advdrv_add_device(&usb4761_driver, device);
	KdPrint("Add a USB-4761 device:boardID=%x.\n",privdata->boardID);
	return device;
}

#else

static INT32S __devinit
advdrv_init_one(struct usb_interface *interface, const struct usb_device_id *id)
{
	private_data *privdata = NULL;
	adv_device *device = NULL;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	struct semaphore *urb_sema = NULL;
	struct semaphore *disconnect_sema = NULL;
	struct urb       *urb_int = NULL;
	
	INT32S i;
	INT32S ret = 0;
     	INT32U do_status;
           
	urb_sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	disconnect_sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	device = (adv_device *)kmalloc(sizeof(adv_device), GFP_KERNEL);
	privdata = kmalloc(sizeof(private_data), GFP_KERNEL);

	if((urb_sema == NULL) || (disconnect_sema == NULL) 
	   ||(privdata == NULL) || (device == NULL)) 
	{
		kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(privdata);
		kfree(device);
		return -ENOMEM;
	}
	
	urb_int = adv_usb_alloc_urb(0, GFP_KERNEL);
	if(urb_int == NULL){
		kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(privdata);
		kfree(device);
		return -ENOMEM;
	}
	
    	//Initialize the private data in the device
	memset(privdata, 0, sizeof(private_data) );
	memset(device, 0, sizeof(adv_device));     
	init_MUTEX(urb_sema);
	init_MUTEX(disconnect_sema);

	_ADV_SET_DEVICE_PRIVDATA(device, privdata);

	privdata->udev = usb_get_dev(interface_to_usbdev(interface));
	privdata->interface = interface;
	
	if (privdata->udev == NULL) {
		return -EFAULT;
	}
	ret = adv_usb_ctrl_msg(privdata->udev, usb_rcvctrlpipe(privdata->udev, 0),
			       MAJOR_SYSTEM, 0x40|0x80,
			       MINOR_READ_SWITCHID, 0,
			       &privdata->boardID, 4);
	if(ret < 0)
	{
		kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(device);
		kfree(privdata);
		usb_free_urb(urb_int);
		return ret;
	}

/* 	init_waitqueue_head(&privdata->event_queue); */
/* 	init_waitqueue_head(&privdata->int_urb_queue); */
/* 	init_completion(&privdata->thread_completion); */
     
/* 	get Do default value */
	ret = adv_get_do_status_nosema(device, 0, (INT16U *)&do_status);
	if(ret < 0)
	{
		kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(device);
		kfree(privdata);
		usb_free_urb(urb_int);
		return ret;
	}

//	privdata->dovalue_save[0] = (INT8U)(do_status & 0xff);
     
	iface_desc = interface->cur_altsetting;
	for(i = 0; i < iface_desc->desc.bNumEndpoints; i++)
	{
		endpoint = &iface_desc->endpoint[i].desc;

		if(!privdata->int_endpointAddr &&
		   (endpoint->bEndpointAddress & USB_DIR_IN) &&
		   ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		    == USB_ENDPOINT_XFER_INT))
		{
			privdata->int_endpointAddr = endpoint->bEndpointAddress;
			privdata->urb_int_in_size =  endpoint->wMaxPacketSize;
	       
		}
     
	}

	privdata->control_endpointAddr = 0;
	privdata->usb_urb_sema = urb_sema;
	privdata->disconnect_sema = disconnect_sema;
	privdata->urb_int = urb_int;
	
	privdata->urb_int_transfer = kmalloc(privdata->urb_int_in_size, GFP_KERNEL);
	privdata->urb_int_transfer_copy = kmalloc(privdata->urb_int_in_size, GFP_KERNEL);
	if((privdata->urb_int_transfer == NULL) || (privdata->urb_int_transfer_copy == NULL))
	{
			kfree(urb_sema);
		kfree(disconnect_sema);
		kfree(device);
		kfree(privdata);
		usb_free_urb(urb_int);
		kfree(privdata->urb_int_transfer);
		kfree(privdata->urb_int_transfer_copy);
		return -ENOMEM;
	}
	
	tasklet_init(&privdata->check_event_tasklet, check_urb_event, (PTR_T)device);
	adv_process_info_header_init(&privdata->ptr_process_info);
	init_waitqueue_head(&privdata->event_wait);
	atomic_set(&privdata->tasklet_processed, 1);
	
	/* Transfer data memory of LONG form little-Endian to Big-Endian */
	privdata->boardID = privdata->boardID;
     
	/* BoardID range : 0 ~15 */
	privdata->boardID = privdata->boardID  & 0x0f;

	spin_lock_init( &privdata->spinlock );

	device->config=0;
	_ADV_SET_DEVICE_BOARDID(device, privdata->boardID);

	usb_set_intfdata(interface, device);
	advdrv_device_set_devname(device, "usb4761");
	advdrv_add_device(&usb4761_driver, device);
	KdPrint("Add a USB-4761 device:boardID=%x.\n",privdata->boardID);

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
	private_data *privdata = NULL;
	adv_device *device = (adv_device *)ptr;
	
	privdata =(private_data*) device->private_data;
	adv_process_info_set_event_all(&privdata->ptr_process_info, 8, 1);
	if(privdata->ulAppCount == 0)
		adv_device_delete(device);
	else
		privdata->udev = NULL;
	
}

#else
static void __devexit advdrv_remove_one(struct usb_interface *interface)
{
	private_data *privdata = NULL;
	adv_device *device = NULL;
     
	device = usb_get_intfdata(interface);
	KdPrint("Remove a USB-4761 device\n" );
	privdata =(private_data*) device->private_data;
	adv_process_info_set_event_all(&privdata->ptr_process_info, 8, 1);
	if(privdata->ulAppCount == 0)
		adv_device_delete(device);
	else
		privdata->udev = NULL;
}
#endif

/**
 * adv_device_delete - remove device
 */
void adv_device_delete(adv_device *device)
{
	private_data *privdata = NULL;
	struct semaphore *urb_sema = NULL;
	struct semaphore *disconnect_sema = NULL;
     
	privdata =(private_data*) device->private_data;
	
	urb_sema = privdata->usb_urb_sema;
	disconnect_sema = privdata->disconnect_sema;
     
	tasklet_kill(&privdata->check_event_tasklet);
	
	if(privdata->urb_int){
//		usb_kill_urb(privdata->urb_int);
		usb_free_urb(privdata->urb_int);
	}
	
	kfree(privdata->urb_int_transfer);
     	kfree(privdata->urb_int_transfer_copy);
	advdrv_remove_device(&usb4761_driver, device);
	
	KdPrint("Remove a USB-4761 device Board ID %x \n", privdata->boardID );
	kfree(privdata);
	kfree(device);
	kfree(urb_sema);
	kfree(disconnect_sema);
}

struct usb_driver usb_4761_driver = 
{
     .name = "usb4761",
     .id_table = serial_usb_tbl,
     .probe = advdrv_init_one,
     .disconnect = advdrv_remove_one,
};



/**
 * usb4761_init - modules initiallize function
 * 
 * Description:  register the driver in Advantech core driver.
 *               
 */
static int __init usb4761_init(void)
{
     INT32S error =0 ;
     struct semaphore *sema = NULL;
     sema = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
     if(sema == NULL)
     {
	  return -ENOMEM;
     }

     init_MUTEX(sema);
     _ADV_SET_DRIVER_SEMA(usb4761_driver, sema);
     advdrv_register_driver(&usb4761_driver);

     error = usb_register(&usb_4761_driver);
     if(error){
	  kfree(sema);
	  advdrv_unregister_driver(&usb4761_driver);
	  return error;
     }

     return 0;
	
}
/**
 * usb4761_exit - Module 
 * 
 * Description:  Exit  USB-4761 driver .
 *************************************************************************/
static void __exit usb4761_exit(void)
{
	struct semaphore *sema = NULL;

	usb_deregister( &usb_4761_driver );       
	advdrv_unregister_driver( &usb4761_driver );
	_ADV_GET_DRIVER_SEMA(usb4761_driver, sema);
	kfree(sema);
}

module_init(usb4761_init);
module_exit(usb4761_exit);

MODULE_DEVICE_TABLE(usb, serial_usb_tbl);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Advantech USB4761 device driver module");
