/********************************************************************
 *				                                    
 * 	    Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		    BeiJing R&D Center                          
 *	                                                            
 * 		    Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1758U Device driver for Linux              
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
 * 	09/12/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1758U.h"

/**
 * pci1758U_interrupt_handler - PCI1758U interrupt handler
 *
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before
 *        the processor entered interrupt code (NULL)                              
 */
irqreturn_t pci1758U_interrupt_handler(INT32S irq, void *dev_id, struct pt_regs *regs)
{
	private_data *privdata = (private_data *) dev_id;
	INT16U max_chan_num;
	INT16U max_port_num = 0;
	INT16U i, j, k;	
	INT16U tmp = 0;
	INT16U mask;
	INT8U wd_reg = 0;
	INT8U int_port_reg = 0;
	INT8U int_chan_reg = 0;
	INT8U status;
	

	
	
	switch (privdata->device_type) {
	case PCI1758UDI:
		max_chan_num = MAX_DI_CHANNEL;
		max_port_num = MAX_DI_PORT;
		int_port_reg = 0x50;
		int_chan_reg = 0x30;
		
		break;
	case PCI1758UDO:
		max_chan_num = 0;
		wd_reg = 0x14;
				
		break;
	case PCI1758UDIO:
		max_chan_num = MAX_DIO_CHANNEL;
		max_port_num = MAX_DIO_PORT;
		int_port_reg = 0x40;
		int_chan_reg = 0x20;
		wd_reg = 0x38;

		break;
	default:
		return IRQ_RETVAL(1);
	}
	

	status = advInpw(privdata, int_port_reg);
	if (status && int_port_reg) {		/* interrupt from DI */
		mask = 0x0001;
		
		/* get interrupt status for all channels */
		for (i = 0; i < max_port_num / 2; i++) {
			tmp = advInpw(privdata, int_chan_reg + (i * 2));
			((INT16U *) privdata->int_stat)[i] = tmp;
		}

		/* check which channel cause the interrupt*/
		for (i = 0, j = 0, k = 0; i < max_chan_num; i++, k++) {
			if (i % 16 == 0) {
				tmp = ((INT16U *) privdata->int_stat)[j];
				j++;
				k = 0;
			}
			
			mask = 0x0001 << k;
			if (tmp & mask) {
				adv_process_info_set_event_all(&privdata->ptr_process_info, i, 1);
				wake_up_interruptible(&privdata->event_wait);

				/* clear interrupt */
				advOutpw(privdata, int_chan_reg + ((j - 1) * 2), mask);
			}
		}
	} else {
		status = advInpw(privdata, wd_reg) & 0x04;
		if (status) {	/* interrupt from watchdog */
			advOutpw(privdata, wd_reg, 0x01);
			advOutpw(privdata, wd_reg, 0x03);

			adv_process_info_set_event_all(&privdata->ptr_process_info,
			max_chan_num, 1);
		}
	}
	

	/* the interrupt isn't belongs to this device */
	if (!status) {
		return IRQ_RETVAL(0);
	}
 
	wake_up_interruptible(&privdata->event_wait);


	return IRQ_RETVAL(1);
}

