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
				  ADS_EVT_DI_PATTERNMATCH_PORT12,
				  ADS_EVT_DI_STATUSCHANGE_PORT1,
				  ADS_EVT_DI_STATUSCHANGE_PORT13,
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
	int fd;
	int i;
	
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
		printf("Device Open Failed\n");
		return -1;
	}
	
	/* step 2: set property */
	if (1) {
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
		
		ret = DRV_DeviceSetProperty(fd, CFG_DiPatternMatchEnabledChannel, &di_enable_chan, sizeof(DiEnableChannel));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
		ret = DRV_DeviceSetProperty(fd, CFG_DiPatternMatchValue, &di_port_value, sizeof(DiPortValue));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
		/* 2. Di status change */
		printf("DI status change:\n");
		printf("DiEnableChannel.port =  ");
		scanf("%d", &di_enable_chan.port);
		printf("\nDiEnableChannel.value = 0x");
		scanf("%x", &di_enable_chan.status);
		
		ret = DRV_DeviceSetProperty(fd, CFG_DiStatusChangeEnabledChannel, &di_enable_chan, sizeof(DiEnableChannel));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
	}
	

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
		case ADS_EVT_DI_PATTERNMATCH_PORT12:
			printf("PORT12 pattern match occured!\n");
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT1:
			printf("PORT1 status change occured!\n");
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT13:
			printf("PORT13 status change occured!\n");
			break;
		default:
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

