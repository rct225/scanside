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
//	Filename   :	thermo.c                                                           |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for measure the temperature function                  |
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
#include <signal.h>
#include <Advantech/advdevice.h>


static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}


int main(int argc, char *argv[])
{
     	PT_TCMuxRead tcm;
	char *filename = NULL;
	char err_msg[100];
	float fp_tmp;
	int fd;
	int ret;


	tcm.DasChan = 0;
	tcm.DasGain = 0;
	tcm.ExpChan = 0;
	tcm.TCType = 0;
	tcm.TempScale = 0;
	tcm.temp = &fp_tmp;     

	/* get argument */
	if (argc < 2) {
		printf("Usage: thermo <device node>\n");
		return -1;
	}
	filename = argv[1];

	/* step 1: open device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
         	DRV_GetErrorMessage(ret, err_msg);
	  	printf("Error Msg:%s\n",err_msg);
		return -1;
	}
	
	stop_flag = 0;

	while (1) {
		if (stop_flag) {
			printf("Out of loop, safe!\n");
			break;
		}

		/* step 2: read fp_tmperature */
		ret = DRV_TCMuxRead(fd, &tcm);
		if (ret) {
	              	DRV_GetErrorMessage(ret, err_msg);
			printf("Error Msg:%s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}
		/* step 3: show fp_tmperature */
		printf("Fp_Tmperature: %f\n", fp_tmp);
		sleep(1);
	}

	/* step 3: close device */
	DRV_DeviceClose(&fd);

	return 0;
} 
