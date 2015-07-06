/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech USB-4761 Device driver for Linux

File Name:
	adv_usb_core.c
Abstract:
	USB Device Driver Core file
Version history
	03/20/2006		Create by zhiyong.xie

************************************************************************/	

#include "../include/advdrv.h"
#include "../../include/advdevice.h"
#include <linux/rwsem.h>
#include <linux/pagemap.h>
#include <linux/usb.h>
#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif


#define dev_ops(device)  (device->driver->ops)

int adv_major 	= 0;
/* int  adv_devfs_dir = 0; */

adv_driver *adv_drivers=NULL; 
struct semaphore *adv_drivers_sema = NULL;

adv_device_loc	*adv_dev_loc=NULL;
struct rw_semaphore *adv_dev_loc_sema = NULL;

/*
 * @advdrv_device_set_boardid()
 * @device - storage pointer of adv_device
 * @boardID - Board ID/Switch ID of the card
 * @function description:
 * -when card's id changed, this function changes
 * -board id information of the device
 * */
int advdrv_device_set_boardid(adv_device *device, unsigned short boardID)
{
	device->boardID = boardID;
	return 0;
}
/*
 * @advdrv_device_get_boardid()
 * @device - storage pointer of adv_device
 * @pboardID - storage pointer which save the Board ID
 * @function description:
 * -get the board id from adv_device structure
 * */
int advdrv_device_get_boardid(adv_device *device, unsigned short *pboardID)
{
	*pboardID = device->boardID;
	return 0;
}
/*
 * @advdrv_device_get_boardid()
 * @device - storage pointer of adv_device
 * @string - string of device name
 * @function description:
 * -copy string as device name to device 
 * NOTE* The string length must EQ or LESS than DEVDICE_NAME_LENGTH
 * */
int advdrv_device_set_devname(adv_device *device, char *string)
{
	memcpy(device->device_name, string, strlen(string)+1);
	return 0;
     
}
/*
 * @advdrv_device_get_boardid()
 * @device - storage pointer of adv_device
 * @string - string which saves device name
 * @function description:
 * -copy string as device name to device 
 * NOTE* The string length must EQ or LESS than DEVDICE_NAME_LENGTH
 * */
int advdrv_device_get_devname(adv_device *device, char *string)
{
	memcpy(string, device->device_name, strlen(device->device_name)+1);

	return 0;
}

/* NOTE: The string length must EQ or LESS than DEVDICE_NODE_NAME_LENGTH */
int advdrv_device_set_nodename(adv_device *device, char *string)
{
	memcpy(device->device_node, string, strlen(string)+1);
	return 0;
}

/* NOTE: The string length must EQ or LARGE than DEVDICE_NODE_NAME_LENGTH */
int advdrv_device_get_nodename(adv_device *device, char *string)
{
	memcpy(string, device->device_node, strlen(string)+1);
	return 0;
     
}


int advdrv_device_set_privdata(adv_device *device ,void * privdata)
{
	device->private_data = privdata;
	return 0;
}

int advdrv_device_get_privdata(adv_device *device, void **privdata)
{
	*privdata = device->private_data;
	return 0;
}

/* __adv_process_info_init()
 * @p_process_info: 
 * @event_enabled : 
 * @event_status  :
 * @event_num     :
 * */
int adv_process_info_header_init(adv_process_info_header *header)
{
	header->header = NULL;
	/* rwlock_init(&header->event_lock); */
	spin_lock_init(&header->event_lock);
	return 0;
}

int __adv_process_info_init(adv_process_info *p_process_info,
                            INT32U *event_enabled,
                            INT32U *event_status,
			    INT32U *event_threshold,
			    INT32U *event_counter,
                            INT32U event_num)
{
	p_process_info->next = NULL;
	p_process_info->tgid = current->tgid;
	p_process_info->error = 0;
	p_process_info->index = 0;
	p_process_info->event_enabled = event_enabled;
	p_process_info->event_status = event_status;
	p_process_info->event_threshold = event_threshold;
	p_process_info->event_counter = event_counter;
	p_process_info->event_num = event_num;
	return 0;
     
}

adv_process_info * __adv_process_info_alloc(INT32U event_num)
{
	adv_process_info *p_process_info = NULL;
	INT32U *event_enabled = NULL;
	INT32U *event_status = NULL;
	INT32U *event_threshold = NULL;
	INT32U *event_counter = NULL;

	p_process_info = kmalloc(sizeof(adv_process_info), GFP_ATOMIC);
	event_enabled = kmalloc(event_num * sizeof(INT32U), GFP_ATOMIC);
	event_status = kmalloc(event_num * sizeof(INT32U), GFP_ATOMIC);
	event_threshold = kmalloc(event_num * sizeof(INT32U), GFP_ATOMIC);
	event_counter = kmalloc(event_num * sizeof(INT32U), GFP_ATOMIC);

	if ((!p_process_info) || (!event_enabled) || (!event_status) ||
	   (!event_threshold) || (!event_counter))
	{
		kfree(p_process_info);
		kfree(event_enabled);
		kfree(event_status);
		kfree(event_threshold);
		kfree(event_counter);
		p_process_info = NULL;
		return NULL;
	}
	memset(p_process_info, 0, sizeof(adv_process_info));
	memset(event_enabled, 0, event_num*sizeof(INT32U));
	memset(event_status, 0, event_num*sizeof(INT32U));
	memset(event_threshold, 0, event_num*sizeof(INT32U));
	memset(event_counter, 0, event_num*sizeof(INT32U));
	__adv_process_info_init(p_process_info, event_enabled,
				event_status, event_threshold,
				event_counter, event_num);
	return p_process_info;
}

int __adv_process_info_free(adv_process_info *p_process_info)
{
	kfree(p_process_info->event_enabled);
	kfree(p_process_info->event_status);
	kfree(p_process_info->event_threshold);
	kfree(p_process_info->event_counter);
	kfree(p_process_info);
	return 0;
}


int __adv_process_info_add(adv_process_info **header,
			   adv_process_info *p_process_info)
{
	p_process_info->next = *header;
	*header = p_process_info;
	return 0;
}

/* __adv_process_info_get
 * @
 * @find the node according to the tgid
 * */
adv_process_info * __adv_process_info_get(adv_process_info *header)
{
	pid_t tgid = current->tgid;
	adv_process_info *p;
     
	for (p = header; p != NULL; p = p->next)
	{
		if (p->tgid == tgid)
			return p;
	}
	return NULL;
}



int _adv_process_info_remove(adv_process_info **header)
{
	adv_process_info *p_process_info;
	adv_process_info *prev;
    
	p_process_info = __adv_process_info_get(*header);
	if (!p_process_info)
		return -1;
     
	if (p_process_info == *header)
	{
		*header = p_process_info->next;
	}else{
		for (prev = *header; prev->next; prev=prev->next)
		{
			if (prev->next == p_process_info)
				break;
		}
		prev->next = p_process_info->next;
	}

	__adv_process_info_free(p_process_info);
     
	return 0;
     
}

