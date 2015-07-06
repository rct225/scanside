/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech USB Device Driver for Linux

File Name:
	USBCtrlCode.h
Abstract:
	Header file of USB device driver. 
Version history
	03/20/2006		Create by zhiyong.xie

************************************************************************/	


#ifndef CTRL_CODE_H
#define CTRL_CODE_H




//
// Major Control Code
//
#define MAJOR_DIO			0x01
	// DIO
	#define MINOR_DO				0x0001
	#define MINOR_DO_READ_TX	0x0002
	#define MINOR_DO_READ_RX	0x0003
	#define MINOR_DI_TX				0x0004
	#define MINOR_DI_RX				0x0005

	

#define MAJOR_AI			0x02
	#define MINOR_AI_SETGAIN		0x0001
	#define MINOR_AI_BINARYIN_TX	0x0002
	#define MINOR_AI_BINARYIN_RX	0x0003
	#define MINOR_MAI_BINARYIN_TX	0x0004
	#define MINOR_MAI_BINARYIN_RX	0x0005
	#define MINOR_MAI_SETGAIN		0x0006
	#define MINOR_FAI_INTSTART		0x0007
	#define MINOR_FAI_TERMINATE		0x0008
	#define MINOR_FAI_INTSCAN		0x0009
	#define MINOR_FAI_SETPACER		0x000a
	#define	MINOR_FAI_ZLP			0x000b
	#define MINOR_FAI_CLEAROVERRUN	0x000c
	#define MINOR_CJCERROR_READ     0x0021
	#define MINOR_CJCERROR_SAVE     0x0022
	#define MINOR_CJCTEMP_READ      0x0023
	#define MINOR_CJCERROR_RESET    0x0024
    #define MINOR_FIXCJCERROR_SAVE  0x0025
	#define MINOR_FIXCJCERROR_LOAD  0x0026

	#define MINOR_START_ADC			0x0030
	#define MINOR_RESET_ADC			0x0031
	#define MINOR_GET_CHLSTATUS_TX  0x0040
    #define MINOR_GET_CHLSTATUS_RX  0x0041
	#define MINOR_SET_CHLSTATUS     0x0042


#define MAJOR_AO			0x03
	#define MINOR_AO_CONFIG			0x0001
	#define MINOR_AO_BINARYOUT		0x0002

#define MAJOR_COUNTER		0x04
	#define MINOR_CNT_EVENTSTART	0x0001
	#define MINOR_CNT_EVENTREAD_TX	0x0002		// reserved in USB4711
	#define MINOR_CNT_EVENTREAD_RX	0x0003
	#define MINOR_CNT_RESET			0x0004
	#define MINOR_CNT_PULSESTART    0x0005
	#define MINOR_CNT_FREQSTART		0x0006
	#define MINOR_CNT_FREQ_READ_TX  0x0007
	#define MINOR_CNT_FREQ_READ_RX  0x0008
	#define MINOR_CNT_GET_BASECLK   0x0009
	#define MINOR_CNT_PWMSETTING   0x000A
	#define MINOR_CNT_PWMENABLE   0x000B
	#define MINOR_CNT_FREQOUTSTART  0x000C	/*modify by zhenyu*/

#define MAJOR_CALIBRATION	0x05
	#define MINOR_CAO_READEEP_TX	0x0001
	#define MINOR_CAO_READEEP_RX	0x0002
	#define MINOR_CAO_WRITEEEP		0x0003
	#define MINOR_CAO_WRITETRIM		0x0004

	#define MINOR_CAI_READEEP_TX	0x0011
	#define MINOR_CAI_READEEP_RX	0x0012
	#define MINOR_CAI_WRITEEEP		0x0013
	#define MINOR_CAI_WRITETRIM		0x0014
    #define MINOR_CAI_RESTORE       0x0021
    #define MINOR_CAI_FULLCALI      0x0022
    #define MINOR_CAI_ZEROCALI      0x0023
	#define MINOR_CAI_FIXFULLCALI   0x0024
    #define MINOR_CAI_FIXZEROCALI   0x0025
	#define MINOR_CAI_COMPLETECALI  0x0026



#define MAJOR_DIRECT_IO		0x10
	// Direct I/O
	#define MINOR_DIRECT_WRITE	0x0001
	#define MINOR_DIRECT_READ	0x0002
	#define MINOR_DIRECT_EE_READ_TX    0x0010
	#define MINOR_DIRECT_EE_READ_RX    0x0011
    #define MINOR_DIRECT_EE_WRITE_TX   0X0012
	#define MINOR_FIXFUNCTIONCALL_TX   0X0013
	#define MINOR_FIXFUNCTIONCALL_RX   0x0014

#define MAJOR_EVENT   0x11
        #define MINOR_EVENT_ENABLE 0x0001

