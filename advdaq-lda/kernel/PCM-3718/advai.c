/****************************************************************************
 *		Copyright 2006 IAG Software Team, 
 *
 *			BeiJing R&D Center 
 *
 *			Advantech Co., Ltd.
 *
 *	Advantech PCI-1716 Device driver for Linux
 *
 * File Name: 
 *	PCI1716.c
 * Abstract:
 *	This file contains routines for ai function.
 * Version history
 *      09/25/2006              Created by zhenyu.zhang
 ************************************************************************/	
#define __NO_VERSION__
#include "PCM3718.h"

static void adv_pacer_setup(private_data *privdata);
static int adSetupInt ( private_data *pDevExt )
{
   UCHAR CtrlByte;


   // Initialize some value


   advOutp( pDevExt, 0x08, 0 );    // clear A/D convert interrupt

   // jiqi add 17/8/2004
   if ( pDevExt->int_cnt == 1 )
   {
      advOutp( pDevExt,  6, 0 );            //disable FIFO interrupt      
   }
   else
   {
      advOutp( pDevExt,  6, 1 );    // enable FIFO interrupt
      advOutp( pDevExt, 25, 0 );    // clear all data in FIFO
      advOutp( pDevExt, 20, 0 );    // clear interrupt request with FIFO
   }

   // setup internal trigger with interrupt transfer

   if ( pDevExt->int_cnt == 1 )
   {
      CtrlByte = (USHORT)pDevExt->IntChan << 4;
   }
   else
   {
      CtrlByte = (USHORT)0;   // interrupt with FIFO
   }
   // jiqi end

   // jiqi mask 17/8/2004
   //CtrlByte = pDevExt->IntChan << 4;

   if ( pDevExt->clk_src )     // external trig
   {
      CtrlByte |= 0x82;
   }
   else  // pacer trig
   {
      CtrlByte |= 0x83;
   }

      CtrlByte = 0xb3;
   advOutp( pDevExt, 9, CtrlByte ); // enable A/D convert interrupt
   //advOutp( pDevExt, 8, 0x20 ); // enable A/D convert interrupt
   //advOutp( pDevExt, 9, 0xb3 ); // enable A/D convert interrupt

   if ( !pDevExt->clk_src )
   {
     // adPacerSetup( pDevExt );     // Setup Pacer generate Frequency
 	adv_pacer_setup(pDevExt);
   }
   return 0;
}
/**
 * adv_pacer_setup - set 82C54 counter                            
 *
 * @privdata: Pointe to the device private data
 * @rate: gate for counter(open/close counter)
 * 
 * step1. set control register                  
 * step2. set counter1 & counter2 to mode 2     
 * step3. write counter 1                       
 *        (low byte & high byte)                
 * step4. write counter 2                       
 *        (low byte & high byte)                
 *                                                            
 * 0x1a: counter1
 * 0x1c: counter2
 */
static void adv_pacer_setup(private_data *privdata)
{
       INT8U temp;
    
       advOutp(privdata,10,0);    /* disable pacer.                     */
    advOutp(privdata,15,0xb4); /* Set counter 2 to mode 2, rate gen. */

       temp = privdata->divisor1& 0xff;
       advOutp(privdata, 14, temp); /* Write counter 2 low byte  */
       temp = privdata->divisor1 >> 8 & 0xff;
       advOutp(privdata, 14, temp); /* Write counter 2 high byte  */
    	
       // Joshua.Lan added here 04/25/2002
    	advOutp(privdata,15,0x74); /* Set counter 1 to mode 2, rate gen.  */
       temp = privdata->divisor2 & 0xff;
       advOutp(privdata, 13, temp); /* Write counter 1 low byte  */
       temp = privdata->divisor2 >> 8 & 0xff;
       advOutp(privdata, 13, temp); /* Write counter 1 high byte  */
    
       return SUCCESS;
 	
}


/**
 * adv_check_dma - check DMA status until the DMA transfer terminated
 *
 * @device: Points to the device object
 */
static INT32S adv_check_dma(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U tmp;


	tmp = advInpDMA(privdata, 0xa8) & 0x10;
	if (tmp != 0x10) {
		tmp = advInpDMA(privdata, 0xa8) & 0x10;
		while (tmp != 0x10) {
			tmp = advInpDMA(privdata, 0xa8) & 0x10;
		}
		advOutpw(privdata, 0x08, 0x00);		
	}
	
	return 0;
}

