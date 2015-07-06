/********************************************************************
 *				                                   
 * 		Copyright 2004 IAG Software Team,                  
 *                                                                  
 * 			BeiJing R&D Center                         
 *	                                                           
 * 			Advantech Co., Ltd.                        
 *                                                                  
 * 	    Advantech PCI-1720 Series Device Driver for Linux             
 *                                                                  
 *                                                                  
 *                                                                  
 * 	File Name:                                                 
 * 		advao.c                                            
 * 	Abstract:                                                  
 * 		This file contains routines for ao function.
 * 	Note:
 * 	        adv_gp_fun() -general purpose function
 *                                                                  
 *                                                                  
 * 	Version history                                            
 * 	01/09/2006			Create by LI.ANG           
 *                                                                  
 ********************************************************************/
#include "PCI1720.h"

static unsigned char mask_a7_4[4] = {
	0x01, 0x02, 0x04, 0x08
};


/**************************************************************
 * Description:  fixup hardware bug for PCI-1724
 *                                                            
 * Input:        channel -dac channel number
 *               
 * Ouput:        32-bits data
 **************************************************************/
static INT32U serial_data_format_a7_a0(INT16U channel)
{
	INT32U data;
     
	if (channel == 55)
		return (0xf0 << 16);
	else {
		data = mask_a7_4[channel / 10];
		data <<= 4;
		data += channel % 10;
		return (data << 16);
	}
}

/**************************************************************
 * Description:  writes a binary value to one of the analog   
 *               output channels, changing the voltage pro-   
 *               duced at the channel.                        
 *               0x0e   -set AO reference control             
 *               0x0a   -set channel 0 output data            
 *               0x0c   -set channel 1 output data            
 *                                                            
 * Input:        device -Points to the device object          
 *               chan   -channel number
 *               bin_data  -binary data to be output
 *               
 * Ouput:        errno                                        
 *               SUCCESS
 **************************************************************/
static INT32S pci1720_gp_ao(adv_device *device, INT16U chan, INT16U bin_data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U wdata;
	INT8U aoref = 0, mask = 0;
     

	aoref = advInp(privdata, 0x08);     
	mask = 0x03 << (chan * 2);
	aoref &= ~mask;
     
	if ((privdata->AOSettings[0].fAOMaxVol == 5.0) &&
	    (privdata->AOSettings[0].fAOMinVol == 0.0))
		mask = 0x00;
     
	if ((privdata->AOSettings[0].fAOMaxVol == 10.0) &&
	    (privdata->AOSettings[0].fAOMinVol == 0.0))
		mask = 0x01;

	if ((privdata->AOSettings[0].fAOMaxVol == 5.0) &&
	    (privdata->AOSettings[0].fAOMinVol == -5.0))
		mask = 0x02;

	if ((privdata->AOSettings[0].fAOMaxVol == 10.0) &&
	    (privdata->AOSettings[0].fAOMinVol == -10.0))
		mask = 0x03;

	mask <<= (chan * 2);
	aoref |= mask;
	advOutp(privdata, 0x08, aoref);
     
	wdata = bin_data & 0x0fff;
	advOutpw(privdata, chan * 2, wdata);
      
	return SUCCESS;
}

/**************************************************************
 * Description:  writes a binary value to one of the analog   
 *               output channels, changing the voltage pro-   
 *               duced at the channel.                        
 *               0x0e   -set AO reference control             
 *               0x0a   -set channel 0 output data            
 *               0x0c   -set channel 1 output data            
 *                                                            
 * Input:        device -Points to the device object          
 *               chan   -channel number
 *               bin_data  -binary data to be output
 *               
 * Ouput:        errno                                        
 *               SUCCESS
 **************************************************************/
