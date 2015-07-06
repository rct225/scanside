/********************************************************************
 *				                                   
 * 		Copyright 2004 IAG Software Team,                  
 *                                                                  
 * 			BeiJing R&D Center                         
 *	                                                           
 * 			Advantech Co., Ltd.                        
 *                                                                  
 * 	    Advantech PCI-1742 Series Device Driver for Linux             
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
#include "PCI1742.h"



/**
 * pci1742_gp_ao - writes a binary value to one of the analog   
 *                 output channels, changing the voltage pro-   
 *                 duced at the channel.
 *                 
 * @device: Points to the device object          
 * @chan: channel number
 * @bin_data: binary data to be output
 *
 * 0x0e   -set AO reference control             
 * 0x0a   -set channel 0 output data            
 * 0x0c   -set channel 1 output data            
 *                                                            
 */
static INT32S pci1742_gp_ao(adv_device *device, INT16U chan, INT16U bin_data)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U wdata;
	INT8U aoref = 0;
	INT8U mask = 0;
     

	aoref = advInp(privdata, 0x0e);     

	/* check AO gain range */
	if ((privdata->AOSettings[chan].fAOMaxVol == 5.0) &&
	    (privdata->AOSettings[chan].fAOMinVol == 0.0))
		mask = 0x02;
     
	if ((privdata->AOSettings[chan].fAOMaxVol == 10.0) &&
	    (privdata->AOSettings[chan].fAOMinVol == 0.0))
		mask = 0x03;

	if ((privdata->AOSettings[chan].fAOMaxVol == 5.0) &&
	    (privdata->AOSettings[chan].fAOMinVol == -5.0))
		mask = 0x00;

	if ((privdata->AOSettings[chan].fAOMaxVol == 10.0) &&
	    (privdata->AOSettings[chan].fAOMinVol == -10.0))
		mask = 0x01;

	/* check AO reference source*/
	if (privdata->AOSettings[chan].usAOSource == 0) { /* internal */
		mask &= 0xfb;
	} else {		/* external */
		mask |= 0x04;
	}
	
	/* check AO gate function */
	if (privdata->sync_flag) {
		mask |= 0x08;
	} else {
		mask &= 0xf7;
	}

	aoref |= mask;
	advOutp(privdata, 0xe + chan, aoref);
     
	wdata = bin_data & 0xffff;
	advOutpw(privdata, 0x0a + (chan * 2), wdata);

	return 0;
}


/**
 * adv_gp_aoscale - gp(general purpose) function called by adv_ao_scale
 *                  and adv_ao_voltageout
 * 
 * @device: Points to the device object          
 * @chan: channel number
 * @output_value: voltage value
 * @output_type: OUTPUT_TYPE_CURRENT or OUTPUT_TYPE_VOLTAGE
 */
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
		case PCI1742:
			max_count = (float) 65535;
			bin_data = (INT16U) ((max_count * (float) (output_value - min_val))
					     / (max_val - min_val));
			break;
		}

		break;
	default:
		break;
	}
	

	return bin_data;
}

/**
 * adv_ao_config - record the output range and polarity selected for each analog
 *                 output channel.
 * 
 * @device: Points to the device object          
 * @arg: argument from user                   
 */
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
	case PCI1742:
		max_chan = 1;
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

/**
 * adv_ao_range_set - ao channel configuretion
 *
 * @device: pointe to the device object
 * @output_type: voltage or current
 * @chan: channel index
 * @ref_src: reference source (0 or 1)
 * @max_value: maxium reference voltage(or current)
 * @min_value: minium reference voltage(or current)
 */
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
	default:
		break;
	}

	privdata->AOSettings[chan].usAOSource = ref_src;
	privdata->output_type[chan] = output_type;
	
	return SUCCESS;
}

/**
 * adv_ao_binary_out - writes a binary value to one of the analog output channels,
 *                     changing the voltage produced at the channel.
 *               
 * @device: Points to the device object          
 * @arg: argument from user
 *
 * 0x0e   -set AO reference control             
 * 0x0a   -set channel 0 output data            
 * 0x0c   -set channel 1 output data            
 */
INT32S adv_ao_binary_out(adv_device *device, void *arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	PT_AOBinaryOut ao_binary_out;
	INT16U ret;



	if (copy_from_user(&ao_binary_out, (void *) arg, sizeof(PT_AOBinaryOut))) {
		return MemoryCopyFailed;
	}	
     
	switch (privdata->device_type) {
	case PCI1742:
		if (ao_binary_out.chan > 1)
			return InvalidChannel;
		else
			ret = pci1742_gp_ao(device, ao_binary_out.chan, ao_binary_out.BinData);
		break;
	default:
		return BoardIDNotSupported;
	}

	return ret;
}

/**
 * adv_ao_voltage_out - writes a floating-point voltage value to one of the analog
 *                      output channels
 *               
 * @device: Points to the device object          
 * @arg: argument from user                   
 */
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
	case PCI1742:
		max_chan = 1;
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
	case PCI1742:
		ret = pci1742_gp_ao(device, chan, wdata);
		break;
	default:
		return BoardIDNotSupported;
	}

	return ret;
}

/**
 * adv_ao_scale - scale a voltage to a binary value
 * 
 * @device: Points to the device object          
 * @arg: argument from user                   
 */
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
	case PCI1742:
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
  

/**
 * adv_enable_ao_sync - enable / disable synchonously control bit
 * 
 * @device: Points to the device object          
 * @arg: argument from user                   
 */
INT32S adv_enable_ao_sync(adv_device *device, INT16U arg)
{
	private_data *privdata = (private_data *) (device->private_data);
	INT16U tmp = arg;
	
     
	if ((tmp != 0) && (tmp != 1))
		return InvalidInputParam;
     
	switch (privdata->device_type) {
	case PCI1742:
		privdata->sync_flag = tmp;
		break;
	default:
		return BoardIDNotSupported;
	}

	return SUCCESS;
}

/**
 * adv_write_sync_ao - output data synchonously
 * 
 * @device: Points to the device object          
 */
INT32S adv_write_sync_ao(adv_device *device)
{
	private_data *privdata = (private_data *) (device->private_data);


	switch (privdata->device_type) {
	case PCI1742:
		advInpdw(privdata, 0x0a);
		break;
	default:
		return BoardIDNotSupported;
	}

	return SUCCESS;
}
