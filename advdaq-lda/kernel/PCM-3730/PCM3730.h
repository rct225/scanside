/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCM3730_H
#define _ADV_PCM3730_H


#include "../include/advdrv.h"

#include "../../include/advdevice.h"

#define PCM3730 0x3730

typedef struct _private_data
{
	ULONG  		iobase;
	ULONG  		iolength;
	INT32U          boardID;
  
	INT32U		pci_slot;
	INT32U		pci_bus;
	INT32U   	irq;	
     
	unsigned short      device_type; 
	/* for DI */
	unsigned long      di_port_reg_start;
	unsigned long      di_port_num;
     
	/* for DO */
	unsigned long      do_port_reg_start;
	unsigned long      do_port_num;
	unsigned char 	dovalue_save[4];//save do value, max do port is 4 

	/* for event function */
	unsigned long      event_type;     //save which event you want to catch
	unsigned long      event_status;   //save event happened
	unsigned long      event_trigger_mode; /* save triger mode bit t channel */

	spinlock_t 	spinlock;
	int		config;
	unsigned short	sigpid;
	unsigned long      counter;

	wait_queue_head_t event_wait;	
	adv_process_info_header  ptr_process_info;
     
}private_data;

int adv_resetdevice( adv_device *device);
int adv_opendevice( adv_device *device);
int adv_closedevice( adv_device *device);
int adv_ioctrl( adv_device *device, unsigned int cmd, unsigned long arg);
int adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  );
int adv_mmap( adv_device *device, struct file *file, struct vm_area_struct * vma);

int adv_get_config( adv_device *device, void* arg);
int adv_clock_ioctl( adv_device *device, void* arg);


int advdrv_mem_free(adv_memory *pdam);
int advdrv_mem_alloc(adv_memory *padvmem, unsigned int size);
int advdrv_mem_mmap(adv_memory *padvmem,struct vm_area_struct *vma);

int adv_dio_ioctl(adv_device *device, void *arg);
int adv_clock_setup(adv_device *device, adv_clock *clock );

INT32S adv_port_read(adv_device *device, INT32U port, void *databuf, INT32U datalen);
INT32S adv_port_write(adv_device *device, INT32U port, void* databuf, INT32U datalen);

//int interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);

int adv_init_device_di_range(private_data *privdata);

int adv_init_device_do_range(private_data *privdata);
int adv_clear_device_int_flag(private_data *privdata);
int adv_get_device_boardID(private_data *privdata);
int adv_disable_device_int(private_data *privdata);

int adv_get_contorl_code(ULONG device_type, ULONG status, BYTE *ctrlcode);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pcm3730_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);

#else
irqreturn_t pcm3730_interrupt_handler(int irq, void *dev_id);

#endif



int adv_enable_event(adv_device *device,ULONG EventType,USHORT Enabled,USHORT Count);
int adv_clear_device_int_flag(private_data *privdata);
long adv_device_set_property(adv_device *device, INT16U nID,  void*  pData,INT32U  lDataLength);
long adv_device_get_property( adv_device *device, INT16U nID, void*  pData,INT32U* pDataLength);
INT32U adv_check_event(adv_device *device, INT32U *EventType, 
		       INT32U Milliseconds);
INT32U adv_get_int_control_code(private_data *privdata, BYTE *ctlcode);


#endif
