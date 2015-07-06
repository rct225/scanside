#ifndef _ADV_DAQ_PROPERTY_H
#define _ADV_DAQ_PROPERTY_H

/**
 * 
 * advproperty.h   header file for property code
 *  
 * Copyright (C) 2006 Advantech Co., Ltd
 *
 * Change Log:
 *       2006/07/13  Initial Version
 *
 */

/* base code */
#define DrvPropertyCode      1

#define CFG_CardID           (DrvPropertyCode + 0)
#define CFG_BoardID          (DrvPropertyCode + 1)
#define CFG_SwitchID         (DrvPropertyCode + 2)
#define CFG_BaseAddress      (DrvPropertyCode + 3)
#define CFG_Interrupt        (DrvPropertyCode + 4)
#define CFG_BusNumber        (DrvPropertyCode + 5)
#define CFG_SlotNumber       (DrvPropertyCode + 6)

#define CFG_CascadeMode      (DrvPropertyCode + 7)

/* AI General ID */
#define CFG_AiChanConfig     (DrvPropertyCode + 8) /* Get/Set single/different end */
#define CFG_AiPacerRate      (DrvPropertyCode + 9) /* Get/Set AI Pacer rate */
#define CFG_AiFifoSize       (DrvPropertyCode + 10) /* Get Ai Fifo Size */
#define CFG_BURNTEST         (DrvPropertyCode + 11)

/* AO General ID */
#define  AO_RANGE_SETTING    (DrvPropertyCode + 12)
#define  CFG_AoPacerRate     (DrvPropertyCode + 13) /* Get/Set AO Pacer rate */
#define  CFG_AoFifoSize      (DrvPropertyCode + 14) /* Get AO Fifo Size */
#define  CFG_AoRangeList     (DrvPropertyCode + 15) /* Get all supporting AO range list for the specified card */
#define  CFG_AoChanRange     (DrvPropertyCode + 16) /* Get/Set AO range for the specified channel */
#define  CFG_GainList        (DrvPropertyCode + 17) /* Get Gain List */
#define CFG_GainNumber       (DrvPropertyCode + 18) /* the number of gain */
#define FEA_AiChannelCount   (DrvPropertyCode + 19) /* the AI channel number */
#define FEA_AoChannelCount   (DrvPropertyCode + 20) /* the AO channel number */
#define FEA_AoResolution   (DrvPropertyCode + 21) /* the AO Resolution */
#define FEA_CntrChannelCount   (DrvPropertyCode + 22) /* the Counter Channel number */
#define  AO_RANGE_SETTINGL_NAME		   "AO_RANGE_SETTING"


/* DIO Configuration ID */
#define  CFG_DioDirection     (DrvPropertyCode + 17) /* each bit for one Auxiliary DIO channel for PCI-1755. */
#define  CFG_DioFdioDirection (DrvPropertyCode + 18) /* 32DI(0)/32DO(1)/ 16 DIO(2) / 8DIO(3) for PCI-1755 */
#define  CFG_DioTerminator    (DrvPropertyCode + 19) /* Bit 0: DI terminator (DI_TERM),ON (0) or OFF(1) for PCI-1755
						     * Bit 1: DO terminator (DI_TERM) ON (0) or OFF(1) for PCI-1755 */
#define  CFG_DiOperationMode  (DrvPropertyCode + 20) /* normal mode
						     * (0) 8255 handshaking
						     * (1) Burst handshaking
						     * (2) for PCI-1755 */
#define  CFG_DioPortDirection (DrvPropertyCode + 21) /* for Port direction setting
						     *    0: IN
						     *    1: OUT
						     *    2: 8255 Mode 0 (Low IN, High OUT)
						     *    3: 8255 Mode 0 (Low OUT, High IN) */
#define CFG_DioPortType       (DrvPropertyCode + 22) /* Get Port type. Find the used value from Variable Group.
						     * Data type: BYTE Array.
						     * Each byte indicates a port type, from port 0 to port n. */

#define CFG_DioChannelDirection  (DrvPropertyCode + 23) /* Get/Set DIO Channel Direction ( IN / OUT ).
							* Find the used value from Variable Group.
							* Data type: DWORD Array.
							* Each element indicates a port setting. */


