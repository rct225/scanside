/********************************************************************
 *				                                    
 * 	Copyright 2004 ABJ Linux Driver Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1741U Device driver for Linux              
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
 * 	12/12/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1741U.h"



/**
 * pci1741U_nofifo_isr - interrupt service routine for without fifo
 *                      data acquisition
 *
 * ptr: point to the private data of device object
 *
 * interrupt when an A/D conversion occurs, read the fifo data
 * into user buffer, until user buffer is half full or full,se
 * nd 'buffer change' or 'terminate' event to user.
 */
static void pci1741U_nofifo_isr(private_data *ptr)
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
		privdata->transfered += (privdata->conv_num / 2);
		privdata->half_ready = 1;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);

		if (privdata->halfready_flag == 2) { /* overrun */
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       4, 1);
			privdata->overrun_flag = 1;
		}
		privdata->halfready_flag = 1;

/* 		privdata->half_ready = 1; */

/* 		if (privdata->overrun_flag){ /\* overrun *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 						       3, */
/* 						       1); */
/* 		} else {  /\* low buffer ready *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
			
/* 			privdata->overrun_flag = 1; */
/* 		} */
	}

	if (privdata->cur_index == privdata->conv_num) {
		privdata->transfered += (privdata->conv_num / 2);
		privdata->half_ready = 2;

/* 		if (!privdata->cyclic) { /\* terminate *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1); */
/* 			advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x8040); */
/* 			privdata->terminated = 1; */
/* 		} else {	/\* high buffer ready *\/ */
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
			
		if (privdata->halfready_flag == 1) { /* overrun */
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       4, 1);
			privdata->overrun_flag = 1;
		}
		privdata->halfready_flag = 2;
/* 		} */

		if (!privdata->cyclic) { /* terminate */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
			advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x0040);
			privdata->terminated = 1;

			/* we schedule 'fai_stop_work' thread to terminate fai */
			schedule_work(&privdata->fai_stop_work);
		}

/* 		privdata->half_ready = 2; */
	     
/* 		if (!privdata->cyclic) { /\* terminate *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1); */
/* 			/\* advOutpDMAw(privdata, 0x68, 0x0000); *\/ */
/* 			advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x8040); */
/* 			privdata->terminated = 1; */
/* 		} else {		/\* buffer change *\/ */
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
		     
/* 			if (privdata->overrun_flag) { /\* overrun *\/ */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       3, */
/* 							       1); */
/* 			} else { */
/* 				privdata->overrun_flag = 1; */
/* 			} */
/* 			/\* privdata->terminal_cnt += 1; *\/ */
/* 		} */
	}

	if (privdata->cur_index == privdata->int_cnt) { /* interrupt count */
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}

	wake_up_interruptible(&privdata->event_wait);
}


/**
 * pci1741U_fifo_isr - interrupt service routine for without fifo
 *                    data acquisition
 *
 * @ptr: point to the private data of device object
 *
 * interrupt when fifo is half full, read the fifo data into
 * user buffer, until user buffer is half full or full, send
 * 'buffer change' or 'terminate' event to user.
 */
static void pci1741U_fifo_isr(private_data *ptr)
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
		
		privdata->item += sizeof(INT16U);
		privdata->cur_index++;

		i++;
	} while ((i < privdata->half_fifo_size)
		 && (privdata->cur_index < privdata->conv_num)
		 && (privdata->cur_index != privdata->conv_num / 2));


	/* set event */
	if (privdata->cur_index == privdata->conv_num / 2) {
		privdata->transfered += (privdata->conv_num / 2);
		privdata->half_ready = 1;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
		
		if (privdata->halfready_flag == 2) { /* overrun */
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       4, 1);
			privdata->overrun_flag = 1;
		}
		privdata->halfready_flag = 1;
	}

	if (privdata->cur_index == privdata->conv_num) {
		privdata->transfered += (privdata->conv_num / 2);
		privdata->half_ready = 2;

		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);

		if (privdata->halfready_flag == 1) { /* overrun */
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       4, 1);
			privdata->overrun_flag = 1;
		}
		privdata->halfready_flag = 2;

		if (!privdata->cyclic) { /* terminate */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
			advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x0040);
			privdata->terminated = 1;

			/* we schedule 'fai_stop_work' thread to terminate fai */
			schedule_work(&privdata->fai_stop_work);
		}
	}

	if (privdata->cur_index == privdata->int_cnt) { /* interrupt count */
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}

	wake_up_interruptible(&privdata->event_wait);
}




/**
 * pci1741U_interrupt_handler - PCI1741U interrupt handler                    
 *
 *                                                            
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before the processor
 *        entered interrupt code (NULL)        
 */
irqreturn_t pci1741U_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	private_data *privdata = (private_data *) dev_id;
	INT16U tmp;
     

	tmp = advInp(privdata, 0x07) & 0x08;
	if (tmp) {
		if (privdata->fifo_enable) {
			pci1741U_fifo_isr(privdata);
		} else {
			pci1741U_nofifo_isr(privdata);
		}

		advOutp(privdata, 0x08, 0x00); /* clear interrupt */
	} else {
		return IRQ_RETVAL(0);
	}
	
     
	return IRQ_RETVAL(1);
}	     
