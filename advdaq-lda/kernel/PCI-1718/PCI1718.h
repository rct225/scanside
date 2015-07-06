/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1718_H
#define _ADV_PCI1718_H


#include "../include/advdrv.h"
//#include "../include/advmem.h"

#include "../../include/advdevice.h"

#define PCI1718HDU 0xa100
#define PCI1718HGU 0xa102

#define MAX_AO_CHAN_NUM 1
#define MAX_AI_DF_CHAN_NUM 8
#define MAX_AI_SE_CHAN_NUM 16

/* speed limit of sample rate */
#define SPEED_DOWNLIMIT 0
#define INT_SPEED_UPLIMIT 100000

/* define internal FAI operation mode */
#define MODE_INT 0
#define MODE_INT_SCAN 1

typedef struct _private_data
{
	PTR_T  		iobase;
	INT32U 		iolength;
	INT16U          boardID;
  
	BYTE		pci_slot;
	BYTE		pci_bus;
	INT32U   	irq;	
     
	INT16U          device_type;
	
	/* for DI */
	unsigned char 	dovalue_save[2];//save do value, max do port is 4 
	/* for counter */
	unsigned short	cnt_init_data[3];//save counter data
	/* for counter freq function */
	struct timeval     freq_pre_times[3]; /* for freq function */
	unsigned short     cnt_pre_counts[3]; /* for freq fuction */
     	/* for counter event function */
	unsigned short     evn_pre_counts[3];//save counter count for event
	unsigned short     evn_first_counts[3]; 
	unsigned long      evn_overflow[3]; 
	unsigned long      evn_err_counts[3]; 
	
	/* for AO  */
	AOSET      AOSettings[MAX_AO_CHAN_NUM];

	/* for AI */
	INT32U          chan_config; /* differential mode: 1 or SE: 0 */
	INT32U          change_gain_time; /* set delay time in get_one_sample */
	INT32U          mux_chan_count;	/* multiple channel counter for MAIBinaryIn */

	adv_user_page   *user_pages; /* save user page in kernel spaces address */
	INT32U          page_num; /* user space number in page */
	INT32U          buf_ready; /* buffer ready? 0 -- NO  1 -- low buffer ready 2 -- high buffer ready */
	INT32U          active_buf; /* 0: low buffer active 1 : high buffer active */
	
	INT32U          start;
	INT32U          page_index; /* identify which page in using*/
        INT32U          cell_index; /* identify cell index in page */
	PTR_T           current_cell; /* which buffer cell wait write   */
	PTR_T           buf_size; /* total of user buffer size */
	INT32U          ai_mode; /* 0x01 -- no fifo, 0x02 --- with fifo */
	INT32U          cyclic;
	INT32U          ai_stop;
	INT32U          fai_running;
	INT16U          over_run;
	
	INT32U          timer_clock; /* internal timer clock source */
	
	INT32U          half_fifo_size;
	INT32U          start_chan;
	INT32U          stop_chan;
	INT32U          num_chan;
	INT16U          divisor1;
	INT16U          divisor2;
	
	unsigned long   int_trigger_mode; /* save interrupt triger mode */
	spinlock_t 	spinlock;
	int		config;
	unsigned short	sigpid;
	
	struct tasklet_struct tasklet;	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	struct tq_struct fai_stop;
	task_queue tq_fai_stop;
#else
	struct work_struct fai_stop;
#endif
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

int advdrv_request_irq(unsigned int irq,int (*handler)(int,void *,struct pt_regs *),unsigned long flags,const char *device, void *dev_id);

INT32S adv_port_read(adv_device *device, INT32U port, void *databuf, INT32U datalen);
INT32S adv_port_write(adv_device *device, INT32U port, void* databuf, INT32U datalen);

INT32S adv_ao_config(adv_device *device, void *arg);
INT32S adv_ao_scale(adv_device *device, void *arg);
INT32S adv_ao_voltage_out(adv_device *device, void *arg);
INT32S adv_ao_binary_out(adv_device *device, void *arg);
INT32S adv_ai_binary_in(adv_device *device, void *arg);
INT32S adv_ai_config(adv_device *device, void *arg);
INT32S adv_ai_voltage_in(adv_device *device, void *arg);
INT32S adv_ai_scale(adv_device *device, void *arg);
INT32S adv_ai_mvoltage_in(adv_device *device, void *arg);
INT32S adv_mai_binary_in(adv_device *device, void* arg);
INT32S adv_mai_config(adv_device *device, void *arg);
INT32S adv_ai_int_start(adv_device *device, void *arg);
INT32S adv_ai_int_scan_start(adv_device *device, void *arg);
INT32U adv_ai_int_terminate(private_data *privdata);
INT32U adv_ai_int_stop(private_data *privdata);
INT32S adv_ai_check(adv_device *device, void *arg);
INT32S adv_clear_flag(adv_device *device, INT32U curbuf);
INT32S adv_fai_transfer(adv_device *device, PT_FAITransfer *lpFAITransfer);
INT32S adv_clear_overrun(adv_device *device);
INT32S adv_cnt_ioctl(adv_device *device, void *arg);
irqreturn_t pci1718_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
INT32S adv_enable_event(adv_device *device,INT32U EventType,INT16U Enabled,INT32U Count);

long adv_device_set_property(adv_device *device, INT16U nID,  void*  pData,INT32U  lDataLength);
long adv_device_get_property( adv_device *device, INT16U nID, void*  pData,INT32U* pDataLength);
INT32U adv_check_event(adv_device *device, INT32U *EventType, INT32U Milliseconds);

void pci1718_tasklet_handler(PTR_T data);

#endif
