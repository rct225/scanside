/**************************************************************************
 *                                                                         
 *                                                                         
 *		Copyright 2006 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *		Advantech PCI-1712 Device driver for Linux         
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	PCI1712.h
 * Abstract:                                                          
 * 	header file for PCI-1712 series driver
 *                                                                         
 * Version history		                                           
 *	9/20/2006			Create by zhenyu.zhang                   
 *                                                                         
 *                                                                         
 ***************************************************************************/	
 
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1712_H
#define _ADV_PCI1712_H
#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include <linux/delay.h>
#include <linux/syscalls.h>

#define EVENT_NONE                       (0)
#define EVENT_AI_LOBUF_OCCURRED          (1 << 0)
#define EVENT_AI_HIBUF_OCCURRED          (1 << 1)
#define EVENT_AI_OVERRUN_OCCURRED        (1 << 2)
#define EVENT_AI_TERMINATED_OCCURRED     (1 << 3)

#define EVENT_AO_LOBUF_OCCURRED          (1 << 4)
#define EVENT_AO_HIBUF_OCCURRED          (1 << 5)
#define EVENT_AO_UNDERRUN_OCCURRED       (1 << 6)
#define EVENT_AO_TERMINATED_OCCURRED     (1 << 7)

#define PCI1712S               0x905410B5
#define PCI1712L               0x000113FE

#define FIFO_SIZE_PCI1712     1024 /* 1K samples(2K Bytes) on-board fifo */

#define DMA_MAXCOUNT          1048576
#define DMA_SPEED_UPLIMIT_PCI1712     250000 
#define DMA_SPEED_DOWNLIMIT_PCI1712   154 
#define INT_SPEED_UPLIMIT    250000 /* 250KHz */


#define MAX_AI_VALUE          65535
#define MAX_PHYSICAL_CHANNEL  16	
#define MAX_GAIN_RANGE        32
#define MAX_CNTS              3

#define INTERNAL_CLOCK_RATE   1000000.0L /* 1MHz */
#define PACER_RATE            10000000.0L	/* 10MHz, pacer(counter1 & counter2)*/


#define	MAX_AO_RANGE	      16

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

typedef struct _ALLOCATEDMABUFFER
{
	INT16U      usCyclicMode;             // cyclic mode : cycle(1)
	ULONG       ulReqBufSize;             // user request buffer size
	ULONG       ulActBufSize;             // kernel actual allocate size
	VOID       *pBuffer;                  // pointer to buffer
	LONG        Status;                   // last error code
} AllocateDmaBufferInfo, *LPAllocateDmaBufferInfo;

typedef struct _FAODMASTARTINFO
{
	INT16U      usEventEnabled;           // enabled(1)
	INT16U      usEventType;              // type of event
	INT16U      usEventCount;             // number of count send to client
	ULONG       dwConvNum;                // number of conversion
	INT16U      usExtTrig;                // external(1)
	INT16U      usDivisor1;               // pacer rate
	INT16U      usDivisor2;               // pacer rate
	INT16U	  usControl;				// Control Word;
	LONG        Status;                   // last error code
} FAODmaStartInfo, *LPFAODmaStartInfo;
typedef struct _FAICHECKINFO
{
  USHORT      usStopped;                // complete(1), incomplete(0)
  ULONG       ulRetrieved;              // conversion count
  USHORT      usOverrun;                // overrun(1)
  USHORT      usHalfReady;              // not-ready(0), first half(1),
                                        // second half(2)
  LONG        Status;                   // last error code
} FAICheckInfo, *LPFAICheckInfo;

typedef struct _FAITRANSFERINFO
{
  USHORT      usOverrun;                // start point
  ULONG       ulStartNum;               // transfer start point
  ULONG       ulStopNum;                // transfer stop point
  LONG        Status;                   // last error code
} FAITransferInfo, *LPFAITransferInfo;

