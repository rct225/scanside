/********************************************************************
 *				                                    
 * 	Copyright 2006 IAG Software Team,                   
 *                                                                  
 * 		BeiJing R&D Center                          
 *	                                                            
 * 		Advantech Co., Ltd.                         
 *                                                                  
 * 	Advantech PCI-1784 Device driver for Linux              
 *                                                                  
 *                                                                  
 *                                                                  
 * File Name:                                                  
 * 	advevt.c                                             
 * Abstract:                                                   
 * 	This file contains routines for events.          
 *                                                                  
 *                                                                  
 * Version history                                             
 * 	10/25/2006			Create by zhenyu.zhang
 *                                                                  
 ********************************************************************/
  
#include "PCI1784.h"
 

INT32S adv_check_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	adv_process_info_header *ptr = &privdata->ptr_process_info;
	PT_CheckEvent check_event;
	INT32S event_th;
	if (copy_from_user(&check_event, arg, sizeof(PT_CheckEvent))) {
		return MemoryCopyFailed;
	}
	
	if (!check_event.EventType) {
		wait_event_interruptible_timeout(privdata->event_wait,
						 adv_process_info_isset_event(ptr) > 0,
						 check_event.Milliseconds * HZ / 1000);

		event_th = adv_process_info_check_event(ptr);

		if (event_th <= 0) {
			check_event.EventType = 0;
		} else {
			switch (event_th - 1) {
			case 0:
				check_event.EventType = ADS_EVT_OVERFLOW_CNT0;
				break;
			case 1:
				check_event.EventType = ADS_EVT_OVERFLOW_CNT1;
				break;
			case 2:
				check_event.EventType = ADS_EVT_OVERFLOW_CNT2;
				break;
			case 3:
				check_event.EventType = ADS_EVT_OVERFLOW_CNT3;
				break;
			case 4:
				check_event.EventType = ADS_EVT_UNDERFLOW_CNT0;
				break;
			case 5:
				check_event.EventType = ADS_EVT_UNDERFLOW_CNT1;
				break;
			case 6:
				check_event.EventType = ADS_EVT_UNDERFLOW_CNT2;
				break;
			case 7:
				check_event.EventType = ADS_EVT_UNDERFLOW_CNT3;
				break;
			case 8:
				check_event.EventType = ADS_EVT_INDEX_CNT0;
				break;
			case 9:
				check_event.EventType = ADS_EVT_INDEX_CNT1;
				break;
			case 10:
				check_event.EventType = ADS_EVT_INDEX_CNT2;
				break;
			case 11:
				check_event.EventType = ADS_EVT_INDEX_CNT3;
				break;
			case 12:
				check_event.EventType = ADS_EVT_DI_INTERRUPT0;
				break;
			case 13:
				check_event.EventType = ADS_EVT_DI_INTERRUPT1;
				break;
			case 14:
				check_event.EventType = ADS_EVT_DI_INTERRUPT2;
				break;
			case 15:
				check_event.EventType = ADS_EVT_DI_INTERRUPT3;
				break;
			case 16:
				check_event.EventType = ADS_EVT_OVERCOMPARE_CNT0;
				break;
			case 17:
				check_event.EventType = ADS_EVT_OVERCOMPARE_CNT1;
				break;
			case 18:
				check_event.EventType = ADS_EVT_OVERCOMPARE_CNT2;
				break;
			case 19:
				check_event.EventType = ADS_EVT_OVERCOMPARE_CNT3;
				break;
			case 20:
				check_event.EventType = ADS_EVT_UNDERCOMPARE_CNT0;
				break;
			case 21:
				check_event.EventType = ADS_EVT_UNDERCOMPARE_CNT1;
				break;
			case 22:
				check_event.EventType = ADS_EVT_UNDERCOMPARE_CNT2;
				break;
			case 23:
				check_event.EventType = ADS_EVT_UNDERCOMPARE_CNT3;
				break;
			case 24:
				check_event.EventType = ADS_EVT_INTERRUPT_TIMER4;
				break;

			default:
				return InvalidEventType;
	
			}
		}		
	} else {
		switch (check_event.EventType) {
		case ADS_EVT_OVERFLOW_CNT0:
			event_th = 0;
			break;
		case ADS_EVT_OVERFLOW_CNT1:
			event_th = 1;
			break;
		case ADS_EVT_OVERFLOW_CNT2:
			event_th = 2;
			break;
		case ADS_EVT_OVERFLOW_CNT3:
			event_th = 3;
			break;		
		case ADS_EVT_UNDERFLOW_CNT0:
			event_th = 4;
			break;
		case ADS_EVT_UNDERFLOW_CNT1:
			event_th = 5;
			break;
		case ADS_EVT_UNDERFLOW_CNT2:
			event_th = 6;
			break;
		case ADS_EVT_UNDERFLOW_CNT3:
			event_th = 7;
			break;
		case ADS_EVT_INDEX_CNT0:
			event_th = 8;
			break;
		case ADS_EVT_INDEX_CNT1:
			event_th = 9;
			break;
		case ADS_EVT_INDEX_CNT2:
			event_th = 10;
			break;
		case ADS_EVT_INDEX_CNT3:
			event_th = 11;
			break;		
		case ADS_EVT_DI_INTERRUPT0:
			event_th = 12;
			break;
		case ADS_EVT_DI_INTERRUPT1:
			event_th = 13;
			break;
		case ADS_EVT_DI_INTERRUPT2:
			event_th = 14;
			break;
		case ADS_EVT_DI_INTERRUPT3:
			event_th = 15;
			break;	
		case ADS_EVT_OVERCOMPARE_CNT0:
			event_th = 16;
			break;
		case ADS_EVT_OVERCOMPARE_CNT1:
			event_th = 17;
			break;
		case ADS_EVT_OVERCOMPARE_CNT2:
			event_th = 18;
			break;
		case ADS_EVT_OVERCOMPARE_CNT3:
			event_th = 19;
			break;		
		case ADS_EVT_UNDERCOMPARE_CNT0:
			event_th = 20;
			break;
		case ADS_EVT_UNDERCOMPARE_CNT1:
			event_th = 21;
			break;
		case ADS_EVT_UNDERCOMPARE_CNT2:
			event_th = 22;
			break;
		case ADS_EVT_UNDERCOMPARE_CNT3:
			event_th = 23;
			break;	
		case ADS_EVT_INTERRUPT_TIMER4:
			event_th = 24;
			break;	
		default:
			return InvalidEventType;
		}
		check_event.EventType = adv_process_info_check_special_event(ptr, event_th);		
	}
	
	if (copy_to_user(arg, &check_event, sizeof(PT_CheckEvent))) {
		return MemoryCopyFailed;
	}
	
	return SUCCESS;
}
		  
  
INT32S adv_enable_event(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_EnableEvent *lp_enable_event;

	lp_enable_event = kmalloc(sizeof(PT_EnableEvent), GFP_KERNEL);
	if (!lp_enable_event) {
		return MemoryAllocateFailed;
	}
     
	if (copy_from_user(lp_enable_event, arg, sizeof(PT_EnableEvent))) {
		kfree(lp_enable_event);	
		return MemoryCopyFailed;
	}

	privdata->evt_cnt = lp_enable_event->Count;

	if (lp_enable_event->Enabled) {
		switch(lp_enable_event->EventType)	{
		case ADS_EVT_OVERFLOW_CNT0:
			adv_process_info_enable_event(&privdata->ptr_process_info, 0, lp_enable_event->Count);
			break;
		case ADS_EVT_OVERFLOW_CNT1:
			adv_process_info_enable_event(&privdata->ptr_process_info, 1, lp_enable_event->Count);
			break;
		case ADS_EVT_OVERFLOW_CNT2:
			adv_process_info_enable_event(&privdata->ptr_process_info, 2, lp_enable_event->Count);
			break;
		case ADS_EVT_OVERFLOW_CNT3:
			adv_process_info_enable_event(&privdata->ptr_process_info, 3, lp_enable_event->Count);
			break;		
		case ADS_EVT_UNDERFLOW_CNT0:
			adv_process_info_enable_event(&privdata->ptr_process_info, 4, lp_enable_event->Count);
			break;
		case ADS_EVT_UNDERFLOW_CNT1:
			adv_process_info_enable_event(&privdata->ptr_process_info, 5, lp_enable_event->Count);
			break;
		case ADS_EVT_UNDERFLOW_CNT2:
			adv_process_info_enable_event(&privdata->ptr_process_info, 6, lp_enable_event->Count);
			break;
		case ADS_EVT_UNDERFLOW_CNT3:
			adv_process_info_enable_event(&privdata->ptr_process_info, 7, lp_enable_event->Count);
			break;
		case ADS_EVT_INDEX_CNT0:
			adv_process_info_enable_event(&privdata->ptr_process_info, 8, lp_enable_event->Count);
			break;
		case ADS_EVT_INDEX_CNT1:
			adv_process_info_enable_event(&privdata->ptr_process_info, 9, lp_enable_event->Count);
			break;
		case ADS_EVT_INDEX_CNT2:
			adv_process_info_enable_event(&privdata->ptr_process_info, 10, lp_enable_event->Count);
			break;
		case ADS_EVT_INDEX_CNT3:
			adv_process_info_enable_event(&privdata->ptr_process_info, 11, lp_enable_event->Count);
			break;		
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_enable_event(&privdata->ptr_process_info, 12, lp_enable_event->Count);
			break;
		case ADS_EVT_DI_INTERRUPT1:
			adv_process_info_enable_event(&privdata->ptr_process_info, 13, lp_enable_event->Count);
			break;
		case ADS_EVT_DI_INTERRUPT2:
			adv_process_info_enable_event(&privdata->ptr_process_info, 14, lp_enable_event->Count);
			break;
		case ADS_EVT_DI_INTERRUPT3:
			adv_process_info_enable_event(&privdata->ptr_process_info, 15, lp_enable_event->Count);
			break;	
		case ADS_EVT_OVERCOMPARE_CNT0:
			adv_process_info_enable_event(&privdata->ptr_process_info, 16, lp_enable_event->Count);
			break;
		case ADS_EVT_OVERCOMPARE_CNT1:
			adv_process_info_enable_event(&privdata->ptr_process_info, 17, lp_enable_event->Count);
			break;
		case ADS_EVT_OVERCOMPARE_CNT2:
			adv_process_info_enable_event(&privdata->ptr_process_info, 18, lp_enable_event->Count);
			break;
		case ADS_EVT_OVERCOMPARE_CNT3:
			adv_process_info_enable_event(&privdata->ptr_process_info, 19, lp_enable_event->Count);
			break;		
		case ADS_EVT_UNDERCOMPARE_CNT0:
			adv_process_info_enable_event(&privdata->ptr_process_info, 20, lp_enable_event->Count);
			break;
		case ADS_EVT_UNDERCOMPARE_CNT1:
			adv_process_info_enable_event(&privdata->ptr_process_info, 21, lp_enable_event->Count);
			break;
		case ADS_EVT_UNDERCOMPARE_CNT2:
			adv_process_info_enable_event(&privdata->ptr_process_info, 22, lp_enable_event->Count);
			break;
		case ADS_EVT_UNDERCOMPARE_CNT3:
			adv_process_info_enable_event(&privdata->ptr_process_info, 23, lp_enable_event->Count);
			break;	
		case ADS_EVT_INTERRUPT_TIMER4:
			adv_process_info_enable_event(&privdata->ptr_process_info, 24, lp_enable_event->Count);
			break;
		default:
			break;
		}
	} else {
		switch(lp_enable_event->EventType)	{
		case ADS_EVT_OVERFLOW_CNT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			break;
		case ADS_EVT_OVERFLOW_CNT1:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			break;
		case ADS_EVT_OVERFLOW_CNT2:
			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			break;
		case ADS_EVT_OVERFLOW_CNT3:
			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			break;		
		case ADS_EVT_UNDERFLOW_CNT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 4);
			break;
		case ADS_EVT_UNDERFLOW_CNT1:
			adv_process_info_disable_event(&privdata->ptr_process_info, 5);
			break;
		case ADS_EVT_UNDERFLOW_CNT2:
			adv_process_info_disable_event(&privdata->ptr_process_info, 6);
			break;
		case ADS_EVT_UNDERFLOW_CNT3:
			adv_process_info_disable_event(&privdata->ptr_process_info, 7);
			break;
		case ADS_EVT_INDEX_CNT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 8);
			break;
		case ADS_EVT_INDEX_CNT1:
			adv_process_info_disable_event(&privdata->ptr_process_info, 9);
			break;
		case ADS_EVT_INDEX_CNT2:
			adv_process_info_disable_event(&privdata->ptr_process_info, 10);
			break;
		case ADS_EVT_INDEX_CNT3:
			adv_process_info_disable_event(&privdata->ptr_process_info, 11);
			break;		
		case ADS_EVT_DI_INTERRUPT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 12);
			break;
		case ADS_EVT_DI_INTERRUPT1:
			adv_process_info_disable_event(&privdata->ptr_process_info, 13);
			break;
		case ADS_EVT_DI_INTERRUPT2:
			adv_process_info_disable_event(&privdata->ptr_process_info, 14);
			break;
		case ADS_EVT_DI_INTERRUPT3:
			adv_process_info_disable_event(&privdata->ptr_process_info, 15);
			break;	
		case ADS_EVT_OVERCOMPARE_CNT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 16);
			break;
		case ADS_EVT_OVERCOMPARE_CNT1:
			adv_process_info_disable_event(&privdata->ptr_process_info, 17);
			break;
		case ADS_EVT_OVERCOMPARE_CNT2:
			adv_process_info_disable_event(&privdata->ptr_process_info, 18);
			break;
		case ADS_EVT_OVERCOMPARE_CNT3:
			adv_process_info_disable_event(&privdata->ptr_process_info, 19);
			break;		
		case ADS_EVT_UNDERCOMPARE_CNT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 20);
			break;
		case ADS_EVT_UNDERCOMPARE_CNT1:
			adv_process_info_disable_event(&privdata->ptr_process_info, 21);
			break;
		case ADS_EVT_UNDERCOMPARE_CNT2:
			adv_process_info_disable_event(&privdata->ptr_process_info, 22);
			break;
		case ADS_EVT_UNDERCOMPARE_CNT3:
			adv_process_info_disable_event(&privdata->ptr_process_info, 23);
			break;	
		case ADS_EVT_INTERRUPT_TIMER4:
			adv_process_info_disable_event(&privdata->ptr_process_info, 24);
			break;
		default:
			return InvalidEventType;
		}

	}
	
	kfree(lp_enable_event);	

	return 0;
}



