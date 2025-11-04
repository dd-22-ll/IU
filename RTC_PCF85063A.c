/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		RTC_PCF85063A.c
	\brief		Real time clock control function
	\author		Chinwei hsu
	\version	0.5
	\date		2019/12/9
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#include "RTC_PCF85063A.h"
#if (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
#include <stdio.h>

#define RTC_PCF85063A_MAJORVER	0
#define RTC_PCF85063A_MINORVER	4

#ifdef RTOS
osMutexId RTC_PCF85063A_Mutex;
osMutexDef(RTC_PCF85063A_Mutex);
#endif

I2C1_Type *pI2C2_RTC_PCF85063A = NULL;
RTC_PCF85063A_Hour_Mode_t RTC_PCF85063A_HourMode;
INTC_IrqHandler RTC_PCF85063A_IrqHandler = NULL;

//------------------------------------------------------------------------------
uint16_t uwRTC_PCF85063A_GetVersion(void)
{
    return ((RTC_PCF85063A_MAJORVER << 8) + RTC_PCF85063A_MINORVER);
}
//------------------------------------------------------------------------------
I2C1_Type *RTC_PCF85063A_I2C_Init(void)
{
	return pI2C_MasterInit (RTC_PCF85063A_I2C_INTERFACE, I2C_SCL_400K);
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_I2C_Recovery(void)
{
	uint32_t ulTo = 1000000;

#if defined(BSP_DVR_SDK)
	GLB->PADIO37 = 7;
	PWM->PWM5_RATE = 1;	
	PWM->PWM5_PERIOD   = 240;
	PWM->PWM5_HIGH_CNT = 120;
	PWM->PWM_EN5 = 1;
#elif defined(BUC_CU)
	GLB->PADIO13 = 7;
	PWM->PWM13_RATE = 1;	
	PWM->PWM13_PERIOD   = 240;
	PWM->PWM13_HIGH_CNT = 120;
	PWM->PWM_EN13 = 1;
#else
	printf("RTC_PCF85063A_I2C fail -> Please setting a PWM pin properly\n");
#endif
	
	while(1)
	{
		--ulTo;
		if (!ulTo)
			break;
	}
	
#if defined(BSP_DVR_SDK)
	GLB->PADIO37 = 2;
#elif defined(BUC_CU)
	GLB->PADIO13 = 4;
#endif
	
	pI2C2_RTC_PCF85063A = RTC_PCF85063A_I2C_Init();
	
	printf("RTC_PCF85063A_I2C fail -> Recovery!\n");
}
//------------------------------------------------------------------------------
uint8_t ubRTC_PCF85063A_I2C_Write(uint8_t *SrcAddr, uint32_t ulSize)
{
#if 0
	uint8_t ubFlag;
	ubFlag = bI2C_MasterProcess (pI2C2_RTC_PCF85063A, 0x51, SrcAddr, ulSize, NULL, 0);
	return ubFlag;
#else
	while(1)
	{
		if(bI2C_MasterProcess (pI2C2_RTC_PCF85063A, 0x51, SrcAddr, ulSize, NULL, 0)==1)
			break;
		else
			RTC_PCF85063A_I2C_Recovery();	
	}
	return 1;
#endif	
}
//------------------------------------------------------------------------------
uint8_t ubRTC_PCF85063A_I2C_Read(uint8_t ubRegAddr, uint8_t *RdAddr, uint32_t ulRdSize)
{
#if 0
	uint8_t ubRegBuf[1] = {ubRegAddr};
	return bI2C_MasterProcess (pI2C2_RTC_PCF85063A, 0x51, ubRegBuf, 1, RdAddr, ulRdSize);
#else
	uint8_t ubRegBuf[1] = {ubRegAddr};
	while(1)
	{
		if(bI2C_MasterProcess (pI2C2_RTC_PCF85063A, 0x51, ubRegBuf, 1, RdAddr, ulRdSize)==1)
			break;
		else
			RTC_PCF85063A_I2C_Recovery();	
	}
	return 1;
#endif
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_Isr(void)
{	
	RTC_PCF85063A_ClrAlarmFlag();
	if(RTC_PCF85063A_IrqHandler!=NULL)
		RTC_PCF85063A_IrqHandler();
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_Init(RTC_PCF85063A_Hour_Mode_t HourMode)
{
	uint8_t pWrtBuf[2];
	uint8_t pRdBuf[1];
	
#ifdef RTOS
	if(RTC_PCF85063A_Mutex == NULL)
	{
		RTC_PCF85063A_Mutex = osMutexCreate(osMutex(RTC_PCF85063A_Mutex));
		if(RTC_PCF85063A_Mutex == NULL)
			printd(DBG_ErrorLvl, "RTC PCF85063A Init: create mutex fail!\n");
	}
#endif
	
	// I2C2 init
	if(pI2C2_RTC_PCF85063A==NULL)
	{
		pI2C2_RTC_PCF85063A = RTC_PCF85063A_I2C_Init();
	}
	
	//disable CKOUT
	if(ubRTC_PCF85063A_I2C_Read(0x1, pRdBuf, 1)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A Init -> read CKOUT fail!\n");
	pRdBuf[0] &= 0xf8;
	pRdBuf[0] |= 0x7;
	pWrtBuf[0] = 0x1;
	pWrtBuf[1] = pRdBuf[0];
	if(ubRTC_PCF85063A_I2C_Write(pWrtBuf, sizeof(pWrtBuf))==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A Init -> write CKOUT fail!\n");
	
	// 12/24 hour mode
	RTC_PCF85063A_HourMode = HourMode;
	if(RTC_PCF85063A_HourMode == RTC_PCF85063A_12hr)
	{
		pWrtBuf[0] = 0x0;
		pWrtBuf[1] = 0x2;
		if(ubRTC_PCF85063A_I2C_Write(pWrtBuf, 2)==0)
			printd(DBG_ErrorLvl, "RTC PCF85063A Init fail!\n");
	}
	
#ifdef BSP_CARCAM_CU_HD_LCDPORTRAIT_DEMO
	GPIO_Intr_Setup(GPIO_INTR_SEL_1, GPIO_INTR_EDGE_TRG, GPIO_INTR_FALLING_EDGE, RTC_PCF85063A_Isr);
#else
	GPIO_Intr_Setup(GPIO_INTR_SEL_10, GPIO_INTR_EDGE_TRG, GPIO_INTR_FALLING_EDGE, RTC_PCF85063A_Isr);
#endif
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_CalendarReset(void)
{
	uint8_t pWrtBuf[2];
	
	// software reset
	pWrtBuf[0] = 0x0;
	pWrtBuf[1] = 0x58;
	if(ubRTC_PCF85063A_I2C_Write(pWrtBuf, 2)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A Init fail!\n");
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_GetCalendar(RTC_PCF85063A_Calendar_t *Calendar)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif
	
	uint8_t pRdBuf[7];
	
	if(ubRTC_PCF85063A_I2C_Read(0x4, pRdBuf, 7)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A get calendar -> fail!\n");	
	
	Calendar->ubSec = (((pRdBuf[0]&0x70)>>4)*10) + (pRdBuf[0]&0xF);
	Calendar->ubMin = (((pRdBuf[1]&0x70)>>4)*10) + (pRdBuf[1]&0xF);
	if(RTC_PCF85063A_HourMode == RTC_PCF85063A_24hr)
	{
		Calendar->ubHour = (((pRdBuf[2]&0x30)>>4)*10) + (pRdBuf[2]&0xF);
	}
	else if(RTC_PCF85063A_HourMode == RTC_PCF85063A_12hr)
	{
		Calendar->ubAMPM = (pRdBuf[2]&0x20)>>5;
		Calendar->ubHour = (((pRdBuf[2]&0x10)>>4)*10) + (pRdBuf[2]&0xF);
	}
	Calendar->ubDate    = (((pRdBuf[3]&0x30)>>4)*10) + (pRdBuf[3]&0xF);
	Calendar->ubWeekday = pRdBuf[4]&0x7;
	Calendar->ubMonth   = (((pRdBuf[5]&0x10)>>4)*10) + (pRdBuf[5]&0xF);
	Calendar->uwYear    = (((pRdBuf[6]&0xF0)>>4)*10) + (pRdBuf[6]&0xF) + 2000;
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
	return;
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_SetCalendar(const RTC_PCF85063A_Calendar_t *Calendar)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif
	
	uint8_t pRdBuf[7];
	uint8_t pWrtBuf[8];
	uint16_t uwYear;
	uint8_t  ubMonth;
	uint8_t  ubWeekday;
	uint8_t  ubDate;
	uint8_t  ubAMPM;
	uint8_t  ubHour;
	uint8_t  ubMin;
	uint8_t  ubSec;
	
	if(ubRTC_PCF85063A_I2C_Read(0x4, pRdBuf, 7)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A set calendar -> get calendar fail!\n");
	
	pRdBuf[0] &= 0x80;	//seconds
	pRdBuf[1] &= 0x80;	//minutes
	pRdBuf[2] &= 0xC0;	//hours
	pRdBuf[3] &= 0xC0;	//days
	pRdBuf[4] &= 0xF8;	//weekdays
	pRdBuf[5] &= 0xE0;	//months
	pRdBuf[6] = 0x0;	//years

	uwYear    = Calendar->uwYear - 2000;
	ubMonth   = Calendar->ubMonth;
	ubWeekday = Calendar->ubWeekday;
	ubDate    = Calendar->ubDate;
	ubAMPM    = Calendar->ubAMPM;
	ubHour    = Calendar->ubHour;
	ubMin     = Calendar->ubMin;
	ubSec     = Calendar->ubSec;
	
	//set data
	pWrtBuf[0] = 0x4;
	pWrtBuf[1] = (pRdBuf[0]) | (((ubSec/10)&0x7)<<4) | ((ubSec%10)&0xF);
	pWrtBuf[2] = (pRdBuf[1]) | (((ubMin/10)&0x7)<<4) | ((ubMin%10)&0xF);
	if(RTC_PCF85063A_HourMode == RTC_PCF85063A_24hr)
	{
		pWrtBuf[3] = (pRdBuf[2]) | (((ubHour/10)&0x3)<<4) | ((ubHour%10)&0xF);
	}
	else if(RTC_PCF85063A_HourMode == RTC_PCF85063A_12hr)
	{
		pWrtBuf[3] = (pRdBuf[2]) | ((ubAMPM&0x1)<<5) | (((ubHour/10)&0x1)<<4) | ((ubHour%10)&0xF);
	}
	pWrtBuf[4] = (pRdBuf[3]) | (((ubDate/10)&0x3)<<4) | ((ubDate%10)&0xF);
	pWrtBuf[5] = (pRdBuf[4]) | (ubWeekday&0x7);
	pWrtBuf[6] = (pRdBuf[5]) | (((ubMonth/10)&0x1)<<4) | ((ubMonth%10)&0xF);
	pWrtBuf[7] = (pRdBuf[6]) | (((uwYear/10)&0xF)<<4) | ((uwYear%10)&0xF);
	
	if(ubRTC_PCF85063A_I2C_Write(pWrtBuf, sizeof(pWrtBuf))==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A set calendar -> set calendar fail!\n");
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
}
//------------------------------------------------------------------------------
uint8_t ubRTC_PCF85063A_ReadRAM(void)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif
	
	uint8_t pRdBuf[1];
	
	if(ubRTC_PCF85063A_I2C_Read(0x3, pRdBuf, 1)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A read RAM fail!\n");
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
	
	return pRdBuf[0];
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_WriteRAM(uint8_t data)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif

	uint8_t pWrtBuf[2];
	
	pWrtBuf[0] = 0x3;
	pWrtBuf[1] = data;
	if(ubRTC_PCF85063A_I2C_Write(pWrtBuf, 2)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A write RAM fail!\n");
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_GetAlarmCalendar(RTC_PCF85063A_AlarmCalendar_t *Calendar)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif
	
	uint8_t pRdBuf[5];
	
	if(ubRTC_PCF85063A_I2C_Read(0xB, pRdBuf, 5)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A get alarm calendar fail!\n");
	
	Calendar->ubSec = (((pRdBuf[0]&0x70)>>4)*10) + (pRdBuf[0]&0xF);
	Calendar->ubMin = (((pRdBuf[1]&0x70)>>4)*10) + (pRdBuf[1]&0xF);
	if(RTC_PCF85063A_HourMode == RTC_PCF85063A_24hr)
	{
		Calendar->ubHour = (((pRdBuf[2]&0x30)>>4)*10) + (pRdBuf[2]&0xF);
	}
	else if(RTC_PCF85063A_HourMode == RTC_PCF85063A_12hr)
	{
		Calendar->ubAMPM = (pRdBuf[2]&0x20)>>5;
		Calendar->ubHour = (((pRdBuf[2]&0x10)>>4)*10) + (pRdBuf[2]&0xF);
	}
	Calendar->ubDate    = (((pRdBuf[3]&0x30)>>4)*10) + (pRdBuf[3]&0xF);
	Calendar->ubWeekday = pRdBuf[4]&0x7;
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
	return;
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_SetAlarmCalendar(const RTC_PCF85063A_AlarmCalendar_t *Calendar, INTC_IrqHandler IsrHandler)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif
	
	uint8_t pWrtBuf[6];
	uint8_t  ubWeekday;
	uint8_t  ubDate;
	uint8_t  ubAMPM;
	uint8_t  ubHour;
	uint8_t  ubMin;
	uint8_t  ubSec;
	
	RTC_PCF85063A_IrqHandler = IsrHandler;
	
	ubWeekday = Calendar->ubWeekday;
	ubDate    = Calendar->ubDate;
	ubAMPM    = Calendar->ubAMPM;
	ubHour    = Calendar->ubHour;
	ubMin     = Calendar->ubMin;
	ubSec     = Calendar->ubSec;

	//set data
	pWrtBuf[0] = 0xB;
	pWrtBuf[1] = (((ubSec/10)&0x7)<<4) | ((ubSec%10)&0xF);
	pWrtBuf[2] = (((ubMin/10)&0x7)<<4) | ((ubMin%10)&0xF);
	if(RTC_PCF85063A_HourMode == RTC_PCF85063A_24hr)
	{
		pWrtBuf[3] = (((ubHour/10)&0x3)<<4) | ((ubHour%10)&0xF);
	}
	else if(RTC_PCF85063A_HourMode == RTC_PCF85063A_12hr)
	{
		pWrtBuf[3] = ((ubAMPM&0x1)<<5) | (((ubHour/10)&0x1)<<4) | ((ubHour%10)&0xF);
	}
	pWrtBuf[4] = (((ubDate/10)&0x3)<<4) | ((ubDate%10)&0xF);
	pWrtBuf[5] = (ubWeekday&0x7);
	
	if(ubRTC_PCF85063A_I2C_Write(pWrtBuf, sizeof(pWrtBuf))==0)
	{
		RTC_PCF85063A_IrqHandler = NULL;
		printd(DBG_ErrorLvl, "RTC PCF85063A set alarm calendar fail!\n");
	}
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_EnableAlarm(void)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif
	
	uint8_t pRdBuf[1];
	uint8_t pWrtBuf[2];
	
	if(ubRTC_PCF85063A_I2C_Read(0x1, pRdBuf, 1)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A enable alarm fail-1!\n");
	
	pWrtBuf[0] = 0x1;
	pWrtBuf[1] = 0x80 | (pRdBuf[0]&0xBF);
	
	if(ubRTC_PCF85063A_I2C_Write(pWrtBuf, 2)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A enable alarm fail-2!\n");
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_DisableAlarm(void)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif
	
	uint8_t pRdBuf[1];
	uint8_t pWrtBuf[2];
	
	if(ubRTC_PCF85063A_I2C_Read(0x1, pRdBuf, 1)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A disable alarm fail-1!\n");
	
	pWrtBuf[0] = 0x1;
	pWrtBuf[1] = pRdBuf[0]&0x3F;
	if(ubRTC_PCF85063A_I2C_Write(pWrtBuf, 2)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A disable alarm fail-2!\n");
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
}
//------------------------------------------------------------------------------
uint8_t ubRTC_PCF85063A_GetAlarmFlag(void)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif
	
	uint8_t pRdBuf[1];
	uint8_t ubTemp;
	
	if(ubRTC_PCF85063A_I2C_Read(0x1, pRdBuf, 1)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A disable alarm fail-1!\n");

	ubTemp = (pRdBuf[0]&0x40)>>6;
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
	
	return ubTemp;
}
//------------------------------------------------------------------------------
void RTC_PCF85063A_ClrAlarmFlag(void)
{
#ifdef RTOS
	osMutexWait(RTC_PCF85063A_Mutex, osWaitForever);
#endif
	
	uint8_t pRdBuf[1];
	uint8_t pWrtBuf[2];
	
	if(ubRTC_PCF85063A_I2C_Read(0x1, pRdBuf, 1)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A clear alarm fail-1!\n");
	
	pWrtBuf[0] = 0x1;
	pWrtBuf[1] = pRdBuf[0]&0xBF;
	if(ubRTC_PCF85063A_I2C_Write(pWrtBuf, 2)==0)
		printd(DBG_ErrorLvl, "RTC PCF85063A clear alarm fail-2!\n");
	
#ifdef RTOS
	osMutexRelease(RTC_PCF85063A_Mutex);
#endif
}

#endif
