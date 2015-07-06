/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1714 Device driver for Linux              
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
 * 	10/09/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1714.h"


/**
 * pci1714_dma_isr - interrupt service routine for DMA
 *                   data acquisition
 *
 * ptr: point to the private data of device object
 */
/* static void pci1714_dma_isr(private_data *ptr) */
/* { */
/* 	private_data *privdata = ptr; */
/* 	INT16U tmp; */
/* 	INT32U virt_addr; */



/* } */

void fai_tasklet_fn(PTR_T arg)
{
	private_data *privdata = (private_data *) (((adv_device *) arg)->private_data);
	INT16U empty_flag_cnt = 0;
	INT16U remain_cnt;
	INT16U trig_idx;
	INT16U last_idx;
	INT16U count;
	INT16U chan;
	INT16U i, j;
	INT16U tmp = 0;	


	
	/* KdPrint("halfready_flag: %d\n", privdata->halfready_flag); */
	
	if (privdata->half_ready == 1) { /* low buffer ready */
		adv_process_info_set_event_all(&privdata->ptr_process_info,
					       1, 1);
		
		 /* data is not taken off, so overrun */
		if (privdata->halfready_flag == 2) {
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       4, 1);
			privdata->overrun_flag = 1;
		}
		privdata->halfready_flag = 1;
	} else if (privdata->half_ready == 2) {	/* high buffer ready */
		adv_process_info_set_event_all(&privdata->ptr_process_info,
					       2, 1);

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
	
	if (privdata->dma_tc_flag) {
		adv_process_info_set_event_all(&privdata->ptr_process_info,
					       3, 1);

		/* waitting for FIFO empty */
		j = 0;
		do {
			empty_flag_cnt = 0;
			
			for (i = 0; i < privdata->num_chan; i++) {
				
				tmp = privdata->start_chan + i;
				
				switch (tmp) {
				case 0: /* check FIFO_AE(fifo almost empty flag) */
					if (advInpw(privdata, 0x10) & 0x0010) {
						empty_flag_cnt++;
					}
					break;
				case 1:	/* check FIFO_AE(fifo almost empty flag) */
					if (advInpw(privdata, 0x10) & 0x1000) {
						empty_flag_cnt++;
					}					
					break;                     
				case 2:	/* check FIFO_AE(fifo almost empty flag) */
					if (advInpw(privdata, 0x12) & 0x0010) {
						empty_flag_cnt++;
					}					
					break;                     
				case 3:	/* check FIFO_AE(fifo almost empty flag) */
					if (advInpw(privdata, 0x12) & 0x1000) {
						empty_flag_cnt++;
					}
					break;					
				}				
			}
			
			j++;			
		} while ((empty_flag_cnt != privdata->num_chan) && j < 5000);


		/* DMA is stopped when FIFO almost empty, so check how many data
		 * remain in fifo */
		trig_idx = last_idx = 0;
		
		for (i = 0; i < privdata->conv_num - 1; i++) {
			
			if ((((privdata->user_buf)[i] & 0x8000) == 0x0000)
			    && (((privdata->user_buf)[i + 1] & 0x8000) == 0x8000)) {
				trig_idx = i + 1;				
			}
			
			if ((((privdata->user_buf)[i] & 0x8000) == 0x8000)
			    && (((privdata->user_buf)[i + 1] & 0x8000) == 0x0000)) {
				last_idx = i + 1;				
			}
			
		}
		
		if ((trig_idx == last_idx) && /* for post trigger mode condition */
		    (((privdata->user_buf)[0] & 0x8000) == 0x8000)) {
			remain_cnt = 0;			
		} else {	/* for other trigger mode condition */
			count = (last_idx - trig_idx + privdata->conv_num) % privdata->conv_num;
			remain_cnt = (count >= privdata->delay_cnt * privdata->num_chan) ?
				0 : (privdata->delay_cnt * privdata->num_chan - count);
		}
		
		/* get data remain in FIFO */
		for (j = (INT16U) (remain_cnt / privdata->num_chan); j > 0; j--) {
			for (i = 0; i < privdata->num_chan; i++) {
				chan = privdata->start_chan + i;
				privdata->user_buf[last_idx] = advInpw(privdata, 2 * chan);
				last_idx = (last_idx + 1) % privdata->conv_num;
			}
		}
	}

	if (privdata->fifo_af_flag) {
		adv_process_info_set_event_all(&privdata->ptr_process_info,
					       4,
					       1);

		/* re-enable almost full interrupt after overrun event occured */
		tmp = privdata->int_ctrl_reg | 0x00aa;
		advOutpw(privdata, 0x20, tmp);

		privdata->fifo_af_flag = 0;
	}

	wake_up_interruptible(&privdata->event_wait);
	

	return;
}

