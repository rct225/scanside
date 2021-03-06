/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 ABJ Linux Driver Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1747 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	PCI1747.h    *
 * Abstract:                                                          
 * 	header file for PCI-1747 series driver
 *                                                                         
 * Version history		                                           
 *	12/25/2006			Create by LI.ANG                   
 *                                                                         
 ***************************************************************************/	
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1747_H
#define _ADV_PCI1747_H
#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include <asm/timex.h>

#define PCI1747         0x1747
#define MIC3747         0x3747
#define PCI1716         0x1716
#define MAX_PHYSICAL_CHANNEL  16
#define MAX_CNTS              3
#define	 MAX_AO_RANGE	      16
#define MAX_DI_CHANNEL        16
#define MAX_DO_CHANNEL        16
#define MAX_DI_PORT           2
#define MAX_DO_PORT           2
#define MAX_DIO_PORT          2

typedef struct _private_data
{
	adv_process_info_header ptr_process_info;
	adv_user_page   *user_pages; /* store user buffer pages */
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;
	struct semaphore *sem_fai;
	struct tasklet_struct   fai_tasklet;
	struct work_struct fai_stop_work;
	

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

	INT16U     fai_running;
	
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
	INT16U     evt_cnt;
	INT16U     evt_stat[4];	/* event status: 0 - disabled, 1 - enabled */
	INT16U     buf_stat;	/* 0 -half full,  1 -full */
	INT16U     halfready_flag; /* for buffer overrun */
	INT16U     half_ready;	/* 0 -buffer not ready
				 * 1 -low buffer ready to receive data
				 * 2 -high buffer ready to receive data */
	INT16U     overrun_flag;
	INT16U     fifo_af_flag;
	INT16U     dma_tc_flag;
	INT16U     terminal_cnt; /* how many times buffer full */
	INT16U     terminated;
	INT16U     item;	/* date item index in one page location */
	INT32U     page_index;	
	INT32U     cur_index;	/* current data item will be transfer */
	INT32U     transfered;
} private_data;

INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);
INT32S adv_clear_flag(adv_device *device, void *arg);
INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);

INT32S adv_fai_int_start(adv_device *device, void *arg);
INT32S adv_fai_int_scan_start(adv_device *device, void *arg);
INT32S adv_fai_dma_start(adv_device *device, void* arg);
INT32S adv_fai_stop(adv_device *device);
INT32S adv_fai_check(adv_device *device, void *arg);
VOID adv_fai_stop_work(VOID *data);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1747_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pci1747_interrupt_handler(int irq, void *dev_id);
#endif

void fai_tasklet_fn(PTR_T arg);

#endif
