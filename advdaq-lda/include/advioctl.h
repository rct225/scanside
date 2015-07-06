#ifndef _ADV_DAQ_IOCTL_H
#define _ADV_DAQ_IOCTL_H

/**
 * 
 * advioctl.h   header file for ioctl command code
 *  
 * Copyright (C) 2006 Advantech Co., Ltd
 *
 * Change Log:
 *       2006/07/14  Initial Version
 *       2006/08/17  Second  Version
 *
 */

#include <linux/ioctl.h>

/* IOCTL defines */
#define ADV_IOC_MAGIC_AI  'a'
#define ADV_IOC_MAGIC_AO  'b'
#define ADV_IOC_MAGIC_DIO 'c'
#define ADV_IOC_MAGIC_CNT 'd'
#define ADV_IOC_MAGIC_EVT 'f'

#define ADV_IOC_MAGIC_PORT 'g'
#define ADV_IOC_MAGIC_UTIL 'h'
#define ADV_IOC_MAGIC_FLAG 'i'
#define ADV_IOC_MAGIC_PROP 'j'

#define ADV_IOC_MAGIC_USB 'k'

/* Utility command */
#define ADV_GET_CONFIG			_IO(ADV_IOC_MAGIC_UTIL, 1)
#define ADV_CLOCK			_IO(ADV_IOC_MAGIC_UTIL, 2)
#define USB_LOCATE_DEVICE    		_IO(ADV_IOC_MAGIC_UTIL, 3)
#define USB_STOP_LOCATE_DEVICE    	_IO(ADV_IOC_MAGIC_UTIL, 4)
#define UNDEVCONFIG                  	_IO(ADV_IOC_MAGIC_UTIL, 5)
#define ISR           			_IO(ADV_IOC_MAGIC_UTIL, 6)
#define DEVCONFIG                 	_IO(ADV_IOC_MAGIC_UTIL, 7)
#define GETDEVINFO			_IO(ADV_IOC_MAGIC_UTIL, 8)
#define GET_DEVICE_DRIVER_NAME          _IO(ADV_IOC_MAGIC_UTIL, 9)
#define USB_SET_DEVICE_BOARDID          _IO(ADV_IOC_MAGIC_UTIL, 10)
#define ISA_ADD_DEVICE                  _IO(ADV_IOC_MAGIC_UTIL, 11)
#define ISA_REMOVE_DEVICE               _IO(ADV_IOC_MAGIC_UTIL, 12)

/* Property command */
#define ADV_DEVGETPROPERTY        	_IO(ADV_IOC_MAGIC_PROP, 1)
#define ADV_DEVSETPROPERTY        	_IO(ADV_IOC_MAGIC_PROP, 2)
#define ADV_GET_ERR_CODE              	_IO(ADV_IOC_MAGIC_PROP, 3)
#define ADV_GET_DEVINFO                 _IO(ADV_IOC_MAGIC_PROP, 4)

/* Port I/O command */
#define ADV_PORTREAD        		_IO(ADV_IOC_MAGIC_PORT, 1)
#define ADV_PORTWRITE        		_IO(ADV_IOC_MAGIC_PORT, 2)


/* Flag command */
#define CLEARFLAG                 	_IO(ADV_IOC_MAGIC_FLAG, 1)
#define FAICHECK			_IO(ADV_IOC_MAGIC_FLAG, 2)
#define CLEAR_OVERRUN                   _IO(ADV_IOC_MAGIC_FLAG, 3)
#define ADV_CLEAR_FLAG                  _IO(ADV_IOC_MAGIC_FLAG, 4)

/* Event command */
#define ADV_ENABLE_EVENT        	_IO(ADV_IOC_MAGIC_EVT, 1)
#define CHECKEVENT			_IO(ADV_IOC_MAGIC_EVT, 2)
#define ADV_ENABLE_EVENT_EX           	_IO(ADV_IOC_MAGIC_EVT, 3)


