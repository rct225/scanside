
#include <linux/module.h>
#include "../include/isa.h"



LIST_HEAD(adv_isa_drivers);
DECLARE_RWSEM(adv_isa_drivers_sem);



int adv_isa_register_driver(struct adv_isa_driver *driver)
{

	printk("Register %s driver\n", driver->name);
	
	INIT_LIST_HEAD(&driver->devices);
	INIT_LIST_HEAD(&driver->isa_drivers);
	init_rwsem(&driver->dev_sem);
	
	if (driver->match == NULL) {
		driver->match = adv_isa_match_one_device;
	}
	
	down_write(&adv_isa_drivers_sem);
	list_add_tail(&driver->isa_drivers, &adv_isa_drivers);
	up_write(&adv_isa_drivers_sem);

	return 0;

}

int adv_isa_unregister_driver(struct adv_isa_driver *driver)
{
	struct adv_isa_driver *drv;
	struct adv_isa_device *dev;
	
	printk("Unregister %s driver\n", driver->name);

	if (unlikely(list_empty(&adv_isa_drivers))) {
		printk("ADV ISA driver list is empty.\n");
		return 0;
	}
	
	down_write(&adv_isa_drivers_sem);
	list_for_each_entry(drv, &adv_isa_drivers, isa_drivers) {
		if (drv == driver) {
			down_write(&drv->dev_sem);
			while (!list_empty(&drv->devices)) {
				dev = list_entry(drv->devices.next, typeof(*dev), device_list);
				
				list_del(&dev->device_list);
				drv->remove(dev);
				kfree(dev);
				dev = NULL;
			}
			up_write(&drv->dev_sem);
			
			list_del(&drv->isa_drivers);
			up_write(&adv_isa_drivers_sem);
			return 0;
		}
	}
	
	printk("%s driver not find.\n", driver->name);
	up_write(&adv_isa_drivers_sem);
		
	return 0;

}


int adv_isa_add_device(struct adv_isa_driver *driver, struct adv_isa_device *device)
{

	printk("add device %s for driver %s\n", 
	       device->device_name, driver->name);
	
	if ((driver == NULL) || (device ==NULL)) {
		printk("ISA driver or device is NULL.\n");
		return -EFAULT;
	}
	
	down_write(&driver->dev_sem);
	list_add_tail(&device->device_list, &driver->devices);
	device->driver = driver;
	up_write(&driver->dev_sem);
	
	return 0;

}

int adv_isa_remove_device(struct adv_isa_driver *driver, struct adv_isa_device *device) 
{
	struct adv_isa_device *dev;

	printk("remove device %s\n",device->device_name);

	if ((driver == NULL) || (device ==NULL)) {
		printk("ISA driver or device is NULL.\n");
		return -EFAULT;
	}

	if (unlikely(list_empty(&driver->devices))) {
		printk("ADV ISA device list is empty.\n");
		return 0;
	}

	down_write(&driver->dev_sem);
	list_for_each_entry(dev, &driver->devices, device_list) {
		if (dev == device) {
			list_del(&dev->device_list);
			break;
		}
	}
	up_write(&driver->dev_sem);
	
	return 0;
}


int adv_isa_match_one_device(struct adv_isa_device *device, struct adv_isa_driver *driver)
{
	if ((strcmp(device->driver_name, driver->name) == 0)) {
		return 0;
	}

	return 1;
}



EXPORT_SYMBOL_GPL(adv_isa_register_driver);
EXPORT_SYMBOL_GPL(adv_isa_unregister_driver);
EXPORT_SYMBOL_GPL(adv_isa_add_device);
EXPORT_SYMBOL_GPL(adv_isa_remove_device);
