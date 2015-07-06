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
//	Filename   :	mad_soft.c                                                         |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for multiple-channels soft ai function                |
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


#define BINARY 1

int main(int argc, char *argv[])
{
	PT_MAIConfig MAIConfig;
	PT_MAIBinaryIn MAIBinaryIn;
	PT_MAIVoltageIn MAIVoltageIn;
	PT_AIScale AIScale;
	unsigned short start_chan;
	unsigned short num_chan;
	unsigned short *gain_array;
	unsigned short *reading_array;
	unsigned int buffer;
	float *voltage_array;
	char *filename = NULL;
	char err_msg[100];
	int ret;
	int fd;
	int i;
	int length;
	int buf;
	


	if (argc < 2) {
		printf("Usage: mad_soft <device node> [start channel] [channel numbers]\n");
		return -1;
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
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}


	/* Step 2: Initialize */
	gain_array = (unsigned short *) malloc(num_chan * sizeof(unsigned short));
	reading_array = (unsigned short *) malloc(num_chan * sizeof(unsigned short));
	voltage_array = (float *) malloc(num_chan * sizeof(float));
	
	for (i = 0; i < num_chan; i++) {
		gain_array[i] = 0;
		reading_array[i] = 0;	
		voltage_array[i] = 0;	
	}

	memset(&MAIConfig, 0, sizeof(PT_MAIConfig));
	memset(&MAIBinaryIn, 0, sizeof(PT_MAIBinaryIn));
	memset(&MAIVoltageIn, 0, sizeof(PT_MAIVoltageIn));


	/* Step 3: Set Single-end or Differential */
	buffer = 0x0000;	/* 0: single-end
				 * 1: differential */
	ret = DRV_DeviceSetProperty(fd, CFG_AiChanConfig, &buffer, sizeof(unsigned int));
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		free(gain_array);
		free(reading_array);
		free(voltage_array);
		DRV_DeviceClose(&fd);
		return -1;
	}

	
	/* Step 4: Mutil-Channel AI Configure */
	MAIConfig.NumChan = num_chan;
	MAIConfig.StartChan = start_chan;
	MAIConfig.GainArray = gain_array;
	
	ret = DRV_MAIConfig(fd, &MAIConfig);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		free(gain_array);
		free(reading_array);
		free(voltage_array);
		DRV_DeviceClose(&fd);
		return -1;
	}

	/* Step 5: Start Multiple-channel AI */
	printf("Ctrl-C to stop !\n");
	signal(SIGINT, stop_loop);
	stop_flag = 0;
	while (1) {
		
		
		if (stop_flag) {
			printf("Out of loop, safe!\n");
			break;
		}
		

#if (BINARY)	/*  AI Binary In*/
		MAIBinaryIn.NumChan = num_chan;
		MAIBinaryIn.StartChan = start_chan;
		MAIBinaryIn.TrigMode = 0;
		MAIBinaryIn.ReadingArray = reading_array;

		
		ret = DRV_MAIBinaryIn(fd, &MAIBinaryIn);

		if (ret) {
			free(gain_array);
			free(reading_array);
			free(voltage_array);
			DRV_DeviceClose(&fd);
			return -1;
		}
     
		
		
		for(i = 0; i < num_chan; i++) {
			AIScale.reading = reading_array[i];
			AIScale.MaxVolt = 5.0;
			AIScale.MaxCount = 4095;
			AIScale.offset = 2048;
			AIScale.voltage = voltage_array + i;

			ret = DRV_AIScale(fd, &AIScale);
			if (ret) {
				free(gain_array);
				free(reading_array);
				free(voltage_array);
				DRV_DeviceClose(&fd);
				return -1;
			}

			printf("Raw data[%d]: 0x%x, Voltage[%d]: %f\n",
			       i, MAIBinaryIn.ReadingArray[i],
			       i, voltage_array[i], (voltage_array + i));
		}
	  
#else           /* Voltage In*/
		MAIVoltageIn.NumChan = num_chan;
		MAIVoltageIn.StartChan = start_chan;
		MAIVoltageIn.GainArray = gain_array;
		MAIVoltageIn.TrigMode = 0;
		MAIVoltageIn.VoltageArray = voltage_array;

		ret = DRV_MAIVoltageIn(fd, &MAIVoltageIn);
		if (ret) {
			free(gain_array);
			free(reading_array);
			free(voltage_array);
			DRV_DeviceClose(&fd);
			return -1;
		}

		for(i = 0; i < num_chan; i++) {
			printf("Voltage[%d]: %f\n", i, voltage_array[i]);
		}
#endif     
		usleep(300000);
		printf("\n");	/* separate output between channels */

		
	}
	
	
 	/* Step 6: Close Device */
	free(gain_array);
	free(reading_array);
	free(voltage_array);
	DRV_DeviceClose(&fd);
	
	return 0; 
}
