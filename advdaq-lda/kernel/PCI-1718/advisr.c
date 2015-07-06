/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1718 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1718 driver module and device. 
Version history
	05/25/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
//#include <linux/config.h>
#include <linux/module.h>

#include "PCI1718.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif

INT32S pci1718_nofifo_isr(private_data *privdata)
{
	INT16U data;
	BYTE status;
	adv_user_page *page = NULL;
	
/* 	status = advInp(privdata, 8); */
/* 	if( !(status & 0x10) ){ */
/* 		return 0; */
/* 	} */
	
	data = advInp(privdata, 0) & 0xff;
	data |=  (advInp(privdata, 1) << 8) & 0xff00;

	page = privdata->user_pages + privdata->page_index;
	
	memcpy( (INT16U *)(page->page_addr + page->offset + privdata->cell_index),
		&data, sizeof(INT16U));

	
	privdata->cell_index += 2;
	privdata->current_cell++;

	if( privdata->cell_index >= page->length){
		privdata->page_index++;
		privdata->cell_index = 0;
		privdata->page_index = privdata->page_index%privdata->page_num;

	}
	/* set ADS_EVT_INTERRUPT */
	adv_process_info_set_event_all(&privdata->ptr_process_info, 4, 1);

	/* set ADS_EVT_AI_LOBUFREADY */
	if( privdata->current_cell == privdata->buf_size/4 ){
		if (privdata->buf_ready) {
			privdata->over_run = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
		
		privdata->buf_ready = 1;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}

	/* set ADS_EVT_AI_HIBUFREADY */
	if( privdata->current_cell == privdata->buf_size/2 ){
		if (privdata->buf_ready) {
			privdata->over_run = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
		privdata->buf_ready = 2;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
	}
	
	/* set ADS_EVT_AI_TERMINATED */
	if( (!privdata->cyclic) && (privdata->current_cell == privdata->buf_size/2)){
		privdata->ai_stop = 1;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
		run_task_queue(&privdata->tq_fai_stop);
#else
		schedule_work(&privdata->fai_stop);
#endif
	}

	privdata->current_cell = privdata->current_cell%(privdata->buf_size/2);	

	
	return 1;
}

INT32S pci1718_fifo_isr(private_data *privdata)
{
	INT16U data;
	BYTE status;
	adv_user_page *page = NULL;
	INT32U i;
	
 	status = advInp(privdata, 25); 
 	if( !(status & 0x02) ){ 
 		return 0; 
	} 
	
	if(privdata->ai_stop){
		advOutp(privdata, 6, 0); /* disable FIFO interrupt */
		advOutp(privdata, 9, 0); /* disable interrupt */
		advOutp(privdata, 20, 0); /* clear interrupt request */
		return 1;
	}
 LOOP:
	i = 0;
	
	do {
		data = advInp(privdata, 23)  & 0x00ff;
		data |=  (advInp(privdata, 24) << 8) & 0xff00;

		
		page = privdata->user_pages + privdata->page_index;
	
		memcpy( (INT16U *)(page->page_addr + page->offset + privdata->cell_index),
			&data, sizeof(INT16U));
		
		privdata->cell_index += 2;
		privdata->current_cell++;
		i++;

		if (privdata->cell_index >= page->length){
			privdata->page_index++;
			privdata->cell_index = 0;
			privdata->page_index = privdata->page_index%privdata->page_num;
		}
		
	} while( (i < privdata->half_fifo_size) && (privdata->current_cell < privdata->buf_size/2));

	/* set ADS_EVT_INTERRUPT */
	adv_process_info_set_event_all(&privdata->ptr_process_info, 4, i);

	/* set ADS_EVT_AI_LOBUFREADY */
	if( privdata->current_cell == privdata->buf_size/4 ){
		if (privdata->buf_ready) {
			privdata->over_run = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
		privdata->buf_ready = 1;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}
	/* set ADS_EVT_AI_HIBUFREADY */
	if (privdata->current_cell == privdata->buf_size/2) {
		if (privdata->buf_ready) {
			privdata->over_run = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		}
		privdata->buf_ready = 2;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
	}
	
	/* set ADS_EVT_AI_TERMINATED */
	if( (!privdata->cyclic) && (privdata->current_cell == privdata->buf_size/2)){
		privdata->ai_stop = 1;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
		run_task_queue(&privdata->tq_fai_stop);
#else
		schedule_work(&privdata->fai_stop);
#endif
	}
	privdata->current_cell = privdata->current_cell%(privdata->buf_size/2);
	
	status = advInp(privdata, 25);
	if(status & 0x02){
		goto LOOP;
	}

	return 1;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1718_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1718_interrupt_handler(int irq, void *dev_id)
#endif
{
	private_data *privdata;
	INT32S ret = 0;
	
	privdata = (private_data *)dev_id;

	if (privdata->ai_mode == 0x01) {
		ret = ((advInp(privdata, 8) & 0x10) ? 1: 0);
		if (ret) {
			advOutp(privdata, 8, 0);
		}

	} else if (privdata->ai_mode == 0x02) {
		ret = ((advInp(privdata, 25) & 0x02) ? 1: 0);
		if (ret) {
			advOutp(privdata, 20, 0);	
		}

	} else {
		ret = 0;
	}

	tasklet_schedule(&privdata->tasklet);
	return IRQ_RETVAL(ret);
}

void pci1718_tasklet_handler(PTR_T dev_id)
{
	private_data   *privdata;
	INT32S ret = 0;

	privdata = (private_data *)dev_id;
	spin_lock(&privdata->spinlock);
	
	if(privdata->ai_mode == 0x01){
		ret = pci1718_nofifo_isr(privdata);
	}
	if(privdata->ai_mode == 0x02){
		ret = pci1718_fifo_isr(privdata);
	}

	wake_up_interruptible(&privdata->event_wait);
	spin_unlock(&privdata->spinlock);
	return;	
}
