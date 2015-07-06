/**************************************************************************
 * 
 *                                                                         
 *		Copyright 2004 IAG Software Team                   
 *                                                                         
 *			BeiJing R&D Center                         
 *                                                                         
 *			Advantech Co., Ltd.                        
 *                                                                         
 *                                                                          
 * File Name:                                                         
 * 	e8051.c
 * Abstract:                                                          
 * 	This is a part of the Advantech Source Code.
 * 	Copyright (C) 1998 Advantech Corporation.
 * 	All rights reserved.
 *                                                                         
 * Version history		                                           
 *	11/20/1998			Create by ytwu (windows driver team)
 *	7/5/2006                        Transplant by LI.ANG
 *                                                                         
 ***************************************************************************/	

#define __NO_VERSION__
#include "PCI1760.h"

/**************************************************************
 * Description:  Issue command to PCI-1760 embedded subsystem.
 *                                                            
 * Input:        device    -Points to the device              
 *               data      -store the data read from I/O port 
 * Ouput:        errno                                        
 *               SUCCESS                                            
 **************************************************************/
INT32S command_out(private_data *privdata, INT32U data, INT16U cmd)
{
/* 	private_data *privdata = (private_data *) (device->private_data); */
	INT16U i, tmp;


	data |= (INT32U) cmd << 16;
	advOutpdw(privdata, IOPC_OMB0, data);

	for (i = 0; i < 65534; i++) {
		tmp = advInp(privdata, IOPC_IMB2);
		if ((tmp & 0xff) == CRO_VALUE)
			break;
	}
	
	if (i >= 65534)
		return EventTimeOut;

	return SUCCESS;
}

/* 1998/12/08/FRED/ Added */
/********************************************************************
 * Description:  Check to see if the Embedded system is in Op mode.
 *                                                            
 * Input:        device    -Points to the device              
 *               data      -store the data read from I/O port 
 * Ouput:        errno                                        
 *               SUCCESS                                            
 *******************************************************************/
INT32S is_em_op_mode(private_data *privdata)
{
/* 	private_data *privdata = (private_data *) (device->private_data); */
	char tag;

	command_out(privdata, 0, CSYS_GETMODE);
	tag = (char) (EmboRead(privdata, IOPC_IMB0) & 0x0ff);

	return (tag == 'N');
}
/* DONE/FRED/ */



