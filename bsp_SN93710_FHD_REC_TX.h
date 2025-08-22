/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_config_bu_sn937X0.h
	\brief		BSP Config header file
	\author		Wales Wang
	\version	0.2
	\date		2020/06/08
	\copyright	Copyright(C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_CONFIG_BU_SN937X0_H_
#define _BSP_CONFIG_BU_SN937X0_H_
//======================================================================================================================
// CAM Board Configuration
//------------------------------------------------------------------------------
#ifdef A7130
//! RF 
#define RF_CKO_GPIONUM				0
#define RF_TXSW_GPIONUM				13
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
#define SENSOR_RESET_OUT_EN         (GPIO->GPIO_OE14)
#define SENSOR_RESET_OUT            (GPIO->GPIO_O14)
#define SENSOR_RST_IO_EN(en)        (GPIO->GPIO_OE14 = en)
#define SENSOR_RST_IO(rst)			(GPIO->GPIO_O14 = rst)  
//------------------------------------------------------------------------------
//! LCD GPIO
//------------------------------------------------------------------------------
//! Audio
//! APP Audio I2S mode define
#define BSP_ADO_I2S_MODE_ENABLE		0
//! APP Audio aec/nr process by hw/sw define
#define APP_ADO_AEC_NR_TYPE			AEC_NR_SW
//! Speaker GPIO
#define SPEAKER_EN(en)				(GPIO->GPIO_O4 = en)
//------------------------------------------------------------------------------
//! Power GPIO
//------------------------------------------------------------------------------
//! LED GPIO
#define	PAIRING_LED_IO				(GPIO->GPIO_O2)
#define	PAIRING_LED_IO_ENABLE		(GPIO->GPIO_OE2)
#define POWER_LED_IO				(GPIO->GPIO_O1)
#define POWER_LED_IO_ENABLE			(GPIO->GPIO_OE1)
#define	SIGNAL_LED_IO				(GPIO->GPIO_O3)
#define	SIGNAL_LED_IO_ENABLE		(GPIO->GPIO_OE3)
//------------------------------------------------------------------------------
//! BSP RTC Time Select
#define BSP_RTC_TIMER_SEL			RTC_TIMER_INTERNAL
//------------------------------------------------------------------------------
//! SD dectect pin enable
#define BSP_SD_DEC_PIN_CUST_EN		0
//------------------------------------------------------------------------------
//! SF Write Protect use GPIO
#define SF_WP_GPIN					14		//!< 0~13, >=14 is no wp pin
#endif	//! End of #ifndef _BSP_CONFIG_H_

