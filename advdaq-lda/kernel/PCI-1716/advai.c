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
#include "PCI1716.h"


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
static void adv_pacer_setup(private_data *privdata, INT8U rate)
{
	INT16U div1 = 0;
	INT16U div2 = 0;
     
     
	if (rate == 0xff) {
		div1 = 2;
		div2 = 0;
	} else {
		div1 = privdata->divisor1;
		div2 = privdata->divisor2;
	}
	advOutpw(privdata, 0x1e, 0x0074);	
	advOutpw(privdata, 0x1a, div1 & 0x00ff);
	advOutpw(privdata, 0x1a, ((div1 & 0xff00) >> 8));

	advOutpw(privdata, 0x1e, 0x00b4); 
	advOutpw(privdata, 0x1c, div2 & 0x00ff);
	advOutpw(privdata, 0x1c, ((div2 & 0xff00) >> 8));
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
	INT32U sgl_size;
	INT32U sgl_cnt = 0;
	INT32U sgl_phyaddr;
	INT32U *sgl_addr;
	INT32U page_num; /* SGL total descriptor numbers in buffer */
	INT32U virt_addr;
	INT32U phy_addr;
	INT32U total_len = 0;
	INT32U cur_len=0;
	INT32U top_len;		/* top half page length */
	INT32U bot_len;		/* bottom half page length */
	INT16U i=0, j=0;
	INT16U tmp;
	
	
	switch (privdata->device_type) {
	case PCI1716:
	case PCI1716L:
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
	/* build SGL table */
	sgl_size = (page_num+1) * 16; /* the size of each descriptor is 16 bytes */
	sgl_addr = kmalloc(sgl_size, GFP_KERNEL);
	if (!sgl_addr) {
		return -ENOMEM;
	}
	sgl_phyaddr = virt_to_phys((void *) sgl_addr);


        /* fill the top half buffer into the SGL table */
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

		/* half buffer cause interrupt */
		if (total_len + cur_len == privdata->conv_num) {
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x0d);
		}

		/* page unalign case */
		if ((total_len < privdata->conv_num)
		    && (total_len + cur_len > privdata->conv_num)) {

			/* for top half page */
			top_len = privdata->conv_num - total_len;
			j = i;

			sgl_addr[j * 4] = phy_addr;
			sgl_addr[j * 4 + 1] = 0x0000;
			sgl_addr[j * 4 + 2] = top_len;
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x0d);

			j += 1;
			sgl_cnt++;

			/* for bottom half page, cause interrupt */
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
	
	if (privdata->cyclic) {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) (sgl_phyaddr | 0x0d);
	} else {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) ((sgl_phyaddr + (i * 16)) | 0x0f);
	}
	/* initialize PCI9054 */
	advOutpDMAw(privdata, 0x04, 0x0001); /* enable pci addresses */
	advOutpDMAw(privdata, 0x08, 0x0000);
	advOutpDMAw(privdata, 0x0a, 0x0024);
	advOutpDMAw(privdata, 0x68, 0x0100);

	advOutpDMAw(privdata, 0x6a, 0x0004);
	advOutpDMAw(privdata, 0x18, 0x000d); /* 16 bits memory width */
	advOutpDMAw(privdata, 0xb0, 0x0000);

	advOutpDMAdw(privdata, 0x80, 0x00021f0d);
	advOutpDMAdw(privdata, 0x90, (INT32U) (sgl_phyaddr | 0x0d)); /* direction: local -> pci */

	advOutpDMA(privdata, 0xa8, 0x09); /* enable channel 0 to transfer data */
	advOutpw(privdata, 0x06, (USHORT)(advInpw(privdata, 0x06) & 0x8040));
	advOutpw(privdata, 0x08, 0x0000); /* clear fifo & interrupt */
	/* start DMA transfer */
	advOutpDMA(privdata, 0xa8, 0x0b);

	if(privdata->clk_src) {
		tmp = 0x0004; 
	} else {
		tmp = 0x0002;		
	}
	
	advOutpw(privdata, 0x06, (advInpw(privdata, 0x06) & 0x8040) | tmp);

	/* setup counter */
	adv_pacer_setup(privdata, 0x00);
	
	privdata->sgl_addr = sgl_addr;
	privdata->sgl_cnt = sgl_cnt;
	privdata->page_num = page_num;
	privdata->user_pages = user_pages;

	return 0;

}

