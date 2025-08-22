/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_SNCC72_TX.h
	\brief		BSP Config header file
	\author		Wales Wang
	\version	0.1
	\date		2020/04/07
	\copyright	Copyright(C) 2019 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_SNCC72_TX_H_
#define _BSP_SNCC72_TX_H_

//------------------------------------------------------------------------------
//! CAM Board
//------------------------------------------------------------------------------
#if defined(BSP_D_SNCC72_TX_V1)
//------------------------------------------------------------------------------
//! RF 
#ifdef RTC676x
#define RF_MODULE_OLD			    //V1.2/V1.2s/V2.0 Module
//#define RF_MODULE_NEW			    //V3.0 Module
#endif
#ifdef A7130
#define RF_CKO_GPIONUM				13
#define RF_TXSW_GPIONUM				5
#endif
#ifdef S2019A
#define sPRF_PWR_CTRL(ctrl)			{																				\
									GPIO->GPIO_O12 = ctrl;															\
									RTC_SetGPO_1(ctrl, ((!ctrl)?RTC_PullDownEnable:RTC_PullDownDisable));			\
									}
#define sPRF_PWR_RST				{												\
									sPRF_PWR_CTRL(0)								\
									osDelay(100);									\
									sPRF_PWR_CTRL(1)								\
									osDelay(50);									\
									}
#endif

// Sensor GPIO -------------------------------------
#define	RN6752_RESET_OUT_EN			GPIO->GPIO_OE1
#define	RN6752_RESET_OUT			GPIO->GPIO_O1
#define TP9950_RESET_OUT_EN			GPIO->GPIO_OE1
#define TP9950_RESET_OUT			GPIO->GPIO_O1
#define SENSOR_RESET_OUT_EN         GPIO->GPIO_OE1
#define SENSOR_RESET_OUT            GPIO->GPIO_O1
#define SENSOR_RST_IO_EN(en)        (GPIO->GPIO_OE14 = en)
#define SENSOR_RST_IO(rst)			(GPIO->GPIO_O14 = rst)
//------------------------------------------------------------------------------
//! LCD GPIO
//------------------------------------------------------------------------------
//! Audio
//! I2S Mode Define
#define BSP_ADO_I2S_MODE_ENABLE		0			
//! AEC / NR Process by HW / SW Define
#define APP_ADO_AEC_NR_TYPE			AEC_NR_SW									
//! Speaker GPIO
#define SPEAKER_EN(en)              (GPIO->GPIO_O17 = en)
//------------------------------------------------------------------------------
//! Power GPIO
//------------------------------------------------------------------------------
//! LED GPIO
#define	PAIRING_LED_IO				(GPIO->GPIO_O2)
#define	PAIRING_LED_IO_ENABLE		(GPIO->GPIO_OE2)
#define POWER_LED_IO				(GPIO->GPIO_O1)
#define POWER_LED_IO_ENABLE			(GPIO->GPIO_OE1)
#define	GET_SIGNAL_LED_IO			(GPIO->GPIO_O3)									
#define	SIGNAL_LED_IO			    (GPIO->GPIO_O3)
#define	SIGNAL_LED_IO_ENABLE		(GPIO->GPIO_OE3)
//! KEY
#define AKEY_DISABLE
//! Auto Pair
#define AUTO_PAIR_PIN               0   //((GPIO->GPIO_I16)?0:1)
//------------------------------------------------------------------------------
//! BSP RTC Time Select
#define BSP_RTC_TIMER_SEL  			RTC_TIMER_NULL
//------------------------------------------------------------------------------
//! SD Card
#define BSP_SD_CARD					0
//! SD Dectect PIN Enable
#define BSP_SD_DEC_PIN_CUST_EN		0
//------------------------------------------------------------------------------
//! SF Write Protect use GPIO
#define SF_WP_GPIN					14		//!< 0~13, >=14 is no wp pin
#endif	//!< End of #ifdef BSP_D_SNCC72_TX_V1
#endif	//!< End of #ifndef _BSP_SNCC72_TX_H_

