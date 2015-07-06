/*
 * ISA bus.
 */

#ifndef __LINUX_ISA_H
#define __LINUX_ISA_H

#include <linux/kernel.h>
#include <linux/rwsem.h>
#include "advdrv.h"
#include "../../include/advdevice.h"


/* LIST_HEAD(adv_isa_drivers); */
/* DECLARE_RWSEM(adv_isa_drivers_sem); */

extern struct list_head adv_isa_drivers;
extern struct rw_semaphore adv_isa_drivers_sem;


struct adv_isa_device;

struct adv_isa_driver {
	struct list_head isa_drivers;
	struct list_head devices;
	
	struct rw_semaphore dev_sem;
	const char *name;
	int (*probe)(struct adv_isa_device *dev);
	int (*remove)(struct adv_isa_device *dev);
	int (*match)(struct adv_isa_device *device, struct adv_isa_driver *driver);
	void (*shutdown)(struct adv_isa_device *dev);
	int (*suspend)(struct adv_isa_device *dev);
	int (*resume)(struct adv_isa_device *dev);

};

struct adv_isa_device {
	struct list_head device_list;
	struct adv_isa_driver *driver;	
	const char device_name[DEVICE_NAME_LENGTH];
	const char driver_name[DRIVER_NAME_LENGTH];

	void *driver_data;
	unsigned long iobase;
	unsigned long irq;
	unsigned long irq_ext;
};


	
	


int adv_isa_register_driver(struct adv_isa_driver *driver);
int adv_isa_unregister_driver(struct adv_isa_driver *driver);

int adv_isa_add_device(struct adv_isa_driver *driver, struct adv_isa_device *device);
int adv_isa_remove_device(struct adv_isa_driver *driver, struct adv_isa_device *device);

int adv_isa_match_one_device(struct adv_isa_device *device, struct adv_isa_driver *driver);

static inline void adv_isa_set_drvdata(struct adv_isa_device *idev, void *data)
{
	idev->driver_data = data;
}

static inline void *adv_isa_get_drvdata(struct adv_isa_device *idev)
{
	return idev->driver_data;
}


#endif /* __LINUX_ISA_H */