INT32S FAIDmaStart(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32S ret = 0;
	INT16U tmp;

	tmp = advInpDMA(privdata,0xa8) & 0x10;
	if (tmp != 0x10) {
		advOutpDMAw(privdata, 0x04, 0x0001);
		advOutpDMAdw(privdata, 0x08, 0x00240000);

		advOutpDMAw(privdata, 0x68, 0x0);
		advOutpDMAw(privdata, 0x6a, 0x0);

		advOutpDMAw(privdata, 0x18, 0x000d);
		advOutpDMAw(privdata, 0xb0, 0x0000);
	
		advOutpDMAw(privdata, 0x80, 0x090d);
		advOutpDMAw(privdata, 0x82, 0x0000);
   		advOutpDMAdw(privdata, 0x84, privdata->phyAddrBlock);

		advOutpDMAdw(privdata, 0x88, 0x00000000);// DMA Channel 0 PCI Address Reg
		advOutpDMAdw(privdata, 0x8C, 0x00000080);
		advOutpDMAw(privdata, 0x90, 0x000b);
		advOutpDMAw(privdata, 0x92, 0x0000);  // DMA Channel0 Descripter Pointer Reg
		advOutpDMAw(privdata, 0x08, 0x0);

		advOutpDMAw(privdata, 0xa8, 0x0009);
		advOutpDMAw(privdata, 0xa8, 0x000b);
		tmp = advInpDMA(privdata,0xa8) & 0x10;
	
		while(tmp != 0x10){
			tmp = advInpDMA(privdata,0xa8)&0x10;
		}
		advOutpDMAw(privdata, 0x08, 0x0);
	}

	

	spin_lock(&privdata->spinlock);

	ret = adv_init_dma(device,
			   (INT32U *) privdata->user_buf,
			   privdata->conv_num * sizeof(INT16U));
	if (ret) {
		return ret;
	}
	
	spin_unlock(&privdata->spinlock);

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
	INT32S ret;
	adv_dma_user int_data;
	
	if (copy_from_user(&int_data, arg, sizeof(adv_dma_user))) {
	  	return -EFAULT;
     	}

	privdata->divisor1 = int_data.divisor1;
	privdata->divisor2 = int_data.divisor2;
	privdata->sample_rate = int_data.samplerate;
	privdata->user_buf = int_data.userptr;
	privdata->conv_num = int_data.datacnt;
	privdata->int_cnt = int_data.intrcount;
	privdata->cyclic = int_data.cyclic;
	privdata->trig_mode = int_data.trig_mode;
	privdata->trig_src = int_data.trig_src;
	privdata->trig_vol = int_data.trigvol;
	privdata->ulIntcount = 0;
	privdata->ulTerminalcnt = 0;
	privdata->clk_src = int_data.clocksrc;
	privdata->src_type = int_data.src_type;

	privdata->retrieved = 0;	
	

	ret = FAIDmaStart(device);
	if(ret) {
		return ret;
	}

	return 0;
}

INT32S adPacerSetup(private_data *privdata)
{
	// setup 8254 for pacer
	USHORT temp;


	advOutpw(
		privdata,
		0x06,
		( USHORT ) ( advInpw( privdata, 0x06 ) & 0x8040 ) );   

	advOutp(
		privdata,
		0x1e,
		0xb4 );  /* Set counter 2 to mode 2, rate gen. */

	advOutp(
		privdata,
		0x1e,
		0x74 );  /* Set counter 1 to mode 2, rate gen. */

	temp = ( USHORT ) ( privdata->divisor1 & 0xff );
	advOutp(
		privdata,
		0x1c,
		( UCHAR ) temp ); /* Write counter 2 low byte  */
	temp = ( USHORT ) ( ( privdata->divisor1 >> 8 ) & 0xff );
	advOutp(
		privdata,
		0x1c,
		( UCHAR ) temp ); /* Write counter 2 high byte  */

	temp = ( USHORT )( privdata->divisor2 & 0xff );
	advOutp(
		privdata,
		0x1a,
		( UCHAR ) temp ); /* Write counter 1 low byte  */
	temp = ( USHORT ) ( ( privdata->divisor2 >> 8 ) & 0xff );
	advOutp(
		privdata,
		0x1a,
		( UCHAR ) temp ); /* Write counter 1 high byte  */

	return 0;
}

