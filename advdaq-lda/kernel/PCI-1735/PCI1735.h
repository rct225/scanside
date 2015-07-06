/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1735 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pci1735.h
 * Abstract:                                                          
 * 	header file for PCI-1735 series driver
 *                                                                         
 * Version history		                                           
 *	10/17/2006			Create by zhenyu.zhang                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1735_H
#define _ADV_PCI1735_H
#include "../include/advdrv.h"

#include "../../include/advdevice.h"
#include <linux/delay.h>

#define PCI1735               0x1735
#define PCI1735RA101          0x1735a101
#define PCI1735RA102          0x1735a102
#define MAX_DI_CHANNEL        32
#define MAX_DO_CHANNEL        32
#define MAX_DI_PORT           4
#define MAX_DO_PORT           4
#define MAX_DIO_PORT          4



#define MAX_AI_VALUE          65535
#define MAX_PHYSICAL_CHANNEL  32
#define MAX_GAIN_RANGE        32
#define MAX_CNTS              3

#define INTERNAL_CLOCK_RATE   1000000.0L /* 1MHz */
#define PACER_RATE            10000000.0L	/* 10MHz, pacer(counter1 & counter2)*/


#define DAC_DATA_NORMAL_MODE  0xc000
#define DAC_DATA_OFFSET_MODE  0x8000
#define DAC_DATA_GAIN_MODE    0x4000


#define OUTPUT_TYPE_VOLTAGE   0x00
#define OUTPUT_TYPE_CURRENT   0x01



typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	adv_user_page   *user_pages; /* store user buffer pages */

	INT32U     iobase;
	INT32U     iolength;
	INT32U     device_type;

	INT16U     board_id;
	INT16U     timer_clock;
	INT8U	   pci_slot;
	INT8U	   pci_bus;
	INT32U     irq;


	INT16U     sync_flag;
	INT16U     start_chan;
	INT16U     stop_chan;
	INT16U     *user_buf;
	INT16U     trig_mode;
	INT16U     clk_src;
	INT16U	   cyclic;	/* 0 none cyclic, 1 cyclic */	
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


}private_data;

INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32S adv_cnt_ioctl(adv_device *device, void *arg);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);
INT32S adv_get_err_code(adv_device *device, void *arg);

irqreturn_t pci1716_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);


#endif




