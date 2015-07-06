/****************************************************************************
 *		Copyright 2006 ABJ Linux Driver Team, 
 *
 *			BeiJing R&D Center 
 *
 *			Advantech Co., Ltd.
 *
 *	Advantech USB-4716 Device driver for Linux
 *
 * File Name: 
 *	advai.c
 * Abstract:
 *	This file contains routines for ai function.
 * Version history
 *      12/06/2006              Created by LI.ANG
 ************************************************************************/	
#include "USB4716.h"

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
	URB_QUEUE *urb_queue = NULL;
	INT16U i;
	INT32S ret;

	
	/* get user buffer pages */
	switch (privdata->device_type) {
	case USB4716:		/* usb-4716 use 16-bits data width */
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
		return -EFAULT;
	}

	/* caculate urb numbers */
	privdata->half_buf_len = privdata->conv_num;
	privdata->urb_num = ((privdata->half_buf_len +  privdata->max_urbbuf_len - 1)
			     / privdata->max_urbbuf_len) * 2;

	
	/* allocate new urb queue according to the new nub number */
	privdata->urb_queue = (URB_QUEUE *) kmalloc(sizeof(URB_QUEUE) * privdata->urb_num,
						   GFP_KERNEL);
	if (!privdata->urb_queue) {
		return -ENOMEM;
	}
	urb_queue = privdata->urb_queue;

	/* initiallize the urb queue */
	for (i = 0; i < privdata->urb_num; i++) {
		urb_queue[i].idx = i;
		urb_queue[i].privdata = (VOID *) privdata;

		urb_queue[i].kern_cache = kmalloc(privdata->max_urbbuf_len, GFP_KERNEL);
		if (!urb_queue[i].kern_cache) {
			return -ENOMEM;
		}		

		/* caculate urb buffer length */
		if (privdata->half_buf_len > privdata->max_urbbuf_len) {
			urb_queue[i].urb_buf_len = privdata->max_urbbuf_len;
		} else {
			urb_queue[i].urb_buf_len = privdata->half_buf_len;
		}
		
		privdata->half_buf_len -= urb_queue[i].urb_buf_len;
		if (privdata->half_buf_len <= 0) {
			privdata->half_buf_len = privdata->conv_num;
		}
		
		/* construct int urbs */
		urb_queue[i].urb = adv_usb_alloc_urb(0, GFP_KERNEL);
		if (!urb_queue[i].urb) {
			return -ENOMEM;
		}
		
		if (privdata->udev == NULL) {
			return -ENODEV;
		}

		usb_fill_bulk_urb(urb_queue[i].urb, privdata->udev,
				 usb_rcvbulkpipe(privdata->udev, privdata->bulk_endpointAddr),
				 (INT8U *) urb_queue[i].kern_cache,
				 urb_queue[i].urb_buf_len,
				 (usb_complete_t) urb_callback_fn,
				 &urb_queue[i]);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
		urb_queue[i].urb->transfer_flags |= USB_QUEUE_BULK;
#endif
/* 		usb_fill_int_urb(urb_queue[i].urb, privdata->udev, */
/* 				 usb_rcvintpipe(privdata->udev, privdata->int_endpointAddr), */
/* 				 (INT8U *) urb_queue[i].kern_cache, */
/* 				 urb_queue[i].urb_buf_len, */
/* 				 (usb_complete_t) urb_callback_fn, */
/* 				 &urb_queue[i], */
/* 				 1); */
	}

	for (i = 0; i < privdata->urb_num; i++) {
		if (privdata->udev == NULL) {
			return -ENODEV;
		}

		ret = adv_usb_submit_urb(urb_queue[i].urb, GFP_KERNEL);
		if (ret) {
			return ret;
		}
	}
/* 	for (i = 0; i < privdata->urb_num; i++) { */
/* 		ret = adv_usb_submit_urb(urb_queue[i].urb, GFP_KERNEL); */
/* 		if (ret) { */
/* 			return ret; */
/* 		} */
/* 	} */

	return 0;
}

static VOID adv_fai_terminate_device(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT32U tmp;
	INT32S ret;
	

	/* send command to device to stop fai */
	if (down_interruptible(privdata->usb_urb_sema)) {
		return;
	}

	if (privdata->udev == NULL) {
		up(privdata->usb_urb_sema);
		return;
	}

	ret = adv_usb_ctrl_msg(privdata->udev, usb_sndctrlpipe(privdata->udev, 0),
			       MAJOR_AI, 0x40,
			       MINOR_FAI_TERMINATE, 0,
			       (INT8U *) &tmp,
			       sizeof(INT32U));
	if (ret < 0) {
		up(privdata->usb_urb_sema);
		return;
	}

	up(privdata->usb_urb_sema);

	return;
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
	

	
	/* for multiple user process */
	if (privdata->fai_running) {
		return -EBUSY;
	}
	privdata->fai_running = 1;


	if (copy_from_user(&int_data, (adv_int_user *) arg, sizeof(adv_int_user))) {
		privdata->fai_running = 0;
		return -EFAULT;
	}

	privdata->terminated = 0;
	privdata->recieve_cnt = 0;
	privdata->half_ready = 0;
	privdata->halfready_flag = 0;

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
		privdata->fai_running = 0;
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
	

	/* for multiple user process */
	if (privdata->fai_running) {
		return -EBUSY;
	}
	privdata->fai_running = 1;

	if (copy_from_user(&int_data, (adv_int_user *) arg, sizeof(adv_int_user))) {
		privdata->fai_running = 0;
		return -EFAULT;
	}

	privdata->terminated = 0;     
	privdata->recieve_cnt = 0;
	privdata->half_ready = 0;
	privdata->halfready_flag = 0;

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
		privdata->fai_running = 0;
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
	INT16U i;

     
	/* terminate the urb and release the kernel cache */
	if (privdata->urb_queue != NULL) {
		for (i = 0; i < privdata->urb_num; i++) {
			if (privdata->urb_queue[i].urb != NULL) {
				usb_kill_urb(privdata->urb_queue[i].urb);
				usb_free_urb(privdata->urb_queue[i].urb);
				privdata->urb_queue[i].urb = NULL;
			}
		
			if (privdata->urb_queue[i].kern_cache != NULL) {
				kfree(privdata->urb_queue[i].kern_cache);
				privdata->urb_queue[i].kern_cache = NULL;
			}
		}
	}
	
	/* terminate the tasklet */
	tasklet_kill(&privdata->urb_tasklet);

	/* release the urb queue */
	if (privdata->urb_queue != NULL) {
		kfree(privdata->urb_queue);
		privdata->urb_queue = NULL;
	}
	
	/* unmap user buffer pages */
	if (privdata->user_pages) {
		adv_unmap_user_pages(privdata->user_pages,
				     privdata->page_num,
				     1);
	}

	/* clear data */
	privdata->user_pages = NULL;
	privdata->hungry_page = NULL;
	privdata->page_idx = 0;
	privdata->page_num = 0;
	privdata->urb_num = 0;
	privdata->terminated = 1;
	privdata->overrun_flag = 0;
	privdata->fifo_overrun = 0;
	privdata->fai_running = 0;

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
	adv_fai_terminate_device((adv_device *) data);
}

/**
 * adv_fai_check - check status of interrupt mode AI
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
	fai_check.retrieved = privdata->recieved;
	fai_check.overrun = privdata->overrun_flag;
	fai_check.HalfReady = privdata->half_ready;

	if (copy_to_user(arg, &fai_check, sizeof(PT_FAICheck))) {
		return -EFAULT;
	}

	return 0;
}
