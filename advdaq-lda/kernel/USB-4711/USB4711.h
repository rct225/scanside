/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2007 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech USB-4711 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	USB4711.h
 * Abstract:                                                          
 * 	header file for USB-4711 series driver
 *                                                                         
 * Version history		                                           
 *	03/15/2007			Create by li.ang
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_USB4711_H
#define _ADV_USB4711_H


#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include "../../include/usbinfo.h"
#include "../../include/usbio.h"

#include <linux/delay.h>
#include <linux/usb.h>
/* #include <linux/scatterlist.h> */

#define USB4711A               0x47110100
#define USB4711B               0x47110110

#define ADV_NCBULK_CONTROL_TRANSFER 0x842
/* #define ADS_EVT_DEVREMOVED    0x400 */
#define MAX_URB_NUM            4
#define FAI_BUSY               0x01

#define USB_SPEED_LOW          0x00
#define USB_SPEED_FULL         0x01
#define USB_SPEED_HIGH         0x02

typedef struct _URB_QUEUE {
	struct urb     *urb;
	
	INT32U         idx;
	adv_user_page  *page;
	int            offset;	
	VOID           *privdata;
	VOID           *kern_cache;
	INT32U         urb_buf_len;
} URB_QUEUE, *PURB_QUEUE;
	
typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	adv_user_page           *user_pages; /* store user buffer pages */
	adv_user_page           *hungry_page; /* unfull user page */
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;
	struct tasklet_struct   urb_tasklet;
	struct usb_device 	*udev; /* the usb device for this device */
	struct usb_interface    *interface; /* the interface for this device */
	struct semaphore        *usb_urb_sema;
	struct work_struct      fai_stop_work;

	
	INT32U      max_urbbuf_len;
	INT8U       control_endpointAddr; /* the address of the control endpoint */
	INT8U       bulk_endpointAddr; /* the address of the interrupt endpoint */
	INT8U       b_interval;
	INT32U      device_type;
	INT32U      board_id;
	INT16U      user_cnt;
	

	URB_QUEUE   *urb_queue;
	INT16U     urb_num;
	INT16U     usb_speed;
	
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
	INT32U     half_buf_len;
	INT32U     buf_len;	

	INT32U     page_num;
	INT16U     evt_cnt;
	INT16U     half_ready;	/* 0 -buffer not ready
				 * 1 -low buffer ready to recieve data
				 * 2 -high buffer ready to recieve data */

	INT16U     overrun_flag; /* overrun flag for user buffer */
	INT16U     halfready_flag;
	INT16U     fifo_overrun; /* overrun flag for HW fifo */
	INT16U     terminated;
	INT32U     page_idx;
	INT32U     recieve_cnt; /* how many bytes has been recieved */
	INT32U     recieved; /* how many data item has been recieved */
	INT16U     load_size;	/* size of data has been load into hungry page */

	INT16U     fai_running;
	
} private_data;

INT32S adv_fai_int_start(adv_device *device, void *arg);
INT32S adv_fai_int_scan_start(adv_device *device, void *arg);
INT32S adv_fai_check(adv_device *device, void *arg);
INT32S adv_fai_stop(adv_device *device);
VOID adv_fai_stop_work(VOID *data);

INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_clear_flag(adv_device *device, void *arg);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);

INT32S adv_locate_usb_device(adv_usb_device *device);
INT32S adv_stop_locate_usb_device(adv_usb_device *device);
INT32S adv_usb_ctrl_transfer(adv_usb_device *device, void *arg);

VOID urb_callback_fn(struct urb *urb, struct pt_regs *regs);
VOID urb_tasklet_fn(PTR_T arg);

#endif
