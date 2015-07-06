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
	unsigned int buffer;
	int length;
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
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}
	
	/* step 2: set property */
	buffer = 0x0101; /* set interrupt trig edge (1: enable, 0: disable) */
	ret = DRV_DeviceSetProperty(fd, CFG_DiInterruptTriggerOnRisingEdge,
				    &buffer, sizeof(unsigned int));
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}
	

	buffer = 0x0101; /* set di interrupt mode
			  * 0: disable
			  * 1: enable with no gate
			  * 2: enable with gate */
	ret = DRV_DeviceSetProperty(fd, CFG_InterruptTriggerSource,
				    &buffer, sizeof(unsigned int));
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	

	/* step 3: enable event */
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
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
	

	ret = DRV_DioEnableEventAndSpecifyDiPorts(fd, ADS_EVT_DI_INTERRUPT0, 0, 2);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	ret = DRV_DioEnableEventAndSpecifyDiPorts(fd, ADS_EVT_DI_INTERRUPT8, 0, 2);
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
		check_event.Milliseconds = 1000; /* wait for one second */
	     
		ret = DRV_CheckEvent(fd, &check_event);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			continue;
		}
		
		switch (check_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0:
		{
			int di_buf = 0;
			
			printf("DI0 interrupt!\n");
			ret = DRV_DioGetLatestEventDiPortsState(fd, ADS_EVT_DI_INTERRUPT0,
								&di_buf, 2);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);

				DRV_DeviceClose(&fd);
				return -1;
			}
			printf("di port: 0x%x\n", di_buf);
		}
		break;
		case ADS_EVT_DI_INTERRUPT8:
		{
			int di_buf = 0;
			printf("DI8 interrupt!\n");
			ret = DRV_DioGetLatestEventDiPortsState(fd, ADS_EVT_DI_INTERRUPT8,
								&di_buf, 2);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);

				DRV_DeviceClose(&fd);
				return -1;
			}
			printf("di port: 0x%x\n", di_buf);
		}
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

	ret = DRV_DioDisableEvent(fd, ADS_EVT_DI_INTERRUPT0);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}
	
	ret = DRV_DioDisableEvent(fd, ADS_EVT_DI_INTERRUPT8);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	/* step 4: disable event */
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	enable_event.EventType = ADS_EVT_DI_INTERRUPT0; /* you can choose */
	enable_event.Enabled = 0;

	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	enable_event.EventType = ADS_EVT_DI_INTERRUPT8; /* you can choose */
	enable_event.Enabled = 0;

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
