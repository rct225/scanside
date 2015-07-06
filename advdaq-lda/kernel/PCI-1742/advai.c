/****************************************************************************
 *		Copyright 2006 ABJ Linux Driver Team, 
 *
 *			BeiJing R&D Center 
 *
 *			Advantech Co., Ltd.
 *
 *	Advantech PCI-1742 Device driver for Linux
 *
 * File Name: 
 *	advai.c
 * Abstract:
 *	This file contains routines for ai function.
 * Version history
 *      12/06/2006              Created by LI.ANG
 ************************************************************************/	
#include "PCI1742.h"


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
	
	if (privdata->fai_running != FAI_DMA_START) {
		advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x9040);
	}	
	
	advOutp(privdata, 0x1e, 0xb4);
	advOutp(privdata, 0x1e, 0x74);

	advOutpw(privdata, 0x1c, div1 & 0x00ff);
	advOutpw(privdata, 0x1c, ((div1 & 0xff00) >> 8));

	advOutpw(privdata, 0x1a, div2 & 0x00ff);
	advOutpw(privdata, 0x1a, ((div2 & 0xff00) >> 8));
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


	
	/* get user buffer pages */
	switch (privdata->device_type) {
	case PCI1742:		/* pci-1742 use 16-bits data width */
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

        adv_map_user_pages(&privdata->user_pages,
			   &privdata->page_num,
			   (PTR_T) user_buf,
			   (PTR_T) data_cnt,
			   WRITE);
	if (!privdata->user_pages) {
		return -ENOMEM;
	}

	/* clear & disable interrupt */
	advOutpDMAw(privdata, 0x68, 0x0000);
	advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x9040);
	advOutpw(privdata, 0x08, 0000);
	advOutpDMAw(privdata, 0x68, 0x0900);
	

	/* set control register for interrupt handle */

	
	tmp = advInpw(privdata, 0x06) & 0x9040;
	if (privdata->clk_src) { /* external mode: waitting for external trigger */
		if (privdata->int_cnt == 1) {
			privdata->fifo_enable = 0;
			advOutpw(privdata, 0x06, tmp | 0x0014);
		} else {
			privdata->fifo_enable = 1;
			advOutpw(privdata, 0x06, tmp | 0x0034);
		}
	} else { /* pacer mode: internal trigger will coming from 82C54 */
		adv_pacer_setup(privdata, 0x00);

		if (privdata->int_cnt == 1) {
			privdata->fifo_enable = 0;
			advOutpw(privdata, 0x06, tmp | 0x0012);
		} else {
			privdata->fifo_enable = 1;
			advOutpw(privdata, 0x06, tmp | 0x0032);
		}
	}
/* 	KdPrint("reg[0x06]: 0x%x\n", advInp(privdata, 0x06)); */
/* 	KdPrint("reg[0x07]: 0x%x\n", advInp(privdata, 0x07)); */
	
	return 0;
}


/**
 * adv_check_dma - check DMA status until the DMA transfer terminated
 *
 * @device: Points to the device object
 */
