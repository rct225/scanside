/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1712 Device driver for Linux              
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
  
#include "PCI1712.h"



void pci1712_tasklet_handler(PTR_T dev_id)
{
	private_data   *privdata;
	privdata = (private_data *)dev_id;

	spin_lock(&privdata->spinlock);



	if(privdata->usFAIRunning) {       

		if(privdata->sub_event & EVENT_AI_LOBUF_OCCURRED) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
		}
		if(privdata->sub_event & EVENT_AI_HIBUF_OCCURRED){
			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
		}
		if(privdata->sub_event & EVENT_AI_OVERRUN_OCCURRED) {
			privdata->usOverrun = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
		
	  
		if (privdata->sub_event & EVENT_AI_TERMINATED_OCCURRED) {            
			privdata->usAITerminateFlag = 1;
			privdata->usAIStopped = 1;

			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
		}


		privdata->sub_event = EVENT_NONE;

	}




	if(privdata->usFAORunning) {       

		if(!privdata->ulUnderRun) {
			if(privdata->sub_event & EVENT_AO_LOBUF_OCCURRED) {
				while ((advInpDMA(privdata, 0xa9) & 0x10) != 0x10) {
					advInpDMA(privdata, 0xa9);
				}
				adv_process_info_set_event_all(&privdata->ptr_process_info, 4, 1);
			}
			if(privdata->sub_event & EVENT_AO_HIBUF_OCCURRED){
				while ((advInpDMA(privdata, 0xa9) & 0x10) != 0x10) {
					advInpDMA(privdata, 0xa9);
				}
				adv_process_info_set_event_all(&privdata->ptr_process_info, 5, 1);
			}
			if(privdata->sub_event & EVENT_AO_UNDERRUN_OCCURRED) {
				privdata->usUnderrunFlag = 1;
				adv_process_info_set_event_all(&privdata->ptr_process_info, 7, 1);
			}
		} else {
			privdata->usUnderrunFlag = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 7, 1);
		}
		
		
	  
		if (privdata->sub_event & EVENT_AO_TERMINATED_OCCURRED) {            
			privdata->usAOTerminateFlag = 1;
			privdata->usAOStopped = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 6, 1);
		}

		privdata->sub_event = EVENT_NONE;


	}
	
	spin_unlock(&privdata->spinlock);
	return;	
}


