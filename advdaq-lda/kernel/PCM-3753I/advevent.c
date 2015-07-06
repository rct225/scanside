

/* base module includes */

#include <linux/module.h>
#include "PCM3753I.h"

/**
 * adv_enable_event -
 * @device - device handle
 * @EventType - Event ID
 * @Enabled - Enable or disable
 * @Count - how many interrupts to raise one event
 *
 */
INT32S adv_enable_event(adv_device *device,ULONG EventType,USHORT Enabled,USHORT Count)
{
	private_data *privdata = (private_data*)(device->private_data);
	INT16U reg_val;
	INT32S ret = 0;
	
	if(Enabled){

		/* set the interrupt control register */
		switch(EventType){
		case ADS_EVT_DI_PATTERNMATCH_PORT0:
			/* save current setting */
			reg_val = advInp(privdata, INT_PATTERN0_CTRL_OFFSET);
			/* stop PA0 pattern match interrupt */
			advOutp(privdata, INT_PATTERN0_CTRL_OFFSET, 0x8a);
			/* Set pattern match value and pattern match enabled channel */
			advOutp(privdata, PATTERN0_VALUE_OFFSET, privdata->cDiPatternMatchValue[0]);
			advOutp(privdata, PATTERN0_ENABLE_OFFSET, privdata->cDiPatternMatchEnabledChannel[0]);
			printk("pm:%x\n", privdata->cDiPatternMatchEnabledChannel[0]);
			printk("pmvalue:%x\n", privdata->cDiPatternMatchValue[0]);

			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				0, Count);
			/* Enable Interrupt */
			if(privdata->cDiPatternMatchEnabledChannel[0])
				reg_val |= 0x01;
			else
				reg_val &= 0xfe;
		
			advOutp(privdata, INT_PATTERN0_CTRL_OFFSET, reg_val);
printk("16 : %x\n",reg_val);

			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT1:
			/* Save current setting */
			reg_val = advInp(privdata, INT_CHANGE0_CTRL_OFFSET);
		
			/* Stop PB0 interrupt */
			advOutp(privdata, INT_CHANGE0_CTRL_OFFSET, 0x8a);
		
			/* Set status Change Enabled Channel */
			advOutp(privdata, CHANGE0_ENABLE_OFFSET, privdata->cDiStatusChangeEnabledChannel[0]);


			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				1, Count);
			/* Enable Interrupt */
			if(privdata->cDiStatusChangeEnabledChannel[0])
				reg_val |= 0x04;
			else
				reg_val &= 0xfb;
		
			advOutp(privdata, INT_CHANGE0_CTRL_OFFSET, reg_val);

			break;
		case ADS_EVT_DI_INTERRUPT16:
			if ( (privdata->int_src_config[0] != 1)
			     && (privdata->int_src_config[0] != 2) )
				return -EFAULT;
			
			/* Read the Port0 DI Interrupt Control Register */
			reg_val = advInp(privdata, INT_GROUP0_CTRL_OFFSET);
			/* Clear the PC0 DI Interrupt old Setting */
			reg_val &= 0x0f;
			/* Stop Port 0 interrupt so that we can configure it safely */
			advOutp(privdata, INT_GROUP0_CTRL_OFFSET, 0x8a);


			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				2, Count);
			reg_val |= (privdata->int_ctrl_reg[0] & 0x07) << 4;
			advOutp(privdata, INT_GROUP0_CTRL_OFFSET, reg_val);

			break;
		case ADS_EVT_DI_INTERRUPT40:
			if ( (privdata->int_src_config[1] != 1)
			     && (privdata->int_src_config[1] != 2) )
				return -EFAULT;
			/* Read the Port0 DI Interrupt Control Register */
			reg_val = advInp(privdata, INT_GROUP1_CTRL_OFFSET);
			/* Clear the PC0 DI Interrupt old Setting */
			reg_val &= 0x0f;

			/* Stop Port 1 interrupt so that we can configure it safely */
			advOutp(privdata, INT_GROUP1_CTRL_OFFSET, 0x8a);

			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				3, Count);
		
			reg_val |= (privdata->int_ctrl_reg[1] & 0x07) << 4;
			advOutp(privdata, INT_GROUP1_CTRL_OFFSET, reg_val);
			break;
		case ADS_EVT_DI_INTERRUPT64:
			if ( (privdata->int_src_config[2] != 1)
			     && (privdata->int_src_config[2] != 2) )
				return -EFAULT;
			/* Read the Port0 DI Interrupt Control Register */
			reg_val = advInp(privdata, INT_GROUP2_CTRL_OFFSET);
			/* Clear the PC0 DI Interrupt old Setting */
			reg_val &= 0x0f;

			/* Stop Port 1 interrupt so that we can configure it safely */
			advOutp(privdata, INT_GROUP2_CTRL_OFFSET, 0x8a);
			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				4, Count);
			reg_val |= (privdata->int_ctrl_reg[2] & 0x07) << 4;
			advOutp(privdata, INT_GROUP2_CTRL_OFFSET, reg_val);

			break;
		case ADS_EVT_DI_INTERRUPT88:
			if ( (privdata->int_src_config[3] != 1)
			     && (privdata->int_src_config[3] != 2) )
				return -EFAULT;
			/* Read the Port0 DI Interrupt Control Register */
			reg_val = advInp(privdata, INT_GROUP3_CTRL_OFFSET);
			/* Clear the PC0 DI Interrupt old Setting */
			reg_val &= 0x0f;

			/* Stop Port 1 interrupt so that we can configure it safely */
			advOutp(privdata, INT_GROUP3_CTRL_OFFSET, 0x8a);

			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				5, Count);
			reg_val |= (privdata->int_ctrl_reg[3] & 0x07) << 4;
			advOutp(privdata, INT_GROUP3_CTRL_OFFSET, reg_val);
			break;
		case ADS_EVT_DI_PATTERNMATCH_PORT12:
			/* Save current setting */
			reg_val = advInp(privdata, INT_PATTERN1_CTRL_OFFSET);
			/* Stop PA0 pattern match interrup */
			advOutp(privdata, INT_PATTERN1_CTRL_OFFSET, 0x8a);
		
			/* Set Pattern Match Value and Pattern Match Enabled Channel */
			advOutp(privdata, PATTERN1_VALUE_OFFSET, privdata->cDiPatternMatchValue[1]);
			advOutp(privdata, PATTERN1_ENABLE_OFFSET, privdata->cDiPatternMatchEnabledChannel[1]);

			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				6, Count);
			/* Enable Interrupt */
			if(privdata->cDiPatternMatchEnabledChannel[1])
				reg_val |= 0x01;
			else
				reg_val &= 0xfe;
		
			advOutp(privdata, INT_PATTERN1_CTRL_OFFSET, reg_val);

			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT13:
			/* Save current setting */
			reg_val = advInp(privdata, INT_CHANGE1_CTRL_OFFSET);
		
			/* Stop PB0 interrupt */
			advOutp(privdata, INT_CHANGE1_CTRL_OFFSET, 0x8a);
		
			/* Set status Change Enabled Channel */
			advOutp(privdata, CHANGE1_ENABLE_OFFSET, privdata->cDiStatusChangeEnabledChannel[1]);

			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				7, Count);
		
			/* Enable Interrupt */
			if(privdata->cDiStatusChangeEnabledChannel[1])
				reg_val |= 0x04;
			else
				reg_val &= 0xfb;
		
			advOutp(privdata, INT_CHANGE1_CTRL_OFFSET, reg_val);
			break;
		case ADS_EVT_DI_INTERRUPT112:
			if ( (privdata->int_src_config[4] != 1)
			     && (privdata->int_src_config[4] != 2) )
				return -EFAULT;
			/* Read the Port0 DI Interrupt Control Register */
			reg_val = advInp(privdata, INT_GROUP4_CTRL_OFFSET);
		
			/* Clear the PC0 DI Interrupt old Setting */
			reg_val &= 0x0f;
		
			/* Stop Port 0 interrupt so that we can configure it safely */
			advOutp(privdata, INT_GROUP4_CTRL_OFFSET, 0x8a);

			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				8, Count);
		
			reg_val |= (privdata->int_ctrl_reg[4] & 0x07) << 4;
			advOutp(privdata, INT_GROUP4_CTRL_OFFSET, reg_val);

			break;
		case ADS_EVT_DI_INTERRUPT136:
			if ( (privdata->int_src_config[5] != 1)
			     && (privdata->int_src_config[5] != 2) )
				return -EFAULT;
			/* Read the Port0 DI Interrupt Control Register */
			reg_val = advInp(privdata, INT_GROUP5_CTRL_OFFSET);
		
			/* Clear the PC0 DI Interrupt old Setting */
			reg_val &= 0x0f;
		
			/* Stop Port 0 interrupt so that we can configure it safely */
			advOutp(privdata, INT_GROUP5_CTRL_OFFSET, 0x8a);

			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				9, Count);
			reg_val |= (privdata->int_ctrl_reg[5] & 0x07) << 4;
			advOutp(privdata, INT_GROUP5_CTRL_OFFSET, reg_val);

			break;
		case ADS_EVT_DI_INTERRUPT160:
			if ( (privdata->int_src_config[6] != 1)
			     && (privdata->int_src_config[6] != 2) )
				return -EFAULT;
			/* Read the Port0 DI Interrupt Control Register */
			reg_val = advInp(privdata, INT_GROUP6_CTRL_OFFSET);
		
			/* Clear the PC0 DI Interrupt old Setting */
			reg_val &= 0x0f;
		
			/* Stop Port 0 interrupt so that we can configure it safely */
			advOutp(privdata, INT_GROUP6_CTRL_OFFSET, 0x8a);

			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				10, Count);
			reg_val |= (privdata->int_ctrl_reg[6] & 0x07) << 4;
			advOutp(privdata, INT_GROUP6_CTRL_OFFSET, reg_val);

			break;
		case ADS_EVT_DI_INTERRUPT184:
			if ( (privdata->int_src_config[7] != 1)
			     && (privdata->int_src_config[7] != 2) )
				return -EFAULT;
			/* Read the Port0 DI Interrupt Control Register */
			reg_val = advInp(privdata, INT_GROUP7_CTRL_OFFSET);
		
			/* Clear the PC0 DI Interrupt old Setting */
			reg_val &= 0x0f;
		
			/* Stop Port 0 interrupt so that we can configure it safely */
			advOutp(privdata, INT_GROUP7_CTRL_OFFSET, 0x8a);


			adv_process_info_enable_event(
				&privdata->ptr_process_info, 
				11, Count);
			reg_val |= (privdata->int_ctrl_reg[7] & 0x07) << 4;
			advOutp(privdata, INT_GROUP7_CTRL_OFFSET, reg_val);
			break;
		default:
			ret = -EFAULT;
			return ret;
		}

	}else{
		switch(EventType)
		{
		case ADS_EVT_DI_PATTERNMATCH_PORT0:
			adv_process_info_disable_event(&privdata->ptr_process_info, 0);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 0)) {
				/* Save current setting */
				reg_val = advInp(privdata, INT_PATTERN0_CTRL_OFFSET);

				/* Disable PA0 Pattern Match */
				reg_val &= 0xfe;
				advOutp(privdata, INT_PATTERN0_CTRL_OFFSET, reg_val);
			}
			    
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT1:
			adv_process_info_disable_event(&privdata->ptr_process_info, 1);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 1)) {
				/* Save current setting */
				reg_val = advInp(privdata, INT_CHANGE0_CTRL_OFFSET);
		
				/* Disable PB0 status change */
				reg_val &= 0xfb;
				advOutp(privdata, INT_CHANGE0_CTRL_OFFSET, reg_val);

			}
			
			break;

		case ADS_EVT_DI_INTERRUPT16:

			adv_process_info_disable_event(&privdata->ptr_process_info, 2);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 2)) {
				/* Read the Port0 DI Interrupt Control Register */
				reg_val = advInp(privdata, INT_GROUP0_CTRL_OFFSET);
		
				/* Clear the PC0 DI Interrupt old Setting */
				reg_val &= 0x0f;
		
				/* Disable PC0 DI interrupt */
				advOutp(privdata, INT_GROUP0_CTRL_OFFSET, reg_val);

			}
			
			break;
		case ADS_EVT_DI_INTERRUPT40:
			adv_process_info_disable_event(&privdata->ptr_process_info, 3);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 3)) {
				/* Read the Port0 DI Interrupt Control Register */
				reg_val = advInp(privdata, INT_GROUP1_CTRL_OFFSET);
		
				/* Clear the PC0 DI Interrupt old Setting */
				reg_val &= 0x0f;
		
				/* Disable PC0 DI interrupt */
				advOutp(privdata, INT_GROUP1_CTRL_OFFSET, reg_val);

			}
		
			break;
		case ADS_EVT_DI_INTERRUPT64:
			adv_process_info_disable_event(&privdata->ptr_process_info, 4);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 4)) {
				/* Read the Port0 DI Interrupt Control Register */
				reg_val = advInp(privdata, INT_GROUP2_CTRL_OFFSET);
		
				/* Clear the PC0 DI Interrupt old Setting */
				reg_val &= 0x0f;
		
				/* Disable PC0 DI interrupt */
				advOutp(privdata, INT_GROUP2_CTRL_OFFSET, reg_val);

			}
		
			break;
		case ADS_EVT_DI_INTERRUPT88:
			adv_process_info_disable_event(&privdata->ptr_process_info, 5);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 5)) {
				/* Read the Port0 DI Interrupt Control Register */
				reg_val = advInp(privdata, INT_GROUP3_CTRL_OFFSET);
		
				/* Clear the PC0 DI Interrupt old Setting */
				reg_val &= 0x0f;
		
				/* Disable PC0 DI interrupt */
				advOutp(privdata, INT_GROUP3_CTRL_OFFSET, reg_val);

			}
		
			break;
		case ADS_EVT_DI_PATTERNMATCH_PORT12:
			adv_process_info_disable_event(&privdata->ptr_process_info, 6);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 6)) {
				/* Save current setting */
				reg_val = advInp(privdata, INT_PATTERN1_CTRL_OFFSET);

				/* Disable PA0 Pattern Match */
				reg_val &= 0xfe;
				advOutp(privdata, INT_PATTERN1_CTRL_OFFSET, reg_val);
			}
			
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT13:
			adv_process_info_disable_event(&privdata->ptr_process_info, 7);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 7)) {
				/* Save current setting */
				reg_val = advInp(privdata, INT_CHANGE1_CTRL_OFFSET);
		
				/* Disable PB0 status change */
				reg_val &= 0xfb;
				advOutp(privdata, INT_CHANGE1_CTRL_OFFSET, reg_val);

			}
		
			break;

		case ADS_EVT_DI_INTERRUPT112:
			adv_process_info_disable_event(&privdata->ptr_process_info, 8);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 8)) {
				/* Read the Port0 DI Interrupt Control Register */
				reg_val = advInp(privdata, INT_GROUP4_CTRL_OFFSET);
		
				/* Clear the PC0 DI Interrupt old Setting */
				reg_val &= 0x0f;
		
				/* Disable PC0 DI interrupt */
				advOutp(privdata, INT_GROUP4_CTRL_OFFSET, reg_val);

			}
		
			break;
		case ADS_EVT_DI_INTERRUPT136:
			adv_process_info_disable_event(&privdata->ptr_process_info, 9);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 9)) {
				/* Read the Port0 DI Interrupt Control Register */
				reg_val = advInp(privdata, INT_GROUP5_CTRL_OFFSET);
		
				/* Clear the PC0 DI Interrupt old Setting */
				reg_val &= 0x0f;
		
				/* Disable PC0 DI interrupt */
				advOutp(privdata, INT_GROUP5_CTRL_OFFSET, reg_val);

			}

			break;
		case ADS_EVT_DI_INTERRUPT160:
			adv_process_info_disable_event(&privdata->ptr_process_info, 10);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 10)) {
				/* Read the Port0 DI Interrupt Control Register */
				reg_val = advInp(privdata, INT_GROUP6_CTRL_OFFSET);
		
				/* Clear the PC0 DI Interrupt old Setting */
				reg_val &= 0x0f;
		
				/* Disable PC0 DI interrupt */
				advOutp(privdata, INT_GROUP6_CTRL_OFFSET, reg_val);

			}

			break;
		case ADS_EVT_DI_INTERRUPT184:
			adv_process_info_disable_event(&privdata->ptr_process_info, 11);
			if (!adv_process_info_isenable_event_all(&privdata->ptr_process_info, 11)) {
				/* Read the Port0 DI Interrupt Control Register */
				reg_val = advInp(privdata, INT_GROUP7_CTRL_OFFSET);
		
				/* Clear the PC0 DI Interrupt old Setting */
				reg_val &= 0x0f;
		
				/* Disable PC0 DI interrupt */
				advOutp(privdata, INT_GROUP7_CTRL_OFFSET, reg_val);

			}

			break;

		default:
			ret = -EFAULT;
			return ret;
		}
	}
	return 0;
}