static INT32S adv_check_dma(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U flush_cache_phy;
	VOID *flush_cache_virt;
	INT16U tmp;


	flush_cache_virt = kmalloc(128, GFP_DMA);
	if (!flush_cache_virt) {
		/* printk("Allocate flush cache error\n"); */
		return -ENOMEM;
	}
	flush_cache_phy = virt_to_phys((void *) flush_cache_virt);


	tmp = advInpDMA(privdata, 0xa8) & 0x10;
	if (tmp != 0x10) {
		advOutpDMAw(privdata, 0x04, 0x0001);
		advOutpDMAdw(privdata, 0x08, 0x00240000);
		
		advOutpDMAw(privdata, 0x68, 0x00);
		advOutpDMAw(privdata, 0x6a, 0x00);

		advOutpDMAw(privdata, 0x18, 0x000d);
		advOutpDMAw(privdata, 0xb0, 0x0000);
		
		advOutpDMAw(privdata, 0x80, 0x090d);
		advOutpDMAw(privdata, 0x82, 0x0000);
		
		advOutpDMAdw(privdata, 0x84, flush_cache_phy);
		advOutpDMAdw(privdata, 0x88, 0x00000000);
		advOutpDMAdw(privdata, 0x8c, 0x00000040);
		
		advOutpDMAw(privdata, 0x90, 0x000b);
		advOutpDMAw(privdata, 0x92, 0x0000);
		
		advOutpw(privdata, 0x08, 0x00);
		
		advOutpDMAw(privdata, 0xa8, 0x0009);
		advOutpDMAw(privdata, 0xa8, 0x000b);
		
		tmp = advInpDMA(privdata, 0xa8) & 0x10;
		while (tmp != 0x10) {
			tmp = advInpDMA(privdata, 0xa8) & 0x10;
		}
		advOutpw(privdata, 0x08, 0x00);
	}

	kfree(flush_cache_virt);
	
	
	return 0;
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
	INT32U page_num;  /* SGL total descriptor numbers in buffer */
	INT32U virt_addr;
	INT32U phy_addr;
	INT32U total_len = 0;
	INT32U cur_len;
	INT32U top_len;		/* top half page length */
	INT32U bot_len;		/* bottom half page length */
	INT16U i, j;

	

	switch (privdata->device_type) {
	case PCI1742:		/* pci-1742 use 16-bits data width */
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
		return -ENOMEM;
	}
	

	/* build SGL table */
	sgl_size = (page_num + 1) * 16; /* the size of each descriptor is 16 bytes
					 * note: the page_num must add '1', because
					 * we may be encounter the problem that page
					 * is unaligned */
	sgl_addr = kmalloc(sgl_size, GFP_DMA);
	if (!sgl_addr) {
		printk("Allocate sgl list error\n");
		return -ENOMEM;
	}
	sgl_phyaddr = virt_to_phys((void *) sgl_addr);


        /* fill the top half buffer into the SGL table */
	for (j = i = 0; i < page_num; i++, j++) {
		virt_addr = user_pages[i].page_addr + user_pages[i].offset;
		phy_addr = virt_to_phys((void *) virt_addr);
		cur_len = user_pages[i].length;
		
		if (((sgl_cnt + 1) * 16) > sgl_size) {
			printk("sgl table overflow !\n");
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

			/* for top half page, cause interrupt */
			top_len = privdata->conv_num - total_len;
			j = i;

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
	
	if (privdata->cyclic) {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) (sgl_phyaddr | 0x0d);
	} else {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) ((sgl_phyaddr + (i * 16)) | 0x0f);
	}
	

	/* initialize PCI9054 */
	/* reg[0x4]: PCI command register, allows the device to respond
	 *           to I/O space accesses. */
	advOutpDMAw(privdata, 0x04, 0x0001); /* enable pci addresses */

	/* reg[0x8]: revision ID. Sillicon revision of the PCI9054
	 * reg[0x9]: register level programming interface. None define. */
	advOutpDMAw(privdata, 0x08, 0x0000);

	/* reg[0xa]: subclass code(other bridge device).
	 * reg[0xb]: base class code(bridge device). */
	advOutpDMAw(privdata, 0x0a, 0x0024);
	
	/* reg[0x68]: interrupt control/status register */
	advOutpDMAw(privdata, 0x68, 0x0100); /* enable PCI int(bit 8) & LOCAL int(bit 11) */

	/* reg[0x6a]: local DMA channel 0 interrupt enable */
	advOutpDMAw(privdata, 0x6a, 0x0004);

	/* reg[0x18]: DMA channel 0 mode register
	 *            bit0~1 - local bus width
	 *                     == 00 means 8-bit bus width
	 *                     == 01 means 16-bit bus width
	 *                     == 10 or 11 means 32-bit bus width */
	advOutpDMAw(privdata, 0x18, 0x000d); /* 16 bits memory width */

	/* reg[0xb0]: DMA threshold register */
	advOutpDMAw(privdata, 0xb0, 0x0000);

	/* reg[0x80]: DMA channel 0 mode register */
	advOutpDMAdw(privdata, 0x80, 0x00021f0d); /* set DMA mode to scatter/gather */

	/* reg[0x90]: DMA channel 0 descriptor pointer register */
	advOutpDMAdw(privdata, 0x90, (INT32U) (sgl_phyaddr | 0x0d)); /* direction: local -> pci */

	/* reg[0xa8]: DMA channel 0 command/status register
	 *            set enable and go bit in DMA command/status register
	 *            channel 0 enable and clear interrupt*/
	advOutpDMA(privdata, 0xa8, 0x09); /* enable channel 0 to transfer data */



	/* counter setting: change AI mode to single value mode */
	advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x8040); /* cler A/D control register */
	advOutpw(privdata, 0x08, 0x0000); /* clear interrupt and FIFO */

        /* start DMA transfer */
	advOutpDMA(privdata, 0xa8, 0x0b);

