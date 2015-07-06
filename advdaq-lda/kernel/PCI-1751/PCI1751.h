/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1751_H
#define _ADV_PCI1751_H


#include "../include/advdrv.h"
#include "../../include/advdevice.h"

#define MAX_DIO_PORT   9
#define MAX_EVENT_NUM  6

#define PCI1751 0x1751
#define MIC3751 0x3751
struct ports_array
{
     INT32U start;
     INT32U num;
};

struct ports_status
{
     INT8U status[MAX_DIO_PORT];
};


typedef struct _private_data
{
	ULONG  		iobase;
	ULONG  		iolength;
	ULONG   	ioDMAbase;
	ULONG   	ioDMAlength;
	INT32U          boardID;

	INT32U          device_type; /* identify PCI-1751 or MIC-3751 */
	INT32U          dio_group_num;
	
	INT32U		pci_slot;
	INT32U		pci_bus;
	INT32U   	irq;	

	unsigned char 	dovalue_save[9];//save do value
	unsigned short	cnt_init_data[3];//save counter data
     
	/* for event function */
	wait_queue_head_t  event_wait;
	adv_process_info_header ptr_process_info;

        /* for interrupt contorl register */
	INT32U       usIntSrcConfig[3]; //save the config source of trigger
     
	unsigned short     usInterruptSrc;
	INT32U     IsCascadeMode;
	INT8U      usDioPortDir[9];
	struct ports_array       ports_event_scan[MAX_EVENT_NUM]; 
	struct ports_status     ports_status_event[MAX_EVENT_NUM];

	spinlock_t 	spinlock;
	int		config;
	unsigned short	sigpid;
}private_data;

int adv_resetdevice( adv_device *device);
int adv_opendevice( adv_device *device);
int adv_closedevice( adv_device *device);
int adv_ioctrl( adv_device *device, unsigned int cmd, unsigned long arg);
int adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  );

int adv_get_config( adv_device *device, void* arg);
int adv_clock_ioctl( adv_device *device, void* arg);


int adv_cnt_ioctl(adv_device *device, void *arg);
int adv_dio_ioctl(adv_device *device, void *arg);
int adv_clock_setup(adv_device *device, adv_clock *clock );

int advdrv_request_irq(unsigned int irq,int (*handler)(int,void *,struct pt_regs *),unsigned long flags,const char *device, void *dev_id);

INT32S adv_port_read(adv_device *device, INT32U port, void *databuf, INT32U datalen);
INT32S adv_port_write(adv_device *device, INT32U port, void* databuf, INT32U datalen);

INT32S adv_device_set_property(adv_device *device, INT16U nID, void*  pData, INT32S  lDataLength);
INT32S adv_device_get_property(adv_device *device, INT16U nID, void*  pData, INT32S* pDataLength);

INT32S adv_enable_event(adv_device *device,USHORT EventType, USHORT Enabled,USHORT Count);
INT32S adv_check_event(adv_device *device, INT32U *EventType, INT32U Milliseconds);

irqreturn_t pci1751_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);

#endif
