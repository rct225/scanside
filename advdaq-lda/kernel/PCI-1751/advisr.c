/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech PCI-1751 Device driver for Linux

File Name:
	advisr.c
Abstract:
	Initialize the PCI-1751 driver module and device. 
Version history
	02/13/2006		Create by zhiyong.xie

************************************************************************/	
#include "PCI1751.h"
static void read_di_port(private_data *privdata, INT32U index)
{
	INT32U port;
	INT32U port_offset;
	
	for(port = 0; port < MAX_DIO_PORT; port++) {
		port_offset = (INT32U)port/3*4 + (INT32U)port%3;
		privdata->ports_status_event[index].status[port] = advInp(privdata, port_offset);
		;
	}
	
}

/**
 * pci1751_interrupt_handle -
 * @irq -
 * @dev_id -
 * @regs-
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t pci1751_interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t pci1751_interrupt_handler(int irq, void *dev_id)
#endif
{
	private_data   *privdata;
	INT16U cmd = 0;
	privdata=( private_data *)dev_id;
	
	spin_lock(&privdata->spinlock);
	
	cmd = advInpw(privdata, 32);
	if (privdata->device_type == PCI1751) {
		cmd &= 0xff;
	} else {
		cmd &= 0xfff;
		advOutpDMA(privdata, 0x4d, advInpDMA(privdata, 0x4d)|0x04);
	}
	
		
	if(!(cmd & 0x888))		/* is pci1751 interrupt */
	{
		spin_unlock(&privdata->spinlock);
		return IRQ_RETVAL(0);
	}
	
	if(cmd & 0x08){ 
          	/* is pci1751 group 0 interrupt */

		
		if (privdata->usIntSrcConfig[0] != 0x03) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, 0, 1);
			read_di_port(privdata, 0);
		} else {
			if (privdata->device_type == PCI1751) {
				adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
				read_di_port(privdata, 2);
			} else {
				adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
				read_di_port(privdata, 3);
			}
		}
	}
     
	if(cmd & 0x80){		/* is pci1751 group 1 interrupt */
		if (privdata->usIntSrcConfig[1] != 0x03) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, 1, 1);
			read_di_port(privdata, 1);
		} else {
			if (privdata->device_type == PCI1751) {
				adv_process_info_set_event_all(&privdata->ptr_process_info, 3, 1);
		
				read_di_port(privdata, 3);
			} else {
				adv_process_info_set_event_all(&privdata->ptr_process_info, 4, 1);
				read_di_port(privdata, 4);
			}
		}
	}

	if(cmd & 0x800){		/* is mic1751 group 2 interrupt */
		if (privdata->usIntSrcConfig[2] != 0x03) {
			adv_process_info_set_event_all(&privdata->ptr_process_info, 2, 1);
			read_di_port(privdata, 2);
		} else {
			if (privdata->device_type == MIC3751) {
				adv_process_info_set_event_all(&privdata->ptr_process_info, 5, 1);
		
				read_di_port(privdata, 5);
			} 
		}
	}
	
	advOutpw(privdata, 32, cmd);
	wake_up_interruptible(&privdata->event_wait);
	spin_unlock(&privdata->spinlock);
	return IRQ_RETVAL(1);

}

