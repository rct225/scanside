/****************************************************************************
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 
	
			Advantech Co., Ltd.

		Advantech PCI-1750 Device driver for Linux

	File Name: 
		advcnt.c
	Abstract:
		This file contains routines for general driver core.

	Version history
	01/12/2006			Create by Zhiyong.Xie

************************************************************************/
#define __NO_VERSION__
#include "PCI1750.h"

/************************************************************************
 * adv_cnt_reset ()
 * Function:	this function provide a method to reset a counter
                
                Step 1: Reset the Counter 2
 * Argument: 
		device 		the operated device
		structs:	the insn structure which to be performed
		data:		NULL
 * Return value: 
		type:		static int
		description:	success or fail
 * Global variable:

 * data description:
 	
***********************************************************************/

static int adv_cnt_reset(adv_device *device,adv_struct *structs,unsigned int *data)
{	
	int chan;
	private_data *dev = (private_data*)(device->private_data);
 
	chan=CR_CHAN(structs->chanspec);
	if (chan != 2) return InvalidChannel;

	outb((chan << 6) + 0x38, dev->iobase + 27);
	outb(0x00, dev->iobase + 24 + 2);
	outb(0x00, dev->iobase + 24 + 2);
     
	return 0;
}

/************************************************************************
 * adv_tmrcnt_set ()
 * Function:	this function set the initiallize data of counter
                0, 1, 2.
                
 * Argument: 
		device 		the operated device
		structs:	the insn structure which to be performed
		data:		a PT_TimerCountSetting structure 
 * Return value: 
		type:		static int
		description:	success or fail
 * Global variable:

 * data description:
 	
***********************************************************************/

static int adv_tmrcnt_set(adv_device *device, adv_struct *structs, unsigned int *data)
{
     private_data *dev = (private_data*)(device->private_data);
     PT_TimerCountSetting *settingdata = (PT_TimerCountSetting *)data;
     USHORT counter = settingdata->counter;
     ULONG Count =settingdata->Count;
    
     if(counter > 2) return -EFAULT;
    
     if(Count < 0x02)return -EFAULT;
    
     switch(counter)
     {
     case 0:
	  if (dev->IsCascadeMode) {
	       if(Count < 0x000020002)
		    return InvalidCountNumber;
	      
	       dev->cnt_init_data[0] = Count & 0xffff;
	      	      
	       dev->cnt_init_data[1] = (Count>>16)&0xffff;
	  } else {
	       dev->cnt_init_data[0] = Count&0xffff;
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

/**
 * adv_cnt_ioctl - this function disturbut the operate of counter
 * @device : the operated device
 * @structs : the insn structure which to be performed
 * @data : a long array,  data[0] : counter number data[1] : times taked data[2] : counts 
 * Return value: 
		type:		static int
		description:	success or fail
 */

int adv_cnt_ioctl(adv_device *device, void *arg)
{
	int ret=SUCCESS;	
	adv_struct structs;
	unsigned int *data = NULL;

	if(copy_from_user(&structs,arg,sizeof(adv_struct))){
		printk("error occured when copy_from_user()..1\n");
		ret= MemoryCopyFailed;
		goto error;
	}
	if(structs.n != 0){
		data=kmalloc(structs.n*sizeof(char),GFP_KERNEL);
		if(!data){
			ret = -EFAULT;
			goto error;
		}
		if(copy_from_user(data,structs.buffer,structs.n*sizeof(char))){
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
	default:
		ret = -EFAULT;
		break;
	}

error:
	if(data) 
		kfree(data);

	return ret;
}


