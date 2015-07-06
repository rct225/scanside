/*****************************************************************
 *		
 *	Copyright 2004 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCI-1760 Device driver for Linux
 *
 * File Name: 
 *	advcnt.c
 * Abstract:
 *	This file contains routines for PCI1760 counter.
 *
 * Version history
 *      02/07/2006			Create by LI.ANG
 *
 *****************************************************************/
#define __NO_VERSION__
#include "PCI1760.h"

/********************************************************************************
 * Description:  read the current counter total without disturbing the counting 
 *               process and return the count value and overflow conditions     
 *                                                                              
 * Input:        device -Points to the device object                            
 *                                                                              
 *               data[0] -counter number
 *               data[1] -overflow flag                                         
 *               data[2] -count value return to user                            
 *                                                                              
 * Ouput:        errno                                                          
 *               SUCCESS                                                              
 ********************************************************************************/
INT32S adv_cnt_event_read(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_CounterEventRead *lp_counter_evt_read;
	INT16U counter;
	INT16U *overflow;
	INT16U tmp;
/* 	INT32U count; */
/* 	INT16U i; */
	


	lp_counter_evt_read = (PT_CounterEventRead *) kmalloc(sizeof(PT_CounterEventRead), GFP_KERNEL);
	if (lp_counter_evt_read == NULL) {
		return -EFAULT;
	}
	
	if (copy_from_user(lp_counter_evt_read, arg, sizeof(PT_CounterEventRead))) {
		return -EFAULT;
	}
	
/* 	counter = lp_counter_evt_read->counter; */
/* 	if (counter > 7) { */
/* 		return -EINVAL; */
/* 	} */

/* 	overflow = lp_counter_evt_read->overflow; */
/* 	if (overflow == NULL) { */
/* 		return -EINVAL; */
/* 	} */
	
/* 	command_out(privdata, (INT32U) counter, CCO_READ); */
/* 	for (i = 0; i < 10000; i++) { */
/* 		if (advInp(privdata, 0x1e) == CCO_READ) { */
/* 			break; */
/* 		} */
/* 	} */
/* 	count = (INT32U) EmboRead(privdata, IOPC_IMB0); */

/* 	if (copy_to_user(lp_counter_evt_read->count, &count, sizeof(INT32U))) { */
/* 		return -EFAULT; */
/* 	} */

/* 	return 0; */
	
	counter = lp_counter_evt_read->counter;
	overflow = lp_counter_evt_read->overflow;

	if (privdata->cnt_overflow_data) {
		privdata->overflow[counter] += 1;
		privdata->cnt_overflow_data = 0;
	}
	
	if (privdata->overflow[counter] > 65535) {
		tmp = 1;
		if (copy_to_user(overflow, &tmp, sizeof(INT16U))) {
			return -EFAULT;
		}
	}
	
	return 0;	
}	
 

/****************************************************************************
 * Description:  configure the specified counter for pulse output and start 
 *               the counter                                                
 *                                                                          
 * Input:        device -Points to the device object
 * 
 *               data[1] -period value read from user                       
 *                                                                          
 * Ouput:        errno                                                      
 *               SUCCESS                                                          
 ****************************************************************************/
/* static INT32S adv_cnt_pwm_enable(adv_device *device, INT16U port) */
/* { */
/* 	private_data *privdata = (private_data*) (device->private_data); */
	

/* 	if (port > 0x03) {	/\* check bit range *\/ */
/* 		return InvalidPort; */
/* 	} */

/* 	command_out(privdata, 0, C_PSEUDO); */
/* 	command_out(privdata, CP_EN_OUTPUT, CREAD_INT_REG); */
	
/* 	command_out(privdata, (INT32U) port, CP_EN_OUTPUT); */
 	
/* 	return SUCCESS; */
/* } */

/****************************************************************************
 * Description:  configure the specified counter for pulse output and start 
 *               the counter                                                
 *                                                                          
 * Input:        device -Points to the device object
 * 
 *               data[1] -period value read from user                       
 *                                                                          
 * Ouput:        errno                                                      
 *               SUCCESS                                                          
 ****************************************************************************/
/* static INT32S adv_cnt_pwm_setting(adv_device *device, INT32U *data) */
/* { */
/* 	private_data *privdata = (private_data*) (device->private_data); */
/* 	PT_CounterPWMSetting *lp_counter_pwm_setting; */
/* 	INT32U out_cnt; */
/* 	float period; */
/* 	float hi_period; */
	
	
/* 	lp_counter_pwm_setting = (PT_CounterPWMSetting *) data; */
	
