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
	unsigned short overflow;
	unsigned short counter;
	unsigned int count;
	char *filename = NULL;
	char err_msg[100];
	int ret;
	int fd;


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
		
		printf("Read Counts : %d\nOverflow: %d\n", count, overflow);
		usleep(500000); /* accord to frequence of event,
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

	/* Step 5: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0;
} 

