/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1716 Device driver for Linux              
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
 * 	09/26/2006			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "PCI1716.h"



/**
 * pci1716_nofifo_isr - interrupt service routine for without fifo
 *                      data acquisition
 *
 * ptr: point to the private data of device object
 *
 * interrupt when an A/D conversion occurs, read the fifo data
 * into user buffer, until user buffer is half full or full,se
 * nd 'buffer change' or 'terminate' event to user.
 */
static void pci1716_nofifo_isr(private_data *ptr)
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
	if (privdata->cur_index == privdata->conv_num / 2) {
		privdata->retrieved += privdata->conv_num / 2;

		if (privdata->halfready_flag == 2) {
			privdata->overrun_flag = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       3,
						       1);
		}
		privdata->half_ready = 1;
		privdata->halfready_flag = 1;

		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}

	if (privdata->cur_index == privdata->conv_num) {
		privdata->retrieved += privdata->conv_num / 2;

		privdata->half_ready = 2;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);

		if (privdata->halfready_flag == 1) {
			privdata->overrun_flag = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       3,
						       1);
		}
		privdata->halfready_flag = 2;

		if (!privdata->cyclic){
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
			advOutpDMAw(privdata, 0x68, 0x0000);
			advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x8040);
		} 

		
	}
	     


	wake_up_interruptible(&privdata->event_wait);
}


/**
 * pci1716_fifo_isr - interrupt service routine for without fifo
 *                    data acquisition
 *
 * ptr: point to the private data of device object
 *
 * interrupt when fifo is half full, read the fifo data into
 * user buffer, until user buffer is half full or full, send
 * 'buffer change' or 'terminate' event to user.
 */
static void pci1716_fifo_isr(private_data *ptr)
{
	private_data *privdata = ptr;
	adv_user_page *page = NULL;
	INT16U tmp;
	INT16U i;

     
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
		privdata->retrieved++;
		
		i++;
	} while ((i < privdata->half_fifo_size) && (privdata->cur_index < privdata->conv_num));



	/* set event */
	if (privdata->cur_index == privdata->conv_num / 2) { /* buffer change */

		
		if (privdata->halfready_flag == 2) {
			privdata->overrun_flag = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
		
		privdata->half_ready = 1;
		privdata->halfready_flag = 1;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}

	if (privdata->cur_index == privdata->conv_num) {
		privdata->half_ready = 2;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);

		if (privdata->halfready_flag == 1) {
			privdata->overrun_flag = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
		privdata->halfready_flag = 2;
		if (!privdata->cyclic) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
			advOutpDMAw(privdata, 0x68, 0x0000);
			advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x8040);
		}

	}

	wake_up_interruptible(&privdata->event_wait);
/* 	advOutp(privdata, 0x08,0); */

}


/**
 * pci1716_dma_isr - interrupt service routine for DMA
 *                   data acquisition
 *
 * ptr: point to the private data of device object
 */
static void pci1716_dma_isr(private_data *ptr)
{
	private_data *privdata = ptr;
	INT16U tmp;


	privdata->retrieved += privdata->conv_num / 2;

	tmp = advInpDMA(privdata, 0xa8);
	if ((tmp & 0x10) == 0x10) {	/* DMA transfer done */
		advOutp(privdata, 0x08, 0x00);
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
		privdata->buf_stat = 1;
	}


	if (!privdata->buf_stat) {
		privdata->cur_index = privdata->conv_num / 2;
		privdata->half_ready = 1;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);

		if (privdata->halfready_flag == 2) {
			privdata->overrun_flag = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
		privdata->halfready_flag = 1;
	} else {
		privdata->cur_index = privdata->conv_num;
		privdata->half_ready = 2;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
		if (privdata->halfready_flag == 1) {
			privdata->overrun_flag = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
		privdata->halfready_flag = 2;

		if (!privdata->cyclic) { /* terminate */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
			privdata->terminated = 1;			
		} 
		
	}

		wake_up_interruptible(&privdata->event_wait);

		privdata->buf_stat = !privdata->buf_stat;

		tmp |= 0x08;	/* clear DMA interrupt */
		advOutpDMA(privdata, 0xa8, tmp);

}


/**
 * pci1716_interrupt_handler - PCI1716 interrupt handler                    
 *
 *                                                            
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before the processor
 *        entered interrupt code (NULL)        
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1716_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1716_interrupt_handler(int irq, void *dev_id)
#endif
{
	private_data *privdata = (private_data *) dev_id;
	INT16U tmp;


	tmp = advInpw(privdata, 0x06) & 0x0800;
	if (tmp) {
		if (privdata->fifo_enable) {
			privdata->ai_mode = 0x01;
/* 			pci1716_fifo_isr(privdata); */
		} else {
			privdata->ai_mode = 0x02;
/* 			pci1716_nofifo_isr(privdata); */
		}

		advOutp(privdata, 0x08, 0x00); /* clear interrupt */
	} else {
		tmp = advInpDMA(privdata, 0x6a) & 0x20;
		if (!tmp) {
			privdata->ai_mode = 0xff;
			return IRQ_RETVAL(0);
		}
		privdata->ai_mode = 0x03;
/* 		pci1716_dma_isr(privdata); */
		advOutpDMA(privdata, 0xa8, advInpDMA(privdata, 0xa8) | 0x08);
	}
	tasklet_schedule(&privdata->tasklet);
	return IRQ_RETVAL(1);
}	     

void pci1716_tasklet_handler(PTR_T dev_id)
{
	private_data   *privdata;

	privdata = (private_data *)dev_id;
	spin_lock(&privdata->spinlock);
	
	if(privdata->ai_mode == 0x01){
		pci1716_fifo_isr(privdata);
	}
	if(privdata->ai_mode == 0x02){
		pci1716_nofifo_isr(privdata);
	}
	if(privdata->ai_mode == 0x03){
		pci1716_dma_isr(privdata);
	}
	wake_up_interruptible(&privdata->event_wait);
	spin_unlock(&privdata->spinlock);
	return;	
}
