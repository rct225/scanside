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
//	Filename   :	ad_dma_bm.c                                                        |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for dma ai function                                   |
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

#define FIFO_SIZE 1024	/* 32K samples */
#define CONVERTION_NUMBER (FIFO_SIZE * 20) /* 800K samples */
#define DATA_TYPE 0		/* 0-BinaryIn, 1-VoltageIn */

/* for test */
#include <signal.h>

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}
/* end */

static int region_busy;

void release_region_func(int fd,
			 PT_EnableEvent *event,
			 unsigned short *buffer0,
			 unsigned short *buffer1,
			 float *buffer2);

int event_process_func(int fd,
		       unsigned short start,
		       unsigned short number,
		       unsigned short *raw,
		       float *voltage);



int main(int argc, char *argv[])
{
	PT_EnableEvent enable_event;
	PT_FAIDmaExStart fai_dma_ex_start;
	PT_CheckEvent check_event;
	PT_FAITransfer fai_transfer;
	PT_FAICheck fai_check;
	
	unsigned short start_chan;
	unsigned short num_chan;
	unsigned short gain_code;
	unsigned long buffer;
	unsigned short *gain_array;
	unsigned short *reading_array;
	unsigned short overrun;
	float *voltage_array;
	void *bufptr_inp;	/* buffer pointer for input */
	void *bufptr_out;	/* buffer pointer for output */
	char *filename = NULL;
	char err_msg[100];
	int tmp;
	int fd;
	int i = 0;


	/* get argument */
	if (argc < 2) {
		printf("Usage: ad_dma_bm <device node> [start channel] [channel numbers] [gain code]\n");
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
	tmp = DRV_DeviceOpen(filename, &fd);
	if (tmp) {
		printf("Open device Failed!\n");
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}

	/* Step 2: Initialize */
	gain_array = (unsigned short *) malloc(num_chan * sizeof(unsigned short));
	reading_array = (unsigned short *) malloc(CONVERTION_NUMBER * sizeof(unsigned short));
	voltage_array = (float *) malloc(CONVERTION_NUMBER * sizeof(float));
	
	memset(gain_array, gain_code, num_chan * sizeof(unsigned short));
	memset(reading_array, 0, CONVERTION_NUMBER * sizeof(unsigned short));
	memset(voltage_array, 0, CONVERTION_NUMBER * sizeof(float));
	
	memset(&fai_dma_ex_start, 0, sizeof(PT_FAIDmaExStart));
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	memset(&check_event, 0, sizeof(PT_CheckEvent));
	memset(&fai_transfer, 0, sizeof(PT_FAITransfer));     

	/* Step 3: Set Single-end or Differential */
/* 	buffer = 0x0000;	/\* 0: single-end */
/* 				 * 1: differential *\/ */
/* 	tmp = DRV_DeviceSetProperty(fd, CFG_AiChanConfig, &buffer, sizeof(unsigned long) * 2); */
/* 	if (tmp) { */
/* 		DRV_GetErrorMessage(tmp, err_msg); */
/* 		/\* printf("err msg: %s\n", err_msg); *\/ */
/* 	} */
/* 	for (i = 0; i < 4; i++) { */
/* 		gain_array[i] = (unsigned short) i; /\* 0,1,2,3 *\/ */
/* 	} */
	
	/* Step 4: Enable event
	 *
	 * U can enable following event:
	 * 
	 * ADS_EVT_AI_BUFCHANGE
	 * ADS_EVT_AI_TERMINATED
	 * ADS_EVT_AI_OVERRUN */
	enable_event.EventType = ADS_EVT_AI_LOBUFREADY;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}

	enable_event.EventType =  ADS_EVT_AI_HIBUFREADY;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}
		
	enable_event.EventType = ADS_EVT_AI_TERMINATED;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}

	enable_event.EventType = ADS_EVT_AI_OVERRUN;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, gain_array, reading_array, voltage_array);
		return -1;
	}


	/* Step 5: Start DMA Transfer */
	fai_dma_ex_start.TrigSrc    = 0; /* 0: internal; 1: external */
	fai_dma_ex_start.TrigMode   = 0; /* 0-pacer; 1-post; 2-delay; 3-about */
	fai_dma_ex_start.SRCType    = 1; /* 0: digital; 1~4: channel0~3 */
	fai_dma_ex_start.TrigEdge   = 0; /* 0: rising; 1: falling */
	fai_dma_ex_start.TrigVol    = 1.5; 
	fai_dma_ex_start.ClockSrc   = 0; /* 0: internal; 1: ext_clk0; 1: ext_clk1 */
	fai_dma_ex_start.CyclicMode = 1;
	fai_dma_ex_start.ulDelayCnt = 50000;
	fai_dma_ex_start.NumChans   = num_chan;
	fai_dma_ex_start.StartChan  = start_chan;
	fai_dma_ex_start.count      = CONVERTION_NUMBER;
	fai_dma_ex_start.SampleRate = 100000;
	fai_dma_ex_start.GainList   = gain_array;
	fai_dma_ex_start.buffer0    = reading_array;

	tmp = DRV_FAIDmaExStart(fd, &fai_dma_ex_start);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		region_busy = 1;
		release_region_func(fd, &enable_event, gain_array,
				    reading_array, voltage_array);
		return -1;
	}
	
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
		check_event.Milliseconds = 0;

		tmp = DRV_CheckEvent(fd, &check_event);
		if (tmp) {
			DRV_GetErrorMessage(tmp, err_msg);
			printf("err msg: %s\n", err_msg);

			continue;
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
			
			tmp = DRV_FAITransfer(fd, &fai_transfer);
			if (tmp) {
				release_region_func(fd, &enable_event, gain_array,
						    reading_array, voltage_array);
				return -1;
			}
			
			tmp = event_process_func(fd,
						 0,
						 num_chan,
						 bufptr_out,
						 voltage_array);
			if (tmp) {
				release_region_func(fd, &enable_event, gain_array,
						    reading_array, voltage_array);
				return -1;
			}

			tmp = DRV_FAICheck(fd, &fai_check);
			if (tmp) {
				DRV_GetErrorMessage(tmp, err_msg);
				printf("err msg: %s\n", err_msg);

				release_region_func(fd, &enable_event, gain_array,
						    reading_array, voltage_array);
				return -1;
			}
			printf("FAICheck:\n");
			printf("\tstopped: %d\n", fai_check.stopped);
			printf("\tretrieved: %d\n", fai_check.retrieved);
			printf("\toverrun: %d\n", fai_check.overrun);
			printf("\tHalfReady: %d\n\n", fai_check.HalfReady);

/* 			tmp = DRV_ClearFlag(fd, ADS_EVT_AI_LOBUFREADY); */
/* 			if (tmp) { */
/* 				release_region_func(fd, &enable_event, gain_array, */
/* 						    reading_array, voltage_array); */
/* 				return -1; */
/* 			} */
			
			break;
		case ADS_EVT_AI_HIBUFREADY:
			i++;
			printf("High buffer ready! (interrupt: %d)\n", i);

			fai_transfer.DataBuffer = bufptr_inp;
			fai_transfer.DataType = DATA_TYPE;
			fai_transfer.start = CONVERTION_NUMBER / 2;
			fai_transfer.count = CONVERTION_NUMBER / 2;
			fai_transfer.overrun = &overrun;
			
			tmp = DRV_FAITransfer(fd, &fai_transfer);
			if (tmp) {
				release_region_func(fd, &enable_event, gain_array,
						    reading_array, voltage_array);
				return -1;
			}
			
			tmp = event_process_func(fd,
						 CONVERTION_NUMBER / 2,
						 num_chan,
						 bufptr_out,
						 voltage_array);
			if (tmp) {
				release_region_func(fd, &enable_event, gain_array,
						    reading_array, voltage_array);
				return -1;
			}

			tmp = DRV_FAICheck(fd, &fai_check);
			if (tmp) {
				DRV_GetErrorMessage(tmp, err_msg);
				printf("err msg: %s\n", err_msg);

				release_region_func(fd, &enable_event, gain_array,
						    reading_array, voltage_array);
				return -1;
			}
			printf("FAICheck:\n");
			printf("\tstopped: %d\n", fai_check.stopped);
			printf("\tretrieved: %d\n", fai_check.retrieved);
			printf("\toverrun: %d\n", fai_check.overrun);
			printf("\tHalfReady: %d\n\n", fai_check.HalfReady);

/* 			tmp = DRV_ClearFlag(fd, ADS_EVT_AI_HIBUFREADY); */
/* 			if (tmp) { */
/* 				release_region_func(fd, &enable_event, gain_array, */
/* 						    reading_array, voltage_array); */
/* 				return -1; */
/* 			} */

			break;
		case ADS_EVT_AI_TERMINATED:
			i++;
			printf("Interrupt AI terminated! (interrupt: %d)\n", i);
			break;
		case ADS_EVT_AI_OVERRUN:
			i++;
			printf("Buffer overrun! (interrupt: %d)\n", i);

/* 			tmp = event_process_func(fd, */
/* 						 start_chan, */
/* 						 num_chan, */
/* 						 NULL, */
/* 						 voltage_array); */
/* 			if (tmp) { */
/* 				release_region_func(fd, &enable_event, gain_array, */
/* 						    reading_array, voltage_array); */
/* 				return -1; */
/* 			} */

			break;
		}
	}
	

	/* Step 7: Disable event */
	enable_event.EventType = ADS_EVT_AI_LOBUFREADY;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, gain_array,
				    reading_array, voltage_array);
		return -1;
	}
		
	enable_event.EventType = ADS_EVT_AI_HIBUFREADY;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, gain_array,
				    reading_array, voltage_array);
		return -1;
	}

	enable_event.EventType = ADS_EVT_AI_OVERRUN;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, gain_array,
				    reading_array, voltage_array);
		return -1;
	}

	enable_event.EventType = ADS_EVT_AI_TERMINATED;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp < 0) {
		release_region_func(fd, NULL, gain_array,
				    reading_array, voltage_array);
		return -1;
	}

	/* Step 8. Stop DMA */
	DRV_FAITerminate(fd);

 	/* Step 9: Close Device */
	free(gain_array);
	free(reading_array);
	free(voltage_array);
	DRV_DeviceClose(&fd);
     
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
		event->EventType = ADS_EVT_AI_LOBUFREADY;
		event->Enabled    = 0;
		event->Count     = 1;
		DRV_EnableEvent(fd, event);

		event->EventType = ADS_EVT_AI_HIBUFREADY;
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
	}
	
	/* terminate DMA */
	DRV_FAITerminate(fd);

	/* free buffer */
	free(buffer0);
	free(buffer1);
	free(buffer2);

	/* close device */
	DRV_DeviceClose(&fd);
}


