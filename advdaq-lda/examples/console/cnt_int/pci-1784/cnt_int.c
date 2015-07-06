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
#include <math.h> 
#include <Advantech/advdevice.h>
#include <signal.h>

static int stop_flag = 0;
void stop_loop(int signo)
{
	stop_flag = 1;
}

int getstatus(int type,int event);
void release(int type, int event);
void* thread_counter_read(void *arg);

#define EVENT_NUM   10		/* enable event number */
#define COUNTER_NUM  4
int fdt;

int main(int argc, char *argv[])
{
	PT_CheckEvent check_event;
	PT_EnableEvent enable_event; 
	PT_ReadPortDword pt_readpdw;
	PT_WritePortDword pt_writepdw;
	int start_counter = 1;
	int counter_num = 1;
	int counter = 1;
	int start_port = 0;
	int port_num = 1 ;
	PTR_T fd = 0;
	pthread_t ntid = 0;
	FP32 clk_freq[COUNTER_NUM+1];
	INT32U pdwdata = 0;
	INT32U event = 0;
	INT32U cntr_value[COUNTER_NUM+1];
	
	INT32U  inputmode[]={1,1,1,1};
//	INT32U  indicatormode[]={1,1,1,1};
	INT32U  indicatormode[]={2,2,2,2};
	INT32U cntr_reset[]={0,0,0,0};
	INT32U cmp_data[] = {500,500,500,500};
	char *filename = NULL, ErrorMsg[100];
	int ret = 0;
	int i = 0;
	int size = 100; 
	int m_icmpdata = 0;
	int m_inputmode = 0;
	int m_timebase = 0;
	int m_dividor = 0;
	int count[EVENT_NUM];
	int type = 0;
	INT32U trig=0xffffff; /*enable all interrupt 1,2,4,8;*/
	if (argc <2) {
		printf("\nUsage: cnt_int_1784 <device node>\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}
	
	filename = argv[1]; 
		
	/* open device */
	ret = DRV_DeviceOpen(filename, &fd);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("DRV_DeviceOpen Error Msg:%s\n",ErrorMsg);
		return -1;
	}

	fdt=fd; /*for thread*/
	for(i=0;i<4;i++){
		ret = DRV_CounterReset(fd,i );
		if (ret) {
			printf("Get CFG_CounterReset err \n");
			DRV_DeviceClose(&fd);
			return -1;
		}
	}

	/* Set Compare data */
	ret = DRV_DeviceSetProperty(fd, CFG_CntrCompareData, cmp_data, size);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("DRV_DeviceSetProperty Error Msg:%s\n",ErrorMsg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	

        /* Set Counter Reset Value and InputMode */
        size=sizeof(cntr_reset);
        ret = DRV_DeviceSetProperty(fd, CFG_CntrResetValue, cntr_reset, size);
        if (ret) {
                DRV_GetErrorMessage(ret, ErrorMsg);
                printf("DRV_DeviceSetProperty Error Msg:%s\n",ErrorMsg);
                DRV_DeviceClose(&fd);
                return -1;
        }
        size = sizeof(indicatormode);
        ret = DRV_DeviceSetProperty(fd, CFG_CntrIndicatorControl, indicatormode, size);
        if (ret) {
                DRV_GetErrorMessage(ret, ErrorMsg);
                printf("DRV_DeviceGetProperty CFG_CntrIndicatorControl Error Msg:%s\n",ErrorMsg);
                DRV_DeviceClose(&fd);
                return ret;
        }
	
	/* Set counter input mode */
        size = sizeof(inputmode);
        ret = DRV_DeviceSetProperty(fd, CFG_CntrInputModeControl, inputmode, size);
        if (ret) {
                DRV_GetErrorMessage(ret, ErrorMsg);
                printf("DRV_DeviceGetProperty CFG_CntrInputModeControl Error Msg:%s\n",ErrorMsg);
                DRV_DeviceClose(&fd);
                return ret;
        }
       
	 /*  create CounterRead thread */
	if(type==0){ 
        ret = pthread_create(&ntid, NULL, thread_counter_read, &counter);
                if (ret) {
                        DRV_GetErrorMessage(ret, ErrorMsg);
                        printf("Create Thread err msg: %s\n", ErrorMsg);
                        DRV_DeviceClose(&fd);
                        return ret;
                }
	} 
	size =sizeof(trig);
	ret = DRV_DeviceSetProperty(fd, CFG_InterruptTriggerSource, &trig, size);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("DRV_DeviceSetProperty Error Msg:%s\n",ErrorMsg);
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
		DRV_DeviceClose(&fd);
		return -1;
	}
	
	/* Counter Interrupt */
	if(type==0){
		size=sizeof(cmp_data);	
	
		event = ADS_EVT_OVERCOMPARE_CNT1;
		ret = DRV_CntrEnableEventAndSpecifyEventCounter(fd,event,start_counter, counter_num);
		if (ret) {
			DRV_GetErrorMessage(ret, ErrorMsg);
			printf("Enable Event failed! error msg:%s\n",  ErrorMsg);
			DRV_DeviceClose(&fd);
			return -1;
                }
	
	}else if(type == 1){//FOR DI interrupt
	
		event= ADS_EVT_DI_INTERRUPT0;
		ret = DRV_DioEnableEventAndSpecifyDiPorts(fd, event,start_port,port_num);
		if (ret ) {
			DRV_GetErrorMessage(ret, ErrorMsg);
			printf("DRV_DioEnableEventAndSpecifyDiPorts Error Msg:%s\n",ErrorMsg);
			DRV_DeviceClose(&fd);
			return -1;
		}
	}
	
        /* check event */
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
                        DRV_DeviceClose(&fd);
		}
		
		switch (check_event.EventType) {
		case ADS_EVT_DI_INTERRUPT0:
			printf("DI0 interrupt!\n");
			ret = getstatus(fd,check_event.EventType);
			if (ret) {  
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_DI_INTERRUPT1:
			printf("DI1 interrupt!\n");
			ret = getstatus(fd,check_event.EventType);
			if (ret) {
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_DI_INTERRUPT2:
			printf("DI2 interrupt!\n");
			ret = getstatus(fd,check_event.EventType);
			if (ret) {
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_DI_INTERRUPT3:
			printf("DI3 interrupt!\n");
			ret = getstatus(fd,check_event.EventType);
			if (ret) {
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_OVERFLOW_CNT0:
		case ADS_EVT_OVERFLOW_CNT1:
		case ADS_EVT_OVERFLOW_CNT2:
		case ADS_EVT_OVERFLOW_CNT3:
			printf("Counter%d OverFlow!\n",counter);
			ret = getstatus(fd,check_event.EventType);
			if (ret) {
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_UNDERFLOW_CNT0:
		case ADS_EVT_UNDERFLOW_CNT1:
		case ADS_EVT_UNDERFLOW_CNT2:
		case ADS_EVT_UNDERFLOW_CNT3:
			printf("Counter%d UnderFlow!\n",counter);
			count[5]++;
			ret = getstatus(fd,check_event.EventType);
			if (ret) {
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_OVERCOMPARE_CNT0:
		case ADS_EVT_OVERCOMPARE_CNT1:
		case ADS_EVT_OVERCOMPARE_CNT2:
		case ADS_EVT_OVERCOMPARE_CNT3:
			printf("Counter%d OverCompare!\n",counter);
			count[6]++;
			ret = getstatus(fd,check_event.EventType);
			if (ret) {
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_UNDERCOMPARE_CNT0:
		case ADS_EVT_UNDERCOMPARE_CNT1:
		case ADS_EVT_UNDERCOMPARE_CNT2:
		case ADS_EVT_UNDERCOMPARE_CNT3:
			printf("Counter%d UnderCompare!\n",counter);
			count[7]++;
			ret = getstatus(fd,check_event.EventType);
			if (ret) {
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_INDEX_CNT0:
		case ADS_EVT_INDEX_CNT1:
		case ADS_EVT_INDEX_CNT2:
		case ADS_EVT_INDEX_CNT3:
			printf("Counter%d Index!\n",counter);
			count[8]++;
			ret = getstatus(fd,check_event.EventType);
			if (ret) {
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_INTERRUPT_TIMER4:
			printf("Timer4 Interrupt!\n");
			count[9]++;
			ret = getstatus(fd,check_event.EventType);
			if (ret) {
				release(fd, check_event.EventType);
				return ret;
			}
			break;
		case ADS_EVT_TIME_OUT:
			printf("Check Time Out!\n");
			break;
		default:
			printf("No Event!\n");

			continue;			
		}	     

	}

	ret = DRV_CounterReset(fd,counter);
	if (ret) {
		DRV_GetErrorMessage(ret, ErrorMsg);
		printf("CounterReset Error Msg:%s\n",ErrorMsg);
	}	

	release(type, event);

	return 0;
}

void release(int fd, int event)
{
	int ret = 0, i =0;
	char ErrorMsg[100];
	
	/* disable event */

	if (event>=ADS_EVT_DI_INTERRUPT0&&event<=ADS_EVT_DI_INTERRUPT3){
		ret=DRV_DioDisableEvent(fd,event);
		if (ret) {
			DRV_GetErrorMessage(ret, ErrorMsg);
			printf("DioDisable DI%d Event Error Msg:%s\n",i,ErrorMsg);
		}
	}else{
		ret = DRV_CntrDisableEvent(fd,event);
		if (ret) {
			DRV_GetErrorMessage(ret, ErrorMsg);
			printf("CntrDisable Error Msg:%s\n",ErrorMsg);
		}
	}

        pthread_exit(0);
	/* step 5: close device */
	DRV_DeviceClose(&fd);

}


/* When check event, get the latest di and counter value */
int getstatus(int fd,int event)
{
	char ErrorMsg[100];
	int i = 0, ret = 0;	
	char di_data[4];
	INT32U cntr_data[4];
	int len;
	len = sizeof(di_data);
	memset(cntr_data, 0, 4*sizeof(INT8U));	
	memset(di_data, 0, 4*sizeof(INT8U));	

	/* DI Interrupt */
	if (event>=ADS_EVT_DI_INTERRUPT0&&event<=ADS_EVT_DI_INTERRUPT3){
		ret=DRV_DioGetLatestEventDiPortsState(fd, event, di_data, len);
		if (ret) {
			DRV_GetErrorMessage(ret, ErrorMsg);
			printf("DRV_DioGetLatestEventDiPortsState Error Msg:%s\n",ErrorMsg);
			return ret;
		}
		
		for(i=0;i<4;i++){
			printf("DI%d data:%x\n", i, di_data[i]);
		}
	}else{/* Counter and Timer4 Interrupt */
		ret=DRV_CntrGetLatestEventCounterValue(fd, event, cntr_data, len);
		if (ret) {
			DRV_GetErrorMessage(ret, ErrorMsg);
			printf("DRV_CntrGetLatestEventCounterValue Error Msg:%s\n",ErrorMsg);
			return ret;
		}
		for(i=0;i<4;i++){
			printf("Counter%d Value:%u\n",i, cntr_data[i]);
		}
	}

	return 0;
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
