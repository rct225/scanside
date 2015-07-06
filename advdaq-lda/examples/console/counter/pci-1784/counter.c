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
//	Description: 	Demo program for counter event counting function                   |
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
#include <signal.h>
#include <Advantech/advdevice.h>

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}


int main(int argc, char *argv[])
{
	PT_CounterEventStart counter_event_start;
	PT_CounterEventRead counter_event_read;
	unsigned short overflow = 0;
	unsigned short counter;
	unsigned int count = 0;
	char *filename = NULL;
	char err_msg[100];
	int ret;
	int fd;
	int i;
	int size;
	INT32U cntr_reset[]={0,0,0,0};
	INT32U inputmode[]={1,1,1,1};


	/* get argument */
	if (argc < 2) {
		printf("Usage: counter <device node> [counter index]\n");
		return -1;
	}
	filename = argv[1];

	if (argv[2] != NULL) {
		counter = (unsigned short) atoi(argv[2]);
	} else {
		counter = 0;
	}

	/* Step 1: Open Device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}

	for(i=0;i<4;i++){
		ret = DRV_CounterReset(fd,i );
		if (ret) {
			printf("Get CFG_CounterReset err \n");
			DRV_DeviceClose(&fd);
			return -1;
		}
	}

	/* Set Counter Reset Value */
	size = sizeof(cntr_reset);
	ret = DRV_DeviceSetProperty(fd, CFG_CntrResetValue, cntr_reset, size);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("DRV_DeviceSetProperty CFG_CntrResetValue Error Msg:%s\n",err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
		
         /* Set counter input mode */
	size = sizeof(inputmode);
        ret = DRV_DeviceSetProperty(fd, CFG_CntrInputModeControl, inputmode, size);
        if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
                printf("DRV_DeviceSetProperty CFG_CntrInputModeControl Error Msg:%s\n",err_msg);
                DRV_DeviceClose(&fd);
                return ret;
        }

	/* Step 2: Start Count */
	memset(&counter_event_start, 0, sizeof(PT_CounterEventStart));
	counter_event_start.counter = counter;
	ret = DRV_CounterEventStart(fd, &counter_event_start);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	/* Step 3: Read Count */
	memset(&counter_event_read, 0, sizeof(PT_CounterEventRead));
	counter_event_read.counter = counter;
	counter_event_read.count = &count;
	counter_event_read.overflow = &overflow;

	signal(SIGINT, stop_loop);
	stop_flag = 0;
	while (1) {
		if (stop_flag)
			break;

		ret = DRV_CounterEventRead(fd, &counter_event_read);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}
		
		printf("Read Counts : %d, Overflow: %d\n", count, overflow);
		if (overflow) {
			break;
		}
		usleep(1000000); /* accord to frequence of event,
				 * calling DRV_CounterEventRead() before
				 * counter overflow,
				 * otherwise count number will be incorrectness */	     
	}
	
	/* Step 4: Reset Counter */
	ret = DRV_CounterReset(fd, counter);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	ret = DRV_CounterEventRead(fd, &counter_event_read);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}
	printf("Last:\nRead Counts : %d, Overflow: %d\n", count, overflow);

	/* Step 5: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0;
} 