/* DI General and others ID */
#define CFG_DiStartMethod     (DrvPropertyCode + 24) /* Software(1), External trigger(2), Pattern match(3) */
#define CFG_DiStopMethod      (DrvPropertyCode + 25) /* Software(1), External trigger(2), Pattern match(3) */
#define CFG_DiPacerSource     (DrvPropertyCode + 26) /* 30MHz(1), 15Mhz(2), 10MHz(3), Counter 0 OUT (4), External (5). */
#define CFG_DiControlSignals  (DrvPropertyCode + 27) /* Bit 0: External DI start signal control (STRRF)
						     * 0 rising edge, 1 falling edge.
						     * 
						     * Bit 1: External DI stop signal control (STPRF)
						     * 0 rising edge, 1 falling edge.
						     * 
						     * Bit 2: DI request signal control (REQRF)
						     * 0 rising edge, 1 falling edge.
						     * 
						     * Bit 3: DI acknowledge signal control (ACKRF)
						     * 0 rising edge, 1 falling edge.
						     * 
						     * Bit 4: DI sampling clock signal control (CLKRF)
						     * 0 rising edge, 1 falling edge */
#define CFG_DiPatternMatchValue (DrvPropertyCode + 28)


/* Fast DO functions paramaters */
#define CFG_DoOperationMode   (DrvPropertyCode + 29) /* Normal (0), Handahaking (1), Burst handshaking(2) */
#define CFG_DoStartMethod     (DrvPropertyCode + 30) /* Software(1), External signal(2) */
#define CFG_DoStopMethod      (DrvPropertyCode + 31) /* Software(1), External signal(2) */
#define CFG_DoPacerSource     (DrvPropertyCode + 32) /* 30, 15, 10 Mhz, Counter1, External signal (value range 1-5) */
#define CFG_DoControlSignals  (DrvPropertyCode + 33) /* Bit 0: External DO start signal control (STRRF)
						     * 0 rising edge, 1 falling edge.
						     * 
						     * Bit 1: External DO stop signal control (STPRF)
						     * 0 rising edge, 1 falling edge.
						     * 
						     * Bit 2: DO request signal control (REQRF)
						     * 0 rising edge, 1 falling edge.
						     * 
						     * Bit 3: DO acknowledge signal control (ACKRF)
						     * 0 rising edge, 1 falling edge.
						     * 
						     * Bit 4: DO sampling clock signal control (CLKRF)
						     * 0 rising edge, 1 falling edge */
#define CFG_DoPresetValue     (DrvPropertyCode + 34) /* DO status before transfering. */

/* for USB device add by Zhiyong */
#define CFG_DoDefaultStatus           CFG_DoPresetValue


/* Get DO data width. The optimized data width when writing. */
/* Data type: LONG.  */
/* See the following for all available setting. */
#define CFG_DoDataWidth       (DrvPropertyCode + 35)	

/* Get DO Channel Count. Max available DO channel count on the card. */
/* Data type: LONG.  */
#define CFG_DoChannelCount    (DrvPropertyCode + 36)	   


/* Get Do Port Count. Max available DO Port count on the card.  */
/* Data type: LONG. */
#define CFG_DoPortCount       (DrvPropertyCode + 37)


/* Counter functions */
#define CFG_CounterCountValue (DrvPropertyCode + 38) /* 82C54 Counter 0-2 value */


/* Get Counter channels' capability. */
 
/* Data type: Long Array.  */
/*     One 'Long' for one channel, each bit of the long indicates */
/* a specified function whether is available or not and the */
/* following function has been defined.  */
#define CFG_CntrChannelCapability      (DrvPropertyCode + 39)

/* Get supported gate of the counter. */
/* Data type: Long.  */
/*     Each bit for a specified gate mode and the */
/* following mode has been defined.  */
#define CFG_CntrSupportedGateMode      (DrvPropertyCode + 40)


/* Get supported counting edge of the counter. */
/* Data type: Long.  */
/*     Each bit for a specified counting edge and the */
/* following bit has been defined.  */
#define CFG_CntrSupportedCountEdge     (DrvPropertyCode + 41)

