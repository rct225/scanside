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
//	Filename   :	mad_int.c                                                          |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for multiple-channels ai with interrupt mode          |
//	                function                                                           | 
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
 
#define FIFO_SIZE 1024 		/* 1024 samples */
#define CONVERTION_NUMBER (FIFO_SIZE * 10)
#define DATA_TYPE 0		/* 0-BinaryIn, 1-VoltageIn */
#define SAMPLE_RATE 100000

static int fai_running;
static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}


int event_process_func(int fd,
		       unsigned short start,
		       unsigned short number,
		       unsigned short *raw,
		       float *voltage);

void release_region_func(int fd,
			 PT_EnableEvent *event,
			 unsigned short *buffer0,
			 unsigned short *buffer1,
			 float *buffer2);



int main(int argc, char *argv[])
{
	PT_FAIIntScanStart fai_int_scan_start;
	PT_FAITransfer fai_transfer;
	PT_EnableEvent enable_event;
	PT_CheckEvent check_event;
	PT_FAICheck   fai_check;
	unsigned short start_chan;
	unsigned short num_chan;
	unsigned short gain_code;
	unsigned short *gain_array;
	unsigned short *reading_array;
	unsigned short overrun;
	float *voltage_array;
	void *bufptr_inp;	/* buffer pointer for input */
	void *bufptr_out;	/* buffer pointer for output */
	char *filename = NULL;
	char err_msg[100];
	unsigned int buffer;
	int ret;
	int fd;
	int i = 0;


	/* get argument */
	if (argc < 2) {
		printf("Usage: mad_int <device node> [start channel] [channel numbers] [gain code]\n");
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

	if (argv[4] != NULL) {
		gain_code = (unsigned short) atoi(argv[4]);
	} else {
		gain_code = 0;
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
	reading_array = (unsigned short *) malloc(CONVERTION_NUMBER * sizeof(unsigned short));
	voltage_array = (float *) malloc(CONVERTION_NUMBER * sizeof(float));
	
	memset(reading_array, 0, CONVERTION_NUMBER * sizeof(unsigned short));
	memset(voltage_array, 0, CONVERTION_NUMBER * sizeof(float));
	
	memset(&fai_int_scan_start, 0, sizeof(PT_FAIIntScanStart));
	memset(&fai_transfer, 0, sizeof(PT_FAITransfer));
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	memset(&check_event, 0, sizeof(PT_CheckEvent));
	
	/* u have to fill gain_array by yourself */
	for (i = 0; i < num_chan; i++) {
		gain_array[i] = gain_code;
	}

	/* Step 3: Set Single-end or Differential */
	buffer = 0x0000;	/* 0: single-end
				 * 1: differential */
	ret = DRV_DeviceSetProperty(fd, CFG_AiChanConfig, &buffer, sizeof(unsigned int));
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}
	
	/* Step 4: Enable event
	 *
	 * U can enable following event:
	 *
	 * ADS_EVT_AI_INTERRUPT
	 * ADS_EVT_AI_LOBUFREADY
	 * ADS_EVT_AI_HIBUFREADY
	 * ADS_EVT_AI_TERMINATED
	 * ADS_EVT_AI_OVERRUN */
	enable_event.EventType = ADS_EVT_AI_OVERRUN;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}
	
	enable_event.EventType = ADS_EVT_AI_LOBUFREADY;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}

	enable_event.EventType =  ADS_EVT_AI_HIBUFREADY;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}
		
	enable_event.EventType = ADS_EVT_AI_TERMINATED;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}


	/* Step 5: Start Fast AI Transfer */
	fai_int_scan_start.TrigSrc    = 0;
	fai_int_scan_start.SampleRate = SAMPLE_RATE;
	fai_int_scan_start.NumChans   = num_chan;
	fai_int_scan_start.StartChan  = start_chan;
	fai_int_scan_start.GainList   = gain_array;
	fai_int_scan_start.buffer     = reading_array;
	fai_int_scan_start.count      = CONVERTION_NUMBER;
	fai_int_scan_start.cyclic     = 1;
	fai_int_scan_start.IntrCount  = FIFO_SIZE / 2; /* 1: without fifo */

	ret = DRV_FAIIntScanStart(fd, &fai_int_scan_start);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, &enable_event, gain_array, reading_array, voltage_array);
		return -1;
	}
	fai_running = 1;
	
	switch (DATA_TYPE) {
	case 0:			/* binary in */
		bufptr_inp = (void *) reading_array;
		bufptr_out = (void *) reading_array;
		break;
	case 1:			/* voltage in */
		bufptr_inp = (void *) voltage_array;
		bufptr_out = NULL;
		break;
	default:
		release_region_func(fd, &enable_event, gain_array,
				    reading_array, voltage_array);
		return -1;
	}
	
	/* Step 6: Check event */
	printf("Waitting for Event, Sleepping... (Ctrl-C to stop)\n");
	signal(SIGINT, stop_loop);
	stop_flag = 0;
	while (1) {
		if ((stop_flag)
		    || (check_event.EventType == ADS_EVT_AI_TERMINATED)) {
			printf("Out of loop, safe!\n");
			break;
		}

		check_event.EventType = 0; /* check all event */
		check_event.Milliseconds = 3000; /* wait for one second */

		ret = DRV_CheckEvent(fd, &check_event);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			release_region_func(fd, &enable_event,
					    gain_array,
					    reading_array,
					    voltage_array);
			return -1;
		}
	
		switch (check_event.EventType) {
		case ADS_EVT_AI_LOBUFREADY:
			i++;
			printf("Low buffer ready! (interrupt: %d)\n", i);

			fai_transfer.DataBuffer = bufptr_inp;
			fai_transfer.DataType = DATA_TYPE;
			fai_transfer.start = 0;
			fai_transfer.count = CONVERTION_NUMBER / 2;
			fai_transfer.overrun = &overrun;
			
			ret = DRV_FAITransfer(fd, &fai_transfer);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);

				release_region_func(fd, &enable_event,
						    gain_array,
						    reading_array,
						    voltage_array);
				return -1;
			}
			
			ret = event_process_func(fd,
						 0,
						 num_chan * 8,
						 bufptr_out,
						 voltage_array);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);

				release_region_func(fd, &enable_event,
						    gain_array,
						    reading_array,
						    voltage_array);
				return -1;
			}

