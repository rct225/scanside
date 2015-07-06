/*
 * *****************************************************************
 * Program     : cntrcompare.c 
 * Description : Demo program for counter compare with interrupt mode
 *               function FOR PCI-1784
 * Boards supp.:
 * APIs used   : DRV_DeviceOpen, DRV_DeviceClose
 *               DRV_CheckEvent
 *               
 * Revision    : 1.00
 * Date        : 03/06/2009
 * Author      : Luo.Jing
 * *****************************************************************
 * */
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
#include <Advantech/advdevice.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>

static int stop_flag = 0;
void stop_loop(int signo)
{
	stop_flag = 1;
}


void release(int fd,int event);
void* thread_counter_read(void *arg);
int changecmpdata(int index, int* cmpdata);

#define EVENT_NUM   2		/* enable event number */
#define COUNTER_NUM  4
int fdt;

int main(int argc, char *argv[])
{ 
	PT_CheckEvent check_event; 
	PT_EnableEvent enable_event; 
	PT_CounterEventStart ptcntrstart;
	 PTR_T fd = 0;
	 int counter = 0;

	pthread_t ntid = 0;
	
	INT32U do_indicator[]={0,0,0,0};
	INT32U*cmp_table;
	char *filename = NULL, ErrorMsg[100];
	INT32U cntr_reset[]= {0,0,0,0};
	int ret = 0;
	int event = 0;
	int i = 0;
	int size = 100;
	int count[EVENT_NUM];
	int cmpnum = 0;
	int m_dogroup = 0;
	int m_inputmode = 0;
	int m_doindicated = 0;
	int m_cmptype = 0;
	int m_inttype = 0;
	INT32U m_cmpdata[]={500,500,500,500};
	INT32U trig=0xffffff;		/*enable all interrupt 1,2,4,8;*/
	INT32U inputmode[]={1,1,1,1};
	if (argc <2) {
		printf("\nUsage: cntrcompare <device node>\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}
	
	filename = argv[1]; 
	if (argv[2] != NULL) {
		counter = (unsigned short) atoi(argv[2]);
	} else {
		counter = 0;
	}
	
	/* step 1: open device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("DRV_DeviceOpen Error Msg:%s\n",ErrorMsg);
		return -1;
	}
	
	for(i=0;i<4;i++){
		ret = DRV_CounterReset(fd,i );
		if (ret) {
			printf("Get CFG_CounterReset err \n");
			DRV_DeviceClose(&fd);
			return -1;
		}
	}

	fdt = fd;		/*for thread*/

	/* Set Counter Reset Value */
	ret = DRV_DeviceSetProperty(fd, CFG_CntrResetValue, cntr_reset, size);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("DRV_DeviceSetProperty Error Msg:%s\n",ErrorMsg);
		DRV_DeviceClose(&fd);
		return -1;
	}
		
         /* Set counter input mode */
        inputmode[counter] = 1;
        ret = DRV_DeviceSetProperty(fd, CFG_CntrInputModeControl, inputmode, size);
        if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
                printf("DRV_DeviceSetProperty CFG_CntrInputModeControl Error Msg:%s\n",ErrorMsg);
                DRV_DeviceClose(&fd);
                return ret;
        }

	/* Set Compare data */
	size =sizeof(m_cmpdata);
	ret = DRV_DeviceSetProperty(fd, CFG_CntrCompareData, m_cmpdata, size);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("DRV_DeviceSetProperty Error Msg:%s\n",ErrorMsg);
		return ret;
	}	

	/* Set CFG_CntrIndicatorControl */
	size = sizeof(do_indicator);
		ret = DRV_DeviceSetProperty(fd, CFG_CntrIndicatorControl, do_indicator, size);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("DRV_DeviceSetProperty CFG_CntrIndicatorControl Error Msg:%s\n",ErrorMsg);
		DRV_DeviceClose(&fd);
		return ret;
	}	

	/* Choose interrupt event type */
	size =sizeof(trig);
	ret = DRV_DeviceSetProperty(fd, CFG_InterruptTriggerSource, &trig, size);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("DRV_DeviceSetProperty Error Msg:%s\n",ErrorMsg);
		return ret;
	}	

        /* step 5: create CounterRead thread */
        ret = pthread_create(&ntid, NULL, thread_counter_read, &counter);
		if (ret) {
			DRV_GetErrorMessage(ret, ErrorMsg);
                	printf("Create Thread err msg: %s\n", ErrorMsg);
			DRV_DeviceClose(&fd);
			return ret;
		}
	
	/* Enable Event */
	enable_event.EventType = ADS_EVT_OVERCOMPARE_CNT0;
	enable_event.Enabled = 1;	
	enable_event.Count = 1;	
	ret = DRV_EnableEvent(fd,&enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("Enable Event failed! error msg:%s\n",  ErrorMsg);
		pthread_exit(0);
		DRV_DeviceClose(&fd);
		return -1;
        }


	/* Start CounterEvent */
	ptcntrstart.counter=counter;
	ret = DRV_CounterEventStart(fd, &ptcntrstart);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("DRV_CounterEventStart failed! error msg:%s\n",  ErrorMsg);
		release(fd,event);
		return -1;
       }


        /* step 4: check event */
	printf("Waitting for Event, Sleepping... (Ctrl-C to stop)\n");     
	signal(SIGINT, stop_loop);
	
	while (1) {
		if (stop_flag){
			printf("Out of loop!\n");
			break;
		}
		check_event.EventType = 0; /* 0 - check all */
		check_event.Milliseconds =1000;
	     
		ret = DRV_CheckEvent(fd, &check_event);
		if (ret) {
			DRV_GetErrorMessage(ret, ErrorMsg);
			printf("DRV_CheckEvent Error Msg:%s\n",ErrorMsg);
		}
		
		switch (check_event.EventType) {
		
		case ADS_EVT_UNDERCOMPARE_CNT0:
		case ADS_EVT_UNDERCOMPARE_CNT1:
		case ADS_EVT_UNDERCOMPARE_CNT2:
		case ADS_EVT_UNDERCOMPARE_CNT3:
			printf("Counter%d UnderCompare!\n",counter);
			
		break;
		case ADS_EVT_OVERCOMPARE_CNT0:
		case ADS_EVT_OVERCOMPARE_CNT1:
		case ADS_EVT_OVERCOMPARE_CNT2:
		case ADS_EVT_OVERCOMPARE_CNT3:
			printf("Counter%d OverCompare!\n",counter);
				
			break;
		case ADS_EVT_TIME_OUT:
			printf("Check Time Out!\n");
			break;
		default:
			break;
			
		}	     
	}
	
	ret = DRV_CounterReset(fd,counter);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("CounterReset Error Msg:%s\n",ErrorMsg);
	}	

	release(fd,event);

	return 0;
}

