/****************************************************************************
 *		Copyright 2006 IAG Software Team, 
 *
 *			BeiJing R&D Center 
 *
 *			Advantech Co., Ltd.
 *
 *	Advantech PCI-1712 Device driver for Linux
 *
 * File Name: 
 *	PCI1712.c
 * Abstract:
 *	This file contains routines for ai function.
 * Version history
 *      09/25/2006              Created by zhenyu.zhang
 ************************************************************************/	
#define __NO_VERSION__
#include "PCI1712.h"


INT32S AdvStartReadOnDevice(adv_device *device) 
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U tmp = 0;

	/* initialize PCI9054 */
	advOutpDMAw(privdata, 0x04, 0x0001);
	advOutpDMAdw(privdata, 0x08, 0x002d0000);
	
	if(privdata->usFAORunning == 0){
	   	if( privdata->trig_mode == 3 ){
		   	advOutpDMAw(privdata, 0x68, 0x0900);
	   	}else{
	      		advOutpDMAw(privdata, 0x68, 0x0100);
	   	}
		   
	   	advOutpDMAw(privdata, 0x6a, 0x0004);
	}

	advOutpDMAw(privdata, 0x18, 0x000d);
	advOutpDMAw(privdata, 0xb0, 0x0000);

	if( privdata->trig_mode == 3 ){
		advOutpDMAdw(privdata, 0x80, 0x00029e09);
	}else{
		advOutpDMAdw(privdata, 0x80, 0x00021f0d);
	}       
	advOutpDMAdw(privdata, 0x90, privdata->phySGLHeader | 0x0d) ;
	advOutpDMA(privdata, 0xa8, 0x09);
	advOutpw(privdata, 0x06, 0x0000);

	if( privdata->trig_mode == 3 || privdata->trig_mode== 2 )	{
	    	advOutpw( privdata, 0x16, 0xb0 ); // 10110000  Mode 0

	      	advOutp(privdata,0x14, (INT8U)( privdata->delay_cnt & 0x00ff) );
	    	advOutp(privdata, 0x14, (INT8U)( (privdata->delay_cnt & 0xff00) >> 8) );
	}

	switch ( privdata->trig_mode )	{
	case 0: // Pacer  
		tmp = 0x0001;  
		break;
	case 1: // Post-trigger
		tmp = 0x0002; 
		break;
	case 2: // Delay trigger
		tmp = 0x2003; 
		break;
	case 3: // Aout trigger, external clock from pin AI_CLK
		tmp = 0x2004;
		break;
	}

	if(privdata->clk_src)
		tmp |= 0x0008;

	if(privdata->src_type)	{
		advOutpw( privdata, 0x2c, (USHORT)(0x0a00 | privdata->trig_vol));
	    	tmp |= 0x0010;
	}

	if(privdata->trig_edge)
		tmp |= 0x0040;

	if(!privdata->trig_src)
		tmp |= 0x8000;

	advOutpw( privdata, 0x08, 0x8000); //yingsong modify this line to prevent clear the Fifo of DA
	advOutpDMA(privdata, 0xa8, 0x0b);

	// Start DMA
	advOutpw( privdata, 0x16, 0x0076 ); // 1110110
	advOutpw( privdata, 0x12, (INT16U)( privdata->divisor1 & 0x00ff) );
	advOutpw( privdata, 0x12, (INT16U)( (privdata->divisor1 & 0xff00) >> 8) );
	advOutpw( privdata, 0x06, tmp );
	
	privdata->usFAIRunning = 1;
	privdata->AIHalfFullCnt = 0;
	privdata->aiHalfReadyFlag = 0;

	return SUCCESS;
	
}



/**
 * adv_init_dma - initialize the bus-master dma
 *
 * @device: Points to the device object
 * @user_buf: user buffer address
 * @data_cnt: user buffer length in byte
 *
 * 1. get user page
 * 2. build scatter-gather list
 * 3. setup 9056 DMA register
 */
