/*****************************************************************
 *		
 *	Copyright 2006 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCI-1716 Device driver for Linux
 *
 * File Name: 
 *	advcnt.c
 * Abstract:
 *	This file contains routines for PCI1716 counter.
 *
 * Version history
 *      09/26/2006			Create by zhenyu.zhang
 *****************************************************************/
#define __NO_VERSION__
#include <PCI1716.h>

/**
 * adv_cnt_event_start - configure the specified counter for an event-counting 
 *                       operation and start the counter
 *
 * @device: Points to the device object                   
 * @structs: copy of argument from user
 * @data: store the data read from I/O port
 *
 * 0x1e   -control register                              
 */
static INT32S adv_cnt_event_start(adv_device *device, adv_struct *structs, void *data)
{
	LPT_CounterEventStart counter_event_start = (LPT_CounterEventStart) data;     
	private_data *privdata = (private_data *) (device->private_data);
	INT16U counter = 0;
	INT16U low_byte, high_byte;



	counter = counter_event_start->counter;
	if (counter > 0) {
		return InvalidInputParam;
	}
	
	privdata->overflow[counter] = 0;
	privdata->first_count[counter] = 1;

	/* configure counter for external clocking */
	advOutpw(privdata, 0x06, (advInpw(privdata, 0x06) | 0x0040) & 0x007f);
     
	advOutp(privdata, 0x1e, (counter << 6 | 0x30));
	advOutp(privdata, counter * 2 + 0x18, 0xff);
	advOutp(privdata, counter * 2 + 0x18, 0xff);

	advOutp(privdata, 0x1e, (counter << 6 | 0x00));
	low_byte = advInp(privdata, counter + 0x18);
	high_byte = advInp(privdata, counter + 0x18);
	privdata->pre_count[counter] = ((high_byte << 8) & 0xff00) | (low_byte & 0x00ff);

	return 0;	
}	

/**
 * adv_cnt_event_read - read the current counter total without disturbing the counting 
 *                      process and return the count value and overflow conditions     
 *
 * @device: Points to the device object                   
 * @structs: copy of argument from user
 * @data: store the data read from I/O port
 *
 * data[0] -counter number
 * data[1] -overflow flag                                         
 * data[2] -count value return to user                            
 */
static INT32S adv_cnt_event_read(adv_device *device, adv_struct *structs, INT32U *data)
{
	private_data *privdata = (private_data*) (device->private_data);
	INT16U low_byte, high_byte;
	INT32U current_count = 0, previous_count = 0;
	INT32U c_data = 0, carry = 0;
	INT16U counter = 0;


		
	counter = data[0];
	if (counter > 0) {
		return InvalidInputParam;
	}
	
	
	advOutp(privdata, 0x1e, (counter << 6 | 0x00));
	low_byte = advInp(privdata, counter * 2 + 0x18);
	high_byte = advInp(privdata, counter * 2 + 0x18);
	current_count = ((high_byte << 8) & 0xff00) | (low_byte & 0x00ff);

	previous_count = privdata->pre_count[counter];
     
	if (privdata->first_count[counter]) {
		if (previous_count == current_count) {
			data[2] = 0;
			return 0;
		} else if ((previous_count > current_count) &&
			   (previous_count != 65535)) {
			privdata->err_count[counter] = previous_count - current_count;
			data[2] = privdata->err_count[counter];
			return 0;
		} else {
			privdata->first_count[counter] = 0;
			privdata->pre_count[counter] = 65535;
			previous_count = privdata->pre_count[counter];
		}
	}
     
	c_data = 65535 - (INT32U) current_count; /* get down-counting numbers */

	if ((previous_count < current_count) && (0 == previous_count))
		privdata->overflow[counter] += 1; /* carry bit increase */

	carry = privdata->overflow[counter] << 16; /* shift carry bit to high 16 */
	c_data += carry;	/* convert 16-bit to 32-bit */
     
	data[1] = 0;
	if (privdata->overflow[counter] > 65535)
		data[1] = 1;
     
	data[2] = c_data + privdata->err_count[counter];

	privdata->pre_count[counter] = current_count;


	return 0;	
}	