INT32S adv_clear_flag(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	privdata->overrun_flag = 0;

	return SUCCESS;
}


INT32S adv_DioSetEventConfig( adv_device *device, void *arg)
{
	AD_DIO_EVENT_CFG diocfg;
	DWORD  dwIndex;
	INT32U eventID;
	INT32U scanStart;
	INT32U scanCount;
	private_data *privdata = (private_data *) device->private_data;

	if (copy_from_user(&diocfg, arg, sizeof(AD_DIO_EVENT_CFG))) {
		return -1;
	}

	eventID = diocfg.eventID;
	scanStart = diocfg.scanStart;
	scanCount = diocfg.scanCount;
	
	
	if ( scanStart != 0 && scanCount != 1)	{
		return -1;
	}

	dwIndex = eventID;
	if ( dwIndex < 4)	{
		privdata->DIEvtCfg[dwIndex].portStart = scanStart;
		privdata->DIEvtCfg[dwIndex].portCount = scanCount;
		privdata->dioIndex = dwIndex;
	}else{
		return -1;
	}
	
	return 0;	
}

INT32S  adv_DioGetEventData( adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	AD_DIO_EVENT_DATA dioevtdata;
	INT32U  eventID;

	if (copy_from_user(&dioevtdata, arg, sizeof(AD_DIO_EVENT_CFG))) {
		printk("get event data error!\n");
		return -EFAULT;
	}

	eventID = dioevtdata.eventID;
	
	if (eventID > 3 ){
		printk("event id=%x !\n",eventID);
		return -EINVAL;
	}
	
	memcpy(dioevtdata.dataBuf, &(privdata->DIEvtCfg[eventID].evtData),sizeof(INT8U));
	dioevtdata.bufLen = sizeof(INT8U);

	if (copy_to_user(arg, &dioevtdata, sizeof(AD_DIO_EVENT_DATA))) {
		printk("copy to user error!\n");
		return  -EFAULT;
	}

	return 0; 
}

