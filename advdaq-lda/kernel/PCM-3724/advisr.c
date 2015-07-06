/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCM-3724 Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	adisr.c                                             
 * Abstract:                                                   
 * 	This file contains routines for interrupt.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	07/20/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "pcm3724.h"


void evt_tasklet_fn(PTR_T arg)
{
	private_data *privdata = (private_data *) (((adv_device *) arg)->private_data);
	INT16U status;
	INT16U mask;
	INT16U i;	
	

	status = privdata->int_stat;
/* 	printk("tasklet status: 0x%x\n", status); */
	
	spin_lock(&privdata->spinlock);
	for (i = 0; i < 9; i++) {
		mask = 0x0001 << i;
		if (status & mask) {
			/* printk("mask: 0x%x, i: 0x%x\n", mask, i); */
			adv_process_info_set_event_all(&privdata->ptr_process_info, i, 1);
		}
	}
	spin_unlock(&privdata->spinlock);
	advOutp(privdata, 0x44, 0x0000); /* clear interrupt */
	wake_up_interruptible(&privdata->event_wait);
}

/**
 * Description:  PCM3724 interrupt handler                    
 *
 *               step1. disable interrupt
 *               step2. call the conresponding ISR            
 *               step3. wake up process in the read queue     
 *               step4. enable interrupt                      
 *                                                            
 * Input:        irq     -interrupt request number            
 *               dev_id  -privdata data (device object)                      
 *               regs    -holds a snapshot of the processor's context before
 *                        the processor entered interrupt code (NULL)                              
 *                                                            
 * Ouput:        the return value of ISR                      
 *                                                            
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pcm3724_interrupt_handler00(INT32S irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pcm3724_interrupt_handler00(INT32S irq, void *dev_id)
#endif
{
	private_data *privdata = (private_data *) dev_id;
/* 	INT8U status; */
/* 	INT16U mask; */
/* 	INT16U i;	 */

	adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
/* 	status = advInp(privdata, 0x0a)/\*  & 0x10 *\/; */
/* 	if (!status) { */
/* 		printk("The interrupt isn't belongs to this device\n"); */
/* 		return IRQ_RETVAL(0); */
/* 	} */
/* 	/\* printk("int0!\n"); *\/ */
	
/* 	advOutp(privdata, 0x08, 0x00); /\* clear interrupt *\/ */
/* 	privdata->int_stat = status; */
	/* printk("status: 0x%x\n", status); */
	
/* 	spin_lock(&privdata->spinlock); */
/* 	for (i = 0; i < 9; i++) { */
/* 		mask = 0x0001 << i; */
/* 		if (privdata->int_stat & mask) { */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, i, 1); */
/* 		} */
/* 	} */
/* 	spin_unlock(&privdata->spinlock); */
	//tasklet_schedule(&privdata->evt_tasklet);
	wake_up_interruptible(&privdata->event_wait);

/* 	advOutp(privdata, 0x44, 0x0000); /\* clear interrupt *\/ */

	return IRQ_RETVAL(1);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pcm3724_interrupt_handler01(INT32S irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pcm3724_interrupt_handler01(INT32S irq, void *dev_id)
#endif
{
	private_data *privdata = (private_data *) dev_id;

	adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
	wake_up_interruptible(&privdata->event_wait);

	return IRQ_RETVAL(1);
}

