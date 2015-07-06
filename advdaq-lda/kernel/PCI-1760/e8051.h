#ifndef	__E8051_H__
#define  __E8051_H__

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
 * 	e8051.h
 * Abstract:                                                          
 * 	This is a part of the Advantech Source Code.
 * 	Copyright (C) 1998 Advantech Corporation.
 * 	All rights reserved.
 *                                                                         
 * Version history		                                           
 *	11/20/1998		Create by ytwu (windows driver team)
 *	7/5/2006                Transplant by LI.ANG
 *                                                                         
 ***************************************************************************/	


#define IOPC_OMB0		0x0c
#define IOPC_OMB1		0x0d
#define IOPC_OMB2		0x0e
#define IOPC_OMB3		0x0f

#define IOPC_IMB0		0x1c
#define IOPC_IMB1		0x1d
#define IOPC_IMB2		0x1e
#define IOPC_IMB3		0x1f

#define INTCSR	0x38

#define C_PSEUDO		0x0 /* Clear IMB2 Register Value */
#define CRO_VALUE		0x1 /* Relay out value */
#define CRO_READBACK		0x2 /* Relay out readback */
#define CRO_EN_TIMER_ON		0x3 /* Relay out timer on enable */
#define CRO_EN_TIMER_OFF	0x4 /* Relay out timer off enable */

/* 1998/11/24 Eric  */
#define CREAD_INT_REG           0x7 /* Read Internal Register */
#define CREAD_VER_FIRM          0xe /* Read Firmware Version */
#define CREAD_VER_HARD          0xf /* Read Hardware Version */
/* Done */


#define CP1_HI			0x10 /* PWM1 HI value(1/10ms) */
#define CP1_LO			0x11 /* PWM1 LO value(1/10ms) */
#define CP2_HI			0x12 /* PWM2 HI value(1/10ms) */
#define CP2_LO			0x13 /* PWM2 LO value(1/10ms) */
#define CP1_COUNT		0x14 /* PWM1 pulse out counting value */
#define CP2_COUNT		0x15 /* PWM2 pulse out counting value */
#define CP_EN_OUTPUT	        0x1f /* PWM output enable */

#define CIS_EN_FILTER	        0x20 /* Isolated input filter enable */
#define CIS_EN_PAT		0x21 /* Isolated input pattern match enable */
#define CIS_PATTERN		0x22 /* Isolated input pattern match value */
#define CIS_EN_RIS		0x23 /* Isolated input rising change enable */
#define CIS_EN_FAL		0x24 /* Isolated input falling change enable */

#define CCO_EN			0x28 /* Counter enable  */
#define CCO_PRESET		0x29 /* Clear counter to preset value */
#define CCO_EN_OVERFLOW_INT     0x2A /* Counter overflow interrupt enable */
#define CCO_EN_PAT_INT		0x2b /* Counter match interrupt enable */
#define CCO_COUNT_EDGE		0x2C /* Counter counting edges */
#define CCO_COUNT		0x2D /* Counter up/down counting setting */

#define CCO_READ		0x2F /* Read COUNTERn counting value */

#define CIS0_HI_FILTER	        0x30 /* Isolated input 0 HI filter value */
#define CIS1_HI_FILTER	        0x31 /* Isolated input 1 HI filter value */
#define CIS2_HI_FILTER	        0x32 /* Isolated input 2 HI filter value */
#define CIS3_HI_FILTER	        0x33 /* Isolated input 3 HI filter value */
#define CIS4_HI_FILTER	        0x34 /* Isolated input 4 HI filter value */
#define CIS5_HI_FILTER	        0x35 /* Isolated input 5 HI filter value */
#define CIS6_HI_FILTER	        0x36 /* Isolated input 6 HI filter value */
#define CIS7_HI_FILTER	        0x37 /* Isolated input 7 HI filter value */

