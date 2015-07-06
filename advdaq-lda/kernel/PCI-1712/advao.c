/********************************************************************
 *				                                   
 * 		Copyright 2006 IAG Software Team,                  
 *                                                                  
 * 			BeiJing R&D Center                         
 *	                                                           
 * 			Advantech Co., Ltd.                        
 *                                                                  
 * 	    Advantech PCI-1712 Series Device Driver for Linux             
 *                                                                  
 *                                                                  
 *                                                                  
 * 	File Name:                                                 
 * 		advao.c                                            
 * 	Abstract:                                                  
 * 		This file contains routines for ao function.
 * 	Note:
 * 	        adv_gp_fun() -general purpose function
 *                                                                  
 *                                                                  
 * 	Version history                                            
 * 	09/21/2006			Create by zhenyu.zhang         
 *                                                                  
 ********************************************************************/
#include "PCI1712.h"



INT32S adFAOLoad(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	FAOLoadInfo TranInfo;
	USHORT   i;
	


	if (copy_from_user(&TranInfo, arg, sizeof(TranInfo))) {
		return -EFAULT;
	}

	privdata->usOverrun = 0;
	privdata->ulBufIndex = 0;
	privdata->usFIFOIndex = 0;

	privdata->Buffer = (USHORT *)(TranInfo.Buffer);
	privdata->ulStartpoint = TranInfo.ulStartNum;
	privdata->ulCount = TranInfo.ulCount;
  
	advOutpw(privdata, 0x0a, (USHORT)(advInpw(privdata, 0x0a) & 0x01ff));
	
	for(i = 0; i < 4096; i++) {
		advOutpw(privdata, 0x30, 0x0001);
	}
	
    
	if( (advInpw(privdata, 0x08) & 0x6000) == 0x6000) {
		privdata->usAODMACount	= (USHORT)(TranInfo.ulCount/2048);
	} else {
		privdata->usAODMACount	= (USHORT)(TranInfo.ulCount/16384);
	}

	advOutpw(privdata, 0x0a, (USHORT)(advInpw(privdata, 0x0a) & 0x00ff));

	advOutpw( privdata,0x08,(USHORT)0x0800 );


	if ((privdata->UserBuffer != NULL) && (privdata->Buffer != NULL)) {
		spin_lock(&privdata->spinlock);
		memcpy(privdata->UserBuffer, privdata->Buffer, privdata->UserBufferCount);
		spin_unlock(&privdata->spinlock);
	}

	TranInfo.Status = 0;


    return 0;
}

INT32S adFAOLoadEx(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	FAOLoadInfo TranInfo;


	if (copy_from_user(&TranInfo, arg, sizeof(TranInfo))) {
		return -EFAULT;
	}
	
	privdata->Buffer = (USHORT *)(TranInfo.Buffer);

	if ((privdata->UserBuffer != NULL) && (privdata->Buffer != NULL)){ 
		spin_lock(&privdata->spinlock);
		memcpy((INT8U *)privdata->UserBuffer + TranInfo.ulStartNum, privdata->Buffer , TranInfo.ulCount * 2);
		spin_unlock(&privdata->spinlock);
	}
	
	TranInfo.Status = 0;
	
	
	return 0;
}


INT32S adFAOCheck(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	FAOCheckInfo checkInfo;
   
	if (copy_from_user(&checkInfo, arg, sizeof(checkInfo))) {
		return -EFAULT;
	}

   
	checkInfo.usStopped   = privdata->usAOStopped;
	checkInfo.ulConverted = privdata->ulConverted;
	checkInfo.usUnderrun  = (USHORT)privdata->ulUnderRun;
	checkInfo.usHalfReady = privdata->usAOHalfReady;
	checkInfo.Status      = 0;

	if (copy_to_user(arg, &checkInfo, sizeof(checkInfo))) {
		return -EFAULT;
	}

   
   
   return SUCCESS;
}

