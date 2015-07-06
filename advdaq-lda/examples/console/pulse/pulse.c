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
//	Filename   :	pulse.c                                                            |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for counter pulse out function                        |
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
	PT_CounterPulseStart counter_pulse_start;
	unsigned short counter;
	float period;
	char *filename = NULL;
	char err_msg[100];
	int ret;
	PTR_T fd;


	/* get argument */
	if (argc < 2) {
		printf("Usage: pulse <device node> [counter index] [period]\n");
		return -1;
	}
	filename = argv[1];

	if (argv[2] != NULL) {
		counter = (unsigned short) atoi(argv[2]);
	} else {
		counter = 0;
	}

	if (argv[3] != NULL) {
		period = (float) atof(argv[3]);
	} else {
		period = 0.0001;
	}

	/* Step 1: Open Device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}

	
	/* Step 2: Start Pulse Out */
	memset(&counter_pulse_start, 0, sizeof(PT_CounterPulseStart));
	counter_pulse_start.counter = counter;
	counter_pulse_start.period  = period;
	counter_pulse_start.UpCycle = period / 2;
	counter_pulse_start.GateMode = 0;

	ret = DRV_CounterPulseStart(fd, &counter_pulse_start);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	printf("Pulse out...(press any key to stop)\n");
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break; 
	}


	/* Step 3: Reset Counter */
	ret = DRV_CounterReset(fd, counter);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}
	
		
	/* Step 4: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0;
}
