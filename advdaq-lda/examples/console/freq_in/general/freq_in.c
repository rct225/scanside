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
//	Filename   :	freq_in.c                                                          |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for counter frequency in function                     |
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
#include <termios.h>
#include <signal.h>
#include <Advantech/advdevice.h>


static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}



int main(int argc, char *argv[])
{
	PT_CounterFreqStart counter_freq_start;
	PT_CounterFreqRead counter_freq_read;
	unsigned short counter;
	float freq;
	char *filename = NULL;
	char err_msg[100];
	int ret;
	int fd;


	/* get argument */
	if (argc < 2) {
		printf("Usage: freq_in <device node> [counter index]\n");
		return -1;
	}
	filename = argv[1];

	if (argv[2] != NULL) {
		counter = (unsigned short) atoi(argv[2]);
	} else {
		counter = 0;
	}


	/* Step 1: Open Device */
	ret = DRV_DeviceOpen(argv[1], &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}
	

	/* Step 2: Start Frequency */
	memset(&counter_freq_start, 0, sizeof(PT_CounterFreqStart));
	counter_freq_start.counter = counter;
	
	ret = DRV_CounterFreqStart(fd, &counter_freq_start);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}
	

	/* Step 3: Read Frequency */
	memset(&counter_freq_read, 0, sizeof(PT_CounterFreqRead));
	counter_freq_read.counter = counter;
	counter_freq_read.freq = &freq;


        /* step 4: check event */
	printf("Waitting for Event... (Ctrl-C to stop)\n");     
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break;
	     
		ret = DRV_CounterFreqRead(fd, &counter_freq_read);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}
		
		printf("Signal freq: %10.3f\n", freq);
		usleep(500000); /* accord your signal frequence
				 * calling DRV_CounterFreqRead
				 * before counter overflow */
	     
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