/**
 * adv_cnt_freq_start - configure the specified counter for frequency measurement 
 *                      and start the counter                                     
 *                                                                         
 * @device: Points to the device object                   
 * @structs: copy of argument from user
 * @data: store the data read from I/O port 
 */
static INT32S adv_cnt_freq_start(adv_device *device, adv_struct *structs, INT32U *data)
{
	LPT_CounterFreqStart counter_freq_start = (LPT_CounterFreqStart) data;
	private_data *privdata = (private_data*)(device->private_data);
     	INT16U counter = 0;
	INT16U ctr_byte = 0;
	INT16U low_byte, high_byte;
	INT32U current_count = 0;
	struct timeval current_tick;


     

	counter = counter_freq_start->counter;
	if (counter > 0) {
		return InvalidInputParam;
	}
	
	privdata->first_count[counter] = 1;
	
	/* configure counter for external clocking */
	advOutpw(privdata, 0x06, (advInpw(privdata, 0x06) | 0x0040) & 0x007f);


	ctr_byte = counter << 6;
	ctr_byte |= 0x30;
	advOutp(privdata, 0x1e, ctr_byte);

	advOutp(privdata, 0x18, 0xff);
	advOutp(privdata, 0x18, 0xff);

	do_gettimeofday(&current_tick);

	ctr_byte = counter << 6;
	advOutp(privdata, 0x1e, ctr_byte);
     
	advOutp(privdata, 0x06, advInp(privdata, 0x06) | 0x00);
	low_byte = advInp(privdata, 0x18);
	high_byte = advInp(privdata, 0x18);
	current_count = ((high_byte << 8) & 0xff00) | (low_byte & 0x00ff);     

	privdata->freq_precount[counter] = current_count;
	privdata->freq_pretick[counter] = current_tick;


	return 0;	
}	

/**
 * adv_cnt_freq_read - read the frequency measurement                    
 *                                                                 
 * @device: Points to the device object               
 * @structs: counter struct for return                
 *                                                                 
 * ticks = tick numbers (current_tick - pre_tick)    
 * HZ = system clock frequency (tick numbers/second) 
 * ticks / HZ = seconds                              
 *
 * data[1] -ticksX value return to user               
 * data[2] -count value return to user               
 */
static INT32S adv_cnt_freq_read(adv_device *device, adv_struct *structs, INT32U *data)
{
	private_data *privdata = (private_data*)(device->private_data);      
	INT16U ctr_byte = 0;
	INT16U counter = 0;
	INT16U low_byte, high_byte;
	INT32U c_data = 0;
	INT32U ticks = 0;
	INT32U current_count = 0, previous_count = 0;
	struct timeval current_tick, previous_tick;



	counter = data[0];
	if (counter > 0) {
		return InvalidInputParam;
	}	

	ctr_byte = counter << 6;
	advOutp(privdata, 0x1e, ctr_byte);

	low_byte = advInp(privdata, 0x18);
	high_byte = advInp(privdata, 0x18);
	current_count = ((high_byte << 8) & 0xff00) | (low_byte & 0x00ff);     

	do_gettimeofday(&current_tick);
	previous_tick = privdata->freq_pretick[counter];
	ticks = (current_tick.tv_sec - previous_tick.tv_sec) * 1000000
		+ (current_tick.tv_usec - previous_tick.tv_usec);


	previous_count = privdata->freq_precount[counter];
	if (current_count == previous_count) {
		data[2] = 0;     
		return 0;
	} else {
		privdata->freq_precount[counter] = 65535;
	}
	
	if (current_count < previous_count) {
		c_data = previous_count - current_count;
	} else {
		c_data = 65535L + previous_count - current_count;
	}
	
	data[1] = ticks;
	data[2] = c_data;     
	privdata->freq_precount[counter] = current_count;
	privdata->freq_pretick[counter] = current_tick;
	privdata->first_count[counter] = 0;


	return 0;	
}	