/* Counter command */
#define ADV_COUNTER			_IO(ADV_IOC_MAGIC_CNT, 1)
#define EVENT_START                  	_IO(ADV_IOC_MAGIC_CNT, 2)
#define EVENT_READ                  	_IO(ADV_IOC_MAGIC_CNT, 3)
#define FREQ_START                  	_IO(ADV_IOC_MAGIC_CNT, 4)
#define FREQ_READ                  	_IO(ADV_IOC_MAGIC_CNT, 5)
#define PULSE_START			_IO(ADV_IOC_MAGIC_CNT, 6)
#define FREQ_OUT			_IO(ADV_IOC_MAGIC_CNT, 7)
#define FOUT_RESET			_IO(ADV_IOC_MAGIC_CNT, 8)
#define CNT_CONFIG			_IO(ADV_IOC_MAGIC_CNT, 9)
#define CNT_WRITE			_IO(ADV_IOC_MAGIC_CNT, 10)
#define CNT_READ			_IO(ADV_IOC_MAGIC_CNT, 11)
#define CNT_BITS			_IO(ADV_IOC_MAGIC_CNT, 12)
#define CNT_RESET			_IO(ADV_IOC_MAGIC_CNT, 13)
#define CNT_PWM_ENABLE                	_IO(ADV_IOC_MAGIC_CNT, 14)
#define CNT_PWM_SETTING               	_IO(ADV_IOC_MAGIC_CNT, 15)
#define TMR_WRITE			_IO(ADV_IOC_MAGIC_CNT, 16)
#define TMR_READ			_IO(ADV_IOC_MAGIC_CNT, 17)
#define TMRCNT_SET                  	_IO(ADV_IOC_MAGIC_CNT, 18)
#define WATCHDOG_START                 	_IO(ADV_IOC_MAGIC_CNT, 19)
#define WATCHDOG_FEED                 	_IO(ADV_IOC_MAGIC_CNT, 20)
#define WATCHDOG_STOP                 	_IO(ADV_IOC_MAGIC_CNT, 21)
#define GET_CLOCK_RATE                  _IO(ADV_IOC_MAGIC_CNT, 22)
#define ADV_SETCOUNTERSTATUS            _IO(ADV_IOC_MAGIC_CNT, 23)
#define ADV_GETCOUNTERSTATUS            _IO(ADV_IOC_MAGIC_CNT, 24)

/* DIO command */
#define ADV_DIO_BITS			_IO(ADV_IOC_MAGIC_DIO, 1)
#define DI				_IO(ADV_IOC_MAGIC_DIO, 2)
#define DO				_IO(ADV_IOC_MAGIC_DIO, 3)
#define GETDO				_IO(ADV_IOC_MAGIC_DIO, 4)
#define DI_COUNTER_RESET              	_IO(ADV_IOC_MAGIC_DIO, 5)
#define DIO_EVENT_GET_DI_STATE          _IO(ADV_IOC_MAGIC_DIO, 6)
#define DIO_ENABLE_EVENT              	_IO(ADV_IOC_MAGIC_DIO, 7)
#define DIO_DISABLE_EVENT               _IO(ADV_IOC_MAGIC_DIO, 8)
#define FDI_TRANSFER                    _IO(ADV_IOC_MAGIC_DIO, 9)
#define ADV_FDO_DMA_START               _IO(ADV_IOC_MAGIC_DIO, 10)
#define ADV_FDO_DMA_STOP                _IO(ADV_IOC_MAGIC_DIO, 11)
#define ADV_FDO_DMA_CHECK               _IO(ADV_IOC_MAGIC_DIO, 12)
#define ADV_FDI_DMA_START               _IO(ADV_IOC_MAGIC_DIO, 13)
#define ADV_FDI_DMA_STOP                _IO(ADV_IOC_MAGIC_DIO, 14)
#define ADV_FDI_DMA_CHECK               _IO(ADV_IOC_MAGIC_DIO, 15)
#define ADV_FDI_DMA_STATUS              _IO(ADV_IOC_MAGIC_DIO, 16)
#define ADV_FDO_DMA_STATUS              _IO(ADV_IOC_MAGIC_DIO, 17)

/* AI command */
#define ADV_AIBINARYIN       		_IO(ADV_IOC_MAGIC_AI, 1)
#define ADV_AIVOLTIN        		_IO(ADV_IOC_MAGIC_AI, 2)
#define ADV_AICONFIG        		_IO(ADV_IOC_MAGIC_AI, 3)
#define ADV_AISCALE        		_IO(ADV_IOC_MAGIC_AI, 4)
#define ADV_AI_DMA_START                _IO(ADV_IOC_MAGIC_AI, 5)
#define ADV_AI_INT_START                _IO(ADV_IOC_MAGIC_AI, 6)
#define ADV_AI_INT_SCAN_START           _IO(ADV_IOC_MAGIC_AI, 7)
#define ADV_FAI_TERMINATE               _IO(ADV_IOC_MAGIC_AI, 8)
#define ADV_AI_STOP                     ADV_FAI_TERMINATE
#define ADV_AI_GET_TIMR_CLOCK           _IO(ADV_IOC_MAGIC_AI, 9)
#define ADV_AI_SET_DIVISOR              _IO(ADV_IOC_MAGIC_AI, 10) 
#define ADV_AI_TRANSFER                 _IO(ADV_IOC_MAGIC_AI, 11)

