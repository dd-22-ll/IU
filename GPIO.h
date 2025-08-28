/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		GPIO_INTR.c
	\brief		GPIO intrrupt function
	\author		Chinwei hsu
	\version	0.1
	\date		2019/03/04
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _GPIO_INTR_H_
#define _GPIO_INTR_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
#include "INTC.h"

//------------------------------------------------------------------------------
#pragma pack(push)
#pragma pack(1)
//------------------------------------------------------------------------------
typedef enum
{
	GPIO_INTR_SEL_0 = 0,
	GPIO_INTR_SEL_1,
	GPIO_INTR_SEL_2,
	GPIO_INTR_SEL_3,
	GPIO_INTR_SEL_4,
	GPIO_INTR_SEL_5,
	GPIO_INTR_SEL_6,
	GPIO_INTR_SEL_7,
	GPIO_INTR_SEL_8,
	GPIO_INTR_SEL_9,
	GPIO_INTR_SEL_10,
	GPIO_INTR_SEL_11,
	GPIO_INTR_SEL_12,
	GPIO_INTR_SEL_13,
	GPIO_INTR_SEL_14,
	GPIO_INTR_SEL_15,
	GPIO_INTR_SEL_16,
	GPIO_INTR_SEL_17,
	GPIO_INTR_SEL_18,
	GPIO_INTR_SEL_19,
	GPIO_INTR_SEL_20,
}GPIO_INTR_SEL_t;
//------------------------------------------------------------------------------
typedef enum
{
	GPIO_INTR_EDGE_TRG = 0,
	GPIO_INTR_LEVEL_TRG
}GPIO_INTR_TRG_MODE_t;
//------------------------------------------------------------------------------
typedef enum
{
	GPIO_INTR_RISING_EDGE = 0,
	GPIO_INTR_FALLING_EDGE,
	GPIO_INTR_HIGH_LEV,
	GPIO_INTR_LOW_LEV
}GPIO_INTR_TRG_LEV_t;
//------------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------------
uint16_t uwGPIO_GetVersion(void);
void GPIO_Intr_Setup(GPIO_INTR_SEL_t SelNum, GPIO_INTR_TRG_MODE_t TrigMode, GPIO_INTR_TRG_LEV_t TrigLev, INTC_FiqHandler GpioIsrHandler);
#endif
