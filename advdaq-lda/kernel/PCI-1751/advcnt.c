/****************************************************************************
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 
	
			Advantech Co., Ltd.

		Advantech PCI-1751 Device driver for Linux

	File Name: 
		advcnt.c
	Abstract:
		This file contains routines for general driver core.

	Version history
	01/12/2006			Create by Zhiyong.Xie

************************************************************************/
#define __NO_VERSION__
#include "PCI1751.h"
//#define PCI1751_DEBUG
//============================================================================
// counter_reset ()
// Function:	this function provide a method to reset a counter
// Argument: 
//		device 		the operated device
//		structs:	the insn structure which to be performed
//		data:		the data read from the counter
// Return value: 
//		type:		static int
//		description:	success or fail
// Global variable:
//
// data description:
// 	data[0]:	function type	= CNT_RESET
//=============================================================================

static INT32S adv_cnt_reset(adv_device *device,adv_struct *structs,unsigned int *data)
{	
	unsigned int counter;
	// unsigned int ofs,ofs1;
	private_data *dev = (private_data*)(device->private_data);
	//ofs=ofs1=0;

	counter = CR_CHAN(structs->chanspec);
	if (counter > 2) 
		return InvalidChannel;
	switch(counter)
	{
	case 0:
		advOutp(dev, 0x1b, (0 << 6) + 0x38);
		advOutp(dev, 0x18, 0x00);
		advOutp(dev, 0x18, 0x00);
		if (dev->IsCascadeMode == 1) {
			advOutp(dev, 0x1b, (1 << 6) + 0x38);
			advOutp(dev, 0x19, 0x00);
			advOutp(dev, 0x19, 0x00);
		}
		break;
	case 1:
	   	advOutp(dev, 0x1b, (1 << 6) + 0x38);
		advOutp(dev, 0x19, 0x00);
		advOutp(dev, 0x19, 0x00);
		break;
	case 2:
		advOutp(dev, 0x1b, (2 << 6) + 0x38);
		advOutp(dev, 0x1a, 0x00);
		advOutp(dev, 0x1a, 0x00);
		break;
	}

	return SUCCESS;
}


static INT32S adv_tmrcnt_set(adv_device *device, adv_struct *structs, INT8U *data)
{
     private_data *dev = (private_data*)(device->private_data);
     PT_TimerCountSetting *settingdata = (PT_TimerCountSetting *)data;
     INT16U counter = settingdata->counter;
     INT32U Count =settingdata->Count;

     if(counter > 2){
	     return -EFAULT;
     }
     
     
     if(Count < 0x02){
	     return -EFAULT;
     }
     
     switch(counter)
     {
     case 0:
	  if(dev->IsCascadeMode)
	  {
	       if(Count < 0x000020002)
		       return -EFAULT;
	       
	       dev->cnt_init_data[0] = Count&0xffff;
	       
	       dev->cnt_init_data[1] = (Count>>16)&0xffff;
	       
	  } else {
	       dev->cnt_init_data[counter] = Count&0xffff;
	  }
	 	  
	  break;
	 	      
     case 1:
	  dev->cnt_init_data[counter] = Count&0xffff;
	  break;
	      
     case 2:
    	  dev->cnt_init_data[counter] = Count & 0xffff;
	 	  
	  break;
     }
    
     return 0;
}

/* static INT32S adv_freq_start(adv_device *device, adv_struct *structs, unsigned int *data) */
/* { */

/* #ifdef PCI1751_DEBUG */
/*      KdPrint(" DEBUG:BGN:adv_freq_start()\n"); */
/* #endif */

/*      private_data *dev = (private_data *)(device->private_data); */
/*      PT_CounterFreqStart *pdata = (PT_CounterFreqStart *)data; */
/*      USHORT counter = pdata->counter; */
/*      /\* avoid warning *\/ */
/*      USHORT GateMode; */
/*      GateMode= pdata->GateMode; */
     
/*      USHORT low_byte; */
/*      USHORT high_byte; */
/*      USHORT current_count; */

/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_freq_start() counter = %d\n", counter); */
/* #endif */
/*      GateMode = GateMode; */
     
/*      if(counter > 2){ */
/* 	  return InvalidChannel; */
/*      } */
     
/*      advOutp(dev, 0x1b, (counter<<6|0x30)); */
/*      advOutp(dev, (USHORT)(24+counter), 0xff); */
/*      advOutp(dev, (USHORT)(24+counter), 0xff); */
     
     
/*      do_gettimeofday(&(dev->freq_pre_times[counter])); */
     
