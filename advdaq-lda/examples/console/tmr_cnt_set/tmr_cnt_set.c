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
//	Filename   :	tmr_cnt_set.c                                                      |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for timer count setting function                      |
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
#include <termios.h>
#include <signal.h>
#include <Advantech/advdevice.h>


#define MAX_GROUPS_NUM 3
#define MAX_DIO_PORT_NUM 9
#define MAX_COUNTER_NUM 3
static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}

int main(int argc, char *argv[])
{
	PTR_T fd;
	char err_msg[100];
	unsigned char *buffer;
	int i, ret;
	INT8U dir[MAX_DIO_PORT_NUM];
	INT32U trigger[MAX_GROUPS_NUM];
	PT_EnableEvent enable_event;
	PT_CheckEvent check_event;
	PT_TimerCountSetting tmr_count_setting;
	unsigned int cascademode;
	
	if(argc != 2) {
		printf("\nUsage: tmr_cnt_set <device node>\n");
		exit(-1);
	}
 
	
	/* step 1: open device */
	ret = DRV_DeviceOpen(argv[1], &fd);
	if(ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}
	
	/* step 2: set device's property */
	for (i = 0; i < MAX_GROUPS_NUM; i++) {
		trigger[i] = 0x03;
	}
	
	ret = DRV_DeviceSetProperty(fd, CFG_InterruptTriggerSource, trigger, MAX_GROUPS_NUM*sizeof(INT32U));
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	/* some device can set counter 0 and counter 1 with cascade mode */
	if ( 0 ) {
		cascademode = 1;
		ret = DRV_DeviceSetProperty(fd, CFG_CascadeMode, &cascademode , sizeof(INT32U));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
	}
	
	for (i = 0; i < MAX_COUNTER_NUM; i++) {
		tmr_count_setting.counter = i;
		tmr_count_setting.Count = 0x3e803e8;
		ret = DRV_TimerCountSetting(fd, &tmr_count_setting);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
	}
	
	/* step 3: Enable Event */
	enable_event.EventType = ADS_EVT_INTERRUPT_TIMER1;
	enable_event.Enabled = 1;
	enable_event.Count = 1;

	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
	}

	enable_event.EventType = ADS_EVT_TERMINATE_CNT2;
	enable_event.Enabled = 1;
	enable_event.Count = 1;

	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
	}

	/* step 4: wait event */
	printf("Waitting for Event, Sleepping... (Ctrl-C to stop)\n");     
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break; 
		check_event.EventType = 0;
		check_event.Milliseconds = 1000;
		ret = DRV_CheckEvent(fd, &check_event);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			break;
		}
		switch(check_event.EventType)
		{
		case ADS_EVT_INTERRUPT_TIMER1:
			printf("ADS_EVT_INTERRUPT_TIMER1 Interrupt \n");
			break;
		case ADS_EVT_TERMINATE_CNT0:
			printf("ADS_EVT_TERMINATE_CNT0 Interrupt \n");
			break;
		case ADS_EVT_TERMINATE_CNT1:
			printf("ADS_EVT_TERMINATE_CNT1 Interrupt \n");
			break;
		case ADS_EVT_TERMINATE_CNT2:
			printf("ADS_EVT_TERMINATE_CNT2 Interrupt \n");
			break;
		case ADS_EVT_TIME_OUT:
			printf("TimeOut\n");
			break;
		default:
			//printf("TimeOut\n");
			break;
		}
	}

	/* step 4: disable event */
	enable_event.EventType = ADS_EVT_TERMINATE_CNT2;
	enable_event.Enabled = 0;
	enable_event.Count = 1;

	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
	}

	enable_event.EventType = ADS_EVT_INTERRUPT_TIMER1;
	enable_event.Enabled = 0;
	enable_event.Count = 1;

	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
	}


	/* step 5: close device */

	DRV_DeviceClose(&fd);

	return 0;
} 