static INT32S pci1723_gp_ao(adv_device *device, INT16U chan, INT16U bin_data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U wdata;
	INT16U aoref = 0, mask = 0;

     
	chan = chan & 0x3;
	aoref = advInpw(privdata, 0x14) & 0x100;     
	aoref |= chan;
     
	if ((privdata->AOSettings[0].fAOMaxVol == 10.0) &&
	    (privdata->AOSettings[0].fAOMinVol == -10.0))
		mask = 0x00;
     
	if ((privdata->AOSettings[0].fAOMaxCur == 20.0) &&
	    (privdata->AOSettings[0].fAOMinCur == 0.0))
		mask = 0x02;

	if ((privdata->AOSettings[0].fAOMaxCur == 20.0) &&
	    (privdata->AOSettings[0].fAOMinCur == 4.0))
		mask = 0x03;

	mask <<= 4;
	aoref |= mask;
	advOutpw(privdata, 0x14, aoref);
	advOutpw(privdata, 0x26, 0x00);
     
	wdata = bin_data & 0xffff;
	advOutpw(privdata, chan * 2, wdata);
     
	return SUCCESS;
}

/**************************************************************
 * Description:  writes a binary value to one of the analog   
 *               output channels, changing the voltage pro-   
 *               duced at the channel.                        
 *               0x0e   -set AO reference control             
 *               0x0a   -set channel 0 output data            
 *               0x0c   -set channel 1 output data            
 *                                                            
 * Input:        device -Points to the device object          
 *               chan   -channel number
 *               bin_data  -binary data to be output
 *               
 * Ouput:        errno                                        
 *               SUCCESS
 **************************************************************/
static INT32S pci1724_gp_ao(adv_device *device, INT16U chan, INT16U bin_data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U wdata = 0, dac_chan = 0;
	INT32U dac_data, status;
     

	wdata = bin_data & 0x3fff;
	dac_chan = ((chan / 8) * 10) + (chan % 8);
     
	/* fixup hardware bug (AO calibration data will lost) */
	dac_data = serial_data_format_a7_a0(dac_chan) | DAC_DATA_NORMAL_MODE | (wdata & 0x3fff);
	advOutpdw(privdata, 0x00, dac_data);
     

	/* we must do following step after output data, this cause by hardware bug */
	do {
		status = advInpdw(privdata, 0x04) & 0x02;
	} while (status);

	mdelay(1000);
     
/*      printk("chan==0x%x\n", chan); */
/*      printk("dac_channel==0x%x\n", dac_chan); */
/*      printk("wdata==0x%x\n", wdata); */
/*      printk("dac_data==0x%lx\n", dac_data); */

	return SUCCESS;
}

/**************************************************************
 * Description:  writes a binary value to one of the analog   
 *               output channels, changing the voltage pro-   
 *               duced at the channel.                        
 *                                                            
 * Input:        device    -Points to the device object          
 *               chan      -channel number
 *               bin_data  -binary data to be output
 *               
 * Ouput:        errno                                        
 *               SUCCESS
 **************************************************************/
static INT32S pci1727_gp_ao(adv_device *device, INT16U chan, INT16U bin_data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U wdata = 0;
	INT16U tmp;
     
	tmp = advInp(privdata, 0x22);
	if (tmp & 0x0001) {
		return DaShiftBusy;
	}
	
	wdata = (bin_data & 0x3f00) >> 8; 
	advOutp(privdata, 0x00 + (chan * 2), wdata);

	wdata = bin_data & 0x00ff;
	advOutp(privdata, 0x00 + (chan * 2 + 1), wdata);
      
	return SUCCESS;
}

/*******************************************************************************
 * Description:  gp(general purpose) function called by adv_ao_scale
 *               and adv_ao_voltageout
 * 
 * Input:        device -Points to the device object          
 *               chan   -channel number
 *               output_value    -voltage value
 *               output_type     -OUTPUT_TYPE_CURRENT or OUTPUT_TYPE_VOLTAGE
 *               
 * Ouput:        binary data
 *******************************************************************************/