INT32S adv_CntrSetEventConfig(adv_device *device, void *arg)
{
	AD_DIO_EVENT_CFG dioevtcfg;
	private_data *privdata = (private_data *) device->private_data;
	INT32U eventID ;
	INT32U scanStart;
	INT32U scanCount;
	if(copy_from_user(&dioevtcfg, arg, sizeof(AD_DIO_EVENT_CFG))){
		return -EFAULT;
	}
	eventID = dioevtcfg.eventID;
	scanStart = dioevtcfg.scanStart;
	scanCount = dioevtcfg.scanCount;
	
	if ( scanStart >= 4	||scanStart + scanCount > 4)	{
		return -EINVAL;
	}

	if ( eventID <= MAX_EVENT_NUMBER)	{
		privdata->CntrEvtCfg[eventID].portStart = scanStart;
		privdata->CntrEvtCfg[eventID].portCount = scanCount;
		privdata->cntrIndex = eventID;
	}else{
		return -EINVAL;
	}

	return 0;	
}

INT32S adv_CntrGetEventData(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	INT32U  eventID;
	AD_CNTR_EVENT_DATA cntevtdata;

	if (copy_from_user( &cntevtdata, arg, sizeof(AD_CNTR_EVENT_DATA))) {
		return -EFAULT;
	}

	eventID = cntevtdata.eventID;
	
	if (eventID > MAX_EVENT_NUMBER ){
		return -EINVAL;
	}
	
	cntevtdata.bufLen = 4 * sizeof(INT32U);
	memcpy(cntevtdata.dataBuf, privdata->CntrEvtCfg[eventID].evtData,4*sizeof(INT32U));
	if (copy_to_user(arg, &cntevtdata, sizeof(AD_CNTR_EVENT_DATA))) {
		return  -EFAULT;
	}
	
	return 0; 
}