/* 			ret = DRV_FAICheck(fd, &fai_check); */
/* 			if (ret) { */
/* 				DRV_GetErrorMessage(ret, err_msg); */
/* 				printf("err msg: %s\n", err_msg); */

/* 				release_region_func(fd, &enable_event, */
/* 						    gain_array, */
/* 						    reading_array, */
/* 						    voltage_array); */
/* 				return -1; */
/* 			} */
/* 			printf("FAICheck:\n"); */
/* 			printf("\tstopped: %d\n", fai_check.stopped); */
/* 			printf("\tretrieved: %d\n", fai_check.retrieved); */
/* 			printf("\toverrun: %d\n", fai_check.overrun); */
/* 			printf("\tHalfReady: %d\n\n", fai_check.HalfReady); */

			break;
		case ADS_EVT_AI_HIBUFREADY:
			i++;
			printf("High buffer ready! (interrupt: %d)\n", i);

			fai_transfer.DataBuffer = bufptr_inp;
			fai_transfer.DataType = DATA_TYPE;
			fai_transfer.start = CONVERTION_NUMBER / 2;
			fai_transfer.count = CONVERTION_NUMBER / 2;
			fai_transfer.overrun = &overrun;
			
			ret = DRV_FAITransfer(fd, &fai_transfer);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);

				release_region_func(fd, &enable_event,
						    gain_array,
						    NULL,
						    voltage_array);
				return -1;
			}

			ret = event_process_func(fd,
						 CONVERTION_NUMBER / 2,
						 num_chan * 8,
						 bufptr_out,
						 voltage_array);
			if (ret) {
				release_region_func(fd, &enable_event,
						    gain_array,
						    reading_array,
						    voltage_array);

				return -1;
			}

/* 			ret = DRV_FAICheck(fd, &fai_check); */
/* 			if (ret) { */
/* 				DRV_GetErrorMessage(ret, err_msg); */
/* 				printf("err msg: %s\n", err_msg); */

