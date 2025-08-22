/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_SN93716_TX.h
	\brief		BSP Config header file
	\author		Pierce
	\version	0.1
	\date		2021/06/29
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_SN93716_TX_H_
#define _BSP_SN93716_TX_H_
//======================================================================================================================
// CAM Board Configuration
//------------------------------------------------------------------------------
//! RF 
#ifdef A7130
#define RF_CKO_GPIONUM				7
#define RF_TXSW_GPIONUM				8
#endif
#ifdef S2019A
#define sPRF_PWR_CTRL(ctrl)			PWM->PWM11_HIGH_CNT = ctrl;
#define sPRF_PWR_RST				{												\
									sPRF_PWR_CTRL(0)								\
									osDelay(100);									\
									sPRF_PWR_CTRL(1)								\
									osDelay(50);									\
									}
#endif
//------------------------------------------------------------------------------
//! Sensor GPIO
#define SENSOR_RESET_OUT_EN         (GPIO->GPIO_OE1)
#define SENSOR_RESET_OUT            (GPIO->GPIO_O1)
#define SENSOR_RST_IO_EN(en)        (GPIO->GPIO_OE1 = en)
#define SENSOR_RST_IO(rst)			(GPIO->GPIO_O1  = rst)  
//------------------------------------------------------------------------------
//! LCD GPIO
//------------------------------------------------------------------------------
//! Audio
//! APP Audio I2S mode define
#define BSP_ADO_I2S_MODE_ENABLE		0
//! APP Audio aec/nr process by hw/sw define
#define APP_ADO_AEC_NR_TYPE			AEC_NR_SW
//! Speaker GPIO
#define SPEAKER_EN(en)				(GPIO->GPIO_O6 = en)
//------------------------------------------------------------------------------
//! Power GPIO
//------------------------------------------------------------------------------
//! LED GPIO
#define	PAIRING_LED_IO				(GPIO->GPIO_O9)//(PWM->PWM3_HIGH_CNT)
#define	PAIRING_LED_IO_ENABLE		(GPIO->GPIO_OE9)//(PWM->PWM_EN3)
#define POWER_LED_IO				(GPIO->GPIO_O5)
#define POWER_LED_IO_ENABLE			(GPIO->GPIO_OE5)
#define	SIGNAL_LED_IO				//(GPIO->GPIO_O8)
#define	SIGNAL_LED_IO_ENABLE		//(GPIO->GPIO_OE8)
//------------------------------------------------------------------------------
//! BSP RTC Time Select
#define BSP_RTC_TIMER_SEL			RTC_TIMER_INTERNAL
//------------------------------------------------------------------------------
//! SD dectect pin enable
#define BSP_SD_DEC_PIN_CUST_EN		0
//------------------------------------------------------------------------------
//! SF Write Protect use GPIO
#define SF_WP_GPIN					14		//!< 4, >=14 is no wp pin
#endif	//! End of #ifndef _BSP_CONFIG_H_

