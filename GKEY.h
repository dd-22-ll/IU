/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		GKEY.h
	\brief		Gpio Key Scan Header File
	\author		Penghui Xue
	\version	1.0
	\date		2019/07/08
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _GKEY_H_
#define _GKEY_H_
//------------------------------------------------------------------------------
#include <stdint.h>
#include "KEY.h"

#define GKEY_DOWN_EVENT					0								//!< Gpio key press down event
#define GKEY_CNT_EVENT					1								//!< Gpio key continue press down event
#define GKEY_UP_EVENT					2								//!< Gpio key release up event

#define GKEY_DET_TIME					(20 / KEY_THREAD_PERIOD)		//!< 20ms
#define GKEY_DEB_TIME					(10 / KEY_THREAD_PERIOD)		//!< 10ms
#define GKEY_CNT_TIME					(100 / KEY_THREAD_PERIOD)		//!< 100ms

#define GKEY_MAX_NUM 21

typedef enum
{
	GKEY_ID0 = 0x10,
	GKEY_ID1,
	GKEY_ID2,
	GKEY_ID3,
	GKEY_ID4,
	GKEY_ID5,
	GKEY_ID6,
	GKEY_ID7,
	GKEY_ID8,
	GKEY_ID9,
	GKEY_ID10,
	GKEY_ID11,
	GKEY_ID12,
	GKEY_ID13,
	GKEY_ID14,
	GKEY_ID15,
	GKEY_ID16,
	GKEY_ID17,
	GKEY_ID18,
	GKEY_ID19,
	GKEY_ID20,
}GKEY_ID_t;

typedef struct
{
	uint8_t  ubGKEY_KeyScanState[GKEY_MAX_NUM];
	uint16_t uwGKEY_KeyScanTime[GKEY_MAX_NUM];
	uint16_t uwGKEY_KeyScanCnt[GKEY_MAX_NUM];
}GKEY_SCAN_t;

//------------------------------------------------------------------------------
#if GKEY_ENABLE
void GKEY_Init(void);
void GKEY_Thread(void);
void GKEY_SetDetPin(uint8_t ubGpioNum);
#else
#define GKEY_Init()			((void)0)
#define GKEY_Thread()		((void)0)
#define GKEY_SetDetPin()	(void(0))
#endif

#endif														// Define _GKEY_H_
