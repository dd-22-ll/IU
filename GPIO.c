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
#include <stdio.h>
#include "GPIO.h"

#define GPIO_MAJORVER	0
#define GPIO_MINORVER	1

INTC_FiqHandler GPIO_INTR_Handler[21] = {NULL};

uint8_t ubGPIO_IntrInitFlag = 0;

//------------------------------------------------------------------------------
uint16_t uwGPIO_GetVersion(void)
{
    return ((GPIO_MAJORVER << 8) + GPIO_MINORVER);
}
//------------------------------------------------------------------------------
void GPIO_Isr(void)
{
	uint8_t i;
	uint32_t ulEvent;
	
	for(i=0; i<21; i++)
	{
		ulEvent = GPIO->GPIO_INTR_FLAG;
		 if (ulEvent & (((uint32_t) 1) << i))
		 {
			if(GPIO_INTR_Handler[i] != NULL)
				GPIO_INTR_Handler[i]();
			SET_BIT(GPIO->CLR_GPIO_INTR, i);
		 }
	}
	
	INTC_IrqClear(INTC_GPIO_IRQ);
}
//------------------------------------------------------------------------------
void GPIO_Intr_Setup(GPIO_INTR_SEL_t SelNum, GPIO_INTR_TRG_MODE_t TrigMode, GPIO_INTR_TRG_LEV_t TrigLev, INTC_FiqHandler GpioIsrHandler)
{
	if(GpioIsrHandler == NULL)
		return;
	
	if(ubGPIO_IntrInitFlag==0)
	{
		ubGPIO_IntrInitFlag = 1;
		INTC_IrqSetup(INTC_GPIO_IRQ, GPIO_Isr);
		INTC_IrqClear(INTC_GPIO_IRQ);
		INTC_IrqEnable(INTC_GPIO_IRQ);
	}
	
	GPIO_INTR_Handler[SelNum] = GpioIsrHandler;
	
	CLR_BIT(GPIO->GPIO_INTR_EN, SelNum);
	CLR_BIT(GPIO->GPIO_INTR_MSK, SelNum);
	SET_BIT(GPIO->CLR_GPIO_INTR, SelNum);
	
	if(TrigMode==GPIO_INTR_EDGE_TRG)
	{
		CLR_BIT(GPIO->GPIO_TRG_MODE, SelNum);
		CLR_BIT(GPIO->GPIO_TRG_EDGE, SelNum);
		if(TrigLev==GPIO_INTR_RISING_EDGE)
		{
			CLR_BIT(GPIO->GPIO_TRG_LEVEL, SelNum);
		}
		else if(TrigLev==GPIO_INTR_FALLING_EDGE)
		{
			SET_BIT(GPIO->GPIO_TRG_LEVEL, SelNum);
		}
	}
	else if(TrigMode==GPIO_INTR_LEVEL_TRG)
	{
		SET_BIT(GPIO->GPIO_TRG_MODE, SelNum);
		CLR_BIT(GPIO->GPIO_TRG_EDGE, SelNum);
		if(TrigLev==GPIO_INTR_HIGH_LEV)
		{
			CLR_BIT(GPIO->GPIO_TRG_LEVEL, SelNum);
		}
		else if(TrigLev==GPIO_INTR_LOW_LEV)
		{
			SET_BIT(GPIO->GPIO_TRG_LEVEL, SelNum);
		}
	}
	
	SET_BIT(GPIO->GPIO_INTR_EN, SelNum);
}