#define CIS0_LO_FILTER	        0x38 /* Isolated input 0 LO filter value */
#define CIS1_LO_FILTER	        0x39 /* Isolated input 1 LO filter value */
#define CIS2_LO_FILTER	        0x3A /* Isolated input 2 LO filter value */
#define CIS3_LO_FILTER	        0x3B /* Isolated input 3 LO filter value */
#define CIS4_LO_FILTER	        0x3C /* Isolated input 4 LO filter value */
#define CIS5_LO_FILTER	        0x3D /* Isolated input 5 LO filter value */
#define CIS6_LO_FILTER	        0x3E /* Isolated input 6 LO filter value */
#define CIS7_LO_FILTER	        0x3F /* Isolated input 7 LO filter value */

#define CCO0_PRESET		0x40 /* Counter 0 preset value */
#define CCO1_PRESET		0x41 /* Counter 1 preset value */
#define CCO2_PRESET		0x42 /* Counter 2 preset value */
#define CCO3_PRESET		0x43 /* Counter 3 preset value */
#define CCO4_PRESET		0x44 /* Counter 4 preset value */
#define CCO5_PRESET		0x45 /* Counter 5 preset value */
#define CCO6_PRESET		0x46 /* Counter 6 preset value */
#define CCO7_PRESET		0x47 /* Counter 7 preset value */

#define CCO0_MATCH		0x48 /* Counter 0 match value */
#define CCO1_MATCH		0x49 /* Counter 1 match value */
#define CCO2_MATCH		0x4A /* Counter 2 match value */
#define CCO3_MATCH		0x4B /* Counter 3 match value */
#define CCO4_MATCH		0x4C /* Counter 4 match value */
#define CCO5_MATCH		0x4D /* Counter 5 match value */
#define CCO6_MATCH		0x4E /* Counter 6 match value */
#define CCO7_MATCH		0x4F /* Counter 7 match value */

#define CRO0_ON_TIMER		0x50 /* Relay 0 timer on value(10ms) */
#define CRO1_ON_TIMER		0x51 /* Relay 1 timer on value(10ms) */
#define CRO2_ON_TIMER		0x52 /* Relay 2 timer on value(10ms) */
#define CRO3_ON_TIMER		0x53 /* Relay 3 timer on value(10ms) */
#define CRO4_ON_TIMER		0x54 /* Relay 4 timer on value(10ms) */
#define CRO5_ON_TIMER		0x55 /* Relay 5 timer on value(10ms) */
#define CRO6_ON_TIMER		0x56 /* Relay 6 timer on value(10ms) */
#define CRO7_ON_TIMER		0x57 /* Relay 7 timer on value(10ms) */

#define CRO0_OFF_TIMER		0x58 /* Relay 0 timer off value(10ms) */
#define CRO1_OFF_TIMER		0x59 /* Relay 1 timer off value(10ms) */
#define CRO2_OFF_TIMER		0x5A /* Relay 2 timer off value(10ms) */
#define CRO3_OFF_TIMER		0x5B /* Relay 3 timer off value(10ms) */
#define CRO4_OFF_TIMER		0x5C /* Relay 4 timer off value(10ms) */
#define CRO5_OFF_TIMER		0x5D /* Relay 5 timer off value(10ms) */
#define CRO6_OFF_TIMER		0x5E /* Relay 6 timer off value(10ms) */
#define CRO7_OFF_TIMER		0x5F /* Relay 7 timer off value(10ms) */

#define C_RD_INTF	        0x60 /* Read Interrupt flag */
#define C_RD_STATE_INTF	  	0x61 /* Read Interrupt flag */
#define C_RD_COUNTER_INFT 	0x62 /* Read Counter Intrrupt flag */

#define CSYS_GETMODE		0xEF

#define EmboRead(x, y)          advInpw(x, y)
#define EmboWrite(x, y)         advOutpw(x, y)

#define SET_PCI_INTCSR(x, y)    advOutp(x, 0x39, y)

#endif /*E8051*/