/* Get supported output mode of the counter when terminal count reached. */
/* Data type: Long.  */
/*     Each bit for a specified output mode and the */
/* following mode has been defined.  */
#define CFG_CntrSupportedOutMode       (DrvPropertyCode + 42)


/* Get available special api of the counter */
/* Data type: Long.  */
/*     Each bit for a specified api and the */
/* following has been defined.  */
#define CFG_CntrAvailableAPI           (DrvPropertyCode + 43)

/* Get event id of the channel which can issue event. */
/* Data type: USHORT Array.  */
/*     One 'USHORT' for one channel, 0 indicates the channel can't */
/* issue event. */
#define CFG_CntrChannelEventID         (DrvPropertyCode + 44)

/* Get the internal clock frequence device used for pulse out,  */
/* PWModulation or timer interrupt, etc... */
/* Data type: float array.  */
/*     Each element of array is a clock frequence value the device  */
/* supported and the frequence should be sorted from lower to higher. */
#define CFG_CntrInternalClockFreq      (DrvPropertyCode + 45)

/* Get the validate range of period and HiPeriod of PWModulation function. */
/* data type: float array */
/*      the data order in array:  */
/*      [min_period][max_period][min_hiperiod][max_hiperiod] */
#define CFG_CntrPWModulateValidRange   (DrvPropertyCode + 46)

/* Interrupt functions */
#define CFG_IrqDiChangeStatusChannel   (DrvPropertyCode + 47) /* Change status channel number */
#define CFG_IrqDiTriggerSignals        (DrvPropertyCode + 48) /* Each bit for each Auxiliary DIO channel */

#define CFG_IrqDiChangeStatusChannel_Name  "IrqDiChangeStatusChannel"  
#define CFG_IrqDiTriggerSignals_Name       "IrqDiTriggerSignals"       

#define CFG_WatchdogCounter            (DrvPropertyCode + 49) /* Watchdog counter */
#define CFG_DoWatchdogValue            (DrvPropertyCode + 50) /* Do status when watchdog overflow */

#define CFG_DiDataWidth      	       (DrvPropertyCode + 51) /* Get DI data width.
							      * The optimized data width when Reading. */
/* Find the used value from Variable Group. */
/* Data type: LONG.    */
#define CFG_DiChannelCount             (DrvPropertyCode + 52)/* Get DI Channel Count.
							     * Max available DI channel count on the card. */
/* Data type: LONG.  */
#define CFG_DiPortCount                (DrvPropertyCode + 53) /* Get DI Port Count.
							      * Max available DI Port count on the card. */


/* Get DI Interrupt supported channel.
 * Data type: BYTE Array.
 *      One bit for one channel. If a bit is 1, 
 *      the channel can issue interrupt. */
#define CFG_DiInterruptSupportedChannel		(DrvPropertyCode + 54)
													

/* Get / Set DI channels which issue interrupt on RISING Edge.
 * Note: whether this property can be set or not depends on device feature.
 * Data type: BYTE Array. 
 *      One bit for one channel. If a bit is 1, 
 *      the channel will issue interrupt on rising edge. */
#define CFG_DiInterruptTriggerOnRisingEdge	 (DrvPropertyCode + 55)
													

/* Get / Set DI channels which issue interrupt on FALLING Edge.
 * Note: whether this property can be set or not depends on device feature.
 * Data type: BYTE Array.  
 *      One bit for one channel. If a bit is 1, 
 *      the channel will issue interrupt on falling edge. */
#define CFG_DiInterruptTriggerOnFallingEdge	  (DrvPropertyCode + 56)



/* Get DI channels which support Status Change interrupt.
 * Data type: BYTE Array. 
 *      One bit for one channel. If a bit is 1,  
 *      the channel can issue interrupt when status changed. */
#define CFG_DiStatusChangeSupportedChannel	  (DrvPropertyCode + 57)


/* Get/Set DI channels which "Status Changed interrupt" function is enabled. 
 * Data type: BYTE Array. 
 *     One bit for one channel. If a bit is 1, 
 *     the channel will issue interrupt when status changed. */
#define CFG_DiStatusChangeEnabledChannel	  (DrvPropertyCode + 58)


