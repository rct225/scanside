/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1716 Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	adisr.c                                             
 * Abstract:                                                   
 * 	This file contains routines for interrupt.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	09/26/2006			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "PCM3718.h"
static void pcm3718_dma_isr(private_data *ptr);
void pcm3718_tasklet_handler(PTR_T dev_id)
{
	
	private_data *privdata = (private_data*)dev_id;
		pcm3718_dma_isr(privdata);

}


/**
 * pcm3718_nofifo_isr - interrupt service routine for without fifo
 *                      data acquisition
 *
 * ptr: point to the private data of device object
 *
 * interrupt when an A/D conversion occurs, read the fifo data
 * into user buffer, until user buffer is half full or full,se
 * nd 'buffer change' or 'terminate' event to user.
 */
static void pcm3718_nofifo_isr(private_data *ptr)
{
	private_data *privdata = ptr;
	adv_user_page *page = NULL;
	INT16U tmp;

	page = privdata->user_pages + privdata->page_index;	
	if (privdata->item >= page->length) {
		privdata->page_index++;
		privdata->item = 0;
	}
	
	privdata->page_index %= privdata->page_num;
	privdata->cur_index %= privdata->conv_num;
	
	page = privdata->user_pages + privdata->page_index;
	if(advInp(privdata,8)&0x80) 
		return;
    	tmp = advInp( privdata, 0 ) ;  
    	tmp = tmp | (  advInp( privdata, 1 ) << 8 ) ; 

	memcpy((INT16U *) (page->page_addr + page->offset + privdata->item),
	      &tmp, sizeof(INT16U));
	privdata->cur_index++;
	privdata->item += 2;
		
	/* set event */
	if (privdata->cur_index == privdata->conv_num / 2) { /* buffer change */
		privdata->half_ready = 1;
		privdata->trans += privdata->conv_num/2;
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
		if (privdata->half_ready == 2) {
			privdata->overrun_flag = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       3,
						       1);
		}
	}

	if (privdata->cur_index == privdata->conv_num) {
		privdata->half_ready = 2;
		privdata->trans += privdata->conv_num/2;
	     
		if (!privdata->cyclic) { /* terminate */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
	           	advOutp( privdata, 9,  0 );           // disable interrupt
	           	advOutp( privdata, 6,  0 );           // disable interrupt
			advOutp(privdata,8,0);
		//	privdata->ai_stop = 1;
		} 
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
		     
		//printk("user buf 1024 :%x\n",privdata->user_buf[1024]);
		//printk("user buf 1025 :%x\n",privdata->user_buf[1025]);
		//printk("user buf 2026 :%x\n",privdata->user_buf[1026]);
		
		if (privdata->half_ready == 1) {
			privdata->overrun_flag = 1;
			adv_process_info_set_event_all(&privdata->ptr_process_info,
						       3,
						       1);
		}
	}

	wake_up_interruptible(&privdata->event_wait);
 }


/**
 * pcm3718_fifo_isr - interrupt service routine for without fifo
 *                    data acquisition
 *
 * ptr: point to the private data of device object
 *
 * interrupt when fifo is half full, read the fifo data into
 * user buffer, until user buffer is half full or full, send
 * 'buffer change' or 'terminate' event to user.
 */
static void pcm3718_fifo_isr(private_data *ptr)
{
	private_data *privdata = ptr;
	INT16U tmp;
	INT16U i;
	adv_user_page *page = NULL;
	/* recieve data */
	i = 0;
	do {
		page = privdata->user_pages + privdata->page_index;
		if (privdata->item >= page->length) {
			privdata->page_index++;
			privdata->item = 0;
		}
	
		privdata->page_index %= privdata->page_num;
		privdata->cur_index %= privdata->conv_num;
	
		page = privdata->user_pages + privdata->page_index;

		
        	// read data
        	tmp = advInp( privdata, 23 ) & 0x00ff;               
        	tmp = tmp | ( ( advInp( privdata, 24 ) << 8 ) & 0xff00 );
		memcpy((INT16U *) (page->page_addr + page->offset + privdata->item),
		       &tmp, sizeof(INT16U));
		
		privdata->item += 2;
		privdata->cur_index++;
		i++;
	} while ((i < privdata->half_fifo_size)
		 && (privdata->cur_index < privdata->conv_num));
	

	/* set event */
	if (privdata->cur_index == privdata->conv_num / 2) { /* buffer change */
		privdata->half_ready = 1;

		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
	}

	if (privdata->cur_index == privdata->conv_num) {
		privdata->half_ready = 2;

	     
		if (!privdata->cyclic) { /* terminate */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
	           	advOutp( privdata, 9,  0 );           // disable interrupt
	           	advOutp( privdata, 6,  0 );           // disable interrupt
		} else {		/* buffer change */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
		}
	}

 		     
	if (privdata->overrun_flag) { /* overrun */
		adv_process_info_set_event_all(&privdata->ptr_process_info,
					       3,
					       1);
		wake_up_interruptible(&privdata->event_wait);
	} else {
		privdata->overrun_flag = 1;
	}


	if (privdata->cur_index == privdata->int_cnt) { /* interrupt count */
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}
	
       advOutp( privdata,  20,  0 );  // clear interrupt request

	wake_up_interruptible(&privdata->event_wait);
}


/**
 * pcm3718_dma_isr - interrupt service routine for DMA
 *                   data acquisition
 *
 * ptr: point to the private data of device object
 */
