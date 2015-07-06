/*
 * *****************************************************************
 * Program     : di_int.c
 * Description : Demo program for digital input with interrupt mode
 *               function
 * Boards supp.:
 * APIs used   : DRV_DeviceOpen, DRV_DeviceClose
 *               DRV_EnableEvent
 *               DRV_CheckEvent
 *               
 * Revision    : 1.00
 * Date        : 11/29/2006
 * Author      : LI.ANG
 * *****************************************************************
 * */
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
	unsigned short buffer;
	char err_msg[100];
	int tmp;
	PTR_T fd;


	if (argc != 2) {
		printf("\nUsage: digin <device node>\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}

	/* step 1: open device */
	tmp = DRV_DeviceOpen(argv[1], &fd);
	if (!fd) {
		return -1;
	}
	
	/* step 2: enable event */
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	enable_event.EventType = ADS_EVT_DI_INTERRUPT0; /* you can choose */
	enable_event.Enabled = 1;
	enable_event.Count = 1;
		
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		DRV_DeviceClose(fd);
		return -1;
	}
	
        /* step 3: check event */
	printf("Waitting for Event, Sleepping... (Ctrl-C to out of loop)\n");     
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break;

		
		check_event.EventType = 0; /* 0 - check all */
		check_event.Milliseconds = 100000;
	     
		tmp = DRV_CheckEvent(fd, &check_event);
		if (tmp < 0) {
			DRV_GetErrorMessage(tmp, err_msg);
			printf("err msg: %s\n", err_msg);

			continue;
		}
		/* printf("event type = 0x%.4x\n", check_event.EventType); */
		
		switch (check_event.EventType) {
		case ADS_EVT_TERMINATE_CNT0:
			printf("CNT0 interrupt!\n");
			break;
		case ADS_EVT_TERMINATE_CNT1:
			printf("CNT1 interrupt!\n");
			break;
		case ADS_EVT_TERMINATE_CNT2:
			printf("CNT2 interrupt!\n");
			break;
		case ADS_EVT_TERMINATE_CNT3:
			printf("CNT3 interrupt!\n");
			break;
		case ADS_EVT_TERMINATE_CNT4:
			printf("CNT4 interrupt!\n");
			break;
		case ADS_EVT_TERMINATE_CNT5:
			printf("CNT5 interrupt!\n");
			break;
		case ADS_EVT_TERMINATE_CNT6:
			printf("CNT6 interrupt!\n");
			break;
		case ADS_EVT_TERMINATE_CNT7:
			printf("CNT7 interrupt!\n");
			break;
		case ADS_EVT_DI_INTERRUPT0:
			printf("DI0 interrupt!\n");
			break;
		}	     
		count++;
		gettimeofday(&now, NULL);

		time = (now.tv_sec - prev.tv_sec) + (now.tv_usec - prev.tv_usec) / (double) 1000000;
		if ((time >= 0.5) && (stop_flag == 0)) {
			freq = (double) enable_event.Count * count / time;
			printf("frequency is %f, count: %d, time: %f\n", freq, count, time);
			count = 0;
			prev = now;
		}
	}

	/* step 4: disable event */
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	enable_event.EventType = ADS_EVT_DI_INTERRUPT0; /* you can choose */
	enable_event.Enabled = 0;

	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		DRV_DeviceClose(&fd);
		return -1;
	}

	/* step 5: close device */
	DRV_DeviceClose(&fd);

	return 0;
} 
