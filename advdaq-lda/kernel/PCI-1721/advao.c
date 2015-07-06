/********************************************************************
 *				                                   
 * 		Copyright 2004 IAG Software Team,                  
 *                                                                  
 * 			BeiJing R&D Center                         
 *	                                                           
 * 			Advantech Co., Ltd.                        
 *                                                                  
 * 	    Advantech PCI-1721 Device driver for Linux             
 *                                                                  
 *                                                                  
 *                                                                  
 * 	File Name:                                                 
 * 		advao.c                                            
 * 	Abstract:                                                  
 * 		This file contains routines for ao function.       
 *                                                                  
 *                                                                  
 * 	Version history                                            
 * 	01/09/2006			Create by LI.ANG           
 *                                                                  
 ********************************************************************/
#include "PCI1721.h"
  
  

/**
 * adv_check_dma - check DMA status until the DMA transfer terminated
 *
 * @device: Points to the device object
 */
static INT32S adv_check_dma(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U *trash_buf = NULL;
	INT32U tb_phyaddr;
	INT16U tmp;
	INT16U timeout;
	


	tmp = advInpDMAw(privdata, 0xa8);
	tmp &= 0x10;
	if (tmp != 0x10) {
		/* initialize PCI9054 */
		trash_buf = kmalloc(256, GFP_DMA);
		if (!trash_buf) {
			return -ENOMEM;
		}
		tb_phyaddr = virt_to_phys((void *) trash_buf);

		advOutpDMAw(privdata, 0x04, 0x0001); /* enable pci addresses */
		advOutpDMAdw(privdata, 0x08, 0x0000);
		advOutpDMAdw(privdata, 0x0a, 0x0024);
	
		advOutpDMAw(privdata, 0x68, 0x0000);
		advOutpDMAw(privdata, 0x6a, 0x0000);
		advOutpDMAw(privdata, 0x18, 0x000d); /* 32 bits width */
		advOutpDMAw(privdata, 0xb0, 0x0000);

		advOutpDMAdw(privdata, 0x80, 0x1d0d);
		advOutpDMAdw(privdata, 0x82, 0x0002);
		advOutpDMAdw(privdata, 0x84, tb_phyaddr);
		advOutpDMAdw(privdata, 0x88, 0x0040);
		advOutpDMAdw(privdata, 0x8c, 0x0080);
		advOutpDMAdw(privdata, 0x90, 0x0003);
		advOutpDMAdw(privdata, 0x92, 0x0000);


		/* init divisor */
		advOutpw(privdata, 0x36, 0x0076);
		advOutpw(privdata, 0x32, privdata->divisor2);
		advOutpw(privdata, 0x32, (privdata->divisor2 >> 8));

		advOutpw(privdata, 0x36, 0x00b6);
		advOutpw(privdata, 0x34, privdata->divisor1);
		advOutpw(privdata, 0x34, (privdata->divisor1 >> 8));

		
		advOutpw(privdata, 0x2a, 0x0000);
		advOutpw(privdata, 0x2c, 0x0000);
		
		advOutpDMAdw(privdata, 0xa8, 0x0009);
		advOutpDMAdw(privdata, 0xa8, 0x0009);

		tmp = advInpDMAw(privdata, 0xa8);
		tmp &= 0x10;
		while (tmp != 0x10) {
			tmp = advInpDMAw(privdata, 0xa8);
			tmp &= 0x10;
			advOutpw(privdata, 0x2c, 0x00);
		}
		advOutpw(privdata, 0x2c, 0x00);
	}	

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

	

	switch (privdata->device_type) {
	case PCI1721:		/* pci-1721 use 12-bits data width */
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
		return -ENODEV;
	}

	
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
		/* KdPrint("Allocate sgl list error\n"); */
		return -ENOMEM;
	}
	sgl_phyaddr = virt_to_phys((void *) sgl_addr);


        /* fill the top half buffer into the SGL table */
	for (j = i = 0; i < page_num; i++, j++) {
		virt_addr = user_pages[i].page_addr + user_pages[i].offset;
		phy_addr = virt_to_phys((void *) virt_addr);
		cur_len = user_pages[i].length;
		
		if (((sgl_cnt + 1) * 16) > sgl_size) {
			/* KdPrint("sgl table overflow !\n"); */
			return -EFAULT;
		}

		sgl_addr[j * 4] = phy_addr;
		sgl_addr[j * 4 + 1] = 0x0040;
		sgl_addr[j * 4 + 2] = cur_len;
		sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x01);


		/* half buffer cause interrupt */
		if (total_len + cur_len == privdata->conv_num) {
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x05);
		}

		/* page unalign case */
		if ((total_len < privdata->conv_num)
		    && (total_len + cur_len > privdata->conv_num)) {

			/* for top half page, cause interrupt */
			top_len = privdata->conv_num - total_len;
			j = i;

			sgl_addr[j * 4] = phy_addr;
			sgl_addr[i * 4 + 1] = 0x0040;
			sgl_addr[j * 4 + 2] = top_len;
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x05);

			j += 1;
			sgl_cnt++;

			/* for bottom half page */
			bot_len = cur_len - top_len;
			virt_addr = user_pages[i].page_addr + user_pages[i].offset + top_len;
			phy_addr = virt_to_phys((void *) virt_addr);

			sgl_addr[j * 4] = phy_addr;
			sgl_addr[i * 4 + 1] = 0x0040;
			sgl_addr[j * 4 + 2] = bot_len;
			sgl_addr[j * 4 + 3] = (INT32U) ((sgl_phyaddr + ((j + 1) * 16)) | 0x01);
		}

		sgl_cnt++;
		total_len += cur_len;
	}
	
	if (privdata->cyclic) {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) (sgl_phyaddr | 0x05);
	} else {
		sgl_addr[(j - 1) * 4 + 3] = (INT32U) ((sgl_phyaddr + (i * 16)) | 0x07);
	}

	/* initialize PCI9054 */
	advOutpDMAw(privdata, 0x04, 0x0001); /* enable pci addresses */
	advOutpDMAdw(privdata, 0x08, 0x0000);
	advOutpDMAdw(privdata, 0x0a, 0x0024);
	
	advOutpDMAw(privdata, 0x68, 0x0180); /* enable PCI int(bit 8) & LOCAL int(bit 11) */
	advOutpDMAw(privdata, 0x6a, 0x0004);
	advOutpDMAw(privdata, 0x18, 0x000d); /* 32 bits width */
	advOutpDMAw(privdata, 0xb0, 0x0000);

	advOutpDMAdw(privdata, 0x80, 0x1b09 + 0x4400);
	advOutpDMAdw(privdata, 0x82, 0x0002);
	advOutpDMAdw(privdata, 0x90, sgl_phyaddr | 0x01); /* can also set direction */


	
	privdata->sgl_addr = sgl_addr;
	privdata->sgl_cnt = sgl_cnt;
	privdata->page_num = page_num;
	privdata->user_pages = user_pages;


	return 0;
}

