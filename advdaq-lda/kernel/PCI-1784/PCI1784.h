/**************************************************************************
 *                                                                        
 *                                                                         
 *		Copyright 2006 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1784 Device driver for Linux         
 *                                                                         
 *                                                                         
 * File Name:                                                         
 * 	pci1784.h
 * Abstract:                                                          
 * 	header file for PCI-1784 series driver
 *                                                                        
 * Version history		                                           
 *	11/22/2006			Create by zhenyu.zhang                   
 *                                                                         
 *                                                                        
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1784_H
#define _ADV_PCI1784_H
#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include <linux/delay.h>

#define PCI1784               0x1784

#define MAX_DI_PORT           1
#define MAX_DO_PORT           1
#define MAX_CNT_NUM 	    5 

#define CNTR_API_CounterConfig     0x1
#define CNTR_API_QCounterAPI 	  0x4

#define ADV_SETCOUNTERPARAM            _IO(ADV_IOC_MAGIC_CNT, 25)
#define ADV_GETCOUNTERPARAM            _IO(ADV_IOC_MAGIC_CNT, 29)
#define ADV_DIOSETEVENTCFG            _IO(ADV_IOC_MAGIC_CNT, 26)
#define ADV_DIOGETEVENTDATA 		_IO(ADV_IOC_MAGIC_CNT, 28)
#define ADV_CNTRSETEVENTCFG            _IO(ADV_IOC_MAGIC_CNT, 27)
#define ADV_CNTRGETEVENTDATA            _IO(ADV_IOC_MAGIC_CNT, 30)
#define MAX_EVENT_NUMBER 25

typedef struct _COUNTER_PARAMS
{
	INT32U ulCntrMode[4];
	INT32U ulIntSrc;
	INT32U ulClkCtl;
	USHORT usDoMode;
	USHORT usIndexReset;
	USHORT usDirtyFlag[8];
	USHORT usClsInt;              //Clear all interrupt
} CTL_CNTR_PARAMS, *PCTL_CNTR_PARAMS;
typedef struct CompareTable
{
	USHORT			bEnabled;
	ULONG			ulAllNum;
	ULONG*			pCompBuffer;
	ULONG			ulCurNum;		// Current Point-Number of Comparator Table
}	CompTable;

typedef struct _CNTR_EVT_CFG{
	USHORT portStart;      // start port to scan.
	USHORT portCount;      // port count to scan.
	ULONG  evtData[4];     // PCI-1784 4 Counter
} CNTR_EVT_CFG, * PCNTR_EVT_CFG;

typedef struct _DI_EVT_CFG{
	USHORT portStart;      // start port to scan.
	USHORT portCount;      // port count to scan.
	UCHAR  evtData;     // PCI-1784 has 1 DI port, 5 Counter
} DI_EVT_CFG, * PDI_EVT_CFG;

typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	adv_user_page   *user_pages; /* store user buffer pages */
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;


	INT32U     iobase;
	INT32U     iolength;
	INT32U     device_type;
	INT32U     irq;	
	INT32U     board_id;
	INT16U     timer_clock;
	INT32U	   pci_slot;
	INT32U	   pci_bus;


	INT16U     start_chan;
	INT16U     stop_chan;
	INT16U     *user_buf;
	INT16U     trig_mode;
	INT16U     clk_src;
	INT16U	   cyclic;	/* 0 none cyclic, 1 cyclic */	

	INT32U     page_num;
	INT32U     *sgl_addr;
	INT32U     sgl_cnt;

	INT16U     evt_cnt;
	INT16U     overrun_flag;
	
	INT16U     item;	/* date item index in one page location */
	INT32U     page_index;	
	INT32U     cur_index;	/* current data item will be transfer */
	INT16U     do_prestate[1];

       USHORT     num_chan;
	INT16U     ulSW; 
	INT32U     usInterruptSrc;
  	ULONG             ulCntrStatus[4];
	INT32U             ulIntSourceData;  // Interrupt source
	INT32U             ulDevPara[4];     // counter mode
	INT32U            ulClkCtl;         // counter clock control
	USHORT            usDoMode;         // do mode
	USHORT            usIndexReset;     // index reset for PCI-1784U
	DI_EVT_CFG		DIEvtCfg[4];
	CNTR_EVT_CFG      CntrEvtCfg[25]; // overflow, underflow, overcomp, undercomp, index , DI and  1 timer
	
	CompTable			sCompTable[4];			// for 4 axis
	INT32U SubsystemID;
	INT32U dioIndex;
	INT32U cntrIndex;
} private_data;
typedef struct _AD_DIO_EVENT_CFG{
   INT32U eventID;
   INT32U scanStart;
   INT32U scanCount;
}AD_DIO_EVENT_CFG, *PAD_DIO_EVENT_CFG,  * LPAD_DIO_EVENT_CFG;

typedef struct _AD_COUNTER_EVENT_DATA{
	ULONG  eventID;
	ULONG* dataBuf;
	ULONG  bufLen;
} AD_COUNTER_EVENT_DATA, *PAD_COUNTER_EVENT_DATA, * LPAD_COUNTER_EVENT_DATA;

typedef struct _AD_DIO_EVENT_DATA{
   INT32U  eventID;
   BYTE *dataBuf;
   INT32U  bufLen;
} AD_DIO_EVENT_DATA, *PAD_DIO_EVENT_DATA,  * LPAD_DIO_EVENT_DATA;

typedef struct _AD_CNTR_EVENT_DATA{
	ULONG  eventID;
	ULONG* dataBuf;
	ULONG  bufLen;
} AD_CNTR_EVENT_DATA, *PAD_CNTR_EVENT_DATA, * LPAD_CNTR_EVENT_DATA;


INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_cnt_ioctl(adv_device *device, void *arg);
INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);
INT32S adv_set_counter_status(adv_device *device, cnt_struct *structs, INT32U *data);
INT32S adv_get_counter_status(adv_device *device, cnt_struct *structs, INT32U *data);

INT32S adv_set_counter_param(adv_device *device, CTL_CNTR_PARAMS *pCntrParam);


INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_clear_overrun(adv_device *device);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_get_err_code(adv_device *device, void *arg);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1784_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pci1784_interrupt_handler(int irq, void *dev_id);
#endif

INT32S adv_CntrSetParam(adv_device *device, void *arg);
INT32S adv_CntrGetParam(adv_device *device, void *arg);
INT32S adv_DioSetEventConfig( adv_device *device, void *arg);
INT32S  adv_DioGetEventData( adv_device *device, void *arg);
INT32S adv_CntrGetEventData(adv_device *device, void *arg);
INT32S adv_CntrSetEventConfig(adv_device *device, void *arg);
#endif




