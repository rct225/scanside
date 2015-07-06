/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 ABJ Linux Driver Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1710 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	PCI1710.h
 * Abstract:                                                          
 * 	header file for PCI-1710 series driver
 *                                                                         
 * Version history		                                           
 *	3/15/2006			Create by LI.ANG                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1710_H
#define _ADV_PCI1710_H
#include "../include/advdrv.h"

#include "../../include/advdevice.h"
#include <asm/timex.h>

#define PCI1710S        0x90501710
#define PCI1710         0x00001710
#define PCI1710B        0xb1001710
#define PCI1710B2       0xb2001710
#define PCI1710C        0xc1001710
#define PCI1710C2       0xc2001710
#define PCI1710L        0x00011710
#define PCI1710LB       0xb1011710
#define PCI1710LB2      0xb2011710
#define PCI1710LC       0xc1011710
#define PCI1710LC2      0xc2011710
#define PCI1710HG       0x00021710
#define PCI1710HGB      0xb1021710
#define PCI1710HGB2     0xb2021710
#define PCI1710HGC      0xc1021710
#define PCI1710HGC2     0xc2021710
#define PCI1710HGL      0x00031710
#define PCI1710HGLB     0xb1031710
#define PCI1710HGLB2    0xb2031710
#define PCI1710HGLC     0xc1031710
#define PCI1710HGLC2    0xc2031710
#define PCI1710U        0xD1001710
#define PCI1710UL       0xD1011710
#define PCI1710HGU      0xD1021710
#define PCI1710HGUL     0xD1031710
#define PCI1711         0x90501711
#define PCI1711B        0xb1001711
#define PCI1711LS       0x90501731
#define PCI1711L        0x00011711
#define PCI1711U        0xC1001711
#define PCI1711UL       0xC1011711

#define PCI1711LB       0xb1011711
#define PCI1713         0x90501713
#define PCI1713U        0xB1001713
#define MIC3713         0x90503713
/* #define PCI1741U        0xa1001741 */

#define MAX_PHYSICAL_CHANNEL  16
#define MAX_CNTS              3


#define	MAX_AO_RANGE	      16

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
	struct semaphore  *fai_sema;
	struct work_struct      fai_stop_work;

	INT32U     iobase;
	INT32U     iolength;
	INT32U     device_type;
	INT16U     irq;	
	INT16U     board_id;
	INT16U     timer_clock;
	INT8U	   pci_slot;
	INT8U	   pci_bus;

	/* INT32U retrieved; */
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

	INT16U     fai_running;

	INT16U     evt_cnt;
	INT16U     buf_stat;	/* 0 -half full,  1 -full */
	INT16U     half_ready;	/* 0 -buffer not ready
				 * 1 -low buffer ready to receive data
				 * 2 -high buffer ready to receive data */
	INT16U     overrun_flag; /* overrun flag for user buffer */
	INT16U     halfready_flag;
	INT16U     terminated;
	
	INT16U     item;	/* date item index in one page location */
	INT32U     page_index;	
	INT32U     cur_index;	/* current data item will be transfer */
	INT32U     transfered;
	
	INT8U      do_prestate[MAX_DO_PORT]; /* for DO previous status */

} private_data;


INT32S adv_dio_ioctl(adv_device *device, VOID *arg);
INT32S adv_read_port(adv_device *device, VOID *arg);
INT32S adv_write_port(adv_device *device, VOID *arg);

INT32S adv_clear_flag(adv_device *device, VOID *arg);
INT32S adv_enable_event(adv_device *device, VOID *arg);
INT32S adv_check_event(adv_device *device, VOID *arg);

INT32S adv_fai_int_start(adv_device *device, VOID *arg);
INT32S adv_fai_int_scan_start(adv_device *device, VOID *arg);
INT32S adv_fai_stop(adv_device *device);
INT32S adv_fai_check(adv_device *device, VOID *arg);
VOID adv_fai_stop_work(VOID *data);

INT32S adv_get_fai_status(adv_device *device, void *arg);


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1710_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pci1710_interrupt_handler(int irq, void *dev_id);
#endif

#endif