static INT16U adv_gp_aoscale(adv_device *device, INT16U chan, float output_value, INT16U output_type)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U bin_data = 0;
	float max_val = 0;
	float min_val = 0;
	float max_count = 0;

	

	switch (output_type) {
	case OUTPUT_TYPE_VOLTAGE:
		max_val = privdata->AOSettings[chan].fAOMaxVol;
		min_val = privdata->AOSettings[chan].fAOMinVol;

		switch (privdata->device_type) {
		case PCI1720:
			max_count = (float) 4095;
			bin_data = (INT16U) ((max_count * (float) (output_value - min_val))
					     / (max_val - min_val));
			break;
		case PCI1723:
			max_count = (float) 65535;
			bin_data = (INT16U) ((max_count * (float) (output_value - min_val))
					     / (max_val - min_val));
			break;
		case PCI1724:
			max_count = (float) 16383;
			bin_data = (INT16U) ((max_count * (float) (output_value - min_val))
					     / (max_val - min_val));
			break;
		case PCI1727:
		{
			float ftmp;
			float ftmp1;
			float ftmp2;
  
			max_count = (float) 16383;
			
			if ((min_val == 0.0) || (min_val == -5.0)) {
				ftmp1 = (float) ((output_value + 10) / 20);
				ftmp2 = (float) ((output_value - min_val) / (max_val - min_val));
				
				if (ftmp2 == 0) {
					if (min_val == -5.0) {
						bin_data = (INT16U) (max_count / 4);
					} else {
						bin_data = (INT16U) (max_count / 2);
					}
				} else {
					ftmp = (float) (ftmp1 / ftmp2);
					bin_data = (INT16U) ((max_count * ftmp * (float) (output_value - min_val))
							     / (max_val - min_val));
				}					
			} else {
				bin_data = (INT16U) ((max_count * (float) (output_value - min_val))
						     / (max_val - min_val));
			}
		}
		break;
		}
/* 	printk("max_cnt = 0x%.8x\n", (INT32U) max_count); */
/* 	printk("out_val = %d\n", (INT32U) output_value); */
/* 	printk("min_val = %d\n", (INT32U) min_val); */
/* 	printk("max_val = %d\n", (INT32U) max_val); */
/* 	printk("bindata = 0x%.4x\n", bin_data); */

		break;
	case OUTPUT_TYPE_CURRENT:
		max_val = privdata->AOSettings[chan].fAOMaxCur;
		min_val = privdata->AOSettings[chan].fAOMinCur;
		
		switch (privdata->device_type) {
		case PCI1720:
			max_count = (float) 4095;
			bin_data = (INT16U) (max_count * (float) (output_value - min_val)
					     / (max_val - min_val));
			break;
		case PCI1723:
			max_count = (float) 65535;
			bin_data = (INT16U) ((max_count - 32768) * (float) (output_value - min_val)
					     / (max_val - min_val)) + 32768;
			break;
		case PCI1724:
			max_count = (float) 16383;
			bin_data = (INT16U) ((max_count - 8192) * (float) (output_value - min_val)
					     / (max_val - min_val)) + 8192;	      
			break;
		case PCI1727:
		{
			float ftmp;
			float ftmp1;
			float ftmp2;
  
			max_count = (float) 16383;
			
			if ((min_val == 20.0) && (min_val == 4.0)) {
				ftmp1 = (float) (output_value / 20);
				ftmp2 = (float) ((output_value - min_val) / (max_val - min_val));
				
				if (ftmp2 == 0) {
					bin_data = (INT16U) ((float) (max_count * min_val));
				} else {
					ftmp = (float) (ftmp1 / ftmp2);
					bin_data = (INT16U) ((max_count * ftmp * (float) (output_value - min_val))
							     / (max_val - min_val));
				}					
			} else {
				bin_data = (INT16U) ((max_count * (float) (output_value - min_val))
						     / (max_val - min_val));
			}
		}
		break;
		}
		break;
	}
	

	return bin_data;
}

