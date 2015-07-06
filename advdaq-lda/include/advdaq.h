/**
 * Author : zhiyong.xie
 * Date   : 11/02/2007
 */
#ifndef __LIB_ADV_DAQ_H__
#define __LIB_ADV_DAQ_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/mman.h>
#include <math.h>

#include "./advdevice.h"

#ifdef __cplusplus
extern "C"{
#endif

/* 1. Device function group */
INT32S DRV_DeviceOpen(char *filename, PTR_T *handle);
INT32S DRV_DeviceClose(PTR_T *handle);
INT32S DRV_DeviceGetProperty(PTR_T fd, INT16U nID, void *pBuffer, INT32U *pLength);
INT32S  DRV_DeviceSetProperty(PTR_T fd, INT16U nID, void *pBuffer, INT32U dwLength);
/* 2. A/D function group */
INT32S DRV_AIConfig(PTR_T fd, PT_AIConfig * lpAIConfig);
INT32S DRV_AIBinaryIn(PTR_T fd, PT_AIBinaryIn * lpAIBinaryIn);
INT32S DRV_AIScale(PTR_T fd, PT_AIScale * lpAIScale);
INT32S DRV_AIVoltageIn(PTR_T fd, PT_AIVoltageIn * lpAIVoltageIn);
INT32S DRV_AICurrentIn(PTR_T fd, PT_AICurrentIn * lpAICurrentIn);
INT32S DRV_MAIConfig(PTR_T fd, PT_MAIConfig * lpMAIConfig);
INT32S DRV_MAIBinaryIn(PTR_T fd, PT_MAIBinaryIn * lpMAIBinaryIn);
INT32S DRV_MAIVoltageIn(PTR_T fd, PT_MAIVoltageIn * lpMAIVoltageIn);
INT32S DRV_MAICurrentIn(PTR_T fd, PT_MAICurrentIn * lpMAICurrentIn);
INT32S DRV_FAIIntStart(PTR_T fd, PT_FAIIntStart * lpFAIIntStart);
INT32S DRV_FAIIntScanStart(PTR_T fd, PT_FAIIntScanStart * lpFAIIntScanStart);
INT32S DRV_FAIDmaExStart(PTR_T fd, PT_FAIDmaExStart * lpFAIDmaExStart);
INT32S DRV_FAIDmaStart(PTR_T fd, PT_FAIDmaStart * lpFAIDmaStart);
INT32S DRV_FAIDmaScanStart(PTR_T fd, PT_FAIDmaScanStart * lpFAIDmaScanStart);
INT32S DRV_GetFIFOSize(PTR_T fd, INT32U *size);
INT32S DRV_FAICheck(PTR_T fd,PT_FAICheck * lpFAICheck);
INT32S DRV_FAITerminate(PTR_T fd);
INT32S DRV_FAITransfer(PTR_T fd, PT_FAITransfer *lpFAITransfer);
	
/* 3. D/A function group */
INT32S DRV_AOConfig(PTR_T fd, PT_AOConfig * lpAOConfig);
INT32S DRV_AOBinaryOut(PTR_T fd, PT_AOBinaryOut * lpAOBinaryOut);
INT32S DRV_AOScale(PTR_T fd, PT_AOScale * lpAOScale);
INT32S DRV_AOVoltageOut(PTR_T fd, PT_AOVoltageOut * lpAOVoltageOut);
INT32S DRV_AOCurrentOut(PTR_T fd, PT_AOCurrentOut * lpAOCurrentOut);
INT32S DRV_EnableSyncAO(PTR_T fd,INT16U enable);
INT32S DRV_WriteSyncAO(PTR_T fd);
INT32S DRV_FAODmaExStart(PTR_T fd, PT_FAODmaExStart * lpFAODmaExStart);
INT32S DRV_FAOLoad(PTR_T fd, PT_FAOLoad * lpFAOLoad);
INT32S DRV_FAOCheck(PTR_T fd, PT_FAOCheck * lpFAOCheck);
INT32S DRV_FAOScale(PTR_T fd, PT_FAOScale * lpFAOScale);
INT32S DRV_FAOTerminate(PTR_T fd);
	
/* 4. DI/O function group */
INT32S DRV_DioReadDIPorts(PTR_T fd, INT32U portstart, INT32U portcount, INT8U *buffer);
INT32S DRV_DioWriteDOPorts(PTR_T fd, INT32U portstart, INT32U portcount, INT8U *buffer);
INT32S DRV_DioReadDOPorts(PTR_T fd, INT32U portstart, INT32U portcount, INT8U *buffer);
INT32S DRV_DioEnableEventAndSpecifyDiPorts(PTR_T fd, INT32U dwEventID, INT32U dwScanStart, INT32U dwScanCount);
INT32S DRV_DioDisableEvent(PTR_T fd, INT32U dwEventID);
INT32S DRV_DioGetLatestEventDiPortsState(PTR_T fd, INT32S dwEventID, BYTE *pBuffer, INT32U dwLength);
INT32S DRV_FDIStart(PTR_T fd, INT16U wCyclic, INT32U dwCount, VOID *pBuf);
INT32S DRV_FDICheck(PTR_T fd, INT32U *pdwStatus, INT32U *pdwRetrieved);
INT32S DRV_FDIStop(PTR_T fd);
INT32S DRV_FDITransfer(PTR_T fd, PT_FDITransfer * lpFDITransfer);
INT32S DRV_FDOStart(PTR_T fd, INT16U wCyclic, ULONG dwCount, VOID *pBuf);
INT32S DRV_FDOCheck(PTR_T fd, ULONG *pdwStatus, ULONG *pdwRetrieved);
INT32S DRV_FDOStop(PTR_T fd);

/*Port I/O*/
INT32S DRV_ReadPortByte(PTR_T fd, PT_ReadPortByte *lpReadPortByte );
INT32S DRV_WritePortByte(PTR_T fd, PT_WritePortByte *lpWritePortByte );
INT32S DRV_ReadPortWord(PTR_T fd, PT_ReadPortWord *lpReadPortWord);
INT32S DRV_WritePortWord(PTR_T fd, PT_WritePortWord *lpWritePortWord );
INT32S DRV_ReadPortDword(PTR_T fd, PT_ReadPortDword *lpReadPortDword);
INT32S DRV_WritePortDword(PTR_T fd, PT_WritePortDword *lpWritePortDword );
	
/* 5. Counter function group */
INT32S DRV_CounterEventStart(PTR_T fd, PT_CounterEventStart * lpCounterEventStart);
INT32S DRV_CounterEventRead(PTR_T fd, PT_CounterEventRead * lpCounterEventRead);
INT32S DRV_CounterConfig(PTR_T fd, PT_CounterConfig * lpCounterConfig);
INT32S DRV_QCounterConfig(PTR_T fd, PT_QCounterConfig * lpQCounterConfig);
INT32S DRV_TimerCountSetting(PTR_T fd, PT_TimerCountSetting * lpTimerCountSetting);
INT32S DRV_CounterFreqStart(PTR_T fd, PT_CounterFreqStart * lpCounterFreqStart);
INT32S DRV_CounterFreqRead(PTR_T fd, PT_CounterFreqRead * lpCounterFreqRead);
INT32S DRV_PWMStartRead(PTR_T fd, PT_PWMStartRead * lpPWMStartRead);
INT32S DRV_CounterPulseStart(PTR_T fd, PT_CounterPulseStart * lpCounterPulseStart);
INT32S DRV_CounterPWMSetting(PTR_T fd, PT_CounterPWMSetting * lpCounterPWMSetting);
INT32S DRV_CounterPWMEnable(PTR_T fd, INT16U Port);
INT32S DRV_FreqOutStart(PTR_T fd, PT_FreqOutStart *lpFreqOutStart);
INT32S DRV_FreqOutReset(PTR_T fd, INT32U Channel);
INT32S DRV_CounterReset(PTR_T fd, INT32U counter);
INT32S DRV_DICounterReset(PTR_T fd, INT32U counter);
INT32S DRV_CntrEnableEventAndSpecifyEventCounter(PTR_T fd, INT32U dwEventID, INT32U dwScanStart, INT32U dwScanCount);
INT32S DRV_CntrDisableEvent(PTR_T fd, INT32S dwEventID);
INT32S DRV_CntrGetLatestEventCounterValue(PTR_T fd, INT32S dwEventID, BYTE *pBuffer, INT32U dwLength);
INT32S DRV_WatchdogStart(PTR_T fd, PT_WatchdogStart *lpWatchdogStart);
INT32S DRV_WatchdogFeed(PTR_T fd);
INT32S DRV_WatchdogStop(PTR_T fd);
	
/* 6. Event mechanism function group */
INT32S DRV_EnableEvent(PTR_T fd, PT_EnableEvent * lpEnableEvent);
INT32S DRV_CheckEvent(PTR_T fd, PT_CheckEvent * lpCheckEvent);
INT32S DRV_ClearFlag(PTR_T fd, INT32U ID);

/* 7. Other function  */
INT32S DRV_GetErrorMessage(INT32S ErrorCode, char *ErrorMsg);
INT32S DRV_TCMuxRead(PTR_T fd, PT_TCMuxRead *lpTCMuxRead);
	
#ifdef __cplusplus
}
#endif

#endif
