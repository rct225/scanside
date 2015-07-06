/* ADVANTECH Co.Ltd code start */
#ifndef _ADV_PCI1757UP_H
#define _ADV_PCI1757UP_H


#include "../include/advdrv.h"
//#include "../include/advmem.h"

#include "../../include/advdevice.h"


typedef struct _private_data
{
	ULONG  		iobase;
	ULONG  		iolength;
	ULONG   	ioDMAbase;
	ULONG   	ioDMAlength;
	INT32U boardID;
  
	INT32U		pci_slot;
	INT32U		pci_bus;
	INT32U   	irq;	

	unsigned char 	dovalue_save[3];//save do value

	/* for event function */
	wait_queue_head_t  event_wait;
	adv_process_info_header ptr_process_info;
	
	/* for interrupt */
	INT32U             usIntSrcConfig; /* save interrupt source config */
	unsigned short     usInterruptSrc;	/* source of interrupt generate from
						 * is configuration of Base + 32*/
          
	unsigned short     usDioPortDir[3]; /* port configureation for input or output
					     * 0 -------IN  1------OUT
					     * 2 -------
					     * 3 ------- PCL - IN  PCH - OUT
					     * 4 ------- PCL - OUT PCH - IN
					     * [0] - PA
					     * [1] - PB
					     * [2] - PC
					     * */
	unsigned long     ulHWorSW; /* I/O configuration by Hardware(SW2) or software */
          
	spinlock_t 	spinlock;
	int		config;
	unsigned short	sigpid;
}private_data;

int adv_resetdevice( adv_device *device);
int adv_opendevice( adv_device *device);
int adv_closedevice( adv_device *device);
int adv_ioctrl( adv_device *device, unsigned int cmd, unsigned long arg);
int adv_read( adv_device *device, char *buf, int nbytes, loff_t *offset  );

INT32S adv_port_read(adv_device *device, INT32U port, void *databuf, INT32U datalen);
INT32S adv_port_write(adv_device *device, INT32U port, void* databuf, INT32U datalen);

#endif