/******************************************************************************
 * Description:  record the output range and polarity selected for each analog
 *               output channel.
 * 
 * Input:        device -Points to the device object          
 *               arg    -argument from user                   
 * Ouput:        errno                                        
 *               0                                            
 ******************************************************************************/
INT32S adv_ao_config(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_AOConfig ao_config;
	INT16U max_chan = 0;
	INT16U ret;
	

	
	if (copy_from_user(&ao_config, arg, sizeof(PT_AOConfig))) {
		return MemoryCopyFailed;
	}

	switch (privdata->device_type) {
	case PCI1720:
		max_chan = 3;
		break;
	case PCI1723:
		max_chan = 7;
		break;
	case PCI1724:
		max_chan = 31;
		break;
	case PCI1727:
		max_chan = 11;
		break;
	default:
		return BoardIDNotSupported;
	}

	if (ao_config.chan > max_chan)
		return InvalidChannel;


	privdata->output_type[ao_config.chan] = OUTPUT_TYPE_VOLTAGE; /* default type */
	
	ret = adv_ao_range_set(device,
			       privdata->output_type[ao_config.chan],
			       ao_config.chan,
			       ao_config.RefSrc,
			       ao_config.MaxValue,
			       ao_config.MinValue);

	return SUCCESS;
}

INT32S adv_ao_range_set(adv_device *device,
			INT16U output_type,
			INT16U chan,
			INT16U ref_src,
			float max_value,
			float min_value)
{
	private_data *privdata = (private_data *) (device->private_data);
	       

	if ((ref_src != 0) && (ref_src != 1))
		return InvalidInputParam;
     
	if ((output_type != 0) && (output_type != 1))
		return InvalidInputParam;

	
	switch (output_type) {
	case OUTPUT_TYPE_VOLTAGE:
		if ((min_value < -10) || (max_value > 10))
			return InvalidInputRange;
     
		privdata->AOSettings[chan].fAOMaxVol = max_value;
		privdata->AOSettings[chan].fAOMinVol = min_value;
		break;
	case OUTPUT_TYPE_CURRENT:
		if ((min_value < 0) || (max_value > 20))
			return InvalidInputRange;
     
		privdata->AOSettings[chan].fAOMaxCur = max_value;
		privdata->AOSettings[chan].fAOMinCur = min_value;
		break;
	}

	privdata->AOSettings[chan].usAOSource = ref_src;
	privdata->output_type[chan] = output_type;
	
	return SUCCESS;
}

/**********************************************************************
 * Description:  get current ao configurations
 * 
 * Input:        device -Points to the device object          
 *               arg    -argument from user                   
 * Ouput:        errno                                        
 *               SUCCESS                                            
 **********************************************************************/
INT32S adv_ao_readconfig(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_AOConfig ao_config;


	if (copy_from_user(&ao_config, arg, sizeof(PT_AOConfig))) {
		return MemoryCopyFailed;
	}
	ao_config.RefSrc = privdata->AOSettings[ao_config.chan].usAOSource;
	ao_config.MaxValue = privdata->AOSettings[ao_config.chan].fAOMaxVol;
	ao_config.MinValue = privdata->AOSettings[ao_config.chan].fAOMinVol;

	if (copy_to_user(arg, &ao_config, sizeof(PT_AOConfig))) {
		return MemoryCopyFailed;
	}

	return SUCCESS;
}

/*****************************************************************************
 * Description:  writes a binary value to one of the analog output channels,
 *               changing the voltage produced at the channel.
 *               
 *               0x0e   -set AO reference control             
 *               0x0a   -set channel 0 output data            
 *               0x0c   -set channel 1 output data            
 *                                                            
 * Input:        device -Points to the device object          
 *               arg    -argument from user                   
 * Ouput:        errno                                        
 *               SUCCESS
 *****************************************************************************/
