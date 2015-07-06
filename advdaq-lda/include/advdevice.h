#ifndef __ADVDEVICE_H__
#define __ADVDEVICE_H__

#include "advcpu.h"
#include "advioctl.h"
#include "adverror.h"
#include "advproperty.h"
#include "advevent.h"
#define NAMELEN 20

/* for name buffer length */
#define DRIVER_NAME_LENGTH      30
#define DEVICE_NAME_LENGTH      30
#define DEVICE_NODE_NAME_LENGTH 100
#define DLL_DRIVER_NAME_LENGTH  (DRIVER_NAME_LENGTH + 6)

typedef struct _port_data 	port_data;
typedef struct _adv_devinfo	adv_devinfo;
typedef	struct _adv_clock	adv_clock;

#define  UCHAR  unsigned char
#define  ULONG  unsigned long
#define  USHORT unsigned short 	
#define  BYTE unsigned char 	
#define  WORD unsigned short 	
#define  DWORD unsigned long
#define  UINT  unsigned int

#define NOT_SUPPORT_FLOAT 0
#define FLOAT_TYPE_SIZE 4

#define MAX_DEVICE 16		/* support 16 devices */ 


#define CR_PACK(chan, rng, aref)  ((((aref) & 0x03) << 24) | (((rng) & 0xff) << 16) | (chan))
#define CR_CHAN(a)	          ((a) & 0xffff)
#define CR_RANGE(a)	          (((a) >> 16) & 0xff)
#define CR_AREF(a)	          (((a) >> 24) & 0x03)

#define AREF_GROUND	0x00	/*anolog ref = analog ground */
#define AREF_COMMON	0x01	/*analog ref = analog common */
#define AREF_DIFF	0x02	/*analog ref = differential */
#define AREF_OTHER	0x03	/*nanlog ref = other(undefined) */


/* define follow for port read/write */
#define AD_READ_PORT_UCHAR   0x0001
#define AD_READ_PORT_USHORT  0x0002
#define AD_READ_PORT_ULONG   0x0004
#define AD_WRITE_PORT_UCHAR  0x0011
#define AD_WRITE_PORT_USHORT 0x0012
#define AD_WRITE_PORT_ULONG  0x0014
/* end */


/* added by LI.ANG */
/* counter mode */
#define MODE_A             0x0000
#define MODE_B             0x0000
#define MODE_C             0x8000
#define MODE_D             0x0010
#define MODE_E             0x0010
#define MODE_F             0x8010
#define MODE_G             0x0020
#define MODE_H             0x0020
#define MODE_I             0x8020
#define MODE_J             0x0030
#define MODE_K             0x0030
#define MODE_L             0x8030
#define MODE_O             0x8040
#define MODE_R             0x8050
#define MODE_U             0x8060
#define MODE_X             0x8070

/* counter direct */
#define PA_MODE_COUNT_UP             0x0008
#define PA_MODE_COUNT_DOWN           0x0000

/* counter edge */
#define PA_MODE_COUNT_RISE_EDGE      0x0000
#define PA_MODE_COUNT_FALL_EDGE      0x0080

/* output enable */
#define PA_MODE_ENABLE_OUTPUT        0x0004
#define PA_MODE_DISABLE_OUTPUT       0x0000

/* output mode */
#define PA_MODE_ACT_HIGH_TC_PULSE    0x0000
#define PA_MODE_ACT_LOW_TC_PULSE     0x0001
#define PA_MODE_TC_TOGGLE_FROM_LOW   0x0002
#define PA_MODE_TC_TOGGLE_FROM_HIGH  0x0003

/* clock source */
#define PA_MODE_COUNT_SRC_OUT_N_M1   0x0100
#define PA_MODE_COUNT_SRC_CLK_N      0x0200
#define PA_MODE_COUNT_SRC_CLK_N_M1   0x0300
#define PA_MODE_COUNT_SRC_FOUT_0     0x0400
#define PA_MODE_COUNT_SRC_FOUT_1     0x0500
#define PA_MODE_COUNT_SRC_FOUT_2     0x0600
#define PA_MODE_COUNT_SRC_FOUT_3     0x0700
#define PA_MODE_COUNT_SRC_GATE_N_M1  0x0c00

/* gate source */
#define PA_MODE_GATE_SRC_GATE_NO     0x0000
#define PA_MODE_GATE_RC_OUT_N_M1     0x1000
#define PA_MODE_GATE_SRC_GATE_N      0x2000
#define PA_MODE_GATE_SRC_GATE_N_M1   0x3000

