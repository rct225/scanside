/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1760 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pci1760.h
 * Abstract:                                                          
 * 	header file for PCI-1760 series driver
 *                                                                         
 * Version history		                                           
 *	3/15/2006			Create by LI.ANG                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
  
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1760_H
#define _ADV_PCI1760_H
#include "../include/advdrv.h"
/* #include "../include/advmem.h" */
#include "../../include/advdevice.h"
#include "e8051.h"

/* #include <linux/iobuf.h> */
#include <linux/list.h>
#include <linux/delay.h>
#include <asm/timex.h>


#define PCI1760               0x1760
 
#define MAX_CNTS              8
#define INTERNAL_CLOCK_RATE   10000000.0 /* 10MHz */
#define PACER_RATE            1000000.0 /* 1MHz */
#define BOARD_NOT_SUPPORT     1

#define MAX_DI_CHANNEL        8
#define MAX_DO_CHANNEL        8
#define MAX_DI_PORT           1
#define MAX_DO_PORT           1


#define INFINITE              0xffffffff

#define adv_set_flag(x, y, z)       ((x)->y = z)

typedef struct _private_data
{
	adv_process_info_header ptr_process_info;
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;

     
	INT32U     iobase;
	INT32U     iolength;
	INT32U     irq;
	INT32U     device_type;
	INT16U     board_id;
	INT8U	   pci_slot;
	INT8U	   pci_bus;



	INT16U     read_timeout;
	INT16U     pattern_data;
	INT16U     status_data;
	INT8U      cnt_match_data;
	INT8U      cnt_overflow_data;

	INT8U      di_pm_enabled_channel; /* pattern match enabled channel */
	INT8U      di_pm_value;	/* pattern match value */
	INT8U      di_sc_enabled_channel; /* status change enabled channel */
	
	INT16U     do_prestate; /* for DO previous status */

	INT32U     overflow[MAX_CNTS];

} private_data;


/* INT32S adv_cnt_ioctl(adv_device *device, void *arg); */
INT32S adv_cnt_event_read(adv_device *device, void *arg);
INT32S adv_fdi_transfer(adv_device *device, void *arg);

/* INT32S adv_dio_ioctl(adv_device *device, void *arg); */
void adv_dio_set_mode(adv_device *device, INT16U port, INT16U direction);

INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_clear_flag(adv_device *device, void *arg);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);
INT32S adv_get_err_code(adv_device *device, void *arg);

irqreturn_t pci1760_interrupt_handler(INT32S irq, void *dev_id, struct pt_regs *regs);
void adv_tasklet_action(INT32U data);

INT32S command_out(private_data *privdata, INT32U data, INT16U cmd); /* e8051 function*/
INT32S is_em_op_mode(private_data *privdata); /* e8051 function */

#endif
