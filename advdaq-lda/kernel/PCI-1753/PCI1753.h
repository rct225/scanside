/* ADVANTECH Co.Ltd */
#ifndef __ADV_PCI1753_H__
#define __ADV_PCI1753_H__


#include "../include/advdrv.h"
#include "../../include/advdevice.h"

#define DIO_PORT0_OFFSET    0
#define DIO_PORT1_OFFSET    1
#define DIO_PORT2_OFFSET    2

#define DIO_PORT3_OFFSET    4
#define DIO_PORT4_OFFSET    5
#define DIO_PORT5_OFFSET    6

#define DIO_PORT6_OFFSET    8
#define DIO_PORT7_OFFSET    9
#define DIO_PORT8_OFFSET    10

#define DIO_PORT9_OFFSET    12
#define DIO_PORT10_OFFSET   13
#define DIO_PORT11_OFFSET   14

#define DIO_PORT12_OFFSET   32
#define DIO_PORT13_OFFSET   33
#define DIO_PORT14_OFFSET   34

#define DIO_PORT15_OFFSET   36
#define DIO_PORT16_OFFSET   37
#define DIO_PORT17_OFFSET   38

#define DIO_PORT18_OFFSET   40
#define DIO_PORT19_OFFSET   41
#define DIO_PORT20_OFFSET   42

#define DIO_PORT21_OFFSET   44
#define DIO_PORT22_OFFSET   45
#define DIO_PORT23_OFFSET   46

#define DIO_GROUP0_CFG_OFFSET    3
#define DIO_GROUP1_CFG_OFFSET    7
#define DIO_GROUP2_CFG_OFFSET    11
#define DIO_GROUP3_CFG_OFFSET    15
#define DIO_GROUP4_CFG_OFFSET    35
#define DIO_GROUP5_CFG_OFFSET    39
#define DIO_GROUP6_CFG_OFFSET    43
#define DIO_GROUP7_CFG_OFFSET    47

#define INT_GROUP0_CTRL_OFFSET   16
#define INT_GROUP1_CTRL_OFFSET   17
#define INT_GROUP2_CTRL_OFFSET   18
#define INT_GROUP3_CTRL_OFFSET   19
#define INT_GROUP4_CTRL_OFFSET   48
#define INT_GROUP5_CTRL_OFFSET   49
#define INT_GROUP6_CTRL_OFFSET   50
#define INT_GROUP7_CTRL_OFFSET   51

#define PATTERN0_VALUE_OFFSET        20
#define PATTERN1_VALUE_OFFSET        52

#define PATTERN0_ENABLE_OFFSET       24
#define PATTERN1_ENABLE_OFFSET       56

#define INT_PATTERN0_CTRL_OFFSET     16
#define INT_PATTERN1_CTRL_OFFSET     48

#define CHANGE0_ENABLE_OFFSET        28
#define CHANGE1_ENABLE_OFFSET        60

#define INT_CHANGE0_CTRL_OFFSET      16
#define INT_CHANGE1_CTRL_OFFSET      48

#define CFG_PORTA_INPUT           0x10
#define CFG_PORTB_INPUT           0x02
#define CFG_PORTC_LOW_INPUT       0x01
#define CFG_PORTC_HIGH_INPUT      0x08

#define MAX_DIO_GROUP   8
#define MAX_DIO_PORT    24
#define MAX_EVENT_NUM  12
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
  
	INT32U		pci_slot;
	INT32U		pci_bus;
	unsigned int   	irq;	
	

	/* for event function */
	wait_queue_head_t  event_wait;
	adv_process_info_header ptr_process_info;
	
	struct ports_array       ports_event_scan[MAX_EVENT_NUM]; 
	struct ports_status     ports_status_event[MAX_EVENT_NUM];

	/* the number of io groups
	 * PCI-1753: 0
	 * PCI-1753E: 1
	 * */
	INT16U    us_exp_board;	
	INT32U    max_dio_group;
	INT32U    dio_port_dir[MAX_DIO_GROUP];
	
	INT8U     cDiStatusChangeEnabledChannel[2];
	INT8U     cDiPatternMatchEnabledChannel[2];
	INT8U     cDiPatternMatchValue[2];
	INT8U     int_trigger_signal[24];
	
	/* for interrupt */
	INT32U int_src_config[MAX_DIO_GROUP];
	INT16U int_ctrl_reg[MAX_DIO_GROUP];
	
	spinlock_t 	spinlock;
	INT32U		config;
}private_data;

int adv_resetdevice( adv_device *device);
int adv_opendevice( adv_device *device);
int adv_closedevice( adv_device *device);
int adv_ioctrl( adv_device *device, unsigned int cmd, unsigned long arg);
int adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  );
int adv_mmap( adv_device *device, struct vm_area_struct * vma);

int adv_get_config( adv_device *device, void* arg);
int adv_clock_ioctl( adv_device *device, void* arg);


int advdrv_mem_free(adv_memory *pdam);
int advdrv_mem_alloc(adv_memory *padvmem, unsigned int size);
int advdrv_mem_mmap(adv_memory *padvmem,struct vm_area_struct *vma);

int adv_dio_ioctl(adv_device *device, void *arg);
int adv_clock_setup(adv_device *device, adv_clock *clock );

INT32S adv_port_read(adv_device *device, INT32U port, void *databuf, INT32U datalen);
INT32S adv_port_write(adv_device *device, INT32U port, void* databuf, INT32U datalen);
INT32S adv_device_set_property(adv_device *device, USHORT nID, void *ubuf, INT32U ulen);

INT32S adv_device_get_property(adv_device *device,INT16U nID, void* ubuf,INT32U* ulen);

irqreturn_t pci1753_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);

INT32S adv_enable_event(adv_device *device,ULONG EventType,USHORT Enabled,USHORT Count);
INT32S adv_check_event(adv_device *device, INT32U *EventType, INT32U Milliseconds);

#endif
