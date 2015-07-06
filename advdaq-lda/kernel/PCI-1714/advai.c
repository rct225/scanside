/****************************************************************************
 *		Copyright 2004 IAG Software Team, 
 *
 *			BeiJing R&D Center 
 *
 *			Advantech Co., Ltd.
 *
 *	Advantech PCI-1714 Device driver for Linux
 *
 * File Name: 
 *	PCI1714.c
 * Abstract:
 *	This file contains routines for ai function.
 * Version history
 *      08/09/2006              Created by LI.ANG
 ************************************************************************/	
#define __NO_VERSION__
#include "PCI1714.h"


/**
 * adv_check_dma - check DMA status until the DMA transfer terminated
 *
 * @device: Points to the device object
 */
static INT32S adv_check_dma(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U tmp;
	INT16U timeout;
	

	tmp = advInpDMA(privdata, 0xa8) & 0x10;
	timeout = 65530;
	while ((tmp != 0x10) && (timeout > 0)) {
		tmp = advInpDMA(privdata, 0xa8) & 0x10;
		timeout--;
	}

	if (!timeout) {
		return -EBUSY;
	}	

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
 * 3. setup 9056 DMA register
 */
static INT32S adv_init_dma(adv_device *device, INT32U *user_buf, INT32U data_cnt)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_user_page *user_pages; /* top half buffer */
	INT32U sgl_size;
	INT32U sgl_cnt = 0;
	INT32U sgl_phyaddr;
	INT32U *sgl_addr;
	INT32U page_num; /* SGL total descriptor numbers in top buffer */
	INT32U virt_addr;
	INT32U phy_addr;
	INT32U total_len = 0;
	INT32U cur_len;
	INT32U top_len;		/* top half page length */
	INT32U bot_len;		/* bottom half page length */
	INT16U i, j;
	INT16U tmp;
	INT32U io_base_addr;	/* add by zhiyong */
	

	switch (privdata->device_type) {
	case PCI1714:		/* pci-1714 use 12-bits data width */
	case PCI1714U:      
	case PCI1714UL:     /* pci-1714UL use 12-bits data width  */
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

	/* detect offset address of A/D sample, add by zhiyong begain */
	if (privdata->num_chan == 4) {
		io_base_addr = 0x0030;
	} else {
		if (privdata->start_chan == 0) {
			io_base_addr = 0x0000;
		} else {
			io_base_addr = 0x0004;
		}
	}
	/* add by zhiyong end */
	
	/* get top half buffer pages */
        adv_map_user_pages(&user_pages,
			   &page_num,
			   (PTR_T) user_buf,
			   (PTR_T) (data_cnt),
			   WRITE);
	if (!user_pages) {
		return -EFAULT;
	}

	/* build SGL table */
	sgl_size = (page_num + 1) * 16; /* the size of each descriptor is 16 bytes
					 * note: the page_num must add '1', because
					 * we may be encounter the problem that page
					 * is unaligned */
	sgl_addr = kmalloc(sgl_size, GFP_DMA);
	if (!sgl_addr) {
		KdPrint("Allocate sgl list error\n");
		return -ENOMEM;
	}
	sgl_phyaddr = virt_to_phys((void *) sgl_addr);


        /* fill the top half buffer into the SGL table */
	for (j = i = 0; i < page_num; i++, j++) {
		virt_addr = user_pages[i].page_addr + user_pages[i].offset;
		phy_addr = virt_to_phys((void *) virt_addr);
		cur_len = user_pages[i].length;
		
		if (((sgl_cnt + 1) * 16) > sgl_size) {
			KdPrint("sgl table overflow !\n");
			return -EFAULT;
		}
		/* comment by zhiyong begin */
/* 		sgl_addr[j * 4] = phy_addr; */
/* 		sgl_addr[j * 4 + 1] = 0x0000; */
/* 		sgl_addr[j * 4 + 2] = cur_len; */
/* 		sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x09); */
		/* comment by zhiyong end */
		/* add codes by zhiyong begin */
		sgl_addr[j * 4] = phy_addr;
		sgl_addr[j * 4 + 1] = io_base_addr;
		sgl_addr[j * 4 + 2] = cur_len;
		sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x09);
		/* add codes by zhiyong end */

		/* half buffer cause interrupt */
		if (total_len + cur_len == privdata->conv_num) {
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x0d);
		}

		/* page unalign case */
		if ((total_len < privdata->conv_num)
		    && (total_len + cur_len > privdata->conv_num)) {

			/* for top half page, cause interrupt */
			top_len = privdata->conv_num - total_len;
			/* 	j = i; */ //comment by zhiyong

			sgl_addr[j * 4] = phy_addr;
			/* comment by zhiyong begin */
/* 	if (privdata->num_chan == 4) { */
/* 				sgl_addr[i * 4 + 1] = 0x0030; */
/* 			} else { */
/* 				if (privdata->start_chan == 0) { */
/* 					sgl_addr[i * 4 + 1] = 0x0000; */
/* 				} else { */
/* 					sgl_addr[i * 4 + 1] = 0x0004; */
/* 				} */
/* 			} */
			/* comment by zhiyong end */
			sgl_addr[j * 4 + 1] = io_base_addr;
			sgl_addr[j * 4 + 2] = top_len;
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x0d);

			j += 1;
			sgl_cnt++;

			/* for bottom half page */
			bot_len = cur_len - top_len;
			virt_addr = user_pages[i].page_addr + user_pages[i].offset + top_len;
			phy_addr = virt_to_phys((void *) virt_addr);

			sgl_addr[j * 4] = phy_addr;
			/* if (privdata->num_chan == 4) { */
/* 				sgl_addr[i * 4 + 1] = 0x0030; */
/* 			} else { */
/* 				if (privdata->start_chan == 0) { */
/* 					sgl_addr[i * 4 + 1] = 0x0000; */
/* 				} else { */
/* 					sgl_addr[i * 4 + 1] = 0x0004; */
/* 				} */
/* 			} */
			sgl_addr[j * 4 + 1] = io_base_addr;
			sgl_addr[j * 4 + 2] = bot_len;
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x09);
		}

		sgl_cnt++;
		total_len += cur_len;
	}
	
	if (privdata->cyclic) {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) (sgl_phyaddr | 0x0d);
	} else if (privdata->trig_mode == TRIG_MODE_ABOUT) {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) (sgl_phyaddr | 0x09);
	} else {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) ((sgl_phyaddr + (i * 16)) | 0x0f);
	}


	/* initialize PCI9056 */
	advOutpDMAw(privdata, 0x04, 0x0001); /* enable pci addresses */
	advOutpDMAw(privdata, 0x08, 0x0000); /* Mode/DMA arbitration low byte */
	advOutpDMAw(privdata, 0x0a, 0x0024); /* enable LOCAL bus BREQi(bit 18)
					      * enable bit 21 for C&J Modes*/

	advOutpDMAw(privdata, 0x68, 0x0900); /* enable PCI interrupt(bit 8)
					      * enable LOCAL interrupt(bit 11) */
	advOutpDMAw(privdata, 0x6a, 0x0004); /* enable DMA channel 0 interrupt */
	advOutpDMAw(privdata, 0x18, 0x0007); /* 32 bits local bus data width */
	advOutpDMAw(privdata, 0xb0, 0x0000); /* DMA threshold */

	if (privdata->num_chan > 1) { /* 32 bits DMA channel 0 local bus data width */
		advOutpDMAdw(privdata, 0x80, 0x00021f07);
	} else {      /* 16 bits DMA channel 0 local bus data width */
		advOutpDMAdw(privdata, 0x80, 0x00021f05);
	}
	advOutpDMAdw(privdata, 0x90,
		     (INT32U) (sgl_phyaddr | 0x09)); /* direction: local -> pci */

	
	advOutpDMA(privdata, 0xa8, 0x09); /* enable channel 0 to transfer data */
	advOutpDMA(privdata, 0xa8, 0x0b); /* start DMA transfer */


        /* clear & reset fifo */
	advOutpw(privdata, 0x10, 0x0303); 
	advOutpw(privdata, 0x12, 0x0303);

