/*****************************************************************
 *		
 *	Copyright 2006 IAG Software Team, 
 *
 *		BeiJing R&D Center 
 *
 *		Advantech Co., Ltd.
 *
 *	Advantech PCI-1784 Device driver for Linux
 *
 * File Name: 
 *	advcnt.c
 * Abstract:
 *	This file contains routines for PCI1784 counter.
 *
 * Version history
 *      11/22/2006			Create by zhenyu.zhang
 *****************************************************************/
#define __NO_VERSION__
#include "PCI1784.h"

INT32S adv_set_counter_param(adv_device *device, CTL_CNTR_PARAMS *arg)
{
	int i;
	CTL_CNTR_PARAMS CntrParam;	
	private_data *privdata = (private_data *) (device->private_data);
	if (copy_from_user(&CntrParam, arg, sizeof(CTL_CNTR_PARAMS))) {
		return MemoryCopyFailed;
	}

	// counter mode
	for (i = 0; i < 4; i++)	{
		if (CntrParam.usDirtyFlag[i])		{
			privdata->ulDevPara[i] = CntrParam.ulCntrMode[i];
			advOutpdw(privdata, i*4, privdata->ulDevPara[i]);
		}
	}
	
	if (CntrParam.usDirtyFlag[4]) 	{
		if (CntrParam.usClsInt){
			privdata->ulIntSourceData = CntrParam.ulIntSrc;
		}else{
			privdata->ulIntSourceData |= CntrParam.ulIntSrc;
		}
		
		if (privdata->ulIntSourceData != 0){
			privdata->ulIntSourceData |= 0x80000000;
		}
		advOutpdw(privdata, 0x20, privdata->ulIntSourceData);
	}
	
	if (CntrParam.usDirtyFlag[5]) 	{
		privdata->ulClkCtl = CntrParam.ulClkCtl;
		advOutpdw(privdata, 0x24, privdata->ulClkCtl);
	}

	if (CntrParam.usDirtyFlag[6]) 	{
		privdata->usDoMode = CntrParam.usDoMode;
		advOutpw(privdata, 0x32, privdata->usDoMode);
	}

	if ((CntrParam.usDirtyFlag[7]) && (privdata->SubsystemID == 0xA200)) 	{
		privdata->usIndexReset = CntrParam.usIndexReset;
		advOutpdw(privdata, 0x2c, privdata->usIndexReset << 4);
	}
/*	
printk("counter 0 mode is %x\n",advInpdw(privdata,0));
printk("counter 1 mode is %x\n",advInpdw(privdata,4));
printk("counter 2 mode is %x\n",advInpdw(privdata,8));
printk("counter 3 mode is %x\n",advInpdw(privdata,12));
printk("counter 20 mode is %x\n",advInpdw(privdata,0x20));
printk("counter 24 mode is %x\n",advInpdw(privdata,0x24));
printk("counter 32 mode is %x\n",advInpdw(privdata,0x32));
printk("counter 2c mode is %x\n",advInpdw(privdata,0x2c));
*/	return 0;
}

INT32S adv_set_counter_status(adv_device *device, cnt_struct *structs, INT32U *data)
{
	private_data *privdata = (private_data *) (device->private_data);

	USHORT usCounter;
	INT32U* pCntrStatus;

	pCntrStatus = data;

	for (usCounter = 0; usCounter < 4; usCounter++)	{
		privdata->ulCntrStatus[usCounter] = pCntrStatus[usCounter];
	}

	return SUCCESS;

}


INT32S adv_get_counter_status(adv_device *device, cnt_struct *structs, INT32U *data)
{
	private_data *privdata = (private_data *) (device->private_data);

	USHORT usCounter;
	INT32U* pCntrStatus;

	pCntrStatus = data;

	for (usCounter = 0; usCounter < 4; usCounter++)	{
		pCntrStatus[usCounter] = privdata->ulCntrStatus[usCounter];
	}
	return SUCCESS;

}
