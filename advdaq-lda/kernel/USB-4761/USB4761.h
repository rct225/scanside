/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech USB-4761 Device driver for Linux

File Name:
	USB4761.h
Abstract:
	Header file of USB-4761 device driver
Version history
	03/20/2006		Create by zhiyong.xie

************************************************************************/	

#ifndef _ADV_USB4761_H
#define _ADV_USB4761_H

//#define USB4761_DEBUG

#include "../include/advdrv.h"


#include "../../include/advdevice.h"

#define	MAX_AO_RANGE	32


#define ADVTECH_USB_PRODUCT_ID 0x4761

#ifdef USB4761_DEBUG
#define PDEBUG(fmt,args...) printk(fmt, ##args)
#else
#define PDEBUG(fmt,arg...)
#endif
     
#define ADS_EVT_DI_INTERRUPT0_Index 0
#define ADS_EVT_DI_INTERRUPT1_Index 1
#define ADS_EVT_DI_INTERRUPT2_Index 2
#define ADS_EVT_DI_INTERRUPT3_Index 3
#define ADS_EVT_DI_INTERRUPT4_Index 4
#define ADS_EVT_DI_INTERRUPT5_Index 5
#define ADS_EVT_DI_INTERRUPT6_Index 6
#define ADS_EVT_DI_INTERRUPT7_Index 7     

#define MAX_DO_PORTS_NUM 1
#define MAX_DI_PORTS_NUM 1
#define MAX_EVENT_NUM    8

struct ports_array
{
     INT32U start;
     INT32U num;
};

struct ports_status
{
     INT8U status[MAX_DI_PORTS_NUM];
};


     
     

typedef struct _private_data
{
	INT32U boardID;

	/* the address of the control endpoint */
	INT8U       control_endpointAddr;

	/* the address of the interrupt endpoint */
	INT8U       int_endpointAddr;

	INT32U      urb_int_in_size;

	/* the usb device for this device */
	struct usb_device  *udev;	 
	/* the interface for this device */
	struct usb_interface       *interface;   

     
	/* which port status need to read back
	 * when event happened
	 * event_scan_port[0] -----    DI 0
	 * event_scan_port[1] -----    DI 1
	 * ...............................
	 * event_scan_port[7] -----    DI 7
	 * */
    	struct ports_array       ports_event_scan[MAX_EVENT_NUM]; 

	/* save the last di status when event occur
	 * event_port_status[0] ------ DI0
	 * ........................................
	 * event_port_status[7] ------ DI7
	 * */
	struct ports_status     ports_status_event[MAX_EVENT_NUM];
      
     	INT16U     us_int_trigger_mode_cfg; //save trigger source edge
	INT16U     us_int_src_cfg; //save the config source of trigger     
	spinlock_t 	spinlock;
	struct semaphore *usb_urb_sema;
	struct semaphore *disconnect_sema;
     
	INT8U             little_endian;

	/* compatible with windows driver */
	INT8U             general_DL;
        
        /* the do status when device reset */
	INT16U            us_do_default_value;

	/* how many application use this device */
	INT32U             ulAppCount;
	
	/* the interrupt endpoint use */
	struct urb        *urb_int; 
	INT8U             *urb_int_transfer;
	INT8U             *urb_int_transfer_copy; /* copy of int urb buffer */
	
	wait_queue_head_t event_wait;
	wait_queue_head_t int_urb_queue;
     
     	struct tasklet_struct check_event_tasklet;
	atomic_t  tasklet_processed;	/* if int urb buffer processed */
	
	adv_process_info_header ptr_process_info;
	
	int		config;
	unsigned short	sigpid;
}private_data;


typedef struct __EE_READ_STRUCT
{
     ULONG ulEEData;
     ULONG ulLastError;
     
}EE_READ_STRUCT, *PEE_READ_STRUCT;

typedef struct __EE_WRITE_STRUCT
{
     ULONG ulEEData;
     USHORT usAddr;
}EE_WRITE_STRUCT, *PEE_WRITE_STRUCT;

typedef struct __EVENT_DATA
{
     UCHAR DIdata;
     UCHAR DIEvent;
}EVENT_DATA, *PEVENT_DATA;

INT32S advdrv_register_driver(adv_driver *driver);
INT32S advdrv_unregister_driver(adv_driver *driver);
INT32S advdrv_add_device( adv_driver *driver,  adv_device *device);
INT32S advdrv_remove_device( adv_driver *driver,  adv_device *device);
     
INT32S adv_resetdevice( adv_device *device);
INT32S adv_opendevice( adv_device *device);
INT32S adv_closedevice( adv_device *device);
INT32S adv_ioctrl( adv_device *device, unsigned int cmd, unsigned long arg);
INT32S adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  );

INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_usb_ctrl_transfer(adv_device *device, USB_TRANSFER_INFO *info);

INT32S adv_set_di_event_mode(adv_device *device, INT32U index);
INT32S adv_set_di_event_trigger_mode(adv_device *device, INT32U port);

INT32S adv_enable_event(adv_device *device, INT16U event_type, INT16U enabled, INT32U count);
INT32S adv_check_event(adv_device *device, INT32U *event_type, INT32U milli_seconds);
VOID check_urb_event(PTR_T data);

INT32S adv_device_set_property(adv_device *device, INT16U nID, void*  pData, INT32U  lDataLength);
INT32S adv_device_get_property(adv_device *device, INT16U nID, void*  pData, INT32U* pDataLength);

#endif