/* 	KdPrint("0x10: 0x%x\n", advInpw(privdata, 0x10)); */
/* 	KdPrint("0x12: 0x%x\n", advInpw(privdata, 0x12)); */
/* 	KdPrint("0x0e: 0x%x\n", advInpw(privdata, 0x0e)); */
/* 	KdPrint("0x1c: 0x%x\n", advInpw(privdata, 0x1c)); */

	tmp = advInpw(privdata, 0x0e) & 0xfff0;
	tmp |= (privdata->trig_mode + 1);
	advOutpw(privdata, 0x0e, tmp); /* set trig mode to start A/D */

	
	privdata->sgl_addr = sgl_addr;
	privdata->sgl_cnt = sgl_cnt;
	privdata->page_num = page_num;
	privdata->user_pages = user_pages;


	return 0;
}


/* static VOID adv_set_len(adv_device *device) */
/* { */
/* 	private_data *privdata = (private_data *) (device->private_data); */


/*         /\* all conv_num has bufferchangevent and it's exactly right *\/ */
/* 	if (privdata->num_chan == 1) { */
/* 		/\* need two byte per length 1 *\/ */
/* 		privdata->read_len1 = privdata->read_len2 = privdata->conv_num/2; */
/* 		if (privdata->conv_num <=8) { /\* AE=0,1,2,3 *\/ */
/* 			/\* avoid conv_num buffer span two pages and most in first page *\/ */
/* 			privdata->read_len3 = privdata->read_len1; */