unsigned long dma_mem_alloc(int size)
{
	int order = get_order(size);
	return __get_dma_pages(GFP_KERNEL,order);
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
 * 3. setup 9054 DMA register
 */
static INT32S adv_init_dma(adv_device *device, INT32U *user_buf, INT32U data_cnt)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_user_page *user_pages;
	INT32U sgl_size,flags;
	INT32U sgl_cnt = 0;
	INT32U sgl_phyaddr;
	INT32U *sgl_addr;
	INT32U page_num; /* SGL total descriptor numbers in buffer */
	INT32U virt_addr;
	INT32U phy_addr;
	INT16U i,ret;
	VOID *tmp;
    	INT8U usControlReg;
	
	switch (privdata->device_type) {
	case PCM3718:
		if ((data_cnt / 2) % 4) { /* We can ensure that top half buffer length is a
					   * multiple of 2, and bottom half buffer is a mu-
					   * ltiple of 2.
					   *
					   * Top half buffer must same as bottom half buffer,
					   * so we need to ensure that any half buffer length
					   * is a multiple of 4 */
			return -EINVAL;
		}

		break;
	default:
		return -EINVAL;
	}
	/* get user buffer pages */
        adv_map_user_pages(&user_pages,
			   &page_num,
			   (PTR_T) user_buf,
			   (PTR_T) data_cnt,
			   WRITE);
	if (!user_pages) {
		return -EFAULT;
	}
	/* build SGL table 
	sgl_size = page_num * 16; 
	sgl_addr = kmalloc(sgl_size, GFP_KERNEL);
	if (!sgl_addr) {
		printk("Allocate sgl list error\n");
		return -ENOMEM;
	}
	sgl_phyaddr = virt_to_phys((void *) sgl_addr);

	for (i = 0; i < page_num; i++) {
		virt_addr = user_pages[i].page_addr + user_pages[i].offset;
		phy_addr = virt_to_phys((void *) virt_addr);

		if (((sgl_cnt + 1) * 16) > sgl_size) {
			printk("sgl table overflow !\n");
			return -EFAULT;
		}

		sgl_addr[i * 4] = phy_addr;
		sgl_addr[i * 4 + 1] = 0x0000;
		sgl_addr[i * 4 + 2] = user_pages[i].length;
		sgl_addr[i * 4 + 3] = (INT32U) ((sgl_phyaddr + ((i + 1) * 16)) | 0x09);

		if (i == page_num / 2) { 
			sgl_addr[i * 4 + 3] = (INT32U) ((sgl_phyaddr + ((i + 1) * 16)) | 0x0d);
		}
		
		sgl_cnt++;

		memset((void *) virt_addr, 0x5a, sizeof(INT8U) * 4);
	}
	
	if (privdata->cyclic) {
		sgl_addr[(i - 1) * 4 + 3] = (INT32U) (sgl_phyaddr | 0x0d);
	} else {
		sgl_addr[(i - 1) * 4 + 3] = (INT32U) ((sgl_phyaddr + (i * 16)) | 0x0f);
	}
*/

	privdata->dmabuf = kmalloc(privdata->conv_num*sizeof(INT16U),GFP_DMA);
	if (!privdata->dmabuf) {
		printk(", unable to allocate DMA buffer, FAIL!\n");
		return -EBUSY;
	}
	memset(privdata->dmabuf,0,privdata->conv_num*sizeof(INT16U));
	privdata->dmapages[0]=2;
	privdata->hwdmaptr = virt_to_bus((void *)privdata->dmabuf);
	privdata->hwdmasize[0]=privdata->conv_num*sizeof(INT16U);


        flags=claim_dma_lock();
        disable_dma(privdata->ioDMAbase);
	clear_dma_ff(privdata->ioDMAbase);
        set_dma_mode(privdata->ioDMAbase, DMA_MODE_READ);
        //set_dma_mode(privdata->ioDMAbase, DMA_MODE_READ|DMA_AUTOINIT);
        set_dma_addr(privdata->ioDMAbase, privdata->hwdmaptr);
        set_dma_count(privdata->ioDMAbase, privdata->conv_num*sizeof(INT16U));//hwdmasize[0]);
        enable_dma(privdata->ioDMAbase);
	ret = get_dma_residue(privdata->ioDMAbase);
printk("get dma residue ret=%x\n",ret);
        release_dma_lock(flags);



    	// setup internal trigger with interrupt transfer
 	usControlReg = (USHORT)privdata->IntChan << 4;
    
	if (privdata->clk_src)    {
		 usControlReg = usControlReg | 0x86;
    	} else    {
		 usControlReg = usControlReg | 0x87;
	}

      usControlReg |= privdata->irq<<4;
	advOutp( privdata, 9, usControlReg );    // enabled pacer trigger
	advOutp( privdata, 8, 0 );               // clear interrupt


    	if (!privdata->clk_src)    {
		 adv_pacer_setup(privdata);
	}
	
	privdata->irqen = usControlReg;
	privdata->sgl_addr = sgl_addr;
	privdata->sgl_cnt = sgl_cnt;
	privdata->page_num = page_num;
	privdata->user_pages = user_pages;


	return 0;

}