static void pcm3718_dma_isr(private_data *ptr)
{
	unsigned long ret,flags,i;
	private_data *privdata = ptr;
	adv_user_page *page = NULL;
	INT16U tmp;
	/* recieve data */
//while(advInp(privdata,8)&0x80) ;
//while(!advInp(privdata,8)&0x10) ;
	i = 0;
			privdata->item = 0;
			privdata->page_index=0;
//memset(privdata->user_buf,0,privdata->hwdmasize[0]);	

	//printk("----------%x convert %x\n",advInp(privdata,8)&0x10,privdata->hwdmasize[0]); 
   	advOutp( privdata,  0x08,  0 );   // clear interrupt request
   	advOutp( privdata,  0x19,  0 );   // clear interrupt request

	do {
		page = privdata->user_pages + privdata->page_index;
		if (privdata->item >= page->length) {
			privdata->page_index++;
			privdata->item = 0;
		}
	
		privdata->page_index %= privdata->page_num;
		privdata->cur_index %= privdata->conv_num;
		i++;
	
		page = privdata->user_pages + privdata->page_index;

		
        	// read data
        	tmp = privdata->dmabuf[privdata->cur_index];               
       
		memcpy((INT16U *) (page->page_addr + page->offset + privdata->item),
		       &tmp, sizeof(INT16U));
		
		//if(tmp&0x01!=1 ) {
		if(i<10 ) {
	//		printk("i :%x advin %x\n",i,tmp);
	//		printk("i :%x advin %x\n",i,advInp(privdata,1));
	//		printk("userbuf : %x\n",privdata->user_buf[i]);
	//		printk("hwuserbuf :%x\n",privdata->hwdmaptr[i]);

		}
		privdata->item += 2;
		privdata->cur_index++;
	} while (privdata->cur_index < privdata->conv_num);

//	memcpy(privdata->user_buf,privdata->dmabuf,privdata->hwdmasize[0]);
//memset(privdata->dmabuf,0,privdata->hwdmasize[0]);	
	///	printk("user buf 0 :%x\n",privdata->user_buf[0]);
	///	printk("user-1 buf %d :%x\n",i-1,privdata->user_buf[i-1]);
	///	printk("user buf %d :%x\n",i,privdata->user_buf[i]);
	//	printk("user buf 20 :%x\n",privdata->user_buf[20]);
	///	printk("cur_index:%d conv_num/2:%d tmp:%x\n",privdata->cur_index,privdata->conv_num/2,tmp);
		//printk("page index:%x page num:%x\n",privdata->page_index,privdata->page_num);

	if (!privdata->buf_stat) {
		privdata->cur_index = privdata->conv_num / 2;
		privdata->half_ready = 1;

		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	} else {
		privdata->cur_index = privdata->conv_num;
		privdata->half_ready = 2;
		privdata->trans += privdata->conv_num;
			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);

		if (!privdata->cyclic) { /* terminate */
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
	           	advOutp( privdata, 9,  0 );           // disable interrupt
	           	advOutp( privdata, 6,  0 );           // disable interrupt
			advOutp(privdata,8,0);
		//	privdata->ai_stop = 1;
		} else {	/* buffer change */

			if (privdata->overrun_flag==1) { /* overrun */
				adv_process_info_set_event_all(&privdata->ptr_process_info,
							       3,
							       1);
			} 
			privdata->overrun_flag = 1;
			
		}
	}

	if (privdata->cur_index == privdata->int_cnt) { /* interrupt count */
		//adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}
//	for(i=0;i<privdata->conv_num*sizeof(INT16U);i++){
//		privdata->user_buf[i] = privdata->dmabuf[i];
//	}

	if(privdata->cyclic){

        flags=claim_dma_lock();
        disable_dma(privdata->ioDMAbase);
	clear_dma_ff(privdata->ioDMAbase);
        set_dma_mode(privdata->ioDMAbase, DMA_MODE_READ);
        //set_dma_mode(privdata->ioDMAbase, DMA_MODE_READ|DMA_AUTOINIT);
        set_dma_addr(privdata->ioDMAbase, privdata->hwdmaptr);
        set_dma_count(privdata->ioDMAbase, privdata->hwdmasize[0]);
	ret = get_dma_residue(privdata->ioDMAbase);
        release_dma_lock(flags);
	}

        enable_dma(privdata->ioDMAbase);
	wake_up_interruptible(&privdata->event_wait);
	
	//privdata->buf_stat = !privdata->buf_stat;
}


/**
 * pcm3718_interrupt_handler - PCI1716 interrupt handler                    
 *
 *                                                            
 * @irq: interrupt request number            
 * @dev_id: privdata data (device object)                      
 * @regs: holds a snapshot of the processor's context before the processor
 *        entered interrupt code (NULL)        
 */
irqreturn_t pcm3718_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	private_data *privdata = (private_data *) dev_id;
	INT16U tmp;
	tmp = advInp(privdata, 0x08)& 0x10;
	if (tmp!=0x10) 
		return IRQ_RETVAL(0);
	spin_lock(&privdata->spinlock);
   	if(privdata->ai_stop){
		advOutp( privdata,  0x09,  0 );   // clear interrupt request
		return IRQ_RETVAL(1);
	}
	if (privdata->fifo_enable) {
		pcm3718_fifo_isr(privdata);
	} else {
		pcm3718_nofifo_isr(privdata);
	}
	if(privdata->dmastart_flag){
		tasklet_schedule(&privdata->tasklet);	
	}
	spin_unlock(&privdata->spinlock);
	advOutp(privdata, 0x08, 0x00); /* clear interrupt */
     
	return IRQ_RETVAL(1);
}	     
