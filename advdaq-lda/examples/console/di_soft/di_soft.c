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
//	Filename   :	di_soft.c                                                          |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for digital input function                            |
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
	PTR_T fd;
	char err_msg[100];
	unsigned char *buffer;
	unsigned long portstart;
	unsigned long portnum;
	int i, j = 0, ret;
     


	if(argc != 2) {
		printf("\nUsage: di_soft <device node>\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}
	printf("Please input parameters:");
	printf("\nstart port :");
	scanf("%d", &portstart); 
	printf("\nport count :");
	scanf("%d", &portnum);
 
	buffer = malloc(portnum * sizeof(unsigned char));
	if (!buffer) {
		return -1;
	}
	memset(buffer, 0, portnum * sizeof(unsigned char));

	/* step 1: open device */
	ret = DRV_DeviceOpen(argv[1], &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		free(buffer);
		return -1;
	}

	/* step 2: read state form the ports */
	printf("DI state... (Ctrl-C to stop)\n");     
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break;


		ret = DRV_DioReadDIPorts(fd, portstart, portnum, buffer);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			free(buffer);
			DRV_DeviceClose(&fd);
			return -1;
		}

		/* step 3: show the ports status */
		for (i = 0; i < portnum; i++) {
			printf("%d Port States 0x%.2x\n\n", portstart + i, buffer[i]);
		}

		usleep(500000);
	}
	
	/* step 4: close device */
	free(buffer);	     
	DRV_DeviceClose(&fd);

	return 0;
} 