/**
 * adv_fai_dma_start - DMA mode AI start function
 *
 * @device: Points to the device object
 * @arg: Argument from user
 */
INT32S adv_fai_dma_start(adv_device *device, void* arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_dma_user fai_dma_ex_start;
	INT16U start_chan;
	INT16U num_chan;
	INT16U tmp;
	INT16U *gain_list;

	INT32S ret;

	/* check parameters */
	if (copy_from_user(&fai_dma_ex_start, arg, sizeof(adv_dma_user))) {
		return MemoryCopyFailed;
	}
	privdata->user_buf = fai_dma_ex_start.userptr;
	privdata->divisor1 = fai_dma_ex_start.divisor1;	
	privdata->divisor2 = fai_dma_ex_start.divisor2;	
	privdata->conv_num = fai_dma_ex_start.datacnt;
	privdata->sample_rate = fai_dma_ex_start.samplerate;
	privdata->clk_src = fai_dma_ex_start.trig_src;
	privdata->half_fifo_size = fai_dma_ex_start.half_fifo_size;
	privdata->cyclic = privdata->cyclic;
	privdata->overrun_flag = 0;
	privdata->dmastart_flag = 1;
	privdata->trans =0;
	privdata->ai_stop =0;
	privdata->buf_stat = 1;
	privdata->page_index=0;
	privdata->half_ready=0;
	
	/* check DMA status 
	ret = adv_check_dma(device);
	if (ret) {
		return ret;
	}*/
	
	/* Init DMA */
	ret = adv_init_dma(device,
			   (INT16U *) privdata->user_buf,
			   privdata->conv_num * sizeof(INT16U));
	if (ret) {
		return ret;
	}

	return 0;
}

	
/**
 * adv_init_int - Set AD control register and map user buffer for interrupt
 *                mode AI
 *
 * @device: Points to the device object
 * @user_buf: user buffer address
 * @data_cnt: user buffer length in byte
 *
 * 1. set AD control register
 * 2. map user buffer
 */