static INT32S adv_init_local_reg(adv_device *device, USHORT trig_src, DWORD sample_rate)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U iocs_mask;

	iocs_mask = advInpw(privdata, 0x2a);
     
	if (trig_src) {
		iocs_mask |= 0x0060;
	} else {
		if (sample_rate == 5000000) {	/* 5M */
			iocs_mask &= 0x001f;
			iocs_mask |= 0x0020;
		} else if (sample_rate == 10000000) {	/* 10M */
			iocs_mask &= 0x001f;
			iocs_mask |= 0x0040;
		} else {
			advOutpw(privdata, 0x36, 0x0076);
			advOutpw(privdata, 0x32, privdata->divisor2);
			advOutpw(privdata, 0x32, (privdata->divisor2 >> 8));

			advOutpw(privdata, 0x36, 0x00b6);
			advOutpw(privdata, 0x34, privdata->divisor1);
			advOutpw(privdata, 0x34, (privdata->divisor1 >> 8));

			iocs_mask &= 0x001f;
		}
	}
	advOutpw(privdata, 0x2c, 0x0000);
     
	advOutpDMAw(privdata, 0xa8, 0x0009);
	advOutpDMAw(privdata, 0xa8, 0x000b);
     
	advOutpw(privdata, 0x2a, iocs_mask);

	return 0;
}


