/************************************************************
 *				                                   
 * 	Copyright 2004 IAG Software Team,                  
 *                                                                  
 * 		BeiJing R&D Center                         
 *	                                                           
 * 		Advantech Co., Ltd.                        
 *                                                                  
 *     Advantech PCI-1718 Device driver for Linux             
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                 
 * 	advai.c                                            
 * Abstract:                                                  
 * 	This file contains routines for ai function.       
 *                                                                  
 *                                                                  
 * Version history                                            
 *      07/14/2006			Create by Zhiyong.Xie
 *                                                                  
 ************************************************************/
  
#define __NO_VERSION__
#include "PCI1718.h"


static INT32S adv_gp_common_fai_start(adv_device *device, INT16U trig_src, INT32U divisor1, INT32U divisor2, INT16U num_chan, INT16U start_chan, INT16U *gain_list, INT16U *buffer, INT32U count, INT16U cyclic, INT16U intr_count, INT16U fai_mode)
{
	private_data *privdata = NULL;
	INT32U event_threshold;
	INT32S ret = 0;
	
	privdata = (private_data *)device->private_data;

	if (privdata->fai_running) {
		return -EBUSY;
	}
	privdata->fai_running = 1;
	
	if(privdata->ai_stop == 0) {
		return -EBUSY;
	}

	if(adv_process_info_isenable_event(&privdata->ptr_process_info, 4)){
		event_threshold = adv_process_info_get_special_event_thresh
			(&privdata->ptr_process_info, 4);
		if( ((event_threshold/intr_count) == 0) 
		    ||((event_threshold%intr_count) != 0)){
			return -EFAULT;
		}
	}

	if (privdata->user_pages != NULL) {
		adv_unmap_user_pages(privdata->user_pages,
				     privdata->page_num, 1);
		privdata->user_pages = 0;
	}

	adv_map_user_pages(&privdata->user_pages, &privdata->page_num,
			   (PTR_T)buffer, count, WRITE);
	if(privdata->user_pages == NULL){
		return -EFAULT;
	}

	
	privdata->start_chan = start_chan;
//	privdata->stop_chan = stop_chan;
	privdata->num_chan = num_chan;
	privdata->buf_size = count;
	privdata->divisor1 = divisor1;
	privdata->divisor2 = divisor2;
	privdata->ai_mode = (intr_count == privdata->half_fifo_size) ? 0x02 : 0x01;
	privdata->buf_ready = 0;
	privdata->page_index = 0;
	privdata->cell_index = 0;
	privdata->ai_stop = 0;
	privdata->current_cell = 0;
	privdata->start = 0;
	privdata->cyclic = cyclic;
	privdata->over_run = 0;
	
	return 0;
}




/**************************************************************
 * Description:  Initiates an asynchronous, multiple-channel
 *               data acquisition
 *
 *               step1. clear control register
 *               step2. set counter2 & counter1 to mode 2
 *               step3. write counter 2
 *                      (low byte & high byte)
 *               step4. write counter 1
 *                      (low byte & high byte)
 *
 *               0x06   -set 'control register'
 *               0x1e   -set 'counter operation mode'
 *               0x1c   -'counter2'
 *               0x1a   -'counter1'
 *
 * Input:        intdata -point to the INT mode ai data
 * Ouput:        void
 *
 **************************************************************/
INT32S adv_ai_int_scan_start(adv_device *device, void *arg)
{
	adv_int_user int_data;
	INT16U fai_mode;
	
	if (copy_from_user(&int_data, (adv_int_user *)arg, sizeof(adv_int_user))){
		return -EFAULT;
	}
	
	if (int_data.numchan == 1) {
		fai_mode = MODE_INT;
	} else {
		fai_mode = MODE_INT_SCAN;
	}
	
	
	return adv_gp_common_fai_start(device, int_data.trig_src, int_data.divisor1,
				       int_data.divisor2, int_data.numchan, int_data.startchan, 
				       int_data.gainlist, int_data.userptr, 
				       int_data.bufsize, int_data.cyclic,
				       int_data.intrcount, fai_mode);
}