static INT32S adv_init_dma(adv_device *device, INT32U *user_buf, INT32U data_cnt)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_user_page *user_pages; /* top half buffer */
	INT32U sgl_size;
	INT32U sgl_cnt = 0;
	INT32U sgl_phyaddr;
	ULONG *sgl_addr;
	INT32U page_num; /* SGL total descriptor numbers in top buffer */
	INT32U virt_addr;
	INT32U phy_addr;
	INT16U i = 0;
	
	INT32U cur_len=0;
	INT32U top_len;		/* top half page length */
	INT32U bot_len;		/* bottom half page length */
	INT16U j=0;
	INT32U total_len = 0;
	switch (privdata->device_type) {
	case PCI1712S:
	case PCI1712L:
		/* pci-1721 use 12-bits data width */
		if ((data_cnt / 2) % 4) { /* We can ensure that top half buffer length is a
					   * multiple of 2, and bottom half buffer is a mu-
					   * ltiple of 2.
					   * 
					   * Top half buffer must same as bottom half buffer,
					   * so we need to ensure that any half buffer length
					   * is a multiple of 4 */
			return -EFAULT;
		}

		break;
	default:
		return -EFAULT;
	}

	
	/* get top half buffer pages */
        adv_map_user_pages(&user_pages,
			   &page_num,
			   (PTR_T) user_buf,
			   (PTR_T) (data_cnt),
			   WRITE);
	if (user_pages == NULL) {
		return -ENOMEM;
	}



	/* build SGL table */
	sgl_size = (page_num + 1) * 16; /* the size of each descriptor is 16 bytes
					 * note: the page_num must add '1', because
					 * we may be encounter the problem that page
					 * is unaligned */
	sgl_addr = kzalloc(sgl_size, GFP_DMA);
	if (!sgl_addr) {
		return -ENOMEM;
	}
	sgl_phyaddr = virt_to_phys((void *) sgl_addr);
	privdata->pVaSGLHeader = sgl_addr;
	privdata->phySGLHeader = sgl_phyaddr;

	
	
	for (j = i = 0; i < page_num; i++, j++) {
		virt_addr = user_pages[i].page_addr + user_pages[i].offset;
		phy_addr = virt_to_phys((void *) virt_addr);
		cur_len = user_pages[i].length;

		
		
		if (((sgl_cnt + 1) * 16) > sgl_size) {
			return -EFAULT;
		}

		sgl_addr[j * 4] = phy_addr;
		sgl_addr[j * 4 + 1] = 0x0000;
		sgl_addr[j * 4 + 2] = cur_len;
		sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x09);

		

		if (total_len + cur_len == privdata->conv_num) {
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x0d);
		}

		/* page unalign case */
		if ((total_len < privdata->conv_num)
		    && (total_len + cur_len > privdata->conv_num)) {

			/* for top half page, cause interrupt */
			top_len = privdata->conv_num - total_len;

			sgl_addr[j * 4] = phy_addr;
			sgl_addr[j * 4 + 1] = 0x0000;
			sgl_addr[j * 4 + 2] = top_len;
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x0d);


			j += 1;
			sgl_cnt++;

			/* for bottom half page */
			bot_len = cur_len - top_len;
			virt_addr = user_pages[i].page_addr + user_pages[i].offset + top_len;
			phy_addr = virt_to_phys((void *) virt_addr);
			sgl_addr[j * 4] = phy_addr;
			sgl_addr[j * 4 + 1] = 0x0000;
			sgl_addr[j * 4 + 2] = bot_len;
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x09);

		}

		


		sgl_cnt++;
		total_len += cur_len;
		
		
	}
	
	if ((privdata->trig_mode == 3) || privdata->cyclic) {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) (sgl_phyaddr | 0x0d);
	} else {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) ((sgl_phyaddr+i*16) | 0x0f);
	}

	

	privdata->sgl_addr = sgl_addr;
	privdata->sgl_cnt = sgl_cnt;
	privdata->page_num = page_num;
	privdata->user_pages = user_pages;

	AdvStartReadOnDevice(device);
	
	return 0;
}