int adv_process_info_remove(adv_process_info_header *header)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_remove(&header->header);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;
}


int _adv_process_info_add(adv_process_info **header,
			 INT32U event_num)
{
	adv_process_info *p_process_info = NULL;
	p_process_info = __adv_process_info_get(*header);
	if (p_process_info)
		return 0;
     
	p_process_info = __adv_process_info_alloc(event_num);
	if (p_process_info) {
		__adv_process_info_add(header, p_process_info);
		return 0;
	}

	return -1;
}

int adv_process_info_add(adv_process_info_header *header,
			 INT32U event_num)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_add(&header->header, event_num);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;
}


int _adv_process_info_enable_event(adv_process_info *header,
				       INT32U event_th,
				       INT32U threshold)
{
	adv_process_info *p;
	p = __adv_process_info_get(header);
	if (!p)
	{
		return -1;		/* NO EXIST */
	}
	*(p->event_enabled + event_th) = 1;
	*(p->event_threshold + event_th) = threshold;

	return 0;
}
int adv_process_info_enable_event(adv_process_info_header *header,
				  INT32U event_th,
				  INT32U threshold)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_enable_event(header->header, 
					     event_th, threshold);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;
}


int _adv_process_info_disable_event(adv_process_info *header,
					INT32U event_th)
{
	adv_process_info *p;
	p = __adv_process_info_get(header);
	if (!p)
	{
		return -1;		/* NO EXIST */
	}
	*(p->event_enabled + event_th) = 0;
	*(p->event_threshold + event_th) = 0;

	return 0;
}
int adv_process_info_disable_event(adv_process_info_header *header,
				    INT32U event_th)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_disable_event(header->header, event_th);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;
}




int _adv_process_info_set_event_all(adv_process_info *header,
				    INT32U event_th, INT32U count)
{
	adv_process_info *p;
     
	for (p = header; p; p = p->next)
	{
		if (!_ADV_ISENABLE_EVENT(p, event_th))
			continue;

		*(p->event_counter + event_th) += count;
		if (*(p->event_counter + event_th) >= *(p->event_threshold+event_th))
		{
			_ADV_SET_EVENT(p, event_th);
			*(p->event_counter + event_th) -= *(p->event_threshold + event_th) ;
		}
	}

	return 0;
}

int adv_process_info_set_event_all(adv_process_info_header *header,
				    INT32U event_th, INT32U count)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_set_event_all(header->header, event_th, count);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;

}



int _adv_process_info_isset_special_event(adv_process_info *header,
					      INT32U event_th)
{
	adv_process_info *p_process_info;
     
	p_process_info = __adv_process_info_get(header);
	if (!p_process_info)
	{
		return -1;		/* NO EXIST */
	}
     
	return _ADV_ISSET_EVENT(p_process_info, event_th);
}

int adv_process_info_isset_special_event(adv_process_info_header *header,
					 INT32U event_th)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_isset_special_event(header->header, event_th);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;

}

int _adv_process_info_get_special_event_thresh(adv_process_info *header,
						  INT32U event_th)
{
	adv_process_info *p_process_info;
     
	p_process_info = __adv_process_info_get(header);
	if (!p_process_info)
	{
		return -1;		/* NO EXIST */
	}
     
	return p_process_info->event_threshold[event_th];
	
}

int adv_process_info_get_special_event_thresh(adv_process_info_header *header, INT32U event_th)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_get_special_event_thresh(header->header, event_th);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;

}



int _adv_process_info_reset_special_event(adv_process_info *header,
					      INT32U event_th)
{
	adv_process_info *p_process_info;
     
	p_process_info = __adv_process_info_get(header);
	if (!p_process_info)
	{
		return -1;		/* NO EXIST */
	}
     
	_ADV_RESET_EVENT(p_process_info, event_th);
	return 0;
}

int adv_process_info_reset_special_event(adv_process_info_header *header,
					  INT32U event_th)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_reset_special_event(header->header, event_th);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;

}

int _adv_process_info_check_special_event(adv_process_info *header,
					      INT32U event_th)
{
	int ret;
	adv_process_info *p_process_info;
     
	p_process_info = __adv_process_info_get(header);
	if (!p_process_info)
	{
		return -1;		/* NO EXIST */
	}
     
	ret = _ADV_ISSET_EVENT(p_process_info, event_th);
	_ADV_RESET_EVENT(p_process_info, event_th);
	return ret;
}

int adv_process_info_check_special_event(adv_process_info_header *header,
					 INT32U event_th)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_check_special_event(header->header, event_th);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;

}


int  _adv_process_info_isenable_event(adv_process_info *header,
				      INT32U event_th)
{
	adv_process_info *p;
	p = __adv_process_info_get(header);
	if (!p) {
		return -1;
	}
	
	return _ADV_ISENABLE_EVENT(p, event_th);
			
}
int  adv_process_info_isenable_event(adv_process_info_header *header,
				     INT32U event_th)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_isenable_event(header->header, event_th);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;

}


int  _adv_process_info_isenable_event_all(adv_process_info *header,
					INT32U event_th)
{
	adv_process_info *p;
	
	for (p = header; p; p = p->next)
	{
		if (event_th > p->event_num)
			return -1;
		
		if ( _ADV_ISENABLE_EVENT(p, event_th))
			return 1;
		
	}
	return 0;
		
}
int  adv_process_info_isenable_event_all(adv_process_info_header *header,
					  INT32U event_th)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_isenable_event_all(header->header, event_th);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;

}


int  _adv_process_info_isset_event(adv_process_info *header)
{
	INT32U i;
	adv_process_info *p;
     
	p = __adv_process_info_get(header);
	if (!p)
	{
		return -1;		/* NO EXIST */
	}
     
	for (i = 0; i < p->event_num; i++)
	{
		if (_ADV_ISSET_EVENT(p, i))
			return 1;
	}
	return 0;
}
int  adv_process_info_isset_event(adv_process_info_header *header)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_isset_event(header->header);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;

}

int  _adv_process_info_check_event(adv_process_info *header)
{
	INT32U event = 0;
	INT32U i;
	adv_process_info *p;
     
	p = __adv_process_info_get(header);
	if (!p)
	{
		return -1;		/* NO EXIST */
	}

	for (i = 0; i < p->event_num; i++,p->index++)
	{
		p->index = (p->index)%(p->event_num);
		
		if ( !(*(p->event_enabled+p->index) ))
		{
			continue;
		}
		event = _ADV_ISSET_EVENT(p, p->index);
		
		if (event)
		{
			_ADV_RESET_EVENT(p, p->index);
			p->index = p->index + 1;
			return p->index;
		}
	}
	return 0;
     
}

int  adv_process_info_check_event(adv_process_info_header *header)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_check_event(header->header);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;
}