/* Get DI channels which support pattern match interrupt. 
 * Data type: BYTE Array.  
 *      One bit for one channel. If a bit is 1, 
 *      the channel can issue interrupt when pattern matched. */
#define CFG_DiPatternMatchSupportedChannel	  (DrvPropertyCode + 59)


/* Get/Set DI channels which "pattern match interrupt" function is enabled. 
 * Data type: BYTE Array.  
 *     One bit for one channel. If a bit is 1,  
 *     the channel will issue interrupt when pattern matched. */
#define CFG_DiPatternMatchEnabledChannel	  (DrvPropertyCode + 60)

/* Get DI Interrupt supported trigger mode. 
 * Data type: LONG.  
 *      which trigger mode was supported. */
#define CFG_DiInterruptSupportedTriggerMode       (DrvPropertyCode + 61)

/* Get DI Pattern Match capability: 
 *      Wether the mask of this port can be set individually.
 * Data Type:  
 *      BYTE Array. 
 *      One byte for a port. If the byte is 1, then the mask 
 *      of this port can be set individually, otherwise it 
 *      can't be done.  
 *      Note: if this property is not supported, it means that 
 *      the mask can be set individually too. */
#define CFG_DiPatternMatchMaskSupportedPort       (DrvPropertyCode + 62)

/* Get DI Status Change capability: 
 *      Wether the mask of this port can be set individually. 
 * Data Type:  
 *      BYTE Array. 
 *      One byte for a port. If the byte is 1, then the mask 
 *      of this port can be set individually, otherwise it 
 *      can't be done.  
 *      Note: if this property is not supported, it means that 
 *      the mask can be set individually too. */
#define CFG_DiStatusChangeMaskSupportedPort       (DrvPropertyCode + 63)


#define CFG_DiTriggerEnableRisingChannel0_Name           "CFG_DiTriggerEnableRisingChannel0"
#define CFG_DiTriggerEnableRisingChannel1_Name           "CFG_DiTriggerEnableRisingChannel1"
#define CFG_DiTriggerEnableFallingChannel0_Name          "CFG_DiTriggerEnableFallingChannel0"
#define CFG_DiTriggerEnableFallingChannel1_Name          "CFG_DiTriggerEnableFallingChannel1"

/* Get / set DI Transfer Request mode */
/* Data type: ULONG */
/*     0: slave mode, 1 master mode */
#define CFG_DiTransferRequestMode     (DrvPropertyCode + 64)


/* Get DO data width. The optimized data width when writing. */
/* Data type: LONG.  */
/*     See the following for all available setting. */
/* #define CFG_DoDataWidth               (DrvPropertyCode + 65) */	

/* Get DO Channel Count. Max available DO channel count on the card. */
/* Data type: LONG.  */
/* #define CFG_DoChannelCount            (DrvPropertyCode + 66) */	   


/* Get Do Port Count. Max available DO Port count on the card.  */
/* Data type: LONG. */
/* #define CFG_DoPortCount               (DrvPropertyCode + 67) */

/* #define CFG_DiInterruptTriggerOnRisingEdge 0x8001 */
/* #define CFG_DiInterruptTriggerOnFallingEdge 0x8002 */

#define CFG_IDITriggerEnableRisingChannel0     (DrvPropertyCode + 65)
#define CFG_IDITriggerEnableFallingChannel0    (DrvPropertyCode + 66)
#define CFG_IDITriggerEnableRisingChannel1     (DrvPropertyCode + 67)
#define CFG_IDITriggerEnableFallingChannel1    (DrvPropertyCode + 68)

#define CFG_DITriggerEnableRisingChannel0      (DrvPropertyCode + 69)
#define CFG_DITriggerEnableFallingChannel0     (DrvPropertyCode + 70)
#define CFG_DITriggerEnableRisingChannel1      (DrvPropertyCode + 71)
#define CFG_DITriggerEnableFallingChannel1     (DrvPropertyCode + 72)


#define CFG_InterruptTriggerSource    (DrvPropertyCode + 73)


#define CFG_AiTimerClock              (DrvPropertyCode + 93)