static INT32S adv_init_int(adv_device *device, INT32U *user_buf, INT32U data_cnt)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U tmp;

	USHORT usControlReg ;
	
	/* get user buffer pages */
	switch (privdata->device_type) {
	case PCM3718:		/* pci-1716 use 16-bits data width */
		if ((data_cnt / 2) % 4) { /* We can ensure that top half buffer length is a
					   * multiple of 2, and bottom half buffer is a mu-
					   * ltiple of 2.
					   * 
					   * Top half buffer must same as bottom half buffer,
					   * so we need to ensure that any half buffer length
					   * is a multiple of 4 */
			return InvalidCountNumber;
		}

		break;
	default:
		return BoardIDNotSupported;
	}



        adv_map_user_pages(&privdata->user_pages,
			   &privdata->page_num,
			   (PTR_T) user_buf,
			   (PTR_T) data_cnt,
			   WRITE);
	if (!privdata->user_pages) {
		return MemoryAllocateFailed;
	}

       advOutp( privdata,  8, ( UCHAR )0 );   // clear interrupt

    	if ( privdata->int_cnt == 1 )	{ 
		privdata->fifo_enable=0; 
       advOutp( privdata,  8, ( UCHAR )0 );   // clear interrupt
      advOutp( privdata,  6, 0xff ); // enable interrupt
//      advOutp( privdata, 25, 0xff ); // clear all data in FIFO
  //    advOutp( privdata, 20, 0x00 );    // clear interrupt with FIFO
}else{
		privdata->fifo_enable=1; 
      advOutp( privdata,  6, 0xff ); // enable interrupt
      advOutp( privdata, 25, 0xff ); // clear all data in FIFO
      advOutp( privdata, 20, 0x00 );    // clear interrupt with FIFO
}

    	// setup internal trigger with interrupt transfer
    	if ( privdata->int_cnt == 1 )	 {
		 usControlReg = (USHORT)privdata->IntChan << 4;
	 } else   {
		 usControlReg = 0;   // interrupt with FIFO
	 }
    
	if (privdata->clk_src)    {
		 usControlReg = usControlReg | 0x82;
    	} else    {
		 usControlReg = usControlReg | 0x83;
	}
    
      usControlReg |= privdata->irq<<4;
	advOutp( privdata, 9, usControlReg );    // enabled pacer trigger
    	if (!privdata->clk_src)    {
		 adv_pacer_setup(privdata);
	}

	return 0;
}

/**
 * adv_fai_int_start - Initiates an asynchronous, single-channel, interrupt
 *                     mode data acquisition                             
 *                                                            
 * device: Point to the device object
 * arg: Argument from user
 */
INT32S adv_fai_int_start(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_int_user int_data;
	INT16U *gain_list;
	INT16U num_chan;
	INT16U start_chan;
int i;	
	INT32S ret=0;


	/* check parameter */
	if (copy_from_user(&int_data, (adv_int_user *)arg, sizeof(adv_int_user))){
		return MemoryCopyFailed;
	}
     
     
	num_chan = 1;
	start_chan = int_data.chan;	
	gain_list = &int_data.gain;
	privdata->ai_stop = 0;
		privdata->usAITerminateFlag= 0;
	privdata->usFAIRunning= 1;

	privdata->divisor1 = int_data.divisor1;
	privdata->divisor2 = int_data.divisor2;
	privdata->sample_rate = int_data.samplerate;
	privdata->user_buf = int_data.userptr;
	privdata->conv_num = int_data.bufsize;
	privdata->int_cnt = int_data.intrcount;
	privdata->cyclic = int_data.cyclic;
	privdata->trans =0;
	privdata->cur_index = 0;
	privdata->clk_src = int_data.trig_src; /* interrupt ai only pacer mode, so we use clock
						* source here */
/*printk("privdata->divisor1=%x\n",privdata->divisor1);
printk("privdata->divisor2=%x\n",privdata->divisor2);
printk("privdata->convnum=%d\n",privdata->conv_num);
printk("privdata->int_cnt=%x\n",privdata->int_cnt);
printk("privdata->clk_src=%x\n",privdata->clk_src);
printk("privdata->cyclic=%x\n",privdata->cyclic);
*/
        /* init Interrupt mode ai*/ 
	ret = adv_init_int(device,
			   (INT32U *) privdata->user_buf,
			   privdata->conv_num*sizeof(INT16U) );
	/*ret = adSetupInt(privdata);*/
	if (ret) {
		return ret;
	}
	return 0;
}

/**
 * adv_fai_int_scan_start - Initiates an asynchronous, multiple-channels, interrupt
 *                          mode data acquisition                             
 *                                                            
 * device: Point to the device object
 * arg: Argument from user
 */
INT32S adv_fai_int_scan_start(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_int_user int_data;
	INT16U *gain_list;
	INT16U num_chan;
	INT16U start_chan;
	
	INT32S ret;

	/* check parameter */
	if (copy_from_user(&int_data, (adv_int_user *)arg, sizeof(adv_int_user))){
		return MemoryCopyFailed;
	}
     
     
	num_chan = int_data.numchan;
	start_chan = int_data.startchan;	
	gain_list = int_data.gainlist;

	privdata->divisor1 = int_data.divisor1;
	privdata->divisor2 = int_data.divisor2;
	privdata->sample_rate = int_data.samplerate;
	privdata->user_buf = int_data.userptr;
	privdata->conv_num = int_data.bufsize;
	privdata->int_cnt = int_data.intrcount;
	privdata->cyclic = int_data.cyclic;
	privdata->trans =0;
	privdata->clk_src = int_data.trig_src; /* interrupt ai only pacer mode, so we use clock
						* source here */

        /* init Interrupt mode ai */
	ret = adv_init_int(device,
			   (INT32U *) privdata->user_buf,
			   privdata->conv_num * sizeof(INT16U));
	if (ret) {
		return ret;
	}

	
	return 0;
}