typedef struct _private_data {
	adv_process_info_header ptr_process_info;
	adv_user_page   *user_pages; /* store user buffer pages */
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;


	struct pci_dev *pdev;
	

	INT32U     ioDMAbase;
	INT32U     ioDMAlength;
	ULONG     iobase;
	INT32U     iolength;
	INT32U     device_type;
	INT32U     irq;	
	INT32U     board_id;
	INT32U	   pci_slot;
	INT32U	   pci_bus;

	unsigned long user_buffer_len;
	
	void *pVaAddrBlock;
	dma_addr_t phyAddrBlock;
	
	ULONG   *pVaSGLHeader;
	dma_addr_t phySGLHeader;
	
	void *pAOCommBufVA;
	dma_addr_t phyAOCommBuf;


	
	INT16U     *user_buf;
	INT16U     *UserBuffer;
	ULONG UserBufferCount;
	INT16U     trig_mode;
	INT16U     clk_src;
	INT16U	   cyclic;	/* 0 none cyclic, 1 cyclic */	
	INT32U     conv_num;	/* convertion number */
	INT32U     sample_rate;
	INT16U     divisor1;

	INT32U     AIHalfFullCnt;
	INT16U aiHalfReadyFlag;
	INT32U  sub_event;
	struct tasklet_struct tasklet;	

	INT16U     usOverrun;
	INT16U     *Buffer;
	ULONG	   ulStartpoint;
	ULONG	   ulCount;

	INT32U     page_num;
	ULONG     *sgl_addr;
	INT32U sgl_phyaddr;
	INT32U     sgl_cnt;

	INT16U     overrun_flag;
	INT16U 	usControl ;
	INT16U   trig_src;
	INT16U   trig_vol;

	ULONG	ulBufIndex;//used to record kernel 128k buffer current address
	ULONG	ulSGLCount;
	ULONG	ulHalfSGLCount;
	ULONG ulRetrieved ;
	ULONG	ulIntcount;
	INT16U     usAOCyclicMode;
	INT16U 	usAITerminateFlag;
	INT16U	sigpid;
	INT16U	tagbuf0;
	INT16U	tagbuf1;
	INT16U	cur_buf;
	ULONG	*pagebuf;
	ULONG	pagebufpages;
	ULONG	ulTrasferIndex;
	INT16U delay_cnt;
	INT16U trig_edge;
	INT16U usExtTrig;
	INT16U usAIHalfReady ;	
	INT16U usFAIRunning;
	INT16U usFAORunning;
	INT16U usAODMACount;
	INT16U usAOHalfReady ;	
	INT16U  usFIFOsize ;

	INT16U				usAOTerminateFlag;     // terminate flag for FAO
	INT16U				usAOStopped;           // complete(1), incomplete(0)
	INT16U				usFIFOIndex;
	ULONG             ulConverted;         // conversion count for AO
	ULONG             ulUnderRun;
	INT16U            usAOLowTransferFlag;
	INT16U            usAOHighTransferFlag;
	INT16U            usUnderrunFlag;
	INT16U            src_type;

	USHORT	        usAIStopped;


} private_data;

typedef struct _FAOLOADINFO
{
	INT16U        usFirstDMA;               // If call adFAOLoad in FAODmaStart then, set TRUE.
	INT16U*       Buffer;                  
	INT32U         ulStartNum;               // start point
	INT32U         ulCount;                    // stop point
	INT32S          Status;                   // last error code
} FAOLoadInfo, *LPFAOLoadInfo;

typedef struct _FAOCHECKINFO
{
	INT16U      usStopped;                // complete(1), incomplete(0)
	INT32U       ulConverted;              // conversion count
	INT16U      usUnderrun;                // overrun(1)
	INT16U      usHalfReady;              // not-ready(0), first half(1),
	// second half(2)
	INT32S        Status;                   // last error code
} FAOCheckInfo, *LPFAOCheckInfo;


INT32S adv_fai_dma_start(adv_device *device, void* arg);


INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32U adv_poll(adv_device *device, struct file *file, struct poll_table_struct *wait);

INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_check_event(adv_device *device, void *arg);

INT32S adFAOLoad(adv_device *device, void *arg);

INT32S adAllocateDmaBuffer(adv_device *device, void *arg);
INT32S adFAODmaStart(adv_device *device, void *arg);
INT32S adFAOTerminate(adv_device *device, void *arg);
VOID adStopDADma(private_data *privdata);
INT32S adFAOStop(adv_device *device, void *arg);
INT32S adFreeDmaBuffer(adv_device *device, void *arg);
INT32S adFAOLoadEx(adv_device *device, void *arg);
INT32S adClearUnderrun(adv_device *device, void *arg);
INT32S adFAOCheck(adv_device *device, void *arg);
INT32S adFAITerminate(adv_device *device, void *arg);
VOID adStopADDma(private_data *privdata);
INT32S adFAICheck(adv_device *device, void *arg);
INT32S adClearFlag(adv_device *device, void *arg);
INT32S adFAITransfer(adv_device *device, void *arg);
INT32S adv_get_devinfo(adv_device *device, void *arg);


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1712_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pci1712_interrupt_handler(int irq, void *dev_id);
#endif


#endif



void pci1712_tasklet_handler(PTR_T dev_id);