int _adv_process_info_set_error(adv_process_info *header,
			       INT32U error)
{
	adv_process_info *p_process_info;
     
	p_process_info = __adv_process_info_get(header);
	if (!p_process_info)
	{
		return -1;		/* NO EXIST */
	}
     
	_ADV_SET_ERROR(p_process_info, error);
	return 0;
}
int adv_process_info_set_error(adv_process_info_header *header,
			       INT32U error)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_set_error(header->header, error);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;
}

int _adv_process_info_get_error(adv_process_info *header)
{
	INT32U error;
	adv_process_info *p_process_info;
     
	p_process_info = __adv_process_info_get(header);
	if (!p_process_info)
	{
		return -1;		/* NO EXIST */
	}
     
	_ADV_GET_ERROR(p_process_info, error);
	return error;
}

INT32U adv_process_info_get_error(adv_process_info_header *header)
{
	INT32S ret;
	PTR_T flags;
	
	spin_lock_irqsave(&header->event_lock, flags);
	ret = _adv_process_info_get_error(header->header);
	spin_unlock_irqrestore(&header->event_lock, flags);

	return ret;
}


/**
 * adv_map_user_pages - get user buffer pages, and fill into the pages_list, this
 *                      function is called by high speed transfer function
 * 
 * @pages_list: a page list, store the user buffer page pointer, page virtual addr,
 *              page length, page offset
 * @page_num: how many pages in user buffer (returned value)
 * @uaddr: user buffer address
 * @count: user buffer size in byte
 * @rw: read/write flag
 *      (0: read)
 *      (1: write)
 */
INT32S adv_map_user_pages(adv_user_page **pages_list, INT32U *page_num, PTR_T uaddr, PTR_T count, INT32S rw)
{
	INT32S res;
	INT32S i;
	INT32S j;
	struct page **pages;
	adv_user_page *page_list = NULL;
		
//	*page_num = ((uaddr & ~PAGE_MASK) + count + ~PAGE_MASK) >> PAGE_SHIFT;
	*page_num = (uaddr + count + PAGE_SIZE - 1)/PAGE_SIZE - uaddr/PAGE_SIZE;

//printk("aaaaaaaaaaaaaaaaaaaaaa\n");
	if ((uaddr + count) < uaddr) {
		*pages_list = NULL;
		return -EFAULT;
	}

	if (count == 0) {
		*pages_list = NULL;
		return -EFAULT;
	}
//printk("bbbbbbbbbbaaaaaaaaaaaa\n");

	pages = kmalloc(*page_num * sizeof(*pages), GFP_KERNEL);
	if (*page_num < 256)
		page_list = kmalloc(*page_num * sizeof(adv_user_page), GFP_KERNEL);
	else 
		page_list = (adv_user_page *)__get_free_pages(GFP_KERNEL, get_order(*page_num * sizeof(adv_user_page)));
	
	if ( (pages == NULL) || (page_list == NULL)) {
		kfree(pages);
		if (*page_num < 256)
			kfree(page_list);
		else
			free_pages((unsigned long)page_list, get_order(*page_num * sizeof(adv_user_page)));
		
		*pages_list = NULL;
		return -ENOMEM;
	}

	down_read(&current->mm->mmap_sem);
	res = get_user_pages(current, current->mm, uaddr, *page_num, 
			     (rw==READ), 0, pages, NULL);
	up_read(&current->mm->mmap_sem);

	if (res != *page_num) {
		*pages_list = NULL;
		goto out_unmap;
	}

	for (i = 0; i < *page_num; i++) {
		flush_dcache_page(pages[i]);
		lock_page(pages[i]);
	}

	page_list[0].page = pages[0];
	page_list[0].offset = uaddr & ~PAGE_MASK;
	page_list[0].page_addr = (PTR_T)kmap(page_list[0].page);
	if (*page_num > 1) {
		page_list[0].length = PAGE_SIZE - page_list[0].offset;
		count -= page_list[0].length;
		for (i = 1; i < *page_num; i++) {
			page_list[i].page = pages[i];
			page_list[i].page_addr = (PTR_T)kmap(page_list[i].page);
			page_list[i].offset = 0;
			page_list[i].length = count < PAGE_SIZE ? count : PAGE_SIZE;
			count -= PAGE_SIZE;
		}
	}
	else{
		page_list[0].length = count;
	}

	kfree(pages);
	*pages_list = page_list;
	return *page_num;
	
 out_unmap:
	if (res > 0) {
		for (j = 0; j < res; j++) {
			page_cache_release(pages[j]);
		}
	}
	kfree(pages);
	if (*page_num < 256)
		kfree(page_list);
	else
		free_pages((unsigned long)page_list, get_order(*page_num * sizeof(adv_user_page)));
		
	return res;
		
}

INT32S adv_unmap_user_pages(adv_user_page *page_list, const INT32U page_num, INT32S dirtied)
{
	INT32S i;
	
	for (i = 0; i < page_num; i++) {
		unlock_page(page_list[i].page);
		kunmap(page_list[i].page);
		if (dirtied && !PageReserved(page_list[i].page)) {
			SetPageDirty(page_list[i].page);
		}
		page_cache_release(page_list[i].page);
	}
	if (page_num < 256)
		kfree(page_list);
	else
		free_pages((unsigned long)page_list, get_order(page_num * sizeof(adv_user_page)));
		
	
	return 0;
	
}


/************************************************************************
 * Function:		 int advdrv_register_driver
 *
 * Description:  	Export the function for all sub driver to register.
 * Parameters:	driver 	-Points to the adv_driver object
 *************************************************************************/
int advdrv_register_driver( adv_driver *driver)
{

	KdPrint("Register %s driver\n", driver->driver_name );
     
	if (down_interruptible(adv_drivers_sema))
		return -ERESTARTSYS;
     
	driver->next = adv_drivers;
	adv_drivers = driver;
	up(adv_drivers_sema);
     
	return 0;


}

/************************************************************************
 * Function:		 int advdrv_unregister_driver
 *
 * Description:  	Export the function for all sub driver to unregister.
 * Parameters:	driver 	-Points to the adv_driver object
 *************************************************************************/
int advdrv_unregister_driver( adv_driver *driver)
{
	adv_driver *prev;
	
	KdPrint("Unregister %s driver\n", driver->driver_name );
     
	if (down_interruptible(adv_drivers_sema))
		return -ERESTARTSYS;
     
	if ( adv_drivers == driver ) {
		adv_drivers=driver->next;
		up(adv_drivers_sema);
		return 0;
	}

	for ( prev = adv_drivers; prev->next; prev = prev->next ) {
		if ( prev->next == driver ) {
			prev->next = driver->next;
			up(adv_drivers_sema);
			return 0;
		}
	}

	PWARN("%s driver NOT FIND\n", driver->driver_name);

	up(adv_drivers_sema);
	return -EINVAL;
}