INT32S adv_CntrGetParam(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	CTL_CNTR_PARAMS CntrParam;
	

	CntrParam.ulIntSrc = privdata->ulIntSourceData;
	memcpy(CntrParam.ulCntrMode, privdata->ulDevPara, sizeof(INT32U) * 4 );
	CntrParam.ulClkCtl = privdata->ulClkCtl;
	CntrParam.usDoMode = privdata->usDoMode;
	CntrParam.usIndexReset = privdata->usIndexReset;

	if(copy_to_user(arg, &CntrParam, sizeof(CTL_CNTR_PARAMS))){
		return -EFAULT;
	}
	
	return 0;
}

INT32S adv_CntrSetParam(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) device->private_data;
	CTL_CNTR_PARAMS CntrParam;
	int i;
	INT32U ulAddr;
	if (copy_from_user(&CntrParam, arg, sizeof(CTL_CNTR_PARAMS))) {
		return -EFAULT;
	}

	// counter mode
	for (i = 0; i < 4; i++)	{
		if (CntrParam.usDirtyFlag[i])		{
			privdata->ulDevPara[i] = CntrParam.ulCntrMode[i];
			ulAddr = i * 4;
			advOutpdw(privdata, ulAddr, privdata->ulDevPara[i]);
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
	
	if (CntrParam.usDirtyFlag[5])	{
		privdata->ulClkCtl = CntrParam.ulClkCtl;
		advOutpdw(privdata, 0x24, privdata->ulClkCtl);
	}

	if (CntrParam.usDirtyFlag[6]) 	{
		privdata->usDoMode = CntrParam.usDoMode;
		advOutpw(privdata, 0x32, privdata->usDoMode);
	}

	if ((CntrParam.usDirtyFlag[7]) && (privdata->SubsystemID == 0xA200)) 	{
		privdata->usIndexReset = CntrParam.usIndexReset;
		advOutpw(privdata, 0x2c, privdata->usIndexReset<<4);
	}
	return 0;
}
