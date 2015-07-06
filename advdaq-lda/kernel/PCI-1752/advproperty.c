/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1752 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1752 driver module and device. 
Version history
	05/25/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
#include <linux/module.h>

#include "PCI1752.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif

/********************************************************************************
 * Description:  we get user buffer through this safe way
 *               
 * Input:        buf   -Points to the user buffer
 *               chan  -user buffer len
 *               size  -size of data that will be transfered to the user buffer
 *               item  -the data to be copy to the user buffer
 *               
 *               
 * Ouput:        errno                                        
 *               SUCCESS
 ********************************************************************************/

static INT32S adv_set_user_buffer(void *buf, INT32U *len, INT32U size, void *item)
{
	INT32U length;
	
	if( (buf == NULL) || (len == NULL) ){
		return -EFAULT;
	}
	
	if(copy_from_user(&length, len, sizeof(INT32U))){
		return -EFAULT;
	}
	
	if(length < size){
		return -EFAULT;
	}
	
	if (copy_to_user(buf, item, size)) {
		return -EFAULT;
	}	

	if(copy_to_user(len, &size, sizeof(INT32U))){
		return -EFAULT;
	}
	
	return 0;
}

/********************************************************************************
 * Description:  we get user buffer through this safe way
 *               
 * Input:        buf   -Points to the user buffer
 *               chan  -user buffer len
 *               size  -size of data that will be received from the user buffer
 *               item  -the data to be received
 *               
 *               
 * Ouput:        errno                                        
 *               SUCCESS
 ********************************************************************************/
static INT32S adv_get_user_buffer(void *buf, INT32U len, INT32U size, void *item)
{
	if( (size > len) || (buf == NULL) ){

	  return -EFAULT;
	}
	
	if(copy_from_user(item, buf, size)){
		return -EFAULT;
	}
	
	return 0;
}

//------------------------------------------------------------------
// Function: Set a configuration data item value.  
//           Each configuration item is distinguish by its name string.
//           Most Configuration item is used to hardware setting, and 
//           had better configured in configuration dialog box for 
//           default value. 
//           Using this function to change its setting if caller need to 
//           change configuration running time.
//           Not every item data is re-writable.
//
// Paramater: lDevHandle, IN, Device hansdle.
//            szParamName, IN, string of configuration item name.
//            pData, IN, Configuration data.
//            lLength, IN, Configuration data length.
//
// Return:    Error code.
//------------------------------------------------------------------
INT32S adv_device_set_property( 
     adv_device *device, 
     INT16U nID, 
     void*  pData,
     INT32U  lDataLength)
{
   
	private_data *privdata = (private_data*)(device->private_data);
	ULONG  diPortCount = 0;
	BYTE* pdata = NULL;
	
	INT16U ctrlcode = 0;
	INT32U i = 0;
	INT32S ret = 0;
	
	diPortCount = privdata->di_port_num;
	
	if(pData == NULL)
		return -EFAULT;
	
     	pdata = (BYTE *)kmalloc(lDataLength * sizeof(BYTE), GFP_KERNEL);
	if (!pdata)
		return -EFAULT;
	
     
	if (copy_from_user(pdata, pData, lDataLength * sizeof(BYTE))) {
		kfree(pdata);
		return -EFAULT;
	}
     
	switch (nID) {
     	case CFG_DiInterruptTriggerOnFallingEdge:
		if (lDataLength < diPortCount) {
			ret =  -EFAULT;
			break;
		}
		if (privdata->device_type == PCI1752) {
			ret = -EINVAL;
			break;
		}
		for (i = 0; i < diPortCount/2; i++) {
			if((*((INT16U *)pdata + i) & 0x0001) == 0x0001){
				privdata->event_trigger_mode &= ~(1<<i);
				ctrlcode = advInpw(privdata, 8 + 2*i) & (~0x04);
				advOutpw(privdata, 8 + 2*i, ctrlcode);
			} else {
				privdata->event_trigger_mode |= (INT16U) (1<<i);
				ctrlcode = advInpw(privdata, 8 + 2*i) | (INT16U )(0x04);
				advOutpw(privdata, 8 + 2*i, ctrlcode);
			}
			
		}
		
		break;
	  
	case CFG_DiInterruptTriggerOnRisingEdge:
		if (lDataLength < diPortCount) {
			ret = -EFAULT;
			break;
		}
		if (privdata->device_type == PCI1752) {
			ret = -EINVAL;
			break;
		}
		for(i = 0; i < diPortCount/2; i++){
			if((*((INT16U *)pdata + i) & 0x0001) == 0x0001){
				privdata->event_trigger_mode |= (INT16U)(1<<i);
				ctrlcode = advInpw(privdata, 8 + 2*i)|(INT16U)0x04;
				advOutpw(privdata, 8 + 2*i, ctrlcode);
			} else {
				privdata->event_trigger_mode &= ~(1<<i);
				ctrlcode = advInpw(privdata, 8 + 2*i) & (~0x04);
				advOutpw(privdata, 8 + 2*i, ctrlcode);
			}
			
		}

		break;

	case CFG_DiEnableLatch:
		if (lDataLength < sizeof(privdata->di_enable_latch)) {
		  ret = -EINVAL;
		  break;
		}
		
		privdata->di_enable_latch = (*(INT16U *)pdata) & ((1<<(diPortCount/2)) -1);
		
		for(i = 0; i < 4; i++){
			ctrlcode = advInpw(privdata, 8 + 2*i);
			ctrlcode &= 0xFE;
			ctrlcode |= (privdata->di_enable_latch & (1<<i)) ? 1 : 0;
			advOutpw(privdata, 8 + 2*i, ctrlcode);
		}
		break;
	
	case CFG_DoEnableChannelFreeze:
		if ((*(INT16U *)pdata & 0x0001) == 0x0001) 
			advOutpw(privdata, 0x12, 0x0001);
		else
			advOutpw(privdata, 0x12, 0x0000);
		break;
		
	default:
		ret = -EFAULT;
		break;
	
		  
	}
          
	kfree(pdata);
     	return ret;
     
     
}//DeviceSetParam