/* gate polarity */
#define PA_MODE_GATE_POSITIVE        0x0000
#define PA_MODE_GATE_NEGATIVE        0x4000

/* frequency out clock source */
#define PA_FOUT_SRC_EXTER_CLK        0x0000
#define PA_FOUT_SRC_CLK_N            0x0100
#define PA_FOUT_SRC_FOUT_N_M1        0x0200
#define PA_FOUT_SRC_CLK_10MHZ        0x0300
#define PA_FOUT_SRC_CLK_1MHZ         0x0400
#define PA_FOUT_SRC_CLK_100KHZ       0x0500
#define PA_FOUT_SRC_CLK_10KHZ        0x0600
#define PA_FOUT_SRC_CLK_1KHZ         0x0700
/* end added */

typedef struct{
	float	min;
	float	max;
} chan_range;

typedef INT32S (*driver_entry)(INT32U, PTR_T, PTR_T);

/* for utilities */
typedef struct _device_bind_driver
{
	char *device;
	char *driver;
}dev_drv;

/* driver information for each thread group */
typedef struct __driver_info
{
	BYTE  driver_name[DLL_DRIVER_NAME_LENGTH];// driver name = lib + driver_name + .so
	PTR_T         driver_handle;            // driver info. handle
	driver_entry  proc_entry;               // driver entry address
}driver_info;


/* device information for each thread group */
typedef struct __device_info
{
	struct __device_info * pnext;     // next device info. address
	INT32U        fd;                 // file descriptor
	INT32U        dev_num;            // device number - minor device number
	driver_info   driver;            // driver info
	INT32U        usage_counter;      // usage count
	PTR_T         private_data;       // deivce info. (Lock data)
} device_info, dev_handle;

typedef struct __adv_get_device_info
{
	INT32U   dev_num;
	BYTE     *driver_name;
}adv_get_device_info;


struct _adv_devinfo{
	ULONG  iobase;
	INT32U  iolength;
	INT32U  ioDMAbase;
	INT32U  ioDMAlength;
	INT32U 	deviceID;
	INT32U   BoardID;
	INT32U	switchID;
	INT32U	pci_slot;
	INT32U	pci_bus;
	INT32U  irq;
	INT32U  irq_ext;

	/* for USB device */
	INT16U  usb_speed;
};

/**
 *  Digital Input Function
 */
typedef struct tagPT_DioReadBit
{
	INT16U port;
	INT16U bit;
	INT16U *state;
}PT_DioReadBit, *LPT_DioReadBit;

typedef struct tagPT_DioReadPortByte
{
	INT16U port;
	INT16U *value;
}PT_DioReadPortByte, *LPT_DioReadPortByte;

typedef struct tagPT_DioReadPortWord
{
	INT16U port;
	INT16U *value;
	INT16U *ValidChannelMask;
}PT_DioReadPortWord, *LPT_DioReadPortWord;

typedef struct tagPT_DioReadPortDword
{
	INT16U port;
	INT32U *value;
	INT32U *VaildChannelMask;
}PT_DioReadPortDword, * LPT_DioReadPortDword;


enum CLOCK_TYPE{AI_CLOCK = 0, AO_CLOCK, DI_CLOCK, DO_CLOCK };
enum CLOCK_SOURCE{INTERNEL = 0, EXTERNEL };
enum TRIG_EDGE{RISING = 0, FALLING };
enum TRIG_SOURCE{TRIG = 0, TRIG1 };

struct _adv_clock{
	int	type;		/* AI, AO, DI, DO  */
	int	source;		/* internel, externel */
	int	edge; 
	long 	rate;
	INT32U 	divisor[2];
};

typedef struct _adv_ai_read{
	long	readnum;	
	long	timeout;	
	INT32U 	flag;
}adv_ai_read;

