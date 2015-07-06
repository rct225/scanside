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
//	Description: 	Demo program for input with interrupt mode function                |
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
#include <signal.h>
#include <Advantech/advdevice.h>

#define MAX_GROUPS_NUM 8
#define MAX_DIO_PORT_NUM 24
static INT16U event_support[] = { ADS_EVT_DI_INTERRUPT0,
				  ADS_EVT_DI_INTERRUPT1,  
				  ADS_EVT_DI_INTERRUPT8,  
				  ADS_EVT_DI_INTERRUPT16,
				  ADS_EVT_DI_INTERRUPT40,    
				  ADS_EVT_DI_INTERRUPT64,
				  ADS_EVT_DI_INTERRUPT88,
				  ADS_EVT_DI_INTERRUPT112,
				  ADS_EVT_DI_INTERRUPT136,
				  ADS_EVT_DI_INTERRUPT160,
				  ADS_EVT_DI_INTERRUPT184,		  
				  0 };

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}


int main(int argc, char *argv[])
{
	PT_EnableEvent enable_event;
	PT_CheckEvent check_event;
	struct timeval prev, now;
	double time, freq;
	unsigned int count;

	char *filename;
	char err_msg[100];
	int ret;
	int fd;
	int groups_int[MAX_GROUPS_NUM];
	int i;
	

	if (argc != 2) {
		printf("\nUsage: di_int <device node>\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}
	filename = argv[1]; 


	/* step 1: open device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}
	
	/* step 2: set property */
	if ( 1 ) {
		/* For that device which DIO's direction can be set by SW */
		INT8U dir[MAX_DIO_PORT_NUM];
		
		for (i = 0; i < MAX_DIO_PORT_NUM; i++)
			dir[i] = DIO_ChannelDir_DI;
		ret = DRV_DeviceSetProperty(fd, CFG_DioChannelDirection, &dir, MAX_DIO_PORT_NUM*sizeof(INT8U));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
	}

	/* set tirgger source */
	for (i = 0; i < MAX_GROUPS_NUM; i++) {
		groups_int[i] = 0x01;
	}

	ret = DRV_DeviceSetProperty(fd, CFG_InterruptTriggerSource,
				    groups_int, sizeof(groups_int));
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	/* set interrupt trigger edge (1: rising edge, 0: falling edge) */
	if (0) {
		unsigned int buffer;
		buffer = 0x0001; 
		ret = DRV_DeviceSetProperty(fd, CFG_DiInterruptTriggerOnRisingEdge,
					    &buffer, sizeof(unsigned int));
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
			printf("event_supprt[%d] is not supported by this device\n", i);
			
			DRV_DeviceClose(&fd);
			return -1;
		}
		
		i++;
	}
	
	
        /* step 4: check event */
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
		case ADS_EVT_DI_INTERRUPT64:
			printf("DI64 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT88:
			printf("DI88 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT112:
			printf("DI112 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT136:
			printf("DI136 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT160:
			printf("DI160 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT184:
			printf("DI184 interrupt!\n");
			break;

		case ADS_EVT_DEVREMOVED:
			printf("Device removed!\n");
			break;
		default:
			printf("NO Event!\n");
			break;
		}	     

		count++;
		gettimeofday(&now, NULL);

		time = (now.tv_sec - prev.tv_sec) + (now.tv_usec - prev.tv_usec)
			/ (double) 1000000;
		if (time >= 0.5) {
			freq = (double) enable_event.Count * count / time;
			printf("frequency is %f, count: %d, time: %f\n", freq, count, time);
			count = 0;
			prev = now;
		}
	}

	
 out:	/* step 4: disable event */
	i = 0;
	while (event_support[i] != 0) {
		memset(&enable_event, 0, sizeof(PT_EnableEvent));
		enable_event.EventType = event_support[i]; /* you can choose */
		enable_event.Enabled = 0;

		ret = DRV_EnableEvent(fd, &enable_event);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			printf("event_supprt[%d] is not supported by this device\n", i);
		}
		i++;
	}
	
	/* step 5: close device */
	DRV_DeviceClose(&fd);

	return 0;
}