/* 			/\* e.g 8=6+2, so if set length1,2,3: 4,4,0, AE:3, */
/* 			 * then 2 samples can't be got *\/ */
/* 			privdata->read_len2 = 0; */
/* 		} else {	/\* AE = 3 *\/ */
/* 			privdata->read_len2 -=4;  */
/* 			privdata->read_len3 =4;	/\* need soft read *\/ */
/* 		} */

/* 		/\* set almost empty according to different length1 *\/ */
/* 		switch (privdata->read_len1) { */
/* 		case 1: */
/* 			advOutpw(privdata, 0x14, 0);  */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 0);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 0);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 0);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 			break; */
/* 		case 2: */
/* 			advOutpw(privdata, 0x14, 1);  */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 1);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 1);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 1);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 			break; */
/* 		case 3: */
/* 			advOutpw(privdata, 0x14, 2);  */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 2);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 2);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 2);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 			break; */
/* 		default: */
/* 			advOutpw(privdata, 0x14, 3); */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 3);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 3);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 3);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 			break; */
/* 		} */
/* 	} /\* end if num_chan ==1 *\/ */
	   
/* 	/\* all have bufferchange *\/ */
/* 	if (privdata->num_chan == 2) { */
/* 		privdata->read_len1 = privdata->read_len2 = privdata->conv_num / 2; */
/* 		if ((privdata->read_len1 % 2) != 0) { */
/* 			privdata->read_len1++; */
/* 			privdata->read_len2--; */
/* 		} /\* len1 is set *\/ */

/* 		if (privdata->conv_num <= 14 ) {		    */
/* 			privdata->read_len2 = 0; */
/* 			privdata->read_len3 = privdata->conv_num - privdata->read_len1; */
/* 		} else {	/\* dwConvNum >=16 and set AE=3 *\/ */
/* 			privdata->read_len3 = 8; */
/* 			privdata->read_len2 -= 8; */
/* 		} */

/* 		/\* set almost empty according to different length1 *\/ */
/* 		switch (privdata->read_len1) { */
/* 		case 2: */
/* 			advOutpw(privdata, 0x14, 0);  */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 0);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 0);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 0);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 			break; */
/* 		case 4: */
/* 			advOutpw(privdata, 0x14, 1);  */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 1);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 1);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 1);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 			break; */
/* 		case 6: */
/* 			advOutpw(privdata, 0x14, 2);  */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 2);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 2);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 2);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 			break; */
/* 		default: */
/* 			advOutpw(privdata, 0x14, 3); */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 3);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 3);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 3);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 			break; */
/* 		}		    */
/* 	} /\* end of if num_chan == 2 *\/ */
   
/* 	if (privdata->num_chan == 4) { */
/* 		privdata->read_len1 = privdata->read_len2 = privdata->conv_num / 2; */
/* 		privdata->read_len3 = 0; */

/* 		if (privdata->read_len1 == 4) { */
/* 			privdata->read_len2 = 0; */
/* 			privdata->read_len3 = 4; */
/* 		} */

/*                 /\* it can't be same with 6,10,12...,because the interrupt */
/* 		 * comes too fast *\/ */
/* 		if(privdata->read_len1 == 8) { */
/* 			privdata->read_len2 = 0; */
/* 			privdata->read_len3 = 8; */
/* 		} */

