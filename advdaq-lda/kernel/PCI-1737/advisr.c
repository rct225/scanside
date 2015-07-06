/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1737 Device driver for Linux              
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
 * 	10/23/2006			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "PCI1737.h"


/**
 * pci1737_interrupt_handler - PCI1737 interrupt handler                    
 *
 *                                                            
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before the processor
 *        entered interrupt code (NULL)        
 */
irqreturn_t pci1737_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	private_data *privdata = (private_data *) dev_id;
	INT8U status;

	switch (privdata->device_type){
	case PCI1737:
		status = advInp(privdata, 0x08);
		if (!(status&0x08)) {
			return IRQ_RETVAL(0);
		}
	
		if (status & 0x08&&(advInp(privdata, 0x03)&0x01))	{
			adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
		}
		break;
		
	case PCI1739:
		status = advInp(privdata, 0x08);
		if (!(status&0x88)) {
			return IRQ_RETVAL(0);
		}

		if (status & 0x08&&(advInp(privdata, 0x03)&0x01))	{
			adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);

		}
		if (status & 0x80&&(advInp(privdata, 0x07)&0x01)){
			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
		}
		break;
	default:
		return IRQ_RETVAL(0);
	}
	
	wake_up_interruptible(&privdata->event_wait);
	advOutp(privdata, 0x08, status|0x80); /* enable interrupt */	
	return IRQ_RETVAL(1);
}	     