/**
 * adv_cnt_pulse_start - configure the specified counter for pulse output and start 
 *                       the counter                                                
 *
 * @device: Points to the device object                        
 * @structs: copy of argument from user
 *
 * data[1]: period value read from user
 */
static INT32S adv_cnt_pulse_start(adv_device *device, cnt_struct *structs, INT32U *data)
{
	LPT_CounterPulseStart counter_pulse_start = (LPT_CounterPulseStart) data;
	private_data *privdata = (private_data*) (device->private_data);
	INT16U ctr_byte = 0, low_byte, high_byte;
	INT16U counter = 0;
	INT32U divisor = 0;

	double period;
	double tmp = 0;


	counter = counter_pulse_start->counter;
	if (counter > 0) {
		return InvalidInputParam;
	}

	/* configure counter_0 for internal clocking, 1MHz */
	advOutp(privdata, 0x06, advInp(privdata, 0x06) & 0x003f);


	period = counter_pulse_start->period * counter_pulse_start->period;
	tmp = INTERNAL_CLOCK_RATE * (double) period; /* period use seconds */
	if ((tmp - (int) tmp) >= 0.5) {
		divisor = (int) tmp + 1;
	} else {
		divisor = (int) tmp;
	}
	
	low_byte = (INT16U) (divisor & 0x000000ff);
	high_byte = (INT16U) ((divisor & 0x0000ff00) >> 8);

	ctr_byte = counter << 6;
	ctr_byte |= 0x36;
	advOutp(privdata, 0x1e, ctr_byte);
     
	advOutp(privdata, 0x18, low_byte);
	advOutp(privdata, 0x18, high_byte);

     
	return 0;
}


/**
 * cnt_reset - this function provide a method to reset a counter 
 *                                                                 
 * @device: Points to the device object               
 * @structs: copy of argument from user
 * @data: NULL
 */
static INT32S cnt_reset(adv_device *device, cnt_struct *structs, INT32U *data)
{	
	private_data *privdata = (private_data *) (device->private_data);
	INT16U ctr_byte = 0;
	INT16U counter = 0;



	counter = CR_CHAN(structs->chanspec);
	if (counter > 2) {
		return InvalidInputParam;
	}

	ctr_byte = counter << 6;
	ctr_byte |= 0x39;
	advOutp(privdata, 0x1e, ctr_byte);
	advOutp(privdata, (counter * 2 + 0x18), 0); /* write the low byte */
	advOutp(privdata, (counter * 2 + 0x18), 0); /* write the high byte */


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
	INT32S ret = 0;	
	void *data = NULL;

	
	if (copy_from_user(&structs, arg, sizeof(adv_struct))) {
		return MemoryCopyFailed;
	}
     
	if (structs.n != 0) {
		data = kmalloc(structs.n, GFP_KERNEL);
		if (!data) {
			return MemoryAllocateFailed;
		}
	  
		if(copy_from_user(data, structs.buffer, structs.n)) {
			kfree(data);
			return MemoryCopyFailed;
		}
	}
     
	switch (structs.type) { 
	case EVENT_START:
		ret = adv_cnt_event_start(device, &structs, data);
		break;
	case EVENT_READ:
		ret = adv_cnt_event_read(device, &structs, (INT32U *) data);
		break;	  
	case PULSE_START:
		ret = adv_cnt_pulse_start(device, &structs, data);
		break;
	case FREQ_START:
		ret = adv_cnt_freq_start(device, &structs, data);
		break;
	case FREQ_READ:
		ret = adv_cnt_freq_read(device, &structs, data);
		break;
	case CNT_RESET:
		ret = cnt_reset(device, &structs, data);
		break;
	default:
		ret = InvalidInputParam;
		break;
	}

	if (ret) {
		kfree(data);
		return ret;
	}
     

	if ((structs.type == EVENT_READ) || (structs.type == FREQ_READ)) {
		if (copy_to_user(structs.buffer, data, structs.n)) {
			kfree(data);
			return MemoryCopyFailed;
		}
	}

	kfree(data);

	return 0;
}