typedef struct _adv_dma_user{
	INT16U  dmachannel;
	VOID    *userptr;
	INT32U  datacnt;        /* conversion number */

	INT16U	startchan;
/* 	INT16U	stopchan; */
	INT16U	numchan;
	INT16U	trig_src;	/* 0 internel, 1 externel */
	INT16U	trigvol;     /* externel trig src threshold voltage */
	INT16U	trig_edge;	/* 0 rising, 1 falling */
	INT16U  src_type;   	/* 
				 * 0 - Digtal 
				 * 1 - Analog input of Channel 0
				 * 2 - Analog input of Channel 1
				 * 3 - Analog input of Channel 2
				 * 4 - Analog input of Channel 3
				 */
	INT16U	trig_mode;     /* 0 pacer, 1 post, 2 delay, 3 about */
	INT16U	cyclic;		/* 0 none cyclic, 1 cyclic */
	INT16U	trig_ext; /* =1 externel trig; =0 internel trig source */
	INT16U	clocksrc;	/* 0 internel, 1 externel */
	/* INT32U	*chanlist; /\* save info of each channel, for old api standard *\/ */
	INT16U  *gainlist;	/* for windows api standard  */
	INT16U	delay_cnt;	/* delay count */
	INT32U  samplerate;
	INT16U	intrcount;
	INT16U  half_fifo_size;
	INT16U	divisor1;
	INT16U	divisor2;
	
} adv_dma_user;

typedef struct _adv_int_user {
	INT16U	trig_src;
	INT16U	divisor1;
	INT16U	divisor2;
	INT16U	numchan;
	INT16U	startchan;
	INT16U	*gainlist;
	INT16U	cyclic;
	INT16U	intrcount;
	INT32U  samplerate;
	INT32U	bufsize;	
	void	*userptr;

	/* for old method pci-1716 pci-1712 */
/* 	INT16U	trig_mode;	 */
/* 	INT32U  *chanlist; */
	/* end */

	INT16U  chan; /* added by LI.ANG for single-channel mode */
	INT16U  gain;
	INT16U  half_fifo_size;
} adv_int_user;

typedef struct _adv_ao_dma_user{
	void	*userptr;
	INT32U	bufsize;	
	INT16U	operation_mode; /* =1 01 ch0 continuous ouput,ch1 single Value operation mode
				 * =2 10 ch1 continuous ouput,ch0 single Value operation mode
				 * =3 11 ch0 and ch1 both continuous output mode */
	INT32U	samplerate;	
	/* INT32U	*chanlist; */
	INT16U	cyclic;
	INT16U	clocksrc;	/* =0 internel, =1 externel */
	INT16U	numchan;
	INT16U	trig_ext;
	INT16U	usDivisor1;	
	INT16U	usDivisor2;	
	INT16U	usControl;	/* control word */
	
} adv_ao_dma_user;

/*
  typedef struct _adv_ai_soft{
  int	startchan;
  int	stopchan;
  void	*buff;
  }adv_ai_soft;
*/

typedef struct _adv_aisoft_data {
	int     channel;
	int     datatype;         /* 0 raw data, 1 float data */
	INT16U  usdata;
	float   fdata;
	INT16U  trigmode;
} adv_aisoft_data;

typedef struct _adv_ai_interrupt_start
{
	int	trig_source;	/* internel, externel */
	int	trig_edge; 
	int	startchan;
	int	stopchan;
	int	readnum;
	int	enablefifo;
	int	cyclic;
}adv_ai_interrupt_start;

typedef struct tagPT_AIStart
{
	INT16U cyclic;		/* IN, 0: non-cyclic 1: cyclic */
	ULONG  count;           /* IN, conversion count  */
	void   *pBuffer;        /* OUT, Buffer for data return */
}PT_AIStart, *LPT_AIStart;

typedef struct tagPT_FastFunctionCheck
{
	ULONG *pdwStatus;       /* OUT, status return buffer */
	ULONG *pdwRetrieved;    /* OUT, Conversion count stored in buffer */
}PT_FastFunctionCheck, *LPT_FastFunctionCheck;


#if NOT_SUPPORT_FLOAT		/* added by LI.ANG for kernel space */
typedef struct _adv_ao_data {
	INT16U chan;		
	INT16U refsrc;                                
	long   max_value;	/* to avoid float point, we use a 4-Bytes space to store
				 * the float point data */
	long   min_value;
} adv_ao_data;
#else
typedef struct _adv_ao_data {
	INT16U channel;		/* 0 or 1 */
	INT16U usdata;
	INT16U refsrc;	/* for reference control */
	float voltage;	/* 5V or 10V */
} adv_ao_data;
#endif

typedef struct _adv_ai_chan{
	INT16U	chan;	
	INT16U	config;	/* Bit[5]:S/D Bit[4]:B/U Bit[3:0]: Gain */
}adv_ai_chan;

/* added by LI.ANG for multiple-channel mode */
typedef struct _adv_mai_data { 
	int datatype;		/* 0 raw data, 1 float data */
	INT16U *usdata;
	float fdata;
	INT16U trigmode;
	INT16U startchan;	
	INT16U stopchan;
	INT16U config[16];   /* Bit[5]:S/D Bit[4]:B/U Bit[2:0]:Gain */
} adv_mai_data;
/* end added */