INT32S FAIDmaStart(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32S ret = 0;
	INT16U tmp;

	
	tmp = advInpDMA(privdata,0xa8) & 0x10;
	if(tmp != 0x10) {
		advOutpDMAw( privdata, 0x04, 0x0001 );
		advOutpDMAdw( privdata, 0x08, 0x002d0000 );
		advOutpDMAw( privdata, 0x18, 0x000d );
		advOutpDMAw( privdata, 0xb0, 0x0000 );
	
		advOutpDMAw( privdata, 0x80, 0x090d );
		advOutpDMAw( privdata, 0x82, 0x0000 );  
   		advOutpDMAdw( privdata, 0x84, privdata->phyAddrBlock);

		advOutpDMAdw( privdata, 0x88, 0x00000000 );// DMA Channel 0 PCI Address Reg
		advOutpDMAdw( privdata, 0x8C, 0x00000080 );
		advOutpDMAw( privdata, 0x90, 0x000b );
		advOutpDMAw( privdata, 0x92, 0x0000 );  // DMA Channel0 Descripter Pointer Reg
		advOutpDMAw(privdata, 0xa8, 0x0009);
		advOutpDMAw(privdata, 0xa8, 0x000b);
		tmp = advInpDMA(privdata,0xa8) & 0x10;
	
		while(tmp != 0x10){
			tmp = advInpDMA(privdata,0xa8) & 0x10;
			}
	}

	spin_lock(&privdata->spinlock);
	
        /* init Interrupt mode ai*/
	ret = adv_init_dma(device,
			   (INT32U *) privdata->user_buf,
			   privdata->conv_num * sizeof(INT16U));
	if (ret) {
		spin_unlock(&privdata->spinlock);
		return ret;
	}
	spin_unlock(&privdata->spinlock);

	return 0;
}



/**
 * adv_fai_dma_start - DMA mode AI start function
 *
 * @denice: Points to the device object
 * @arg: Argument from user
 */
INT32S adv_fai_dma_start(adv_device *device, void* arg)
{
	
	private_data *privdata = (private_data *) (device->private_data);
	adv_dma_user int_data;
	
	if (copy_from_user(&int_data, arg, sizeof(adv_dma_user))) {
	  	return -EFAULT;
     	}

	privdata->divisor1 = int_data.divisor1;
	privdata->sample_rate = int_data.samplerate;
	privdata->user_buf = int_data.userptr;
	privdata->conv_num = int_data.datacnt;
	privdata->cyclic = int_data.cyclic;
	privdata->trig_mode = int_data.trig_mode;
	privdata->trig_src = int_data.trig_src;
	privdata->trig_vol = int_data.trigvol;
	privdata->clk_src = int_data.clocksrc;
	privdata->src_type = int_data.src_type;
	privdata->delay_cnt = int_data.delay_cnt;
	privdata->trig_edge = int_data.trig_edge;


	privdata->usAITerminateFlag = 0;
	privdata->ulRetrieved = 0;
	privdata->usAIStopped = 0;
	privdata->usAOStopped = 0;
	privdata->ulIntcount = 0;

	
	FAIDmaStart(device);

	

	return 0;

}

VOID adStopADDma(private_data *privdata)
{
	INT8U  control;
	ULONG  tmp;
	INT8U  uctmp6a;
	
    if(privdata->usFAIRunning) {
	    advOutpw(privdata, 0x06, 0x0000);
	    if (!privdata->usFAORunning){
		    advOutpw(privdata, 0x08, 0x0000);
		    uctmp6a = advInpDMA(privdata, 0x6a);
		    uctmp6a = uctmp6a & 0xfb;
		    advOutpDMA(privdata, 0x6a, uctmp6a);
		    advOutpDMAw(privdata, 0x68, 0x0000);
	    }
		
	    tmp = advInpDMAdw(privdata, 0x80);
	    advOutpDMAdw(privdata, 0x80, (ULONG)(tmp & 0xfffffbff));
		
	    if((0 == privdata->cyclic) && (1 == privdata->usAITerminateFlag)){
			
		    control = advInpDMA(privdata, 0xa8);
		    advOutpDMA(privdata, 0xa8, (INT8U)(control & 0xfe)); 
	    }else{
		    control = advInpDMA(privdata, 0xa8);
		    advOutpDMA(privdata, 0xa8, (INT8U)(control & 0xfe));
				
			
	    }
    }
}


