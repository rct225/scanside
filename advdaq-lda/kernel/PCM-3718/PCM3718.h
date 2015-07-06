/**************************************************************************
 *                                                                        
 *                                                                       
 *		Copyright 2007 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCM-3718 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pcM3718.h
 * Abstract:                                                          
 * 	header file for PCM-3718 series driver
 *                                                                         
 * Version history		                                           
 *	3/6/2007			Create by zhenyu.zhang                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	

/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI3718_H
#define _ADV_PCI3718_H
#include "../include/advdrv.h"
//#include "../include/advmem.h"

#include "../../include/advdevice.h"
#include <linux/delay.h>
#include <linux/eisa.h>
#include <linux/module.h>
#include <linux/device.h>
#include <asm-i386/dma.h>

#define PCM3718               0x03718

#define MAX_DI_PORT           2
#define MAX_DO_PORT           2
#define PCM3718_TOTAL_SIZE    0x19

#define MAX_PHYSICAL_CHANNEL  16	

typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	adv_user_page   *user_pages; /* store user buffer pages */
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;


	INT32U     iobase;
	INT32U     irq;
	INT32U     ioDMAbase;
	INT32U     iolength;
	INT32U     device_type;
	INT32U     board_id;
	INT16U     timer_clock;
	INT32U	  pci_slot;
	INT32U	  pci_bus;
	INT16U     fifo_size; /* size of hw fifo on board */
	INT16U     half_fifo_size;
	INT16U     fifo_enable;	/* size of hw fifo on board */
	INT16U     int_cnt;	/* how many interrupts trig an event */
	INT16U	usAITerminateFlag;

	INT16U usFAIRunning;
	INT32U trans;
	INT16U     start_chan;
	INT16U     stop_chan;
	INT16U     *user_buf;
	INT16U     trig_mode;
	INT16U     clk_src;
	INT16U	  cyclic;	/* 0 none cyclic, 1 cyclic */	
	INT8U irqen;
	INT32U     page_num;
	INT32U     *sgl_addr;
	INT16U     overrun_flag;
	INT16U     dmastart_flag;
	INT32U  sgl_cnt;
	
	INT16U     item;	/* date item index in one page location */
	INT32U     page_index;	

	INT8U     do_prestate[MAX_DO_PORT];
	/* ******************************************* */
	unsigned short* 	dmabuf;	// pointers to begin of DMA buffers
	unsigned int 	dmapages[1];	// len of DMA buffers in PAGE_SIZEs
	unsigned short* hwdmaptr;	// hardware address of DMA buffers
	unsigned int 	hwdmasize[1]; 	// len of DMA buffers in Bytes
       USHORT     num_chan;
	INT16U     ulSW; 
	INT16U     divisor1;
	INT16U     divisor2;
	INT32U   IntChan;
	INT32U     conv_num;	/* convertion number */
	INT32U     sample_rate;
	INT32U     cur_index;	/* current data item will be transfer */
	INT16U     buf_stat;	/* 0 -half full,  1 -full */
	INT16U     sd_mask; 
	INT16U     evt_cnt;
	INT16U     half_ready;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	struct tq_struct fai_stop;
	task_queue tq_fai_stop;
#else
	struct work_struct fai_stop;
#endif
	INT32U     ai_stop;
	struct tasklet_struct tasklet;
	INT16U sigpid;
} private_data;

INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32S adv_cnt_ioctl(adv_device *device, void *arg);

INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_clear_overrun(adv_device *device);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);
INT32S adv_get_err_code(adv_device *device, void *arg);



INT32S adv_fai_dma_start(adv_device *device, void* arg);
INT32S adv_fai_int_start(adv_device *device, void* arg);
INT32S adv_fai_int_scan_start(adv_device *device, void* arg);
INT32S adv_fai_stop(adv_device *device);
INT32S adv_fai_transfer(adv_device *device, PT_FAITransfer *lpFAITransfer);
INT32S adv_ai_check(adv_device *device, void *arg);
irqreturn_t pcm3718_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
void pcm3718_tasklet_handler(PTR_T dev_id);
#endif





