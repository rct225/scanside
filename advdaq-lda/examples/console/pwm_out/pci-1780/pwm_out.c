/*
 * **********************************************************
 * Program     : pwm_out.c
 * Description : Demo program for counter PWM out function
 * Boards supp.:
 * APIs used   : DRV_DeviceOpen, DRV_DeviceClose
 *               DRV_CounterPWMSetting
 *               DRV_CounterPWMEnable
 *               DRV_CounterReset
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
	PT_CounterPWMSetting counter_pwm_setting;
	unsigned short counter;
	char *filename = NULL;
	char err_msg[100];
	int tmp;
	PTR_T fd;


	/* get argument */
	if (argc < 2) {
		printf("Usage: pwm_out <device node> [counter index]\n");
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
	

	/* Step 2: Config Counter PWM */
	memset(&counter_pwm_setting, 0, sizeof(PT_CounterPWMSetting));
	counter_pwm_setting.Port = counter;
	counter_pwm_setting.Period  = 1; /* seconds */
	counter_pwm_setting.HiPeriod = 0.5; /* should be > 0.5 */
	counter_pwm_setting.OutCount = 0; /* 0 for cyclic mode */
	counter_pwm_setting.GateMode = 0;

	tmp = DRV_CounterPWMSetting(fd, &counter_pwm_setting);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}

	/* Step 3: Enable Counter PWM Out */
	tmp = DRV_CounterPWMEnable(fd, counter);
	if (tmp) {
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	printf("PWM Out...(press any key to stop)\n");
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break; 
	}


	/* Step 4: Reset Counter */
	tmp = DRV_CounterReset(fd, counter);
	if (tmp) {
		DRV_DeviceClose(&fd);
		return -1;
	}
	
		
	/* Step 5: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0;
} 