INT32S adSetupInt(private_data *privdata)
{

	// Initialize some value

	advOutpDMAw(privdata, 0x68, 0x0000); 
	advOutpw(privdata, 0x06, (USHORT)(advInpw(privdata, 0x06) & 0x8040 ) ); // disable interrupt
	advOutpw(privdata, 0x08, 0x0000);    // clear interrupt with FIFO

	advOutpDMAw(privdata, 0x68, 0x0900); 
	if (privdata->clk_src) {  
		if (privdata->int_cnt == 1) {
			privdata->fifo_enable = 0;
			advOutpw( privdata, 0x06, (USHORT)((advInpw(privdata, 0x06) & 0x8040 ) | 0x001c) );
		} else {
			privdata->fifo_enable = 1;	      
			advOutpw(privdata, 0x06, (USHORT)((advInpw(privdata, 0x06) & 0x8040 ) | 0x003c));
		}
	} else { 
		adPacerSetup( privdata );
		if ( privdata->int_cnt == 1 ){
			privdata->fifo_enable = 0;
			advOutpw( privdata, 0x06, (USHORT)((advInpw(privdata, 0x06) & 0x8040 ) | 0x0012) );
		} else {
			privdata->fifo_enable = 1;	      
			advOutpw( privdata, 0x06, (USHORT)((advInpw(privdata, 0x06) & 0x8040 ) | 0x0032) );
		}
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


	
	/* get user buffer pages */
	switch (privdata->device_type) {
	case PCI1716:		/* pci-1716 use 16-bits data width */
	case PCI1716L:
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

        adv_map_user_pages(&privdata->user_pages,
			   &privdata->page_num,
			   (PTR_T) user_buf,
			   (PTR_T) data_cnt,
			   WRITE);
	if (!privdata->user_pages) {
		return -EFAULT;
	}

	adSetupInt(privdata);
	
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
	INT16U start_chan;
	INT32S ret;
	

	/* check parameter */
	if (copy_from_user(&int_data, (adv_int_user *)arg, sizeof(adv_int_user))){
		return -EFAULT;
	}

     	privdata->terminated = 0;
	start_chan = int_data.chan;	
	gain_list = &int_data.gain;


	privdata->halfready_flag = 0;
	privdata->half_ready = 0;
	
	privdata->divisor1 = int_data.divisor1;
	privdata->divisor2 = int_data.divisor2;



	privdata->sample_rate = int_data.samplerate;
	privdata->user_buf = int_data.userptr;
	privdata->conv_num = int_data.bufsize;
	privdata->int_cnt = int_data.intrcount;
	privdata->cyclic = int_data.cyclic;
	privdata->clk_src = int_data.trig_src;

	

	privdata->retrieved = 0;
	
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
		return -EFAULT;
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
	privdata->clk_src = int_data.trig_src; /* interrupt ai only pacer mode, so we use clock
						* source here */

	privdata->retrieved = 0;

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
     

     
	advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x0040);
	advOutpw(privdata, 0x08, 0x0000);

	tmp = advInpDMAdw(privdata, 0x80);
	advOutpDMAdw(privdata, 0x80, tmp & 0xfffffbff); /* clear DMA interrupt */

	tmp = advInpDMA(privdata, 0xa8) & 0x10;
	if (!tmp) { /* DMA transfer not complete, abort transfer */
		tmp = advInpDMA(privdata, 0xa8) | 0x04;
	}
	advOutpDMA(privdata, 0xa8, tmp & 0xfe);	
	advOutpDMAw(privdata, 0x68, 0x0000);
	tasklet_kill(&privdata->tasklet);

	if (privdata->user_pages) {
		adv_unmap_user_pages(privdata->user_pages,
				     privdata->page_num,
				     1);
	}
	
	if (privdata->sgl_addr) {
		kfree(privdata->sgl_addr);
	};

	privdata->user_pages = 0;
	privdata->sgl_addr = 0;
	privdata->buf_stat = 0;
	privdata->item = 0;
	privdata->terminated = 1;
	privdata->cur_index = 0;
	privdata->page_index = 0;
	privdata->overrun_flag = 0;


	adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);


	return 0;
}

/**
 * DLL_FAICheck - check status of interrupt mode AI
 * 
 *
 * @device: Point to the device object
 */
INT32S adv_fai_check(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_FAICheck fai_check;
	

	fai_check.ActiveBuf = 0;
	fai_check.stopped = privdata->terminated;
	fai_check.retrieved = privdata->retrieved;
	fai_check.overrun = privdata->overrun_flag;
	fai_check.HalfReady = privdata->half_ready;

	if (copy_to_user(arg, (void *) &fai_check, sizeof(PT_FAICheck))) {
		return -EFAULT;
	}

	return 0;
}



