/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1750_H
#define _ADV_PCI1750_H

//#define PCI1750_DEBUG
#include "../include/advdrv.h"
//#include "../include/advmem.h"

#include "../../include/advdevice.h"

#define	MAX_AO_RANGE	32

#define ADS_EVT_PORT0_DI0      TriggerSource_DI0   
#define ADS_EVT_PORT0_DI0_4    TriggerSource_DI0_DI4  
#define ADS_EVT_PORT0_TMR1     TriggerSource_TMR1     

#define ADS_EVT_PORT1_DI8      TriggerSource_DI8      
#define ADS_EVT_PORT1_DI8_12   TriggerSource_DI8_DI12 
#define ADS_EVT_PORT1_CNT2     TriggerSource_CNT2     
#define ADS_EVT_PORT_DISABLE   TriggerSource_Disable  




typedef struct _private_data
{
	ULONG  		iobase;
	ULONG  		iolength;
	ULONG   	ioDMAbase;
	ULONG   	ioDMAlength;
	INT32U          boardID;
  
	INT32U		pci_slot;
	INT32U		pci_bus;
	INT32U   	irq;	
//     ai_info 	ai_info;
//	ao_info		ao_info;
	unsigned char 	dovalue_save[2];//save do value
	unsigned short	cnt_init_data[3];//save counter data
	/* for freq function */
	struct timeval     freq_pre_times[3]; /* for freq function */
	unsigned short     cnt_pre_counts[3]; /* for freq fuction */
     
	/* for event count function */
	unsigned short     evn_pre_counts[3];//save counter count for event
	unsigned short     evn_first_counts[3]; 
	unsigned long      evn_overflow[3]; 
	unsigned long      evn_err_counts[3]; 
     
	/* for event function */
	wait_queue_head_t event_wait;
	adv_process_info_header ptr_process_info;
	
	INT32U      usIntSrcConfig[2]; /* save interrupt source config */
     
	unsigned short     usInterruptSrc;
	INT32U             IsCascadeMode;

	spinlock_t 	spinlock;
	int		config;
	unsigned short	sigpid;
}private_data;

int adv_resetdevice( adv_device *device);
int adv_opendevice( adv_device *device);
int adv_closedevice( adv_device *device);
int adv_ioctrl( adv_device *device, unsigned int cmd, unsigned long arg);
int adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  );
int adv_mmap( adv_device *device, struct file *file, struct vm_area_struct * vma);

int adv_get_config( adv_device *device, void* arg);
int adv_clock_ioctl( adv_device *device, void* arg);

int adv_cnt_ioctl(adv_device *device, void *arg);
int adv_dio_ioctl(adv_device *device, void *arg);
int adv_clock_setup(adv_device *device, adv_clock *clock );


INT32S adv_port_read(adv_device *device, INT32U port, void *databuf, INT32U datalen);
INT32S adv_port_write(adv_device *device, INT32U port, void* databuf, INT32U datalen);

#endif
