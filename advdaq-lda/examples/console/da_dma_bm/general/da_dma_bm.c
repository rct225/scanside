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
//	Filename   :	da_dma_bm.c                                                        |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for dma ao function                                   |
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

#define FIFO_SIZE 1024		/* 1K samples */
#define CONVERTION_NUMBER (FIFO_SIZE * 200)

/* for test */
#include <signal.h>

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}
/* end */

void release_region_func(int fd,
			 PT_EnableEvent *event,
			 unsigned short *buffer0,
			 unsigned short *buffer1,
			 float *buffer2);


int main(int argc, char *argv[])
{
	PT_EnableEvent enable_event;
	PT_FAODmaExStart fao_dma_ex_start;
	PT_FAOScale fao_scale;
	PT_AOConfig ao_config;
	PT_CheckEvent check_event;
	PT_FAOLoad fao_load;
	PT_FAOCheck fao_check;
	unsigned short start_chan;
	unsigned short num_chan;
	unsigned short *binary_array;
	unsigned short *data_buffer;
	float *voltage_array;
	float buff;
	char *filename = NULL;
	char err_msg[100];
	int half, tmp;
	int fd;
	int i;


	/* get argument */
	if (argc < 2) {
		printf("Usage: da_dma_bm <device node> [start channel] [channel numbers]\n");
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
	tmp = DRV_DeviceOpen(filename, &fd);
	if (tmp < 0) {
		printf("Open device Failed!\n");
		return -1;
	}

	/* tmp = DRV_FAOTerminate(fd); */
	
	
	/* Step 2: Initialize */
	binary_array = (unsigned short *) malloc(CONVERTION_NUMBER * sizeof(unsigned short));
	data_buffer = (unsigned short *) malloc(CONVERTION_NUMBER * sizeof(unsigned short));
	voltage_array = (float *) malloc(CONVERTION_NUMBER * sizeof(float));
	
	memset(binary_array, 0, CONVERTION_NUMBER * sizeof(unsigned short));
	memset(data_buffer, 0, CONVERTION_NUMBER * sizeof(unsigned short));
	memset(voltage_array, 0, CONVERTION_NUMBER * sizeof(float));
	memset(&fao_dma_ex_start, 0, sizeof(PT_FAODmaExStart));
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	memset(&check_event, 0, sizeof(PT_CheckEvent));
	memset(&ao_config, 0, sizeof(PT_AOConfig));
	memset(&fao_scale, 0, sizeof(PT_FAOScale));
	memset(&fao_load, 0, sizeof(PT_FAOLoad));


	/* step 3: Config */
	for (i = start_chan; i < start_chan + num_chan; i++) {
		ao_config.chan = i;
		ao_config.RefSrc = 0;
		ao_config.MaxValue = 10.0;
		ao_config.MinValue = -10.0;
		DRV_AOConfig(fd, &ao_config);
	}
	

	/* Step 4: Enable event
	 *
	 * U can enable following event:
	 *
	 * ADS_EVT_AO_LOBUF_TRANSFERED
	 * ADS_EVT_AO_HIBUF_TRANSFERED
	 * ADS_EVT_AO_TERMINATED
	 * */
	enable_event.EventType = ADS_EVT_AO_LOBUF_TRANSFERED;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, NULL, binary_array, voltage_array);
		return -1;
	}
		
	enable_event.EventType = ADS_EVT_AO_HIBUF_TRANSFERED;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, NULL, binary_array, voltage_array);
		return -1;
	}

	enable_event.EventType = ADS_EVT_AO_UNDERRUN;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, NULL, binary_array, voltage_array);
		return -1;
	}

	enable_event.EventType = ADS_EVT_AO_TERMINATED;
	enable_event.Enabled    = 1;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, NULL, binary_array, voltage_array);
		return -1;
	}


	/* Step 5: Start DMA Transfer */

	/* fill buffer data (Square Wave Data)*/
	for (i = 0; i < CONVERTION_NUMBER; i++) {
		tmp = i % 2048;	/* 2048 is period */
		half = (int) ((float) 2048 / 2.0);
		if (tmp <= half) {
			voltage_array[i] = -2;
		} else {
			voltage_array[i] = 2;
		}
	}

	for (i = start_chan; i < start_chan + num_chan; i++) {
		fao_scale.chan = i;
		fao_scale.count = CONVERTION_NUMBER;
		fao_scale.VoltArray = voltage_array;
		fao_scale.BinArray = binary_array;
		DRV_FAOScale(fd, &fao_scale);
	}
	memcpy(data_buffer, binary_array, CONVERTION_NUMBER * sizeof(unsigned short));
	
