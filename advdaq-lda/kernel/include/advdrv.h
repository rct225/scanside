/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech USB Device Driver for Linux

File Name:
	advdrv.h
Abstract:
	Header file for USB device driver. 
Version history
	03/20/2006		Create by zhiyong.xie

************************************************************************/	

 
#ifndef __ADVDRV_H__
#define __ADVDRV_H__


#include <linux/poll.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <linux/usb.h>
#include <linux/pci.h>
#include <linux/workqueue.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
#include <linux/config.h>
#endif

#include "../../include/advcpu.h"
#include "../../include/usbctrlcode.h"
#include "../../include/usberrorcode.h"
#include "../../include/usbinfo.h"
#include "../../include/usbio.h"
#include "isa.h"

#define VENDOR_PCI_ID_ADVANTECH			0x13fe
#define VENDOR_ID_ADVANTECH                     VENDOR_PCI_ID_ADVANTECH

#define VENDOR_USB_ID_ADVANTECH                 0x1809

#define GET_DYNAMIC_MAJOR	0

#define PCI_BAR_WIDTH		0x04
#define PCI_NUM_BARS		6
#define ADV_NDEVICES		16


/* for specify device type */
#define ADV_PCI_DEVICE          0
#define ADV_USB_DEVICE          1

#define ADV_PCI_DRIVER          ADV_PCI_DEVICE
#define ADV_USB_DRIVER          ADV_USB_DEVICE

#define ADV_SYS_NODE            254

/* for name buffer length */
#define DRIVER_NAME_LENGTH      30
#define DEVICE_NAME_LENGTH      30
#define DEVICE_NODE_NAME_LENGTH 100

/* for advantech coding debug */
#define DEBUG_LINE_ERROR        0
#define DEBUG_FUNC_ERROR        1

/*Read or write IO port */
#define advInp(privdata, port )      \
   inb(privdata->iobase + port )
   
#define advInpw(privdata, port )      \
   inw(privdata->iobase + port )
#define advInpdw(privdata, port )      \
   inl(privdata->iobase + port )
   
#define advOutp(privdata, port, data )  \
   outb(data,privdata->iobase + port )
#define advOutpw(privdata, port, data )  \
   outw(data, privdata->iobase + port )
#define advOutpdw(privdata, port, data ) \
   outl(data, privdata->iobase + port );
#define advInpDMA(privdata, port )          \
   inb(privdata->ioDMAbase + port )
                                                                                                                             
#define advInpDMAw(privdata, port )         \
   inw(privdata->ioDMAbase + port )
                                                                                                                             
#define advInpDMAdw(privdata, port )        \
   inl(privdata->ioDMAbase + port )
                                                                                                                             
#define advOutpDMA(privdata, port, data ) \
    outb(data, privdata->ioDMAbase+port )
                                                                                                                             
#define advOutpDMAw(privdata, port, data ) \
    outw(data, privdata->ioDMAbase+port )

#define advOutpDMAdw(privdata, port, data ) \
    outl(data, privdata->ioDMAbase+port )

/* for debugging messages */
#undef PCRIT
#define PCRIT(fmt, args...) printk( KERN_CRIT "advdrv: CRITICAL ERROR: " fmt, ## args)

#undef PWARN
#define PWARN(fmt, args...) printk( KERN_WARNING "advdrv: warning: " fmt, ## args)

#ifdef DEBUG
#undef DEBUG
#endif

#define DEBUG
#ifdef DEBUG
#define KdPrint(format, args...)	printk("Advantech: " format , ## args)
#define AdvDebugFunc()	                printk("%s error!\n ", __FUNCTION__);
#define AdvDebugLine()	                printk("line: %d!\n ", __LINE__);
#else
#define KdPrint(format, args...)
#endif


/* compatible with Linux kernel version 2.4 and 2.6 */
/* ######################################################################### */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#define wait_event_interruptible_timeout(wq, condition, timeout)\
        wait_event_interruptible(wq, condition)
#define irqreturn_t void
#define IRQ_RETVAL(handled)

#define adv_usb_alloc_urb(iso_packets, mem_flags) usb_alloc_urb(iso_packets)
#define adv_usb_submit_urb(purb, mem_flags) usb_submit_urb(purb)
 