/**
* pci1712_interrupt_handler - PCI1712 interrupt handler                    
*
*                                                            
* @irq: interrupt request number            
* @dev_id: privdata data (device object)                      
* @regs: holds a snapshot of the processor's context before the processor
*        entered interrupt code (NULL)        
*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1712_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1712_interrupt_handler(int irq, void *dev_id)
#endif
{
	private_data *privdata = (private_data *) dev_id;
	INT8U	  tmp;  
	INT16U  tmp1;  
	INT8U   tmp2;  
	INT16U  usLocalIntStatus;           

	tmp  = advInpDMA( privdata, 0xa8 );
	tmp1 = advInpw( privdata, 0x06 );
	usLocalIntStatus = advInpw( privdata, 0x08 );

	if(usLocalIntStatus & 0x0400){
		printk(KERN_ERR "=========08 register: %x==============\n", usLocalIntStatus);
	}

	tmp2 = advInpDMA( privdata, 0x6a );


	if (((tmp2 & 0x20) != 0x20) && ((usLocalIntStatus & 0x1) != 0x1)) {
	  	return IRQ_RETVAL(0);
	}

	if (((tmp2 & 0x20) == 0x20)) {
		
		privdata->ulRetrieved += privdata->conv_num / 2;
		if((tmp & 0x10) != 0x10) {
			tmp = tmp | 0x08;
			advOutpDMA(privdata, 0xa8, tmp);
			if (privdata->AIHalfFullCnt % 2 == 0) {
				privdata->usAIHalfReady = 1;
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1); */
				privdata->sub_event |= EVENT_AI_LOBUF_OCCURRED;
				
				if(privdata->aiHalfReadyFlag == 2) {
/* 					adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1); */
					privdata->sub_event |= EVENT_AI_OVERRUN_OCCURRED;
					privdata->usOverrun = 1;
				}

				privdata->aiHalfReadyFlag = 1;

			} 
		}

		if (privdata->AIHalfFullCnt % 2 == 1) {
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1); */
			privdata->sub_event |= EVENT_AI_HIBUF_OCCURRED;
			privdata->usAIHalfReady = 2;

			if(privdata->aiHalfReadyFlag == 1) {
/* 				adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1); */
				privdata->sub_event |= EVENT_AI_OVERRUN_OCCURRED;
				privdata->usOverrun = 1;
			}

			privdata->aiHalfReadyFlag = 2;

			

		}
		privdata->AIHalfFullCnt ++;
      
		tmp  = advInpDMA(privdata, 0xa8);
		if( (tmp & 0x10) == 0x10 ) {
/* 			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1); */
			privdata->sub_event |= EVENT_AI_TERMINATED_OCCURRED;
			privdata->usAIStopped = 1;
			
			
			advOutp(privdata, 0x08, 0x00);
			tmp = tmp | 0x08;
			advOutpDMA(privdata, 0xa8, tmp);
		}
	  
	}



	if ((usLocalIntStatus & 0x1) == 0x1) {

	  	advOutp(privdata, 0x08, 0x00); 

	  	if (!(usLocalIntStatus & 0x1000)){
	  		if((( usLocalIntStatus & 0x2000 ) == 0x00) && (privdata->usFAORunning)) {             
		  		if(privdata->usAOCyclicMode) {
			  		if(privdata->ulBufIndex >= privdata->usAODMACount) {
				  		privdata->ulBufIndex = 0;
					}
					if ((privdata->pAOCommBufVA != NULL) && (privdata->UserBuffer != NULL)) {
						
						memcpy((INT8U *)privdata->pAOCommBufVA + privdata->usFIFOIndex * (privdata->usFIFOsize * 1024),
						       ((INT8U *)(privdata->UserBuffer)+ (privdata->ulBufIndex *  privdata->usFIFOsize * 1024)), 
						       (privdata->usFIFOsize * 1024));
					}
			  
			 		 privdata->ulBufIndex ++;
		  		}  else  {
					if(privdata->usAODMACount > 2) {
						if ((privdata->pAOCommBufVA != NULL) && (privdata->UserBuffer != NULL)) 
							memcpy((INT8U *)privdata->pAOCommBufVA + privdata->usFIFOIndex*( privdata->usFIFOsize * 1024 ),
							       ((INT8U *)(privdata->UserBuffer)+
								(privdata->ulBufIndex *  privdata->usFIFOsize * 1024)), 
							       (privdata->usFIFOsize * 1024));
						privdata->ulBufIndex ++;
					}
  				}




				if (privdata->usAOCyclicMode) {
					advOutpDMAdw(privdata, 0x94, 0x0000091d); 
                        
					advOutpDMAdw(privdata, 0x98, (privdata->phyAOCommBuf + (privdata->usFIFOsize * 1024 * privdata->usFIFOIndex)));
					advOutpDMAdw(privdata, 0x9c, 0x00000030);
					advOutpDMAdw(privdata, 0xa0, privdata->usFIFOsize * 1024);
					advOutpDMAdw(privdata, 0xa4, 0x00000003);
					advOutpDMA(privdata, 0xa9, 0x0b); 

					privdata->ulTrasferIndex++;    

					if (privdata->ulBufIndex >= ((ULONG)(privdata->usAODMACount))) {
						if (privdata->usAOHalfReady != 2) {
/* 							adv_process_info_set_event_all(&privdata->ptr_process_info, 5, 1); */
							privdata->sub_event |= EVENT_AO_HIBUF_OCCURRED;
							
							privdata->usAOHighTransferFlag = 1;
							privdata->usUnderrunFlag = 0;
						}
						privdata->usAOHalfReady = 2;
			   
					} else if (privdata->ulBufIndex >= ((ULONG)(privdata->usAODMACount)/2)) {
						if (privdata->usAOHalfReady != 1) {
/* 							adv_process_info_set_event_all(&privdata->ptr_process_info, 4, 1); */
							privdata->sub_event |= EVENT_AO_LOBUF_OCCURRED;
							privdata->usAOLowTransferFlag = 1; 
							privdata->usUnderrunFlag = 0; 
						}
						privdata->usAOHalfReady = 1;
					} else {
						if (privdata->ulBufIndex == 0) {
							privdata->usAOHalfReady = 0;
						}                
					}

					
					if (((privdata->usAOLowTransferFlag) && (privdata->usAOHalfReady == 2) && (!privdata->usUnderrunFlag))
					    || ((privdata->usAOHighTransferFlag) && (privdata->usAOHalfReady == 1) && (!privdata->usUnderrunFlag)))
					{
						privdata->ulUnderRun = 1;
/* 						adv_process_info_set_event_all(&privdata->ptr_process_info, 7, 1); */
							privdata->sub_event |= EVENT_AO_UNDERRUN_OCCURRED;

					}
					
					privdata->ulConverted = privdata->ulTrasferIndex * privdata->usFIFOsize * 512;

					privdata->usFIFOIndex = ~(privdata->usFIFOIndex) & 0x0001;

				} else {
					if((privdata->ulTrasferIndex < (ULONG)privdata->usAODMACount)) {
						advOutpDMAdw(privdata, 0x94, 0x0000091d); 
               
						advOutpDMAdw(privdata, 0x98, (privdata->phyAOCommBuf + (privdata->usFIFOsize * 1024 * privdata->usFIFOIndex)));
						advOutpDMAdw(privdata, 0x9c, 0x00000030);
						advOutpDMAdw(privdata, 0xa0, privdata->usFIFOsize * 1024);
						advOutpDMAdw(privdata, 0xa4, 0x00000003);
						advOutpDMA(privdata, 0xa9, 0x0b);

						privdata->ulTrasferIndex++;    
               
						if (privdata->ulBufIndex >= ((ULONG)(privdata->usAODMACount))) {
							if (privdata->usAOHalfReady != 2) {
/* 						adv_process_info_set_event_all(&privdata->ptr_process_info, 5, 1); */
							privdata->sub_event |= EVENT_AO_HIBUF_OCCURRED;
						privdata->usAOStopped = 1;
								privdata->usAOHighTransferFlag = 1;
								privdata->usUnderrunFlag = 0;
							}
							privdata->usAOHalfReady = 2;

							
						} else if (privdata->ulBufIndex >= ( (ULONG)(privdata->usAODMACount)/2)) {
							if (privdata->usAOHalfReady != 1) {
/* 						adv_process_info_set_event_all(&privdata->ptr_process_info, 4, 1); */
							privdata->sub_event |= EVENT_AO_LOBUF_OCCURRED;
								privdata->usAOLowTransferFlag = 1;
								privdata->usUnderrunFlag = 0;
							}
							privdata->usAOHalfReady = 1;
						} else {
							if (privdata->ulBufIndex == 0) {
								privdata->usAOHalfReady = 0;
							}                
						}
               
						privdata->ulConverted = privdata->ulTrasferIndex * privdata->usFIFOsize * 512;

						if(privdata->ulBufIndex >= privdata->usAODMACount)
							privdata->ulBufIndex = 0;     
						privdata->usFIFOIndex = ~(privdata->usFIFOIndex) & 0x0001;


					} else {
						privdata->usAOStopped = 1;
						privdata->usAOTerminateFlag = 1;
/* 						adv_process_info_set_event_all(&privdata->ptr_process_info, 6, 1); */
						privdata->sub_event |= EVENT_AO_TERMINATED_OCCURRED;
						privdata->ulUnderRun      = 0;
						privdata->usAOHalfReady = 2;
						privdata->ulConverted = privdata->usAODMACount * privdata->usFIFOsize * 512;
					}
				}
			}
		}else{
			if (((tmp1 & 0x4000)==0x4000) && (privdata->trig_mode==3)) {
				adv_process_info_set_event_all(&privdata->ptr_process_info, 6, 1);
				privdata->sub_event |= EVENT_AO_TERMINATED_OCCURRED;
				privdata->ulRetrieved = privdata->conv_num;
				advOutp(privdata,0x08, 0x00);
				tmp = tmp|0x08;
				advOutpDMA(privdata, 0xa8, tmp);
			}
		}
	}



	tasklet_schedule(&privdata->tasklet);

	if (waitqueue_active(&privdata->event_wait)){
		wake_up_interruptible(&privdata->event_wait);
	}
	
	return IRQ_RETVAL(1);
}	 