INT32S adv_fao_dma_start(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	adv_dma_user dma_data;
	INT32S ret;
     

	
	if (privdata->ao_running) {
		/* printk("AO has been run by someone else...\n"); */
		return -EBUSY;
	}
	privdata->ao_running = 1;
	
	
	if (copy_from_user(&dma_data, arg, sizeof(adv_dma_user))) {
		/* KdPrint("copy_from_user failed !\n"); */
		return -EFAULT;
	}
     
	/* set normal ao mode flag */
	privdata->ao_mode_flag = NORMAL_AO_MODE;
	privdata->low_buf_flag = 0;
	privdata->high_buf_flag = 0;
	privdata->terminate_flag = 0;
	privdata->half_ready = 0;
	
/*      /\* set reference source and polar *\/ */
/*      enabled_chan = adv_set_ref_ctrl(device, &fao_dma_ex_start, NULL); */

	/* save some parameters */
	privdata->divisor1 = dma_data.divisor1;
	privdata->divisor2 = dma_data.divisor2;
	privdata->cyclic = dma_data.cyclic;
	privdata->conv_num = dma_data.datacnt;
	privdata->user_buf = dma_data.userptr;
     
     
     	/* check DMA status */
	ret = adv_check_dma(device);
	if (ret) {
		/* KdPrint("ret: %d\n", ret); */
		return ret;
	}

	/* initialize DMA */
	ret = adv_init_dma(device, (INT32U *) privdata->user_buf,
			   privdata->conv_num * SAMPLE_DATA_LENGTH);
	if (ret) {
		return ret;
	}

	/* initialize local register, prepare to start DMA ao */
	ret = adv_init_local_reg(device, dma_data.trig_src, dma_data.samplerate);
	if (ret) {
		return ret;
	}

	return 0;
}


INT32S adv_fao_check(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_FAOCheck fao_check;
     


	if (copy_from_user(&fao_check, arg, sizeof(PT_FAOCheck))) {
		/* KdPrint("copy_from_user failed !\n"); */
		return -EFAULT;
	}

	/* spin_lock(&privdata->spinlock); */

	fao_check.ActiveBuf = 0;
	fao_check.CurrentCount = 0; /* for waveform count, but not used */
	fao_check.overrun = privdata->underrun_flag; /* only in linux */
	fao_check.HalfReady = privdata->half_ready;
	if (privdata->terminate_flag) {
		fao_check.stopped = 1;
	} else {
		fao_check.stopped = 0;
	}

	/* spin_unlock(&privdata->spinlock); */

	if (copy_to_user(arg, &fao_check, sizeof(PT_FAOCheck))) {
		/* KdPrint("copy_to_user failed !\n"); */
		return -EFAULT;
	}
     

	return 0;
}


/**
 * adv_fao_stop - schedued in tasklet, when fao terminated but region is not
 *                released and device is not stopped.
 *
 * @device: Point to the device object
 */
VOID adv_fao_stop_work(VOID *data)
{
	adv_fao_stop((adv_device *) data);
}

/**
 * adv_fao_stop - stop high speed ai(DMA & interrupt mode), and release
 *                allocated regions
 *
 * @device: Point to the device object
 */
INT32S adv_fao_stop(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U tmp;
     

	
	tmp = advInpDMAdw(privdata, 0x80);
	advOutpDMAdw(privdata, 0x80, tmp & 0xfffffbff); /* clear DMA interrupt */

	tmp = advInpDMA(privdata, 0xa8) & 0x10;
	if (!tmp) { /* DMA transfer not complete, abort transfer */
		tmp = advInpDMA(privdata, 0xa8) | 0x04;
	}
	advOutpDMA(privdata, 0xa8, tmp & 0xfe);	
	advOutpDMAw(privdata, 0x68, 0x0000);
	advOutpw(privdata, 0x2c, 0x0000); /* clear fifo */

	
	if (privdata->user_pages) {
		adv_unmap_user_pages(privdata->user_pages,
				     privdata->page_num,
				     1);
	}
	

	if (privdata->sgl_addr) {
		kfree(privdata->sgl_addr);
	}

	adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);

	privdata->terminate_flag = 1;
	privdata->user_pages = 0;
	privdata->sgl_addr = 0;
	privdata->buf_stat = 0;
	privdata->item = 0;
	privdata->cur_index = 0;
	privdata->page_index = 0;
	privdata->underrun_flag = 0;
	privdata->terminal_cnt = 0;
	privdata->ao_running = 0;
	privdata->half_ready = 0;
	

	return 0;
}