/* 	advOutpDMAw(privdata, 0x04, 0x0001); /\* enable pci addresses *\/ */
/* 	advOutpDMAw(privdata, 0x08, 0x0000); */
/* 	advOutpDMAw(privdata, 0x0a, 0x0024); */
	
/* 	advOutpDMAw(privdata, 0x68, 0x0100); /\* enable PCI int(bit 8) & LOCAL int(bit 11) *\/ */
/* 	advOutpDMAw(privdata, 0x6a, 0x0004); */
/* 	advOutpDMAw(privdata, 0x18, 0x000d); /\* 16 bits memory width *\/ */
/* 	advOutpDMAw(privdata, 0xb0, 0x0000); */

/* 	advOutpDMAdw(privdata, 0x80, 0x00021f0d); */
/* 	advOutpDMAdw(privdata, 0x90, (INT32U) (sgl_phyaddr | 0x0d)); /\* direction: local -> pci *\/ */

/* 	advOutpDMA(privdata, 0xa8, 0x09); /\* enable channel 0 to transfer data *\/ */

/* 	/\* initialize local register *\/ */
/* 	advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x9040); */
/* 	advOutpw(privdata, 0x08, 0x0000);	 */

/* 	/\* start DMA transfer *\/ */
/* 	advOutpDMA(privdata, 0xa8, 0x0b); */
	

	if (privdata->clk_src) {
		advOutpw(privdata, 0x06, (advInpw(privdata, 0x06) & 0x9040) | 0x04);
	} else {
		advOutpw(privdata, 0x06, (advInpw(privdata, 0x06) & 0x9040) | 0x02);
	}

	/* setup counter */
	adv_pacer_setup(privdata, 0x00);
	
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
	adv_dma_user dma_data;
	INT16U start_chan;
	INT16U num_chan;
	INT16U *gain_list;
	INT32S ret;


	
	/* check if AI has been running by somebody else */
	/* down_interruptible(privdata->sem_fai); */
	if (privdata->fai_running) {
		return -EBUSY;
	}
	privdata->fai_running = FAI_DMA_START;
	
	/* check parameters */
	if (copy_from_user(&dma_data, arg, sizeof(adv_dma_user))) {
		return -EFAULT;
	}

	start_chan = dma_data.startchan;
	num_chan = dma_data.numchan;
	gain_list = dma_data.gainlist;

/* 	privdata->low_buf_flag = 0; */
/* 	privdata->high_buf_flag = 0; */
	privdata->terminated = 0;     
	privdata->half_ready = 0;
	privdata->halfready_flag = 0;
	privdata->transfered = 0;
	
	privdata->divisor1 = dma_data.divisor1;
	privdata->divisor2 = dma_data.divisor2;
	privdata->user_buf = dma_data.userptr;
	privdata->conv_num = dma_data.datacnt;
	privdata->sample_rate = dma_data.samplerate;
	privdata->half_fifo_size = dma_data.half_fifo_size;
	privdata->clk_src = dma_data.trig_src;
	privdata->cyclic = dma_data.cyclic;
	
	
	/* check DMA status */
	ret = adv_check_dma(device);
	if (ret) {
		return ret;
	}

	/* Init DMA */
	ret = adv_init_dma(device, (INT32U *) privdata->user_buf,
			   privdata->conv_num * sizeof(INT16U));
	if (ret) {
		return ret;
	}
