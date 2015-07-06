/*
 * **********************************************************
 * Program     : freq_out.c
 * Description : Demo program for counter pulse out function
 * Boards supp.:
 * APIs used   : DRV_DeviceOpen, DRV_DeviceClose
 *               DRV_CounterPulseStart
 *               
 * Revision    : 1.00
 * Date        : 09/05/2006
 * Author      : LI.ANG
 * **********************************************************
 * */
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
 

/* for test */
#include <signal.h>

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}
/* end */

int main(int argc, char *argv[])
{
	PT_FreqOutStart freq_out_start;
	unsigned short counter;
	char *filename = NULL;
	char err_msg[100];
	int tmp;
	PTR_T fd;


	/* get argument */
	if (argc < 2) {
		printf("Usage: freq_out <device node> [counter index]\n");
		return -1;
	}
	filename = argv[1];

	if (argv[2] != NULL) {
		counter = (unsigned short) atoi(argv[2]);
	} else {
		counter = 0;
	}


	/* Step 1: Open Device */
	tmp = DRV_DeviceOpen(filename, &fd);
	if (tmp) {
		return -1;
	}
	

	/* Step 2: Start Pulse Out */
	memset(&freq_out_start, 0, sizeof(PT_FreqOutStart));
	freq_out_start.usChannel = counter;
	freq_out_start.usDivider = 1; /* 1~16 */
	freq_out_start.usFoutSrc = PA_FOUT_SRC_CLK_1KHZ; /* refer to manual */

	tmp = DRV_FreqOutStart(fd, &freq_out_start);
	if (tmp < 0) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	printf("Frequency out...(press any key to stop)\n");
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break; 
	}

	/* Step 3: Stop Frequency Out */
	tmp = DRV_FreqOutReset(fd, counter);
	if (tmp < 0) {
		DRV_DeviceClose(&fd);
		return -1;
	}
	
		
	/* Step 4: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0;
}