/************************************************************************
 * Function:		 int advdrv_add_usb_device
 *
 * Description:  	Export the function for all sub driver to add a device in driver object.
 * Parameters:	driver 	-Points to the adv_driver object
 				device	-Points to the adv_device object
*************************************************************************/
int advdrv_add_device( adv_driver *driver,  adv_device *device)
{
	if ((driver == NULL) || (device == NULL)) {
		return -EFAULT;
	}
	
	if (down_interruptible(driver->driver_sema))
		return -ERESTARTSYS;
	/* detect whether the device exist with same board id */
	/* * usb device with board id  */
	if ((device->boardID != 0) ||  (*device->device_name != 'p')) {
		adv_device *pdev = driver->devices;
		while (pdev) {
			if ((pdev->boardID == device->boardID)
			    && (!strcmp(pdev->device_name, device->device_name))) {
				up(driver->driver_sema);
				return -EEXIST;
			}
			pdev = pdev->next;
		} 
	}
	
	/* add device to devices list */
	device->next = driver->devices;
	driver->devices = device;
	device->driver = driver;
	device->config = 0;
     
	up(driver->driver_sema);
     
	return 0;
}

/************************************************************************
 * Function:		 int advdrv_remove_usb_device
 *
 * Description:  	Export the function for all sub driver to remove a device in driver object.
 * Parameters:	driver 	-Points to the adv_driver object
 				device	-Points to the adv_device object
*************************************************************************/
int advdrv_remove_device( adv_driver *driver,  adv_device *device)
{
	adv_device *prev;
	int i;
     
	if (down_interruptible(driver->driver_sema))
		return -ERESTARTSYS;

	if ( driver->devices == device ) {
		driver->devices = device->next;
	} else {
		for ( prev=driver->devices; prev->next; prev=prev->next) {
			if ( prev->next == device ) {
				prev->next = device->next;//dangerous point operation marked by zdd
				device->config = 0;
		    
				break;
			}
		}
	}

	up(driver->driver_sema);

	down_write(adv_dev_loc_sema);
     
	for (i = 0; i < ADV_NDEVICES; i++) {
		if (adv_dev_loc[i].device_ptr == device) {
			adv_dev_loc[i].device_ptr = NULL;
			adv_dev_loc[i].driver_ptr = NULL;
			adv_dev_loc[i].devcnf = 0;
			break;
		}
	}
	up_write(adv_dev_loc_sema);
	return 0;
}



/************************************************************************
 * Function:		 int advdrv_poll
 *
 * Description:  	for poll system call
 * Parameters:	        file --- Points to a file struct
 *************************************************************************/

static unsigned int advdrv_poll(struct file *file, struct poll_table_struct *wait)
{
	int ret = 0;
	int minor=MINOR(file->f_dentry->d_inode->i_rdev);
	adv_device	*device=NULL;
	adv_device_loc *dev_loc;
     

	
	down_read(adv_dev_loc_sema);
	dev_loc=adv_dev_loc+minor;
	if (!dev_loc->devcnf) {
		printk("Please Config your device node first!\n");
		printk("Please run advdevice_config to config device.\n");
		up_read(adv_dev_loc_sema);
		return -EIO;
	}
	device =dev_loc->device_ptr;
	up_read(adv_dev_loc_sema);
     
	if (device->driver->ops->poll)
		ret=device->driver->ops->poll(device, file, wait);
	return ret;
}

/************************************************************************
 * Function:		 int advdrv_read
 *
 * Description:  	The file operation: Read the datas from the device. It will callback the
 				sub driver.
*************************************************************************/
ssize_t advdrv_read(struct file *filep, char *buf, size_t nbytes, loff_t *offset )
{

	int minor=MINOR(filep->f_dentry->d_inode->i_rdev);
	adv_device_loc *dev_loc = NULL;
	adv_device *device = NULL;
     
	down_read(adv_dev_loc_sema);
	dev_loc = adv_dev_loc + minor;
	if (!dev_loc->devcnf) {
		printk("Please Config your device node first!\n");
		printk("Please run advdevice_config to config device.\n");
		up_read(adv_dev_loc_sema);
		return -EIO;
	}
	device = dev_loc->device_ptr;
	up_read(adv_dev_loc_sema);
     
	if (device == NULL)
		return -EIO;
     
	if ( dev_ops(device)->read )
		return dev_ops(device)->read(device, buf, nbytes, offset);
	else
		return -EIO;

}


/************************************************************************
 * Function:		 int advdrv_read
 *
 * Description:  	The file operation: Read the datas from the device. It will callback the
 				sub driver.
*************************************************************************/
ssize_t advdrv_write(struct file *filep, const char *buf, size_t nbytes, loff_t *offset )
{
	int minor=MINOR(filep->f_dentry->d_inode->i_rdev);
	adv_device_loc *dev_loc = NULL;
	adv_device *device = NULL;
     
	down_read(adv_dev_loc_sema);
	dev_loc = adv_dev_loc + minor;
	if (!dev_loc->devcnf) {
		printk("Please Config your device node first!\n");
		printk("Please run advdevice_config to config device.\n");
		up_read(adv_dev_loc_sema);
		return -EIO;
	}
	device = dev_loc->device_ptr;
	up_read(adv_dev_loc_sema);
     
	if (device == NULL)
		return -EIO;
     
	if ( dev_ops(device)->write)
		return dev_ops(device)->write(device, buf, nbytes, offset );
	else
		return -EIO;
}


/************************************************************************
 * Function:		 int advdrv_mmap
 *
 * Description:  	The file operation: Map the kernel memory to user.
 				It will callback the sub driver.
 				sub driver.
*************************************************************************/
int advdrv_mmap(struct file *filep, struct vm_area_struct *vma)
{
     
	int minor=MINOR(filep->f_dentry->d_inode->i_rdev);
	adv_device_loc *dev_loc = NULL;
	adv_device *device = NULL;
     
	down_read(adv_dev_loc_sema);
	dev_loc = adv_dev_loc + minor;
	
	if (!dev_loc->devcnf) {
		printk("Please Config your device node first!\n");
		printk("Please run advdevice_config to config device.\n");
		up_read(adv_dev_loc_sema);
		return -EIO;
	}
	device = dev_loc->device_ptr;
	up_read(adv_dev_loc_sema);

	if (device == NULL)
		return -EIO;

	if ( dev_ops(device)->mmap ) {
		return dev_ops(device)->mmap(device, filep, vma);
	}
	return -EIO;
     
}