struct _port_data
{
	int port;
	int size;
	INT32U data;
};

/* zdd */
#define MAX_SAMPLES	256
typedef struct _adv_struct{
	INT32U n;
	INT32U type;	
	INT32U chanspec;
	INT32U portstart;
	INT32U portcount;
	INT32U data;
	INT8U *buffer;
	INT16U gatemode;
	INT32U unused[5];
	INT16U amplify; /* amplifying multiple for non-float frequency */
	float frequency;     /* added by LI.ANG for counter freq_in */

} adv_struct, cnt_struct;

typedef struct tagGAINLIST {	/* added by LI.ANG */
	INT16U usGainCode;	
	float fMaxGainVal;
	float fMinGainVal;
	char szGainStr[16];
} GAINLIST;

typedef struct tagPT_AOConfig{
	INT16U	chan;
	INT16U	RefSrc;		/* =0 internal ref, =1 external ref */
	float	MaxValue;
	float	MinValue;
} PT_AOConfig,*LPT_AOConfig;

typedef struct tagPT_AOBinaryOut{
	INT16U	chan;
	INT16U	BinData;
}PT_AOBinaryOut, *LPT_AOBinaryOut;

typedef struct tagPT_AOVoltageOut{
	INT16U	chan;
	float	OutputValue;
}PT_AOVoltageOut,*LPT_AOVoltageOut;

typedef struct tagPT_AOCurrentOut{
	INT16U	chan;
	float	OutputValue;
}PT_AOCurrentOut,*LPT_AOCurrentOut;

typedef struct tagPT_AOScale{
	INT16U	chan;
	float	OutputValue;
	INT16U	*BinData;
}PT_AOScale, *LPT_AOScale;

typedef struct tagAOSET {
	INT16U	usAOSource;	/* =0 internal, =1 external */
	INT16U  usAOType;	/* added by LI.ANG: 0 -voltage out
				 *                  1 -current out */
	float	fAOMaxVol;
	float	fAOMinVol;
	float	fAOMaxCur;	/* maximun output voltage */
	float	fAOMinCur;	/* minimun output voltage */
} AOSET, *LPAOSET;

typedef struct tagAORANGESET {
	INT16U	usGainCount;	/* how many gains will be supported */
	INT16U	usAOSource;	/* =0 internal,=1 external */
	INT16U	usAOType;	/* =0 voltage,=1 current */
	INT16U	usChan;

	float	fAOMax;		/* maximum output voltage */
	float	fAOMin;
} AORANGESET, *LPAORANGESET;

/* added by LI.ANG 2006-04-24 */
typedef struct tagPT_FAODmaExStart {
	INT16U     TrigSrc;
	INT32U     SampleRate;
	INT16U     StartChan;
	INT16U     NumChans;
	INT32U     *buffer;
	INT32U     count;
	INT16U     CyclicMode;
	INT16U     PacerSource;
	INT32U     Reserved[4];
	void       *pReserved[4];
} PT_FAODmaExStart, *LPT_FAODmaExStart;

typedef struct tagPT_FAOWaveFormStart {
	INT16U     TrigSrc;
	INT32U     SampleRate;
	INT32U     WaveCount;
	INT32U     Count;
	INT16U     *Buffer;
	INT16U     EnabledChannel;
	INT32U     Reserved[4];
	void       *pReserved[4];
} PT_FAOWaveFormStart, *LPT_FAOWaveFormStart;

typedef struct tagPT_FAOCheck {
	INT16U     ActiveBuf;
	INT16U     stopped;
	INT32U      CurrentCount;
	INT16U     overrun;
	INT16U     HalfReady;
} PT_FAOCheck, *LPT_FAOCheck;

typedef struct tagPT_FAOScale {
	INT16U     chan;
	INT32U      count;
	float      *VoltArray;
	INT16U     *BinArray;
} PT_FAOScale, *LPT_FAOScale;

typedef struct tagPT_FAOLoad {
	INT16U     ActiveBuf;
	INT16U     *DataBuffer;
	INT32U     start;
	INT32U      count;
} PT_FAOLoad, *LPT_FAOLoad;
/* end added */

typedef struct tagPT_FAIIntStart{
	INT16U	TrigSrc;
	INT32U	SampleRate;
	INT16U	chan;
	INT16U	gain;
	INT16U	*buffer;
	INT32U	count;
	INT16U	cyclic;
	INT16U	IntrCount; /* =1 without fifo, =0 or =half_fifo_size with fifo */
	/* INT32U *chanlist; */
} PT_FAIIntStart, *LPT_FAIIntStart;