/* 	period = lp_counter_pwm_setting->Period; */
/* 	hi_period = lp_counter_pwm_setting->HiPeriod; */
/* 	out_cnt = lp_counter_pwm_setting->OutCount; */
	

/* 	if (lp_counter_pwm_setting->Port > 1) { */
/* 		return InvalidPort; */
/* 	} */

/* 	if ((period > 131070) || (hi_period > 65535)) { */
/* 		return InvalidInputRange; */
/* 	} */

/* 	if (!(lp_counter_pwm_setting->Port)) { /\* port 0 *\/ */
/* 		command_out(privdata, 0, C_PSEUDO); */
/* 		command_out(privdata, (INT32U) hi_period, CP1_HI); */
/* 		command_out(privdata, (INT32U) (period - hi_period), CP1_LO); */
/* 		command_out(privdata, (INT32U) out_cnt, CP1_COUNT); */
/* 	} else {		/\* port 1 *\/ */
/* 		command_out(privdata, 0, C_PSEUDO); */
/* 		command_out(privdata, (INT32U) hi_period, CP2_HI); */
/* 		command_out(privdata, (INT32U) (period - hi_period), CP2_LO); */
/* 		command_out(privdata, (INT32U) out_cnt, CP2_COUNT); */
/* 	} */
	
/* 	return SUCCESS; */
/* } */


/*************************************************************
 * Description:  turn off the specified counter operation
 *                                                                 
 * Input:        device -Points to the device object               
 *               structs -counter struct                           
 *                                                                 
 * Ouput:        errno                                             
 *               SUCCESS                                                 
 *************************************************************/
/* static INT32S cnt_reset(adv_device *device, cnt_struct *structs, INT32U *data) */
/* {	 */
/* 	private_data *privdata = (private_data *) (device->private_data); */
/* 	INT16U counter = 0; */
/* 	INT16U tmp; */


/* 	counter = CR_CHAN(structs->chanspec); */
/* 	if (counter > 7) */
/* 		return EINVAL; */

/* 	command_out(privdata, 0, C_PSEUDO); */
/* 	command_out(privdata, CP_EN_OUTPUT, CREAD_INT_REG); */
     
/* 	tmp = EmboRead(privdata, IOPC_IMB0); */
/* 	tmp &= ~counter; */
/* 	command_out(privdata, tmp, CP_EN_OUTPUT); */
     
/* 	return SUCCESS; */
/* } */


/*******************************************************************
 * Description:  dispatch function                                 
 *                                                                 
 * Input:        device -Points to the device object               
 *               arg -argument from user                           
 *                                                                 
 * Ouput:        errno                                             
 *               SUCCESS                                                 
 *******************************************************************/
/* INT32S adv_cnt_ioctl(adv_device *device, void *arg) */
/* { */
/* 	cnt_struct structs; */
/* 	INT32U *data = NULL; */
/* 	INT32S ret = 0;	 */

	
/* 	if (copy_from_user(&structs, arg, sizeof(adv_struct))) { */
/* 		return MemoryCopyFailed; */
/* 	} */
     
/* 	if (structs.n != 0) { */
/* 		data = kmalloc(structs.n, GFP_KERNEL); */
/* 		if (!data) { */
/* 			return MemoryAllocateFailed; */
/* 		} */
	  
/* 		if(copy_from_user(data, structs.buffer, structs.n)) { */
/* 			return MemoryCopyFailed; */
/* 		} */
/* 	} */
     
/* 	switch (structs.type) {  */
/* 	case EVENT_READ: */
/* 		ret = adv_cnt_event_read(device, (INT32U *) data); */
/* 		break;	   */
/* 	case CNT_PWM_ENABLE: */
/* 		ret = adv_cnt_pwm_enable(device, (INT16U) (structs.portstart)); */
/* 		break; */
/* 	case CNT_PWM_SETTING: */
/* 		ret = adv_cnt_pwm_setting(device, (INT32U *) data); */
/* 		break; */
/* 	case CNT_RESET: */
/* 		ret = cnt_reset(device, &structs, NULL); */
/* 		break; */
/* 	default: */
/* 		printk("Counter type Error!\n"); */
/* 		break; */
/* 	} */

/* 	if (!ret) { */
/* 		if ((structs.type == EVENT_READ) || (structs.type == FREQ_READ)) { */
/* 			if (copy_to_user(structs.buffer, data, structs.n)) { */
/* 				return MemoryCopyFailed; */
/* 			} */
/* 		} */
/* 	} */

/* 	if(data) */
/* 		kfree(data); */

/* 	return ret; */
/* } */
