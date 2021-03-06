
#define __NO_VERSION__
#include "PCM3761I.h"
 
static INT32S adv_di_read(adv_device *device, adv_struct *structs, INT8U *data) 
{ 
	private_data *privdata = (private_data*)(device->private_data); 
 	INT8U port_reg[MAX_DI_PORT]; 
 	INT16U i; 


 	switch (privdata->device_type) { 
	case PCM3761I:
 		port_reg[0] = 0x01; 
 		break; 
 	default: 
 		return -EINVAL; 
 	} 
        
 	for (i = 0; i < structs->portcount; i++) { 
 		data[i] = advInp(privdata, port_reg[i + structs->portstart]); 
 	} 

 	return 0;	 
 }	 


/**
 * adv_do_write - write digital output data to the specified   
 *                digital I/O port                             
 *                                                            
 * @device: Points to the device
 * @structs: copy of argument from user
 * @data: store the data write to I/O port 
 */
static INT32S adv_do_write(adv_device *device, adv_struct *structs, INT8U *data)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT8U port_reg[MAX_DI_PORT];
	INT16U i;



	switch (privdata->device_type) {
	case PCM3761I:
		port_reg[0] = 0x00;
		break;
	default:
		return -EINVAL;
	}
        
	if (down_interruptible(privdata->dio_sema)) {
		return -EFAULT;
	}
	
	for (i = 0; i < structs->portcount; i++) {
		advOutp(privdata, port_reg[i + structs->portstart], data[i]);
		privdata->do_prestate[i + structs->portstart] = data[i];
	}

	up(privdata->dio_sema);

	return 0;	
}	


/**
 * adv_do_get - read back latest output data from the specified   
 *              digital output port                             
 *                                                            
 * @device: Points to the device              
 * @structs: copy of argument from user
 * @data: store the data read from previous output data
 */
static INT32S adv_do_get(adv_device *device, cnt_struct *structs, INT16U *data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT8U port_reg[MAX_DI_PORT];
	INT16U i;


	switch (privdata->device_type) {
	case PCM3761I:
		port_reg[0] = 0x00;
		break;
	default:
		return -EINVAL;
	}

        
	if (down_interruptible(privdata->dio_sema)) {
		return -EFAULT;
	}

	for (i = 0; i < structs->portcount; i++) {
		data[i] = advInp(privdata, port_reg[i + structs->portstart]);
	}

	up(privdata->dio_sema);

	return 0;	
}	


/**
 * adv_dio_ioctl - dio dispatch function
 *                                                            
 * @device: Points to the device              
 * @arg: argument from user
 */
INT32S adv_dio_ioctl(adv_device *device, void *arg)
{
	adv_struct structs;
	INT8U *data = NULL;
	INT32S ret = 0;	


	if (copy_from_user(&structs, arg, sizeof(adv_struct))) {
		return -EFAULT;
	}
     
	if (structs.buffer != NULL) {
		data = kmalloc(structs.portcount * sizeof(INT8U), GFP_KERNEL);
		if (!data) {
			return -ENOMEM;
		}
		
		if (copy_from_user(data, structs.buffer, structs.portcount * sizeof(INT8U))) {
			kfree(data);
			return -EFAULT;
		}
	}
	
	switch (structs.type) {
 	case DI: 
 		ret = adv_di_read(device, &structs, data); 
 		break; 
	case DO:
		ret = adv_do_write(device, &structs, data);
		break;
	case GETDO:
		ret = adv_do_get(device, &structs, (INT16U *) data);
		break;
	default:
		kfree(data);
		return -EINVAL;
	}

	if (!ret) {
		if (copy_to_user(structs.buffer, data, structs.portcount * sizeof(INT8U))) {
			kfree(data);
			return -EFAULT;
		}
	}

	kfree(data);

	return ret;
}
