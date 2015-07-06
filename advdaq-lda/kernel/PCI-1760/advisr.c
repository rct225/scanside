/********************************************************************
 *				                                    
 * 	Copyright 2004 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-176x Device driver for Linux              
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
  
#include "PCI1760.h"

/************************************************************************
 * Description:  PCI1760 interrupt handler                    
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
 **************************************************************/
irqreturn_t pci1760_interrupt_handler(INT32S irq, void *dev_id, struct pt_regs *regs)
{
	private_data *privdata = (private_data *) dev_id;
	INT16U tmp = 0;
	INT32U i;
     


	tmp = advInp(privdata, 0x3a);
	if (tmp != 0x00c0) {
		/* printk("The interrupt isn't belongs to this device\n"); */
		return IRQ_RETVAL(0);
	}
	advOutp(privdata, 0x39, 0x0e); /* disable interrupt */
	
	advOutp(privdata, 0x0e, 0x00); /* clear InputMailBox */
	for (i = 0; i < 60000; i++) {
		if (advInp(privdata, 0x1e) == 0x00) {
			break;
		}
	}
     
	advOutp(privdata, 0x0e, 0x60); /* command: read interrupt flag */
	for (i = 0; i < 60000; i++) {
		if (advInp(privdata, 0x1e) == 0x60) {
			break;
		}
	}


	tmp = advInp(privdata, 0x1c); /* read interrupt flag */
	
	if (tmp & 0x01) {	/* pattern match */
		privdata->pattern_data = tmp;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}

	if (tmp & 0x02) {	/* status change */
		advOutp(privdata, 0x0e, 0x61); /* command: read IDI edge change flags */
		for (i = 0; i < 10000; i++) {
			if (advInp(privdata, 0x1e) == 0x61) {
				break;
			}
		}

		/* read the edge change flags
		 * note: hardware manual is not correct, so we fixed the bug at here
		 *       put low byte to high, put high byte to low */
		privdata->status_data = advInp(privdata, 0x1d)
			| (advInp(privdata, 0x1c) << 8); 

		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
	}

	if (tmp & 0x04) {	/* counter overflow/match */
		advOutp(privdata, 0x0e, 0x62); /* command: read counter overflow/match */
		for (i = 0; i < 10000; i++) {
			if (advInp(privdata, 0x1e) == 0x62) {
				break;
			}
		}

		/* read the counter match flags */
		privdata->cnt_match_data = advInp(privdata, 0x1c);
		if (privdata->cnt_match_data & 0xff) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
		}

		/* read the counter overflow flags */
		privdata->cnt_overflow_data = advInp(privdata, 0x1d);
		if (privdata->cnt_overflow_data & 0xff) {
			advOutp(privdata, 0x0c, 0xff);
			advOutp(privdata, 0x0e, 0x29);
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
	}	   
	wake_up_interruptible(&privdata->event_wait);
	advOutp(privdata, 0x39, 0x20); /* enable interrupt */

	return IRQ_RETVAL(1);
}

