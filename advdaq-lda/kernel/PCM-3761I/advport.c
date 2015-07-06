
#include "PCM3761I.h"


/**
 * adv_read_port - read specified IO port
 *                                                            
 * @device: Points to the device object          
 * @arg: argument from user
 */
INT32S adv_read_port(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);     
	PT_PortOperation port_operation;
	INT32U port_num, data_len, ret_len;
	INT32U *buf;
	long port_type;
	void *user_buf;



	if (copy_from_user(&port_operation, arg, sizeof(PT_PortOperation))) {
		return MemoryCopyFailed;
	}

	port_type = port_operation.PortType;
	port_num = port_operation.PortNumber;
	data_len = port_operation.DataLength;
	ret_len = port_operation.ReturnedLength;
	user_buf = port_operation.DataBuffer;

	buf = (INT32U *) kmalloc(sizeof(INT32U), GFP_KERNEL);
	if (!buf) {
		return MemoryAllocateFailed;
	}
	memset(buf, 0, sizeof(INT32U));

	if (data_len > sizeof(INT32U)) {
		kfree(buf);
		return InvalidInputParam;
	}
     
	switch (port_type) {
	case AD_READ_PORT_UCHAR:
		*(INT8U *) buf = advInp(privdata, port_num);
		inl(privdata->iobase + port_num);
		ret_len = sizeof(INT8U);
		break;
	case AD_READ_PORT_USHORT:
		*(INT16U *) buf = advInpw(privdata, port_num);
		ret_len = sizeof(INT16U);
		break;
/* 	case AD_READ_PORT_ULONG: */
/* 		*(INT32U *) buf = advInpdw(privdata, port_num); */
/* 		ret_len = sizeof(INT32U); */
/* 		break; */
	default:
		break;
	}

	if (copy_to_user(user_buf, buf, data_len)) {
		kfree(buf);     
		return MemoryCopyFailed;
	}

	if (copy_to_user((void *) arg, &port_operation, sizeof(PT_PortOperation))) {
		kfree(buf);     
		return MemoryCopyFailed;
	}
     
	kfree(buf);

	return 0;
}


/**
 * adv_write_port - write specified IO port
 *                                                            
 * @device: Points to the device object          
 * @arg: argument from user
 */
INT32S adv_write_port(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);     
	PT_PortOperation port_operation;
	INT32U port_num, data_len, ret_len;
	INT32U *buf;
	long port_type;     
	void *user_buf;
     


	if (copy_from_user(&port_operation, arg, sizeof(PT_PortOperation))) {
		return MemoryCopyFailed;
	}

	port_type = port_operation.PortType;
	port_num = port_operation.PortNumber;
	data_len = port_operation.DataLength;
	ret_len = port_operation.ReturnedLength;
	user_buf = port_operation.DataBuffer;

	buf = (INT32U *) kmalloc(sizeof(INT32U), GFP_KERNEL);
	if (!buf) {
		return MemoryAllocateFailed;
	}
	memset(buf, 0, sizeof(INT32U));

	if (copy_from_user(buf, (INT32U *) user_buf, data_len)) {
		kfree(buf);
		return MemoryCopyFailed;
	}
     
	if (data_len > sizeof(INT32U)) {
		kfree(buf);
		return InvalidInputParam;
	}

	switch (port_type) {
	case AD_WRITE_PORT_UCHAR:
		advOutp(privdata, port_num, *((INT8U *) buf));
		ret_len = sizeof(INT8U);
		break;
	case AD_WRITE_PORT_USHORT:
		advOutpw(privdata, port_num, *(INT16U *) buf);
		ret_len = sizeof(INT16U);
		break;
/* 	case AD_WRITE_PORT_ULONG: */
/* 		advOutpdw(privdata, port_num, *(INT32U *) buf); */
/* 		ret_len = sizeof(INT32U); */
/* 		break; */
	default:
		break;
	}

	if (copy_to_user((void *) arg, &port_operation, sizeof(PT_PortOperation))) {
		kfree(buf);     
		return MemoryCopyFailed;
	}

	kfree(buf);     

	return 0;
}