INT32S adv_ao_binary_out(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_AOBinaryOut ao_binary_out;
	INT16U ret;



	if (copy_from_user(&ao_binary_out, (void *) arg, sizeof(PT_AOBinaryOut))) {
		return MemoryCopyFailed;
	}	
     
	switch (privdata->device_type) {
	case PCI1720:
		if (ao_binary_out.chan > 3)
			return InvalidChannel;
		else
			ret = pci1720_gp_ao(device, ao_binary_out.chan, ao_binary_out.BinData);
		break;
	case PCI1723:
		if (ao_binary_out.chan > 7)
			return InvalidChannel;
		else
			ret = pci1723_gp_ao(device, ao_binary_out.chan, ao_binary_out.BinData);
		break;
	case PCI1724:
		if (ao_binary_out.chan > 31)
			return InvalidChannel;
		else
			ret = pci1724_gp_ao(device, ao_binary_out.chan, ao_binary_out.BinData);
		break;
	case PCI1727:
		if (ao_binary_out.chan > 11)
			return InvalidChannel;
		else
			ret = pci1727_gp_ao(device, ao_binary_out.chan, ao_binary_out.BinData);
		break;
	default:
		return BoardIDNotSupported;
	}

	return ret;
}

/****************************************************************************
 * Description:  writes a floating-point voltage value to one of the analog
 *               output channels
 *               
 * Input:        device -Points to the device object          
 *               arg    -argument from user                   
 * Ouput:        errno                                        
 *               SUCCESS                                            
 ****************************************************************************/
INT32S adv_ao_voltage_out(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_AOVoltageOut ao_voltage_out;
	INT16U chan, wdata, max_chan = 0;
	float max_value, min_value, output_value;
	INT16U ret;


	
	if (copy_from_user(&ao_voltage_out, arg, sizeof(PT_AOVoltageOut))) {
		return MemoryCopyFailed;
	}	

	chan = ao_voltage_out.chan;
	output_value = ao_voltage_out.OutputValue;

	switch (privdata->device_type) {
	case PCI1720:
		max_chan = 3;
		break;
	case PCI1723:
		max_chan = 7;
		break;
	case PCI1724:
		max_chan = 31;
		break;
	case PCI1727:
		max_chan = 11;
		break;
	default:
		return BoardIDNotSupported;
	}
	if (chan > max_chan)
		return InvalidChannel;
     
	max_value = privdata->AOSettings[chan].fAOMaxVol;
	min_value = privdata->AOSettings[chan].fAOMinVol;

	if ((output_value < min_value) || (output_value > max_value))
		return InvalidInputRange;

	wdata = adv_gp_aoscale(device, chan, output_value, privdata->output_type[chan]);

	switch (privdata->device_type) {
	case PCI1720:
		ret = pci1720_gp_ao(device, chan, wdata);
		break;
	case PCI1723:
		ret = pci1723_gp_ao(device, chan, wdata);
		break;
	case PCI1724:
		ret = pci1724_gp_ao(device, chan, wdata);
		break;
	case PCI1727:
		ret = pci1727_gp_ao(device, chan, wdata);
		break;
	default:
		return BoardIDNotSupported;
	}

	return ret;
}

/****************************************************************************
 * Description:  writes a floating-point current value to one of the analog
 *               output channels
 *               
 * Input:        device -Points to the device object          
 *               arg    -argument from user                   
 * Ouput:        errno                                        
 *               SUCCESS                                            
 ****************************************************************************/
