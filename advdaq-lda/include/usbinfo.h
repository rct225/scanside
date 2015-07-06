/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech USB Device Driver for Linux

File Name:
	UsbInfo.h
Abstract:
	Header file of USB device driver.
Version history
	03/20/2006		Create by zhiyong.xie

************************************************************************/	



#ifndef _USB_INFO_H
#define _USB_INFO_H

#define ADV_USB_CTRRW_MUTEX_NAME	"ADV_USB_CTRRW_MUTEX"
#define ADV_AI_MUTEX_NAME           "ADV_AI_MUTEX"
#define ADV_DI_MUTEX_NAME           "ADV_DI_MUTEX"
#define ADV_MAI_MUTEX_NAME          "ADV_MAI_MUTEX"

#define MAX_AI_CHANNELS              16

#ifdef _USB_MC_PACK_1

#pragma pack(push, 1)

#endif
#include "advdevice.h"

///Interrupt Event///---------------------------------
typedef struct _USB_EnableEvent
{
	INT16U      EventType;
	INT16U      Enabled;
	INT16U      EventTrigger;  ///1 :rising 0 falling
} USB_EnableEvent, * PUSB_EnableEvent;
//
// DI/O ---------------------------------------------------------------
//

// DO BYTE
typedef struct _USB_DO
{
	INT16U	usChannel;
	INT16U	usSize;
	INT32U	ulData;
} USB_DO, *PUSB_DO;

// DO BYTE Readback
typedef struct _USB_DO_READ_TX
{
	INT16U	usChannel;
	INT16U	usSize;
} USB_DO_READ_TX, *PUSB_DO_READ_TX;

typedef struct _USB_DO_READ_RX
{
	// Please put "ulRetCode"(error return code) in the top, because firmware 
	// will return "system last error" in 1st INT32U.
	//	INT32U	ulRetCode;					
	INT32U ulRetCode;
	INT16U	usSize;
	//	INT32U	ulData;
	INT32U ulData;
} USB_DO_READ_RX, *PUSB_DO_READ_RX;

// DI BYTE
typedef struct _USB_DI_TX
{
	INT16U	usChannel;
	INT16U	usSize;
} USB_DI_TX, *PUSB_DI_TX;

typedef struct _USB_DI_RX
{
	// Please put "ulRetCode"(error return code) in the top, because firmware 
	// will return "system last error" in 1st INT32U.
	//	INT32U	ulRetCode;
	INT32U ulRetCode;
	INT16U	usSize;
	//	INT32U	ulData;
	INT32U ulData;
} USB_DI_RX, *PUSB_DI_RX;


//
// AI ---------------------------------------------------------------
//

// AI Set Gain
typedef struct _USB_AI_SET_GAIN
{
	INT16U	usChannel;
	INT16U	usGain;
#ifdef AI_CHANNEL_CONFIG
	INT16U  usChanConfig;  //wang.long add channel config
#endif
} USB_AI_SETGAIN, *PUSB_AI_SETGAIN;


// AI BinaryIn
typedef struct _USB_AI_BINARYIN_TX
{
	INT16U	usChannel;
	INT16U	usTrigMode;			//	0-normal (software) 1-external 
	
} USB_AI_BINARYIN_TX, *PUSB_AI_BINARYIN_TX;

typedef struct _USB_AI_BINARYIN_RX
{
	// Please put "ulRetCode"(error return code) in the top, because firmware 
	// will return "system last error" in 1st INT32U.
	INT32U	ulRetCode;				

	INT32U	ulData;				// we make Data to "INT32U" for 32bit AI in the feature
	
} USB_AI_BINARYIN_RX, *PUSB_AI_BINARYIN_RX;


//
// MAI ---------------------------------------------------------------
// 

// MAI Set Gain
typedef struct _USB_MAI_SET_GAIN
{
	INT16U	usStartChan;
	INT16U	usNumChan;
#ifdef AI_CHANNEL_CONFIG
	INT16U  usChanConfig;  //wang.long add channel config
	INT16U  usStopChan;    //wang.long add stop channel
#endif
	INT16U	usGainArray[MAX_AI_CHANNELS];
	
} USB_MAI_SETGAIN, *PUSB_MAI_SETGAIN;

// MAI BinaryIn
typedef struct _USB_MAI_BINARYIN_TX
{
	INT16U	usStartChan;
	INT16U	usNumChan;
	INT16U	usTrigMode;
#ifdef AI_CHANNEL_CONFIG
	INT16U  usStopChan;
#endif
} USB_MAI_BINARYIN_TX, *PUSB_MAI_BINARYIN_TX;

