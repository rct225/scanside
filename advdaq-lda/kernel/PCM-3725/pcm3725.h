/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2004 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-3725 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	pcm3725.h
 * Abstract:                                                          
 * 	header file for PCI-3725 series driver
 *                                                                         
 * Version history		                                           
 *	3/15/2006			Create by LI.ANG                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCM3725_H
#define _ADV_PCM3725_H
#include "../include/advdrv.h"

#include "../../include/advdevice.h"
#include <linux/delay.h>
/* #include <linux/version.h> */
#define PCM3725               0x3725
#define MAX_CONNECTORS        6
#define PCM3725_TOTAL_SIZE    0x02
#define CFG_REG_OFFSET        3
#define MAX_DIO_DIR           2
#define PORT_PER_CON          3

#define MIC3725               0x00003725

#define MAX_CNTS              8

#define COUNTER_CLOCK_RATE    1000000 /* 1MHz */

#define PA_FOUT_SRC_EXTER_CLK  0x0000
#define PA_FOUT_SRC_CLK_N      0x0100
#define PA_FOUT_SRC_FOUT_N_M1  0x0200
#define PA_FOUT_SRC_CLK_10MHZ  0x0300
#define PA_FOUT_SRC_CLK_1MHZ   0x0400
#define PA_FOUT_SRC_CLK_100KHZ 0x0500
#define PA_FOUT_SRC_CLK_10KHZ  0x0600
#define PA_FOUT_SRC_CLK_1KHZ   0x0700
#define PA_FOUT_SRC_CLK_20MHZ  0x0800
#define PA_FOUT_SRC_CLK_5MHZ   0x0900

#define CNT0_MODE_REG          0x00
#define CNT0_LOAD_REG          0x02
#define CNT0_HOLD_REG          0x04
#define CNT0_CMD_REG           0x06

#define CNT_CMD_DISARM         0x00
#define CNT_CMD_LOAD           0x01
#define CNT_CMD_DISARM_SAVE    0x02
#define CNT_CMD_STEP           0x03
#define CNT_CMD_ARM            0x04
#define CNT_CMD_LOAD_ARM       0x05
#define CNT_CMD_SAVE           0x06 
#define CNT_CMD_RESET          0x07

#define CMD_ENABLE_REG         0x40

#define FOUT_0_REG             0x50

#define READY                  0x00
#define EVENT_COUNT            0x01
#define FREQ_MEASUREMENT       0x02
#define PULSE_OUTPUT           0x04
#define PWM_OUTPUT             0x08
#define FREQ_OUTPUT            0x10
#define PWM_OUTPUTMODIFY       0x20
#define PWM_MEASUREMENT        0x40
#define PWM_GATESRC            0x08

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
	INT16U     board_id;
	INT8U	   pci_slot;
	INT8U	   pci_bus;
	INT16U     int_src;

/* 	INT16U     clk_src; */
/* 	INT16U     cnt_mode; */
/* 	INT16U     load_val; */
/* 	INT16U     hold_val; */
/* 	INT16U     divisor; */


	INT16U     evt_cnt;
	INT16U     int_stat;

	INT16U      user_cnt;	
	INT8U      do_prestate[MAX_DO_PORT]; /* for DO previous status */

/* 	INT32U     first_count[MAX_CNTS]; */
/* 	INT32U     pre_count[MAX_CNTS]; */
/* 	INT32U     err_count[MAX_CNTS]; */
/* 	INT32U     overflow[MAX_CNTS]; */
/* 	INT16U     cnt_src[MAX_CNTS]; */
/* 	INT16U     cnt_cfg_flag[MAX_CNTS]; /\* indicate wether the counter has */
/* 					    * been configed *\/	 */
/* 	INT16U     freq_src[MAX_CNTS]; */
/* 	INT32U     freq_precount[MAX_CNTS]; */
/* 	INT32U     freq_overflow[MAX_CNTS]; */
/* 	INT32U     freq_predata[MAX_CNTS]; */
/* 	struct timeval freq_pretick[MAX_CNTS]; */

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
irqreturn_t pcm3725_interrupt_handler(INT32S irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pcm3725_interrupt_handler(INT32S irq, void *dev_id);
#endif

void evt_tasklet_fn(PTR_T arg);

#endif