/* 				release_region_func(fd, &enable_event, */
/* 						    gain_array, */
/* 						    reading_array, */
/* 						    voltage_array); */
/* 				return -1; */
/* 			} */
/* 			printf("FAICheck:\n"); */
/* 			printf("\tstopped: %d\n", fai_check.stopped); */
/* 			printf("\tretrieved: %d\n", fai_check.retrieved); */
/* 			printf("\toverrun: %d\n", fai_check.overrun); */
/* 			printf("\tHalfReady: %d\n\n", fai_check.HalfReady); */

			break;
		case ADS_EVT_AI_TERMINATED:
			i++;
			printf("Interrupt AI terminated! (interrupt: %d)\n", i);

			ret = event_process_func(fd,
						 start_chan,
						 num_chan * 8,
						 bufptr_out,
						 voltage_array);
			if (ret) {
				release_region_func(fd, &enable_event,
						    gain_array,
						    reading_array,
						    voltage_array);
				return -1;
			}
			break;
		case ADS_EVT_AI_OVERRUN:
			printf("Buffer overrun!\n");
			break;
		case ADS_EVT_DEVREMOVED:
			printf("Device removed!\n");
			release_region_func(fd, &enable_event,
					    gain_array,
					    reading_array,
					    voltage_array);

			return 0;
		case ADS_EVT_TIME_OUT:
			printf("Check event time out!\n");
			break;
		default:
			printf("NO Event!\n");
			break;
		}
	}
	

	/* Step 7: Disable event */
	enable_event.EventType = ADS_EVT_AI_LOBUFREADY;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}
		
	enable_event.EventType = ADS_EVT_AI_HIBUFREADY;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}
		
	enable_event.EventType = ADS_EVT_AI_TERMINATED;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}

	enable_event.EventType = ADS_EVT_AI_OVERRUN;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}

	/* Step 8. Stop Fast AI and Release Regions */
	release_region_func(fd, NULL, gain_array, reading_array, voltage_array);

     
	return 0; 
}

int event_process_func(int fd,
		       unsigned short start,
		       unsigned short number,
		       unsigned short *raw,
		       float *voltage)
{
	PT_AIScale ai_scale;
	char err_msg[100];
	int ret;
	int i;

		
	if ((raw == NULL) && (voltage != NULL)) {
		for(i = start; i < start + number; i++) {
			printf("Voltage[%d]: %f\n", i, voltage[i]);
		}
		printf("\n");
	} else if ((raw != NULL) && (voltage != NULL)) {
		for(i = start; i < start + number; i++) {
			ai_scale.reading = raw[i];
			ai_scale.MaxVolt = 0.625;
			ai_scale.MaxCount = 4095;
			ai_scale.offset = 2048;
			ai_scale.voltage = voltage + i;
			
			ret = DRV_AIScale(fd, &ai_scale);
			if (ret) {
				DRV_GetErrorMessage(ret, err_msg);
				printf("err msg: %s\n", err_msg);

				return -1;
			}
			
			printf("Raw data[%d]: 0x%.4x, Voltage[%d]: %f\n",
			       i, raw[i],
		       i, voltage[i]);
		}
		printf("\n");
	} else {
		return -1;
	}

	return 0;
}

void release_region_func(int fd,
			 PT_EnableEvent *event,
			 unsigned short *buffer0,
			 unsigned short *buffer1,
			 float *buffer2)
{

	/* disable event */
	if (event != NULL) {
		event->EventType = ADS_EVT_AI_HIBUFREADY;
		event->Enabled    = 0;
		event->Count     = 1;
		DRV_EnableEvent(fd, event);

		event->EventType = ADS_EVT_AI_LOBUFREADY;
		event->Enabled    = 0;
		event->Count     = 1;
		DRV_EnableEvent(fd, event);

		event->EventType = ADS_EVT_AI_TERMINATED;
		event->Enabled    = 0;
		event->Count     = 1;
		DRV_EnableEvent(fd, event);

		event->EventType = ADS_EVT_AI_OVERRUN;
		event->Enabled    = 0;
		event->Count     = 1;
		DRV_EnableEvent(fd, event);

		event->EventType = ADS_EVT_AI_OVERRUN;
		event->Enabled    = 0;
		event->Count     = 1;
		DRV_EnableEvent(fd, event);
	}
	
	/* terminate interrupt */
	if (fai_running) {
		DRV_FAITerminate(fd);
	}
	

	/* free buffer */
	if (!buffer0) {
		free(buffer0);
	}
	
	if (!buffer1) {
		free(buffer1);
	}
	
	if (!buffer2) {
		free(buffer2);
	}

	/* close device */
	DRV_DeviceClose(&fd);
}