void release(int fd,int event)
{
	int ret = 0;
	char ErrorMsg[100];
	PT_EnableEvent enable_event; 
        
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	
	/* disable event */
	enable_event.EventType = event;
	enable_event.Enabled = 0;
	enable_event.Count = 1;
	
	ret = DRV_EnableEvent(fd,&enable_event);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("Enable Error Msg:%s\n",ErrorMsg);
	}
	
	pthread_exit(0);

	/* step 5: close device */
	DRV_DeviceClose(&fd);

}




void* thread_counter_read(void *arg)
{
        int tmp = 0,i = 0;
        char err_msg[100];
        PT_CounterEventRead counter_event_read;
        unsigned short overflow = 0;
        unsigned short counter = 0;
        unsigned int count=0;
        counter = *((int*)arg);

        memset(&counter_event_read, 0, sizeof(PT_CounterEventRead));
        counter_event_read.counter = counter;
        counter_event_read.count = &count;
        counter_event_read.overflow = &overflow;

        while (1) {
		if (stop_flag){
			printf("Out of thread!\n");
			break;
		}
                i++;
                tmp = DRV_CounterEventRead(fdt, &counter_event_read);
                if (tmp) {
                        DRV_GetErrorMessage(tmp, err_msg);
                        printf("CounterEventRead err msg: %s\n", err_msg);
                	return (void*)tmp;
		}
 
                printf("[%d] Read Counts : %d, Overflow: %d\n",i, count,overflow);
                usleep(980000); /* accord to frequence of event,
                                  * calling DRV_CounterEventRead() before
                                  * counter overflow,
                                  * otherwise count number will be incorrectness */
 
        }
	return (void*)0;
}

