/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_drivers.c
	\brief		VBM PU/BU Demo/EV driver
	\author		Hanyi Chiu
	\version	0.8
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#if defined VBM_PU || defined VBM_BU

#include <stdio.h>
#include "BSP.h"
#include "UART.h"
#include "WDT.h"
#include "TIMER.h"
#include "CQ_API.h"
#include "RTC_API.h"
#include "CIPHER_API.h"
#include "DMAC_API.h"
#include "SD_API.h"
#include "APBC.h"
#include "APP_CFG.h"
#include "KNL.h"
#include "GKEY.h"
#include "CLI.h"
#ifdef CFG_UART1_ENABLE
#include "UI_UART1.h"
#endif

void RETARGET_Init (UART1_Type *ptUart);
#ifdef BSP_SN93711_FHD_REC_RX_V4
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
    APBC_Init();

	//! LED
	GPIO->GPIO_OE2 	= 1;
	GPIO->GPIO_O2  	= 1;
	GPIO->GPIO_OE3 	= 1;
	GPIO->GPIO_O3 	= 0;
	GPIO->GPIO_OE13	= 1;
	GPIO->GPIO_OE0	= 1;

	//! BL Control
	PWM->PWM3_RATE  	= 127;
	PWM->PWM3_PERIOD 	= 0xC00;
	PWM->PWM3_HIGH_CNT 	= 0xA00;
	//! BL Enable
	GPIO->GPIO_OE11 = 1;
	GPIO->GPIO_O11  = 0;
	PWM->PWM_EN3    = 0;

	//! Speaker
	GPIO->GPIO_OE12 = 1;

	//! LCD POWER	
	GPIO->GPIO_OE10 = 1;
	GPIO->GPIO_O10  = 0;

	//! RTC GPIO1 -> RF POWER
	RTC_SetGPO_1(1, RTC_PullDownDisable);

	printd(DBG_CriticalLvl, "SONiX SN93701 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#if (defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6)|| defined(BSP_D_SN93703_TC358778_RX_V6))
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
  APBC_Init();

	//! LED
	//GPIO->GPIO_OE2 	= 1;
	//GPIO->GPIO_O2  	= 1;
	//GPIO->GPIO_OE3 	= 1;
	//GPIO->GPIO_O3 	= 0;
	//GPIO->GPIO_OE13	= 1;
	//GPIO->GPIO_OE0	= 1;

	//! BL Control
	//PWM->PWM2_RATE  	= 127;
	//PWM->PWM2_PERIOD 	= 0xC00;
	//PWM->PWM2_HIGH_CNT 	= 0xA00;
	//! BL Enable
	GPIO->GPIO_OE0 = 1;
	GPIO->GPIO_O0  = 0;
	//PWM->PWM_EN2    = 0;

	//! Speaker
	GPIO->GPIO_OE12 = 1;

	//! LCD POWER	
	GPIO->GPIO_OE2 = 1;
	GPIO->GPIO_O2  = 0;

	//! LCD Reset
	PWM->PWM2_RATE  	= 127;
	PWM->PWM2_PERIOD 	= 1;
	PWM->PWM2_HIGH_CNT = 1;
	PWM->PWM_EN2    	= 1;


	//! flash en
	PWM->PWM9_RATE  	= 127;
	PWM->PWM9_PERIOD 	= 1;
	PWM->PWM9_HIGH_CNT = 0;
	PWM->PWM_EN9    	= 0;


	//! LCD SDA
	PWM->PWM1_RATE  	= 127;
	PWM->PWM1_PERIOD 	= 1;
	PWM->PWM1_HIGH_CNT = 0;
	PWM->PWM_EN1    	= 1;


	//! RTC GPIO1 -> RF POWER
	RTC_SetGPO_1(1, RTC_PullDownDisable);
	
#if (defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))
	printd(DBG_CriticalLvl, "SONiX SN93701 High Speed Mode Start!\n");
#endif
#if (defined(BSP_D_SN93703_TC358778_RX_V6))
    printd(DBG_CriticalLvl, "SONiX SN93703 High Speed Mode Start!\n");
#endif
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMPU_EV
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
    APBC_Init();
	SDIO_Init();

	//! BL	
	GPIO->GPIO_OE10 = 1;
	GPIO->GPIO_O10  = 1;

	printd(DBG_CriticalLvl, "SONiX SN9370X High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_SN93710_FHD_REC_TX_V4_1
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
#ifdef CFG_UART1_ENABLE
	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_1, UART1_rtoscli_recv);
#endif
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();

//	//! Reset Sensor
//    GPIO->GPIO_OE14 = 1;
//    GPIO->GPIO_O14  = 0;
    
	//! LED
	GPIO->GPIO_OE1 	= 1;
	GPIO->GPIO_O1	= 1;
	GPIO->GPIO_OE2 	= 1;
	GPIO->GPIO_O2	= 0;
	GPIO->GPIO_OE3 	= 1;
	GPIO->GPIO_O3	= 0;

	//! Speaker
	GPIO->GPIO_OE4 	= 1;

	//! RTC GPIO1 -> RF POWER
	RTC_SetGPO_1(1, RTC_PullDownDisable);

	printd(DBG_CriticalLvl, "SONiX SN937X0 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93714_TX_V1
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_1, CLI_rtoscli_recv);
	RETARGET_Init(UART1);
	TIMER_Init();
	CQ_Init();

#if !APP_SD_FUNC_ENABLE
	//! LED
	SIGNAL_LED_IO 			= 0;	//! LED B
	SIGNAL_LED_IO_ENABLE	= 1;
#endif	
	POWER_LED_IO 			= 1;	//! LED R
	POWER_LED_IO_ENABLE		= 1;
	PAIRING_LED_IO			= 0;	//! LED G
	PAIRING_LED_IO_ENABLE	= 1;
	//! Speaker
	GPIO->GPIO_OE12 	= 1;
#if  (defined(S2019A)||defined(A7130))
	//! S2019 RF PWR CTRL
	PWM->PWM1_RATE  	= 127;
	PWM->PWM1_PERIOD 	= 1;
	PWM->PWM1_HIGH_CNT	= 1;
	PWM->PWM_EN1    	= 1;
#endif
	printd(DBG_CriticalLvl, "SONiX SN93714 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93716_TX_V1
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();

	POWER_LED_IO 			= 1;	//! LED R
	POWER_LED_IO_ENABLE		= 1;
	PAIRING_LED_IO			= 0;	//! LED B
	PAIRING_LED_IO_ENABLE	= 1;

	//! RF 5V Power
	PWM->PWM11_RATE  	= 127;
	PWM->PWM11_PERIOD 	= 1;
	PWM->PWM11_HIGH_CNT = 0;
	PWM->PWM_EN11    	= 1;
	//! Speaker
	GPIO->GPIO_OE6 = 1;
	printd(DBG_CriticalLvl, "SONiX SN93716 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC72_TX_V1
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
#ifdef CFG_UART1_ENABLE
	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_1, UART1_rtoscli_recv);
#endif
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
	APBC_Init();

    //! Reset RN6752
    GPIO->GPIO_OE1  = 1;
    GPIO->GPIO_O1   = 0;
	
	//! LED
	GPIO->GPIO_O20	= 1;
	GPIO->GPIO_OE20	= 1;
	
	//! GPIO16 PULL UP
	GLB->TRSTN_PDN  = 0;
	GPIO->GPIO_OE16	= 0;
	GPIO->GPIO_O16	= 0;
	
	//GPIO Key
	GLB->PADIO8 	= 0;
    GKEY_SetDetPin(8);
	GPIO->GPIO_OE8  = 0;	//Input Mode
	
	//speaker
	GLB->TCK_PDN 	= 1;
	GPIO->GPIO_OE17 = 1;	
	GPIO->GPIO_O17 	= 0;

	
	//! RF PWR CTRL
	GPIO->GPIO_O18	= 1;
	GPIO->GPIO_OE18	= 1;
	
	printd(DBG_CriticalLvl, "SONiX SNCC72 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93712_VBM_TX_V2
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();

	//! RF Power
	GPIO->GPIO_OE4 = 1;
	GPIO->GPIO_O4 = 1;

//	//! Reset Sensor
//    GPIO->GPIO_OE1 = 1;
//    GPIO->GPIO_O1  = 0;

	//! Speaker
	GPIO->GPIO_OE16	= 1;

	//! LED
	GPIO->GPIO_OE18	= 1;
	GPIO->GPIO_O18	= 1;
	GPIO->GPIO_OE19	= 1;
	GPIO->GPIO_O19	= 0;

	//! Pairing Key
	GKEY_SetDetPin(6);
	GPIO->GPIO_PULL_EN6 = 0;
	GPIO->GPIO_OE6 	= 0;

	printd(DBG_CriticalLvl, "SONiX SN93712 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMBU_EV
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);	
	TIMER_Init();
	CQ_Init();
	SDIO_Init();
	
	printd(DBG_CriticalLvl, "SONiX SN9370X High Speed Mode Start!\n");	
}
#endif
//------------------------------------------------------------------------------

#endif //! End #if defined VBM_PU || defined VBM_BU
