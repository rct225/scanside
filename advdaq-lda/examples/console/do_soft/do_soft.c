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
//	Filename   :	do_soft.c                                                          |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for digital output function                           |
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
#include <Advantech/advdevice.h>


int main(int argc, char *argv[])
{
	unsigned short buffer = 0;
	unsigned long port;
	char *filename;
	char err_msg[100];
	PTR_T fd;
	int ret;
	int i;
     

	if (argc != 2) {
		printf("\nUsage: do_soft <device node>\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}

	printf("Please input parameters:");
	printf("\nOutput port :");
	scanf("%d", &port); 
	printf("\nport status: 0x");
	scanf("%x", &buffer);

	printf("%x", &buffer);
	
	filename = argv[1]; 
	/* step 1: open device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return ret;
	}

	/* step 2: DO */
	ret = DRV_DioWriteDOPorts(fd, port, 1, (unsigned char *) &buffer);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return ret;
	}
     	  
	/* setp 3: close device */
	DRV_DeviceClose(&fd);

	return 0;
} 