/*      advOutp(dev, 27, (counter<<6|0x0)); */
/*      low_byte = advInp(dev, (USHORT)(24 + counter)); */
/*      high_byte = advInp(dev, (USHORT)(24 + counter)); */

/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_freq_start() low_byte = %x, high_byte = %x\n", low_byte, high_byte); */
/* #endif */
     
/*      current_count = (low_byte & 0xff) | ((high_byte<<8) & 0xff00); */
/*      dev->cnt_pre_counts[counter] = current_count; */

/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_freq_start() start count = %x\n", dev->cnt_pre_counts[counter]); */
/* #endif */
     
/*      return SUCCESS; */
     
/* } */
/* static int adv_freq_read(adv_device *device, adv_struct *structs, unsigned int *data) */
/* { */
/* #ifdef PCI1751_DEBUG */
/*      KdPrint(" DEBUG:BGN:adv_freq_read()\n"); */
/* #endif */

/*      private_data *dev = (private_data *)(device->private_data); */
/*      unsigned long *pdata = (unsigned long *)data; */
/*      unsigned long  counter = pdata[0]; */
    
/*      /\* data[0] : IN : counter number: 0, 1, 2 */
/*       * data[1] : OUT: how many time used in test freq: us */
/*       * data[2] : OUT: how many events happed during testing freq */
/*       * *\/ */

/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_freq_read() counter = 0X%x\n", counter); */
/* #endif */

/*      struct timeval current_time; */
     
/*      USHORT low_byte; */
/*      USHORT high_byte; */
/*      USHORT current_count; */
     
/*      if(counter > 2){ */
/* 	  return InvalidChannel; */
/*      } */

/*      /\* get count and time *\/ */
    
/*      do_gettimeofday(&(current_time)); */
/*      advOutp(dev, 27, (counter<<6 | 0x00)); */
/*      low_byte = advInp(dev, 24 + counter); */
/*      high_byte = advInp(dev, 24 + counter); */

          
/*      current_count = (low_byte & 0xff) | ((high_byte << 8) & 0xff00); */
     
     
/*      struct timeval pre_time; */
/*      pre_time = dev->freq_pre_times[counter]; */
/*      /\* compute count and time , save  *\/ */
/*      if(dev->cnt_pre_counts[counter] < current_count) */
/*      { */
/* 	  pdata[2] = dev->cnt_pre_counts[counter] + 65535L -current_count; */
/*      } */
/*      else{ */
/* 	  pdata[2] = dev->cnt_pre_counts[counter] - current_count; */
/*      } */
     
/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_freq_read() precount = 0x%x, nowcount = 0x%x\n", dev->cnt_pre_counts[counter], current_count); */
/* #endif */
     
/*      pdata[1] = ((current_time.tv_sec - pre_time.tv_sec)*1000000 + (current_time.tv_usec - pre_time.tv_usec)); */
/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_freq_read() counts = %d time = %d\n", pdata[2], pdata[1]); */
/* #endif */

/*      dev->cnt_pre_counts[counter] = current_count; */
/*      dev->freq_pre_times[counter] = current_time; */
          
/*      return SUCCESS; */
     
/* } */


/* static int adv_event_start(adv_device *device, adv_struct *structs, unsigned int *data) */
/* { */
/*      private_data *dev = (private_data *)(device->private_data); */
/*      PT_CounterFreqStart *pdata = (PT_CounterFreqStart *)data; */
/*      USHORT counter = pdata->counter; */
/*      USHORT GateMode = pdata->GateMode; */
     
/*      USHORT low_byte; */
/*      USHORT high_byte; */
/*      USHORT current_count; */

/*      GateMode = GateMode; */
     
/*      if(counter > 2){ */
/* 	  return InvalidChannel; */
/*      } */
     
/*      dev->evn_first_counts[counter]= 1; */
/*      dev->evn_overflow[counter] = 0; */
     
/*      advOutp(dev, 27, (counter<<6|0x30)); */
/*      advOutp(dev, (USHORT)(24+counter), 0xff); */
/*      advOutp(dev, (USHORT)(24+counter), 0xff); */
        