/**
 * pci1714_interrupt_handler - PCI1714 interrupt handler                    
 *
 *                                                            
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before the processor
 *        entered interrupt code (NULL)        
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1714_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1714_interrupt_handler(int irq, void *dev_id)
#endif
{
	private_data *privdata = (private_data *) dev_id;
	INT16U tmp;

     

	tmp = advInpw(privdata, 0x20);
	if (tmp & 0x8000) {	/* interrupt from board */
		if (tmp & 0x0100) { /* DMA_TC interrupt */
			if (advInpw(privdata, 0x0e) & 0x4000) {
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       3, */
/* 							       1); */
				privdata->dma_tc_flag = 1;
			}
		} else if (tmp & 0x00aa) { /* AF interrupt */
			tmp = privdata->int_ctrl_reg & 0xff00;
			advOutpw(privdata, 0x20, tmp);

			privdata->fifo_af_flag = 1;

			
			if ((privdata->cyclic == 0) /* non-cyclic mode */
			    && (privdata->conv_num <= privdata->half_fifo_size * 2)) {
				privdata->overrun_flag = 0;
			}

/* 			if (privdata->fifo_af_flag) { */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       4, */
/* 							       1); */
/* 			} */
		}

/* 		wake_up_interruptible(&privdata->event_wait); */

/* 		/\* re-enable almost full interrupt after overrun event occured *\/ */
/* 		if (privdata->fifo_af_flag) { */
/* 			tmp = privdata->int_ctrl_reg | 0x00aa; */
/* 			advOutpw(privdata, 0x20, tmp); */

/* 			privdata->fifo_af_flag = 0; */
/* 		} */
		
		advOutp(privdata, 0x22, 0x00); /* clear interrupt */
	} else {		/* interrupt from 9056 */
		tmp = advInpDMAw(privdata, 0x6a) & 0x20;
		if (!tmp) {	/* no interrupt occur */
			return IRQ_RETVAL(0);
		}
		
		tmp = advInpDMA(privdata, 0xa8);
		if (tmp & 0x10) {	/* DMA transfer done */
			advOutpw(privdata, 0x20, 0x0000);

			privdata->buf_stat = 1;
		}

		if (!privdata->buf_stat) {
			privdata->transfered += (privdata->conv_num / 2);
			privdata->half_ready = 1;

/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 						       1, 1); */

/* 			if (privdata->halfready_flag) { /\* overrun *\/ */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       4, 1); */
/* 				privdata->overrun_flag = 1; */
/* 			} */
/* 			privdata->halfready_flag = 1; */
		} else {
			privdata->transfered += (privdata->conv_num / 2);
			privdata->half_ready = 2;
		
/* 			if (!privdata->cyclic) { /\* terminate *\/ */
/* 				privdata->terminated = 1; */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       3, 1); */
/* 			} else {	/\* high buffer ready *\/ */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       2, 1); */
			
/* 				if (privdata->halfready_flag) { /\* overrun *\/ */
/* 					adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 								       4, 1); */
/* 					privdata->overrun_flag = 1; */
/* 				} */
/* 			privdata->halfready_flag = 2; */
/* 			} */
		}
/* 		privdata->halfready_flag = 1; */
/* 		wake_up_interruptible(&privdata->event_wait); */

		privdata->buf_stat = !privdata->buf_stat;

		tmp |= 0x08;	/* clear DMA interrupt */
		advOutpDMA(privdata, 0xa8, tmp);
	}

	tasklet_schedule(&privdata->fai_tasklet);
     

	return IRQ_RETVAL(1);
}	     

/**
 * pci1714_interrupt_handler - PCI1714 interrupt handler                    
 *
 *                                                            
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before the processor
 *        entered interrupt code (NULL)        
 */
/* irqreturn_t pci1714_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs) */
/* { */
/* 	private_data *privdata = (private_data *) dev_id; */
/* 	INT16U tmp; */
/* /\* 	INT32U virt_addr; *\/ */

     

