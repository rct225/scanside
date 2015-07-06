/*
 * ****************************************************
 * Program     : do_soft_wd.c
 * Description : Demo program for digital output with
 *               watchdog function
 * Boards supp.:
 * APIs used   : DRV_DeviceOpen, DRV_DeviceClose
 *               AdxDioGetCurrentDoPortsState
 *               DRV_WatchdogStart
 *               DRV_WatchdogFeed
 *               DRV_WatchdogStop
 *               DRV_DeviceSetProperty
 *               DRV_EnableEvent
 *               DRV_CheckEvent
 *               
 * Revision    : 1.00
 * Date        : 09/19/2006
 * Author      : LI.ANG
 * ***************************************************
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
#include <termios.h>
#include <pthread.h>
#include <Advantech/advdevice.h>

/* define follow to achieve press anykey quit circle */
#include <signal.h>

static int stop_flag;
void stop_loop(int signo)
{
	stop_flag = 1;
}
static int endwin_flag;
struct termio save;
int kbhit(void);
void initscr(void);
void endwin(void);
/* end */
void *thread_feed_watchdog(void *arg);


#define DO_PORT_NUM    8

int main(int argc, char *argv[])
{
	PT_EnableEvent enable_event;
	PT_CheckEvent check_event;
	unsigned long wd_counter;
	unsigned short port;
	unsigned char buffer;
	unsigned char do_preset_value[DO_PORT_NUM];
	unsigned char do_overflow_value[DO_PORT_NUM];	
	pthread_t ntid;
	char *filename;
	char err_msg[100];
	int i;
	int tmp;
	int fd;

     

	if (argc < 2) {
		printf("\nUsage: do_soft_wd <device node> [output port]\n");
		printf("\nPlease input a relative DEVICE NODE!\n\n");
		exit(1);
	}

	if (argv[2] != NULL) {
		port = (unsigned short) atoi(argv[2]);
	} else {
		port = 0;
	}


	filename = argv[1]; 
	/* step 1: open device */
	tmp = DRV_DeviceOpen(filename, &fd);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		return -1;
	}

	/* step 2: set parameters for do */
	for (i = 0; i < DO_PORT_NUM; i++) {
		do_preset_value[i] = 0x01;
	}
	DRV_DeviceSetProperty(fd, CFG_DoPresetValue,
			      &do_preset_value, DO_PORT_NUM);

	for (i = 0; i < DO_PORT_NUM; i++) {
		do_overflow_value[i] = 0x01;
	}
	tmp = DRV_DeviceSetProperty(fd, CFG_DoWatchdogValue,
				    &do_overflow_value, DO_PORT_NUM);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	for (i = 0; i < DO_PORT_NUM; i++) {
		do_overflow_value[i] = 0x00;
	}
	tmp = DO_PORT_NUM;
	tmp = DRV_DeviceGetProperty(fd, CFG_DoWatchdogValue, do_overflow_value, &tmp);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);


		DRV_DeviceClose(&fd);
		return -1;
	}
	for (i = 0; i < DO_PORT_NUM; i++) {
		printf("do_overflow_value[%d]: 0x%x\n", i, do_overflow_value[i]);
	}


        /* step 3: enable event */
	memset(&enable_event, 0, sizeof(PT_EnableEvent));
	enable_event.EventType = ADS_EVT_WATCHDOG_OVERRUN;
	enable_event.Enabled = 1;
	enable_event.Count = 1;
	
	tmp = DRV_EnableEvent(fd, &enable_event);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	/* step 4: set watchdog counter */
	wd_counter = 10000000;	/* 1 second */
	tmp = DRV_DeviceSetProperty(fd, CFG_WatchdogCounter, &wd_counter, sizeof(unsigned long));
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

	/* step 5: start watchdog */
	tmp = DRV_WatchdogStart(fd);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}
	

	/* step 6: create feed watchdog thread */
	tmp = pthread_create(&ntid, NULL, thread_feed_watchdog, &fd);
	if (tmp) {
		DRV_GetErrorMessage(tmp, err_msg);
		printf("err msg: %s\n", err_msg);
		DRV_DeviceClose(&fd);
		return -1;
	}

		
	/* step 7: check event */
	printf("Waitting for Event, Sleepping... (Ctrl-C to stop)\n");
	signal(SIGINT, stop_loop);
	stop_flag = 0;

	while (1) {
		if (stop_flag)
			break;

		check_event.EventType = 0; /* check all */
		check_event.Milliseconds = 0;
	     
		tmp = DRV_CheckEvent(fd, &check_event);
		if (tmp) {
			continue;
		}

		switch (check_event.EventType) {
		case ADS_EVT_WATCHDOG_OVERRUN:
			DRV_DioReadDOPorts(fd, port, 1, &buffer);
			printf("\nWatchdog overflow, DO readback: 0x%.2x\n\n", buffer);
			printf("\nWatchdog stoped, Ctrl-C to terminate the process\n\n");
			break;
		default:
			break;
		}
	}
	if (!endwin_flag) {
		endwin();
	}
	
	
	/* step 8: stop watchdog */
	DRV_WatchdogStop(fd);
     	  
	/* setp 9: close device */
	DRV_DeviceClose(&fd);

	return 0;
} 

void *thread_feed_watchdog(void *arg)
{
	int tmp;
	int fd;
	

	fd = *((int *) arg);

	initscr();
	while (1) {
		if (kbhit()) {
			break;
		}

		tmp = DRV_WatchdogFeed(fd);
		if (tmp < 0) {
			printf("feed watchdog failed!\n");
			pthread_exit(0);
		}

		printf("feeding watchdog... (press any key to stop)\n");
		usleep(500000);	/* feed watchdog every 500ms */
	}
	endwin_flag = 1;
	endwin();
}

int kbhit()
{
	int n;

	ioctl(0, FIONREAD, &n);

	return n;
}

void endwin()
{
	ioctl(0, TCSETA, &save);
}

void initscr()
{
	struct termio term;
	if (ioctl(0, TCGETA, &term) == -1) {
		fprintf(stderr,"standard input not a tty\n");
		exit(1);
	}
	save = term;
	term.c_lflag &= ~ICANON;
	term.c_lflag &= ~ECHO;
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 0;

	ioctl(0, TCSETA, &term);
}