/* 		if ((privdata->read_len1 == 6) || (privdata->read_len1 >= 10)) { */
/* 			privdata->read_len2 -= 6; */
/* 			privdata->read_len3 = 6; */
/* 		} */
	  
/* 		/\* set almost empty according to different length1 *\/ */
/* 		if (privdata->read_len1 <= 4) { */
/* 			advOutpw(privdata, 0x14, 0);  */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 0);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 0);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 0);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 		} else { */
/* 			advOutpw(privdata, 0x14, 1);  */
/* 			advOutpw(privdata, 0x14, 1); */
/* 			advOutpw(privdata, 0x16, 1);  */
/* 			advOutpw(privdata, 0x16, 1); */
/* 			advOutpw(privdata, 0x18, 1);  */
/* 			advOutpw(privdata, 0x18, 1); */
/* 			advOutpw(privdata, 0x1a, 1);  */
/* 			advOutpw(privdata, 0x1a, 1); */
/* 		} */
/* 	} /\* end of if num_chan == 4 *\/ */
/* } */


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
	INT32S ret;


	
	/* check if AI has been running by somebody else */
	if (privdata->fai_running) {
		return -EBUSY;
	}
	privdata->fai_running = 1;


	/* check parameters */
	if (copy_from_user(&dma_data, arg, sizeof(adv_dma_user))) {
		return -EFAULT;
	}

	privdata->start_chan = dma_data.startchan;
	privdata->num_chan = dma_data.numchan;
	privdata->divisor1 = dma_data.divisor1;
	privdata->divisor2 = dma_data.divisor2;
	privdata->user_buf = dma_data.userptr;
	privdata->conv_num = dma_data.datacnt;
	privdata->sample_rate = dma_data.samplerate;
	privdata->half_fifo_size = dma_data.half_fifo_size;
	privdata->clk_src = dma_data.trig_src;
	privdata->trig_mode = dma_data.trig_mode;
	privdata->cyclic = dma_data.cyclic;
	privdata->delay_cnt = dma_data.delay_cnt;
	
	privdata->halfready_flag = 0;
	privdata->half_ready = 0;
	privdata->fifo_af_flag = 0;
	privdata->dma_tc_flag = 0;
	privdata->transfered = 0;
	privdata->terminated = 0;
/* 	privdata->low_buf_flag = 0; */
/* 	privdata->high_buf_flag = 0; */
	

	/* check DMA status */
	ret = adv_check_dma(device);
	if (ret) {
		return ret;
	}
	
	/* init DMA */
	ret = adv_init_dma(device, (INT32U *) privdata->user_buf,
			   privdata->conv_num * sizeof(INT16U));
	if (ret) {
		return ret;
	}

	return 0;
}

	
/**
 * adv_fai_stop_work - schedued in tasklet, when fai terminated but region is not
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
     

     
	advOutpw(privdata, 0x0a, 0x0000); /* disable A/D */
	advOutpw(privdata, 0x20, 0x0000); /* disable all interrupt */
	
	tmp = advInpDMAdw(privdata, 0x80);
	advOutpDMAdw(privdata, 0x80, tmp & 0xfffffbff); /* clear DMA interrupt */

	tmp = advInpDMA(privdata, 0xa8) & 0x10;
	if (!tmp) { /* DMA transfer not complete, abort transfer */
		tmp = advInpDMA(privdata, 0xa8) | 0x04;
	}
	advOutpDMA(privdata, 0xa8, tmp & 0xfe);	
	advOutpDMAw(privdata, 0x68, 0x0000);

	/* set trig mode to SW */
	advOutpw(privdata, 0x0e, advInpw(privdata, 0x0e) & 0xfff0);
	
	if (privdata->user_pages) {
		adv_unmap_user_pages(privdata->user_pages,
				     privdata->page_num,
				     1);
	}
	

	if (privdata->sgl_addr) {
		kfree(privdata->sgl_addr);
	}

	/* set terminate event */
	adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
/* 	/\* terminate the tasklet *\/ */
/* 	tasklet_kill(&privdata->fai_tasklet); */

	privdata->user_pages = 0;
	privdata->sgl_addr = 0;
	privdata->buf_stat = 0;
	privdata->item = 0;
	privdata->page_index = 0;
	privdata->overrun_flag = 0;
	privdata->terminal_cnt = 0;
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