INT32S adFAITerminate(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);

	printk(KERN_DEBUG "==> Ads1712s adFAITerminate()\n");
	if (privdata->usFAIRunning) {
		adStopADDma(privdata);
		privdata->usFAIRunning = 0;
		privdata->usAIStopped = 1;
		if (!privdata->usAITerminateFlag){
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);

			privdata->usAITerminateFlag = 1;
		}


		if (privdata->user_pages) {
			adv_unmap_user_pages(privdata->user_pages,
					     privdata->page_num,
					     1);
			privdata->user_pages = NULL;
		}
	
		if (privdata->sgl_addr) {
			kfree(privdata->sgl_addr);
			privdata->sgl_addr = NULL;
		};
	}
	

	privdata->user_pages = 0;
	privdata->sgl_addr = 0;
	privdata->usFAIRunning = 0;

	printk(KERN_DEBUG "<== Ads1712s adFAITerminate()\n");

	return SUCCESS;
}




INT32S adClearUnderrun(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	ULONG ulEventType;


	if (copy_from_user(&ulEventType, arg, sizeof(ULONG))) {
	  	return -EFAULT;
     	}


	if (ulEventType == ADS_EVT_AO_LOBUF_TRANSFERED)
		privdata->usAOLowTransferFlag = 0;
	if (ulEventType == ADS_EVT_AO_HIBUF_TRANSFERED)
		privdata->usAOHighTransferFlag = 0;
	if (ulEventType == ADS_EVT_AO_UNDERRUN)
		privdata->ulUnderRun = 0;
	
	
	return 0;
}

INT32S adClearFlag(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	ULONG et;


	if (copy_from_user(&et, arg, sizeof(ULONG))) {
	  	return -EFAULT;
     	}



	switch(et) {
	case ADS_EVT_AO_LOBUF_TRANSFERED:
		privdata->usAOLowTransferFlag = 0;
		break;
	case ADS_EVT_AO_HIBUF_TRANSFERED:
		privdata->usAOHighTransferFlag = 0;
		break;
	case ADS_EVT_AO_UNDERRUN:
		privdata->ulUnderRun = 0;
		break;
	case ADS_EVT_AI_LOBUFREADY:
		privdata->aiHalfReadyFlag = 0;
		break;
	case ADS_EVT_AI_HIBUFREADY:
		privdata->aiHalfReadyFlag = 0;
		break;
	case ADS_EVT_AI_OVERRUN:
		privdata->usOverrun = 0;
		break;
	default:
		return -EINVAL;
	}
	
	
	return 0;
}


INT32S adFAICheck(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	FAICheckInfo checkInfo;
 

	if (copy_from_user(&checkInfo, arg, sizeof(checkInfo))) {
		return -EFAULT;
	}

        checkInfo.usStopped   = privdata->usAIStopped;
        checkInfo.ulRetrieved = privdata->ulRetrieved;
        checkInfo.usOverrun   = 0; 
        checkInfo.usHalfReady = privdata->usAIHalfReady;
        checkInfo.Status      = 0;

	if (copy_to_user(arg, &checkInfo, sizeof(checkInfo))) {
		return -EFAULT;
	}

	printk(KERN_DEBUG"<== Ads1712s adFAICheck()\n");

	return SUCCESS;
}


INT32S adFAITransfer(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	FAITransferInfo tranInfo;
 
	printk(KERN_DEBUG "==> Ads1712s adFAITransfer()\n");

	if (copy_from_user(&tranInfo, arg, sizeof(tranInfo))) {
		return -EFAULT;
	}

	privdata->usOverrun = 0;
	tranInfo.Status = 0;

	if (copy_to_user(arg, &tranInfo, sizeof(tranInfo))) {
		return -EFAULT;
	}


	printk(KERN_DEBUG"<== Ads1712s adFAITransfer()\n");

	return SUCCESS;
}
