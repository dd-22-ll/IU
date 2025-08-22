/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		KEY.h
	\brief		KEY Header file
	\author		Hanyi Chiu
	\version	0.5
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _KEY_H_
#define _KEY_H_																   
//------------------------------------------------------------------------------
#include "_510PF.h"
#include "BSP.h"
//! KEY function
#if (defined(BSP_SN93711_FHD_REC_RX_V4)|| defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6)|| defined(BSP_D_SN93703_TC358778_RX_V6))
#define PKEY_ENABLE				1
#define AKEY_ENABLE				1
#define GKEY_ENABLE				0
#elif (defined(BSP_SN93710_FHD_REC_TX_V4_1) || defined(BSP_D_SN93714_TX_V1) || defined(BSP_D_SN93716_TX_V1))
#define PKEY_ENABLE				1
#define AKEY_ENABLE				1
#define GKEY_ENABLE				0
#elif (defined(BSP_D_SN93712_VBM_TX_V2)||defined(BSP_D_SNCC72_TX_V1))
#define PKEY_ENABLE				0
#define AKEY_ENABLE				0
#define GKEY_ENABLE				1
#endif

#include "PKEY.h"
#include "AKEY.h"
#include "GKEY.h"

#define KEY_THREAD_PERIOD		10 //! Unit: ms

typedef enum
{
	PKEY,
	AKEY,
	GKEY,
}KEY_Type_t;

typedef enum
{
	KEY_DOWN_ACT,
	KEY_CNT_ACT,
	KEY_UP_ACT
}KEY_Action_t;

typedef enum
{
	KEY_UNKNOW_STATE,
	KEY_DET_STATE,
	KEY_DEBONC_STATE,
	KEY_CNT_STATE
}KEY_ScanState_t;

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
typedef struct
{	
	uint8_t  ubKeyAction;
	uint8_t  ubKeyID;
	uint16_t uwKeyCnt;
}KEY_Event_t;
#pragma pack(pop)

//----------------------------------------------------------
void KEY_Init(osMessageQId *pEventQueueHandle);
void KEY_QueueSend(KEY_Type_t tKeyType, void *pvKeyEvent);
void KEY_Suspend(void);
void KEY_Resume(void);
//----------------------------------------------------------
#endif
