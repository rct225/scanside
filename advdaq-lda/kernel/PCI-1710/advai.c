/****************************************************************************
 *		Copyright 2006 ABJ Linux Driver Team, 
 *
 *			BeiJing R&D Center 
 *
 *			Advantech Co., Ltd.
 *
 *	Advantech PCI-1710 Device driver for Linux
 *
 * File Name: 
 *	advai.c
 * Abstract:
 *	This file contains routines for ai function.
 * Version history
 *      12/06/2006              Created by LI.ANG
 ************************************************************************/	
#include "PCI1710.h"


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
static VOID adv_pacer_setup(private_data *privdata, INT8U rate)
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
     	advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x0040);

	advOutpw(privdata, 0x1e, 0x0074);	
	advOutpw(privdata, 0x1a, div1 & 0x00ff);
	advOutpw(privdata, 0x1a, ((div1 & 0xff00) >> 8));

	advOutpw(privdata, 0x1e, 0x00b4); 
	advOutpw(privdata, 0x1c, div2 & 0x00ff);
	advOutpw(privdata, 0x1c, ((div2 & 0xff00) >> 8));
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
	if ((data_cnt / 2) % 4) { /* We can ensure that top half buffer length is a
				   * multiple of 2, and bottom half buffer is a mu-
				   * ltiple of 2.
				   * 
				   * Top half buffer must same as bottom half buffer,
				   * so we need to ensure that any half buffer length
				   * is a multiple of 4 */
		return -EINVAL;
	}
	
        adv_map_user_pages(&privdata->user_pages,
			   &privdata->page_num,
			   (PTR_T) user_buf,
			   (PTR_T) data_cnt,
			   WRITE);
	if (!privdata->user_pages) {
		return -EFAULT;
	}
	
	/* clear & disable interrupt */
	/* advOutpDMAw(privdata, 0x68, 0x0000); */
	advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x0040);
	advOutpw(privdata, 0x08, 0x0000);
	/* advOutpDMAw(privdata, 0x68, 0x0900); */
	

	/* set control register for interrupt handle */
	tmp = advInpw(privdata, 0x06) & 0x0040;
	/* KdPrint("0x06: 0x%.4x\n", tmp); */
	if (privdata->clk_src) { /* external mode: waitting for external trigger */
		if (privdata->int_cnt == 1) {
			privdata->fifo_enable = 0;
			advOutpw(privdata, 0x06, tmp | 0x001c);
		} else {
			privdata->fifo_enable = 1;
			advOutpw(privdata, 0x06, tmp | 0x003c);
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
		/* KdPrint("0x06: 0x%.4x\n", tmp); */
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
	INT32S ret;
	

/* 	/\* check if AI has been running by somebody else *\/ */
/* 	down_interruptible(privdata->fai_sema); */

	if (copy_from_user(&int_data, (adv_int_user *) arg, sizeof(adv_int_user))) {
		return -EFAULT;
	}

	privdata->terminated = 0;     
	privdata->halfready_flag = 0;
	privdata->transfered = 0;
	/* privdata->terminal_cnt = 0; */
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

	privdata->fai_running = 1;
	
	
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
	

/* 	/\* check if AI has been running by somebody else *\/ */
/* 	down_interruptible(privdata->fai_sema); */
	
	if (copy_from_user(&int_data, (adv_int_user *) arg, sizeof(adv_int_user))) {
		return -EFAULT;
	}

	privdata->terminated = 0;     
	privdata->halfready_flag = 0;
	privdata->transfered = 0;
	/* privdata->terminal_cnt = 0; */
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

/* 	printk("conv_num: %d\n", privdata->conv_num); */
/* 	printk("user_buf: %p\n", privdata->user_buf); */
        /* init Interrupt mode ai */
	ret = adv_init_int(device,
			   (INT32U *) privdata->user_buf,
			   privdata->conv_num * sizeof(INT16U));
	if (ret) {
		return ret;
	}
	
	privdata->fai_running = 1;
	

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
	INT8U ctr_byte;
	
     
	/* KdPrint("adv_fai_stop!\n"); */

	if(privdata->fai_running) {

		privdata->fai_running = 0;
		advOutpw(privdata, 0x06, advInpw(privdata, 0x06) & 0x0040);
		advOutpw(privdata, 0x08, 0x0000);


		if (privdata->user_pages) {
			adv_unmap_user_pages(privdata->user_pages,
					     privdata->page_num,
					     1);
		}

		adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);

		/* reset counter pulse out */
		ctr_byte = 0x01 << 6;	/* counter 1 */
		ctr_byte |= 0x39;
		advOutp(privdata, 0x1e, ctr_byte);
		advOutp(privdata, (0x01 * 2 + 0x18), 0); /* write the low byte */
		advOutp(privdata, (0x01 * 2 + 0x18), 0); /* write the high byte */

		ctr_byte = 0x02 << 6;	/* counter 2 */
		ctr_byte |= 0x39;
		advOutp(privdata, 0x1e, ctr_byte);
		advOutp(privdata, (0x02 * 2 + 0x18), 0); /* write the low byte */
		advOutp(privdata, (0x02 * 2 + 0x18), 0); /* write the high byte */

	
	}
	
	
	privdata->user_pages = 0;
	privdata->buf_stat = 0;
	privdata->item = 0;
	privdata->cur_index = 0;
	privdata->page_index = 0;
	privdata->overrun_flag = 0;
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
	
/* 	if(privdata->fai_running) { */

		fai_check.ActiveBuf = 0;
		fai_check.stopped = privdata->terminated;
		fai_check.retrieved = privdata->transfered;
		fai_check.overrun = privdata->overrun_flag;
		fai_check.HalfReady = privdata->half_ready;

		if (copy_to_user(arg, (void *) &fai_check, sizeof(PT_FAICheck))) {
			return -EFAULT;
		}
/* 	} */
	

	return 0;
}

INT32S adv_get_fai_status(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	
	if (copy_to_user((void *)arg, &privdata->fai_running, sizeof(privdata->fai_running))) {
		return -EFAULT;
	}

	return SUCCESS;
	
}