/************************************************************************
 * Function:		 int advdrv_mmap
 *
 * Description:  	The file operation: Opens the device
 				It will callback the sub driver.
 				sub driver.
*************************************************************************/
int advdrv_open (struct inode *inode, struct file *filep)
{

//     int ret = 0;
	adv_device	*device=NULL;
	adv_device_loc     *dev_loc = NULL;
     
	int minor = MINOR(inode->i_rdev);
     
	/* for system detect */
	if (minor == ADV_SYS_NODE)
		return 0;

     
	down_read(adv_dev_loc_sema);
	dev_loc = adv_dev_loc + minor;
		
	if (dev_loc == NULL)
	{
		printk("Operation not allowed: No card installed at this minor\n");
		up_read(adv_dev_loc_sema);
		return -ENODEV; // Not found
	}

     
	if (!dev_loc->devcnf) {
		printk("Please bind your device file first!\n");
		printk("Please run advdevice_bind to bind device.\n");
		up_read(adv_dev_loc_sema);
		return -ENODEV;
	}

	device =dev_loc->device_ptr;
	up_read(adv_dev_loc_sema);

     
	if (device == NULL)
	{
		down_write(adv_dev_loc_sema);
		dev_loc->devcnf=0;
		up_write(adv_dev_loc_sema);
		return -ENODEV; // Not found
	}

	try_module_get(dev_loc->driver_ptr->owner);
	
	

	atomic_inc(&dev_loc->dev_count);

	
	
	if (dev_ops(device)->opendevice) {
		return dev_ops(device)->opendevice(device);
	}
     
	return -EIO;
     
}

/************************************************************************
 * Function:		 int advdrv_release
 *
 * Description:  	The file operation: Close the device
 				It will callback the sub driver.
*************************************************************************/
int advdrv_release (struct inode *inode, struct file *filep)
{

	int minor=MINOR(inode->i_rdev);
	int ret = 0;
	adv_device *device =NULL;
	adv_device_loc *dev_loc = NULL;



     
	/* for system detect */
	if (minor == ADV_SYS_NODE)
		return 0;
     
	down_read(adv_dev_loc_sema);
	dev_loc=adv_dev_loc+minor;
	if (!dev_loc->devcnf) {
		printk("Please Config your device node first!\n");
		printk("Please run advdevice_config to config device.\n");
		up_read(adv_dev_loc_sema);
		return -EIO;
	}
	device =dev_loc->device_ptr;
	up_read(adv_dev_loc_sema);
     
	if (device == NULL) {
		down_write(adv_dev_loc_sema);
		dev_loc->devcnf=0;
		up_write(adv_dev_loc_sema);
		return -EIO;
	}


	module_put(dev_loc->driver_ptr->owner);

	if(dev_loc->fai_filep == filep) {
		if (device->driver->ops->ioctrl) {

			ret = device->driver->ops->ioctrl(device, dev_loc->fai_cmd, dev_loc->fai_arg);
		}
		if(ret) {
			return ret;
		}
		dev_loc->fai_filep = NULL;

	}


	if(dev_loc->fai_filep == filep) {
		if (device->driver->ops->ioctrl) {
			ret = device->driver->ops->ioctrl(device, dev_loc->fai_cmd, dev_loc->fai_arg);
		}
		if(ret) {
			return ret;
		}
		dev_loc->fai_filep = NULL;

	}
	if(dev_loc->fao_filep == filep) {
		if (device->driver->ops->ioctrl) {
			ret = device->driver->ops->ioctrl(device, dev_loc->fao_cmd, dev_loc->fao_arg);
		}
		if(ret) {
			return ret;
		}
		dev_loc->fao_filep = NULL;

	}
	if(dev_loc->fdi_filep == filep) {
		if (device->driver->ops->ioctrl) {
			ret = device->driver->ops->ioctrl(device, dev_loc->fdi_cmd, dev_loc->fdi_arg);
		}
		if(ret) {
			return ret;
		}
		dev_loc->fdi_filep = NULL;

	}
	if(dev_loc->fdo_filep == filep) {
		if (device->driver->ops->ioctrl) {
			ret = device->driver->ops->ioctrl(device, dev_loc->fdo_cmd, dev_loc->fdo_arg);
		}
		if(ret) {
			return ret;
		}
		dev_loc->fdo_filep = NULL;

	}
	
	

	
	
	if(!atomic_dec_and_test(&dev_loc->dev_count)) {
		return 0;
	}

	if (dev_ops(device)->closedevice)
		return  dev_ops(device)->closedevice(device);
	  
	return -EIO;
}

/* find first device meet requset but not config */
adv_device *select_and_alloc_device(StruDevConfig *lpDevConf,int minor)
{
	int i=0;
	adv_device	*prevdev=NULL;
	adv_driver	*prevdrv=NULL;
	
	if (down_interruptible(adv_drivers_sema))
		return NULL;
     
	for (prevdrv = adv_drivers; prevdrv; prevdrv = prevdrv->next) {
		if (down_interruptible(prevdrv->driver_sema)) {
			up(adv_drivers_sema);
			return NULL;
		}
	  
		if (strcmp(prevdrv->driver_name,lpDevConf->driver_name) == 0) {
			for (prevdev = prevdrv->devices; prevdev; prevdev = prevdev->next) {
				i++;
				if ((!prevdev->config) 
				   && (lpDevConf->board_id == prevdev->boardID)
				   && (strcmp(lpDevConf->device_name, prevdev->device_name) == 0)
				   && ((lpDevConf->iobase == 0) || (lpDevConf->iobase == prevdev->iobase))
				   && ((lpDevConf->slot == 0) || (lpDevConf->slot == prevdev->slot))
				   && ((lpDevConf->irq == 0) || (lpDevConf->irq == prevdev->irq))) {
					up(prevdrv->driver_sema);
					up(adv_drivers_sema);
					return prevdev;
				}
			}
		}
		up(prevdrv->driver_sema);
	}

	up (adv_drivers_sema);
	if (prevdrv == NULL) {
		if (!i) {
			printk("Driver for %s should be insmod first!\n",
			       lpDevConf->device_name);
		}
	}
	return NULL;
}

/* find first device meet request */
adv_device *select_and_alloc_device_first(StruDevConfig *lpDevConf,int minor)
{
	int i=0;
	adv_device	*prevdev=NULL;
	adv_driver	*prevdrv=NULL;
	if (down_interruptible(adv_drivers_sema))
		return NULL;
     
	for (prevdrv=adv_drivers;prevdrv;prevdrv=prevdrv->next)
	{
		if (down_interruptible(prevdrv->driver_sema))
		{
			up(adv_drivers_sema);
			return NULL;
		}
	  
		if (strcmp(prevdrv->driver_name,lpDevConf->driver_name)==0)
		{
	      
	       
			for (prevdev = prevdrv->devices; prevdev; prevdev = prevdev->next)
			{
				i++;
				if ((lpDevConf->board_id == prevdev->boardID) &&
				   ( strcmp(lpDevConf->device_name, prevdev->device_name) == 0))
				{
					up(prevdrv->driver_sema);
					up(adv_drivers_sema);
					return prevdev;
				}else if ((lpDevConf->board_id == 0xff) &&
					 (strcmp(lpDevConf->device_name, prevdev->device_name) == 0))
				{
					up(prevdrv->driver_sema);
					up(adv_drivers_sema);
					return prevdev;
				}
				
			}
	       	       
		}
		up(prevdrv->driver_sema);
	}
	up(adv_drivers_sema);
	if (prevdrv == NULL) {
		if (!i) {
			printk("Driver for %s should be insmod first!\n",
			       lpDevConf->device_name);
		}		
	}
	return NULL;
}

