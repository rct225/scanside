/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1753 Device driver for Linux

File Name:
	advinit.c
Abstract:
	Initialize the PCI-1753 driver module and device. 
Version history
	05/25/2006		Create by zhiyong.xie

************************************************************************/	

/* base module includes */

#include <linux/module.h>

#include "PCI1753.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif
static void read_di_port(private_data *privdata, INT32U index)
{
	INT32U port;
	INT32U port_offset;
	
	for(port = 0; port < privdata->max_dio_group * 3; port++) {
		if(port > 11){
			port_offset = (INT32U)port/3*4 + (INT32U)port%3 + 16;
		} else {
			port_offset = (INT32U)port/3*4 + (INT32U)port%3;
		}
		privdata->ports_status_event[index].status[port] = advInp(privdata, port_offset);
		;
	}
	
}

/**
 * pci1753_interrupt_handler - isr of pci1753
 * @irq - number of irq
 * @dev_id - pointer to private data register to system
 * @regs - save register of system
 */
irqreturn_t pci1753_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	private_data   *privdata;
	BYTE cmd;
	INT32U i;
	INT32U ret = 0;
	privdata=( private_data *)dev_id;

	
	spin_lock(&privdata->spinlock);
	
	cmd = advInp(privdata, INT_PATTERN0_CTRL_OFFSET);
	/* Pattern Match Interrupt */
	if(cmd & 0x02)	
	{
		adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
		read_di_port(privdata, 0);
		ret = 1;
	}
	/* Change Status Interrupt */
	if(cmd & 0x08)	
	{
		adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
		read_di_port(privdata, 1);
		ret = 1;
	}
	/* DI_Interrupt16 */
	if(cmd & 0x80)	
	{
		adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 2);
		read_di_port(privdata, 2);
		ret = 1;
	}
	advOutp(privdata, INT_PATTERN0_CTRL_OFFSET, cmd);
	
	/* di interrupt 40 64 88 etc */
	for(i = 1; i < 4; i++)
	{
		cmd = advInp(privdata, INT_GROUP0_CTRL_OFFSET + i);
		if(cmd & 0x80){
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2+i, 1);
			read_di_port(privdata, 2 + i);
			ret = 1;
			advOutp(privdata, INT_GROUP0_CTRL_OFFSET + i, cmd);
		}
		
	}
	
	/* PCI-1753E expand board */
	if(privdata->us_exp_board == 1){
		cmd = advInp(privdata, INT_PATTERN1_CTRL_OFFSET);
		/* Pattern Match Interrupt */
		if(cmd & 0x02)	
		{
			adv_process_info_set_event_all(&privdata->ptr_process_info, 6, 1);
			read_di_port(privdata, 6);
			ret = 1;
		}
		/* Change Status Interrupt */
		if(cmd & 0x08)	
		{
			adv_process_info_set_event_all(&privdata->ptr_process_info, 7, 1);
			read_di_port(privdata, 7);
			ret = 1;
		}
		/* di interrupt 16 */
		if(cmd & 0x80)	
		{
			adv_process_info_set_event_all(&privdata->ptr_process_info, 8, 1);
			read_di_port(privdata, 8);
			ret = 1;
		}
		advOutp(privdata, INT_PATTERN1_CTRL_OFFSET, cmd);
	
		/* DI_Interrupt 136 160 184 etc */
		for(i = 1; i < 4; i++)
		{
			cmd = advInp(privdata, INT_GROUP4_CTRL_OFFSET + i);
			if(cmd & 0x80){
				adv_process_info_set_event_all(&privdata->ptr_process_info, 8+i, 1);
				read_di_port(privdata, 8+i);
				ret = 1;
				advOutp(privdata, INT_GROUP4_CTRL_OFFSET + i, cmd);
			}
		}
	}
	
	if(ret)
		wake_up_interruptible(&privdata->event_wait);

	spin_unlock(&privdata->spinlock);
	return IRQ_RETVAL(ret);

}


