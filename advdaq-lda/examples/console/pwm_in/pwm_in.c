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
//	Description: 	Demo program for counter PWM measure function                      |
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
#include <Advantech/advdevice.h>

/* for hard test */
#include <signal.h>

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}
/* end */

int main(int argc, char *argv[])
{
	PT_PWMStartRead pwm_start_read;
	unsigned short counter;
	char *filename = NULL;
	char err_msg[100];
	int tmp;
	PTR_T fd;
	float hi_period;
	float low_period;
	

	/* for hard test */
	signal(SIGINT, stop_loop);
	
	/* get argument */
	if (argc < 2) {
		printf("Usage: pwm_in <device node> [counter index]\n");
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
	if (tmp) {
		printf("Open Device Failed!\n");
		return -1;
	}
	

	/* Step 2: Config Counter PWM */
	memset(&pwm_start_read, 0, sizeof(PT_PWMStartRead));
	pwm_start_read.usChan = counter;
	pwm_start_read.flHiperiod = &hi_period; /* seconds */
	pwm_start_read.flLowperiod = &low_period; /* should be > 0.5 */

	stop_flag = 0;
	while (1) {
		if (stop_flag) {
			printf("Out of loop, safe!\n");
			break;
		}
		
		tmp = DRV_PWMStartRead(fd, &pwm_start_read);
		if (tmp) {
			DRV_GetErrorMessage(tmp, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}
		printf("High Period: %.4fus, Low Period: %.4fus\n",
		       hi_period,
		       low_period);
 		usleep(500000); /* accord your signal frequence 
 				 * calling DRV_PWMStartRead 
 				 * before counter overflow  */
	}

	/* Step 3: Reset Counter */
	tmp = DRV_CounterReset(fd, counter);
	if (tmp) {
		DRV_DeviceClose(&fd);
		return -1;
	}
	
		
	/* Step 5: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0;
} 