#define usb_set_intfdata(interface, privdata)  do{ interface->private_data = privdata; }while(0)
#define usb_get_intfdata(interface) (interface->private_data)
#define adv_usb_ctrl_msg(dev, pipe, request, requesttype, \
                         value, index, data, size) \
        usb_control_msg(dev, pipe, request, requesttype, \
                        value, index, data, size, 10*HZ)
#define usb_kill_urb(purb)   usb_unlink_urb(purb)

/* ################### */
#else

#define adv_usb_alloc_urb(iso_packets, mem_flags) usb_alloc_urb(iso_packets, mem_flags)
#define adv_usb_submit_urb(purb, mem_flags) usb_submit_urb(purb, mem_flags)

#define adv_usb_ctrl_msg(dev, pipe, request, requesttype, \
                         value, index, data, size) \
        usb_control_msg(dev, pipe, request, requesttype, \
                        value, index, data, size, 10*HZ)
 
#endif
/* ######################################################################## */

/* for compatible */
/* #define P_PCI_DEVICE(device) ((adv_pci_device *)device) */
/* #define P_USB_DEVICE(device) ((adv_usb_device *)device) */

/* device is struct adv_device */
#define _DEV_OPS(device) (device->driver->ops) 

/* #define _1ADV_SET_DEVICE_BOARDID(x, y) ((x)->boardID = y) */

#define _ADV_SET_DEVICE_BOARDID(device, MboardID)\
        do{ device->boardID = MboardID;}while(0)

#define _ADV_GET_DEVICE_BOARDID(device, MboardID)\
        do{ MboardID = device->boardID;}while(0)

#define _ADV_SET_DEVICE_DEVNAME(device, pstring)\
        do{ memcpy(device->device_name, pstring, strlen(pstring));}while(0)
#define _ADV_GET_DEVICE_DEVNAME(device, pstring)\
        do{ memcpy(pstring, device->device_name, \
                   strlen(device->device_name));\
        }while(0)

#define _ADV_SET_DEVICE_NODENAME(device, pstring)\
        do{ memcpy(device->device_node, pstring, strlen(pstring));}while(0)
#define _ADV_GET_DEVICE_NODENAME(device, pstring)\
        do{ memcpy(pstring, device->device_node, strlen(pstring));}while(0)

#define _ADV_SET_DEVICE_IRQ(device, irq_set)\
        do{ device->irq = irq_set;}while(0)
#define _ADV_GET_DEVICE_IRQ(device, irq_set)\
        do{ irq_set = device->irq;}while(0)

#define _ADV_SET_DEVICE_IOBASE(device, iobase_set)\
        do{ device->iobase = iobase_set;}while(0)
#define _ADV_GET_DEVICE_IOBASE(device, iobase_set)\
        do{ iobase_set = device->iobase;}while(0)

#define _ADV_SET_DEVICE_SLOT(device, slot_set)\
        do{ device->slot = slot_set;}while(0)
#define _ADV_GET_DEVICE_SLOT(device, slot_set)\
        do{ slot_set = device->slot;}while(0)

#define _ADV_SET_DEVICE_BUS(device, bus_set)\
        do{ device->bus = bus_set;}while(0)
#define _ADV_GET_DEVICE_BUS(device, bus_set)\
        do{ bus_set = device->bus;}while(0)

#define _ADV_SET_DEVICE_PRIVDATA(device, privdata)\
        do{ device->private_data = privdata;}while(0)
#define _ADV_GET_DEVICE_PRIVDATA(device, privdata)\
        do{ privdata = device->private_data;}while(0)

#define _ADV_SET_DRIVER_SEMA(driver, psema)\
	do{ driver.driver_sema = psema;}while(0)
#define _ADV_GET_DRIVER_SEMA(driver, psema)\
        do{ psema = driver.driver_sema;}while(0)

#define _ADV_ENABLE_EVENT(p_process_info, event_th)\
        do{         \
             *(p_process_info->event_enabled + event_th) = 1; \
        }while(0)

#define _ADV_DISABLE_EVENT(p_process_info, event_th)\
        do{         \
             *(p_process_info->event_enabled + event_th) = 0; \
        }while(0)

#define _ADV_ISENABLE_EVENT(p_process_info, event_th) \
        (*(p_process_info->event_enabled + event_th))

#define _ADV_SET_EVENT(p_process_info, event_th) \
        do{   \
            *(p_process_info->event_status + event_th) = 1;\
         }while(0)

#define _ADV_ISSET_EVENT(p_process_info, event_th) \
        (*(p_process_info->event_status + event_th))

