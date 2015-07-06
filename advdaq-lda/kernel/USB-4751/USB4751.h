/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2007 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech USB-4751 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	USB4751.h
 * Abstract:                                                          
 * 	header file for USB-4751 series driver
 *                                                                         
 * Version history		                                           
 *	01/26/2007			Create by zhenyu.zhang                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_USB4751_H
#define _ADV_USB4751_H
#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include "../../include/usbio.h"
#include <linux/delay.h>
#include <linux/usb.h>
#include <linux/fcntl.h>
#define ADV_NCBULK_CONTROL_TRANSFER          0x842
//#define ADS_EVT_DEVREMOVED 0x400
#define  ADS_EVT_PORT0_Index 0
#define  ADS_EVT_PORT1_Index 1
#define  ADS_EVT_Interrupt_Index	 2
#define  ADS_EVT_DEVREMOVED_Index	 3
#define EVT_PORT0                   ( 0 )
#define EVT_PORT1                   ( 1 )
#define EVT_DI_INTERRUPT_BASE       ( 0 )
#define EVT_DI_INT_COUNT            ( 2 )
#define USB4751               0x4751
#define ADS_EVT_NUM 4 
#define MAX_EVTBUF_LEN        64
typedef struct _EventData
{
	UCHAR EventType;
	UCHAR PortData[6];
}EVENTDATA,*PEVENTDATA;

typedef struct tagTRANSMEM
{
	UCHAR         nEventNum;
	EVENTDATA    EData[9];
}TRANSMEM,*PTRANSMEM;

typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	wait_queue_head_t          event_wait;

	struct usb_device           *udev;	       //the usb device for this device
	struct usb_interface        *interface;     //the interface for this device
	struct semaphore           *usb_urb_sema;
	struct tasklet_struct urb_tasklet;

	INT32U     device_type;
	INT32U     ulAppcount;
	INT16U      usEventEnabled;
	INT32U        hEventCount[ADS_EVT_NUM];
	INT32U        hCurrentCount[ADS_EVT_NUM];
	
	INT16U     board_id;
	INT16U     evt_cnt;
	INT16U	   evt_style;
	struct urb* urb_int;
	INT8U 	   *urb_int_transfer;
	INT8U *urb_int_transfer_copy;
	__u8       int_endpointAddr; /* the address of the interrupt endpoint */
	ULONG      urb_int_in_size;
	atomic_t  tasklet_processed;	/* if int urb buffer processed */
	
} private_data;

INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_usb_ctrl_transfer(adv_usb_device *device, void *arg);
INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);
INT32S adv_get_err_code(adv_device *device, void *arg);
VOID urb_tasklet_fn(PTR_T arg);
void urb_int_callback(struct urb *urb, struct pt_regs *regs);
void adv_device_delete(adv_device *device);
#endif