/*      advOutp(dev, 27, (counter<<6|0x0)); */
/*      low_byte = advInp(dev, (USHORT)(24 + counter)); */
/*      high_byte = advInp(dev, (USHORT)(24 + counter)); */

     
/*      current_count = (low_byte & 0xff) | ((high_byte<<8) & 0xff00); */
/*      dev->evn_pre_counts[counter] = current_count; */
/*      /\* try *\/ */
/*      dev->evn_err_counts[counter] = 0; */
     
     
/*      return SUCCESS; */
/* } */

/* static int adv_event_read(adv_device *device, adv_struct *structs, unsigned int *data) */
/* { */
/*      private_data *dev = (private_data *)(device->private_data); */
/*      unsigned long *pdata = (unsigned long *)data; */
/*      unsigned long  counter = pdata[0]; */
    
/*      /\* data[0] : IN : counter number: 0, 1, 2 */
/*       * data[1] : OUT: overflow */
/*       * data[2] : OUT: how many events happed during testing freq */
/*       * *\/ */

/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_event_read() counter = 0X%x\n", counter); */
/* #endif */

/*      USHORT low_byte; */
/*      USHORT high_byte; */
/*      USHORT current_count; */
/*      unsigned long c_data; */
     
/*      if(counter > 2){ */
/* 	  return InvalidChannel; */
/*      } */

/*      /\* get count and time *\/ */
/* //     outb(0x00|(counter<<6), dev->iobase + 27); */
/*      advOutp(dev, 27, (counter<<6 | 0x00)); */
/*      low_byte = advInp(dev, 24 + counter); */
/*      high_byte = advInp(dev, 24 + counter); */
               
/*      current_count = (low_byte & 0xff) | ((high_byte << 8) & 0xff00); */
          
/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_event_read() current count = %x\n",  current_count); */
/* #endif */
    
/*      if(dev->evn_first_counts[counter]) */
/*      { */
/* 	  if(dev->evn_pre_counts[counter] == current_count) */
/* 	  { */
/* 	       pdata[2] = 0; */
/* 	       return SUCCESS; */
/* 	  } */
/* 	  else if( (dev->evn_pre_counts[counter] > current_count) &&(dev->evn_pre_counts[counter] != 65535)){ */
/*             /\* this for low fraquance, because 8254 need 2 clocks to start *\/ */
/* 	       dev->evn_err_counts[counter] = dev->evn_pre_counts[counter] - current_count; */
/* 	       pdata[2] = dev->evn_err_counts[counter]; */
/* 	       return SUCCESS; */
/* 	  } */
/* 	  else */
/* 	  { */
/* 	       dev->evn_first_counts[counter] = 0; */
/* 	       dev->evn_pre_counts[counter] = 65535; */
/* 	  } */
/*      } */
     
/*      c_data = 65535L - (unsigned long)current_count; */
/*      if((dev->evn_pre_counts[counter] < current_count) && (0 == dev->evn_first_counts[counter])) */
/*      { */
/* 	  dev->evn_overflow[counter] += 1L; */
/*      } */
/*      unsigned long carry; */
/*      carry = dev->evn_overflow[counter] << 16; */
/*      c_data += carry; */
       
/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_event_read() carry = %x c_data = %u\n", carry, c_data); */
/* #endif */
     
/*      pdata[1] = 0; */
/*      if(dev->evn_overflow[counter] > 65535L) */
/*      { */
/* 	  pdata[1] = 1; */
/*      } */
/*      pdata[2] = c_data + dev->evn_err_counts[counter]; */
/*      dev->evn_pre_counts[counter] = current_count; */
     
       
/* #ifdef PCI1751_DEBUG */
/*      printk(" adv_event_read() overflow = %d counts = %d\n", pdata[1], pdata[2]); */
/* #endif */

/*     return SUCCESS; */
     
/* } */

/* int adv_pulse_start(adv_device *device, adv_struct *structs, unsigned int *data) */
/* { */
/* #ifdef PCI1751_DEBUG */
/*      KdPrint(" DEBUG:BGN:adv_event_start()\n"); */
/* #endif */
/*      private_data *dev = (private_data *)(device->private_data); */
/*      PT_CounterPulseStart *pdata = (PT_CounterPulseStart *)data; */
/*      USHORT counter = pdata->counter; */
/*      USHORT GateMode = pdata->GateMode; */
/*      //    USHORT IsCascadeMode = pdata->IsCascadeMode; */
/*      USHORT IsCascadeMode = dev->IsCascadeMode; */
     
