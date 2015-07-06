#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/mman.h>
#include <termios.h>
#include <signal.h>
#include <Advantech/advdevice.h>

#define  TIMER_BASE_50KHZ 50000.0
#define  TIMER_BASE_5KHZ  5000.0
#define  TIMER_BASE_500HZ 500.0
#define  TIMER_BASE_50HZ  50.0
#define  TIMER_BASE_5HZ   5.0

static int stop_flag = 0;
void stop_loop(int signo)
{
	stop_flag = 1;
}

int main(int argc, char *argv[])
{
	PTR_T fd = 0;
	char err_msg[100];
	struct timeval prev, now;
	unsigned int usDividor = 0;
	int i = 0;
	int ret = 0;
	int count = 0;
	int m_iUnit = 0;
	int size = 100;
	int time_interval = 0;
	float time =0;
	int fFreq = 0;
	float fFreqBase = 0;
	float fTimeFreq[5];
	INT32U trig=0xffffffff;
	INT32U inputmode[]={1,1,1,1};
	PT_EnableEvent enable_event;
	PT_CheckEvent check_event;
	PT_TimerCountSetting tmr_count_setting;

	if(argc < 2) {
		printf("\nUsage: TmrCntSet_1784 <device node>\n");
		return -1;
	}
 
	/* step 1: open device */
	ret = DRV_DeviceOpen(argv[1], &fd);
	if(ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("Open the device Failed! %s\n",err_msg);
		return -1;
	}
	
	for(i=0;i<4;i++){
		ret = DRV_CounterReset(fd,i );
		if (ret) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("Get CFG_CounterReset err msg: %s\n", err_msg);
			DRV_DeviceClose(&fd);
			return -1;
		}
	}

	/* step 2: set device's property CFG_CntrClockFrequecny */
	time_interval=100000;
	
	fFreq=(float)(1000000/time_interval);

	ret = DRV_DeviceGetProperty(fd, CFG_CntrClockFrequency, fTimeFreq, &size);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("Get CFG_CntrClockFrequency err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	fTimeFreq[0] = fFreq;
	
	if (fFreq > TIMER_BASE_5KHZ){
		fFreqBase = TIMER_BASE_50KHZ;
	}else if((fFreq <= TIMER_BASE_5KHZ) && (fFreq > TIMER_BASE_500HZ)){
		fFreqBase = TIMER_BASE_5KHZ;
	}else if ((fFreq <= TIMER_BASE_500HZ) && (fFreq > TIMER_BASE_50HZ)){
		fFreqBase = TIMER_BASE_500HZ;
	}else if ((fFreq <= TIMER_BASE_50HZ) && (fFreq > TIMER_BASE_5HZ)){
		fFreqBase = TIMER_BASE_50HZ;
	}else{
		fFreqBase = TIMER_BASE_5HZ;
	}
	
	fTimeFreq[4] = fFreqBase;
	
	ret = DRV_DeviceSetProperty(fd, CFG_CntrClockFrequency, fTimeFreq, size);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("Set CFG_CntrClockFrequency err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	usDividor = (USHORT)(time_interval * fFreqBase / 1000000 + 0.5);
	if (usDividor == 0){
		usDividor = 1;
	}
	
	// Real timer interval
	time_interval = 1000000 / fFreqBase * usDividor;
	if (time_interval >= 1000000)	{
		time_interval = time_interval / 1000000;
	}else if ((time_interval >= 1000) && (time_interval < 1000000)){
		time_interval = time_interval / 1000;
	}
	
	size = sizeof(inputmode);
	ret = DRV_DeviceSetProperty(fd, CFG_CntrInputModeControl,inputmode , size);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("Set CFG_CntrClockFrequency err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	ret = DRV_DeviceSetProperty(fd, CFG_InterruptTriggerSource,&trig , 4);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("Set CFG_CntrClockFrequency err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	tmr_count_setting.counter = 4;
	tmr_count_setting.Count = usDividor;
	
	ret = DRV_TimerCountSetting(fd, &tmr_count_setting);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("DRV_TimerCountSetting err msg: %s,%d\n", err_msg,ret);
		DRV_DeviceClose(&fd);
		return -1;
	}

	/* step 3: Enable Event */
	enable_event.EventType = ADS_EVT_INTERRUPT_TIMER4;
	enable_event.Enabled   = 1;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("Enable Event failed! error msg:%s\n",  err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	/* step 4: wait event */
	printf("Waitting for Event, Sleepping... (Ctrl-C to stop)\n");     
	signal(SIGINT, stop_loop);
	gettimeofday(&prev, NULL);

	while (1) {
		if (stop_flag){
			printf("Out of loop!\n");	
			break; 
		}
		
		check_event.EventType = 0;
		check_event.Milliseconds = 1000000;
		ret = DRV_CheckEvent(fd, &check_event);
		if (ret ) {
			DRV_GetErrorMessage(ret, err_msg);
			printf("CheckEvent err msg: %s\n", err_msg);
			break;
		}
		
		switch(check_event.EventType)		{
		case ADS_EVT_INTERRUPT_TIMER4:
			//get the interval time between two event
			count++;
			gettimeofday(now, NULL);
       			time = (now.tv_sec - prev.tv_sec) + (now.tv_usec - prev.tv_usec) / (double) 1000000;
			printf("Run time = %f s\n",time);
			printf("%d:ADS_EVT_INTERRUPT_TIMER1 Interrupt \n",count);
			break;
			
		case ADS_EVT_TIME_OUT:
			printf("Time Out\n");
			break;
			
		default:
			printf("No Event\n");
			break;
		}
	}

	/* step 4: disable event */
	enable_event.EventType = ADS_EVT_INTERRUPT_TIMER4;
	enable_event.Enabled   = 0;
	enable_event.Count     = 1;
	ret = DRV_EnableEvent(fd, &enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, err_msg);
		printf("Disable Event failed! err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	/* step 5: close device */

	DRV_DeviceClose(&fd);

	return 0;
} 