/* 	KdPrint("reg[0x06]: 0x%x\n", advInp(privdata, 0x06)); */
/* 	KdPrint("reg[0x07]: 0x%x\n", advInp(privdata, 0x07)); */
	

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
	INT32S ret;
	

	/* check if AI has been running by somebody else */
	/* down_interruptible(privdata->sem_fai); */
	if (privdata->fai_running) {
		return -EBUSY;
	}
	privdata->fai_running = FAI_INT_START;


	if (copy_from_user(&int_data, (adv_int_user *) arg, sizeof(adv_int_user))) {
		return -EFAULT;
	}

	privdata->terminated = 0;     
	privdata->transfered = 0;
	privdata->divisor1 = int_data.divisor1;
	privdata->divisor2 = int_data.divisor2;
	privdata->sample_rate = int_data.samplerate;
	privdata->user_buf = int_data.userptr;
	privdata->conv_num = int_data.bufsize;
	privdata->int_cnt = int_data.intrcount; 
	privdata->cyclic = int_data.cyclic;
	privdata->half_fifo_size = int_data.half_fifo_size;
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
	INT32S ret;
	

	/* check if AI has been running by somebody else */
	/* down_interruptible(privdata->sem_fai); */
	if (privdata->fai_running) {
		return -EBUSY;
	}
	privdata->fai_running = FAI_INT_SCAN_START;


	if (copy_from_user(&int_data, (adv_int_user *) arg, sizeof(adv_int_user))) {
		return -EFAULT;
	}

	privdata->terminated = 0;     
	privdata->transfered = 0;
	privdata->divisor1 = int_data.divisor1;
	privdata->divisor2 = int_data.divisor2;
	privdata->sample_rate = int_data.samplerate;
	privdata->user_buf = int_data.userptr;
	privdata->conv_num = int_data.bufsize;
	privdata->int_cnt = int_data.intrcount; 
	privdata->cyclic = int_data.cyclic;
	privdata->half_fifo_size = int_data.half_fifo_size;
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
 * adv_fai_stop - schedued in tasklet, when fai terminated but region is not
 *                released and device is not stopped.
 *
 * @device: Point to the device object
 */
VOID adv_fai_stop_work(VOID *data)
{
	adv_fai_stop((adv_device *) data);
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
     

	if (privdata->fai_running == FAI_DMA_START) {
		advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x9040);
		advOutpw(privdata, 0x08, 0x0000);

		tmp = advInpDMAdw(privdata, 0x80);
		advOutpDMAdw(privdata, 0x80, tmp & 0xfffffbff); /* clear DMA interrupt */

/* 		tmp = advInpDMA(privdata, 0xa8) & 0x10; */
/* 		if (!tmp) { /\* DMA transfer not complete, abort transfer *\/ */
/* 			tmp = advInpDMA(privdata, 0xa8) | 0x04; */
/* 		} */
		tmp = advInpDMA(privdata, 0xa8);
		advOutpDMA(privdata, 0xa8, tmp & 0xfe);
		/* advOutpDMAw(privdata, 0x68, 0x0000); */
	} else {
		advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x9040);
		advOutpw(privdata, 0x08, 0x0000);
	}

/* 	/\* terminate the tasklet *\/ */
/* 	tasklet_kill(&privdata->fai_tasklet); */


	if (privdata->user_pages) {
		adv_unmap_user_pages(privdata->user_pages,
				     privdata->page_num,
				     1);
	}
	
	if (privdata->sgl_addr) {
		kfree(privdata->sgl_addr);
	}
	
	
	/* when FAI terminate, pacer_out pin will not pulse out, so: */
	/* reset counter1 */
	advOutp(privdata, 0x1e, 0x79);
	advOutp(privdata, 0x1a, 0x00);
	advOutp(privdata, 0x1a, 0x00);
	/* reset counter2 */
	advOutp(privdata, 0x1e, 0xb9);
	advOutp(privdata, 0x1c, 0x00);
	advOutp(privdata, 0x1c, 0x00);

	/* set terminate event for users */
	adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);

	privdata->user_pages = 0;
	privdata->sgl_addr = 0;
	privdata->buf_stat = 0;
	privdata->item = 0;
	privdata->cur_index = 0;
	privdata->page_index = 0;
	privdata->terminal_cnt = 0;
	privdata->overrun_flag = 0;
	privdata->fai_running = 0;
	privdata->terminated = 1;
	
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
	fai_check.retrieved = privdata->transfered;
	fai_check.overrun = privdata->overrun_flag;
	fai_check.HalfReady = privdata->half_ready;

	if (copy_to_user(arg, (void *) &fai_check, sizeof(PT_FAICheck))) {
		return -EFAULT;
	}

	return 0;
}