static void *adv_vmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;

	/* Round it off to PAGE_SIZE */
	size = PAGE_ALIGN(size);

	mem = vmalloc_32(size);
	if (!mem) {
		return NULL;
	}
	

	memset(mem, 0, size);	/* Clear the ram out, no junk to the user */
	adr = (unsigned long) mem;

	while ((long)size > 0) {
/* 		SetPageReserved(vmalloc_to_page((void *)adr)); */
		SetPageLocked(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	return mem;
}

static void adv_vfree(void *mem, unsigned long size)
{
	unsigned long adr;

	if (!mem)
		return;

	size = PAGE_ALIGN(size);

	adr = (unsigned long) mem;
	while ((long)size > 0) {
/* 		ClearPageReserved(vmalloc_to_page((void *)adr)); */
		ClearPageLocked(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
}


INT32S adAllocateDmaBuffer(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U  tmp,i;
	AllocateDmaBufferInfo AllocateInfo;


	if (copy_from_user(&AllocateInfo, arg, sizeof(AllocateInfo))) {
	  return -EFAULT;
     }

	

	 privdata->usAOCyclicMode = AllocateInfo.usCyclicMode;

	 AllocateInfo.Status = 0;

	 privdata->pAOCommBufVA = NULL;
	 privdata->UserBuffer = NULL;
	 privdata->UserBufferCount = AllocateInfo.ulReqBufSize;

	 advOutpw(privdata, 0x0a, (INT16U)(advInpw(privdata, 0x0a) & 0x01ff));
	 for(i = 0; i < 4096; i++) {
		 advOutpw(privdata, 0x30, 0x0001);
	 }
	 if((advInpw(privdata, 0x08) & 0x6000) == 0x6000) {
		 privdata->pAOCommBufVA = kzalloc(2 * PAGE_SIZE, GFP_DMA);
		 privdata->phyAOCommBuf = virt_to_phys(privdata->pAOCommBufVA);
		 
		 
		 if (privdata->pAOCommBufVA == NULL) {
			 return -ENOMEM;
		 }

		 tmp = 4;
	 } else {
		 privdata->pAOCommBufVA = kzalloc(16 * PAGE_SIZE, GFP_DMA);
		 privdata->phyAOCommBuf = virt_to_phys(privdata->pAOCommBufVA);

		 if (privdata->pAOCommBufVA == NULL) {
			 return -ENOMEM;
		 }

		 tmp = 32;
	 }


	 advOutpw(privdata, 0x0a, (INT16U)(advInpw(privdata, 0x0a) & 0x00ff));
	 advOutpw(privdata, 0x08, 0x0800);


	 privdata->user_buffer_len = AllocateInfo.ulReqBufSize;
/* 	 privdata->UserBuffer = vmalloc(AllocateInfo.ulReqBufSize); */
	 privdata->UserBuffer = adv_vmalloc(AllocateInfo.ulReqBufSize);
	 if(!privdata->UserBuffer) {
		 kfree(privdata->pAOCommBufVA);
		 
		 return -EFAULT;
		 
	 } 
	 AllocateInfo.pBuffer      = privdata->UserBuffer;
	 AllocateInfo.ulActBufSize = AllocateInfo.ulReqBufSize;
	if (copy_to_user(arg, &AllocateInfo, sizeof(AllocateInfo))) {
		return -EFAULT;
	}

	 return 0;
}



INT32S adFAODmaStart(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	FAODmaStartInfo   DmaInfo;
	USHORT		i, tmp;



	if (copy_from_user(&DmaInfo, arg, sizeof(DmaInfo))) {
		return -EFAULT;
	}


	privdata->usExtTrig          = DmaInfo.usExtTrig;
	privdata->divisor1         = DmaInfo.usDivisor1;
	privdata->usControl          = DmaInfo.usControl;

	privdata->usAOTerminateFlag  = 0;
	privdata->usAOStopped          = 0;
	privdata->usAOHalfReady        = 0;
	privdata->usFIFOIndex        = 0;
    

	privdata->ulConverted        = 0;
	privdata->ulUnderRun         = 0;

	privdata->ulUnderRun = 0;
	privdata->usAOLowTransferFlag = 0;
	privdata->usAOHighTransferFlag = 0;
	privdata->sub_event = EVENT_NONE;
	
    
	advOutpDMAdw(privdata, 0x08,   0x002d0000);
	advOutpDMAw(privdata, 0x018, 0x000d); 

	tmp = advInpDMA(privdata, 0xa9) & 0x10;
	if (tmp != 0x10) {
		advOutpDMAdw( privdata, 0x94, 0x0000091d ); //DMA1 mode
		advOutpDMAdw( privdata, 0x98, privdata->phyAddrBlock);
		advOutpDMAdw( privdata, 0x9c, 0x00000030 );
		advOutpDMAdw( privdata, 0xa0, 0x00000080 );
		advOutpDMAdw( privdata, 0xa4, 0x00000003 );
		
		advOutpw(privdata, 0x08, 0x0800); // clear DA fifo
		
		advOutpDMA( privdata, 0xa9, 0x01 ); 
		tmp = advInpDMA(privdata, 0xa9);
		advOutpDMA(privdata, 0xa9, (INT8U)(tmp|0x02));
		
		do {
			tmp = advInpDMA(privdata, 0xa9) & 0x10;
			advOutpw(privdata, 0x08, 0x0800); // clear DA fifo
		} while(tmp != 0x10);
		
		advOutpw(privdata, 0x08, 0x0800); // clear DA fifo
		
	}

	advOutpDMAw(privdata, 0x068, 0x0900); 
	advOutpDMAw(privdata, 0x06a, 0x0004); 
	
	advOutpw(privdata, 0x16, 0x0036);
	advOutpw(privdata, 0x10, (USHORT)(privdata->divisor1 & 0x00ff));	
	advOutpw(privdata, 0x10, (USHORT)((privdata->divisor1 & 0xff00)>>8));	
	
	advOutpw(privdata, 0x08, 0x0800);   // Don't clear AD fifo.
	advOutpw( privdata,0x0a,(USHORT)((advInpw(privdata, 0xa) & 0xff00) | privdata->usControl ) ); 
	
	for(i = 0; i < 4096; i++){
		advOutpw(privdata, 0x30, *(privdata->Buffer+i));
		if (2 == privdata->usAODMACount){
			if (2048 == i){
				adv_process_info_set_event_all(&privdata->ptr_process_info, 4, 1);
				privdata->usAOHalfReady = 1;
			}
			if (4095 == i) {
				adv_process_info_set_event_all(&privdata->ptr_process_info, 5, 1);
				privdata->usAOHalfReady = 2;
			}	
		}
	}
   
	if((advInpw(privdata, 0x08) & 0x6000) == 0x6000) {
		privdata->usFIFOsize = 4;
	} else {
		advOutpw(privdata, 0x08, 0x0800);	//don't clear AD fifo
		privdata->usFIFOsize = 32;

		for(i=0; i<32768; i++){
			advOutpw(privdata, 0x30, *(privdata->Buffer+i));
			if (2 == privdata->usAODMACount){
				if (16384 == i){
					adv_process_info_set_event_all(&privdata->ptr_process_info, 4, 1);
					privdata->usAOHalfReady = 1;
				}
				if (32767 == i) {
					adv_process_info_set_event_all(&privdata->ptr_process_info, 5, 1);
					privdata->usAOHalfReady = 2;
					
				}
			}
		}
	}


	privdata->ulBufIndex = 2;
	privdata->ulTrasferIndex = 2;

	if (privdata->usExtTrig) {
		advOutpw( privdata,0x0a,(USHORT)(advInpw(privdata, 0xa) & 0x7fff) );
	} else {
		advOutpw(privdata,0x0a,(USHORT)(advInpw(privdata, 0xa) | 0x8000));
	}  
	privdata->usFAORunning = 1;


	return 0;
}


VOID adStopDADma(private_data *privdata)
{
	INT8U  control;
	ULONG  tmp = 0;

	if (privdata->usFAORunning) {
		advOutpw(privdata,0x08,(INT16U)0x8800);
		if (!privdata->usFAIRunning){
			advOutpDMAw(privdata, 0x68, 0x0000);
		}

		tmp = advInpDMAdw(privdata, 0x94);
		advOutpDMAdw(privdata, 0x94, (ULONG)(tmp & 0xfffffaff));
		
		tmp = advInpDMAw(privdata, 0x68);
		advOutpDMAw(privdata, 0x68, (INT16U)( tmp & 0xf7ff));
		
		control = advInpDMA(privdata, 0xa9);
		advOutpDMA(privdata, 0xa9, (INT8U)(control & 0xfe)); 

	}
}


INT32S adFAOTerminate(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	   
	if(privdata->usFAORunning){
		adStopDADma(privdata);
		privdata->usFAORunning = 0;
		privdata->usAOStopped = 1;
		if (!privdata->usAOTerminateFlag){
			adv_process_info_set_event_all(&privdata->ptr_process_info, 6, 1);
			privdata->usAOTerminateFlag = 1;
		}

	}
	return 0;
}

INT32S adFAOStop(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
    



	if(privdata->usFAORunning){

		adStopDADma(privdata);
    
		privdata->usFAORunning = 0;	 
		privdata->usAOStopped = 1;
		 
	}


    return 0;
}




INT32S adFreeDmaBuffer(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);

	
	
	
	if (privdata->pAOCommBufVA) {
		kfree(privdata->pAOCommBufVA);
		privdata->pAOCommBufVA = NULL;
	}

	if(privdata->UserBuffer) {
		adv_vfree(privdata->UserBuffer, privdata->user_buffer_len);
		privdata->UserBuffer = NULL;
	}

	return 0;
}
