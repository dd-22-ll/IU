/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI.h
	\brief		User Interface Header file (for High Speed Mode)
	\author		Hanyi Chiu
	\version	0.8
	\date		2017/11/30
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _UI_HS_H_
#define _UI_HS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "KEY.h"
#include "APP_CFG.h"
#include "PROFILE_API.h"

#define 	UI_Q_SIZE				30
#define 	UI_TASK_PERIOD			200 					//! Unit: ms
#define		UI_ENABLE				TRUE
#define		UI_DISABLE				FALSE

typedef void (*pvKeyEventFunc)(void);

typedef enum
{
	rUI_FAIL,
	rUI_SUCCESS,
}UI_Result_t;

typedef enum
{
	CAM1,
	CAM2,
	CAM3,
	CAM4,
	CAM_2T = 2,
	CAM_4T = 4,
	NO_CAM = 0xF,
}UI_CamNum_t;

typedef enum
{
	PKEY_EVENT,
	AKEY_EVENT,
	GKEY_EVENT,
	SCANMODE_EVENT,
	FWUPG_EVENT,
	EVENT_NONE,
}UI_EventType_t;

typedef enum
{
	SINGLE_VIEW = 1,
	SCAN_VIEW,	
	DUAL_VIEW,
	QUAD_VIEW,
	TRIPLE_2L1R_VIEW,
	TRIPLE_1L2R_VIEW,
	TRIPLE_2T1B_VIEW,
	TRIPLE_1T2B_VIEW,
	TRIPLE_3COL_VIEW,
}UI_CamViewType_t;
#define IS_UI_DISP3T_VIEW(view)		((((TRIPLE_2L1R_VIEW == view) || (TRIPLE_1L2R_VIEW == view)) ||		\
									  ((TRIPLE_2T1B_VIEW == view) || (TRIPLE_1T2B_VIEW == view)) ||		\
                                       (TRIPLE_3COL_VIEW == view))?1:0)

typedef enum
{
	//! Qual View
	DISP_UPPER_LEFT = 0,
	DISP_UPPER_RIGHT,
	DISP_LOWER_LEFT,
	DISP_LOWER_RIGHT,
	//! Dual View
	DISP_LEFT,
	DISP_RIGHT,
	//! Triple View
	DISP_3T_BOTTOM,
	DISP_3T_TOP,
	DISP_3T_3CLEFT,
	DISP_3T_3CMID,
	DISP_3T_3CRIGHT,
	//! Single View
	DISP_1T,
}UI_DisplayLocation_t;

typedef enum
{
	PS_VOX_MODE,
	PS_ADOONLY_MODE,
	PS_WOR_MODE,	
	PS_ECO_MODE,
	POWER_NORMAL_MODE = 8,
}UI_PowerSaveMode_t;

typedef enum
{
	VOL_LVL0,
	VOL_LVL1,
	VOL_LVL2,
	VOL_LVL3,
	VOL_LVL4,
	VOL_LVL5,
}UI_VolumeLvl_t;

typedef enum
{
#define OSDLOGOPOOL(idx,addr)  OSDLOGO_##idx,
#include "OSDLogo_Table.h"
	OSDLOGO_MAX
}UI_OSDLogoPool_t;

typedef enum
{
#define OSD1IMGPOOL(idx,addr,posx,posy)  OSD1IMG_##idx,
#include "OSD1Image_Table.h"
	OSD1IMG_MAX
}UI_OSD1ImagePool_t;

typedef enum
{
#define OSD2IMGPOOL(idx,addr,posx,posy)  OSD2IMG_##idx,
#include "OSD2Image_Table.h"
	OSD2IMG_MAX
}UI_OSD2ImagePool_t;

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
typedef struct
{
	UI_EventType_t 	tEventType;
	void 	    	*pvEvent;
}UI_Event_t;

typedef struct
{
	uint8_t ubKeyID;
	uint8_t uwKeyCnt;					//!< Periodic: 100ms
	pvKeyEventFunc KeyEventFuncPtr;
	void (*pvKeyTone)(void);
}UI_KeyEventMap_t;
#pragma pack(pop)

//------------------------------------------------------------------------------
void UI_Init(osMessageQId *pvMsgQId);
uint32_t ulUI_BufSetup(uint32_t ulBUF_StartAddr);
void UI_PlugIn(void);
//------------------------------------------------------------------------------

void UI_DrawString(const char *str, uint16_t startX, uint16_t startY);
void UI_DrawReverseString23(const char *str, uint16_t startX, uint16_t startY);
void UI_DrawString23(const char *str, uint16_t startX, uint16_t startY);
void UI_Drawbox(void);
void UI_Drawhalfbox(void);
void UI_DrawWhiteSquare(uint16_t x, uint16_t y);
void UI_DrawHalfWhiteSquare(uint16_t x, uint16_t y);
void MenuBackground(void);
//------------------------------------------------------------------------------
void MoveboxDown(void);
void MoveboxUp(void);
void EnterKeyHandler(void);
void MenuExitHandler(void);
//------------------------------------------------------------------------------
void UI_RefreshScreen(void);
void UI_ShowMenuKey(void);
void UI_ShowMenu(void);
void UI_ShowKeyLock(void);
void UI_ShowZoom(void);
void UI_ShowLanguage(void);
void UI_ShowSetting(void);
void UI_ShowPairUnits(void);
void UI_Info(void);
void UI_ShowSleepTimer(void);

//------------------------------------------------------------------------------
osMessageQId *pUI_GetEventQueueHandle(void);
void UI_StopUpdateThread(void);
void UI_StartUpdateThread(void);
void UI_SendMessageToAPP(void *pvMessage);
//------------------------------------------------------------------------------
void UI_OnInitDialog(void);
void UI_StateReset(void);
void UI_UpdateFwUpgStatus(void *ptUpgStsReport);
void UI_UpdateAppStatus(void *ptAppStsReport);
void UI_UpdateStatus(uint16_t *pThreadCnt);
void UI_EventHandles(UI_Event_t *ptEventPtr);
APP_EventMsg_t *tUI_ViewTypeSetup(UI_CamViewType_t tViewType);
void UI_FrameTRXFinish(uint8_t ubFrmRpt);
void UI_SwitchViewTypeFg(uint8_t ubFg);
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
void UI_RemotePlayStopTwc(void);
void UI_RemotePlayTrigger(uint8_t ubEn);
uint8_t UI_ChkRemotePlayTrigger(void);
#endif
//------------------------------------------------------------------------------

#endif