/**
 * adv_fai_stop - stop high speed ai(DMA & interrupt mode), and release
 *                allocated regions
 *
 * @device: Point to the device object
 */
INT32S adv_fai_stop(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U tmp;
    	advOutp(privdata,8,0); 
    	advOutp(privdata,9,0); 
    	advOutp(privdata,20,0); 
    	advOutp(privdata,6,0); 
   	advOutp(privdata,10,0x01); 
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
	if (privdata->user_pages) {
	adv_unmap_user_pages(privdata->user_pages,privdata->page_num,1);
        //free_pages(privdata->hwdmaptr[0], privdata->hwdmasize[0]);
	}
/*	if (privdata->usFAIRunning) {
		privdata->usFAIRunning = 0;
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
	//		kfree(privdata->sgl_addr);
			privdata->sgl_addr = NULL;
		};
	}
*/	

	
	/*if (privdata->sgl_addr) {
		kfree(privdata->sgl_addr);
	};*/
	kfree(privdata->dmabuf );
	privdata->user_pages = 0;
	privdata->sgl_addr = 0;
	privdata->buf_stat = 0;
	privdata->item = 0;
	privdata->cur_index = 0;
	privdata->page_index = 0;
	privdata->overrun_flag = 0;
	privdata->dmastart_flag = 0;
	privdata->ai_stop = 1;
	return 0;
}


/* int adv_fai_check(adv_device *device, void *arg) */
/* { */
/* 	PT_FAICheck lpFAICheck; */
/* 	private_data *privdata = (private_data *) (device->private_data); */

/* 	if (copy_from_user(&lpFAICheck, arg, sizeof(PT_FAICheck))) { */
/* 		printk("Copy FAICheck data to kernel error!\n"); */
/* 		return EFAULT; */
/* 	} */

/* 	lpFAICheck.HalfReady = privdata->cur_buf; */

/* 	if (copy_to_user(arg, &lpFAICheck, sizeof(PT_FAICheck))) { */
/* 		printk("Copy FAICheck data to user error!\n"); */
/* 		return EFAULT; */
/* 	} */

/* 	return 0; */
/* } */


/**
 * adv_clear_overrun - clear OVERRUN flag
 *
 * @device: Point to the device object
 */
INT32S adv_clear_overrun(adv_device *device)
{
     private_data *privdata = (private_data *) (device->private_data);


     privdata->overrun_flag = 0;


     return 0;
}

INT32S adv_fai_transfer(adv_device *device, PT_FAITransfer *lpFAITransfer)
{
	PT_FAITransfer fai_transfer;
	private_data *privdata = (private_data *) (device->private_data);
	
	if (copy_from_user(&fai_transfer,lpFAITransfer, sizeof(PT_FAITransfer))) {
		return  -EFAULT;
	}

	privdata->overrun_flag = 0;
	privdata->half_ready=0;
	
	if (copy_to_user(fai_transfer.overrun, &(privdata->overrun_flag), sizeof(privdata->overrun_flag))) {
		return -EFAULT;
	}
	return 0;
}
INT32S adv_ai_check(adv_device *device, void *arg)
{
	PT_FAICheck fai_check;
	private_data *privdata = (private_data *) (device->private_data);


//	if(privdata->dmastart_flag) {
		if (copy_from_user(&fai_check, arg, sizeof(PT_FAICheck))) {
			return -EFAULT;
		}

		fai_check.HalfReady = privdata->half_ready;
		fai_check.ActiveBuf = 0;
		fai_check.stopped = privdata->ai_stop;
		fai_check.retrieved = privdata->trans;
		fai_check.overrun = privdata->overrun_flag;
	
		if (copy_to_user(arg, &fai_check, sizeof(PT_FAICheck))) {
			return  -EFAULT;
		}
//	}
	
	return 0;
}
