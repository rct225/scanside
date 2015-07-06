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
//	Filename   :	pwm_out.c                                                          |
//	Version	   : 	1.0.0                                                              |
//	Date	   :	11/17/2007                                                         |
//                                                                                         | 
//	Description: 	Demo program for counter PWM out function                          |
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

int main(int argc, char *argv[])
{
	PT_CounterPWMSetting counter_pwm_setting;
	unsigned short counter;
	char *filename = NULL;
	char err_msg[100];
	float hi_period;
	float period;
	int fd;
	int ret;


	
	/* get argument */
	if (argc < 2) {
		printf("Usage: pwm_out <device node> [counter index]\n");
		return -1;
	}
	filename = argv[1];

	if (argc > 2) {
		counter = (unsigned short) atoi(argv[2]);
	} else {
		counter = 0;
	}


	/* Step 1: Open Device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}
	
        hi_period = 5000;	/* 500000us */
        period = 10000;		/* 1000000us */
	
	/* Step 2: Config Counter PWM */
	memset(&counter_pwm_setting, 0, sizeof(PT_CounterPWMSetting));
	counter_pwm_setting.Port = counter;
	counter_pwm_setting.HiPeriod = hi_period;
	counter_pwm_setting.Period = period;
        counter_pwm_setting.OutCount = 1000; 
        counter_pwm_setting.GateMode = 100;
	stop_flag = 0;

	printf("PWM out...(press any key to stop)\n");
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag) {
			printf("Out of loop, safe!\n");
			break;
		}

		ret = DRV_CounterPWMSetting(fd, &counter_pwm_setting);
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}

	        ret = DRV_CounterPWMEnable(fd, counter);	
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("err msg: %s\n", err_msg);

			DRV_DeviceClose(&fd);
			return -1;
		}
		printf("High Period: %.4fus, Period: %.4fus\n",
		       hi_period * 100,
		       period) * 100;

		usleep(500000); /* accord your signal frequence
				 * calling DRV_PWMStartRead
				 * before counter overflow */
	}

	/* Step 3: Reset Counter */
	ret = DRV_CounterReset(fd, counter);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("err msg: %s\n", err_msg);

		DRV_DeviceClose(&fd);
		return -1;
	}
		
	/* Step 5: Close Device */
	DRV_DeviceClose(&fd);
	
	return 0;
} 
