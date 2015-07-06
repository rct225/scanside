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
//	Filename   :	freq_out.c                                                         |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for counter frequency out function                    |
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
	PT_FreqOutStart freq_out_start;
	unsigned short counter, foutsrc, divider;
	float freq;
	char *filename = NULL;
	char err_msg[100];
	int ret;
	int fd;


	/* get argument */
	if (argc < 2) {
		printf("Usage: freq_out <device node> [counter] [divider] [foutsrc]\n");
		return -1;
	}

	filename = argv[1];

	if (argc > 2) {
		counter = (unsigned short) atoi(argv[2]);
	} else {
		counter = 0;
	}

        if (argc > 3) {
		divider = (unsigned short) atoi(argv[3]);
        } else {
		divider = 200;
        }

        if (argc > 4) {
                foutsrc = (unsigned short) atoi(argv[4]);
	} else { 
		foutsrc = 2 ; 
	}


	/* Step 1: Open Device */
	ret = DRV_DeviceOpen(argv[1], &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}
	
	/* Step 2: Start Frequency */
	memset(&freq_out_start, 0, sizeof(PT_FreqOutStart));
	freq_out_start.usChannel = counter;
	freq_out_start.usDivider = divider;
        freq_out_start.usFoutSrc = foutsrc;

	printf("Freq out put... (Ctrl-C to stop)\n");     
	signal(SIGINT, stop_loop);
 	stop_flag = 0;
         
	while (1) {
		if (stop_flag)
			break;
		
		ret = DRV_FreqOutReset(fd, counter);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}

		ret = DRV_FreqOutStart(fd, &freq_out_start);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("FreqOutStart err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}
		
		sleep(5); /* accord your signal frequence
			   * calling DRV_CounterFreqRead
			   * before counter overflow */
	}

        ret = DRV_FreqOutReset(fd, counter);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	/* Step 3: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0;
} 