/*      float period_sqrt = pdata->period; */
/*      double tmpValue, tmp_divisor; */
/*      float period = period_sqrt * period_sqrt; */
/*      USHORT divisor; */
/*      USHORT low_byte, high_byte; */
     
/*      GateMode = GateMode; */
     
/*      if(counter > 2)  */
/*      { */
/* 	  return InvalidChannel; */
/*      } */
     
/*      if(period > 428) */
/*      { */
/* #ifdef PCI1751_DEBUG */
/* 	  printk("pulse period is large than 428s\n"); */
/* #endif */
/* 	  return InvalidPacerRate; */
/*      } */
     
/*      tmpValue = 10000000*(double)period; */
/*      if((IsCascadeMode == 1) && (counter == 1)) */
/*      { */
/* 	  tmp_divisor = 3.16227766 * 1000 * period_sqrt; */
/* 	  /\* sqrt(10000000*period */
/*            * = */
/* 	   * sqrt(10) * 1000 * period_sqrt */
/* 	   * *\/ */
/* 	  if((tmp_divisor - (unsigned int)tmp_divisor) > 0.5) */
/* 	       divisor = (unsigned int)tmp_divisor + 1; */
/* 	  else */
/* 	       divisor = (unsigned int)tmp_divisor ; */
	  
/* 	  low_byte = divisor & 0x00ff; */
/* 	  high_byte = (divisor>>8) & 0x00ff; */
	  
/* 	  advOutp(dev, 27, ((0<<6)|0x36)); */
/* 	  advOutp(dev, 24, low_byte); */
/* 	  advOutp(dev, 24, high_byte); */
	  
/* 	  advOutp(dev, 27, ((1<<6)|0x36)); */
/* 	  advOutp(dev, 25, low_byte); */
/* 	  advOutp(dev, 25, high_byte); */
	  
/*      } */
/*      else{ */
/* 	  if(tmpValue > 65535L) */
/* 	       return InvalidCountNumber; */
/* 	  if((tmpValue - (unsigned int)tmpValue) > 0.5) */
/* 	       divisor = (unsigned int)tmpValue + 1; */
/* 	  else */
/* 	       divisor = (unsigned int)tmpValue; */
		  
/* 	  low_byte = divisor & 0x00ff; */
/* 	  high_byte = (divisor>>8) & 0x00ff; */
	  
/* 	  advOutp(dev, 27, (counter<<6)|0x36); */
/* 	  advOutp(dev, 24+counter, low_byte); */
/* 	  advOutp(dev, 24+counter, high_byte); */
	  
/*      } */
          
/*      return SUCCESS; */
/* } */

INT32S adv_cnt_ioctl(adv_device *device, void *arg)
{
	INT32S ret=0;	
	adv_struct structs;
	INT8U *data = NULL;
	
	if(copy_from_user(&structs, arg, sizeof(adv_struct))){
		ret = -EFAULT;
		goto error;
	}

	if(structs.n != 0){
		data=kmalloc(structs.n*sizeof(INT8U), GFP_KERNEL);
		if(!data){
			ret = -ENOMEM;
			goto error;
		}
		if(copy_from_user(data, structs.buffer, structs.n*sizeof(INT8U))){
			ret = -EFAULT;
			goto error;
		}
	}
     
	switch(structs.type)
	{ 
	case CNT_RESET:
		ret = adv_cnt_reset(device, &structs, data);
		break;
	case TMRCNT_SET:
		ret = adv_tmrcnt_set(device, &structs, data);
		break;
/* 	case FREQ_START: */
/* 		ret = adv_freq_start(device, &structs, data); */
/* 		break; */
/* 	case FREQ_READ: */
/* 		ret = adv_freq_read(device, &structs, data); */
/* 		if(ret) */
/* 			break; */
/* 		ret = copy_to_user(structs.buffer, data, structs.n*sizeof(char)); */
/* 		break; */
/* 	case EVENT_START: */
/* 		ret = adv_event_start(device, &structs, data); */
/* 		break; */
/* 	case EVENT_READ: */
/* 		ret = adv_event_read(device, &structs, data); */
/* 		if(ret) */
/* 			break; */
/* 		ret = copy_to_user(structs.buffer, data, structs.n*sizeof(char)); */
/* 		break; */
/* 	case PULSE_START: */
/* 		ret = adv_pulse_start(device, &structs, data); */
/* 		break; */
	  
	default:
		ret = -EFAULT;
		break;
	}

 error:
	if(data) 
		kfree(data);
	return ret;
}