#define _ADV_RESET_EVENT(p_process_info, event_th) \
        do{ \
           *(p_process_info->event_status + event_th) = 0;\
         }while(0)
 
#define _ADV_SET_ERROR(p_process_info, err) \
         do{   \
             p_process_info->error = err; \
         }while(0)

#define _ADV_GET_ERROR(p_process_info, err)\
         do{        \
             err = p_process_info->error; \
         }while(0)



typedef struct _attach{
	unsigned char	attached;
}attach;


/* basic PCI operations */

/* for holding dma memory pages */
typedef struct _adv_memory{
	unsigned long size;	//memory size
	unsigned long order;	//memory page order
	void* membuf;		//memory pointer
}adv_memory;

typedef struct _adv_driver{
	struct _adv_driver *next;
	struct _adv_device *devices;
	const char * driver_name;
	const char * devfs_name;
	struct _adv_general_ops* ops;
	struct semaphore *driver_sema;
	unsigned int driver_type;
/* 	struct device_driver driver; */
	struct module *owner;
	const char *mod_name;
	
	
}adv_driver;

typedef struct _adv_device{
	struct _adv_device *next;
	struct _adv_driver *driver;//the driver object 
	unsigned int device_type;
	int     	devfs_handle;	                //Reserve to devfs
	char      device_name[DEVICE_NAME_LENGTH]; //the device name
	char      device_node[DEVICE_NODE_NAME_LENGTH]; /* device node binded
							 * with this device*/
	unsigned short boardID;
	int 		config;

	/* for PCI Device Compatible with old drivers */
	struct pci_dev 	*pci_dev;	
	unsigned long iobase;
	unsigned long irq;
	unsigned long irq_ext;
	unsigned short bus;
	unsigned short slot;
	

	/* for PCI Device end */

	/* for USB Device Compatible with old drivers */

	struct usb_device 	        *udev;	       //the usb device for this device
	struct usb_interface       *interface;     //the interface for this device
	/* for USB Device end */

	void		*private_data;	        //private data
     
}adv_device, adv_usb_device;

/* typedef struct _adv_pci_device{ */
/*      struct _adv_base_device base_device; */
/*      struct pci_dev 	*pci_dev;	 */
/*      unsigned long iobase; */
/*      unsigned char slot; */
/*      void		*private_data;	        //private data */
/* }adv_device, adv_pci_device; */


/* typedef struct _adv_usb_device{ */
/*      struct _adv_base_device base_device; */
/*      struct usb_device 	        *udev;	       //the usb device for this device */
/*      struct usb_interface       *interface;     //the interface for this device */
/*      void		*private_data;	        //private data */
/* }adv_usb_device; */

typedef struct _adv_device_loc {
	unsigned int	minor;
	unsigned short	devcnf;
	unsigned int    device_type; /* PCI_DEVICE or USB_DEVICE and so on */
	adv_driver      *driver_ptr;
	adv_device   	*device_ptr;
	atomic_t        dev_count;

	struct file     *fai_filep;
	struct file     *fao_filep;
	struct file     *fdi_filep;
	struct file     *fdo_filep;

	unsigned int    fai_cmd;
	unsigned int    fao_cmd;
	unsigned int    fdi_cmd;
	unsigned int    fdo_cmd;

	unsigned long fai_arg;
	unsigned long fao_arg;
	unsigned long fdi_arg;
	unsigned long fdo_arg;
	
	
} adv_device_loc;

typedef struct _adv_general_ops {	
	int (*opendevice)(adv_device *);
	int (*closedevice)(adv_device *);
	int (*ioctrl)(adv_device *,unsigned int , unsigned long );
	int (*resetdevice)(adv_device *);
	int (*read)(adv_device *, char *, int, loff_t *  );
	int (*write)(adv_device *, const char *, int, loff_t *);
	int (*mmap)(adv_device*, struct file *, struct vm_area_struct * );
	unsigned int (*poll)(adv_device *, struct file *, struct poll_table_struct *);
} adv_general_ops;


typedef struct _SGL9056 {
	PTR_T mem_bus_addr;
	PTR_T io_bus_addr;
	PTR_T trans_size;
	PTR_T next_addr;
} SGL_9056;

typedef struct _adv_dma_system{
	void *sgladdr;
/*      struct kiobuf	*iobuf; */
	unsigned long  *kbuf;
	unsigned long  kbuf_pages;
	unsigned long  iobase;
	unsigned long  ioDMAbase;
	unsigned int   ai_running;
	unsigned int   ao_running;
	unsigned long  ulSGLCount;
	unsigned long  ulHalfSGLCount;
}adv_dma_system;

