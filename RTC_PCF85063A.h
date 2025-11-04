/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		RTC_PCF85063A.h
	\brief		Real time clock control function
	\author		Chinwei hsu
	\version	0.5
	\date		2019/12/9
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _RTC_PCF85063A_H_
#define _RTC_PCF85063A_H_
//------------------------------------------------------------------------------

#include "APP_CFG.h"
#if (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)

#include "_510PF.h"
#include "INTC.h"
#include "GPIO.h"
#include "I2C.h"

#ifdef BSP_CARCAM_CU_HD_LCDPORTRAIT_DEMO
	#define RTC_PCF85063A_I2C_INTERFACE		I2C_1
#else
	#define RTC_PCF85063A_I2C_INTERFACE		I2C_2
#endif
//------------------------------------------------------------------------------
#pragma pack(push)
#pragma pack(1)
//------------------------------------------------------------------------------
typedef enum
{
	RTC_PCF85063A_12hr,
	RTC_PCF85063A_24hr
}RTC_PCF85063A_Hour_Mode_t;
//------------------------------------------------------------------------------
typedef struct
{
	uint16_t uwYear;
	uint8_t  ubMonth;
	uint8_t  ubWeekday;	//	0:Sunday, 1:Monday, 2:Tuesday, 3:Wednesday, 4:Thursday, 5:Friday, 6:Saturday
	uint8_t  ubDate;
	uint8_t  ubAMPM;	// 0:AM, 1:PM
	uint8_t  ubHour;
	uint8_t  ubMin;
	uint8_t  ubSec;
}RTC_PCF85063A_Calendar_t;
//------------------------------------------------------------------------------
typedef struct
{
	uint8_t  ubWeekday;	//	0:Sunday, 1:Monday, 2:Tuesday, 3:Wednesday, 4:Thursday, 5:Friday, 6:Saturday
	uint8_t  ubDate;
	uint8_t  ubAMPM;	// 0:AM, 1:PM
	uint8_t  ubHour;
	uint8_t  ubMin;
	uint8_t  ubSec;
}RTC_PCF85063A_AlarmCalendar_t;
//------------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------------
void RTC_PCF85063A_Init(RTC_PCF85063A_Hour_Mode_t HourMode);
void RTC_PCF85063A_CalendarReset(void);
void RTC_PCF85063A_GetCalendar(RTC_PCF85063A_Calendar_t *Calendar);
void RTC_PCF85063A_SetCalendar(const RTC_PCF85063A_Calendar_t *Calendar);
uint8_t ubRTC_PCF85063A_ReadRAM(void);
void RTC_PCF85063A_WriteRAM(uint8_t data);
void RTC_PCF85063A_GetAlarmCalendar(RTC_PCF85063A_AlarmCalendar_t *Calendar);
void RTC_PCF85063A_SetAlarmCalendar(const RTC_PCF85063A_AlarmCalendar_t *Calendar, INTC_IrqHandler IsrHandler);
void RTC_PCF85063A_EnableAlarm(void);
void RTC_PCF85063A_DisableAlarm(void);
uint8_t ubRTC_PCF85063A_GetAlarmFlag(void);
void RTC_PCF85063A_ClrAlarmFlag(void);
#endif

#endif
