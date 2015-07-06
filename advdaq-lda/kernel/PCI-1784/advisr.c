/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1784 Device driver for Linux              
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
 * 	11/22/2006			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "PCI1784.h"


/**
 * pci1784_interrupt_handler - PCI1784 interrupt handler                    
 *
 *                                                            
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before the processor
 *        entered interrupt code (NULL)        
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1784_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1784_interrupt_handler(int irq, void *dev_id)
#endif
{
	private_data *privdata = (private_data *) dev_id;
	INT32U dwIntStatus;
	INT32U mask,usEvtIndex=0;
	int i=0;
	INT16U start,count;
	dwIntStatus = advInpdw(privdata, 0x20);
	if (dwIntStatus & 0x80000000) {
	for(i=0;i<24;i++){
		mask = 0x00000001 << i;
		if(mask & dwIntStatus){
			adv_process_info_set_event_all(&privdata->ptr_process_info,i,1);
		}
	}

	if(dwIntStatus&(ULONG)0x10000000){
		adv_process_info_set_event_all(&privdata->ptr_process_info,i,1);
	}
	usEvtIndex = privdata->cntrIndex;
	start = privdata->CntrEvtCfg[usEvtIndex].portStart;
	count = privdata->CntrEvtCfg[usEvtIndex].portCount;
	for(i=start;i<start+count;i++){
		if(privdata->ulDevPara[i]&0x100){
			advOutpdw(privdata,0x28,(0x1<<i));
		}
		privdata->CntrEvtCfg[usEvtIndex].evtData[i] = advInpdw(privdata,i*4);
	}

	usEvtIndex = privdata->dioIndex;
	privdata->DIEvtCfg[usEvtIndex].evtData = advInpdw(privdata,0x30)>>4;
	wake_up_interruptible(&privdata->event_wait);
	advInpdw(privdata,0x24);
		return IRQ_RETVAL(0);
	
	}else{
		return IRQ_RETVAL(1);
	}
}	     
   

