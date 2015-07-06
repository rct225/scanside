//---------------------------------------------------------------------------+
//                                                                           |
//                            Advantech eAutomation Corp                     |
//                                                                           | 
//                       Copyright @2005-2007,  All Rights Reserved          |
//                                                                           |
//                              Web: www.advantech.com                       |
//---------------------------------------------------------------------------+

//---------------------------------------------------------------------------|
//                                  The Examples for DAQ                     |
//	Filename   :	di_pattn.c                                           |
//	Version	   : 	1.0.0                                                |
//	Date	   :	11/17/2007                                           |
//                                                                           | 
//	Description: 	Demo program for DI pattern match and status change  |
//                      function                                             |
//                                                                           |
//	This code and information is provided "as is" without warranty of any|
//	kind, either expressed or implied, including but not limited to the  |
//      implied, warranties of merchantability and/or fitness for a          |
//      particular purpose.                                                  |
//                                                                           |
//---------------------------------------------------------------------------|

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
#include <termios.h> 
#include <signal.h>
#include <Advantech/advdevice.h>

static INT16U event_support[] = { ADS_EVT_DI_PATTERNMATCH_PORT0,
/* 				  ADS_EVT_DI_STATUSCHANGE_PORT0, */
/* 				  ADS_EVT_COUNTERMATCH, */
/* 				  ADS_EVT_COUNTEROVERFLOW, */
				  0 };

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}
 
int main(int argc, char *argv[])
{
	PT_CheckEvent check_event;
	PT_EnableEvent enable_event;
	DiEnableChannel di_enable_chan;
	DiPortValue     di_port_value;
	unsigned int ret;
	char err_msg[100];
	INT32U hi_val[8];
	INT32U low_val[8];
	INT32U cnt_te;
	INT32U cnt_match;
	INT32U cnt_preset[8];
	INT32U cnt_value[8];
	INT32U cnt_overflow;
	INT32U cnt_trigger;
	INT32U counter_enable;
	INT8U  filter_enable;
	INT32U di_status;
	int fd;
	int i;

	for (i = 0; i < 8; ++i) {
		hi_val[i] = 1;
		low_val[i] = 1;
		cnt_preset[i] = 300;
		cnt_value[i] = 1000;
		
	}
	cnt_match = 0xff;
	counter_enable = 0x80;
	cnt_te = 0xff;
	cnt_overflow = 0xff;
	cnt_trigger = 0xff;
	filter_enable = 0x80;

	if (argc != 2) {
		printf("\nUsage: di_pattn <device node>\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}

        /* step 1: open device */
	ret = DRV_DeviceOpen((char *)argv[1], &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}
	
	/* step 2: set property */
#if 1
		/*1. pattern match */
		printf("DI pattern match function:\n");
		printf("DiEnableChannel.port =  ");
		scanf("%d", &di_enable_chan.port);
		printf("\nDiEnableChannel.status = 0x");
		scanf("%x", &di_enable_chan.status);
		printf("\nDiPortValue.port= %d\n", di_enable_chan.port);
		printf("DiPortValue.value =  0x");
		scanf("%x", &di_port_value.value);
		
		di_port_value.port = di_enable_chan.port;
		
		ret = DRV_DeviceSetProperty(fd, CFG_DiPatternMatchEnabledChannel,
					    &di_enable_chan, sizeof(DiEnableChannel));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

		ret = DRV_DeviceSetProperty(fd, CFG_DiPatternMatchValue,
					    &di_port_value, sizeof(DiPortValue));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

		/* 2. Di status change */
             	di_status = 0x80;
		ret = DRV_DeviceSetProperty(fd, CFG_DiStatusChangeOnRisingEdge, &di_status, sizeof(INT32U));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

		ret = DRV_DeviceSetProperty(fd, CFG_DiFilterEnable, &filter_enable, sizeof(INT8U));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}


		ret = DRV_DeviceSetProperty(fd, CFG_DiFilterLowValue, low_val, sizeof(low_val));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			return -1;
		}

		ret = DRV_DeviceSetProperty(fd, CFG_DiFilterHiValue, hi_val, sizeof(hi_val));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
		

		ret = DRV_DeviceSetProperty(fd, CFG_DiCounterEnable, &counter_enable, sizeof(INT32U));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

		ret = DRV_DeviceSetProperty(fd, CFG_DiCounterTrigEdge, &cnt_trigger, sizeof(INT32U));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
		ret = DRV_DeviceSetProperty(fd, CFG_DiCounterMatchEnable, &cnt_match, sizeof(INT8U));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

		ret = DRV_DeviceSetProperty(fd, CFG_DiCounterPreset, cnt_preset, sizeof(cnt_preset));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

		ret = DRV_DeviceSetProperty(fd, CFG_DiCounterValue, &cnt_value, sizeof(cnt_value));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

		ret = DRV_DeviceSetProperty(fd, CFG_DiCounterOverflow, &cnt_overflow, sizeof(INT32U));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

#endif	
		
        /* step 3: enable event */
	i = 0;
	while (event_support[i] != 0) {
		memset(&enable_event, 0, sizeof(PT_EnableEvent));
		enable_event.EventType = event_support[i]; /* you can choose */
		enable_event.Enabled = 1;
		enable_event.Count = 1;
		
		ret = DRV_EnableEvent(fd, &enable_event);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
		i++;
	}
	
	/* step 4: check event */
	memset(&check_event, 0, sizeof(PT_CheckEvent));

	printf("Waitting for Event, Sleepping... (Ctrl-C to stop)\n");     
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break;
			     
		check_event.EventType = 0; /* 0 - check all */
		check_event.Milliseconds = 1000; /* wait for one second */
		ret = DRV_CheckEvent(fd, &check_event);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			continue;
		}
		
		switch(check_event.EventType) {
		case ADS_EVT_DI_PATTERNMATCH_PORT0:
			printf("PORT0 pattern match occured!\n");
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT0:
			printf("PORT0 status change occured!\n");
			break;
		case ADS_EVT_COUNTERMATCH:
			printf("counter match occured!\n");
			break;
		case ADS_EVT_COUNTEROVERFLOW:
			printf("counter overflow occured!\n");
			break;

		}

	}

        /* step 3: enable event */
	i = 0;
	while (event_support[i] != 0) {
		memset(&enable_event, 0, sizeof(PT_EnableEvent));
		enable_event.EventType = event_support[i]; /* you can choose */
		enable_event.Enabled = 0;
		enable_event.Count = 1;
		
		ret = DRV_EnableEvent(fd, &enable_event);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
		i++;
	}

	/* step 4: close device */
	DRV_DeviceClose(&fd);

	return 0;
} 