#define ADV_FAI_DMA_CHECK               _IO(ADV_IOC_MAGIC_AI, 12)
#define ADV_FAI_CHECK                   _IO(ADV_IOC_MAGIC_AI, 13)
#define ADV_FAI_TRANSFER                _IO(ADV_IOC_MAGIC_AI, 14)
#define ADV_FAI_DMA_STATUS              _IO(ADV_IOC_MAGIC_AI, 15)
#define ADV_FAI_INT_STATUS              _IO(ADV_IOC_MAGIC_AI, 16)
#define ADV_FAI_INTSCAN_STATUS          _IO(ADV_IOC_MAGIC_AI, 17)
#define ADV_FAI_INT_START_EXT          _IO(ADV_IOC_MAGIC_AI, 18)
#define ADV_FAI_CALIBRATION_SET          _IO(ADV_IOC_MAGIC_AI, 19)
#define ADV_FAI_INT_START                _IO(ADV_IOC_MAGIC_AI, 20)


/* AO command */
#define ADV_AOBINARYOUT               	_IO(ADV_IOC_MAGIC_AO, 1)
#define ADV_AOVOLTOUT                	_IO(ADV_IOC_MAGIC_AO, 2)
#define ADV_AOCURROUT                	_IO(ADV_IOC_MAGIC_AO, 3)
#define ADV_AOCONFIG                	_IO(ADV_IOC_MAGIC_AO, 4)
#define ADV_AOSCALE                	_IO(ADV_IOC_MAGIC_AO, 5)
#define ADV_ENABLEAOSYNC        	_IO(ADV_IOC_MAGIC_AO, 6)
#define ADV_AOWRITESYNC			_IO(ADV_IOC_MAGIC_AO, 7)
#define ADV_MAICONFIG        		_IO(ADV_IOC_MAGIC_AO, 8)
#define ADV_MAIBINARYIN        		_IO(ADV_IOC_MAGIC_AO, 9)
#define ADV_MAIVOLTIN        		_IO(ADV_IOC_MAGIC_AO, 10)
#define ADV_AO_DMA_START             	_IO(ADV_IOC_MAGIC_AO, 11)
#define ADV_AO_WAVEFORM_START        	_IO(ADV_IOC_MAGIC_AO, 12)
#define ADV_AO_CHECK                 	_IO(ADV_IOC_MAGIC_AO, 13)
#define ADV_AO_SCALE                 	_IO(ADV_IOC_MAGIC_AO, 14)
#define ADV_AO_LOAD                  	_IO(ADV_IOC_MAGIC_AO, 15)
#define ADV_FAO_TERMINATE               _IO(ADV_IOC_MAGIC_AO, 16)
#define ADV_AO_STOP                  	ADV_FAO_TERMINATE


#define ADV_FAO_DMA_CHECK               _IO(ADV_IOC_MAGIC_AO, 17)
#define ADV_ALLOCATE_DMA_BUFFER         _IO(ADV_IOC_MAGIC_AO, 18)
#define ADV_FAO_LOAD                    _IO(ADV_IOC_MAGIC_AO, 19)
#define ADV_FAO_DMA_START               _IO(ADV_IOC_MAGIC_AO, 20)
#define ADV_FREE_DMA_BUFFER             _IO(ADV_IOC_MAGIC_AO, 21)
#define ADV_FAO_STOP                    _IO(ADV_IOC_MAGIC_AO, 22)
#define ADV_FAO_LOADEX                  _IO(ADV_IOC_MAGIC_AO, 23)
#define ADV_CLEAR_UNDERRUN              _IO(ADV_IOC_MAGIC_AO, 24)
#define ADV_FAO_CHECK                   _IO(ADV_IOC_MAGIC_AO, 25)
#define ADV_FAO_DMA_STATUS              _IO(ADV_IOC_MAGIC_AO, 26)



/* USB transfer command */
#define ADV_USB_CTRL_TRANSFER           _IO(ADV_IOC_MAGIC_USB, 1)

#endif
