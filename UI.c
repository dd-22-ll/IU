/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification. 

	\file		UI.c
	\brief		User Interface (for High Speed Mode)
	\author		Hanyi Chiu
	\version	0.11
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include "UI.h"
#include "APP_HS.h"
#include "LCD.h"
#include "OSD.h"
#include "BUF.h"

#ifdef VBM_PU
#include "UI_VBMPU.h"
#endif
#ifdef VBM_BU
#include "UI_VBMBU.h"
#endif
osThreadId osUI_ThreadId;
osMessageQId UI_EventQueue;
osMessageQId *pAPP_MessageQH;
static void UI_Thread(void const *argument);
static void UI_EventThread(void const *argument);
//------------------------------------------------------------------------------
void UI_Init(osMessageQId *pvMsgQId)
{
	UI_StateReset();
	pAPP_MessageQH = pvMsgQId;
    osMessageQDef(UI_EventQueue, UI_Q_SIZE, UI_Event_t);
    UI_EventQueue = osMessageCreate(osMessageQ(UI_EventQueue), NULL);
	KEY_Init(&UI_EventQueue);
	osThreadDef(UI_EventThread, UI_EventThread, THREAD_PRIO_UIEVENT_HANDLER, 1, THREAD_STACK_UIEVENT_HANDLER);
	osThreadCreate(osThread(UI_EventThread), NULL);
	KNL_SetBbFrmMonitCbFunc(UI_FrameTRXFinish);
#if (OP_AP && APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    UI_FSInfoInstall();
    KNL_SetTxFSCbFunc(UI_TxFsCbFunc);
#endif    
#if (OP_STA && APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    KNL_SetRemoteFSCbFunc(UI_TXFS_UpdateToCU);
    KNL_SetRemotePlayCbFunc(UI_TXPLY_UpdateToCU);
#endif
}
//------------------------------------------------------------------------------
uint32_t ulUI_BufSetup(uint32_t ulBUF_StartAddr)
{
	uint32_t ulUI_BufSize = 0;

#ifdef VBM_PU
	ulUI_BufSize = ulOSD_CalBufSize(HD_WIDTH, HD_HEIGHT);
	OSD_SetOsdBufAddr(ulBUF_StartAddr);
	printd(DBG_Debug2Lvl, "OSD Buffer Addr 0x%X[0x%X]\n", ulBUF_StartAddr, ulUI_BufSize);
#endif

	return ulUI_BufSize;
}
//------------------------------------------------------------------------------
void UI_PlugIn(void)
{
#ifdef VBM_PU
	LCD_RESET(10);
	LCD_Init(LCD_LCD_PANEL);
	LCD_SetLcdBufAddr(ulBUF_GetBlkBufAddr(0, BUF_LCD_IP));
	LCDBL_ENABLE(UI_ENABLE);
	tOSD_Init(OSD_WEIGHT_8DIV8, uwLCD_GetLcdHoSize(), uwLCD_GetLcdVoSize(), 0, 0, OSD_SCALE_1X, OSD_SCALE_1X);
	UI_OnInitDialog();
#endif
	osThreadDef(UI_Thread, UI_Thread, THREAD_PRIO_UI_HANDLER, 1, THREAD_STACK_UI_HANDLER);
	osUI_ThreadId = osThreadCreate(osThread(UI_Thread), NULL);
}
//------------------------------------------------------------------------------
#include "RC.h"
extern RC_INFO tRC_Info[4];
#ifdef VBM_PU
extern UI_BUStatus_t tUI_CamStatus[CAM_4T];
extern uint8_t demo_menu;
extern UI_State_t tUI_State;
uint8_t ui_show_delay = 15;
uint8_t demo_engmode = 0;
void UI_TimerShow(void)
{
		if (ui_show_delay)
			ui_show_delay--;
	
		
			uint8_t current_fps = ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_1_MAIN);
			uint16_t uwAntOsdImg[5] = {OSD2IMG_N80_ANT0, OSD2IMG_N80_ANT1, OSD2IMG_N80_ANT2, OSD2IMG_N80_ANT3, OSD2IMG_N80_ANT4};
			uint16_t uwNumOsdImg[11] = {OSD2IMG_N80_NUM0, OSD2IMG_N80_NUM1, OSD2IMG_N80_NUM2, OSD2IMG_N80_NUM3, OSD2IMG_N80_NUM4,\
																	OSD2IMG_N80_NUM5, OSD2IMG_N80_NUM6, OSD2IMG_N80_NUM7, OSD2IMG_N80_NUM8, OSD2IMG_N80_NUM9, OSD2IMG_N80_NUMX};
			uint8_t ant_lvl = 0;
			uint8_t fps_num = 0;
																	
																	
			if (current_fps >= 9)
					ant_lvl = 4;			
			else if (current_fps >= 6)
					ant_lvl = 3;				
			else if (current_fps >= 3)
					ant_lvl = 2;				
			else if (current_fps >= 1)
					ant_lvl = 1;	
			else
					ant_lvl = 0;	
			
			OSD_IMG_INFO tOsdImgInfo;		
		
		
		
		if ((demo_menu == 0) && (!ui_show_delay) && (1 != demo_engmode))
		{

			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwAntOsdImg[ant_lvl], 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_FPS, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			
			if (current_fps < 10)
			{
				fps_num = current_fps;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNumOsdImg[fps_num], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_NUMX, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 29;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
			}	
			else
			{
				fps_num = current_fps/10;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNumOsdImg[fps_num], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

				fps_num = current_fps%10;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNumOsdImg[fps_num], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 29;				
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);					
			}			
		}
		else if ((demo_menu == 0) && (!ui_show_delay) && (1 == demo_engmode))
		{
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwAntOsdImg[ant_lvl], 1, &tOsdImgInfo);
			tOsdImgInfo.uwYStart += 40*11;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_FPS, 1, &tOsdImgInfo);
			tOsdImgInfo.uwYStart += 40*11;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			
			if (current_fps < 10)
			{
				fps_num = current_fps;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNumOsdImg[fps_num], 1, &tOsdImgInfo);
				tOsdImgInfo.uwYStart += 40*11;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_NUMX, 1, &tOsdImgInfo);
				tOsdImgInfo.uwYStart += 40*11;
				tOsdImgInfo.uwXStart += 29;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
			}	
			else
			{
				fps_num = current_fps/10;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNumOsdImg[fps_num], 1, &tOsdImgInfo);
				tOsdImgInfo.uwYStart += 40*11;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

				fps_num = current_fps%10;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNumOsdImg[fps_num], 1, &tOsdImgInfo);
				tOsdImgInfo.uwYStart += 40*11;
				tOsdImgInfo.uwXStart += 29;				
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);					
			}		

		}
		
}
#endif
//------------------------------------------------------------------------------
static void UI_Thread(void const *argument)
{
	static uint16_t uwUI_TaskCnt = 0;
	while(1)
	{
		UI_UpdateStatus(&uwUI_TaskCnt);
		#ifdef VBM_PU
			printf("UI_Thread:  AntLvl[%d]\n", ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_1_MAIN));
			UI_TimerShow();
		#endif
		
		#ifdef VBM_BU
		//printf(">> TargetQp[0]=%d, TargetQp[1]=%d \n", ubRC_GetTargetQp(0), ubRC_GetTargetQp(1));		//todo
		//printf(">> TargetQp[2]=%d, TargetQp[3]=%d \n", ubRC_GetTargetQp(2), ubRC_GetTargetQp(3));	
		//printf(">> rcQp[0]=%d, rcQp[1]=%d \n", tRC_Info[0].ubTargetQp, tRC_Info[1].ubTargetQp);
		//printf(">> rcQp[2]=%d, rcQp[3]=%d \n", tRC_Info[2].ubTargetQp, tRC_Info[3].ubTargetQp);		
		#endif
		
		osDelay(UI_TASK_PERIOD);
	}
}
//------------------------------------------------------------------------------
static void UI_EventThread(void const *argument)
{
	UI_Event_t tUI_Event;
	while(1)
	{
        osMessageGet(UI_EventQueue, &tUI_Event, osWaitForever);
		UI_EventHandles(&tUI_Event);
	}
}
//------------------------------------------------------------------------------
osMessageQId *pUI_GetEventQueueHandle(void)
{
	return &UI_EventQueue;
}
//------------------------------------------------------------------------------
void UI_SendMessageToAPP(void *pvMessage)
{
	if(osMessagePut(*pAPP_MessageQH, pvMessage, 0) != osOK)
	{
		printd(DBG_ErrorLvl, "APP Q Full\n");
#if KNL_DEBUG_APP_Q_FULL_EN
		KNL_AutoEnterFileListStop();
#endif
#if KNL_DEBUG_INFO_EN
		KNL_PrintDbgInfo();
#endif
	}
}
//------------------------------------------------------------------------------
void UI_StopUpdateThread(void)
{
	if((NULL != osUI_ThreadId) && (osOK != osThreadIsSuspended(osUI_ThreadId)))
		osThreadSuspend(osUI_ThreadId);
}
//------------------------------------------------------------------------------
void UI_StartUpdateThread(void)
{
	if((NULL != osUI_ThreadId) && (osOK == osThreadIsSuspended(osUI_ThreadId)))
		osThreadResume(osUI_ThreadId);
}
//------------------------------------------------------------------------------
void UI_FrameTRXFinish(uint8_t ubFrmRpt)
{
	switch(ubFrmRpt)
	{
	#ifdef VBM_PU
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
	#endif
	#ifdef VBM_BU
		case 0xF:
	#endif
	#if (!defined(BSP_D_SN93716_TX_V1) && (!defined(BSP_D_SN93714_TX_V1) || !APP_SD_FUNC_ENABLE))
		SIGNAL_LED_IO = !SIGNAL_LED_IO;
	#endif
		break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