typedef struct tagPT_FAIIntScanStart{
	INT16U	TrigSrc;
	INT32U	SampleRate;
	INT16U	NumChans;
	INT16U	StartChan;
	INT16U	*GainList;
	INT16U	*buffer;
	INT32U	count;
	INT16U	cyclic;
	INT16U	IntrCount;
	/* INT32U *chanlist;	/\* for old method pci-1716 pci-1712 *\/ */
} PT_FAIIntScanStart, *LPT_FAIIntScanStart;
typedef struct tagPT_FAIDmaStart
{
    USHORT TrigSrc;
    DWORD SampleRate;
    USHORT chan;
    USHORT gain;
    USHORT *buffer;
    ULONG count;
} PT_FAIDmaStart, * LPT_FAIDmaStart;

typedef struct tagPT_FAIDmaScanStart 
{
    USHORT TrigSrc;
    DWORD SampleRate;
    USHORT NumChans;
    USHORT StartChan;
    USHORT *GainList;
    USHORT *buffer;
    ULONG count;
} PT_FAIDmaScanStart, * LPT_FAIDmaScanStart;

typedef struct tagPT_FAIDmaExStart{
	INT16U	TrigSrc;
	INT16U	TrigMode;
	INT16U	ClockSrc;
	INT16U	TrigEdge;
	INT16U	SRCType;
	float	TrigVol;
	INT16U	CyclicMode;
	INT16U	NumChans;
	INT16U	StartChan;
	INT32U	ulDelayCnt;
	INT32U	count;
	INT32U	SampleRate;
	INT16U	*GainList;
	INT16U	*CondList;
	INT16U	*buffer0;
	INT16U	*buffer1;
	INT16U	*Pt1;
	INT16U	*Pt2;
	INT16U	*Pt3;
	/* INT32U *chanlist;	 */
}PT_FAIDmaExStart, *LPT_FAIDmaExStart;

typedef struct tagPT_FAITransfer
{
	INT16U ActiveBuf;
	VOID *DataBuffer;
	INT16U DataType;
	INT32U start;
	INT32U count;
	INT16U *overrun;
} PT_FAITransfer, *LPT_FAITransfer;


typedef struct tagPT_CheckEvent {
	INT32U EventType; /* note: not compatible with windows API now ! */
	INT32U Milliseconds;
} PT_CheckEvent, *LPT_CheckEvent;


typedef struct tagPT_EnableEvent{
	INT32U	EventType;
	INT16U	Enabled;
	INT16U	Count;
} PT_EnableEvent, *LPT_EnableEvent;


typedef struct tagPT_DIFilter {
	INT16U  usEventType;
	INT16U  usEventEnable;	/* event function switcher (0 or 1) */
	INT16U  usCount;	/* event count */

	INT16U  usEnable;	/* IDIn(0~7) digital filter enable/disable setting */
	INT16U  *usHiValue;	/* record filter high status value array pointer */
	INT16U  *usLowValue;	/* record filter low status value array pointer */
} PT_DIFilter, *LPT_DIFilter;

typedef struct tagPT_DIPattern {
	INT16U  usEventType;
	INT16U  usEventEnable;	/* event function switcher (0 or 1) */
	INT16U  usCount;	/* event count */

	INT16U  usEnable;	/* IDIn(0~7) pattern match enable/disable setting */
	INT16U  usValue;	/* pattern match preset value */
} PT_DIPattern, *LPT_DIPattern;

typedef struct tagPT_DICounter {
	INT16U  usEventType;
	INT16U  usEventEnable;	/* event function switcher (0 or 1) */
	INT16U  usCount;	/* event count */

	INT16U  usEnable;	/* IDIn(0~7) counter enable/disable setting */
	INT16U  usTrigEdge;	/* Counter Trigger edge: 0 - rising, 1 - falling */
	INT16U  usMatchEnable;	/* the value match interrupt function of IDIn(0~7)
				 * up-counter enable/disable setting. */
	INT16U  *usPreset;	/* counter previous setting value */
	INT16U  *usValue;	/* counter match value */
	INT16U  usOverflow;	/* counter overflow value */
	INT16U  usDirection;	/* up/down counter direction
				 * (not used) */
} PT_DICounter, *LPT_DICounter;

