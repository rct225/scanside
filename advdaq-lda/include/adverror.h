#ifndef _ADV_DAQ_ERROR_H
#define _ADV_DAQ_ERROR_H

/**
 * 
 * adverror.h   header file for error code and error message
 *  
 * Copyright (C) 2006 Advantech Co., Ltd
 *
 * Change Log:
 *       2006/07/14  Initial Version
 *
 */

#define SUCCESS                  0
#define DrvErrorCode             1
/* #define KeErrorCode              100 */
/* #define DnetErrorCode            200 */
/* #define GeniDrvErrorCode         300 */
/* #define OPCErrorCode             1000 */

#define MemoryAllocateFailed     	(DrvErrorCode + 0)
#define ConfigDataLost           	(DrvErrorCode + 1)
#define InvalidDeviceHandle      	(DrvErrorCode + 2)
#define AIConversionFailed       	(DrvErrorCode + 3)
#define AIScaleFailed            	(DrvErrorCode + 4)
#define SectionNotSupported      	(DrvErrorCode + 5)
#define InvalidChannel           	(DrvErrorCode + 6)
#define InvalidGain              	(DrvErrorCode + 7)
#define DataNotReady             	(DrvErrorCode + 8)
#define InvalidInputParam        	(DrvErrorCode + 9)
#define NoExpansionBoardConfig   	(DrvErrorCode + 10)
#define InvalidAnalogOutValue    	(DrvErrorCode + 11)
#define ConfigIoPortFailed       	(DrvErrorCode + 12)
#define CommOpenFailed           	(DrvErrorCode + 13)
#define CommTransmitFailed       	(DrvErrorCode + 14)
#define CommReadFailed           	(DrvErrorCode + 15)
#define CommReceiveFailed        	(DrvErrorCode + 16)
#define CommConfigFailed         	(DrvErrorCode + 17)
#define CommChecksumError        	(DrvErrorCode + 18)
#define InitError                	(DrvErrorCode + 19)
#define DMABufAllocFailed        	(DrvErrorCode + 20)
#define IllegalSpeed             	(DrvErrorCode + 21)
#define ChanConflict             	(DrvErrorCode + 22)
#define BoardIDNotSupported      	(DrvErrorCode + 23)
#define FreqMeasurementFailed    	(DrvErrorCode + 24)
#define CreateFileFailed         	(DrvErrorCode + 25)
#define FunctionNotSupported     	(DrvErrorCode + 26)
#define LoadLibraryFailed        	(DrvErrorCode + 27)
#define GetProcAddressFailed     	(DrvErrorCode + 28)
#define InvalidDriverHandle      	(DrvErrorCode + 29)
#define InvalidModuleType        	(DrvErrorCode + 30)
#define InvalidInputRange        	(DrvErrorCode + 31)
#define InvalidWindowsHandle     	(DrvErrorCode + 32)
#define InvalidCountNumber       	(DrvErrorCode + 33)
#define InvalidInterruptCount    	(DrvErrorCode + 34)
#define InvalidEventCount        	(DrvErrorCode + 35)
#define OpenEventFailed          	(DrvErrorCode + 36)
#define InterruptProcessFailed   	(DrvErrorCode + 37)
#define InvalidDOSetting         	(DrvErrorCode + 38)
#define InvalidEventType         	(DrvErrorCode + 39)
#define EventTimeOut             	(DrvErrorCode + 40)
#define InvalidDmaChannel        	(DrvErrorCode + 41)
#define IntDmaChannelBusy        	(DrvErrorCode + 42)
#define CheckRunTimeClassFailed  	(DrvErrorCode + 43)
#define CreateDllLibFailed       	(DrvErrorCode + 44)
#define ExceptionError           	(DrvErrorCode + 45)
#define RemoveDeviceFailed       	(DrvErrorCode + 46)
#define BuildDeviceListFailed    	(DrvErrorCode + 47)
#define NoIOFunctionSupport      	(DrvErrorCode + 48)

#define ResourceConflict         	(DrvErrorCode + 49)
#define InvalidClockSource	 	(DrvErrorCode + 50)
#define InvalidPacerRate	 	(DrvErrorCode + 51)
#define InvalidTriggerMode    	 	(DrvErrorCode + 52)
#define InvalidTriggerEdge     	 	(DrvErrorCode + 53)
#define InvalidTriggerSource   	 	(DrvErrorCode + 54)
#define InvalidTriggerVoltage    	(DrvErrorCode + 55)
#define InvalidCyclicMode        	(DrvErrorCode + 56)
#define InvalidDelayCount        	(DrvErrorCode + 57)
#define InvalidBuffer            	(DrvErrorCode + 58)
#define OverloadedPCIBus         	(DrvErrorCode + 59)
#define OverloadedInterruptRequest 	(DrvErrorCode + 60)
#define ParamNameNotSupported      	(DrvErrorCode + 61)
#define CheckEventFailed           	(DrvErrorCode + 62)
#define InvalidPort                	(DrvErrorCode + 63)
#define DaShiftBusy                	(DrvErrorCode + 64)
#define ThermoCoupleDisconnect     	(DrvErrorCode + 65)
#define MemoryCopyFailed              	(DrvErrorCode + 66)
#define InvalidCommandType           	(DrvErrorCode + 67)  
#define DmaNotStart                	(DrvErrorCode + 68)
#define AcquireDataTimeOut              (DrvErrorCode + 69)

#define CalibrationFailed               (DrvErrorCode + 70)

#define SystemSignalInterrupt           (DrvErrorCode + 99)
#define SystemError                     (DrvErrorCode + 100)

#endif
