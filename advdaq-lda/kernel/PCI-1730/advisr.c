/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1730 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1730 driver module and device. 
Version history
	05/25/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
#include <linux/module.h>

#include "PCI1730.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1730_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1730_interrupt_handler(int irq, void *dev_id)
#endif
{
	private_data   *privdata;
	unsigned char cmd;
     
	privdata=(private_data *)dev_id;
     

	cmd = advInp(privdata, 0x10);
	if(!(cmd & 0x0f))
	{
		return IRQ_RETVAL(0);
	}

	spin_lock(&privdata->spinlock);

     
	if(cmd & 0x01){
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
	}
	if(cmd & 0x02){
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
	}
	if(cmd & 0x04){
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
	}
	if(cmd & 0x08){
		adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
	}
	/* clear interrupt flag */
	wake_up_interruptible(&privdata->event_wait);
	spin_unlock(&privdata->spinlock);
	advOutp(privdata, 0x10, cmd);

	return IRQ_RETVAL(1);
}
