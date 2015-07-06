/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1710 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pci1710.h
 * Abstract:                                                          
 * 	header file for PCI-1710 series driver
 *                                                                         
 * Version history		                                           
 *	3/15/2006			Create by LI.ANG                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1720_H
#define _ADV_PCI1720_H
#include "../include/advdrv.h"

#include "../../include/advdevice.h"
#include <linux/delay.h>

#define PCI1720               0x1720
#define PCI1723               0x1723
#define PCI1724               0x1724
#define PCI1727               0x1727


#define BOARD_NOT_SUPPORT     1
#define	MAX_AO_RANGE	      32

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
	adv_process_info_header ptr_process_info;
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;

	INT32U     iobase;
	INT32U     iolength;
	INT32U     device_type;
	INT16U     board_id;
	INT8U	   pci_slot;
	INT8U	   pci_bus;
	INT16U     irq;

	INT8U      do_prestate[MAX_DO_PORT]; /* for DO previous status */
} private_data;



INT32S adv_dio_ioctl(adv_device *device, void *arg);

INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);
#endif




