/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1742 Device driver for Linux              
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
 * 	01/09/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1742.h"

void fai_tasklet_fn(PTR_T arg)
{
	private_data *privdata = (private_data *) (((adv_device *) arg)->private_data);

	

/* 	KdPrint("tasklet!\n"); */
/* 	KdPrint("half_ready: %d\n", privdata->half_ready); */
/* 	KdPrint("cur_index: %d\n", privdata->cur_index); */
/* 	KdPrint("conv_num / 2: %d\n", privdata->conv_num / 2); */

	if (privdata->half_ready == 1) { /* low buffer ready */
/* 		if (privdata->low_buf_flag) { */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 4, 1); */
/* 			privdata->overrun_flag = 1; */
/* 		} else if (privdata->evt_stat[1]) { /\* low buffer ready *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
/* 			privdata->low_buf_flag = 1; */
/* 		} */
		adv_process_info_set_event_all(&privdata->ptr_process_info,
					       1, 1);

		if (privdata->halfready_flag == 2) { /* data is not taken off, so overrun */
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       4, 1);
			privdata->overrun_flag = 1;
		}
		privdata->halfready_flag = 1;

	} else if (privdata->half_ready == 2) {	/* high buffer ready */
/* 		if (privdata->high_buf_flag) { */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 4, 1); */
/* 			privdata->overrun_flag = 1; */
/* 		} else if (privdata->evt_stat[2]) { /\* high buffer ready *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1); */
/* 			privdata->high_buf_flag = 1; */
/* 		} */
/* 		privdata->terminal_cnt += 1; */
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);

		if (!privdata->cyclic) { /* terminate */
			privdata->terminated = 1;

			/* we schedule 'fai_stop_work' thread to terminate fai */
			schedule_work(&privdata->fai_stop_work);
		} else {
			/* data is not taken off, so overrun */
			if (privdata->halfready_flag == 1) {
				adv_process_info_set_event_all(&privdata->ptr_process_info,
							       4, 1);
				privdata->overrun_flag = 1;
			}
			privdata->halfready_flag = 2;
		}
	}
	

	wake_up_interruptible(&privdata->event_wait);
	

	return;
}


/**
 * pci1742_nofifo_isr - interrupt service routine for without fifo
 *                      data acquisition
 *
 * ptr: point to the private data of device object
 *
 * interrupt when an A/D conversion occurs, read the fifo data
 * into user buffer, until user buffer is half full or full,se
 * nd 'buffer change' or 'terminate' event to user.
 */
static void pci1742_nofifo_isr(private_data *ptr)
{
	private_data *privdata = ptr;
	adv_user_page *page = NULL;
	INT16U tmp;



	page = privdata->user_pages + privdata->page_index;	
	if (privdata->item >= page->length) {
		privdata->page_index++;
		privdata->item = 0;
	}
	
	privdata->page_index %= privdata->page_num;
	privdata->cur_index %= privdata->conv_num;
	
	page = privdata->user_pages + privdata->page_index;

	tmp = advInpw(privdata, 0x0000);
	memcpy((INT16U *) (page->page_addr + page->offset + privdata->item),
	       &tmp, sizeof(INT16U));
		
	privdata->item += 2;
	privdata->cur_index++;

	/* set event */
	privdata->half_ready = 0;

	if (privdata->cur_index == privdata->conv_num / 2) { /* buffer change */
		privdata->transfered += (privdata->conv_num / 2);
		privdata->half_ready = 1;

		/* adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
	}

	if (privdata->cur_index == privdata->conv_num) {
		privdata->transfered += (privdata->conv_num / 2);
		privdata->half_ready = 2;
	     
		if (!privdata->cyclic) { /* terminate */
			/* adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1); */
			advOutpDMAw(privdata, 0x68, 0x0000);
			advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x9040);
		} /* else {		/\* buffer change *\/ */
			/* adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
		     
/* 			if (privdata->overrun_flag) { /\* overrun *\/ */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       3, */
/* 							       1); */
/* 			} else { */
/* 				privdata->overrun_flag = 1; */
/* 			} */
/* 		} */
	}

	tasklet_schedule(&privdata->fai_tasklet);

/* 	if (privdata->cur_index == privdata->int_cnt) { /\* interrupt count *\/ */
/* 		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1); */
/* 	} */

/* 	wake_up_interruptible(&privdata->event_wait); */
}


/**
 * pci1742_fifo_isr - interrupt service routine for without fifo
 *                    data acquisition
 *
 * ptr: point to the private data of device object
 *
 * interrupt when fifo is half full, read the fifo data into
 * user buffer, until user buffer is half full or full, send
 * 'buffer change' or 'terminate' event to user.
 */