typedef struct tagPT_DIStatus {
	INT16U  usEventType;
	INT16U  usEventEnable;	/* event function switcher (0 or 1) */
	INT16U  usCount;	/* event count */

	INT16U  usEnable;	/* IDIn(0~7) status change enable/disable setting
				 * (not used) */
	INT16U  usRisingedge;	/* record rising edge trigger type */
	INT16U  usFallingedge;	/* record falling edge trigger type */
} PT_DIStatus, *LPT_DIStatus;

typedef union tagPT_EnableEventEx {
	PT_DIFilter Filter;
	PT_DIPattern Pattern;
	PT_DICounter Counter;
	PT_DIStatus Status;
} PT_EnableEventEx, *LPT_EnableEventEx;

typedef struct tagPT_FDITransfer {
	INT16U  usEventType;
	INT32U  *ulRetData;
} PT_FDITransfer, *LPT_FDITransfer;

typedef struct _adv_dio_dma_user 
{
	INT8U cyc_mode;		/* Cyclic(others) / NonCyclic(0) */
	INT8U ucDioDirection;
	INT8U ucOperationMode;
	INT8U ucStartMethod;
	INT8U ucStopMethod;
	INT8U ucPacerSource;
	INT32U dwCounterValue;
	INT32U dwDiDmaMode;
	INT32U dwDoDmaMode;
	VOID  *user_ptr;
	INT32U convert_num;
}adv_dio_dma_user;



typedef struct tagPT_FAICheck{
	INT16U	ActiveBuf;
	INT16U	stopped;
	INT32U	retrieved;
	INT16U	overrun;
	INT16U	HalfReady;
}PT_FAICheck, *LPT_FAICheck;
/* end */
typedef struct _StruDevInfo
{
	char driver_name[DRIVER_NAME_LENGTH];//char dev_name[30];
	char device_name[DEVICE_NAME_LENGTH];
	char device_node_name[DEVICE_NODE_NAME_LENGTH];
	unsigned long board_id;
	unsigned long io_base;
	unsigned long irq;
	unsigned long irq_ext;
	unsigned short slot;
	unsigned short bus;	
}StruDevInfo;

typedef struct _StruDevConfig
{
	char driver_name[DRIVER_NAME_LENGTH];
	char device_name[DEVICE_NAME_LENGTH];
	char device_node_name[DEVICE_NODE_NAME_LENGTH];
     
	//    char card_id[NAMELEN]; /* added by LI.ANG for multiple-card support */
	INT16U minor;		/* added by Zy.Xie for specify which device
				 * node bind with device */
     
	INT16U board_id;
	unsigned long iobase;
	unsigned long irq;
	unsigned long irq_ext;
	unsigned char slot;
	INT16U board_id_set;	/* Set USB device with new board id  */
}StruDevConfig;

/* typedef struct _StruDevInfo */
/* { */
/*      char dev_name[30]; */
/*      char board_id[30]; */
/*      char io_base[30]; */
/* }StruDevInfo; */

/* typedef struct _StruDevConfig */
/* { */
/*      char driv_name[NAMELEN]; */
/*      char device_name[NAMELEN];	/\* added by LI.ANG for multiple-card support *\/ */
/*      char card_id[NAMELEN]; */
/*      INT16U board_id; */
/* }StruDevConfig; */

typedef struct tagPT_DeviceGetParam
{
	INT16U  nID;
	INT32U  *Length;
	void    *pData;
}PT_DeviceGetParam, *LPT_DeviceGetParam;

typedef struct tagPT_DeviceSetParam
{
	INT16U  nID;
	INT32U   Length;
	void    *pData;
	INT32U   Reserved[4];
	void    *pReserved[4];
}PT_DeviceSetParam, *LPT_DeviceSetParam;

typedef struct tagPT_AIConfig{
	INT16U	DasChan;
	INT16U	DasGain;
} PT_AIConfig, *LPT_AIConfig;