int device_alloc(adv_device *device, StruDevConfig *lpDevConf)
{
//	adv_device_loc *dev=adv_dev_loc+lpDevConf->minor;
	if (!device) {
		return -ENODEV;
	}
	
	if (device->config) {
		return -EBUSY;
	}
	
	device->config=1;
     
	memcpy(device->device_node, lpDevConf->device_node_name, DEVICE_NODE_NAME_LENGTH-1);
	device->device_node[DEVICE_NODE_NAME_LENGTH-1] = 0;
	return 0;
}

int adv_device_detach(int minor)
{
	int ret = 0;
	adv_device_loc *dev = NULL;
	
	down_write(adv_dev_loc_sema);
	dev =adv_dev_loc+minor;
	if (dev->devcnf) {
		dev->devcnf=0;
		dev->device_ptr->config = 0;
	  
		memset(dev->device_ptr->device_node, 0, DEVICE_NODE_NAME_LENGTH);
		dev->device_ptr = NULL;
		dev->driver_ptr = NULL;
			  
	}else{
		ret = -ENODEV;
	}
	up_write(adv_dev_loc_sema);
	return ret;
}

static int GetDevInfo(StruDevInfo *lpDevInfo)
{
  
	adv_driver	*prevdrv = NULL;
	adv_device	*device = NULL;
	int i=0;
	int j=0;
	
	if (down_interruptible(adv_drivers_sema))
		return -ERESTARTSYS;
     
	for (prevdrv=adv_drivers;prevdrv;prevdrv=prevdrv->next)
	{
		if (down_interruptible(prevdrv->driver_sema))
		{
			up(adv_drivers_sema);
			return -ERESTARTSYS;
		}
	  
		for (device=prevdrv->devices;device;device=device->next)
		{
			j++;
			memcpy((lpDevInfo+i)->device_name, device->device_name,strlen(device->device_name)+1);
			//sprintf((lpDevInfo+i)->board_id,"%d",device->boardID);
			(lpDevInfo + i)->board_id = device->boardID;
			(lpDevInfo + i)->io_base = device->iobase;
			(lpDevInfo + i)->slot = device->slot;
			(lpDevInfo + i)->irq = device->irq;
			(lpDevInfo + i)->irq_ext = device->irq_ext;
			(lpDevInfo + i)->bus = device->bus;
			memcpy((lpDevInfo+i)->device_node_name, device->device_node, strlen(device->device_node)+1);
			i++;
		}

		up(prevdrv->driver_sema);
	  
	}
	up(adv_drivers_sema);
	return 0;
     
}


/************************************************************************
 * Function:		 int advdrv_ioctl
 *
 * Description:  	The file operation: handle ioctl commands
 				It will callback the sub driver.
*************************************************************************/

