
/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCM3761I_H
#define _ADV_PCM3761I_H
#include "../include/advdrv.h"
#include "../../include/advdevice.h"

#include <linux/list.h>
#include <linux/delay.h>
#include <asm/timex.h>


#define PCM3761I               0x3761
 
#define MAX_CNTS              8
#define INTERNAL_CLOCK_RATE   10000000.0 /* 10MHz */
#define PACER_RATE            1000000.0 /* 1MHz */
#define BOARD_NOT_SUPPORT     1

#define MAX_EVENT_NUM         10

#define MAX_DI_CHANNEL        16
#define MAX_DO_CHANNEL        16
#define MAX_DI_PORT           2
#define MAX_DO_PORT           2
#define MAX_DIO_PORT          2

#define INFINITE              0xffffffff

#define adv_set_flag(x, y, z)       ((x)->y = z)

typedef struct _private_data
{
	adv_process_info_header ptr_process_info;
	wait_queue_head_t       event_wait;
	spinlock_t              spinlock;
	struct semaphore        *dio_sema;

	INT32U     iobase;
	INT32U     iolength;
	INT32U     irq;
	INT32U     device_type;
	INT16U     board_id;
	INT8U	   pci_slot;
	INT8U	   pci_bus;


	INT16U	   evt_cnt;
	INT16U     int_trig_mode;
	INT16U     trig_rising;
	INT16U     trig_falling;
	
	INT8U      do_prestate[MAX_DO_PORT]; /* for DO previous status */

	INT32U     overflow[MAX_CNTS];

} private_data;


INT32S adv_dio_ioctl(adv_device *device, void *arg);

INT32S adv_read_port(adv_device *device, void *arg);
INT32S adv_write_port(adv_device *device, void *arg);

INT32S adv_check_event(adv_device *device, void *arg);
INT32S adv_enable_event(adv_device *device, void *arg);
INT32S adv_clear_flag(adv_device *device, void *arg);

INT32S adv_dev_get_property(adv_device *device, void *arg);
INT32S adv_dev_set_property(adv_device *device, void *arg);
/* INT32S adv_get_err_code(adv_device *device, void * arg); */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pcm3761i_interrupt_handler(INT32S irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pcm3761i_interrupt_handler(INT32S irq, void *dev_id);
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1762_interrupt_handler(INT32S irq, void *dev_id, struct pt_regs *regs);
#else
irqreturn_t pci1762_interrupt_handler(INT32S irq, void *dev_id);
#endif


void adv_tasklet_action(INT32U data);

#endif
