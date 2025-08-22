/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_config_pu_sn93711.h
	\brief		BSP Config header file
	\author		Wales Wang
	\version	0.2
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_CONFIG_PU_SN93711_H_
#define _BSP_CONFIG_PU_SN93711_H_
//------------------------------------------------------------------------------

extern unsigned char invalid_param;
#ifdef A7130
//! RF GPIO
#define RF_CKO_GPIONUM				1
#define RF_TXSW_GPIONUM				7
#endif
#ifdef S2019A
#define sPRF_PWR_CTRL(ctrl)		    {								\
									RTC_SetGPO_1(ctrl, ((!ctrl)?RTC_PullDownEnable:RTC_PullDownDisable));			\
                                    }
#define sPRF_PWR_RST			    {								\
                                    sPRF_PWR_CTRL(0);               \
                                    osDelay(100);                   \
                                    sPRF_PWR_CTRL(1);               \
                                    osDelay(50);                    \
                                    }
#endif
//------------------------------------------------------------------------------
//! Sensor GPIO
//------------------------------------------------------------------------------
//! LCD GPIO
#define LCDBL_ENABLE(en)		{	GPIO->GPIO_OE0 = en;	GPIO->GPIO_O0 = en; }		//PWM->PWM_EN2 	= en;	

#define LCD_BACKLIGHT_CTRL(LvL)		(LvL = LvL)//(PWM->PWM2_HIGH_CNT = LvL)
#define	LCD_PWR_ENABLE				(GPIO->GPIO_O2 = 0)	//output low
#define	LCD_PWR_DISABLE				(GPIO->GPIO_O2 = 1)	//output high

#if (defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6)|| defined(BSP_D_SN93703_TC358778_RX_V6))
#define	LCD_RESET(x)		{ PWM->PWM2_HIGH_CNT = 0;			\
													osDelay(x);						\
													PWM->PWM2_HIGH_CNT = 1;}	
#else
#define	LCD_RESET(x)
#endif
//------------------------------------------------------------------------------
//! Video GPIO
//------------------------------------------------------------------------------
//! Audio
//! APP Audio I2S mode define
#define BSP_ADO_I2S_MODE_ENABLE		0
//! APP Audio aec/nr process by hw/sw define
#define APP_ADO_AEC_NR_TYPE			AEC_NR_SW
//! Speaker GPIO
#define SPEAKER_EN(en)				(invalid_param)//(GPIO->GPIO_O12 = en)
//------------------------------------------------------------------------------
//! Power GPIO
//------------------------------------------------------------------------------
//! LED GPIO
#define POWER_LED_IO						(invalid_param)//(GPIO->GPIO_O2)
#define POWER_LED_IO_ENABLE			(invalid_param)//(GPIO->GPIO_OE2)
#define	SIGNAL_LED_IO						(invalid_param)//(GPIO->GPIO_O3)
#define	SIGNAL_LED_IO_ENABLE		(invalid_param)//(GPIO->GPIO_OE3)
//------------------------------------------------------------------------------
//! BSP RTC Time Select
#define BSP_RTC_TIMER_SEL			RTC_TIMER_INTERNAL

//------------------------------------------------------------------------------
//! SD Dectect PIN Enable
#define BSP_SD_DEC_PIN_CUST_EN		0
//------------------------------------------------------------------------------
//! SF Write Protect use GPIO
#define SF_WP_GPIN					14		//!14 is no wp pin
#endif	//! End of #ifndef _BSP_CONFIG_H_