int advdrv_ioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg)
{
	adv_device	*dev=NULL;
	adv_device	*device=NULL;
	struct adv_isa_device *isa_dev = NULL;
	struct adv_isa_driver *isa_drv = NULL;
	int minor = 0;
	adv_device_loc *dev_loc = NULL;
	int ret = 0;
	int isa_dev_rm_flag = 0;
	
	StruDevInfo *lpDevInfo;
	StruDevConfig lpDevConf;

	if (cmd == GETDEVINFO) {

		lpDevInfo=(StruDevInfo *)kmalloc(MAX_DEVICE*sizeof(StruDevInfo),GFP_KERNEL);
		memset(lpDevInfo,0,MAX_DEVICE*sizeof(StruDevInfo));
		ret = GetDevInfo(lpDevInfo);
		if (ret != 0) {
			kfree(lpDevInfo);
			return ret;
		}
	  
		if (copy_to_user((char *)arg,lpDevInfo,sizeof(StruDevInfo)*MAX_DEVICE)) {
			kfree(lpDevInfo);
			return -EFAULT;
		}
		kfree(lpDevInfo);
		return 0;
	}


	if (cmd == UNDEVCONFIG) {
		if (copy_from_user(&lpDevConf,(void*)arg,sizeof(StruDevConfig))) {
			return -EFAULT;
		}
		lpDevConf.driver_name[DRIVER_NAME_LENGTH-1]=0;
		lpDevConf.device_name[DEVICE_NAME_LENGTH-1]=0;
		lpDevConf.device_node_name[DEVICE_NODE_NAME_LENGTH-1]=0;
	 
		minor = lpDevConf.minor;
		dev_loc = adv_dev_loc + minor;
		
		if ( (dev_loc->devcnf)
		     && (lpDevConf.board_id == dev_loc->device_ptr->boardID)
		     && (strcmp(lpDevConf.device_name, dev_loc->device_ptr->device_name) == 0)
		     && ((lpDevConf.iobase == 0) || (lpDevConf.iobase == dev_loc->device_ptr->iobase))
		     && ((lpDevConf.slot == 0) || (lpDevConf.slot == dev_loc->device_ptr->slot))
		     && ((lpDevConf.irq == 0) || (lpDevConf.irq == dev_loc->device_ptr->irq))) {
					
		return adv_device_detach(lpDevConf.minor);
		}
		return -ENODEV;
	}
	
	if (cmd==DEVCONFIG) {
		if (copy_from_user(&lpDevConf,(void*)arg,sizeof(StruDevConfig))) {
			return -EFAULT;
		}
	      	       
		lpDevConf.driver_name[DRIVER_NAME_LENGTH-1]=0;
		lpDevConf.device_name[DEVICE_NAME_LENGTH-1]=0;
		lpDevConf.device_node_name[DEVICE_NODE_NAME_LENGTH-1]=0;
	 
		minor = lpDevConf.minor;
		down_write(adv_dev_loc_sema);
		dev_loc = adv_dev_loc + minor;
	
		if (!dev_loc->devcnf) {
			if ((dev = select_and_alloc_device(&lpDevConf,minor))==NULL) {
				up_write(adv_dev_loc_sema);
				return -ENODEV;
			}

			ret = device_alloc(dev, &lpDevConf);
			if (ret != 0) {
				up_write(adv_dev_loc_sema);
				return ret;
			}
			dev_loc->device_ptr = dev;
			dev_loc->driver_ptr = dev->driver;
			dev_loc->devcnf=1;
		} else {
			up_write(adv_dev_loc_sema);
			return -EBUSY;
		}
		up_write(adv_dev_loc_sema);
		return 0;
	  
	}

	if(cmd==ISA_ADD_DEVICE){
		if(copy_from_user(&lpDevConf,(void*)arg,sizeof(StruDevConfig))){
			return -EFAULT;
		}
	      	       
		lpDevConf.driver_name[DRIVER_NAME_LENGTH-1]=0;
		lpDevConf.device_name[DEVICE_NAME_LENGTH-1]=0;
		lpDevConf.device_node_name[DEVICE_NODE_NAME_LENGTH-1]=0;
	 

		isa_dev = kmalloc(sizeof(*isa_dev),GFP_KERNEL);
		if (!isa_dev) {
			printk(KERN_ERR 
			       "Failed to allocate memory for isa device.\n");
			return -ENOMEM;
		} 
		isa_dev->iobase = lpDevConf.iobase;
		isa_dev->irq = lpDevConf.irq;
		isa_dev->irq_ext = lpDevConf.irq_ext;
		memcpy(isa_dev->device_name, lpDevConf.device_name, DEVICE_NAME_LENGTH);
		memcpy(isa_dev->driver_name, lpDevConf.driver_name, DRIVER_NAME_LENGTH);
		

		if (unlikely(list_empty(&adv_isa_drivers))) {
			printk(KERN_ERR "ISA driver list is empty.\n");
			kfree(isa_dev);
			return -ENODEV;
		}

		
		down_read(&adv_isa_drivers_sem);
		list_for_each_entry(isa_drv, &adv_isa_drivers, isa_drivers) {
			if (!isa_drv->match) {
				printk(KERN_ERR "match function is NULL.\n");
				up_read(&adv_isa_drivers_sem);
				kfree(isa_dev);
				return -EFAULT;
			}
			
			if ((isa_drv->match(isa_dev, isa_drv) == 0)) {
				if (!isa_drv->probe) {
					printk(KERN_ERR "probe function is NULL.\n");
					up_read(&adv_isa_drivers_sem);
					kfree(isa_dev);
					return -EFAULT;
				}
					
				ret = isa_drv->probe(isa_dev);
				if (ret) {
					printk(KERN_ERR "Failed to probe ISA device.\n");
					kfree(isa_dev);
					up_read(&adv_isa_drivers_sem);
					return ret;
				}
				adv_isa_add_device(isa_drv, isa_dev);
				
				break;
			}
		}
		up_read(&adv_isa_drivers_sem);
		
		if (&isa_drv->isa_drivers == &adv_isa_drivers) {
			printk(KERN_ERR 
			       "ISA driver does not exist or match.\n");
			kfree(isa_dev);
			return -ENODEV;
		}
		

		return 0;
	}
	

	if (cmd == ISA_REMOVE_DEVICE) {
		if(copy_from_user(&lpDevConf,(void*)arg,sizeof(StruDevConfig))){
			return -EFAULT;
		}

		
		down_read(&adv_isa_drivers_sem);
		list_for_each_entry(isa_drv, &adv_isa_drivers, isa_drivers) {
			down_read(&isa_drv->dev_sem);
			list_for_each_entry(isa_dev, &isa_drv->devices, device_list) {
				if ((isa_drv->match(isa_dev, isa_drv) == 0)
				    && (strcmp(isa_dev->device_name, lpDevConf.device_name) == 0)
				    && ((lpDevConf.iobase == 0) || (lpDevConf.iobase == isa_dev->iobase))
				    && ((lpDevConf.irq == 0) || (lpDevConf.irq == isa_dev->irq))) {
					if (!isa_drv->remove) {
						printk(KERN_ERR "remove function is NULL.\n");
						up_read(&isa_drv->dev_sem);
						up_read(&adv_isa_drivers_sem);
						return -EFAULT;
					}
					
					isa_drv->remove(isa_dev);
					isa_dev_rm_flag = 1;
					
					break;
				}
/* 				printk("devname: %s, iobase: 0x%x, irq: 0x%x\n", isa_dev->device_name, isa_dev->iobase, isa_dev->irq); */
			}
/* 			printk("drvname: %s\n", isa_drv->name); */
			up_read(&isa_drv->dev_sem);
			if (isa_dev_rm_flag) {
				break;
			}
		}
		up_read(&adv_isa_drivers_sem);
		
		
/* 		if (&isa_drv->isa_drivers == &adv_isa_drivers) { */
		if (!isa_dev_rm_flag) {
			printk(KERN_ERR"ISA device does not exist or match.\n");
			return -ENODEV;
		}

/* 		if (&isa_dev->device_list == &isa_drv->devices) { */
		if (!isa_dev_rm_flag) {
			printk(KERN_ERR
			       "ISA device does not exist or match.\n");
			return -ENODEV;
		}

		adv_isa_remove_device(isa_drv, isa_dev);
		kfree(isa_dev);
		isa_dev = NULL;

		return 0;
	}


	if (cmd == GET_DEVICE_DRIVER_NAME) {

		adv_get_device_info p_get_device_info;
		if ( copy_from_user(&p_get_device_info, (void *)arg, sizeof(adv_get_device_info))) {
			return -EFAULT;
		}
		
		minor = p_get_device_info.dev_num;
		down_read(adv_dev_loc_sema);
		dev_loc = adv_dev_loc + minor;
		
		if (dev_loc->device_ptr == NULL) {
			up_read(adv_dev_loc_sema);
			return -ENODEV;
		}
		
		if (copy_to_user(p_get_device_info.driver_name, dev_loc->driver_ptr->driver_name, strlen(dev_loc->driver_ptr->driver_name)+1)) {
			up_read(adv_dev_loc_sema);
			return -EFAULT;
		}
		up_read(adv_dev_loc_sema);
		return 0;
	}
	
	minor = MINOR(inode->i_rdev);

	down_write(adv_dev_loc_sema);
	dev_loc = adv_dev_loc+minor;
     
	if (!dev_loc->devcnf) {
		printk("Please Config your device node first!\n");
		printk("Please run advdevice_config to config device.\n");
		return -ENXIO;
	}

	device =dev_loc->device_ptr;
	up_write(adv_dev_loc_sema);

	if (device == NULL) {
		return -ENODEV;
	}

	
	switch(cmd) {
	case ADV_AI_DMA_START:
	case ADV_AI_INT_START:
	case ADV_FAI_INT_START_EXT:
	case ADV_AI_INT_SCAN_START:
		if(!dev_loc->fai_filep) {
			dev_loc->fai_filep = filep;
			dev_loc->fai_cmd = ADV_FAI_TERMINATE;

		} else {

			return -EBUSY;
		}
		break;
	case ADV_FAO_DMA_START:
	case ADV_AO_DMA_START:
		if(!dev_loc->fao_filep) {
			dev_loc->fao_filep = filep;
			dev_loc->fao_cmd = ADV_FAO_TERMINATE;
		} else {
			return -EBUSY;
		}
		break;
	case ADV_FDI_DMA_START:
		if(!dev_loc->fdi_filep) {
			dev_loc->fdi_filep = filep;
			dev_loc->fdi_cmd = ADV_FDI_DMA_STOP;
		} else {
			return -EBUSY;
		}
		break;
	case ADV_FDO_DMA_START:
		if(!dev_loc->fdo_filep) {
			dev_loc->fdo_filep = filep;
			dev_loc->fdo_cmd = ADV_FDO_DMA_STOP;
		} else {
			return -EBUSY;
		}
		break;

	default:
		break;
	}
	
	if(dev_loc->fai_filep != filep) {
		switch(cmd) {
		case ADV_FAI_TERMINATE:
			return -EBUSY;
		default:
			break;
		}
	}
	

	if(dev_loc->fao_filep != filep) {
		switch(cmd) {
		case ADV_FAO_TERMINATE:
			return -EBUSY;
		default:
			break;
		}
	}


	if(dev_loc->fdi_filep != filep) {
		switch(cmd) {
		case ADV_FDI_DMA_STOP:
			return -EBUSY;
		default:
			break;
		}
	}


	if(dev_loc->fdo_filep != filep) {
		switch(cmd) {
		case ADV_FDO_DMA_STOP:
			return -EBUSY;
		default:
			break;
		}
	}

	
	if (device->driver->ops->ioctrl) {
		ret = device->driver->ops->ioctrl(device, cmd,arg);
	}
	if(ret) {
		return ret;
	}
	if(dev_loc->fai_filep == filep) {
		switch(cmd) {
		case ADV_FAI_TERMINATE:
			dev_loc->fai_filep = NULL;
			break;
		default:
			break;
		}
	}
	if(dev_loc->fao_filep == filep) {
		switch(cmd) {
		case ADV_FAO_TERMINATE:
			dev_loc->fao_filep = NULL;
			break;
		default:
			break;
		}
	}
	if(dev_loc->fdi_filep == filep) {
		switch(cmd) {
		case ADV_FDI_DMA_STOP:
			dev_loc->fdi_filep = NULL;
			break;
		default:
			break;
		}
	}
	if(dev_loc->fdo_filep == filep) {
		switch(cmd) {
		case ADV_FDI_DMA_STOP:
			dev_loc->fdo_filep = NULL;
			break;
		default:
			break;
		}
	}
	
	
	return 0;
}