typedef struct tagDEVFEATURES
{
	
/* #ifndef _WIN32_WCE */
/*     CHAR     szDriverVer[8];    // device driver version */
/*     CHAR     szDriverName[MAX_DRIVER_NAME_LEN];  // device driver name */
/* #else */
/*     TCHAR     szDriverVer[8];    // device driver version */
/*     TCHAR     szDriverName[MAX_DRIVER_NAME_LEN];  // device driver name */
/* #endif */

    DWORD    dwBoardID;         // board ID
    USHORT   usMaxAIDiffChl;    // Max. number of differential channel
    USHORT   usMaxAISiglChl;    // Max. number of single-end channel
    USHORT   usMaxAOChl;        // Max. number of D/A channel
    USHORT   usMaxDOChl;        // Max. number of digital out channel
    USHORT   usMaxDIChl;        // Max. number of digital input channel
    USHORT   usDIOPort;         // specifies if programmable or not
    USHORT   usMaxTimerChl;     // Max. number of Counter/Timer channel
    USHORT   usMaxAlarmChl;     // Max number of  alram channel
    USHORT   usNumADBit;        // number of bits for A/D converter
    USHORT   usNumADByte;       // A/D channel width in bytes.
    USHORT   usNumDABit;        // number of bits for D/A converter.
    USHORT   usNumDAByte;       // D/A channel width in bytes.
    USHORT   usNumGain ;        // Max. number of gain code
    GAINLIST glGainList[16];    // Gain listing
    DWORD    dwPermutation[4];  // Permutation
} DEVFEATURES, *LPDEVFEATURES;


typedef struct tagPT_DeviceGetFeatures
{
    LPDEVFEATURES  buffer;
    USHORT         size;
} PT_DeviceGetFeatures, *LPT_DeviceGetFeatures;

typedef struct tagPT_QCounterConfigSys
{
    USHORT      SysClock;
    USHORT      TimeBase;
    USHORT      TimeDivider;
    USHORT      CascadeMode;
} PT_QCounterConfigSys,  * LPT_QCounterConfigSys;
typedef struct tagPT_QCounterConfig
{
    USHORT      counter;
    USHORT      LatchSrc;
    USHORT      LatchOverflow;
    USHORT      ResetOnLatch;
    USHORT      ResetValue;
} PT_QCounterConfig,  * LPT_QCounterConfig;
typedef struct tagPT_QCounterStart
{
    USHORT      counter;
    USHORT      InputMode;
} PT_QCounterStart,  * LPT_QCounterStart;

typedef struct tagPT_QCounterRead
{
    USHORT      counter;
    USHORT   *overflow;
    INT32U    *LoCount;
    INT32U    *HiCount;
} PT_QCounterRead,  * LPT_QCounterRead;


/* typedef struct tagPT_AIGetConfig */
/* { */
/*     LPDEVCONFIG_AI buffer; */
/*     INT16U         size; */
/* } PT_AIGetConfig, * LPT_AIGetConfig; */

typedef struct tagPT_AIBinaryIn
{
	INT16U      chan;
	INT16U      TrigMode;
	INT16U      *reading;
} PT_AIBinaryIn, * LPT_AIBinaryIn;

typedef struct tagPT_AIScale
{
	INT16U      reading;
	float       MaxVolt;
	INT16U      MaxCount;
	INT16U      offset;
	float       *voltage;
} PT_AIScale, * LPT_AIScale;

typedef struct tagPT_AIVoltageIn
{
	INT16U      chan;
	INT16U      gain;
	INT16U      TrigMode;
	float       *voltage;
} PT_AIVoltageIn, * LPT_AIVoltageIn;

typedef struct tagPT_AICurrentIn
{
	INT16U      chan;
	INT16U      gain;
	INT16U      TrigMode;
	float       *Current;
} PT_AICurrentIn, * LPT_AICurrentIn;


typedef struct tagPT_MAIConfig
{
	INT16U      NumChan;
	INT16U      StartChan;
	INT16U      *GainArray;
} PT_MAIConfig, * LPT_MAIConfig;

typedef struct tagPT_MAIBinaryIn
{
	INT16U      NumChan;
	INT16U      StartChan;
	INT16U      TrigMode;
	INT16U      *ReadingArray;
} PT_MAIBinaryIn, * LPT_MAIBinaryIn;

typedef struct tagPT_MAIVoltageIn
{
	INT16U      NumChan;
	INT16U      StartChan;
	INT16U      *GainArray;
	INT16U      TrigMode;
	float       *VoltageArray;
} PT_MAIVoltageIn, * LPT_MAIVoltageIn;

typedef struct tagPT_MAICurrentIn
{
	INT16U      NumChan;
	INT16U      StartChan;
	INT16U      *GainArray;
	INT16U      TrigMode;
	float       *CurrentArray;
} PT_MAICurrentIn, * LPT_MAICurrentIn;


typedef struct tagPT_ReadPortByte
{
	INT16U    port;		/* Input */
	INT16U    *ByteData;	/* Output */
} PT_ReadPortByte, *LPT_ReadPortByte;

