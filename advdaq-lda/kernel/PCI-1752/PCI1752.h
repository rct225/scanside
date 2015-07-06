/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1752_H
#define _ADV_PCI1752_H


#include "../include/advdrv.h"
#include "../../include/advdevice.h"

#define PCI1752 0x1752
#define PCI1754 0x1754
#define PCI1756 0x1756
#define SUB_PCI1752 0x9050
#define SUB_PCI1752U 0xa100
#define SUB_PCI1752USO 0xa101

typedef struct _private_data
{
	ULONG  		iobase;
	ULONG  		iolength;
	INT32U          boardID;
  
	INT32U		pci_slot;
	INT32U		pci_bus;
	INT32U   	irq;	
     
	INT32U          device_type; /* define the device type
					  * PCI1752
					  * PCI1754
					  * PCI1756
					  * **********************/

	INT32U          di_port_reg_start;
	INT32U          di_port_num;
	INT32U          do_port_reg_start;
	INT32U          do_port_num;
	
	/* for event function */

	/* save triger mode bit t channel */
	unsigned long      event_trigger_mode;

	/* save di status when event occur */
	/* 0 --- for Group0 interrupt
	 * 1 --- for Group1 interrupt
	 * 2 --- for Group2 interrupt
	 * 3 --- for Group3 interrupt
	 * */
	INT8U              event_di_status[4][8]; 
	INT32U             event_di_port_start[4];
	INT32U             event_di_port_num[4];
	INT32U             di_enable_latch;
	
	spinlock_t 	spinlock;
	int		config;
	unsigned short	sigpid;
	unsigned long      counter;

	wait_queue_head_t event_wait;	
	adv_process_info_header  ptr_process_info;
     
}private_data;

INT32S adv_resetdevice( adv_device *device);
INT32S adv_opendevice( adv_device *device);
INT32S adv_closedevice( adv_device *device);
INT32S adv_ioctrl( adv_device *device, unsigned int cmd, unsigned long arg);
INT32S adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  );
INT32S adv_mmap( adv_device *device, struct file *file, struct vm_area_struct * vma);

INT32S adv_get_config( adv_device *device, void* arg);
INT32S adv_clock_ioctl( adv_device *device, void* arg);


INT32S advdrv_mem_free(adv_memory *pdam);
INT32S advdrv_mem_alloc(adv_memory *padvmem, unsigned int size);
INT32S advdrv_mem_mmap(adv_memory *padvmem,struct vm_area_struct *vma);

INT32S adv_dio_ioctl(adv_device *device, void *arg);
INT32S adv_clock_setup(adv_device *device, adv_clock *clock );

INT32S advdrv_request_irq(unsigned int irq,int (*handler)(int,void *,struct pt_regs *),unsigned long flags,const char *device, void *dev_id);

INT32S adv_port_read(adv_device *device, INT32U port, void *databuf, INT32U datalen);
INT32S adv_port_write(adv_device *device, INT32U port, void* databuf, INT32U datalen);

INT32S adv_init_device_di_range(private_data *privdata);

INT32S adv_init_device_do_range(private_data *privdata);
INT32S adv_clear_device_int_flag(private_data *privdata);
INT32S adv_get_device_boardID(private_data *privdata);
INT32S adv_disable_device_int(private_data *privdata);
INT32S adv_disable_device_freeze(private_data *privdata);

INT32S adv_get_contorl_code(ULONG device_type, ULONG status, BYTE *ctrlcode);
irqreturn_t pci1752_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);

INT32S adv_enable_event(adv_device *device,ULONG EventType,USHORT Enabled,USHORT Count);
INT32S adv_clear_device_int_flag(private_data *privdata);
INT32S adv_device_set_property(adv_device *device, INT16U nID,  void*  pData,INT32U  lDataLength);
INT32S adv_device_get_property( adv_device *device, INT16U nID, void*  pData,INT32U* pDataLength);
INT32U adv_check_event(adv_device *device, INT32U *EventType, 
		       INT32U Milliseconds);
INT32U adv_get_int_control_code(private_data *privdata, BYTE *ctlcode);


#endif