/**************************************************************
 * Description:  stop data acquisition and release the region for
 *               user call
 *
 * Input:        device -point to the device object
 * Ouput:        errno
 *               SUCCESS
 **************************************************************/
INT32U adv_ai_int_terminate(private_data *privdata)
{
	INT32U pre_stop_status;
     
	pre_stop_status = privdata->ai_stop;
	privdata->ai_stop = 1;
     
	if(privdata->ai_mode == 0x01){
		advOutp(privdata, 9, 0); /* disable interrupt */
		advOutp(privdata, 8, 0);
	}
	if(privdata->ai_mode == 0x02){
		advOutp(privdata, 6, 0); /* disable FIFO interrupt */
		advOutp(privdata, 9, 0); /* disable interrupt */
		advOutp(privdata, 20, 0); /* clear interrupt request */
	}
     
	tasklet_kill(&privdata->tasklet);

	if(!pre_stop_status){
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
	}
	
	if (privdata->user_pages) {
		adv_unmap_user_pages(privdata->user_pages, privdata->page_num, 1);
		privdata->user_pages = 0;
	}

	privdata->fai_running = 0;
	
	return 0;
}

/**************************************************************
 * Description:  stop data acquisition for bottom half call
 *
 * Input:        device -point to the device object
 * Ouput:        errno
 *               SUCCESS
 **************************************************************/
INT32U adv_ai_int_stop(private_data *privdata)
{
	INT32U pre_stop_status;
     
	pre_stop_status = privdata->ai_stop;
	privdata->ai_stop = 1;
     
	if(privdata->ai_mode == 0x01){
		advOutp(privdata, 9, 0); /* disable interrupt */
		advOutp(privdata, 8, 0);
	}
	if(privdata->ai_mode == 0x02){
		advOutp(privdata, 6, 0); /* disable FIFO interrupt */
		advOutp(privdata, 9, 0); /* disable interrupt */
		advOutp(privdata, 20, 0); /* clear interrupt request */
	}
     
	tasklet_kill(&privdata->tasklet);

	if(!pre_stop_status){
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
	}

	privdata->fai_running = 0;
	
	return 0;
}

INT32S adv_ai_check(adv_device *device, void *arg)
{
	PT_FAICheck fai_check;
	private_data *privdata = (private_data *) (device->private_data);

	if (copy_from_user(&fai_check, arg, sizeof(PT_FAICheck))) {
		return -EFAULT;
	}

	fai_check.HalfReady = privdata->buf_ready;
	fai_check.ActiveBuf = 0;
	fai_check.stopped = privdata->ai_stop;
	fai_check.retrieved = privdata->current_cell;
	fai_check.overrun = privdata->over_run;
	
	if (copy_to_user(arg, &fai_check, sizeof(PT_FAICheck))) {
		return  -EFAULT;
	}

	return 0;
}

INT32S adv_fai_transfer(adv_device *device, PT_FAITransfer *lpFAITransfer)
{
	PT_FAITransfer fai_transfer;
	private_data *privdata = (private_data *) (device->private_data);
	
	if (copy_from_user(&fai_transfer,lpFAITransfer, sizeof(PT_FAITransfer))) {
		return  -EFAULT;
	}
	
	if (copy_to_user(fai_transfer.overrun, &(privdata->over_run), sizeof(privdata->over_run))) {
		return -EFAULT;
	}
	privdata->over_run = 0;
	privdata->buf_ready = 0;
	return 0;
}

INT32S adv_clear_flag(adv_device *device, INT32U EventID)
{
	private_data *privdata = (private_data *) (device->private_data);

	switch (EventID) {
	case ADS_EVT_AI_LOBUFREADY:
	case ADS_EVT_AI_HIBUFREADY:
		privdata->buf_ready = 0;
		break;
	case ADS_EVT_AI_OVERRUN:
		privdata->over_run = 0;
		break;
	}
     
	return 0;
}



