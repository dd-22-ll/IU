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
static uint16_t g_rectangle_x = 39; // box X
static uint16_t g_rectangle_y = 135; // box Y
static uint8_t g_is_menu_visible = 1; //0 not visible
static uint8_t g_current_menu_level = 4; //0 mainmenu, 1,2,3 submenu
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
void UI_ShowMenuKey(void)
{
    g_is_menu_visible = !g_is_menu_visible; 

    if (g_is_menu_visible) {
        g_rectangle_x = 39;
        g_rectangle_y = 135;
    }

    UI_RefreshScreen();
}
//------------------------------------------------------------------------------
void UI_ShowMenu(void)
{
	//tOsdImgInfo.uwXStart = 0;
  //tOsdImgInfo.uwYStart = 0;	
	
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
	
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString("MENU", 280, 50);
  UI_DrawString("KEY LOCK", 100, 140);
  UI_DrawString("ZOOM", 100, 180);
  UI_DrawString("LANGUAGE", 100, 220);
  UI_DrawString("SETTINGS", 100, 260);
  UI_DrawString("PAIR UNITS", 100, 300);
  UI_DrawString("INFO", 100, 340);
  UI_DrawString("EXIT", 100, 380);

  UI_DrawString("ENGLISH", 450, 220);
	
	UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_ShowKeyLock(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);

  UI_DrawString("KEY LOCK", 280, 50);
  UI_DrawString("ACTIVATE KEY LOCK", 100, 140);
  UI_DrawString("AUTO ACTIVATE", 100, 180);
  UI_DrawString("EXIT", 100, 220);
    
  UI_DrawString("AFTER 10 SEC", 450, 180);

	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);

  UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_ShowZoom(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);

  UI_DrawString("ZOOM", 280, 50);
  UI_DrawString("DIGITAL ZOOM", 100, 140);
  UI_DrawString("EXIT", 100, 180);

  UI_DrawString("OFF", 450, 140);

	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);

  UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_ShowLanguage(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);

  UI_DrawString("LANGUAGE", 250, 50);

  UI_DrawString("ENGLISH", 150, 140);
  UI_DrawString("DANSK", 150, 180);
  UI_DrawString("SUOMI", 150, 220);
  UI_DrawString("FRANCAIS", 150, 260);

  UI_DrawString("NORSK", 450, 140);
  UI_DrawString("SVENSKA", 450, 180);
  UI_DrawString("DEUTCH", 450, 220);
	UI_DrawString("NEDERLANDS", 450, 260);

  UI_DrawString("EXIT", 100, 320);


	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
		
  UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_ShowSetting(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);

  UI_DrawString("SETTINGS", 250, 50);

  UI_DrawString("MICROPHONE SENSITIVITY", 100, 140);
  UI_DrawString("VOLUME", 100, 180);
  UI_DrawString("VIBRATION", 100, 220);
  UI_DrawString("SLEEP TIMER", 100, 260);
  UI_DrawString("DISPLAY SETTINGS", 100, 300);
  UI_DrawString("BABY UNIT SETTINGS", 100, 340);
  UI_DrawString("TEMPERATURE ALARM", 100, 380);
  UI_DrawString("EXIT", 100, 420);

  UI_DrawString("5", 500, 140);
  UI_DrawString("4", 500, 180);
  UI_DrawString("HIGH", 480, 220);

	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);

  UI_Drawbox(); 
}
//------------------------------------------------------------------------------
void UI_ShowPairUnits(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);

    UI_DrawString("PAIR UNITS", 240, 50);

    UI_DrawString("THIS UNIT IS PAIRED WITH", 100, 100);
    UI_DrawString("BABY UNIT 1", 100, 140);

    UI_DrawString("CONNECT NEW BABY UNIT", 100, 180);
    UI_DrawString("UNPAIR BABY UNIT", 100, 220);
    UI_DrawString("INFO", 100, 260);
    UI_DrawString("EXIT", 100, 300);
    
    UI_DrawString("YOU CAN CONNECT UP TO 3 BABY UNITS", 100, 420);
    UI_DrawString("TO THIS PARENT UNIT", 100, 450);

		OSD_IMG_INFO tOsdImgInfoLine;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
		tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
        
    tOsdImgInfoLine.uwXStart = 39;
    tOsdImgInfoLine.uwYStart = 380;
    tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);

    UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_Info(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);

    UI_DrawString("INFO", 240, 50);

    UI_DrawString("ABOUT THIS MODEL", 100, 100);
    UI_DrawString("FEATURES", 100, 140);

    UI_DrawString("CONNECT NEW BABY UNIT", 100, 180);
    UI_DrawString("CONTACT INFORMATION", 100, 220);
    UI_DrawString("EXIT", 100, 260);

		OSD_IMG_INFO tOsdImgInfoLine;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
		tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);

    UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_TimerShow(void)
{
		if (ui_show_delay)
			ui_show_delay--;
	
    static uint8_t isUiDrawn = 0; 
    
    if (isUiDrawn == 0) 
    {
        OSD_IMG_INFO tOsdImgInfo1;
        
        tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo1);
        tOSD_Img1(&tOsdImgInfo1, OSD_QUEUE);

        UI_DrawString("BC", 300, 300);

        UI_Drawbox();

        isUiDrawn = 1; 
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
			//printf("UI_Thread:  AntLvl[%d]\n", ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_1_MAIN));
			//UI_TimerShow();
			//UI_ShowSetting();
			UI_ShowPairUnits();
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
void UI_DrawString(const char *str, uint16_t startX, uint16_t startY)
{
    OSD_IMG_INFO tCharImgInfo;
    uint16_t currentX = startX;
    int i = 0;

    for (i = 0; str[i] != '\0'; ++i)
    {
        char character = str[i];
        uint16_t imageIndex = 0;

        switch (character)
        {
            case 'A': imageIndex = OSD2IMG_N80_A; break;
            case 'B': imageIndex = OSD2IMG_N80_B; break;
            case 'C': imageIndex = OSD2IMG_N80_C; break;
						case 'D': imageIndex = OSD2IMG_N80_D; break;
            case 'E': imageIndex = OSD2IMG_N80_E; break;
					  case 'F': imageIndex = OSD2IMG_N80_F; break;
					  case 'G': imageIndex = OSD2IMG_N80_G; break;
						case 'H': imageIndex = OSD2IMG_N80_H; break;
					  case 'I': imageIndex = OSD2IMG_N80_I; break;
					  case 'J': imageIndex = OSD2IMG_N80_J; break;
					  case 'K': imageIndex = OSD2IMG_N80_K; break;
					  case 'L': imageIndex = OSD2IMG_N80_L; break;
					  case 'M': imageIndex = OSD2IMG_N80_M; break;
					  case 'N': imageIndex = OSD2IMG_N80_N; break;
					  case 'O': imageIndex = OSD2IMG_N80_O; break;
					  case 'P': imageIndex = OSD2IMG_N80_P; break;
					  case 'Q': imageIndex = OSD2IMG_N80_Q; break;
					  case 'R': imageIndex = OSD2IMG_N80_R; break;
					  case 'S': imageIndex = OSD2IMG_N80_S; break;
					  case 'T': imageIndex = OSD2IMG_N80_T; break;
					  case 'U': imageIndex = OSD2IMG_N80_U; break;
						case 'V': imageIndex = OSD2IMG_N80_V; break;
						case 'W': imageIndex = OSD2IMG_N80_W; break;
						case 'X': imageIndex = OSD2IMG_N80_X; break;
						case 'Y': imageIndex = OSD2IMG_N80_Y; break;
						case 'Z': imageIndex = OSD2IMG_N80_Z; break;
						case ' ': imageIndex = OSD2IMG_SPACE1; break;
						case '0': imageIndex = OSD2IMG_N80_NUM0; break;
            case '1': imageIndex = OSD2IMG_N80_NUM1; break;
            case '2': imageIndex = OSD2IMG_N80_NUM2; break;
            case '3': imageIndex = OSD2IMG_N80_NUM3; break;
            case '4': imageIndex = OSD2IMG_N80_NUM4; break;
            case '5': imageIndex = OSD2IMG_N80_NUM5; break;
            case '6': imageIndex = OSD2IMG_N80_NUM6; break;
            case '7': imageIndex = OSD2IMG_N80_NUM7; break;
            case '8': imageIndex = OSD2IMG_N80_NUM8; break;
            case '9': imageIndex = OSD2IMG_N80_NUM9; break;
            default: continue; 
        }

        if (tOSD_GetOsdImgInfor(1, OSD_IMG2, imageIndex, 1, &tCharImgInfo) == OSD_OK)
        {
            tCharImgInfo.uwXStart = currentX;
            tCharImgInfo.uwYStart = startY;

            tOSD_Img2(&tCharImgInfo, OSD_QUEUE);

            currentX += tCharImgInfo.uwHSize; 
        }
    }
}
//------------------------------------------------------------------------------
void UI_Drawbox(void)
{
	OSD_IMG_INFO horizontal_line_info;
  OSD_IMG_INFO vertical_line_info;


    if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &horizontal_line_info) != OSD_OK)
    {
        printf("Error: Failed to get horizontal line image info.\n");
        return;
    }
    if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2VERTICALLINE, 1, &vertical_line_info) != OSD_OK)
    {
        printf("Error: Failed to get vertical line image info.\n");
        return;
    }


    uint16_t horizontal_width = horizontal_line_info.uwHSize;
    uint16_t vertical_height = vertical_line_info.uwVSize;


    horizontal_line_info.uwXStart = g_rectangle_x;
    horizontal_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    horizontal_line_info.uwYStart = g_rectangle_y + vertical_height - horizontal_line_info.uwVSize;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    vertical_line_info.uwXStart = g_rectangle_x;
    vertical_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&vertical_line_info, OSD_QUEUE);

    vertical_line_info.uwXStart = g_rectangle_x + horizontal_width - vertical_line_info.uwHSize;
    tOSD_Img2(&vertical_line_info, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void MoveboxDown(void)
{
    if (g_is_menu_visible == 1)
    {
        switch (g_current_menu_level)
        {
            case 0: 
                if (g_rectangle_y == 140) { g_rectangle_y = 180; }      
                else if (g_rectangle_y == 180) { g_rectangle_y = 220; }
                else if (g_rectangle_y == 220) { g_rectangle_y = 260; }
                else if (g_rectangle_y == 260) { g_rectangle_y = 300; }
                else if (g_rectangle_y == 300) { g_rectangle_y = 340; }
                else if (g_rectangle_y == 340) { g_rectangle_y = 380; }
                else if (g_rectangle_y == 380) { g_rectangle_y = 140; }
                else { g_rectangle_y = 140; } 
                break;

            case 1: 
                if (g_rectangle_y == 130) { g_rectangle_y = 170; }      
                else if (g_rectangle_y == 170) { g_rectangle_y = 210; }
                else if (g_rectangle_y == 210) { g_rectangle_y = 130; } 
                else { g_rectangle_y = 130; } // ??????
                break;

            case 4: 
                if (g_rectangle_y == 130) { g_rectangle_y = 170; }  
                else if (g_rectangle_y == 170) { g_rectangle_y = 210; }
                else if (g_rectangle_y == 210) { g_rectangle_y = 250; } 
                else if (g_rectangle_y == 250) { g_rectangle_y = 290; } 
                else if (g_rectangle_y == 290) { g_rectangle_y = 330; }
                else if (g_rectangle_y == 330) { g_rectangle_y = 370; }
                else if (g_rectangle_y == 370) { g_rectangle_y = 410; }
                else if (g_rectangle_y == 410) { g_rectangle_y = 130; }
                else { g_rectangle_y = 130; } 
                break;

            case 5: 
                if (g_rectangle_y == 170) { g_rectangle_y = 210; }  
                else if (g_rectangle_y == 210) { g_rectangle_y = 250; } 
                else if (g_rectangle_y == 250) { g_rectangle_y = 290; } 
                else if (g_rectangle_y == 290) { g_rectangle_y = 170; } 
                else { g_rectangle_y = 170; } // ??????
                break;

            // default: break; 
        }
        UI_RefreshScreen();
    }
}
//------------------------------------------------------------------------------
void MoveboxUp(void)
{
    if(g_is_menu_visible == 1)
    {
        switch (g_current_menu_level)
        {
            case 0: 
                if (g_rectangle_y == 140) { g_rectangle_y = 380; }
                else if (g_rectangle_y == 180) { g_rectangle_y = 140; }
                else if (g_rectangle_y == 220) { g_rectangle_y = 180; }
                else if (g_rectangle_y == 260) { g_rectangle_y = 220; }
                else if (g_rectangle_y == 300) { g_rectangle_y = 260; }
                else if (g_rectangle_y == 340) { g_rectangle_y = 300; }
                else if (g_rectangle_y == 380) { g_rectangle_y = 340; }
                else { g_rectangle_y = 140; }
                break;

            case 1: 
                if (g_rectangle_y == 130) { g_rectangle_y = 210; }
                else if (g_rectangle_y == 170) { g_rectangle_y = 130; }
                else if (g_rectangle_y == 210) { g_rectangle_y = 170; }
                else { g_rectangle_y = 130; }
                break;

            case 4: 
                if (g_rectangle_y == 130) { g_rectangle_y = 410; }
                else if (g_rectangle_y == 170) { g_rectangle_y = 130; } 
                else if (g_rectangle_y == 210) { g_rectangle_y = 170; }
                else if (g_rectangle_y == 250) { g_rectangle_y = 210; }
                else if (g_rectangle_y == 290) { g_rectangle_y = 250; }
                else if (g_rectangle_y == 330) { g_rectangle_y = 290; }
                else if (g_rectangle_y == 370) { g_rectangle_y = 330; }
                else if (g_rectangle_y == 410) { g_rectangle_y = 370; }
                else { g_rectangle_y = 130; }
                break;

            case 5: 
                if (g_rectangle_y == 170) { g_rectangle_y = 290; }
                else if (g_rectangle_y == 210) { g_rectangle_y = 170; } 
                else if (g_rectangle_y == 250) { g_rectangle_y = 210; } 
                else if (g_rectangle_y == 290) { g_rectangle_y = 250; } 
                else { g_rectangle_y = 170; }
                break;

            // default: break;
        }
        
        UI_RefreshScreen();
    }
}
//------------------------------------------------------------------------------
void EnterKeyHandler(void) // MoveboxDown, based on location enter submenu
{
    if(g_is_menu_visible == 1)
    {
        if(g_current_menu_level == 0) //main menu
        {
            //"KEY LOCK"  g_rectangle_y  140
            if(g_rectangle_y == 135) 
            {
                g_current_menu_level = 1; //KeyLock
            }
            else if(g_rectangle_y == 180) 
						{ 
								g_current_menu_level = 2; //Zoom
						}
						else if(g_rectangle_y == 220)
						{
								g_current_menu_level = 3; //Language
						}
						else if(g_rectangle_y == 260)
						{
								g_current_menu_level = 4; //Setting
						}
						else if(g_rectangle_y == 300)
						{
								g_current_menu_level = 5; //Pair units
						}
						else if(g_rectangle_y == 340)
						{
								g_current_menu_level = 6; //Info
						}
						else if(g_rectangle_y == 380)
						{
								g_current_menu_level = 7; //Exit
						}

        }
      else if(g_current_menu_level == 1) //submenu
      {
      }
      else if(g_current_menu_level == 2) 
      {
      }
      else if(g_current_menu_level == 3) 
      {
      }
			else if(g_current_menu_level == 4) 
      {
      }
      else if(g_current_menu_level == 5) 
      {
      }
      else if(g_current_menu_level == 6) 
      {
      }
				
      UI_RefreshScreen();
		}
}
//------------------------------------------------------------------------------
void UI_RefreshScreen(void)
{
	//CLEAN
	OSD_ClearImg1Buf();
	OSD_ClearImg2Buf();
	
  if(g_is_menu_visible == 1)
  {
      if(g_current_menu_level == 0)
      {
					UI_ShowMenu(); 
      }
      else if(g_current_menu_level == 1)
      {
					UI_ShowKeyLock();
      }
			else if(g_current_menu_level == 2)
			{
					UI_ShowZoom();
			}
			else if(g_current_menu_level == 3)
			{
					UI_ShowLanguage();
			}
			else if(g_current_menu_level == 4)
			{
					UI_ShowSetting();
			}
	}

	OSD_IMG_INFO tFakeInfo = {0};
  tOSD_Img2(&tFakeInfo, OSD_UPDATE);
}

