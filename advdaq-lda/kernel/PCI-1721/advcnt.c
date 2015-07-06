/*****************************************************************
 *		
 *	Copyright 2004 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCI-1721 Device driver for Linux
 *
 * File Name: 
 *	advcnt.c
 * Abstract:
 *	This file contains routines for PCI1721 counter.
 *
 * Version history
 *      02/07/2006			Create by LI.ANG
 *
 *****************************************************************/
#define __NO_VERSION__
#include <PCI1721.h>

/***********************************************************************
 * Description:  configure the specified counter for an event-counting 
 *               operation and start the counter                       
 *               0x1e   -control register                              
 *                                                                     
 * Input:        device -Points to the device object                   
 *                                                                     
 * Ouput:        errno                                                 
 *               SUCCESS                                                     
 ***********************************************************************/
static int adv_cnt_event_start(adv_device *device, void *data)
{

     private_data *privdata = (private_data *) (device->private_data);
     LPT_CounterEventStart counter_event_start = (LPT_CounterEventStart) data;     
     USHORT ctr_byte, low_byte, high_byte, counter;

#ifdef PCI1721_DEBUG
     printk("DEBUG:BGN:%s()..\n", __FUNCTION__);
#endif

     counter = counter_event_start->counter;

     if (counter > 0)
	  return -EINVAL;

     privdata->overflow[counter] = 0;
     privdata->first_cnt[counter] = 1;
     
     advOutpw(privdata, 0x2a, advInpw(privdata, 0x2a) | 0x03);

     ctr_byte = counter << 6;
     ctr_byte |= 0x30;
     advOutpw(privdata, 0x36, ctr_byte);

     advOutpw(privdata, 0x30, 0xff);
     advOutpw(privdata, 0x30, 0xff);

     ctr_byte = counter << 6;
     advOutpw(privdata, 0x36, ctr_byte);

     low_byte = advInpw(privdata, 0x30);
     high_byte = advInpw(privdata, 0x30);
     privdata->pre_cnt[counter] = ((high_byte << 8) & 0xff00) | (low_byte & 0x00ff);

#ifdef PCI1721_DEBUG
     printk("DEBUG:END:%s()..\n", __FUNCTION__);
#endif
     return SUCCESS;	
}	

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
static int adv_cnt_event_read(adv_device *device, ULONG *data)
{
     private_data *privdata = (private_data *) (device->private_data);
     USHORT ctr_byte, low_byte, high_byte, counter;
     ULONG current_count = 0, previous_count = 0;
     ULONG c_data = 0, carry = 0;

		
#ifdef PCI1721_DEBUG
     printk("DEBUG:BGN:%s()\n", __FUNCTION__);
#endif

     counter = data[0];
     
     if (counter > 0)
	  return -EINVAL;

     ctr_byte = counter << 6;
     advOutpw(privdata, 0x36, ctr_byte);

     low_byte = advInpw(privdata, 0x30);
     high_byte = advInpw(privdata, 0x30);
     current_count = ((high_byte << 8) & 0xff00) | (low_byte & 0x00ff);
     previous_count = privdata->pre_cnt[counter];
     
     if (privdata->first_cnt[counter]) {
	  if (previous_count == current_count) {
	       data[2] = 0;
	       return 0;
	  } else if ((previous_count > current_count) &&
		     (previous_count != 65535)) {
	       privdata->err_cnt[counter] = previous_count - current_count;
	       data[2] = privdata->err_cnt[counter];
	       return 0;
	  } else {
	       privdata->first_cnt[counter] = 0;
	       privdata->pre_cnt[counter] = 65535;
	       previous_count = privdata->pre_cnt[counter];
	  }
     }
     
     c_data = 65535 - (ULONG) current_count; /* get down-counting numbers */

     if ((previous_count < current_count) && (0 == previous_count))
	  privdata->overflow[counter] += 1; /* carry bit increase */

     carry = privdata->overflow[counter] << 16; /* shift carry bit to high 16 */
     c_data += carry; 		/* convert 16-bit to 32-bit */
     
     data[1] = 0;
     if (privdata->overflow[counter] > 65535)
	  data[1] = 1;
     
     data[2] = c_data + privdata->err_cnt[counter];

     privdata->pre_cnt[counter] = current_count;

#ifdef PCI1721_DEBUG
     printk("DEBUG:END:%s()\n", __FUNCTION__);
#endif
     return SUCCESS;	
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
static int adv_cnt_pulse_start(adv_device *device, ULONG *data)
{
     private_data *privdata = (private_data*) (device->private_data);
     LPT_CounterPulseStart counter_pulse_start = (LPT_CounterPulseStart) data;
     USHORT ctr_byte, low_byte, high_byte, counter;
     ULONG divisor = 0;
     double period;
     double tmp = 0;

#ifdef  PCI1721_DEBUG
     printk("DEBUG:BGN:%s()..\n", __FUNCTION__);
#endif


     counter = counter_pulse_start->counter;
     period = counter_pulse_start->period * counter_pulse_start->period;

     if (counter > 0)
	  return -EINVAL;

     advOutpw(privdata, 0x2a, (advInpw(privdata, 0x2a) & 0xfe) | 0x02);

     tmp = INTERNAL_CLOCK_RATE * (double) period; /* period use seconds */

     if ((tmp - (int) tmp) >= 0.5)
	  divisor = (int) tmp + 1;
     else
	  divisor = (int) tmp;

     ctr_byte = counter << 6;
     ctr_byte |= 0x36;
     advOutpw(privdata, 0x36, ctr_byte);
     

     low_byte = (USHORT) (divisor & 0x000000ff);
     high_byte = (USHORT) ((divisor & 0x0000ff00) >> 8);

     advOutpw(privdata, 0x30, low_byte);
     advOutpw(privdata, 0x30, high_byte);

#ifdef  PCI1721_DEBUG
     printk("DEBUG:END:%s()..\n", __FUNCTION__);
#endif
     return SUCCESS;
}


/*******************************************************************
 * Description:  this function provide a method to reset a counter 
 *                                                                 
 * Input:        device -Points to the device object               
 *               structs -counter struct                           
 *                                                                 
 * Ouput:        errno                                             
 *               SUCCESS                                                 
 *******************************************************************/
static int cnt_reset(adv_device *device, cnt_struct *structs, unsigned long *data)
{	
     private_data *privdata = (private_data *) (device->private_data);
     int counter = 0;

#ifdef PCI1721_DEBUG
     printk(" DEBUG:BGN:%s()\n", __FUNCTION__);
#endif

     counter = CR_CHAN(structs->chanspec);

     if (counter > 0)
	  return -EINVAL;

     advOutpw(privdata, 0x36, 0x39); 
     advOutpw(privdata, 0x30, 0); /* write the low byte */
     advOutpw(privdata, 0x30, 0); /* write the high byte */

#ifdef PCI1721_DEBUG
     printk(" DEBUG:END:%s()\n", __FUNCTION__);
#endif

     return SUCCESS;
}


/*******************************************************************
 * Description:  dispatch function                                 
 *                                                                 
 * Input:        device -Points to the device object               
 *               arg -argument from user                           
 *                                                                 
 * Ouput:        errno                                             
 *               SUCCESS                                                 
 *******************************************************************/
int adv_cnt_ioctl(adv_device *device, void *arg)
{
     cnt_struct structs;
     void *data = NULL;
     int ret = 0;	

#ifdef PCI1721_DEBUG
     printk(" DEBUG:BGN:%s()\n", __FUNCTION__);
#endif
	
     if (copy_from_user(&structs, arg, sizeof(adv_struct))) {
	  printk("error occured when copy_from_user()...\n");
	  ret = -EFAULT;
	  goto error;
     }
     
     if (structs.n != 0) {
	  data = kmalloc(structs.n, GFP_KERNEL);
	  if (!data) {
	       ret = -ENOMEM;
	       goto error;
	  }
	  
	  if(copy_from_user(data, structs.buffer, structs.n)) {
	       ret = -EFAULT;
	       goto error;
	  }
     }
     
     switch (structs.type) { 
     case EVENT_START:
	  ret = adv_cnt_event_start(device, (void *) data);
	  break;
     case EVENT_READ:
	  ret = adv_cnt_event_read(device, (ULONG *) data);
	  break;	  
     case PULSE_START:
	  ret = adv_cnt_pulse_start(device, (ULONG *) data);
	  break;
     case CNT_RESET:
	  ret = cnt_reset(device, &structs, NULL);
	  break;
     default:
	  printk("Counter type Error!\n");
	  break;
     }

     if (ret < 0)
	  goto error;

     if ((structs.type == EVENT_READ) || (structs.type == FREQ_READ)) {
	  if (copy_to_user(structs.buffer, data, structs.n)) {
	       printk("copy_to_user occured error!\n");
	       ret = -EFAULT;
	       goto error;
	  }
     }

 error:
     if(data)
	  kfree(data);

#ifdef PCI1721_DEBUG
     printk(" DEBUG:END:%s()\n", __FUNCTION__);
#endif
     return ret;
}