/**
 * adv_check_event -
 * @device - device handle
 * @EventType - Event ID
 * @Millisecond - wait time
 *
 */
INT32S adv_check_event(adv_device *device, INT32U *EventType, INT32U Milliseconds)
{
	private_data *privdata = (private_data*)(device->private_data);
	adv_process_info_header *p = &privdata->ptr_process_info;
	INT32S event_th = 0;
		
	if(!(*EventType))
	{
		event_th = adv_process_info_check_event(p);
		
		if(event_th <= 0){
			*EventType = 0;
		}else{
			switch(event_th -1)
			{
			case 0:
				*EventType = ADS_EVT_DI_PATTERNMATCH_PORT0;
				break;
			case 1:
				*EventType = ADS_EVT_DI_STATUSCHANGE_PORT1;
				break;
			case 2:
				*EventType = ADS_EVT_DI_INTERRUPT16;
				break;
			case 3:
				*EventType = ADS_EVT_DI_INTERRUPT40;
				break;
			case 4:
				*EventType = ADS_EVT_DI_INTERRUPT64;
				break;
			case 5:
				*EventType = ADS_EVT_DI_INTERRUPT88;
				break;
			case 6: 
				*EventType = ADS_EVT_DI_PATTERNMATCH_PORT12;
				break;
			case 7:
				*EventType = ADS_EVT_DI_STATUSCHANGE_PORT13;
				break;
			case 8:
				*EventType = ADS_EVT_DI_INTERRUPT112;
				break;
			case 9:
				*EventType = ADS_EVT_DI_INTERRUPT136;
				break;
			case 10:
				*EventType = ADS_EVT_DI_INTERRUPT160;
				break;
			case 11:
				*EventType = ADS_EVT_DI_INTERRUPT184;
				break;
			}
			
		}
	  
	}
	else{
		switch(*EventType)
		{
		case ADS_EVT_DI_PATTERNMATCH_PORT0:
			event_th = 0;
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT1:
			event_th = 1;
			break;
		case ADS_EVT_DI_INTERRUPT16:
			event_th = 2;
			break;
		case ADS_EVT_DI_INTERRUPT40:
			event_th = 3;
			break;
		case ADS_EVT_DI_INTERRUPT64:
			event_th = 4;
			break;
		case ADS_EVT_DI_INTERRUPT88:
			event_th = 5;
			break;
		case ADS_EVT_DI_PATTERNMATCH_PORT12:
			event_th = 6;
			break;
		case ADS_EVT_DI_STATUSCHANGE_PORT13:
			event_th = 7;
			break;
		case ADS_EVT_DI_INTERRUPT112:
			event_th = 8;
			break;
		case ADS_EVT_DI_INTERRUPT136:
			event_th = 9;
			break;
		case ADS_EVT_DI_INTERRUPT160:
			event_th = 10;
			break;
		case ADS_EVT_DI_INTERRUPT184:
			event_th = 11;
			break;
		default:
			event_th = -1;
			break;
		}
		if((event_th >= 0) && (event_th <=11)){
			*EventType = adv_process_info_check_special_event(p, event_th);
		}else{
			*EventType = 0;
		}
		
	}
     
	return 0;
     
}

	  