struct file_operations advdrv_fops = {
	open:	advdrv_open,
	release:	advdrv_release,
	ioctl:	advdrv_ioctl,
	read:	advdrv_read,
	write:     advdrv_write,
	mmap:	advdrv_mmap,
	poll:	advdrv_poll,
};


/************************************************************************
 * static int __init advdrv_init(void)
 * 
 * Description:  Initialize the advdrv_core driver : Register char device and get the  major
 *************************************************************************/
static int __init advdrv_init(void)
{
	int i;
          
	/* Get Dynamic Major from the Kernel */
	adv_major = register_chrdev(GET_DYNAMIC_MAJOR, "advdrv", &advdrv_fops);
	if (adv_major < 0) {
		KdPrint(KERN_WARNING "advdev: Can't get major %d\n", adv_major);
		return -EPFNOSUPPORT;  /* Protocol not supported */
	}
			
	printk("Registering Driver with major %i.\n", adv_major);
  
	adv_dev_loc = (adv_device_loc *)kmalloc(sizeof(adv_device_loc)*ADV_NDEVICES, GFP_KERNEL);
	if (!adv_dev_loc)
		return -ENOMEM;
	memset(adv_dev_loc, 0, sizeof(adv_device_loc)*ADV_NDEVICES);
 
	for (i=0;i<ADV_NDEVICES;i++) {
		adv_dev_loc[i].minor=i;
	}
	/* initialize semaphore */
	adv_drivers_sema = (struct semaphore *)kmalloc(sizeof(struct semaphore), GFP_KERNEL);
	adv_dev_loc_sema = (struct rw_semaphore *)kmalloc(sizeof(struct rw_semaphore), GFP_KERNEL);
	if ((adv_drivers_sema == NULL) || (adv_dev_loc_sema == NULL))
	{
		kfree(adv_dev_loc);
		if (adv_drivers_sema)
			kfree(adv_drivers_sema);
		if (adv_dev_loc_sema)
			kfree(adv_dev_loc_sema);
		return -ENOMEM;
	  
	}
     
	init_MUTEX(adv_drivers_sema);
	init_rwsem(adv_dev_loc_sema);
   
	return 0; 
}

/************************************************************************
 * static void __exit advdrv_cleanup(void)
 * 
 * Description:  Close driver - Clean up remaining memory and unregister
 * 								the major.
 *************************************************************************/
static void __exit advdrv_exit(void)
{ 
	unregister_chrdev(adv_major, "advdrv");
/* 	unregister_chrdev(adv_major, "advdev"); */
	kfree(adv_dev_loc);
	kfree(adv_dev_loc_sema);
	kfree(adv_drivers_sema);
//    printk("Device driver removed.\n"); 
}



module_init(advdrv_init);
module_exit(advdrv_exit);

EXPORT_SYMBOL_GPL(advdrv_register_driver);
EXPORT_SYMBOL_GPL(advdrv_unregister_driver);
EXPORT_SYMBOL_GPL(advdrv_remove_device);
EXPORT_SYMBOL_GPL(advdrv_add_device);

EXPORT_SYMBOL_GPL(advdrv_device_set_boardid);

EXPORT_SYMBOL_GPL(advdrv_device_get_boardid);
EXPORT_SYMBOL_GPL(advdrv_device_set_devname);
EXPORT_SYMBOL_GPL(advdrv_device_get_devname);
EXPORT_SYMBOL_GPL(advdrv_device_set_nodename);
EXPORT_SYMBOL_GPL(advdrv_device_get_nodename);
EXPORT_SYMBOL_GPL(advdrv_device_set_privdata);
EXPORT_SYMBOL_GPL(advdrv_device_get_privdata);

EXPORT_SYMBOL_GPL(adv_process_info_header_init);
EXPORT_SYMBOL_GPL(adv_process_info_remove);
EXPORT_SYMBOL_GPL(adv_process_info_add);
EXPORT_SYMBOL_GPL(adv_process_info_enable_event);
EXPORT_SYMBOL_GPL(adv_process_info_disable_event);
EXPORT_SYMBOL_GPL(adv_process_info_isenable_event);
EXPORT_SYMBOL_GPL(adv_process_info_set_event_all);
EXPORT_SYMBOL_GPL(adv_process_info_get_special_event_thresh);
EXPORT_SYMBOL_GPL(adv_process_info_isset_event);
EXPORT_SYMBOL_GPL(adv_process_info_isset_special_event);
EXPORT_SYMBOL_GPL(adv_process_info_reset_special_event);
EXPORT_SYMBOL_GPL(adv_process_info_check_special_event);
EXPORT_SYMBOL_GPL(adv_process_info_isenable_event_all);
EXPORT_SYMBOL_GPL(adv_process_info_check_event);
EXPORT_SYMBOL_GPL(adv_process_info_set_error);
EXPORT_SYMBOL_GPL(adv_process_info_get_error);

EXPORT_SYMBOL_GPL(adv_map_user_pages);
EXPORT_SYMBOL_GPL(adv_unmap_user_pages);
MODULE_LICENSE("GPL");

MODULE_AUTHOR("Advantech Linux driver Team");
