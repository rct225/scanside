/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1721 Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	advisr.c                                             
 * Abstract:                                                   
 * 	This file contains routines for interrupt.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	10/09/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1721.h"

void fao_tasklet_fn(PTR_T arg)
{
	private_data *privdata = (private_data *) (((adv_device *) arg)->private_data);


	/* KdPrint("halfready_flag: %d\n", privdata->halfready_flag); */
	
	if (privdata->half_ready == 1) { /* low buffer ready */
		if (privdata->low_buf_flag) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
			privdata->underrun_flag = 1;
		} else if (privdata->evt_stat[0]) { /* low buffer transfered */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
			privdata->low_buf_flag = 1;
		}
	} else if (privdata->half_ready == 2) {	/* high buffer ready */

		if (privdata->high_buf_flag) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
			privdata->underrun_flag = 1;
		} else if (privdata->evt_stat[1]) { /* high buffer transfered */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
			privdata->high_buf_flag = 1;
		}
		privdata->terminal_cnt += 1;
			
		if (!privdata->cyclic && privdata->evt_stat[2]) { /* terminate */
			/* we should stop dma at once in this line */
			advOutpDMAw(privdata, 0xa8, 0x0008);
			/* we schedule 'fao_stop_work' thread to terminate fao */
			schedule_work(&privdata->fao_stop_work);
		}
	}

	wake_up_interruptible(&privdata->event_wait);
	

	return;
}


/**
 * pci1721_interrupt_handler - PCI1721 interrupt handler                    
 *
 *                                                            
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before the processor
 *        entered interrupt code (NULL)        
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1721_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1721_interrupt_handler(int irq, void *dev_id)
#endif
{
	adv_device *device = (adv_device *) dev_id;
	private_data *privdata = (private_data *) (device->private_data);
	INT16U tmp;
     

	tmp = advInpDMAw(privdata, 0x6a) & 0x20;
	if (!tmp) {
		return IRQ_RETVAL(0);
	}
		
/* 	tmp = advInpDMA(privdata, 0xa8); */
/* 	tmp |= 0x08;	/\* clear DMA interrupt *\/ */
/* 	advOutpDMA(privdata, 0xa8, tmp); */
/* 	tmp = advInpDMA(privdata, 0xa8); */
/* 	if (tmp & 0x10) {	/\* DMA transfer done *\/ */
/* 		privdata->buf_stat = 1; */
/* 	} */
	     
/* 	printk("buf_stat: %d\n", privdata->buf_stat); */
	
	if (!privdata->buf_stat) {
		privdata->cur_index = privdata->conv_num / 2;
		privdata->half_ready = 1;

/* 		if (privdata->low_buf_flag) { */
/* 			/\* printk("low f: %d\n", privdata->low_buf_flag); *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1); */
/* 			privdata->underrun_flag = 1; */
/* 		} else if (privdata->evt_stat[0]) { /\* low buffer transfered *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1); */
/* 			privdata->low_buf_flag = 1; */
/* 		} */
	} else {
		privdata->cur_index = privdata->conv_num;
		privdata->half_ready = 2;
		
/* 		if (privdata->high_buf_flag) { */
/* 			/\* printk("high f: %d\n", privdata->high_buf_flag); *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1); */
/* 			privdata->underrun_flag = 1; */
/* 		} else if (privdata->evt_stat[1]) { /\* high buffer transfered *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
/* 			privdata->high_buf_flag = 1; */
/* 		} */
/* 		privdata->terminal_cnt += 1; */
			
/* 		if (!privdata->cyclic && privdata->evt_stat[2]) { /\* terminate *\/ */
/* 			/\* we should stop dma at once in this line *\/ */
/* 			advOutpDMAw(privdata, 0xa8, 0x0008); */
/* 			/\* we schedule 'fao_stop_work' thread to terminate fao *\/ */
/* 			schedule_work(&privdata->fao_stop_work); */
/* 		} */
	}

/* 	wake_up_interruptible(&privdata->event_wait); */

	privdata->buf_stat = !privdata->buf_stat;

	tasklet_schedule(&privdata->fao_tasklet);

	tmp = advInpDMA(privdata, 0xa8);
	tmp |= 0x08;	/* clear DMA interrupt */
	advOutpDMA(privdata, 0xa8, tmp);

	return IRQ_RETVAL(1);
}
