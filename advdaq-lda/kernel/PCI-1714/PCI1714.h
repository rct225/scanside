/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1714 Device driver for Linux         
 *                                                                         
 *                                                                         
 * File Name:                                                        
 * 	pci1714.h
 * Abstract:                                                         
 * 	header file for PCI-1714 driver
 *                                                                        
 * Version history		                                          
 *	9/26/2006			Create by LI.ANG                  
 *                                                                        
 *                                                                        
 ***************************************************************************/	

/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1714_H
#define _ADV_PCI1714_H

#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include <linux/delay.h>

#define PCI1714                 0x00001714
#define PCI1714U                0xb1001714
#define PCI1714UL_A             0xa1011714
#define PCI1714UL_B             0xb1011714
#define PCI1714UL               0xa1011714


#define INTERNAL_CLK_RATE     60000000 /* 60MHz */
#define INTERNAL_CLK_RATE_UL  20000000 /* 20MHz */
#define DMA_MAX_RATE_UL       10000000 /* 10MHz */
#define DMA_MIN_RATE_UL       78125 /* 78.125KHz */
#define DMA_MAX_RATE          30000000 /* 30MHz */
#define DMA_MIN_RATE          234375 /* 234.375KHz */

#define TRIG_MODE_PACER       0
#define TRIG_MODE_POST        1
#define TRIG_MODE_DELAY       2
#define TRIG_MODE_ABOUT       3

#define TRIG_SRC_DI           0
#define TRIG_SRC_AI0          1
#define TRIG_SRC_AI1          2
#define TRIG_SRC_AI2          3
#define TRIG_SRC_AI3          4

#define CLK_INTERNAL          0
#define CLK_EXTERNAL_0        1
#define CLK_EXTERNAL_1        1

#define TRIG_EDGE_RISE        0
#define TRIG_EDGE_FALL        1

#define CYCLIC_NONE           0
#define CYCLIC_TRUE           1

#define FIFO_SIZE             32768 /* 32K samples(64K Bytes) on-board fifo */
#define FIFO_SIZE_UL          8192 /* 8K samples(16K Bytes) on-board fifo */
#define DMA_MAXCOUNT          31457280 /* we suggest this limitation, if the sample rate
					* is not very fast(>=10M), because in low sample
					* rate, we needn't allocate a very big user buf-
					* fer, it's no significance.
					* 
					* but, it the sample rate is very fast, we shou-
					* ld allocate a big buffer for it.
					* 
					* (ex. sample rate is 10M(or 30M), we should ch-
					* ange the DMA_MAXCOUNT to 32M, for the high sp-
					* eed sample */

#define MAX_AI_VALUE          4095
#define MAX_PHYSICAL_CHANNEL  4	
#define MAX_GAIN_RANGE        32
#define MAX_CNTS              3

typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	adv_user_page   *user_pages; /* for int mode: store total user buffer pages */
	adv_user_page   *user_pages_0; /* for dma sgl: store user top half buffer pages */
	adv_user_page   *user_pages_1; /* for dma sgl: store user bottom half buffer pages */
	wait_queue_head_t       event_wait;
	struct tasklet_struct   fai_tasklet;
	spinlock_t              spinlock;
	struct work_struct      fai_stop_work;
	

	INT32U     ioDMAbase;
	INT32U     ioDMAlength;
	INT32U     iobase;
	INT32U     iolength;
	INT32U     device_type;
	INT16U     irq;	
	INT16U     board_id;
	INT16U     timer_clock;
	INT8U	   pci_slot;
	INT8U	   pci_bus;


	INT32U   read_len1;
	INT32U   read_len2;
	INT32U   read_len3;
	
	INT8U      fai_running;

	INT16U     gain_array[MAX_PHYSICAL_CHANNEL];
	INT16U     sd_mask; /* 0: single-ended
			     * 1: differential */	
	INT16U     start_chan;
	INT16U     stop_chan;
	INT16U     num_chan;
	INT16U     *user_buf;
	INT16U     trig_src;
	float      trig_vol;	/* trig voltage level */
	INT16U     trig_mode;
	INT16U     trig_edge;
	INT16U     trig_src_type;
	INT16U     delay_cnt;
	INT16U     clk_src;
	INT16U	   cyclic;	/* 0 none cyclic, 1 cyclic */	
	INT16U     fifo_size; /* size of hw fifo on board */
	INT16U     half_fifo_size;
	INT16U     fifo_enable;	/* size of hw fifo on board */
	INT16U     int_cnt;	/* how many interrupts trig an event */
	INT32U     conv_num;	/* convertion number */
	INT32U     sample_rate;

	INT32U     max_int_freq;
	INT16U     clk_src_reg;
	INT16U     clk_ctrl_reg;
	INT16U     int_ctrl_reg;
	INT16U     trig_ctrl_reg;
	
	INT16U     divisor1;
	INT16U     divisor2;
	INT16U     int_stat;

	INT32U     page_num;	/* for int mode:  buffer total page numbers */
	INT32U     page_num_0;	/* for DMA sgl: top half buffer page numbers */
	INT32U     page_num_1;	/* for DMA sgl: bottom half buffer page numbers */
	INT32U     *sgl_addr;
	INT32U     sgl_cnt;

	INT16U     evt_cnt;
	INT16U     evt_stat[5];	/* event status: 0 - disabled, 1 - enabled */
	INT16U     buf_stat;	/* 0 -half full,  1 -full */
	INT16U     half_ready;	/* 0 -buffer not ready
				 * 1 -low buffer ready to receive data
				 * 2 -high buffer ready to receive data */
	INT16U     overrun_flag;
/* 	INT16U     low_buf_flag; */
/* 	INT16U     high_buf_flag; */
	INT16U     halfready_flag;
	INT16U     fifo_overrun; /* overrun flag for HW fifo */
	INT16U     fifo_af_flag;
	INT16U     dma_tc_flag;
	INT16U     terminated;
	INT16U     terminal_cnt; /* how many times buffer full */
	
	INT16U     item;	/* date item index in one page location */
	INT32U     page_index;	
	INT32U     transfered;	/* data has been transfered */
	INT32U     cur_index;	/* current data item will be transfer */

} private_data;



INT32S adv_fai_dma_start(adv_device *device, void* arg);
INT32S adv_fai_stop(adv_device *device);
INT32S adv_fai_check(adv_device *device, void *arg);
VOID adv_fai_stop_work(VOID *data);

INT32S adv_ai_set_divisor(adv_device *device, void *arg);
INT32S adv_ai_get_timer_clock(adv_device *device, void *arg);

INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_clear_flag(adv_device *device, void *arg);


INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1714_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pci1714_interrupt_handler(int irq, void *dev_id);
#endif

void fai_tasklet_fn(PTR_T arg);


#endif