#define CFG_DiEnableLatch             (DrvPropertyCode + 94)
#define DI_Enable_Latch               CFG_DiEnableLatch              
/* for PCI-1758U used */
#define CFG_DiFilterEnablePort0       (DrvPropertyCode + 106)
#define CFG_DiFilterEnablePort1       (DrvPropertyCode + 107)
#define CFG_DiFilterEnablePort2       (DrvPropertyCode + 108)
#define CFG_DiFilterEnablePort3       (DrvPropertyCode + 109)
#define CFG_DiFilterEnablePort4       (DrvPropertyCode + 110)
#define CFG_DiFilterEnablePort5       (DrvPropertyCode + 111)
#define CFG_DiFilterEnablePort6       (DrvPropertyCode + 112)
#define CFG_DiFilterEnablePort7       (DrvPropertyCode + 113)
#define CFG_DiFilterEnablePort8       (DrvPropertyCode + 114)
#define CFG_DiFilterEnablePort9       (DrvPropertyCode + 115)
#define CFG_DiFilterEnablePort10      (DrvPropertyCode + 116)
#define CFG_DiFilterEnablePort11      (DrvPropertyCode + 117)
#define CFG_DiFilterEnablePort12      (DrvPropertyCode + 118)
#define CFG_DiFilterEnablePort13      (DrvPropertyCode + 119)
#define CFG_DiFilterEnablePort14      (DrvPropertyCode + 120)
#define CFG_DiFilterEnablePort15      (DrvPropertyCode + 121)
#define CFG_DiFilterEnableForAll      (DrvPropertyCode + 122)
#define CFG_DiFilterIntervalCounter   (DrvPropertyCode + 123)
/* for PCI-1758U end */

/* for PCI-1784 begin */
#define CFG_CntrChannelCount          (DrvPropertyCode + 124)
#define CFG_CntrResolution            (DrvPropertyCode + 125)
#define CFG_CntrResetValue            (DrvPropertyCode + 126)
#define CFG_Cntr0SupportedEventType   (DrvPropertyCode + 127)
#define CFG_Cntr1SupportedEventType   (DrvPropertyCode + 128)
#define CFG_Cntr2SupportedEventType   (DrvPropertyCode + 129)
#define CFG_Cntr3SupportedEventType   (DrvPropertyCode + 130)
#define CFG_Cntr4SupportedEventType   (DrvPropertyCode + 131)
#define CFG_CntrSupportedCounterLock  (DrvPropertyCode + 132)

#define CFG_CntrIndicatorControl     (DrvPropertyCode + 133)
#define CFG_CntrCounterLockControl   (DrvPropertyCode + 134)
#define CFG_Cntr0SupportedClockFrequency (DrvPropertyCode + 135)
#define CFG_Cntr1SupportedClockFrequency (DrvPropertyCode + 136)
#define CFG_Cntr2SupportedClockFrequency (DrvPropertyCode + 137)
#define CFG_Cntr3SupportedClockFrequency (DrvPropertyCode + 138)
#define CFG_Cntr4SupportedClockFrequency (DrvPropertyCode + 139)
#define CFG_CntrClockFrequency        (DrvPropertyCode + 140)
#define CFG_Cntr0SupportedInputMode   (DrvPropertyCode + 141)
#define CFG_Cntr1SupportedInputMode   (DrvPropertyCode + 142)
#define CFG_Cntr2SupportedInputMode   (DrvPropertyCode + 143)
#define CFG_Cntr3SupportedInputMode   (DrvPropertyCode + 144)
#define CFG_CntrInputModeControl      (DrvPropertyCode + 145)
#define CFG_CntrCompareData           (DrvPropertyCode + 146)
#define CFG_CntrDigitalFilter         (DrvPropertyCode + 147)
#define CFG_CntrIndexReset            (DrvPropertyCode + 148)
#define CFG_CntrSupportedIndicator  (DrvPropertyCode + 149)
/* for PCI-1784 end */