typedef struct tagPT_WritePortByte
{
	INT16U    port;		/* Input */
	INT16U    ByteData;	/* Input */
} PT_WritePortByte, * LPT_WritePortByte;

typedef struct tagPT_ReadPortWord
{
	INT16U    port;		/* Input */
	INT16U    *WordData;	/* Output */
} PT_ReadPortWord, * LPT_ReadPortWord;

typedef struct tagPT_WritePortWord
{
	INT16U    port;		/* Input */
	INT16U    WordData;	/* Input */
} PT_WritePortWord, * LPT_WritePortWord;

typedef struct tagPT_ReadPortDword
{
	INT16U    port;		/* Input */
	INT32U    *DWordData;	/* Output */
} PT_ReadPortDword, * LPT_ReadPortDword;

typedef struct tagPT_WritePortDword
{
	INT16U    port;		/* Input */
	INT32U    DWordData;	/* Input */
} PT_WritePortDword, * LPT_WritePortDword;

typedef struct tagPT_PortOperation
{
	long    PortType;
	INT32U   PortNumber;
	void   *DataBuffer;
	INT32U   DataLength;
	INT32U   ReturnedLength;
} PT_PortOperation, * LPT_PortOperation;
     
     
/* add Zy.Xie */
typedef struct tagPT_TimerCountSeting
{
	INT16U counter;
	INT32U Count;
}PT_TimerCountSetting, *LPT_TimerCountSetting;


typedef struct tagPT_CounterFreqRead
{
	INT16U counter;
	float *freq;
}PT_CounterFreqRead, *LPT_CounterFreqRead;

typedef struct tagPT_CounterFreqStart
{
	INT16U counter;
	INT16U GatePeriod;
	INT16U GateMode;
}PT_CounterFreqStart, *LPT_CounterFreqStart;

typedef struct tagPT_PWMStartRead
{
	INT16U usChan;
	FP32   *flHiperiod;
	FP32   *flLowperiod;
} PT_PWMStartRead, *LPT_PWMStartRead;


typedef struct tagPT_CounterEventRead
{
	INT16U counter;
	INT16U *overflow;
	INT32U *count;
}PT_CounterEventRead, *LPT_CounterEventRead;

typedef struct tagPT_CounterEventStart
{
	INT16U counter;
	INT16U GateMode;
}PT_CounterEventStart, *LPT_CounterEventStart;

typedef struct tagPT_CounterPulseStart
{
	INT16U counter;
	float period;
	float UpCycle;
	INT16U GateMode;
	INT16U IsCascadeMode;
}PT_CounterPulseStart, *LPT_CounterPulseStart;

typedef struct tagPT_CounterPWMSetting {

	INT16U     Port;
	float      Period;
	float      HiPeriod;
	INT32U     OutCount;
	INT16U     GateMode;
	
} PT_CounterPWMSetting, *LPT_CounterPWMSetting;
/* add Zy.xie end */

typedef struct tagPT_FreqOutStart {
	INT16U usChannel;
	INT16U usDivider;
	INT16U usFoutSrc;
} PT_FreqOutStart, *LPT_FreqOutStart;

typedef struct tagPT_CounterConfig {
	INT16U usCounter;
	INT16U usInitValue;
	INT16U usCountMode;
	INT16U usCountDirect;
	INT16U usCountEdge;
	INT16U usOutputEnable;
	INT16U usOutputMode;
	INT16U usClkSrc;
	INT16U usGateSrc;
	INT16U usGatePolarity;
} PT_CounterConfig, *LPT_CounterConfig;


typedef struct tagPWMValidRange {
	FP32 MinPeriod;
	FP32 MaxPeriod;
	FP32 MinHighPeriod;
	FP32 MaxHighPeriod;	
} PWMValidRange, *LPPWMValidRange;

	
typedef struct tagPT_WatchdogStart {
	INT32S Reserve0;
	INT32S Reserve1;
} PT_WatchdogStart, *LPT_WatchdogStart;
	

typedef struct tagPT_TCMuxRead 
{
	INT16U DasChan;
	INT16U DasGain;
	INT16U ExpChan;
	INT16U TCType;
	INT16U TempScale;
	FP32   *temp;
} PT_TCMuxRead, LPT_TCMuxRead;

/* for property */
typedef struct tagDiPortValue
{
	INT32U port;
	INT32U value;
} DiPortValue, *LPDiPortValue;

typedef struct tagDiEnableChannel
{
	INT32U port;
	INT32U status;
}DiEnableChannel, *LPDiEnableChannel;


/* end */
#endif
