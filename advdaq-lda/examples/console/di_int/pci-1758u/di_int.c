//-----------------------------------------------------------------------------------------|
//                                                                                         |
//                            Advantech eAutomation Corp                                   |
//                                                                                         | 
//                       Copyright @2005-2007,  All Rights Reserved                        |
//                                                                                         |
//                              Web: www.advantech.com                                     |       
//-----------------------------------------------------------------------------------------|

//-----------------------------------------------------------------------------------------|
//                                  The Examples for DAQ                                   |
//	Filename   :	di_int.c                                                           |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for digital input with interrupt mode                 |
//               	function                                                           | 
//                                                                                         | 
//	This code and information is provided "as is" without warranty of any              |
//	kind, either expressed or implied, including but not limited to the implied,       |
//	warranties of merchantability and/or fitness for a particular purpose.             |
//                                                                                         |
//-----------------------------------------------------------------------------------------|
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
#include <Advantech/advdevice.h>

#define DI_PORT_NUM    8
#define DO_PORT_NUM    8

/* for test */
#include <signal.h>

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}
/* end */

int main(int argc, char *argv[])
{
	PT_EnableEvent enable_event;
	PT_CheckEvent check_event;
	struct timeval prev, now;
	double time, freq;
	unsigned int count;
	unsigned short i;
	unsigned char buffer[DI_PORT_NUM];
	int tmp;
	int len;
	char *filename;
	char err_msg[100];
	int ret;
	int fd;
	

	if (argc != 2) {
		printf("\nUsage: di_int <device node>\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}
	filename = argv[1]; 


	/* step 1: open device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret < 0) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}
	

	
	/* step 2: set interrupt trig edge (1: enable, 0: disable) */
	for (i = 0; i < DI_PORT_NUM; i++) {
		buffer[i] = 0x01;
	}
	ret = DRV_DeviceSetProperty(fd, CFG_DiInterruptTriggerOnRisingEdge,
				    buffer, DI_PORT_NUM);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	for (i = 0; i < DI_PORT_NUM; i++) {
		buffer[i] = 0xff;
	}
	ret = DRV_DeviceSetProperty(fd, CFG_DiInterruptTriggerOnFallingEdge,
				    buffer, DI_PORT_NUM);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

/* 	ret = DRV_DeviceSetProperty(fd, CFG_DoWatchdogValue, buffer, DO_PORT_NUM); */
/* 	if (ret) { */
/* 		DRV_GetErrorMessage(ret, err_msg); */
/* 		printf("err msg: %s\n", err_msg); */
/* 		DRV_DeviceClose(&fd); */
/* 		return -1; */
/* 	} */

/* 	tmp = 300; */
/* 	ret = DRV_DeviceSetProperty(fd, CFG_DiFilterIntervalCounter, &tmp, 4); */
/* 	if (ret) { */
/* 		DRV_GetErrorMessage(ret, err_msg); */
/* 		printf("err msg: %s\n", err_msg); */
/* 		DRV_DeviceClose(&fd); */
/* 		return -1; */
/* 	} */

/* 	tmp = 0; */
/* 	len = sizeof(tmp); */
/* 	for (i = 0; i < DO_PORT_NUM; i++) { */
/* 		buffer[i] = 0x22; */
/* 	} */
/* 	ret = DRV_DeviceGetProperty(fd, CFG_DiFilterIntervalCounter, &tmp, &len); */
/* 	if (ret) { */
/* 		DRV_GetErrorMessage(ret, err_msg); */
/* 		printf("err msg: %s\n", err_msg); */
/* 		DRV_DeviceClose(&fd); */
/* 		return -1; */
/* 	} */
/* 	printf("tmp: %d\n", tmp); */
/* 	printf("len: %d\n", len); */
/* 	for (i = 0; i < DO_PORT_NUM; i++) { */
/* 		printf("buf[%d]: 0x%x\n", i, buffer[i]); */
/* 	} */

/* 	for (i = 0; i < DI_PORT_NUM; i++) { */
/* 		buffer[i] = 0xaa; */
/* 	} */
/* 	ret = DRV_DeviceGetProperty(fd, CFG_DiInterruptTriggerOnFallingEdge, buffer, &len); */
/* 	if (ret) { */
/* 		DRV_GetErrorMessage(ret, err_msg); */
/* 		printf("err msg: %s\n", err_msg); */
/* 		DRV_DeviceClose(&fd); */
/* 		return -1; */
/* 	} */
/* 	for (i = 0; i < DI_PORT_NUM; i++) { */
/* 		printf("falling buf[%d]: 0x%x\n", i, buffer[i]); */
/* 	} */
/* 	printf("len: %d\n", len); */
	

/* 	/\* set filter function (1: enable, 0: disable) *\/ */
/* 	for (i = 0; i < DI_PORT_NUM; i++) { */
/* 		buffer[i] = 0xff; */
/* 	} */
/* 	ret = DRV_DeviceSetProperty(fd, CFG_DiFilterEnableForAll, */
/* 				    buffer, DI_PORT_NUM); */
/* 	if (ret) { */
/* 		DRV_GetErrorMessage(ret, err_msg); */
/* 		printf("err msg: %s\n", err_msg); */
/* 		DRV_DeviceClose(&fd); */
/* 		return -1; */
/* 	} */

/* 	/\* set filter interval *\/ */
/* 	count = 0x2625a0;		/\* 500ms(1Hz) *\/ */
/* 	ret = DRV_DeviceSetProperty(fd, CFG_DiFilterIntervalCounter, */
/* 				    &count, sizeof(count)); */
/* 	if (ret) { */
/* 		DRV_GetErrorMessage(ret, err_msg); */
/* 		printf("err msg: %s\n", err_msg); */
/* 		DRV_DeviceClose(&fd); */
/* 		return -1; */
/* 	} */

	/* step 3: enable event */
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	enable_event.EventType = ADS_EVT_TIME_OUT; /* you can choose */
	enable_event.Enabled = 1;
	enable_event.Count = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	enable_event.EventType = ADS_EVT_DI_INTERRUPT0; /* you can choose */
	enable_event.Enabled = 1;
	enable_event.Count = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	enable_event.EventType = ADS_EVT_DI_INTERRUPT8; /* you can choose */
	enable_event.Enabled = 1;
	enable_event.Count = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	enable_event.EventType = ADS_EVT_DI_INTERRUPT16; /* you can choose */
	enable_event.Enabled = 1;
	enable_event.Count = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

        /* step 4: check event */
	printf("Waitting for Event, Sleepping... (Ctrl-C to stop)\n");     
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break;

		check_event.EventType = 0; /* 0 - check all */
		check_event.Milliseconds = 3000;
	     
		ret = DRV_CheckEvent(fd, &check_event);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			continue;
		}
		
		switch (check_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			printf("DI0 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT8:
			printf("DI8 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT16:
			printf("DI16 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT24:
			printf("DI24 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT32:
			printf("DI32 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT40:
			printf("DI40 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT48:
			printf("DI48 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT56:
			printf("DI56 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT63:
			printf("DI63 interrupt!\n");
			break;
		case ADS_EVT_TIME_OUT:
			printf("Check event timeout!\n");
			break;
		default:
			break;
		}
	}

	
	/* step 4: disable event */
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	enable_event.EventType = ADS_EVT_TIME_OUT; /* you can choose */
	enable_event.Enabled = 0;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret < 0) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	enable_event.EventType = ADS_EVT_DI_INTERRUPT0; /* you can choose */
	enable_event.Enabled = 0;
	enable_event.Count = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	enable_event.EventType = ADS_EVT_DI_INTERRUPT8; /* you can choose */
	enable_event.Enabled = 0;
	enable_event.Count = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	enable_event.EventType = ADS_EVT_DI_INTERRUPT16; /* you can choose */
	enable_event.Enabled = 0;
	enable_event.Count = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}


	/* step 5: close device */
	DRV_DeviceClose(&fd);

	return 0;
}