#define MAJOR_SYSTEM		0x7f
	#define MINOR_READ_SWITCHID		0x0001	
	#define MINOR_WRITE_SWITCHID	0x0002	
	#define MINOR_GET_LAST_ERROR	0x0003	
	#define MINOR_DOWNLOAD_PROG		0x0004
	#define MINOR_GET_FW_VERSION	0x0005
	#define MINOR_USBLED			0x0006
	#define MINOR_DEVICE_OPEN		0x0007
	#define MINOR_DEVICE_CLOSE		0x0008
	#define MINOR_LOCATE			0x0009
    #define MINOR_GET_FW_SIZE       0x000a     //yingsong add to support the 
                                               //uniform FW download utility
	#define MINOR_GET_FW_PAGESIZE_LO 0x000b
	#define MINOR_GET_FW_PAGESIZE_HI 0x000c  
	#define MINOR_GET_USB_HW_INFO	 0x000d    
	#define MINOR_GET_USB_FLAG       0x000e  //wanglong add to get usb flag from 
                                             //FW, the flag defined by the driver
	#define MINOR_DELAY             0x000f
	#define MINOR_RESETDEVICE       0x0010 

//-----------------------------------------------------------------
#define MAJOR_TEST			0x80
	//MAJOR_TEST = 0x8F Interrupt Control
	//usCtrlCode_Minor (Just For INT1 Control)
	#define INT_Disable		0
	#define INT_SRC_HF		1
	#define INT_SRC_EMPTY	2
	#define INT_SRC_USER	3

#define MAJOR_TEST_AO		0x90
	//usCtrlCode_Minor
	#define AO0_Range		0x00
	#define AO1_Range		0x01
	#define AO0_OUT			0x10
	#define AO1_OUT			0x11
		//usChan
		#define Range0		0x00
		#define Range1		0x01
		#define Range2		0x02
		#define Range3		0x03
		#define Range4		0x04
		
	#define AO0_Cal			0x20
	#define AO1_Cal			0x21
		//usChan
		#define Offset_Up	0x01
		#define Offset_Down	0x81
		#define Gain_Up		0x02
		#define Gain_Down	0x82

	#define AO0_Save_EEP	0x30
	#define AO1_Save_EEP	0x31
	#define AO0_Save_ALL	0x40
	#define AO1_Save_ALL	0x41

	#define AO0_Read_EEP	0x50
	#define AO1_Read_EEP	0x51
		//usChan
		#define Range0		0x00
		#define Range1		0x01
		#define Range2		0x02
		#define Range3		0x03
		#define Range4		0x04

#define MAJOR_TEST_AI		0xA0
	//usCtrlCode_Minor
	#define Set_CH_Range	0x00
			//pBuf : Is HHHHH : Start - Stop - S/D - B/U - Gain
	#define Set_Trigger_Mode	0x01
			//pBuf : Is Mode : 0 ~ 3
			#define MASK_SW		0
			#define MASK_PACER	1
			#define MASK_EXT	2
			#define MASK_GATE	3
	#define Set_Scan_CH			0x02
			//pBuf : Is HHHH : SSPP : Start And Stop Channel
	#define Set_INT_SRC			0x03

	#define Cal_AI				0x04
		//usChan
		#define Read_AI		0x00
			//pBuf Is Channel Code : 0 ~ F
		#define Offset_Up	0x01
		#define Offset_Down	0x81
		#define Gain_Up		0x02
		#define Gain_Down	0x82
		#define PGA_Up		0x03
		#define PGA_Down	0x83
		#define Save_AI_Gain	0x04
		#define Save_AI_Offset	0x05
		#define Save_AI_PGA		0x06
		#define Read_AI_Par		0x07
		#define Step1		0xE1
			//pBuf : Is HHHHH : Start - Stop - S/D - B/U - Gain
		#define Step2		0xE2
		#define Step3		0xE3
			//pBuf : Is GHHH : G: AO0 GainIndex HHH: AO0 Data
		#define Step4		0xE4
		#define Step5		0xE5
		#define Step6		0xE6
		#define Step7		0xE7

#define MAJOR_TEST_DIO		0xB0
	//usCtrlCode_Minor
			//pBuf : Is Data
	#define Set_DO_Data		0x00
	#define Get_DO_Status	0x01
	#define Get_DI_Status	0x02

#define MAJOR_TEST_EEPROM	0xC0
	//usCtrlCode_Minor
	#define Read_EEPROM		0x00
	#define Write_EEPROM	0x01
	#define Read_Module_ID	0x02
	#define Write_Module_ID	0x03
		//usChan / Offset = Address : 0x00 ~ 0xFF
			//pBuf : Is Data

//
// Minor Control Code
//


#endif