typedef struct _USB_MAI_BINARYIN_RX
{
	// Please put "ulRetCode"(error return code) in the top, because firmware 
	// will return "system last error" in 1st INT32U.
	INT32U	ulRetCode;				

	INT32U	ulData[MAX_AI_CHANNELS];		// we make Data to "INT32U" for 32bit AI in the feature
	
} USB_MAI_BINARYIN_RX, *PUSB_MAI_BINARYIN_RX;


//
// FAI ---------------------------------------------------------------
// 

// FAI
typedef struct _USB_FAI_INTSTART
{
	INT16U	usChannel;				// channel
	INT16U	usGain;					// gain
	INT16U	usExtTrig;				// trigger source (1:Ext, 0:Internal)
	INT16U	usDivisor;				// Pacer divisor
	INT32U	ulConvNum;				// Conversion count
	INT16U	usCyclic;				// Cyclic mode (1:Cyclic, 0:Non-Cyclic)
	INT32U	ulSampleRate;			// sample rate(Hz)
#ifdef AI_CHANNEL_CONFIG
	INT16U  usChanConfig;           //wang.long add channel config
#endif
} USB_FAI_INTSTART, *PUSB_FAI_INTSTART;

// FAI Scan
typedef struct _USB_FAI_INTSCAN
{
	INT16U	usStartChan;
	INT16U	usNumChan;				
	INT16U	usExtTrig;						// trigger source (1:Ext, 0:Internal)
	INT16U	usDivisor;						// Pacer divisor
	INT32U	ulConvNum;						// Conversion count
	INT16U	usCyclic;						// Cyclic mode (1:Cyclic, 0:Non-Cyclic)
	INT32U	ulSampleRate;					// sample rate(Hz)
#ifdef AI_CHANNEL_CONFIG
	INT16U  usChanConfig;                    //wang.long add channel config
	INT16U  usStopChan;             //wang.long add stop channel
#endif
	INT16U	usGainArray[MAX_AI_CHANNELS];	// gain array
	
} USB_FAI_INTSCAN, *PUSB_FAI_INTSCAN;

typedef struct _USB_FAI_SETPACER
{
	INT16U	usTimerClock;			// Unit: MHz
	INT16U	usDivisor;				// Pacer divisor
	INT32U	ulSampleRate;			// sample rate(Hz)

} USB_FAI_SETPACER, *PUSB_FAI_SETPACER;


//
// AO ---------------------------------------------------------------
//
typedef struct _USB_AO_CONFIG
{
	INT16U	usChan;
	INT16U	usRefSrc;					// Internal(0), External(1)
	INT16U	usRangeCode;				// AO range code
	
} USB_AO_CONFIG, *PUSB_AO_CONFIG;

typedef struct _USB_AO_BINARYOUT
{
	INT16U	usChan;
	INT16U	usBinData;					// AO data
	
} USB_AO_BINARYOUT, *PUSB_AO_BINARYOUT;


//
// COUNTER ---------------------------------------------------------------
//

typedef struct _USB_CNT_EVENTSTART				// Counter EventStart
{
	INT16U	usCounter;
} USB_CNT_EVENTSTART, *PUSB_CNT_EVENTSTART;

typedef struct _USB_CNT_EVENTREAD_RX			// Counter EventRead
{
	// Please put "ulRetCode"(error return code) in the top, 
	// because firmware will return "system last error" in 
	// 1st INT32U.
	INT32U	ulRetCode;		

	INT32U	ulCount;
	INT16U	usOverflow;

} USB_CNT_EVENTREAD_RX, *PUSB_CNT_EVENTREAD_RX;

typedef struct _USB_CNT_RESET					// Counter Reset
{
	INT16U	usCounter;
} USB_CNT_RESET, *PUSB_CNT_RESET;

typedef struct _USB_CNT_PULSESTART					// Counter Pulse Start
{
	INT16U	usCounter;
#ifdef MAX_COUNTER_PARA
	INT32U  Context[MAX_COUNTER_PARA];
#endif
} USB_CNT_PULSESTART, *PUSB_CNT_PULSESTART;

typedef struct _USB_CNT_FREQSTART                 	// Counter FreqIn Start
{
	INT16U usCounter;
#ifdef MAX_COUNTER_PARA
	INT32U  Context[MAX_COUNTER_PARA];
#endif
}USB_CNT_FREQSTART,*PUSB_CNT_FREQSTART;

/*modify by zhenyu .....begin*/
typedef struct _USB_CNT_EVENTREAD_TX
{
	USHORT	usCounter;
} USB_CNT_EVENTREAD_TX, *PUSB_CNT_EVENTREAD_TX;

typedef struct _USB_CNT_FREQOUTSTART                 	
{
	USHORT usCounter;
#ifdef MAX_COUNTER_PARA
	ULONG  Context[MAX_COUNTER_PARA];
#endif
}USB_CNT_FREQOUTSTART,*PUSB_CNT_FREQOUTSTART;
/*modify by zhenyu .....end*/

