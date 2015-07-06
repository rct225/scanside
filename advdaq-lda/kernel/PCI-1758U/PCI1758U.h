/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1758U Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pci1758U.h
 * Abstract:                                                          
 * 	header file for PCI-1758U series driver
 *                                                                         
 * Version history		                                           
 *	09/12/2006			Create by LI.ANG                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
  
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1758U_H
#define _ADV_PCI1758U_H
#include "../include/advdrv.h"
#include "../../include/advdevice.h"


#include <linux/list.h>
#include <linux/delay.h>
#include <asm/timex.h>


#define PCI1758UDI           0xa100
#define PCI1758UDO           0xa101
#define PCI1758UDIO          0xa102
#define PCI1758UDIOOV        0x0000 /* old-version */

 
#define MAX_CNTS              8
#define INTERNAL_CLOCK_RATE   10000000.0 /* 10MHz */
#define PACER_RATE            1000000.0 /* 1MHz */
#define BOARD_NOT_SUPPORT     1

#define MAX_DI_CHANNEL        128
#define MAX_DO_CHANNEL        128
#define MAX_DIO_CHANNEL       64
#define MAX_DI_PORT           16
#define MAX_DO_PORT           16
#define MAX_DIO_PORT          8



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


	INT16U	   evt_cnt;
	INT8U      int_rising_trig[MAX_DI_PORT]; /* 1: enable
						  * 0: disable */
	INT8U      int_falling_trig[MAX_DI_PORT]; /* 1: enable
				           	   * 0: disable */
	INT8U      int_stat[MAX_DI_PORT];

	INT8U      di_filter_enable[MAX_DI_PORT];
	INT32U     di_filter_interval;
	
	INT32U     overflow[MAX_CNTS];

	INT32U     watchdog_counter;
	INT8U      do_preset_value[MAX_DO_PORT];
	INT8U      do_watchdog_value[MAX_DO_PORT];
} private_data;


INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_cnt_ioctl(adv_device *device, void *arg);

INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_clear_flag(adv_device *device, void *arg);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);
INT32S adv_get_err_code(adv_device *device, void * arg);

void adv_eeprom_read(private_data *ptr, INT8U addr, INT16U *data);
void adv_eeprom_write(private_data *ptr, INT8U addr, INT16U data);
void adv_eeprom_enable(private_data *ptr);

irqreturn_t pci1758U_interrupt_handler(INT32S irq, void *dev_id, struct pt_regs *regs);

void adv_tasklet_action(INT32U data);

#endif




