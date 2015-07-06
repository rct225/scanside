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
//	Filename   :	port_write.c                                                       |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for write port function                               |
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
#include <Advantech/advdevice.h>
 

int main(int argc, char *argv[])
{
	PT_WritePortByte WritePortByte;
	PT_WritePortWord WritePortWord;
	char err_msg[100];
	int fd; 
	int ret; 

	if(argc != 2) {
		printf("\nUsage: port_write <device node>\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}
     
	/* step 1: open device */
	ret = DRV_DeviceOpen(argv[1], &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}

	/* step 2: write port */
	memset(&WritePortByte, 0, sizeof(PT_WritePortByte));
	WritePortByte.port = 0x12;
	WritePortByte.ByteData = 0x55;

	ret = DRV_WritePortByte(fd, &WritePortByte);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	/* step 3: close device */
	DRV_DeviceClose(&fd);

	return 0;
}
