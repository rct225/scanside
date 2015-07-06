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
//	Filename   :	counter.c                                                          |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for counter event counting                            |
//                      function                                                           |
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
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/mman.h>
#include <Advantech/advdevice.h>
 

/* for test */
#include <signal.h>
#include <pthread.h>

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}

static void * chk_evt_fn(void *arg)
{
	int fd = (int) arg;
	PT_CheckEvent check_event;
	char err_msg[100];
	int tmp;
	int i = 0;
	

	while (!stop_flag) {
		check_event.EventType = 0; /* check all event */
		check_event.Milliseconds = 10000; /* no wait */

		tmp = DRV_CheckEvent(fd, &check_event);
		if (tmp) {
			DRV_GetErrorMessage(tmp, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return NULL;
		}
		/* printf("evttype: %d\n", check_event.EventType); */
		
		switch (check_event.EventType) {
		case ADS_EVT_TERMINATE_CNT0:
			i++;
			printf("Terminate CNT0! (interrupt: %d)\n", i);
			break;
		case ADS_EVT_TERMINATE_CNT7:
			i++;
			printf("Terminate CNT7! (interrupt: %d)\n", i);
			break;
		default:
			break;
		}
	}
}

/* end */

int main(int argc, char *argv[])
{
	PT_CounterEventStart counter_event_start;
	PT_CounterEventRead counter_event_read;
	PT_CounterConfig counter_config;
	PT_EnableEvent enable_event;
	/* PT_CheckEvent check_event; */
	unsigned short overflow = 0;
	unsigned short counter = 0;
	unsigned int count = 0;
	char *filename = NULL;
	char err_msg[100];
	int tmp;
	int fd;
	int i = 0;
	pthread_t chk_evt_tid;
	

	/* get argument */
	if (argc < 2) {
		printf("Usage: cnt_event <device node> [counter index]\n");
		return -1;
	}
	filename = argv[1];

	if (argv[2] != NULL) {
		counter = (unsigned short) atoi(argv[2]);
	} else {
		counter = 0;
	}


	/* Step 1: Open Device */
	tmp = DRV_DeviceOpen(filename, &fd);
	if (tmp < 0) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}

	/* Step 2: Config Counter */
	memset(&counter_config, 0, sizeof(PT_CounterConfig));
	counter_config.usCounter = counter;
	counter_config.usInitValue = 60000; /* 1~65535 */
	counter_config.usCountMode = MODE_O;	/* refer to manual */
	counter_config.usCountDirect = PA_MODE_COUNT_UP;
	counter_config.usCountEdge = PA_MODE_COUNT_RISE_EDGE;
	counter_config.usOutputEnable = PA_MODE_ENABLE_OUTPUT;
	counter_config.usOutputMode = PA_MODE_ACT_HIGH_TC_PULSE/* PA_MODE_TC_TOGGLE_FROM_LOW */;
	counter_config.usClkSrc = PA_MODE_COUNT_SRC_FOUT_1;
	counter_config.usGateSrc = PA_MODE_GATE_SRC_GATE_N;
	counter_config.usGatePolarity = PA_MODE_GATE_POSITIVE;
	
	tmp = DRV_CounterConfig(fd, &counter_config);
	if (tmp < 0) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	/* Step 3: Start Count */
	memset(&counter_event_start, 0, sizeof(PT_CounterEventStart));
	counter_event_start.counter = counter;
	
	tmp = DRV_CounterEventStart(fd, &counter_event_start);
	if (tmp < 0) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	

	/* Step 4: Enable event
	 *
	 * U can enable following event:
	 *
	 * ADS_EVT_TERMINATE_CNT0
	 * ADS_EVT_TERMINATE_CNT1
	 * ADS_EVT_DI_INTERRUPT0
	 * */
	enable_event.EventType = ADS_EVT_TERMINATE_CNT7;
	enable_event.Enabled   = 1;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	
	/* Step 5: Check Event */
	tmp = pthread_create(&chk_evt_tid, NULL, chk_evt_fn, (void *) fd);
	if (tmp) {
			DRV_DeviceClose(&fd);
			return -1;
	}

	/* Step 6: Read Count */
	memset(&counter_event_read, 0, sizeof(PT_CounterEventRead));
	counter_event_read.counter = counter;
	counter_event_read.count = &count;
	counter_event_read.overflow = &overflow;

	signal(SIGINT, stop_loop);
	stop_flag = 0;
	while (1) {
		if (stop_flag)
			break;

		tmp = DRV_CounterEventRead(fd, &counter_event_read);
		if (tmp < 0) {
			DRV_DeviceClose(&fd);
			return -1;
		}
		
		printf("Read Counts : %d\nOverflow: %d\n", count, overflow);
		usleep(980000);  /* accord to frequence of event,
				  * calling DRV_CounterEventRead() before
				  * counter overflow,
				  * otherwise count number will be incorrectness */	     
	}

 	/* Step 7: Counter Reset */
	tmp = DRV_CounterReset(fd, counter);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}
	stop_flag = 0;


	/* Step 8: Disable event
	 *
	 * ADS_EVT_TERMINATE_CNT0
	 * ADS_EVT_TERMINATE_CNT1
	 * ADS_EVT_DI_INTERRUPT0
	 * */
	enable_event.EventType = ADS_EVT_TERMINATE_CNT7;
	enable_event.Enabled   = 0;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}


	/* Step 9: Read Count (checking count is whether zero)*/
	while (1) {
		if (stop_flag)
			break;

		tmp = DRV_CounterEventRead(fd, &counter_event_read);
		if (tmp < 0) {
			DRV_DeviceClose(&fd);
			return -1;
		}
		
		printf("Read Counts : %d\nOverflow: %d\n", count, overflow);
		usleep(1000000);  /* accord to frequence of event,
				  * calling DRV_CounterEventRead() before
				  * counter overflow,
				  * otherwise count number will be incorrectness */	     
	}

	
	/* Step 10: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0;
} 
