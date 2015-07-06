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

//#define PCI1752_DEBUG
/**
 *  adv_init_device_di_range - initialize device  start and end di channel
 *  @privdata - pointer to the special data of the device
 *  
 */
INT32S adv_init_device_di_range(private_data *privdata)
{
     ULONG err = 0;
     switch(privdata->device_type)
     {
     case PCI1752:
	  privdata->di_port_reg_start = 0;
	  privdata->di_port_num = 0;
	  break;
     case PCI1754:
	  privdata->di_port_reg_start = 0;
	  privdata->di_port_num = 8;
	  break;
	
     case PCI1756:
	  privdata->di_port_reg_start = 0;
	  privdata->di_port_num = 4;
	  break;
     default:
	  err = -1;
	  break;
	  
     }
     return err;
}
/**
 *  adv_init_device_do_range - initialize the device start and end do channel
 *  @privdata - pointer to the special data of the device
 *  
 */
INT32S adv_init_device_do_range(private_data *privdata)
{
     ULONG err = 0;
      
     switch(privdata->device_type)
     {
     case PCI1752:
	  privdata->do_port_reg_start = 0;
	  privdata->do_port_num = 8;
	  break;

     case PCI1754:
	  privdata->do_port_reg_start = 0;
	  privdata->do_port_num = 0;
	  break;
	
     case PCI1756:
	  privdata->do_port_reg_start = 0;
	  privdata->do_port_num = 4;
	  break;
     default:
	  err = -1;
	  break;
	  
     }
     return err;
}
/**
 * adv_clear_device_int_flag - clear the device's interrupt flag
 * @privdata - pointer to the special data of the device
 * 
 */
INT32S adv_clear_device_int_flag(private_data *privdata)
{
	ULONG err = 0;
     
	switch(privdata->device_type)
	{
	case PCI1752:
		break;
	case PCI1754:
		advOutpw(privdata, 0x08, 0x0008);
		advOutpw(privdata, 0x0a, 0x0008);
		advOutpw(privdata, 0x0c, 0x0008);
		advOutpw(privdata, 0x0e, 0x0008);
		break;
	case PCI1756:
		advOutpw(privdata, 0x08, 0x0008);
		advOutpw(privdata, 0x0a, 0x0008);
		break;
	default:
		err = -1;
		break;
	}
	return err;
}
/**
 *  adv_disable_device_int - disable the device interrupt and latch function
 *  @privdata - pointer to the special data of the device
 *  
 */
INT32S adv_disable_device_int(private_data *privdata)
{
	ULONG err = 0;

	switch(privdata->device_type)
	{
	case PCI1752:
		break;
	case PCI1754:
		advOutpw(privdata, 0x08, 0x0000);
		advOutpw(privdata, 0x0a, 0x0000);
		advOutpw(privdata, 0x0c, 0x0000);
		advOutpw(privdata, 0x0e, 0x0000);
		break;
	case PCI1756:
		advOutpw(privdata, 0x08, 0x0000);
		advOutpw(privdata, 0x0a, 0x0000);
		break;
	default:
		err = -1;
		break;
	}
	return err;
}

/**
 * adv_disable_device_freeze - disable the freeze function of the device
 * 
 */
INT32S adv_disable_device_freeze(private_data *privdata)
{
	ULONG err = 0;

	switch(privdata->device_type)
	{
	case PCI1752:
	case PCI1756:
		advOutpw(privdata, 0x12, 0x0000);
		break;
	case PCI1754:
		break;
	default:
		err = -1;
		break;
	}
	return err;
}
/**
 * adv_get_device_boardID - get boardID of the device
 * @privdata - pointer to the special data of the device
 * 
 */
INT32S adv_get_device_boardID(private_data *privdata)
{  
     privdata->boardID = advInpw(privdata, 0x10) & 0x0f;
     return 0;
}
/**
 * adv_get_int_control_code - get the control code according interrupt status
 * @privdata - pointer to the special data of the device
 * @ctlcode - output contorl code
 * 
 */
/* INT32U adv_get_int_control_code(private_data *privdata, BYTE *ctlcode) */
/* { */
/* 	adv_process_info_header *p = &privdata->ptr_process_info; */
/* 	INT32U i; */
/* 	INT32U enable; */
     
/* 	*ctlcode = 0; */
     
/* 	for(i = 0; i < 4; i++) */
/* 	{ */
/* 		enable = adv_process_info_isenable_event_all(p, i); */
/* 		if(enable){ */
/* 			*ctlcode |= 1<<i; */
/* 		} */
/* 	} */
/* 	printk("ctlcode : %x\n", *ctlcode); */
/* 	return 0; */
/* } */

/* caculate the control code according the signal status
 * the status include:  interrupt enable/disable status
 *                      interrupt trigger mode status*/
/* int adv_get_contorl_code(ULONG device_type, ULONG status, BYTE *ctrlcode) */
/* { */
/*      int ret = 0; */
/*      *ctrlcode = 0; */

/*      switch(device_type) */
/*      { */
/*      case PCI1752: */
/*      case PCI1733: */
/* 	  if(status & 0x0001) *ctrlcode |= 0x01; /\* for IDI0 *\/ */
/* 	  if(status & 0x0002) *ctrlcode |= 0x02; /\* for IDI1 *\/ */
/* 	  if(status & 0x010000) *ctrlcode |= 0x04; /\* for DI0 or IDI16 *\/ */
/* 	  if(status & 0x020000) *ctrlcode |= 0x08; /\* for DI1 or IDI17 *\/ */
/* 	  break; */
/*      case PCI1736: */
/* 	  if(status & 0x0001) *ctrlcode |= 0x01; /\* for IDI0 *\/ */
/* 	  if(status & 0x0002) *ctrlcode |= 0x02; /\* for IDI1 *\/ */
/* 	  break; */
/*      default: */
/* 	  ret = -1; */
/* 	  break; */
/*      } */
/*      return ret; */
/* } */


