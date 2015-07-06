
/* base module includes */
#include <linux/module.h>

#include "PCM3730I.h"

#ifndef CONFIG_PCI
#  error "This driver needs have PCI support."
#endif


int adv_init_device_di_range(private_data *privdata)
{
     ULONG err = 0;
     switch(privdata->device_type)
     {
     case PCM3730I:
	  privdata->di_port_reg_start = 0;
	  privdata->di_port_num = 2;
	  break;
     default:
	  err = -1;
	  break;
	  
     }
     return err;
}

int adv_init_device_do_range(private_data *privdata)
{
     ULONG err = 0;
      
     switch(privdata->device_type)
     {
     case PCM3730I:
	  privdata->do_port_reg_start = 0;
	  privdata->do_port_num = 2;
	  break;
     default:
	  err = -1;
	  break;
	  
     }
     return err;
}

int adv_clear_device_int_flag(private_data *privdata)
{
     ULONG err = 0;
     
     switch(privdata->device_type)
     {
     case PCM3730I:
	  advOutp(privdata, 0x10, 0x0f);
	  break;

     default:
	  err = -1;
	  break;
     }
     return err;
}
int adv_disable_device_int(private_data *privdata)
{
     ULONG err = 0;
     
     switch(privdata->device_type)
     {
     case PCM3730I:
	  advOutp(privdata, 0x08, 0x00);
	  break;
     default:
	  err = -1;
	  break;
     }
     return err;
}


int adv_get_device_boardID(private_data *privdata)
{  
     privdata->boardID = advInp(privdata, 0x04) & 0x0f;
     return 0;
     
}
INT32U adv_get_int_control_code(private_data *privdata, BYTE *ctlcode)
{
	adv_process_info_header *p = &privdata->ptr_process_info;
	INT32U i;
	INT32U enable;
     
	*ctlcode = 0;
     
	for(i = 0; i < 4; i++)
	{
		enable = adv_process_info_isenable_event_all(p, i);
		if(enable){
			*ctlcode |= 1<<i;
		}
	}
	return 0;
}

/* caculate the control code according the signal status
 * the status include:  interrupt enable/disable status
 *                      interrupt trigger mode status*/
int adv_get_contorl_code(ULONG device_type, ULONG status, BYTE *ctrlcode)
{
     int ret = 0;
     *ctrlcode = 0;

     switch(device_type)
     {
     case PCM3730I:
		if(status&0x0100)
			*ctrlcode=1;
		else if(status&0x0001)
			*ctrlcode=2;
		else
			*ctrlcode=3;
			
		
     default:
	  ret = -1;
	  break;
     }
     return ret;
}