int event_process_func(int fd,
		       unsigned short start,
		       unsigned short number,
		       unsigned short *raw,
		       float *voltage)
{
	PT_AIScale ai_scale;
	unsigned short i;
	INT32S tmp;
	INT32U count;
	

	if ((raw == NULL) && (voltage != NULL)) {
		for(i = start; i < start + number; i++) { /* print four periods data */
			printf("Voltage[%d]: %f\n", i, voltage[i]);
		}
/* 		count = 50000;	/\* 1/4 period *\/ */

/* 		for(i = 0; i < 4; i++) { /\* print four periods data *\/ */
/* 			printf("Voltage[%d]: %f\n", i, voltage[i * count]); */
/* 		} */
/* 		printf("\n"); */
	} else if ((raw != NULL) && (voltage != NULL)) {
		for(i = start; i < start + number; i++) {
			ai_scale.reading = raw[i];
			ai_scale.MaxVolt = 5.0;
			ai_scale.MaxCount = 4095;
			ai_scale.offset = 2048;
			ai_scale.voltage = voltage + i;
			
			tmp = DRV_AIScale(fd, &ai_scale);
			if (tmp) {
				return -1;
			}
			
			printf("Raw data[%d]: 0x%.4x, Voltage[%d]: %f\n",
			       i, raw[i], i, voltage[i]);
		}
		printf("\n");
	} else {
		return -1;
	}
	
	
	return 0;
}