//------------------------------------------------------------------
// Function: Get a configuration data item value.  
//           Each configuration item is distinguish by its name string.
//
// Paramater: lDevHandle, IN, Device hansdle.
//            szParamName, IN, string of configuration item name.
//            pData, IN, Configuration data.
//            lLength, IN, Configuration data length.
//
// Return:    Error code.
//------------------------------------------------------------------
INT32S adv_device_get_property(adv_device *device, INT16U nID, void*  pdata, INT32U* plength) 
{
	private_data *privdata = (private_data*)(device->private_data);
	INT32U di_port_count = privdata->di_port_num;
	INT32U do_port_count = privdata->do_port_num;
	INT32U temp;
	INT32S i = 0;
	INT16U *data = NULL;
	
	switch ( nID ) {
	case CFG_BoardID:
		return adv_set_user_buffer(pdata, plength, 
					   sizeof(privdata->boardID),
					   &privdata->boardID);

	case CFG_BaseAddress:
		return adv_set_user_buffer(pdata, plength,
					   sizeof(privdata->iobase),
					   &privdata->iobase);

	case CFG_Interrupt:		/* irq number */
		return adv_set_user_buffer(pdata, plength,
					   sizeof(privdata->irq),
					   &privdata->irq);
	case CFG_BusNumber:
		return adv_set_user_buffer(pdata, plength,
					   sizeof(privdata->pci_bus),
					   &privdata->pci_bus);
	
	case CFG_SlotNumber:
		return adv_set_user_buffer(pdata, plength,
					   sizeof(privdata->pci_slot),
					   &privdata->pci_slot);
	     
	case CFG_DiDataWidth:
		return adv_set_user_buffer(pdata, plength,
					   sizeof(di_port_count),
					   &di_port_count);

	case CFG_DoDataWidth:
		return adv_set_user_buffer(pdata, plength,
					   sizeof(do_port_count),
					   &do_port_count);
	
	case CFG_DiChannelCount:
		temp = di_port_count*8;
		return adv_set_user_buffer(pdata, plength,
					   sizeof(temp),
					   &temp);
	
	case CFG_DoChannelCount:
		temp = do_port_count*8;
		return adv_set_user_buffer(pdata, plength,
					   sizeof(temp),
					   &temp);
	
	case CFG_DiPortCount:
		return adv_set_user_buffer(pdata, plength,
					   sizeof(di_port_count),
					   &di_port_count);
	
	case CFG_DoPortCount:
		return adv_set_user_buffer(pdata, plength,
					   sizeof(do_port_count),
					   &do_port_count);
	
	case CFG_DiInterruptTriggerOnRisingEdge:
		if (copy_from_user(&temp, plength, sizeof(INT32U)))
			return -EFAULT;
		if(temp < di_port_count){
			return -EFAULT;
		}
		if (privdata->device_type == PCI1752) {
			return -EINVAL;
		}
		
		data = kmalloc(8*sizeof(INT8U), GFP_KERNEL);
		if (!data)
			return -EFAULT;
		memset(data, 0, sizeof(data));
		
		temp = 1;
		for (i = 0; i < di_port_count/2; i++) {
			if(((privdata->event_trigger_mode & temp) >> i) == 1) 
				data[i] = 0x0001;
			else 
				data[i] = 0x0000;
			
			temp = temp << 1;
		}

		temp = adv_set_user_buffer(pdata, plength, di_port_count, data);
		kfree(data);
		return temp;
		

	case CFG_DiInterruptTriggerOnFallingEdge:
		if (copy_from_user(&temp, plength, sizeof(INT32U)))
			return -EFAULT;
		
		if(temp < di_port_count){
			return InvalidInputParam;
		}
		if (privdata->device_type == PCI1752) {
			return -EINVAL;
		}
		data = kmalloc(8*sizeof(INT8U), GFP_KERNEL);
		if (!data)
			return -EFAULT;
		
		memset(data, 0, sizeof(data));
		temp = 1;
		for (i = 0; i < di_port_count/2; i++) {
			if ((privdata->event_trigger_mode & temp) == 0)
				data[i] = 0x0001;
			else
				data[i] = 0x0000;
			
			temp = temp << 1;
		}
		temp =  adv_set_user_buffer(pdata, plength, di_port_count, data);
		kfree(data);
		return temp;
	
	case CFG_DiEnableLatch:
		privdata->di_enable_latch &= 0x0f;
		return adv_set_user_buffer(pdata, plength, sizeof(privdata->di_enable_latch), &privdata->di_enable_latch);

		
	case CFG_DoEnableChannelFreeze:
		temp = advInpw(privdata, 0x12) & 0x1;
		return adv_set_user_buffer(pdata, plength, sizeof(temp), &temp);
		
	default:
		return -EFAULT;
	}

	return 0;

}//DeviceGetParam