/* 	for (i = 0; i < CONVERTION_NUMBER; i++) { */
/* 		binary_array[i] = 0x599; */
/* 	} */
	

 	fao_dma_ex_start.TrigSrc = 0; /* 0: internal; 1: external */
 	fao_dma_ex_start.SampleRate = 200000; //CONVERTION_NUMBER;
 	fao_dma_ex_start.count = CONVERTION_NUMBER;
 	fao_dma_ex_start.StartChan = start_chan; 
 	fao_dma_ex_start.NumChans = num_chan; 
 	fao_dma_ex_start.buffer = (INT32U *) binary_array; 
 	fao_dma_ex_start.CyclicMode = 1; 
 	fao_dma_ex_start.PacerSource = 0; 

	tmp = DRV_FAODmaExStart(fd, &fao_dma_ex_start);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);

		release_region_func(fd, &enable_event, NULL,
				    binary_array, voltage_array);
		return -1;
	}
	
/* 	buff = 200000.0; */
/* 	tmp = sizeof(float); */
/* 	tmp = DRV_DeviceSetProperty(fd, CFG_AoPacerRate, &buff, sizeof(float)); */
/* 	if (tmp) { */
/* 		DRV_GetErrorMessage(tmp, err_msg); */
/* 		printf("err msg: %s\n", err_msg); */
/* 		release_region_func(fd, &enable_event, NULL, */
/* 				    binary_array, voltage_array); */
/* 		return -1; */
/* 	} */