static void pci1742_fifo_isr(private_data *ptr)
{
	private_data *privdata = ptr;
	adv_user_page *page = NULL;
	INT16U tmp;
	INT16U i;


/* 	KdPrint("fifo isr!\n"); */
	
	/* recieve data */
	i = 0;
	do {
		page = privdata->user_pages + privdata->page_index;
		if (privdata->item >= page->length) {
			privdata->page_index++;
			privdata->item = 0;
		}
	
		privdata->page_index %= privdata->page_num;
		privdata->cur_index %= privdata->conv_num;
	
		page = privdata->user_pages + privdata->page_index;

		
		tmp = advInpw(privdata, 0x0000);
		memcpy((INT16U *) (page->page_addr + page->offset + privdata->item),
		       &tmp, sizeof(INT16U));
		
		privdata->item += 2;
		privdata->cur_index++;
		i++;
	} while ((i < privdata->half_fifo_size)
		 && (privdata->cur_index < privdata->conv_num));
	

	/* set event */
	privdata->half_ready = 0;

	if (privdata->cur_index == privdata->conv_num / 2) { /* buffer change */
		privdata->transfered += (privdata->conv_num / 2);
		privdata->half_ready = 1;

		/* adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
	}

	if (privdata->cur_index == privdata->conv_num) {
		privdata->transfered += (privdata->conv_num / 2);
		privdata->half_ready = 2;

	     
		if (!privdata->cyclic) { /* terminate */
			/* adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1); */
			advOutpDMAw(privdata, 0x68, 0x0000);
			advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x9040);
		} /* else {		/\* buffer change *\/ */
			/* adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
		     
/* 			if (privdata->overrun_flag) { /\* overrun *\/ */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       3, */
/* 							       1); */
/* 				wake_up_interruptible(&privdata->event_wait); */
/* 			} else { */
/* 				privdata->overrun_flag = 1; */
/* 			} */
/* 		} */
	}

	tasklet_schedule(&privdata->fai_tasklet);

/* 	if (privdata->cur_index == privdata->int_cnt) { /\* interrupt count *\/ */
/* 		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1); */
/* 	} */

/* 	wake_up_interruptible(&privdata->event_wait); */
}


/**
 * pci1742_dma_isr - interrupt service routine for DMA
 *                   data acquisition
 *
 * ptr: point to the private data of device object
 */
static void pci1742_dma_isr(private_data *ptr)
{
	private_data *privdata = ptr;
	INT16U tmp;

	
/* 	KdPrint("dma isr!\n"); */
	
	tmp = advInpDMA(privdata, 0xa8);
	if ((tmp & 0x10) == 0x10) {	/* DMA transfer done */
		advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x9040);
		advOutp(privdata, 0x08, 0x00);

		/* clear DMA interrupt */
		advOutpDMA(privdata, 0xa8, tmp | 0x08);
		
		/* privdata->buf_stat = 1; */
	} else {
		/* clear DMA interrupt */
		advOutpDMA(privdata, 0xa8, tmp | 0x08);
	}
		
	     

	if (!privdata->buf_stat) {
		privdata->transfered += privdata->conv_num / 2;
		privdata->half_ready = 1;

		/* adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
	} else {
		privdata->transfered += privdata->conv_num / 2;
		privdata->half_ready = 2;

/* 		if (!privdata->cyclic) { /\* terminate *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1); */
/* 		} else {	/\* buffer change *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */

/* 			if (privdata->overrun_flag) { /\* overrun *\/ */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       3, */
/* 							       1); */
/* 			} else { */
/* 				privdata->overrun_flag = 1; */
/* 			} */
/* 		} */
	}

	privdata->buf_stat = !privdata->buf_stat;

	tasklet_schedule(&privdata->fai_tasklet);

/* 	if (privdata->cur_index == privdata->int_cnt) { /\* interrupt count *\/ */
/* 		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1); */
/* 	} */

/* 	wake_up_interruptible(&privdata->event_wait); */
}


/**
 * pci1742_interrupt_handler - PCI1742 interrupt handler                    
 *
 *                                                            
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before the processor
 *        entered interrupt code (NULL)        
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1742_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1742_interrupt_handler(int irq, void *dev_id)
#endif
{
	private_data *privdata = (private_data *) dev_id;
	INT16U tmp;
     

/* 	printk("isr!\n"); */
/* 	mdelay(1000); */
	
	tmp = advInp(privdata, 0x07) & 0x08;
	if (tmp) { /* interrupt from A/D or FIFO */
/* 		KdPrint("isr!\n"); */
		
		if (privdata->fifo_enable) {
			pci1742_fifo_isr(privdata);
		} else {
			pci1742_nofifo_isr(privdata);
		}

		/* clear interrupt */
		advOutp(privdata, 0x08, 0x00);
		/* advOutp(privdata, 0x09, 0x00); */
	} else { /* interrupt from PCI9054 */
		tmp = advInp(privdata, 0x6a) & 0x20;
		if (tmp != 0x20) {
			return IRQ_RETVAL(0);
		}

		pci1742_dma_isr(privdata);

		/* clear DMA interrupt */
		/* advOutpDMA(privdata, 0xa8, advInpDMA(privdata, 0xa8) | 0x08); */
	}
     
	return IRQ_RETVAL(1);
}	     