/* 	tmp = advInpw(privdata, 0x20); */
/* 	if (tmp & 0x8000) {	/\* interrupt from board *\/ */
/* 		if (tmp & 0x0100) { /\* DMA_TC interrupt *\/ */
/* 			if (advInpw(privdata, 0x0e) & 0x4000) { */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       3, */
/* 							       1); */
/* 				privdata->dma_tc_flag = 1; */

/* /\* 			virt_addr = privdata->user_pages[0].page_addr *\/ */
/* /\* 				+ privdata->user_pages[0].offset; *\/ */
/* /\* 			KdPrint("low ready: 0x%x\n", *((INT16U *) virt_addr)); *\/ */
/* 			} */
/* 		} else if (tmp & 0x00aa) { /\* AF interrupt *\/ */
/* 			tmp = privdata->int_ctrl_reg & 0xff00; */
/* 			advOutpw(privdata, 0x20, tmp); */

/* 			privdata->fifo_af_flag = 1; */
/* 			/\* privdata->overrun_flag = 1; *\/ */
			
/* 			if ((privdata->cyclic == 0) /\* non-cyclic mode *\/ */
/* 			    && (privdata->conv_num <= privdata->half_fifo_size * 2)) { */
/* 				privdata->overrun_flag = 0; */
/* 			} */

/* 			if (privdata->fifo_af_flag) { */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       4, */
/* 							       1); */
/* 			}			 */
/* 		} */

/* 		wake_up_interruptible(&privdata->event_wait); */

/* 		/\* re-enable almost full interrupt after overrun event occured *\/ */
/* 		if (privdata->fifo_af_flag) { */
/* 			tmp = privdata->int_ctrl_reg | 0x00aa; */
/* 			advOutpw(privdata, 0x20, tmp); */

/* 			privdata->fifo_af_flag = 0; */
/* 		} */
		
/* 		advOutp(privdata, 0x22, 0x00); /\* clear interrupt *\/ */
/* 	} else {		/\* interrupt from 9056 *\/ */
/* 		tmp = advInpDMAw(privdata, 0x6a) & 0x20; */
/* 		if (!tmp) {	/\* no interrupt occur *\/ */
/* 			return IRQ_RETVAL(0); */
/* 		} */
		
/* 		tmp = advInpDMA(privdata, 0xa8); */
/* 		if (tmp & 0x10) {	/\* DMA transfer done *\/ */
/* 			advOutpw(privdata, 0x20, 0x0000); */

/* 			privdata->buf_stat = 1; */
/* 		} */
/* 		/\* KdPrint("advisr\n");	 *\/ */
/* 		if (!privdata->buf_stat) { */
/* 			privdata->cur_index = privdata->conv_num / 2; */
/* 			privdata->half_ready = 1; */

/* /\* 			virt_addr = privdata->user_pages[0].page_addr *\/ */
/* /\* 				+ privdata->user_pages[0].offset; *\/ */
/* /\* 			KdPrint("low ready: 0x%x\n", *((INT16U *) virt_addr) & 0xfff); *\/ */
		
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 						       1, 1); */

/* 			if (privdata->halfready_flag) { /\* overrun *\/ */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       4, 1); */
/* 				privdata->overrun_flag = 1; */
/* 			} */
/* 			privdata->halfready_flag = 1; */
/* 		} else { */
/* 			privdata->cur_index = privdata->conv_num; */
/* 			privdata->half_ready = 2; */
		
/* 			if (!privdata->cyclic) { /\* terminate *\/ */
/* 				privdata->terminated = 1; */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       3, 1); */
/* 			} else {	/\* high buffer ready *\/ */
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 							       2, 1); */
			
/* 				if (privdata->halfready_flag) { /\* overrun *\/ */
/* 					adv_process_info_set_event_all(&privdata->ptr_process_info, */
/* 								       4, 1); */
/* 					privdata->overrun_flag = 1; */
/* 				} */
/* 				privdata->halfready_flag = 1; */
/* 			} */
/* 		} */

/* 		wake_up_interruptible(&privdata->event_wait); */

/* 		privdata->buf_stat = !privdata->buf_stat; */

/* 		tmp |= 0x08;	/\* clear DMA interrupt *\/ */
/* 		advOutpDMA(privdata, 0xa8, tmp); */
/* /\* 		pci1714_dma_isr(privdata); *\/ */
/* 	} */
     
/* 	return IRQ_RETVAL(1); */
/* }	      */
