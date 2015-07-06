/*****************************************************************
 *		
 *	Copyright 2006 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCI-1758U Device driver for Linux
 *
 * File Name: 
 *	advcnt.c
 * Abstract:
 *	This file contains routines for PCI1758U counter.
 *
 * Version history
 *      09/18/2006			Create by LI.ANG
 *
 *****************************************************************/
#define __NO_VERSION__
#include "PCI1758U.h"


/**
 * adv_watchdog_start - start watchdog counter
 *                                                                              
 * @device: Points to the device object                            
 */
static INT32S adv_watchdog_start(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U max_port_num;
	INT16U wd_cnt_val;
	INT8U wd_reg[3];
	INT8U tmp;
	INT16U i;


	switch (privdata->device_type) {
	case PCI1758UDI:
		return -EINVAL;
	case PCI1758UDO:
		max_port_num = MAX_DO_PORT;
		wd_reg[0] = 0x10;
		wd_reg[1] = 0x12;
		wd_reg[2] = 0x14;

		break;
	case PCI1758UDIO:
		max_port_num = MAX_DIO_PORT;
		wd_reg[0] = 0x34;
		wd_reg[1] = 0x36;
		wd_reg[2] = 0x38;

		break;
	default:
		max_port_num = 0;
		wd_reg[0] = 0x00;
		wd_reg[1] = 0x00;
		wd_reg[2] = 0x00;

		return -ENODEV;
	}

	adv_eeprom_enable(privdata); /* EWEN */

	for (i = 0; i < max_port_num / 2; i++) {
		adv_eeprom_write(privdata, 0x00 + (i * 2), /* WRITE */
				 ((INT16U *) privdata->do_preset_value)[i]);
	}
 
	for (i = 0; i < max_port_num / 2; i++) {
		adv_eeprom_write(privdata, 0x10 + (i * 2), /* WRITE */
				 ((INT16U *) privdata->do_watchdog_value)[i]);
	}


	wd_cnt_val = privdata->watchdog_counter & 0xffff;
	advOutpw(privdata, wd_reg[0], wd_cnt_val);

	wd_cnt_val = privdata->watchdog_counter >> 16;
	advOutpw(privdata, wd_reg[1], wd_cnt_val);

	tmp = advInp(privdata, wd_reg[2]);
	advOutp(privdata, wd_reg[2], tmp & 0xfe);
	advOutp(privdata, wd_reg[2], tmp | 0x01);
	for (i = 0; i < 8; i++) {
		printk("preset[%d]: 0x%x\n", i, privdata->do_preset_value[i]);
	}
	for (i = 0; i < 8; i++) {
		printk("overflow[%d]: 0x%x\n", i, privdata->do_watchdog_value[i]);
	}
	printk("wd_cnt[%d]: 0x%x\n", i, privdata->watchdog_counter);
	
	
	return 0;	
}	
 

/**
 * adv_watchdog_feed - reset watchdog counter to preset value
 *               
 * @device: Points to the device object
 */
static INT32S adv_watchdog_feed(adv_device *device)
{
	private_data *privdata = (private_data*) (device->private_data);
	INT8U wd_reg;
	INT8U tmp;


	switch (privdata->device_type) {
	case PCI1758UDI:
		return -EINVAL;
	case PCI1758UDO:
		wd_reg = 0x14;

		break;
	case PCI1758UDIO:
		wd_reg = 0x38;
		
		break;
	default:
		return -ENODEV;
	}

	tmp = advInp(privdata, wd_reg);
	advOutp(privdata, wd_reg, tmp & 0xfe);
	advOutp(privdata, wd_reg, tmp | 0x01);

 	
	return 0;
}


/**
 * adv_watchdog_stop - stop the watchdog counter
 *                                                                          
 * @device: Points to the device object
 */
static INT32S adv_watchdog_stop(adv_device *device)
{
	private_data *privdata = (private_data*) (device->private_data);
	INT8U wd_reg;
	

	switch (privdata->device_type) {
	case PCI1758UDI:
		return -EINVAL;
	case PCI1758UDO:
		wd_reg = 0x14;
		break;
	case PCI1758UDIO:
		wd_reg = 0x38;
		
		break;
	default:
		return -ENODEV;
	}
	
	advOutp(privdata, wd_reg, 0x00);

	return 0;
}


/**
 * adv_cnt_ioctl - counter dispatch function                                 
 *                                                                 
 * @device: Points to the device object               
 * @arg: argument from user                           
 */
INT32S adv_cnt_ioctl(adv_device *device, void *arg)
{
	cnt_struct structs;
	INT32S ret;	

	
	if (copy_from_user(&structs, arg, sizeof(adv_struct))) {
		return -EFAULT;
	}
     
     
	switch (structs.type) { 
	case WATCHDOG_START:
		ret = adv_watchdog_start(device);
		break;	  
	case WATCHDOG_FEED:
		ret = adv_watchdog_feed(device);
		break;
	case WATCHDOG_STOP:
		ret = adv_watchdog_stop(device);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
