/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1716 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pci1716.h
 * Abstract:                                                          
 * 	header file for PCI-1716 series driver
 *                                                                         
 * Version history		                                           
 *	9/20/2006			Create by zhenyu.zhang                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1716_H
#define _ADV_PCI1716_H
#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include <linux/delay.h>

#define PCI1716         0x00001716
#define PCI1716L        0x00011716
#define FIFO_SIZE_PCI1716     1024 /* 1K samples(2K Bytes) on-board fifo */

#define DMA_MAXCOUNT          1048576
#define DMA_SPEED_UPLIMIT_PCI1716     250000 
#define DMA_SPEED_DOWNLIMIT_PCI1716   154 
#define INT_SPEED_UPLIMIT    250000 /* 250KHz */


#define MAX_AI_VALUE          65535
#define MAX_PHYSICAL_CHANNEL  16	
#define MAX_GAIN_RANGE        32
#define MAX_CNTS              3

#define INTERNAL_CLOCK_RATE   1000000.0L /* 1MHz */
#define PACER_RATE            10000000.0L	/* 10MHz, pacer(counter1 & counter2)*/


#define	MAX_AO_RANGE	      16

#define DAC_DATA_NORMAL_MODE  0xc000
#define DAC_DATA_OFFSET_MODE  0x8000
#define DAC_DATA_GAIN_MODE    0x4000


#define OUTPUT_TYPE_VOLTAGE   0x00
#define OUTPUT_TYPE_CURRENT   0x01

#define MAX_DI_CHANNEL        16
#define MAX_DO_CHANNEL        16
#define MAX_DI_PORT           2
#define MAX_DO_PORT           2
#define MAX_DIO_PORT          2


typedef struct _private_data {
	struct tasklet_struct tasklet;
	adv_process_info_header ptr_process_info;
	adv_user_page   *user_pages; /* store user buffer pages */
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;


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
	ULONG      retrieved;

	void *pVaAddrBlock;
	dma_addr_t phyAddrBlock;

	INT8U      output_type[MAX_AO_RANGE];
	AOSET      AOSettings[MAX_AO_RANGE];     
	INT16U     sync_flag;

	INT8U      ai_mode;
	INT16U     gain_array[MAX_PHYSICAL_CHANNEL];
	INT16U     sd_mask; /* 0: single-ended
			     * 1: differential */	
	INT16U     start_chan;
	INT16U     stop_chan;
	INT16U     *user_buf;
	INT16U     trig_mode;
	INT16U   trig_src;
	INT16U   trig_vol;
	INT16U   src_type;
	INT16U     clk_src;
	INT16U	   cyclic;	/* 0 none cyclic, 1 cyclic */	
	INT16U     fifo_size; /* size of hw fifo on board */
	INT16U     half_fifo_size;
	INT16U     fifo_enable;	/* size of hw fifo on board */
	INT16U     int_cnt;	/* how many interrupts trig an event */
	INT32U     conv_num;	/* convertion number */
	INT32U     sample_rate;
	INT16U     divisor1;
	INT16U     divisor2;

	INT32U     page_num;
	INT32U     *sgl_addr;
	INT32U     sgl_cnt;

	INT16U     evt_cnt;
	INT16U     buf_stat;	/* 0 -half full,  1 -full */
	INT16U     half_ready;	/* 0 -buffer not ready
				 * 1 -low buffer ready to receive data
				 * 2 -high buffer ready to receive data */
	INT16U     overrun_flag;
	INT16U     terminated;
	INT16U     halfready_flag;
	
	INT16U     item;	/* date item index in one page location */
	INT32U     page_index;	
	INT32U     cur_index;	/* current data item will be transfer */

	INT8U      do_prestate[MAX_DO_PORT]; /* for DO previous status */

	INT32U     first_count[MAX_CNTS];
	INT32U     pre_count[MAX_CNTS];
	INT32U     err_count[MAX_CNTS];
	INT32U     overflow[MAX_CNTS];

	INT32U     freq_precount[MAX_CNTS];
	INT32U     freq_overflow[MAX_CNTS];
	INT32U     freq_predata[MAX_CNTS];
	struct timeval freq_pretick[MAX_CNTS];
	/* ******************************************* */


     ULONG      term_cnt;	


     ULONG      buf_change_cnt;	




     USHORT     num_chan;
     USHORT     enabled_chan;
     USHORT     enabled_int;
     
     USHORT     terminate_flag;
     USHORT     dct_flag;
     USHORT     almost_full_flag;
     USHORT     ai_running;
     
     USHORT	evt_bufchange;
     USHORT	evt_terminate;
     USHORT	evt_overrun;
     USHORT	evt_status;

     USHORT     timer_clk;


     USHORT	*userptr;
     USHORT	*kptr;
     ULONG	kptr_trans;
     USHORT	kptr_loc; //current addr of kptr
     ULONG	kptrpages;//save ao kernel buffer pages alloced
     int	config;
     ULONG	ulBufIndex;//used to record kernel 128k buffer current address
     adv_memory	phyblock;
     ULONG	ulSGLCount;
     ULONG	ulHalfSGLCount;
     ULONG	ulIntcount;
     ULONG	ulTerminalcnt;
     USHORT	usBufChangeEvent;
     USHORT	usAOBufChangeEvent;
     USHORT	usOverRunEvent;
     USHORT	usTerminateEvent;
     USHORT	usAOTerminateEvent;
     USHORT	sigpid;
     USHORT	tagbuf0;
     USHORT	tagbuf1;
     USHORT	cur_buf;
     ULONG	*pagebuf;
     ULONG	pagebufpages;

} private_data;


INT32S adv_ai_binary_in(adv_device *device, void* arg);
INT32S adv_ai_mbinary_in(adv_device *device, void* arg);
INT32S adv_ai_config(adv_device *device, void* arg);
INT32S adv_mai_config(adv_device *device, void *arg);
INT32S adv_ai_voltage_in(adv_device *device, void *arg);
INT32S adv_ai_mvoltage_in(adv_device *device, void *arg);
INT32S adv_ai_scale(adv_device *device, void *arg);

INT32S adv_fai_dma_start(adv_device *device, void* arg);
INT32S adv_fai_int_start(adv_device *device, void* arg);
INT32S adv_fai_int_scan_start(adv_device *device, void* arg);
INT32S adv_fai_stop(adv_device *device);
INT32S adv_fai_check(adv_device *device, void *arg);

INT32S adv_ai_set_divisor(adv_device *device, void *arg);
INT32S adv_ai_get_timer_clock(adv_device *device, void *arg);


INT32S adv_ao_binary_out(adv_device *device, void *arg);
INT32S adv_ao_voltage_out(adv_device *device, void *arg);
INT32S adv_ao_scale(adv_device *device, void *arg);
INT32S adv_ao_config(adv_device *device, void *arg);
INT32S adv_ao_range_set(adv_device *device,
			USHORT output_type,
			USHORT chan,
			USHORT ref_src,
			float max_value,
			float min_value);
INT32S adv_enable_ao_sync(adv_device *device, INT16U arg);
INT32S adv_write_sync_ao(adv_device *device);

INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32S adv_cnt_ioctl(adv_device *device, void *arg);

INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_clear_flag(adv_device *device, void *arg);
INT32S FAIDmaStart(adv_device *device);


INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);
INT32S adv_get_err_code(adv_device *device, void *arg);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1716_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pci1716_interrupt_handler(int irq, void *dev_id);
#endif


void pci1716_tasklet_handler(PTR_T data);

#endif




