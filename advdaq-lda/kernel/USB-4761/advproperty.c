#include "USB4761.h"

/**
 * adv_get_usbdata_format - get device's data format
 * @device - device handle
 */
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

/**
 * adv_get_firmware_version -
 * @device - device handle
 */




/**
 * adv_set_device_property - set device property
 * @device - device handle
 * @nID - property indentifer
 * @pData - buffer
 * @lDataLength - buffer size in byte
 */
INT32S adv_device_set_property(adv_device *device, INT16U nID, void*  pData, INT32U  lDataLength)
{
   
	private_data *privdata = (private_data*)(device->private_data);
	INT32S ret = 0;
	BYTE* pdata = NULL;
     
	pdata = (BYTE *)kmalloc(lDataLength * sizeof(BYTE), GFP_KERNEL);
	if(!pdata){
		return -ENOMEM;
	}
     
	ret = copy_from_user(pdata, pData, lDataLength * sizeof(BYTE));
	if(ret){
		kfree(pdata);
		return ret;
	}
     
	switch(nID)
	{
     	case CFG_IrqDiTriggerSignals: /* Di trigger falling:0 rising: 1 */
		if(pData == NULL || lDataLength < sizeof(BYTE)){
			ret = -EFAULT;
			break;
		}
		privdata->us_int_trigger_mode_cfg = (INT16U)((*(INT16U *)pdata) & 0xFF);
		adv_set_di_event_trigger_mode(device, 0);
		break;
     
	case CFG_DiInterruptTriggerOnRisingEdge:
		if(pData == NULL || lDataLength < sizeof(BYTE)){
			ret = -EFAULT;
			break;
		}
		privdata->us_int_trigger_mode_cfg |= (INT16U)((*(INT16U *)pdata) & 0xFF);
		adv_set_di_event_trigger_mode(device, 0);
		break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
		if(pData == NULL || lDataLength < sizeof(BYTE)){
			ret = -EFAULT;
			break;
		}

		privdata->us_int_trigger_mode_cfg &= ~(INT16U)((*(INT16U *)pdata) & 0xFF);
		adv_set_di_event_trigger_mode(device, 0);
	  
		break;
/* 	case CFG_BoardID: */
/* 		ret = adv_set_board_id(device, (*(INT32U *)pdata)); */
/* 		if(!ret){ */
/* 			device->boardID = (*(INT32U *)pdata) & 0x0f; */
/* 			privdata->boardID = (*(ULONG *)pdata) & 0x0f; */
/* 		} */
		
/* 		break; */
/* 	case CFG_DoDefaultStatus: */
/* 		ret = adv_direct_write_EEdata(device, 0x1, (*(INT16U *)pdata)&0xff); */
/* 		if(ret){ */
/* 			privdata->us_do_default_value = (*(INT16U *)pdata)&0xff; */
/* 		} */
		
		break;
	  
	default:
		ret = -EFAULT;
		break;
	}

	kfree(pdata);
     
	return ret;
}



/**
 * adv_device_get_property - get device property
 * @device -
 * @nID -
 * @pData -
 * @pDataLength -
 */
INT32S adv_device_get_property(adv_device *device, INT16U nID, void*  pData, INT32U* pDataLength)
{
	private_data *privdata = (private_data*)(device->private_data);

	void*  cfgData = NULL;
	INT32U DataLength;		/* user send */
	INT32U sizeNeed= 0;
	BYTE   data[64];
	INT32S ret = 0;
	INT32U  diPortCount = 1;

	INT32U ulTemp;

	ret = copy_from_user(&DataLength, pDataLength, sizeof(INT32U));
	if(ret){
		return ret;
	}
     
	memset(data, 0, sizeof(data));
     
	cfgData = data;
     
	switch ( nID )
	{
	case CFG_BoardID:
	{
		sizeNeed = sizeof(privdata->boardID);
		memcpy(data, &privdata->boardID, sizeNeed);
	}
	break;

     
	case CFG_DiDataWidth:
	case CFG_DoDataWidth:
	{
		// pData should be a LONG *
		sizeNeed = sizeof(INT32U);
		memcpy(data, &diPortCount, sizeNeed);
	}
	break;
	case CFG_DiChannelCount:
	case CFG_DoChannelCount:
	{
		// pData should be a long *
		sizeNeed = sizeof(INT32U);
		*(INT32U *)data = diPortCount * 8;
	}
	break;
	case CFG_DiPortCount:
	case CFG_DoPortCount:
	{
		// pData should be a long *
		sizeNeed = sizeof(INT32U);
		*(INT32U *)data = diPortCount;
	}
	break;

	case CFG_DiInterruptTriggerOnRisingEdge:
	{
		sizeNeed = sizeof(BYTE);
		memcpy(data, &privdata->us_int_trigger_mode_cfg, sizeNeed);
	}
     
	break;
	  
	case CFG_DiInterruptTriggerOnFallingEdge:
	{
		sizeNeed = sizeof(BYTE);
		*(INT32U *)data =(INT32U) ~privdata->us_int_trigger_mode_cfg;
	}
     
	break;
     
	case CFG_IrqDiTriggerSignals:
	{
		sizeNeed = sizeof(BYTE);
		*(INT32U *)data = (INT32U)privdata->us_int_trigger_mode_cfg;
	}
	break;
     
	  
	default:
		ret = -EFAULT;
		break;
	}

	if(ret < 0){
		return ret;
	}
	
	ret = copy_to_user(pDataLength, &sizeNeed, sizeof(INT32U));
	if(ret){
		return ret;
	}

	if ( pData == NULL ){
		return -EFAULT;
	}

	if ( DataLength < sizeNeed ){
		return -EFAULT;
	}

	ret = copy_to_user(pData, data, sizeNeed);
	if(ret){
		return ret;
	}
	    
	return 0;
}