INT32S adv_ao_current_out(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_AOCurrentOut ao_current_out;
	INT16U chan, wdata = 0, max_chan = 0;
	float max_value, min_value, output_value;
	INT16U ret;


	if (copy_from_user(&ao_current_out, arg, sizeof(PT_AOCurrentOut))) {
		return MemoryCopyFailed;
	}	

	chan = ao_current_out.chan;
	output_value = ao_current_out.OutputValue;

	switch (privdata->device_type) {
	case PCI1720:
		max_chan = 3;
		break;
	case PCI1723:
		max_chan = 7;
		break;
	case PCI1724:
		max_chan = 31;
		break;
	case PCI1727:
		max_chan = 11;
		break;
	default:
		return BoardIDNotSupported;
	}

	if (chan > max_chan)
		return InvalidChannel;
     
	max_value = privdata->AOSettings[chan].fAOMaxCur;
	min_value = privdata->AOSettings[chan].fAOMinCur;

	
	if ((output_value < min_value) || (output_value > max_value))
		return InvalidInputRange;

	wdata = adv_gp_aoscale(device, chan, output_value, privdata->output_type[chan]);

	switch (privdata->device_type) {
	case PCI1720:
		ret = pci1720_gp_ao(device, chan, wdata);
		break;
	case PCI1723:
		ret = pci1723_gp_ao(device, chan, wdata);
		break;
	case PCI1724:
		ret = pci1724_gp_ao(device, chan, wdata);
		break;
	case PCI1727:
		ret = pci1727_gp_ao(device, chan, wdata);
		break;
	default:
		return BoardIDNotSupported;
	}

	return ret;
}

/**************************************************************
 * Description:  scale a voltage to a binary value
 * 
 * Input:        device -Points to the device object          
 *               arg    -argument from user                   
 * Ouput:        errno                                        
 *               SUCCESS                                            
 **************************************************************/
INT32S adv_ao_scale(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_AOScale ao_scale;
	INT16U bin_data;
	INT16U chan;
	

	if (copy_from_user(&ao_scale, arg, sizeof(PT_AOScale))) {
		return MemoryCopyFailed;
	}	

	switch (privdata->device_type) {
	case PCI1720:
	case PCI1723:
	case PCI1724:
	case PCI1727:
		chan = ao_scale.chan;
		bin_data = adv_gp_aoscale(device, chan, ao_scale.OutputValue, privdata->output_type[chan]);
		break;
	default:
		return BoardIDNotSupported;
	}
	
	if (copy_to_user(ao_scale.BinData, &bin_data, sizeof(INT16U))) {
		return MemoryCopyFailed;
	}	

	return SUCCESS;
}
  

/**********************************************************************
 * Description:  enable / disable synchonously control bit
 * 
 * Input:        device -Points to the device object          
 *               arg    -argument from user                   
 *               
 * Ouput:        errno
 *               SUCCESS
 **********************************************************************/
INT32S adv_enable_ao_sync(adv_device *device, INT16U arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U tmp = arg;
	
     
	if ((tmp != 0) && (tmp != 1))
		return InvalidInputParam;
     
	switch (privdata->device_type) {
	case PCI1720:
		tmp &= 0x01;
		advOutp(privdata, 0x0f, (INT8U) tmp);
		break;
	case PCI1723:
		tmp &= 0x0001;
		advOutpw(privdata, 0x12, (INT16U) tmp);
		break;
	case PCI1724:
		tmp &= 0x0001;
		advOutpw(privdata, 0x04, (INT16U) tmp);
		break;
	case PCI1727:
		tmp &= 0x01;
		advOutp(privdata, 0x1c, (INT8U) tmp);
		break;
	default:
		return BoardIDNotSupported;
	}

	return SUCCESS;
}

/************************************************************
 * Description:  output data synchonously
 * 
 * Input:        device -Points to the device object          
 *               
 * Ouput:        errno
 *               SUCCESS
 ***********************************************************/
INT32S adv_write_sync_ao(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);


	switch (privdata->device_type) {
	case PCI1720:
		advOutp(privdata, 0x09, 0x00);
		break;
	case PCI1723:
		advOutpw(privdata, 0x20, 0x00);
		break;
	case PCI1724:
		advOutp(privdata, 0x0c, 0x00);
		break;
	case PCI1727:
		advOutp(privdata, 0x1d, 0x00);
		break;
	default:
		return BoardIDNotSupported;
	}

	return SUCCESS;
}
