/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1752 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1752 driver module and device. 
Version history
	05/25/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */
#include <linux/module.h>

#include "PCI1752.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif

/**
 * pci1752_interrupt_handler - interrupt service route
 * @irq - irq number
 * @dev_id - private data
 * @regs - point to the data of the register
 */
irqreturn_t pci1752_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	private_data   *privdata;
	INT16U cmd;
	INT32S ret = 0;
	INT32S i;
	INT32S j;
	INT32U count = 0;
	
	privdata=(private_data *)dev_id;
     
	spin_lock(&privdata->spinlock);
	
	count = privdata->di_port_num;
	
	for(i = 0; i < count/2; i++){
		cmd = advInpw(privdata, 0x08 + 2*i);

		if(cmd & 0x08){
			ret = 1;
			for(j = 0; j < count; j++){
				privdata->event_di_status[i][j] = advInp(privdata, j);
			}
			adv_process_info_set_event_all(&privdata->ptr_process_info, i, 1);
			advOutpw(privdata, 0x08+2*i, cmd);
			
		}
	}
	
	wake_up_interruptible(&privdata->event_wait);
	spin_unlock(&privdata->spinlock);
	return IRQ_RETVAL(ret);
}