/* for PCI-1760 begin */
#define CFG_DiFilterEnable            (DrvPropertyCode + 150)
#define CFG_DiFilterHiValue           (DrvPropertyCode + 151)
#define CFG_DiFilterLowValue          (DrvPropertyCode + 152)
#define CFG_DiPatternEnable           (DrvPropertyCode + 153)
#define CFG_DiPatternValue            (DrvPropertyCode + 154)
#define CFG_DiCounterEnable           (DrvPropertyCode + 155)
#define CFG_DiCounterTrigEdge         (DrvPropertyCode + 156)
#define CFG_DiCounterMatchEnable      (DrvPropertyCode + 157)
#define CFG_DiCounterPreset           (DrvPropertyCode + 158)
#define CFG_DiCounterValue            (DrvPropertyCode + 159)
#define CFG_DiCounterOverflow         (DrvPropertyCode + 160)
#define CFG_DiCounterDirection        (DrvPropertyCode + 161)
#define CFG_DiStatusEnable            (DrvPropertyCode + 162)
#define CFG_DiStatusChangeOnRisingEdge        (DrvPropertyCode + 163)
#define CFG_DiStatusChangeOnFallingEdge       (DrvPropertyCode + 164)
/* for PCI-1760 end */
#define CFG_AiCyclicMode               (DrvPropertyCode + 165)



#define CFG_FirmWareVersion           (DrvPropertyCode + 166)

#define CFG_DoEnableChannelFreeze     (DrvPropertyCode + 167)

#define CFG_CntrInternalClockFreqMode     (DrvPropertyCode + 168)

#define CFG_CURRENT_4TO20MA_EXCEPTIONSETTING (DrvPropertyCode + 169)

#define CFG_EEPROM_RWBYTE   (DrvPropertyCode + 170)

#define CFG_InterruptExt        (DrvPropertyCode + 171)

/* for USB device add by Zhiyong */
/* This section must use bit data */
#define CNTR_ChlCap_EventCounting            0x0001 /* bit 0 */
#define CNTR_ChlCap_FreqMeasurement          0x0002 /* bit 1 */
#define CNTR_ChlCap_FreqOut                  0x0004 /* bit 2 */
#define CNTR_ChlCap_TimerInterrupt           0x0008 /* bit 3 */
#define CNTR_ChlCap_CounterInterrupt         0x0010 /* bit 4 */
#define CNTR_ChlCap_PulseOut                 0x0020 /* bit 5 */
#define CNTR_ChlCap_WatchDogTimer            0x0040 /* bit 6 */
#define CNTR_ChlCap_CascadeWithNext          0x0080 /* bit 7 */
#define CNTR_ChlCap_CascadeWithPrev          0x0100 /* bit 8 */
#define CNTR_ChlCap_PulseWidthMeasurement    0x0200 /* bit 9 */
#define CNTR_ChlCap_PulseWidthModulation     0x0400 /* bit 10 */

#define CNTR_CountEdge_RisingEdge            0x0001
#define CNTR_CountEdge_FaillingEdge          0x0002

#define CNTR_API_DICounterReset              0x0001
#define CNTR_API_DICounterEventRead          0x0002
#define CNTR_API_CounterConfig               0x0004

#define CNTR_OutMode_HighPulse               0x0001
#define CNTR_OutMode_LowPulse                0x0002
#define CNTR_OutMode_LowToHigh               0x0004
#define CNTR_OutMode_HighToLow               0x0008

#define CNTR_GateMode_NoGate                 0x0001
#define CNTR_GateMode_HighLevel              0x0002
#define CNTR_GateMode_LowLevel               0x0004

#define CFG_DiInterruptMode          0x3C90	/*modify by zhenyu*/
#define CFG_DioPresetChanDir          0x3007	/*modify by zhenyu*/
/* end section */

/* Property value */
#define DIO_ChannelDir_DI             0x00
#define DIO_ChannelDir_DO             0xff
#define DIO_ChannelDir_HighHalf_DO    0xf0
#define DIO_ChannelDir_LowHalf_DO     0x0f
#define DIO_ChannelDir_Invalid        (DrvPropertyCode + 78)

#define DIO_PortType_DI               (DrvPropertyCode + 79)
#define DIO_PortType_DO               (DrvPropertyCode + 80)
#define DIO_PortType_SeperateDIO      (DrvPropertyCode + 81)
#define DIO_PortType_8255_PortA       (DrvPropertyCode + 82)
#define DIO_PortType_8255_PortC       (DrvPropertyCode + 83)
#define DIO_PortType_Individual       (DrvPropertyCode + 84)

