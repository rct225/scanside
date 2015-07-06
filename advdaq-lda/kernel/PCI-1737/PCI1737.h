/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1737 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pci1737.h
 * Abstract:                                                          
 * 	header file for PCI-1737 series driver
 *                                                                         
 * Version history		                                           
 *	10/30/2006			Create by zhenyu.zhang                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1737_H
#define _ADV_PCI1737_H
#include "../include/advdrv.h"

#include "../../include/advdevice.h"
#include <linux/delay.h>

#define PCI1737               0x1737
#define PCI1739               0x1739

#define PCI1737RA101           0x1737a101
#define PCI1737RA102           0x1737a102
#define PCI1739RA101           0x1739a101
#define PCI1739RA102           0x1739a102
#define MAX_DI_PORT           6
#define MAX_DO_PORT           6
#define MAX_DIO_PORT          6


typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	adv_user_page   *user_pages; /* store user buffer pages */
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;


	INT32U     iobase;
	INT32U     iolength;
	INT32U     device_type;
	INT32U     irq;	
	INT32U     board_id;
	INT16U     timer_clock;
	INT32U	   pci_slot;
	INT32U	   pci_bus;
	INT32U     port_num;
	INT32U     port_group;

	INT16U     start_chan;
	INT16U     stop_chan;
	INT16U     *user_buf;
	INT16U     trig_mode;
	INT16U     clk_src;
	INT16U	   cyclic;	/* 0 none cyclic, 1 cyclic */	

	INT32U     page_num;
	INT32U     *sgl_addr;
	INT32U     sgl_cnt;
	INT32U     di_port_cnt;
	INT32U     do_port_cnt;
	INT32U     di_channel_cnt;
	INT32U     do_channel_cnt;

	INT16U     evt_cnt;
	INT16U     overrun_flag;
	
	INT16U     item;	/* date item index in one page location */
	INT32U     page_index;	
	INT32U     cur_index;	/* current data item will be transfer */

	INT8U      do_prestate[MAX_DIO_PORT]; /* for DO previous status */

	/* ******************************************* */

       USHORT     num_chan;
 	INT16U    usDioPortDir[MAX_DIO_PORT]; /* port configureation for input or output
					     * 0 -------IN  1------OUT
					     * 2 -------
					     * 3 ------- PCL - IN  PCH - OUT
					     * 4 ------- PCL - OUT PCH - IN
					     * [0] - PA
					     * [1] - PB
					     * [2] - PC
					     * */
	INT16U     ulSW; 
	INT8U     usInterruptSrc;
	INT32U     usIntSrcConfig[2]; //save the config source of trigger
	INT16U     int_trig_mode;	
} private_data;

INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);


INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_clear_overrun(adv_device *device);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);

irqreturn_t pci1737_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);


#endif