typedef struct __adv_process_info
{
	struct __adv_process_info *next;
	pid_t tgid;
	INT32U error;		/* save error code */
	INT32U *event_enabled;	/* save event types which have been enable */
	INT32U *event_status;      /* save event status */
	INT32U *event_threshold;	/* threshold of event */
	INT32U *event_counter;	/* counter of interrupt */
	INT32U event_num;		/* save event type num */
	INT32U index;		/* index of the check event */
}adv_process_info;

typedef struct __adv_process_info_header
{
	adv_process_info * header; /* header of adv_process_info list */
	spinlock_t           event_lock; /* read/write spinlock */
}adv_process_info_header;

typedef struct __adv_user_page
{
	struct page *page;
	PTR_T page_addr;	/* page start address in kernel space */
	PTR_T length;
	PTR_T offset;
}adv_user_page;



/* export the functions*/
int advdrv_register_driver(adv_driver *driver);
int advdrv_unregister_driver(adv_driver *driver);

int advdrv_add_device( adv_driver *driver, adv_device *device);
int advdrv_remove_device( adv_driver *driver, adv_device *device);

int advdrv_device_set_boardid(adv_device *device, unsigned short boardID);
int advdrv_device_get_boardid(adv_device *device, unsigned short *pboardID);


int advdrv_device_set_devname(adv_device *device, char *string);
int advdrv_device_get_devname(adv_device *device, char *string);

int advdrv_device_set_nodename(adv_device *device, char *string);
int advdrv_device_get_nodename(adv_device *device, char *string);

int advdrv_device_set_privdata(adv_device *device ,void * privdata);
int advdrv_device_get_privdata(adv_device *device, void **privdata);

/* for set information relative process */
int adv_process_info_header_init(adv_process_info_header *header);
int adv_process_info_remove(adv_process_info_header *header);
int adv_process_info_add(adv_process_info_header *header, INT32U event_num);
int adv_process_info_enable_event(adv_process_info_header *header, INT32U event_th, INT32U threshold);
int adv_process_info_disable_event(adv_process_info_header *header, INT32U event_th);
int adv_process_info_set_event_all(adv_process_info_header *header,INT32U event_th, INT32U count);
int adv_process_info_isset_event(adv_process_info_header *header);
int adv_process_info_isset_special_event(adv_process_info_header *header, INT32U event_th);
int adv_process_info_reset_special_event(adv_process_info_header *header, INT32U event_th);
int adv_process_info_get_special_event_thresh(adv_process_info_header *header, INT32U event_th);
int adv_process_info_reset_event(adv_process_info_header *header, INT32U event_th);
int adv_process_info_check_special_event(adv_process_info_header *header,
					 INT32U event_th);
int  adv_process_info_isenable_event(adv_process_info_header *header,
				     INT32U event_th);
int adv_process_info_isenable_event_all(adv_process_info_header *header, INT32U event_th);
int adv_process_info_check_event(adv_process_info_header *header);
int adv_process_info_set_error(adv_process_info_header *header,INT32U error);
INT32U adv_process_info_get_error(adv_process_info_header *header);

INT32S adv_map_user_pages(adv_user_page **pages, INT32U *page_num, PTR_T uaddr, PTR_T count, INT32S rw);
INT32S adv_unmap_user_pages(adv_user_page *page_list, const INT32U page_num, INT32S dirtied);

/* int advdrv_suspend_device( adv_driver *regdriver,  adv_device *device); */
/* int advdrv_resume_device( adv_driver *regdriver,  adv_device *device); */
/* int advdrv_init_dma( adv_dma_system *dma_system, void *arg ); */

/*kernel memory */
//int advdrv_mem_mmap(adv_memory *padvmem, struct vm_area_struct *vma);
//int advdrv_mem_free(adv_memory *pdam);
//int advdrv_mem_alloc(adv_memory* padvmem, unsigned int size);

//void * rvmalloc(unsigned long size);
//void rvfree(void * mem, unsigned long size);
//int rvmmap(void *mem, unsigned memsize, struct vm_area_struct *vma);
//int adv_di_ioctl(adv_device *device, void *arg);
//int adv_do_ioctl(adv_device *device, void *arg);
//int size_to_order(int sz);
	
#endif