#define DI_DataWidth_Byte             (DrvPropertyCode + 85)
#define DI_DataWidth_Word             (DrvPropertyCode + 86)
#define DI_DataWidth_DWord            (DrvPropertyCode + 87)
#define DO_DataWidth_Byte             (DrvPropertyCode + 88)
#define DO_DataWidth_Word             (DrvPropertyCode + 89)
#define DO_DataWidth_DWord            (DrvPropertyCode + 90)

#define DI_Int_SingleEdgeTrigger      (DrvPropertyCode + 91)
#define DI_Int_DualEdgeTrigger        (DrvPropertyCode + 92)


/* for PCI1750 used */
#define TriggerSource_DI0             0x01
#define TriggerSource_DI0_DI4         0x02
#define TriggerSource_TMR1            0x03

#define TriggerSource_DI8             0x01
#define TriggerSource_DI8_DI12        0x02
#define TriggerSource_CNT2            0x03
#define TriggerSource_Disable         0x00
/* for PCI-1750 end */

/* for PCI-1751 used */
#define TriggerSource_PC0             0x01
#define TriggerSource_PC0_PC4         0x02
#define TriggerSource_PC10            0x01
#define TriggerSource_PC10_PC14       0x02
/* for PCI-1751 end */


/* for pci-1784 */
#define CNTR0OVERFLOWENABLE           0x1 /* Counter 0 overflow interrupt */
#define CNTR1OVERFLOWENABLE           0x2 /* Counter 1 overflow interrupt */
#define CNTR2OVERFLOWENABLE           0x4 /* Counter 2 overflow interrupt */
#define CNTR3OVERFLOWENABLE           0x8 /* Counter 3 overflow interrupt */
#define CNTR0UNDERFLOWENABLE          0x10 /* Counter 0 underflow interrupt */
#define CNTR1UNDERFLOWENABLE          0x20 /* Counter 1 underflow interrupt */
#define CNTR2UNDERFLOWENABLE          0x40 /* Counter 2 underflow interrupt */
#define CNTR3UNDERFLOWENABLE          0x80 /* Counter 3 underflow interrupt */
#define CNTR0INDEXENABLE              0x100 /* Counter 0 index interrupt */
#define CNTR1INDEXENABLE              0x200 /* Counter 1 index interrupt */
#define CNTR2INDEXENABLE              0x400 /* Counter 2 index interrupt */
#define CNTR3INDEXENABLE              0x800 /* Counter 3 index interrupt */
#define CNTR0DIENABLE 	              0x1000 /* Counter 0 digital input interrupt */
#define CNTR1DIENABLE                 0x2000 /* Counter 1 digital input interrupt */
#define CNTR2DIENABLE                 0x4000 /* Counter 2 digital input interrupt */
#define CNTR3DIENABLE                 0x8000 /* Counter 3 digital input interrupt */
#define CNTR0OVERCOMPAREENABLE        0x10000 /* Counter 0 over compare interrupt */
#define CNTR1OVERCOMPAREENABLE        0x20000 /* Counter 1 over compare interrupt */
#define CNTR2OVERCOMPAREENABLE        0x40000 /* Counter 2 over compare interrupt */
#define CNTR3OVERCOMPAREENABLE        0x80000 /* Counter 3 over compare interrupt */
#define CNTR0UNDERCOMPAREENABLE       0x100000 /* Counter 0 under compare interrupt */
#define CNTR1UNDERCOMPAREENABLE       0x200000 /* Counter 1 under compare interrupt */
#define CNTR2UNDERCOMPAREENABLE       0x400000 /* Counter 2 under compare interrupt */
#define CNTR3UNDERCOMPAREENABLE       0x800000 /* Counter 3 under compare interrupt */
#define TIMERPULSEENABLE              0x1000000 /* Timer pulse Interrupt */
#define IEOVERRALLENABLE              0x10000000 /* IE Overall interrupt */
/*for pci-1784 end*/


/* Property value end*/
#endif
