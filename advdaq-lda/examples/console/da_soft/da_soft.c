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
//	Filename   :	da_soft.c                                                          |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for soft ao function                                  |
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
#include <termios.h>
#include <math.h>
#include <sys/mman.h>
#include <signal.h>
#include <Advantech/advdevice.h>


static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}


#define CUR_OUTPUT 0
#define VOL_OUTPUT 1
#define BIN_OUTPUT 2

int main(int argc, char *argv[])
{
	PT_AOConfig lpAOConfig;
	PT_AOCurrentOut lpAOCurrentOut;     
	PT_AOVoltageOut lpAOVoltageOut;     
	PT_AOBinaryOut lpAOBinaryOut;
	PT_AOScale lpAOScale;
	unsigned short start_chan;
	unsigned short bindata;
	float outval;
	float maxval;
	float minval;
	double count = 0;
	char *filename = NULL;
	char err_msg[100];
	float buffer;
	int buf;
	AORANGESET ao_chan_range;
	int length;
	int fd;	
	int ret;
	int output_type = VOL_OUTPUT;
	int i;

	
	/* get argument */
	if (argc < 2) {
		printf("Usage: da_soft <device node> [start channel]\n");
		return -1;
	}
	filename = argv[1];

	if (argv[2] != NULL) {
		start_chan = (unsigned short) atoi(argv[2]);
	} else {
		start_chan = 0;
	}

	maxval = 5.0;
	minval = 0.0;
	
	/* Step 1: Open Device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret < 0) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}

	
	/* Step 2: Config AO Setting (optional)
	 * 
	 * note  : default output type is voltage, no choice.
	 */ 
	if (output_type != CUR_OUTPUT) {
		memset(&lpAOConfig, 0, sizeof(PT_AOConfig));
		lpAOConfig.chan = start_chan;
		lpAOConfig.RefSrc = 0;	/* =0 internel ref, =1 external ref */
		lpAOConfig.MaxValue = maxval;
		lpAOConfig.MinValue = minval;

		ret = DRV_AOConfig(fd, &lpAOConfig);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
	}
	
	for (i = 0; i < 4; i++) {
		/* before range set */
		memset(&ao_chan_range, 0, sizeof(ao_chan_range));
		printf("Before range set[%d]:\n", i);
		printf("\tgainCount: %d\n", ao_chan_range.usGainCount);
		printf("\taoSource: %d\n", ao_chan_range.usAOSource);
		printf("\taoType: %d\n", ao_chan_range.usAOType);
		printf("\tchan: %d\n", ao_chan_range.usChan);
		printf("\taoMax: %f\n", ao_chan_range.fAOMax);
		printf("\taoMin: %f\n", ao_chan_range.fAOMin);
		
		ao_chan_range.usGainCount = 6;
		ao_chan_range.usAOSource = 0; /* =0 internal, =1 external */
		ao_chan_range.usAOType = 0; /* =0 voltage, =1 current */
		ao_chan_range.usChan = i;
		ao_chan_range.fAOMax = maxval;
		ao_chan_range.fAOMin = minval;
	
		ret = DRV_DeviceSetProperty(fd, CFG_AoChanRange,
					    &ao_chan_range, sizeof(ao_chan_range));
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

		ret = sizeof(ao_chan_range);
		ret = DRV_DeviceGetProperty(fd, CFG_AoChanRange,
					    &ao_chan_range, &ret);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}

		/* after range set */
		printf("After range set[%d]:\n", i);
		printf("\tgainCount: %d\n", ao_chan_range.usGainCount);
		printf("\taoSource: %d\n", ao_chan_range.usAOSource);
		printf("\taoType: %d\n", ao_chan_range.usAOType);
		printf("\tchan: %d\n", ao_chan_range.usChan);
		printf("\taoMax: %f\n", ao_chan_range.fAOMax);
		printf("\taoMin: %f\n", ao_chan_range.fAOMin);
	}

	/* Step 3: AO output */
	signal(SIGINT, stop_loop);
	stop_flag = 0;
	while (1) {
		if (stop_flag) {
			printf("Out of loop, safe!\n");
			break;
		}

		count++;
		usleep(500000);

		outval = 2.5;
/* 		if ((outval < minval) || (outval > maxval)) { */
/* 			printf("output is out of range!\n"); */
/* 			break; */
/* 		} */		
		
		if (output_type == CUR_OUTPUT) {
			
			memset(&lpAOCurrentOut, 0, sizeof(PT_AOCurrentOut));
			lpAOCurrentOut.chan = start_chan;
			lpAOCurrentOut.OutputValue = outval;

			ret = DRV_AOCurrentOut(fd, &lpAOCurrentOut);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);
				DRV_DeviceClose(&fd);
				return -1;
			}
		} else if (output_type == VOL_OUTPUT) {
			
			memset(&lpAOVoltageOut, 0, sizeof(PT_AOVoltageOut));
			lpAOVoltageOut.chan = start_chan;
			lpAOVoltageOut.OutputValue = outval;

			ret = DRV_AOVoltageOut(fd, &lpAOVoltageOut);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);
				DRV_DeviceClose(&fd);
				return -1;
			}
		}
		else if (output_type == BIN_OUTPUT) {
			
			memset(&lpAOScale, 0, sizeof(PT_AOScale));
			lpAOScale.chan = start_chan;
			lpAOScale.OutputValue = outval;
			lpAOScale.BinData = &bindata;
		
			ret = DRV_AOScale(fd, &lpAOScale);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);
				DRV_DeviceClose(&fd);
				return -1;
			}
		
			memset(&lpAOBinaryOut, 0, sizeof(PT_AOBinaryOut));
			lpAOBinaryOut.chan = start_chan;
			lpAOBinaryOut.BinData = bindata;
			printf("bindata: 0x%x\n", bindata);
		
			ret = DRV_AOBinaryOut(fd, &lpAOBinaryOut);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);
				DRV_DeviceClose(&fd);
				return -1;
			}
		}
		printf("out is %f, max is %f, min is %f \n",
		       outval, maxval, minval);
	}

	/* step 4: Close Device */
	DRV_DeviceClose(&fd);

	return 0;
}

