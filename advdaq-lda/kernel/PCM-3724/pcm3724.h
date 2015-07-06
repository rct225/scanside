/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-3724 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pcm3724.h
 * Abstract:                                                          
 * 	header file for PCI-3724 series driver
 *                                                                         
 * Version history		                                           
 *	3/15/2006			Create by LI.ANG                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCM3724_H
#define _ADV_PCM3724_H
#include "../include/advdrv.h"

#include "../../include/advdevice.h"
#include <linux/delay.h>
/* #include <linux/version.h> */
#define PCM3724               0x3724
#define MAX_CONNECTORS        6
#define PCM3724_TOTAL_SIZE    0x0a
#define CFG_REG_OFFSET        3
#define MAX_DIO_DIR           2
#define PORT_PER_CON          3

#define MIC3724               0x00003724


#define MAX_DI_CHANNEL        24
#define MAX_DO_CHANNEL        24
#define MAX_DI_PORT           3
#define MAX_DO_PORT           3
#define MAX_DIO_PORT          3


typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;
	struct tasklet_struct   evt_tasklet;


	INT32U     iobase;
	INT32U     iolength;
	INT32U     device_type;
	INT16U     irq;	
	INT16U     irq_ext;	
	INT16U     board_id;
	INT8U	   pci_slot;
	INT8U	   pci_bus;
	INT16U     int_src;

	INT16U     evt_cnt;
	INT16U     int_stat;

	INT16U      user_cnt;	
	INT8U      do_prestate[MAX_DO_PORT]; /* for DO previous status */

} private_data;



INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32S adv_cnt_ioctl(adv_device *device, void *arg);

INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);
INT32S adv_get_device_name(adv_device *device, void *arg);
INT32S adv_get_err_code(adv_device *device, void *arg);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pcm3724_interrupt_handler00(INT32S irq, void *dev_id, struct pt_regs *regs);
irqreturn_t pcm3724_interrupt_handler01(INT32S irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pcm3724_interrupt_handler00(INT32S irq, void *dev_id);
irqreturn_t pcm3724_interrupt_handler01(INT32S irq, void *dev_id);
#endif

void evt_tasklet_fn(PTR_T arg);

#endif




