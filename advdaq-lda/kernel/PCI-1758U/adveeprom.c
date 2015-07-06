/********************************************************************
 *				                                    
 * 	    Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		    BeiJing R&D Center                          
 *	                                                            
 * 		    Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1758U Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	adveeprom.c                                             
 * Abstract:                                                   
 * 	This file contains routines for EEPROM read/write.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	09/21/2006			Create by LI.ANG
 *                                                                  
 ********************************************************************/
  
#include "PCI1758U.h"


/**
 * adv_clk_hold - EEPROM clock hold function
 *
 * @x - times to hold on the special edge
 */
static void adv_clk_hold(INT16U x)
{
	INT16U i = 0;

	if (x < 2000) {
		x= 20000;
	}
	
	do {
		i++;
	} while (i < x);
}


/**
 * adv_eeprom_read - read EEPROM
 *
 * @device: Points to the device private data
 * @addr: address of data storage at EEPROM
 * @data: data to be read from EEPROM
 */
void adv_eeprom_read(private_data *ptr, INT8U addr, INT16U *data)
{
	private_data *privdata = ptr;
	INT16U eeprom_addr;
	INT16U port_data;
	INT16U di_value;
	INT16U do_value;
	INT16U tmp;
	INT16U i;


	switch (privdata->device_type) {
	case PCI1758UDO:
		eeprom_addr = 0x1a;
		break;
	case PCI1758UDIO:
		eeprom_addr = 0x3c;
		break;
	default:
		eeprom_addr = 0x00;
		break;
	}

	tmp = 0x01;
	tmp = (tmp << 2) | 0x02;
	tmp = (tmp << 6) | addr;
	tmp = tmp << 7;
	advOutpw(privdata, eeprom_addr, 0x08); /* CS rising */
	

	for (i = 0; i < 9; i++) {
		di_value = ((tmp & (0x8000 >> i)) != 0);
		di_value = (di_value << 1) & 0x0002;
		di_value = di_value | 0x08;
		advOutpw(privdata, eeprom_addr, di_value); /* DI rising */

		adv_clk_hold(0);

		di_value = di_value | 0x04;
		advOutpw(privdata, eeprom_addr, di_value);

		adv_clk_hold(0);

		di_value = di_value & 0xfffb;
		advOutpw(privdata, eeprom_addr, di_value);

		adv_clk_hold(0);
	}

	do_value = 0;	
	for (i = 0; i < 16; i++) {
		advOutpw(privdata, eeprom_addr, 0x0c);

		adv_clk_hold(0);

		port_data = advInpw(privdata, eeprom_addr);
		do_value = (do_value << 1) | (port_data & 0x0001);

		adv_clk_hold(0);

		advOutpw(privdata, eeprom_addr, 0x08);

		adv_clk_hold(0);		
	}

	advOutpw(privdata, eeprom_addr, 0x00);

	*data = do_value;
}


/**
 * adv_eeprom_write - write EEPROM
 *
 * @ptr: Points to the device private data
 * @addr: address of data storage at EEPROM
 * @data: data to be write to EEPROM
 */
void adv_eeprom_write(private_data *ptr, INT8U addr, INT16U data)
{
	private_data *privdata = ptr;
	INT16U eeprom_addr;
	INT16U di_value;
	INT16U port;
	INT32U tmp;
	INT16U i = 0;



	switch (privdata->device_type) {
	case PCI1758UDO:
		eeprom_addr = 0x1a;
		break;
	case PCI1758UDIO:
		eeprom_addr = 0x3c;
		break;
	default:
		eeprom_addr = 0x00;
		break;
	}

	tmp = 0x01;
	tmp = (tmp << 2) | 0x01;
	tmp = (tmp << 6) | addr;
	tmp = (tmp << 16) | data;
	tmp = tmp << 7;
	advOutpw(privdata, eeprom_addr, 0x08); /* CS rising */

	for (i = 0; i < 25; i++) {
		di_value = ((tmp & (0x80000000 >> i)) != 0);
		di_value = (di_value << 1) & 0x0002;
		di_value = di_value | 0x08;
		advOutpw(privdata, eeprom_addr, di_value); /* DI rising */

		adv_clk_hold(0);

		di_value = di_value | 0x04;
		advOutpw(privdata, eeprom_addr, di_value);

		adv_clk_hold(0);

		di_value = di_value & 0xfffb;
		advOutpw(privdata, eeprom_addr, di_value);
	}

	advOutpw(privdata, eeprom_addr, 0x00);

	adv_clk_hold(0);

	advOutpw(privdata, eeprom_addr, 0x04);

	adv_clk_hold(0);

	advOutpw(privdata, eeprom_addr, 0x00);

	adv_clk_hold(0);

	advOutpw(privdata, eeprom_addr, 0x0c);

	adv_clk_hold(0);

	advOutpw(privdata, eeprom_addr, 0x08);

	adv_clk_hold(0);

	port = 0;
	while ((port == 0x00) && (i < 1000)) {		
		adv_clk_hold(0);
		port = advInpw(privdata, eeprom_addr) & 0x01;

	}
	advOutpw(privdata, eeprom_addr, 0x00);

	adv_clk_hold(0);
}


/**
 * adv_eeprom_enable - enable EEPROM write/erase
 *
 * @device: Points to the device private data
 */
void adv_eeprom_enable(private_data *ptr)
{
	private_data *privdata = ptr;
	INT16U eeprom_addr;	
	INT16U di_value;
	INT32U tmp;
	INT16U i;


	switch (privdata->device_type) {
	case PCI1758UDO:
		eeprom_addr = 0x1a;
		break;
	case PCI1758UDIO:
		eeprom_addr = 0x3c;
		break;
	default:
		eeprom_addr = 0x00;
		break;
	}
	tmp = 0x9800;
	advOutpw(privdata, eeprom_addr, 0x08); /* CS rising */


	for (i = 0; i < 10; i++) {
		di_value = ((tmp & (0x8000 >> i)) != 0);
		di_value = (di_value << 1) & 0x0002;
		di_value = di_value | 0x08;
		advOutpw(privdata, eeprom_addr, di_value); /* DI rising */

		adv_clk_hold(0);

		di_value = di_value | 0x04;
		advOutpw(privdata, eeprom_addr, di_value);

		adv_clk_hold(0);

		di_value = di_value & 0xfffb;
		advOutpw(privdata, eeprom_addr, di_value);
	}
	advOutpw(privdata, eeprom_addr, 0x00);

	adv_clk_hold(0);
	adv_clk_hold(0);
}
