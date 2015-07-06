/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2007 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech USB-4718 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	USB4718.h
 * Abstract:                                                          
 * 	header file for USB-4718 series driver
 *                                                                         
 * Version history		                                           
 *	01/26/2007			Create by zhenyu.zhang                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_USB4718_H
#define _ADV_USB4718_H
#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include <linux/delay.h>
#include <linux/usb.h>
#include <linux/timer.h>

#define USB4718               0x4718
#define MAX_AI_CHANNEL	 8  
#define ADV_NCBULK_CONTROL_TRANSFER          0x842
//#define ADS_EVT_DEVREMOVED 0x400

typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	wait_queue_head_t       event_wait;
	struct usb_device           *udev;	
	struct semaphore *usb_urb_sema;
	INT32U     device_type;
	INT32U ulAppcount;
	INT16U     board_id;
	INT16U     evt_cnt;
	INT16U	   evt_style;
	struct timer_list usb_timer;

	USHORT              usCjcTemperature[7];   //0~4 stores the CJC temperature
                                               //5   stores the CJC error
	                                           //6   stores the number of cjc data
	ULONG               ulCJCIndex;
} private_data;

typedef  struct tagINTERNALGAINLIST
{
   INT16U   usGainCde;
   FP32 fMaxGainVal;
   FP32    fMinGainVal;
   INT8U szGainStr[16];
   LONG     lParam;
   LONG     lCali;
} INTERNALGAINLIST, *PINTERNALGAINLIST;

typedef struct _SET_AICHLSTATUS_STRUCT
{
	INT32U   ulStatus[MAX_AI_CHANNEL];
	INT16U  usChlStatusCode;
	INT16U  usStartChan;
    	INT16U  usChanNum;
} SET_AICHLSTATUS_STRUCT, *PSET_AICHLSTATUS_STRUCT;


typedef struct _CJC_TEMP_READ_STRUCT
{
    ULONG ulCjcTemp;
    ULONG ulStatus;

}CJC_TEMP_READ_STRUCT, *PCJC_TEMP_READ_STRUCT;




extern adv_driver usb4718_driver;
INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_usb_ctrl_transfer(adv_usb_device *device, void *arg);
INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);

static INT32S usb4718_readtemp(private_data* ptr);
int init_usb4718_timer(private_data *privdata);

#endif




