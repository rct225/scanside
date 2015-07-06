/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1742 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pci1742.h
 * Abstract:                                                          
 * 	header file for PCI-1742 series driver
 *                                                                         
 * Version history		                                           
 *	3/15/2006			Create by LI.ANG                   
 *                                                                         
 ***************************************************************************/
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1742_H
#define _ADV_PCI1742_H
#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include <linux/delay.h>

#define PCI1742               0x1742

#define DMA_MAXCOUNT          1048576
#define FIFO_SIZE             1024 /* 1K samples(2K Bytes) on-board fifo */
#define DMA_MAX_RATE          1000000 /* 1MHz */
#define DMA_MIN_RATE          154000 /* 154KHz */
#define INT_MAX_RATE          250000 /* 250KHz */
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

#define FAI_DMA_START         1
#define FAI_INT_START         2
#define FAI_INT_SCAN_START    3

typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	adv_user_page   *user_pages; /* store user buffer pages */
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;
	struct semaphore  *sem_fai;
	struct tasklet_struct   fai_tasklet;
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

	
	INT8U      output_type[MAX_AO_RANGE];
	AOSET      AOSettings[MAX_AO_RANGE];     
	INT16U     sync_flag;
	INT16U     gain_array[MAX_PHYSICAL_CHANNEL];
	INT16U     sd_mask; /* 0: single-ended
			     * 1: differential */
	INT16U     start_chan;
	INT16U     stop_chan;
	INT16U     num_chan;
	INT16U     *user_buf;
	INT16U     trig_mode;
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

	INT16U     fai_running;
	INT16U     evt_cnt;
	INT16U     evt_stat[5];	/* event status: 0 - disabled, 1 - enabled */
	INT16U     buf_stat;	/* 0 -half full,  1 -full */
	INT16U     halfready_flag; /* for buffer overrun */
	INT16U     half_ready;	/* 0 -buffer not ready
				 * 1 -low buffer ready to receive data
				 * 2 -high buffer ready to receive data */
	INT16U     fifo_af_flag;
	INT16U     dma_tc_flag;
/* 	INT16U     high_buf_flag; */
/* 	INT16U     low_buf_flag; */
	INT16U     overrun_flag;
	INT16U     terminated;
	INT16U     terminal_cnt; /* how many times buffer full */


	INT16U     item;	/* date item index in one page location */
	INT32U     page_index;	
	INT32U     transfered;	/* data has been transfered */
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
} private_data;

INT32S adv_fai_dma_start(adv_device *device, void* arg);
INT32S adv_fai_int_start(adv_device *device, void* arg);
INT32S adv_fai_int_scan_start(adv_device *device, void* arg);
INT32S adv_fai_stop(adv_device *device);
INT32S adv_fai_check(adv_device *device, VOID *arg);
VOID adv_fai_stop_work(VOID *data);

INT32S adv_ai_set_divisor(adv_device *device, void *arg);
INT32S adv_ai_get_timer_clock(adv_device *device, void *arg);

INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_clear_flag(adv_device *device, void *arg);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1742_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pci1742_interrupt_handler(int irq, void *dev_id);
#endif

void fai_tasklet_fn(PTR_T arg);

#endif

