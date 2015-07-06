/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
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
 * 	07/20/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1761.h"

/************************************************************************
 * Description:  PCI1761 interrupt handler                    
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1761_interrupt_handler(INT32S irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1761_interrupt_handler(INT32S irq, void *dev_id)
#endif
{
	private_data *privdata = (private_data *) dev_id;
	INT8U status;
	/* INT8U tmp; */
	INT8U mask;
	INT16U i;	


	/* note: the hardware manual is not correct, Appendix C
	 *       except to C.2 */     
	status = advInp(privdata, 0x05);
	if (!status) {
		/* printk("The interrupt isn't belongs to this device\n"); */
		return IRQ_RETVAL(0);
	}
	
	/* tmp = advInp(privdata, 0x03); */
	/* advOutp(privdata, 0x03, 0x00); /\* disable interrupt *\/ */
	
	for (i = 0; i < 8; i++) {
		mask = 0x01 << i;
		if (status & mask) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, i, 1);
		}		
	}
	wake_up_interruptible(&privdata->event_wait);

	advOutp(privdata, 0x05, 0xff); /* clear interrupt */
	/* advOutp(privdata, 0x03, tmp); /\* enable interrupt *\/ */

	return IRQ_RETVAL(1);
}

/************************************************************************
 * Description:  PCI1762 interrupt handler                    
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1762_interrupt_handler(INT32S irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1762_interrupt_handler(INT32S irq, void *dev_id)
#endif
{
	private_data *privdata = (private_data *) dev_id;
	INT16U status;
	INT16U tmp;
	INT16U mask;
	INT16U i;	


	status = advInpw(privdata, 0x06) & 0x0101;
	if (!status) {
		/* printk("The interrupt isn't belongs to this device\n"); */
		return IRQ_RETVAL(0);
	}
	
	tmp = advInpw(privdata, 0x06); /* load interrupt status */
/* 	advOutpw(privdata, 0x06, tmp & 0xfdfd); /\* disable interrupt *\/ */

	
	for (i = 0; i < 2; i++) {
		mask = 0x0001 << (i * 8);
		if (status & mask) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, i * 8, 1);
		}		
	}
	wake_up_interruptible(&privdata->event_wait);

	advOutpw(privdata, 0x06, tmp | 0x0101);	/* clear interrupt */
/* 	advOutp(privdata, 0x06, tmp); /\* enable interrupt *\/ */

	return IRQ_RETVAL(1);
}