/* 	buff = 0; */
/* 	tmp = sizeof(float); */
/* 	tmp = DRV_DeviceGetProperty(fd, CFG_AoPacerRate, &buff, &tmp); */
/* 	if (tmp) { */
/* 		DRV_GetErrorMessage(tmp, err_msg); */
/* 		printf("err msg: %s\n", err_msg); */
/* 		release_region_func(fd, &enable_event, NULL, */
/* 				    binary_array, voltage_array); */
/* 		return -1; */
/* 	} */
/* 	printf("ao rate: %f\n", buff);	 */

	/* Step 6: Check event */
	printf("Waitting for Event, Sleepping... (Ctrl-C to stop)\n");
	signal(SIGINT, stop_loop);
	stop_flag = 0;
	while (1) {
		if ((stop_flag)
		    || (check_event.EventType == ADS_EVT_AO_TERMINATED)) {
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
		case ADS_EVT_AO_LOBUF_TRANSFERED:
			printf("Low Buffer Transfered!\n");

			fao_load.DataBuffer = data_buffer;
			fao_load.start = 0;
			fao_load.count = CONVERTION_NUMBER / 2;

/* 			tmp = DRV_FAOCheck(fd, &fao_check); */
/* 			if (tmp) { */
/* 				DRV_GetErrorMessage(tmp, err_msg); */
/* 				printf("err msg: %s\n", err_msg); */
/* 				release_region_func(fd, NULL, NULL, binary_array, voltage_array); */
/* 				return -1; */
/* 			} */
/* 			printf("overrun: %d\n", fao_check.overrun); */
/* 			printf("HalfReady: %d\n", fao_check.HalfReady); */
/* 			printf("stopped: %d\n", fao_check.stopped); */
			

			tmp = DRV_FAOLoad(fd, &fao_load);
			if (tmp) {
				DRV_GetErrorMessage(tmp, err_msg);
				printf("err msg: %s\n", err_msg);
				release_region_func(fd, NULL, NULL, binary_array, voltage_array);
				return -1;
			}

			break;
		case ADS_EVT_AO_HIBUF_TRANSFERED:
			printf("High Buffer Transfered!\n");

			fao_load.DataBuffer = data_buffer + (CONVERTION_NUMBER / 2);
			fao_load.start = CONVERTION_NUMBER / 2;
			fao_load.count = CONVERTION_NUMBER / 2;

/* 			tmp = DRV_FAOCheck(fd, &fao_check); */
/* 			if (tmp) { */
/* 				DRV_GetErrorMessage(tmp, err_msg); */
/* 				printf("err msg: %s\n", err_msg); */
/* 				release_region_func(fd, NULL, NULL, binary_array, voltage_array); */
/* 				return -1; */
/* 			} */
/* 			printf("overrun: %d\n", fao_check.overrun); */
/* 			printf("HalfReady: %d\n", fao_check.HalfReady); */
/* 			printf("stopped: %d\n", fao_check.stopped); */

			tmp = DRV_FAOLoad(fd, &fao_load);
			if (tmp) {
				DRV_GetErrorMessage(tmp, err_msg);
				printf("err msg: %s\n", err_msg);
				release_region_func(fd, NULL, NULL, binary_array, voltage_array);
				return -1;
			}

			break;
		case ADS_EVT_AO_UNDERRUN:
/* 			tmp = DRV_FAOCheck(fd, &fao_check); */
/* 			if (tmp) { */
/* 				DRV_GetErrorMessage(tmp, err_msg); */
/* 				printf("err msg: %s\n", err_msg); */
/* 				release_region_func(fd, NULL, NULL, binary_array, voltage_array); */
/* 				return -1; */
/* 			} */
/* 			printf("overrun: %d\n", fao_check.overrun); */
/* 			printf("HalfReady: %d\n", fao_check.HalfReady); */
/* 			printf("stopped: %d\n", fao_check.stopped); */
			printf("Underrun!\n");
			break;
		case ADS_EVT_AO_TERMINATED:
			printf("DMA terminated!\n");
			break;
		}
	}
	

	/* Step 7: Disable event */
	enable_event.EventType = ADS_EVT_AO_LOBUF_TRANSFERED;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, NULL, binary_array, voltage_array);
		return -1;
	}
		
	enable_event.EventType = ADS_EVT_AO_HIBUF_TRANSFERED;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, NULL, binary_array, voltage_array);
		return -1;
	}

	enable_event.EventType = ADS_EVT_AO_UNDERRUN;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, NULL, binary_array, voltage_array);
		return -1;
	}

	enable_event.EventType = ADS_EVT_AO_TERMINATED;
	enable_event.Enabled    = 0;
	enable_event.Count     = 1;
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		release_region_func(fd, NULL, NULL, binary_array, voltage_array);
		return -1;
	}

	/* Step 8. Stop DMA */
	DRV_FAOTerminate(fd);

 	/* Step 9: Close Device */
	free(binary_array);
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
		event->EventType = ADS_EVT_AO_LOBUF_TRANSFERED;
		event->Enabled    = 0;
		event->Count     = 1;
		DRV_EnableEvent(fd, event);

		event->EventType = ADS_EVT_AO_HIBUF_TRANSFERED;
		event->Enabled    = 0;
		event->Count     = 1;
		DRV_EnableEvent(fd, event);

		event->EventType = ADS_EVT_AO_TERMINATED;
		event->Enabled    = 0;
		event->Count     = 1;
		DRV_EnableEvent(fd, event);
	}
	
	/* terminate DMA */
	DRV_FAOTerminate(fd);

	/* free buffer */
	free(buffer0);
	free(buffer1);
	free(buffer2);

	/* close device */
	DRV_DeviceClose(&fd);
}


