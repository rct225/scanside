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
//	Filename   :	ad_soft.c                                                          |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for soft ai function                                  |
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



#define BINARY 0

int main(int argc, char *argv[])
{
	PT_AIConfig AIConfig;
	PT_AIBinaryIn AIBinaryIn;
	PT_AIVoltageIn AIVoltageIn;
	PT_AIScale AIScale;
	unsigned short wdata;
	unsigned short channel;
	unsigned short gain;
	unsigned int buffer;
	float voltage = 0;
	char *filename = NULL;
	char err_msg[100];
	int ret;
	int fd;



	if (argc < 2) {
		printf("Usage: ad_soft <device node> [channel index] [gain code]\n");
		return -1;
	}
	filename = argv[1];


     
	if (argv[2] != NULL) {
		channel = (unsigned short) atoi(argv[2]);
	} else {
		channel = 0;
	}
	

	if (argv[3] != NULL) {
		gain = (unsigned short) atoi(argv[3]);
	} else {
		gain = 0;
	}

	
	/* Step 1: Open Device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}

	memset(&AIConfig, 0, sizeof(PT_AIConfig));
	memset(&AIBinaryIn, 0, sizeof(PT_AIBinaryIn));
	memset(&AIVoltageIn, 0, sizeof(PT_AIVoltageIn));


	/* Step 3: Set Single-end or Differential */
	buffer = 0x0000;	/* 0: single-end */
	ret = DRV_DeviceSetProperty(fd, CFG_AiChanConfig, &buffer, sizeof(unsigned int));
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	/* Step 2: Config AI Setting */
	AIConfig.DasChan = channel;
	AIConfig.DasGain = gain;

	ret = DRV_AIConfig(fd, &AIConfig);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}


	/* Step 3: Start Single-channel AI */
	printf("Ctrl-C to stop !\n");
	signal(SIGINT, stop_loop);
	stop_flag = 0;
	while (1) {
		if (stop_flag) {
			printf("Out of loop, safe!\n");
			break;
		}

#if (BINARY)	/* Binary In*/
		AIBinaryIn.chan = channel;
		AIBinaryIn.TrigMode = 0;
		AIBinaryIn.reading = &wdata;

		ret = DRV_AIBinaryIn(fd, &AIBinaryIn);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}
		
		AIScale.reading = wdata;
		AIScale.MaxVolt = 5.0;
		AIScale.MaxCount = 4095;
		AIScale.offset = 2048;
		AIScale.voltage = &voltage;

		ret = DRV_AIScale(fd, &AIScale);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}
		
		printf("Binary: 0x%x Voltage: %f\n", wdata, voltage);
		
#else           /* Voltage In*/
		AIVoltageIn.chan = channel;
		AIVoltageIn.gain = gain;
		AIVoltageIn.TrigMode = 0;
		AIVoltageIn.voltage = &voltage;

		ret = DRV_AIVoltageIn(fd, &AIVoltageIn);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}

		printf("Voltage: %f\n", voltage);
#endif     
		usleep(300000);
	}

	/* Step 4: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0; 
}