typedef struct _USB_CNT_FREQ_READ_Tx         // Counter FreqIn Read
{
	INT16U usCounter;
}USB_CNT_FREQ_READ_Tx,*PUSB_CNT_FREQ_READ_Tx;

typedef struct _USB_CNT_FREQ_READ_Rx            // Counter FreqIn Read
{
	INT32U  RetCode;
#ifdef MAX_COUNTER_PARA
	INT32U  Context[MAX_COUNTER_PARA];
#endif
}USB_CNT_FREQ_READ_Rx,*PUSB_CNT_FREQ_READ_Rx;

//
// CALIBRATION ---------------------------------------------------------------
//
//
// AO Calibration : Read EEP
//
typedef struct _USB_CAO_READEEP_TX
{
	INT16U	usChan;
	INT16U	usRangeCode;			// AO range code
	INT16U	usOffsetorGain;			// 0: gain ,  1: offset 
	INT16U	usUserorDefault;		// 1: user setting , 2: default setting

} USB_CAO_READEEP_TX, *PUSB_CAO_READEEP_TX;

typedef struct _USB_CAO_READEEP_RX
{
	// Please put "ulRetCode"(error return code) in the top, because firmware 
	// will return "system last error" in 1st INT32U.
	INT32U	ulRetCode;	

	INT16U	usData;					// AO calibration data (Read form EEP)

} USB_CAO_READEEP_RX, *PUSB_CAO_READEEP_RX;


// AO Calibration : write EEP
typedef struct _USB_CAO_WRITEEEP
{
	INT16U	usChan;
	INT16U	usRangeCode;			// AO range code
	INT16U	usOffsetorGain;			// 0: gain ,  1: offset 
	INT16U	usData;					// AO trim data (Write to EEP)
	INT16U	usUserorDefault;		// 1: user setting , 2: default setting

} USB_CAO_WRITEEEP, *PUSB_CAO_WRITEEEP;

// AO Calibration : set trim. data
typedef struct _USB_CAO_SETTRIM
{
	INT16U	usChan;
	INT16U	usOffsetorGain;			// 0: gain ,  1: offset 
	INT16U	usData;					// AO trim. data (Write to DAC)

} USB_CAO_WRITETRIM, *PUSB_CAO_WRITETRIM;


//
// AI Calibration : Read EEP
//
typedef struct _USB_CAI_READEEP_TX
{
	INT16U	usType;			// 0: ADC_Gain ,  1: ADC_Offset  2:PGA_Offset
	INT16U	usUserorDefault;		// 1: user setting , 2: default setting

} USB_CAI_READEEP_TX, *PUSB_CAI_READEEP_TX;

typedef struct _USB_CAI_READEEP_RX
{
	// Please put "ulRetCode"(error return code) in the top, because firmware 
	// will return "system last error" in 1st INT32U.
	INT32U	ulRetCode;	
	INT16U	usData;					// AO calibration data (Read form EEP)

} USB_CAI_READEEP_RX, *PUSB_CAI_READEEP_RX;


// AI Calibration : write EEP
typedef struct _USB_CAI_WRITEEEP
{
	INT16U	usType;			// 0: ADC_Gain ,  1: ADC_Offset  2:PGA_Offset
	INT16U	usData;			// AI trim data (Write to EEP)
	INT16U	usUserorDefault;		// 1: user setting , 2: default setting

} USB_CAI_WRITEEEP, *PUSB_CAI_WRITEEEP;

// AI Calibration : set trim. data
typedef struct _USB_CAI_SETTRIM
{
	INT16U	usType;			// 0: ADC_Gain ,  1: ADC_Offset  2:PGA_Offset
	INT16U	usData;			// AI trim. data (Write to DAC PGA)

} USB_CAI_WRITETRIM, *PUSB_CAI_WRITETRIM;


typedef struct _GET_USB_HW_INFO_STRUCT
{
	INT8U LittleEndian;         //0: BigEndian, 1: LittleEndian
	INT8U GeneralDL;            //0: General FW download forbidden
	//1: General FW download permitted
}GET_USB_HW_INFO_STRUCT, *PGET_USB_HW_INFO_STRUCT;

typedef struct _USB_CNT_PWMSETTING
{
	INT16U usCounter;
#ifdef MAX_COUNTER_PARA
	INT32U Context[MAX_COUNTER_PARA];
#endif
}USB_CNT_PWMSETTING,*PUSB_CNT_PWMSETTING;


typedef struct _USB_CNT_PWMENABLE
{
	INT16U usCounter;
#ifdef MAX_COUNTER_PARA
	INT32U Context[MAX_COUNTER_PARA];
#endif
}USB_CNT_PWMENABLE,*PUSB_CNT_PWMENABLE;


/* #ifdef _USB_MC_PACK_1 */

/* #pragma pack(pop) */

/* #endif */


#endif
