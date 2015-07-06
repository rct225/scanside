/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2007 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech USB-4750 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	USB4750.h
 * Abstract:                                                          
 * 	header file for USB-4750 series driver
 *                                                                         
 * Version history		                                           
 *	03/15/2007			Create by li.ang
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_USB4750_H
#define _ADV_USB4750_H
#include "../include/advdrv.h"

#include "../../include/advdevice.h"
#include "../../include/usbinfo.h"
#include "../../include/usbio.h"

#include <linux/delay.h>
#include <linux/usb.h>
#define USB4750               0x4750

#define ADV_NCBULK_CONTROL_TRANSFER 0x842

#define MAX_EVTBUF_LEN        64
#define MAX_EVTBUF_NUM        20
#define MAX_EVENT_NUM           2

#define EVENT_ID_DI0          0x01 
#define EVENT_ID_DI8          0x02
#define ADS_EVT_INTERRUPT_ID  0x04
#define EVENT_ID_NONE         0xff

#define MAX_DI_CHANNEL        16
#define MAX_DO_CHANNEL        16
#define MAX_DI_PORT           2
#define MAX_DO_PORT           2
#define MAX_DIO_PORT          2


typedef struct _event_port_info {
	INT32U start;
	INT32U count;
} event_port_info;

typedef struct _EVENT_DATA {
	INT8U EventType;
	INT8U PortData[2];
} EVENT_DATA, *PEVENT_DATA;

typedef struct _SW_EVENT_FIFO {
	INT16U EventCount;
	INT8U bEmpty;
	INT8U bOverRun;
	EVENT_DATA EventDataFifo[MAX_EVTBUF_NUM];
} SW_EVENT_FIFO, *PSW_EVENT_FIFO;

typedef struct _DI_EVT_CFG {
	INT16U port_start;
	INT16U port_count;
	INT16U evt_data;
} DI_EVT_CFG, *PDI_EVT_CFG;

typedef struct __EE_WRITE_STRUCT {
        INT16U      usEEData;
	INT16U      usAddr;
} EE_WRITE_STRUCT, *PEE_WRITE_STRUCT;
	
typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	wait_queue_head_t       event_wait;
	event_port_info         event_port[MAX_EVENT_NUM];
	struct tasklet_struct   urb_tasklet;
	struct usb_device 	*udev; /* the usb device for this device */
	struct usb_interface    *interface; /* the interface for this device */
	struct semaphore        *usb_urb_sema;
	struct urb              *urb_int; 
	spinlock_t 	        spinlock;     
	atomic_t    tasklet_processed; /* if int urb buffer processed */

	INT32U      device_type;
	INT32U      board_id;
	INT16U      user_cnt;

	INT8U       *event_buf;
	INT8U       *event_buf_clone;
	INT32U      max_evtbuf_len;
	INT8U       control_endpointAddr; /* the address of the control endpoint */
	INT8U       int_endpointAddr; /* the address of the interrupt endpoint */

	INT16U      evt_data;
	
	DI_EVT_CFG  di_evt_cfg[MAX_EVENT_NUM];
	
	INT16U      trig_edge;
	INT16U      di_int_mode;
	INT16U      do_preset;
} private_data;

INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);

INT32S adv_dio_ioctl(adv_device *device, void *arg);

INT32S adv_locate_usb_device(adv_usb_device *device);
INT32S adv_stop_locate_usb_device(adv_usb_device *device);
INT32S adv_usb_ctrl_transfer(adv_usb_device *device, void *arg);

VOID urb_callback_fn(struct urb *urb, struct pt_regs *regs);
VOID urb_tasklet_fn(PTR_T arg);

#endif
