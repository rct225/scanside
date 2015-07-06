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
//	Filename   :	da_sync.c                                                          |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for sync ao function                                  |
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
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <sys/mman.h>
#include "../../../include/advdevice.h"

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
	PT_AOConfig	 lpAOConfig;
	PT_AOVoltageOut  lpAOVoltageOut;     
	PT_AOBinaryOut	 lpAOBinaryOut;
	PT_AOScale       lpAOScale;
	unsigned short   bindata;
	unsigned short   start_chan;
	unsigned short   num_chan;
	float            outval;
	double           count = 0;
	char             *filename = NULL;
	char             err_msg[100];
	int              tmp;
	int              fd;
	int              i;
	

	
	if (argc < 2) {
		printf("\nUsage: da_sync <device node> [start channel] [channel numbers]\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}
	filename = argv[1];

	if (argv[2] != NULL) {
		start_chan = (unsigned short) atoi(argv[2]);
	} else {
		start_chan = 0;
	}

	if (argv[3] != NULL) {
		num_chan = (unsigned short) atoi(argv[3]);

		if (num_chan == 0) { /* channel numbers must > 0 */
			num_chan = 1;
		}
	} else {
		num_chan = 1;
	}

	
	/* Step 1: Open Device */
	tmp = DRV_DeviceOpen(filename, &fd);
	if (tmp < 0) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}

	/* Step 2: Config AO Setting (optional) */ 
	for (i = start_chan; i < start_chan + num_chan; i++) {
		memset(&lpAOConfig, 0, sizeof(PT_AOConfig));
		lpAOConfig.chan = i;
		lpAOConfig.RefSrc = 0;	/* =0 internel ref, =1 external ref */
		lpAOConfig.MaxValue = 10.0;
		lpAOConfig.MinValue = -10.0;
	
		tmp = DRV_AOConfig(fd, &lpAOConfig);
		if (tmp) {
			DRV_GetErrorMessage(tmp, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}
	}

	/* Step 3: Enable Sync AO */
	tmp = DRV_EnableSyncAO(fd, 1);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	/* Step 4: AO Output */
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break;

		count++;
		
		outval = 3.5;
		printf("outval is %f, max is %f, min is %f \n", outval, lpAOConfig.MaxValue, lpAOConfig.MinValue);
		
		/* using DRV_AOVoltageOut() API*/
		for (i = start_chan; i < start_chan + num_chan; i++) {
			memset(&lpAOVoltageOut, 0, sizeof(PT_AOVoltageOut));
			lpAOVoltageOut.chan = i;
			lpAOVoltageOut.OutputValue = outval;

			tmp = DRV_AOVoltageOut(fd, &lpAOVoltageOut);
			if (tmp) {
				DRV_GetErrorMessage(tmp, err_msg);
				printf("err msg: %s\n", err_msg);
			
				DRV_DeviceClose(&fd);
				return -1;
			}
		}		

		printf("\nWrite Sync AO now ? (y/n)");
		tmp = getchar();
		printf("\n");
	  
		if ((char) tmp == 'y') {
			printf("\nWrite sync ao now ...\n");
			if (DRV_WriteSyncAO(fd)) {
				DRV_GetErrorMessage(tmp, err_msg);
				printf("err msg: %s\n", err_msg);

				DRV_DeviceClose(&fd);
				return -1;
			}
			continue;
		} else
			break;

	}

	
	/* Step 3: Disable Sync AO */
	tmp = DRV_EnableSyncAO(fd, 0);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	/* step 5: Close Device */
	DRV_DeviceClose(&fd);
	return 0;
}
