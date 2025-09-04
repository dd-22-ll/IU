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
static uint16_t g_rectangle_x = 40; // box X
static uint16_t g_rectangle_y = 110; // box Y
static uint8_t g_is_menu_visible = 1; //0 not visible
static uint8_t g_current_menu_level = 0; //0 mainmenu, 1,2,3 submenu
static uint8_t g_current_language = 0; //current selection language
//static uint8_t g_language_cursor_pos = 0;
static uint8_t g_current_menu_index = 0; // menu index
// Menu(Level 0)
const uint16_t MAIN_MENU_Y_POS[] = {110, 150, 190, 230, 270, 310, 350};
const uint8_t MAIN_MENU_ITEM_COUNT = sizeof(MAIN_MENU_Y_POS) / sizeof(uint16_t);
// KeyLock(Level 1)
const uint16_t KEYLOCK_MENU_Y_POS[] = {110, 150, 190};
const uint8_t KEYLOCK_MENU_ITEM_COUNT = sizeof(KEYLOCK_MENU_Y_POS) / sizeof(uint16_t);
// Zoom(Level 2)
const uint16_t ZOOM_MENU_Y_POS[] = {110, 150};
const uint8_t ZOOM_MENU_ITEM_COUNT = sizeof(DISPLAY_SETTINGS_MENU_Y_POS) / sizeof(uint16_t);
// Settings(Level 4)
const uint16_t SETTINGS_MENU_Y_POS[] = {110, 150, 190, 230, 270, 310, 350, 390};
const uint8_t SETTINGS_MENU_ITEM_COUNT = sizeof(SETTINGS_MENU_Y_POS) / sizeof(uint16_t);
// Pair Units(Level 5)
const uint16_t PAIRUNITS_MENU_Y_POS[] = {110, 150, 190, 230};
const uint8_t PAIRUNITS_MENU_ITEM_COUNT = sizeof(PAIRUNITS_MENU_Y_POS) / sizeof(uint16_t);
// Sleep Timer(Level 7)
const uint16_t SLEEPTIMER_MENU_Y_POS[] = {110, 150};
const uint8_t SLEEPTIMER_MENU_ITEM_COUNT = sizeof(SLEEPTIMER_MENU_Y_POS) / sizeof(uint16_t);
// Language dot
typedef struct {
    uint16_t x;
    uint16_t y;
} Point;

const Point LANGUAGE_MENU_POS[] = {
    {140, 130}, // 0: ENGLISH
    {140, 170}, // 1: DANSK
    {140, 210}, // 2: SUOMI
    {140, 250}, // 3: FRANCAIS
    {440, 130}, // 4: NORSK
    {440, 170}, // 5: SVENSKA
    {440, 210}, // 6: DEUTCH
    {440, 250}, // 7: NEDERLANDS
    {90,  310}  // 8: EXIT
};
const uint8_t LANGUAGE_MENU_ITEM_COUNT = sizeof(LANGUAGE_MENU_POS) / sizeof(Point);

// --- SETTINGS STATE VARIABLES---
static uint8_t g_edit_mode = 0;  //0: navigation, 1: edit
//Stores SETTINGS menu items
static uint8_t g_settings_sensitivity = 5;
static uint8_t g_settings_volume = 4;
static uint8_t g_settings_vibration = 1;  //0: OFF, 1: ON, 2: HIGH
//Store ZOOM menu items
static uint8_t g_zoom_level = 0; // 0: OFF, 1: X2
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
	MenuBackground();
	
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString("MENU", 280, 52);
  UI_DrawString("KEY LOCK", 52, 118);
  UI_DrawString("ZOOM", 52, 158);
  UI_DrawString("LANGUAGE", 52, 198);
  UI_DrawString("SETTINGS", 52, 238);
  UI_DrawString("PAIR UNITS", 52, 278);
  UI_DrawString("INFO", 52, 318);
  UI_DrawString("EXIT", 52, 358);
	
 //white square: x: 425, y: 192   blank: x:40, y:111
	UI_DrawWhiteSquare(425, 192);
	
  static const char* language_strings[] = {
    "ENGLISH",    // 0
    "DANSK",      // 1
    "SUOMI",      // 2
    "FRANCAIS",   // 3
    "NORSK",      // 4
    "SVENSKA",    // 5
    "DEUTCH",     // 6
    "NEDERLANDS"  // 7
  };
	
	if (g_current_language >= 0 && g_current_language < sizeof(language_strings) / sizeof(language_strings[0])) 
	{
		UI_DrawReverseString23(language_strings[g_current_language], 465, 198);
  }
		
	UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_ShowKeyLock(void)
{
	MenuBackground();

  UI_DrawString("KEY LOCK", 240, 52);
  UI_DrawString("ACTIVATE KEY LOCK", 52, 118);
  UI_DrawString("AUTO ACTIVATE", 52, 158);
  UI_DrawString("EXIT", 52, 198);
  
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawWhiteSquare(425, 152);
	UI_DrawReverseString23("AFTER10SEC", 435, 161);

  UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_KeyLockActivate(void)
{
}
//------------------------------------------------------------------------------
void UI_ShowZoom(void)
{
	MenuBackground();
	
	OSD_IMG_INFO tInfor;
	tInfor.uwHSize = 588;	//width
	tInfor.uwVSize = 243;
	tInfor.uwXStart = 26;
	tInfor.uwYStart = 211;
	OSD_EraserImg1(&tInfor);

  UI_DrawString("ZOOM", 280, 52);
  UI_DrawString("DIGITAL ZOOM", 52, 118);
  UI_DrawString("EXIT", 52, 158);

	OSD_IMG_INFO tOsdImgInfoLine1;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine1);
  tOSD_Img2(&tOsdImgInfoLine1, OSD_QUEUE);
	
	OSD_IMG_INFO tOsdImgInfoLine2;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENUHLINE, 1, &tOsdImgInfoLine2);
	tOsdImgInfoLine2.uwXStart = 28;
	tOsdImgInfoLine2.uwYStart = 209;	
  tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);

	UI_DrawHalfWhiteSquare(508, 110);
  if (g_zoom_level == 0)
  {
      UI_DrawReverseString23("OFF", 530, 120);
  }
  else // g_zoom_level == 1
  {
      UI_DrawReverseString23("X2", 530, 120);
  }
	
	if(g_edit_mode == 0)
	{
		UI_Drawbox();
	}
}
//------------------------------------------------------------------------------
void UI_ShowLanguage(void)
{
	MenuBackground();

  UI_DrawString("LANGUAGE", 250, 52);

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
	
	OSD_IMG_INFO tDotInfo;
  uint16_t dot_x = 0, dot_y = 0;
    
  if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DOT, 1, &tDotInfo) == OSD_OK)
  {
      switch(g_current_language)
      {
          case 0: dot_x = 120; dot_y = 140; break; // ENGLISH
          case 1: dot_x = 120; dot_y = 180; break; // DANSK
          case 2: dot_x = 120; dot_y = 220; break; // SUOMI
          case 3: dot_x = 120; dot_y = 260; break; // FRANCAIS
          case 4: dot_x = 420; dot_y = 140; break; // NORSK
          case 5: dot_x = 420; dot_y = 180; break; // SVENSKA
          case 6: dot_x = 420; dot_y = 220; break; // DEUTCH
          case 7: dot_x = 420; dot_y = 260; break; // NEDERLANDS
          default: dot_x = 0; break;
      }

      if (dot_x > 0)
      {
          tDotInfo.uwXStart = dot_x;
          tDotInfo.uwYStart = dot_y;
          tOSD_Img2(&tDotInfo, OSD_QUEUE);
      }
  }
	
  g_rectangle_x = LANGUAGE_MENU_POS[g_current_menu_index].x;
  g_rectangle_y = LANGUAGE_MENU_POS[g_current_menu_index].y;
		
  UI_Drawhalfbox();
}
//------------------------------------------------------------------------------
void UI_ShowSetting(void)
{
	char value_str[4];
	MenuBackground();

  UI_DrawString("SETTINGS", 250, 52);
  UI_DrawString("MICROPHONE SENSITIVITY", 52, 118);
  UI_DrawString("VOLUME", 52, 158);
  UI_DrawString("VIBRATION", 52, 198);
  UI_DrawString("SLEEP TIMER", 52, 238);
  UI_DrawString("DISPLAY SETTINGS", 52, 278);
  UI_DrawString("BABY UNIT SETTINGS", 52, 318);
  UI_DrawString("TEMPERATURE ALARM", 52, 358);
  UI_DrawString("EXIT", 52, 398);
	
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawHalfWhiteSquare(508, 110);
	
	sprintf(value_str, "%d", g_settings_sensitivity);
  UI_DrawString(value_str, 530, 118);
	sprintf(value_str, "%d", g_settings_volume);
  UI_DrawString(value_str, 530, 158);
	
	const char* vibration_str = "";
	switch (g_settings_vibration)
  {
    case 0:
        vibration_str = "OFF";
        break;
    case 1:
        vibration_str = "ON";
        break;
    case 2:
        vibration_str = "HIGH";
        break;
  }
	UI_DrawString(vibration_str, 490, 198);
	
  if(g_edit_mode == 0)
	{
		UI_Drawbox(); 
	}
	
}
//------------------------------------------------------------------------------
void UI_ShowPairUnits(void)
{
	MenuBackground();

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
	MenuBackground();

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
void UI_ShowSleepTimer(void)
{
    MenuBackground(); 

    UI_DrawString("SLEEP TIMER", 220, 52);

    UI_DrawString("ON/OFF", 52, 118);
    UI_DrawString("STOP", 52, 158);
    UI_DrawString("SLEEP HISTORY", 52, 198);
    UI_DrawString("EXIT", 52, 238);

    OSD_IMG_INFO tOsdImgInfoLine;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
    tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);

    OSD_IMG_INFO tDotInfo;
    if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DOT, 1, &tDotInfo) == OSD_OK)
    {
        //g_sleep_timer_setting determines the y of point
        uint16_t dot_y_pos[] = {118, 158, 198, 238};
        if (g_sleep_timer_setting < 4) {
            tDotInfo.uwXStart = 450;
            tDotInfo.uwYStart = dot_y_pos[g_sleep_timer_setting];
            tOSD_Img2(&tDotInfo, OSD_QUEUE);
        }
    }
    
    g_rectangle_x = 40;
    g_rectangle_y = SLEEP_TIMER_MENU_Y_POS[g_current_menu_index];
    UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_TimerShow(void)
{
}
#endif
//------------------------------------------------------------------------------
static void UI_Thread(void const *argument)
{
	static uint16_t uwUI_TaskCnt = 0;
	while(1)
	{
		//UI_UpdateStatus(&uwUI_TaskCnt);		//refresh 
		#ifdef VBM_PU
			//printf("UI_Thread:  AntLvl[%d]\n", ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_1_MAIN));
			//UI_ShowMenu();
			//UI_ShowLanguage();
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
void UI_DrawReverseString23(const char *str, uint16_t startX, uint16_t startY)
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
            case 'A': imageIndex = OSD2IMG_R23_A; break;
            case 'B': imageIndex = OSD2IMG_R23_B; break;
            case 'C': imageIndex = OSD2IMG_R23_C; break;
						case 'D': imageIndex = OSD2IMG_R23_D; break;
            case 'E': imageIndex = OSD2IMG_R23_E; break;
					  case 'F': imageIndex = OSD2IMG_R23_F; break;
					  case 'G': imageIndex = OSD2IMG_R23_G; break;
						case 'H': imageIndex = OSD2IMG_R23_H; break;
					  case 'I': imageIndex = OSD2IMG_R23_I; break;
					  case 'J': imageIndex = OSD2IMG_R23_J; break;
					  case 'K': imageIndex = OSD2IMG_R23_K; break;
					  case 'L': imageIndex = OSD2IMG_R23_L; break;
					  case 'M': imageIndex = OSD2IMG_R23_M; break;
					  case 'N': imageIndex = OSD2IMG_R23_N; break;
					  case 'O': imageIndex = OSD2IMG_R23_O; break;
					  case 'P': imageIndex = OSD2IMG_R23_P; break;
					  case 'Q': imageIndex = OSD2IMG_R23_Q; break;
					  case 'R': imageIndex = OSD2IMG_R23_R; break;
					  case 'S': imageIndex = OSD2IMG_R23_S; break;
					  case 'T': imageIndex = OSD2IMG_R23_T; break;
					  case 'U': imageIndex = OSD2IMG_R23_U; break;
						case 'V': imageIndex = OSD2IMG_R23_V; break;
						case 'W': imageIndex = OSD2IMG_R23_W; break;
						case 'X': imageIndex = OSD2IMG_R23_X; break;
						case 'Y': imageIndex = OSD2IMG_R23_Y; break;
						case 'Z': imageIndex = OSD2IMG_R23_Z; break;
						case ' ': imageIndex = OSD2IMG_SPACE1; break;
						case '0': imageIndex = OSD2IMG_R23_NUM0; break;
            case '1': imageIndex = OSD2IMG_R23_NUM1; break;
            case '2': imageIndex = OSD2IMG_R23_NUM2; break;
            case '3': imageIndex = OSD2IMG_R23_NUM3; break;
            case '4': imageIndex = OSD2IMG_R23_NUM4; break;
            case '5': imageIndex = OSD2IMG_R23_NUM5; break;
            case '6': imageIndex = OSD2IMG_R23_NUM6; break;
            case '7': imageIndex = OSD2IMG_R23_NUM7; break;
            case '8': imageIndex = OSD2IMG_R23_NUM8; break;
            case '9': imageIndex = OSD2IMG_R23_NUM9; break;
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
void UI_DrawString23(const char *str, uint16_t startX, uint16_t startY)
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
            case 'A': imageIndex = OSD2IMG_PX23_A; break;
            case 'B': imageIndex = OSD2IMG_PX23_B; break;
            case 'C': imageIndex = OSD2IMG_PX23_C; break;
						case 'D': imageIndex = OSD2IMG_PX23_D; break;
            case 'E': imageIndex = OSD2IMG_PX23_E; break;
					  case 'F': imageIndex = OSD2IMG_PX23_F; break;
					  case 'G': imageIndex = OSD2IMG_PX23_G; break;
						case 'H': imageIndex = OSD2IMG_PX23_H; break;
					  case 'I': imageIndex = OSD2IMG_PX23_I; break;
					  case 'J': imageIndex = OSD2IMG_PX23_J; break;
					  case 'K': imageIndex = OSD2IMG_PX23_K; break;
					  case 'L': imageIndex = OSD2IMG_PX23_L; break;
					  case 'M': imageIndex = OSD2IMG_PX23_M; break;
					  case 'N': imageIndex = OSD2IMG_PX23_N; break;
					  case 'O': imageIndex = OSD2IMG_PX23_O; break;
					  case 'P': imageIndex = OSD2IMG_PX23_P; break;
					  case 'Q': imageIndex = OSD2IMG_PX23_Q; break;
					  case 'R': imageIndex = OSD2IMG_PX23_R; break;
					  case 'S': imageIndex = OSD2IMG_PX23_S; break;
					  case 'T': imageIndex = OSD2IMG_PX23_T; break;
					  case 'U': imageIndex = OSD2IMG_PX23_U; break;
						case 'V': imageIndex = OSD2IMG_PX23_V; break;
						case 'W': imageIndex = OSD2IMG_PX23_W; break;
						case 'X': imageIndex = OSD2IMG_PX23_X; break;
						case 'Y': imageIndex = OSD2IMG_PX23_Y; break;
						case 'Z': imageIndex = OSD2IMG_PX23_Z; break;
						case ' ': imageIndex = OSD2IMG_SPACE1; break;
						case '0': imageIndex = OSD2IMG_PX23_NUM0; break;
            case '1': imageIndex = OSD2IMG_PX23_NUM1; break;
            case '2': imageIndex = OSD2IMG_PX23_NUM2; break;
            case '3': imageIndex = OSD2IMG_PX23_NUM3; break;
            case '4': imageIndex = OSD2IMG_PX23_NUM4; break;
            case '5': imageIndex = OSD2IMG_PX23_NUM5; break;
            case '6': imageIndex = OSD2IMG_PX23_NUM6; break;
            case '7': imageIndex = OSD2IMG_PX23_NUM7; break;
            case '8': imageIndex = OSD2IMG_PX23_NUM8; break;
            case '9': imageIndex = OSD2IMG_PX23_NUM9; break;
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
void UI_Drawhalfbox(void)
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

    uint16_t original_horizontal_width = horizontal_line_info.uwHSize;
    uint16_t half_horizontal_width = original_horizontal_width / 2;
    uint16_t vertical_height = vertical_line_info.uwVSize;

    horizontal_line_info.uwHSize = half_horizontal_width;


    horizontal_line_info.uwXStart = g_rectangle_x;
    horizontal_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    horizontal_line_info.uwYStart = g_rectangle_y + vertical_height - horizontal_line_info.uwVSize;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    vertical_line_info.uwXStart = g_rectangle_x;
    vertical_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&vertical_line_info, OSD_QUEUE);

    vertical_line_info.uwXStart = g_rectangle_x + half_horizontal_width - vertical_line_info.uwHSize;
    tOSD_Img2(&vertical_line_info, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawWhiteSquare(uint16_t x, uint16_t y)
{
    OSD_IMG_INFO tOsdImgInfoblank;

    if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_WHITESQUARE, 1, &tOsdImgInfoblank) != OSD_OK)
    {
        printf("Error: Failed to load WHITE SQUARE image\n");
        return;
    }

    tOsdImgInfoblank.uwXStart = x;
    tOsdImgInfoblank.uwYStart = y;

    tOSD_Img2(&tOsdImgInfoblank, OSD_QUEUE);
}
//------------------------------------------------------------------------------
void UI_DrawHalfWhiteSquare(uint16_t x, uint16_t y)
{
    OSD_IMG_INFO tOsdImgInfoblank;

    if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_WHITESQUARE, 1, &tOsdImgInfoblank) != OSD_OK)
    {
        printf("Error: Failed to load WHITE SQUARE image\n");
        return;
    }

    tOsdImgInfoblank.uwXStart = x;
    tOsdImgInfoblank.uwYStart = y;

    tOsdImgInfoblank.uwHSize = 92;

    tOSD_Img2(&tOsdImgInfoblank, OSD_QUEUE);
}
//------------------------------------------------------------------------------
void MenuBackground(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
}
//------------------------------------------------------------------------------
void MoveboxDown(void)
{
    if(!g_is_menu_visible) return;
	
		if(g_current_menu_level == 2 && g_edit_mode == 1)
		{
			g_zoom_level = !g_zoom_level;
		}
		else if(g_current_menu_level == 4 && g_edit_mode == 1)
    {
			switch(g_current_menu_index)
        {
           case 0: // MICROPHONE SENSITIVITY
              if(g_settings_sensitivity > 1) g_settings_sensitivity--;
              break;
           case 1: // VOLUME
              if(g_settings_volume > 0) g_settings_volume--;
              break;
           case 2: //VIBRATION
							if(g_settings_vibration > 0) 
							{
								g_settings_vibration--;
							}
							else
							{
								g_settings_vibration = 2;
							}
							break;
        }
		}
		else
		{
        switch (g_current_menu_level)
        {
            case 0: // Menu
                g_current_menu_index = (g_current_menu_index + 1) % MAIN_MENU_ITEM_COUNT;
                g_rectangle_y = MAIN_MENU_Y_POS[g_current_menu_index];
                break;

            case 1: // KeyLock 
                g_current_menu_index = (g_current_menu_index + 1) % KEYLOCK_MENU_ITEM_COUNT;
                g_rectangle_y = KEYLOCK_MENU_Y_POS[g_current_menu_index];
                break;
						
            case 3: // Language
                g_current_menu_index = (g_current_menu_index + 1) % LANGUAGE_MENU_ITEM_COUNT;
                break;

            case 4: // Settings
                g_current_menu_index = (g_current_menu_index + 1) % SETTINGS_MENU_ITEM_COUNT;
                g_rectangle_y = SETTINGS_MENU_Y_POS[g_current_menu_index];
                break;

            case 5: // Pair Units
                g_current_menu_index = (g_current_menu_index + 1) % PAIRUNITS_MENU_ITEM_COUNT;
                g_rectangle_y = PAIRUNITS_MENU_Y_POS[g_current_menu_index];
                break;

            default:
                return; 
        }
		}
    UI_RefreshScreen();
		
}

//------------------------------------------------------------------------------
void MoveboxUp(void)
{
    if(!g_is_menu_visible) return;
	
		if(g_current_menu_level == 2 && g_edit_mode == 1)
		{
			g_zoom_level = !g_zoom_level;
		}
		else if(g_current_menu_level == 4 && g_edit_mode == 1)
    {
			switch(g_current_menu_index)
        {
           case 0: // MICROPHONE SENSITIVITY
              if(g_settings_sensitivity < 5) g_settings_sensitivity++;
              break;
					 
           case 1: // VOLUME
              if(g_settings_volume < 8) g_settings_volume++;
              break;
					 
           case 2: //VIBRATION
							if(g_settings_vibration < 2) 
							{
								g_settings_vibration++;
							}
							else
							{
								g_settings_vibration = 0;
							}
							break;
        }
		}
		else
		{
         switch (g_current_menu_level)
        {
            case 0:
                g_current_menu_index = (g_current_menu_index + MAIN_MENU_ITEM_COUNT - 1) % MAIN_MENU_ITEM_COUNT;
                g_rectangle_y = MAIN_MENU_Y_POS[g_current_menu_index];
                break;

            case 1:
                g_current_menu_index = (g_current_menu_index + KEYLOCK_MENU_ITEM_COUNT - 1) % KEYLOCK_MENU_ITEM_COUNT;
                g_rectangle_y = KEYLOCK_MENU_Y_POS[g_current_menu_index];
                break;
						
            case 3: 
                g_current_menu_index = (g_current_menu_index + LANGUAGE_MENU_ITEM_COUNT - 1) % LANGUAGE_MENU_ITEM_COUNT;
                break;
						
            case 4:
                g_current_menu_index = (g_current_menu_index + SETTINGS_MENU_ITEM_COUNT - 1) % SETTINGS_MENU_ITEM_COUNT;
                g_rectangle_y = SETTINGS_MENU_Y_POS[g_current_menu_index];
                break;

            case 5:
                g_current_menu_index = (g_current_menu_index + PAIRUNITS_MENU_ITEM_COUNT - 1) % PAIRUNITS_MENU_ITEM_COUNT;
                g_rectangle_y = PAIRUNITS_MENU_Y_POS[g_current_menu_index];
                break;

            default:
                return;
        }
      
    }
		UI_RefreshScreen();
}
//------------------------------------------------------------------------------
void EnterKeyHandler(void)
{
    if (!g_is_menu_visible) {
        return;
    }

    switch (g_current_menu_level)
    {
        case 0:
            if (g_current_menu_index == MAIN_MENU_ITEM_COUNT - 1)  //escape_button
            {
                MenuExitHandler();  
            }
            else  //ok_butten
            {
                g_current_menu_level = g_current_menu_index + 1; 
                
                g_current_menu_index = 0; 
							
                switch(g_current_menu_level)
                {
                    case 1: g_rectangle_y = KEYLOCK_MENU_Y_POS[0]; break;
                    case 4: g_rectangle_y = SETTINGS_MENU_Y_POS[0]; break;
                    case 5: g_rectangle_y = PAIRUNITS_MENU_Y_POS[0]; break;
                }
            }
            break;

        case 1: 
            if (g_current_menu_index == KEYLOCK_MENU_ITEM_COUNT - 1) 
            {
                MenuExitHandler(); 
            }
            else {  }//KeyLock
            break;
						
				case 2:
						if(g_edit_mode ==0)
						{
							if(g_current_menu_index == 0)
							{
								g_edit_mode = 1;
							}
							else
							{
								MenuExitHandler();
							}
						}
						else
						{
							g_edit_mode = 0;
						}
						break;
						
        case 4: 
             if (g_edit_mode == 0)
            {
								if(g_current_menu_index == SETTINGS_MENU_ITEM_COUNT - 1)
								{
									MenuExitHandler(); 
								}
								else
								{
									if(g_current_menu_index == 0 || g_current_menu_index == 1 || g_current_menu_index == 2)
									{
										g_edit_mode = 1;
									}
								}
            }
            else if(g_edit_mode == 1)
						{
							g_edit_mode = 0;
						}
            break;

        case 5: 
            if (g_current_menu_index == PAIRUNITS_MENU_ITEM_COUNT - 1)
            {
                MenuExitHandler(); 
            }
            else {  }//pair
            break;

    }
		
    UI_RefreshScreen();
}
//------------------------------------------------------------------------------
void MenuExitHandler(void)
{
    if (!g_is_menu_visible) {
        return;
    }

    if (g_current_menu_level > 0)
    {
        uint8_t previous_menu = g_current_menu_level;

        g_current_menu_level = 0;

        if (previous_menu >= 1 && previous_menu <= 6)
        {
            g_current_menu_index = previous_menu - 1;
            g_rectangle_y = MAIN_MENU_Y_POS[g_current_menu_index];
        }
        else
        {
            g_current_menu_index = 0;
            g_rectangle_y = MAIN_MENU_Y_POS[0];
        }
    }
    else
    {
        g_is_menu_visible = 0;
    }
    UI_RefreshScreen();
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
			else if(g_current_menu_level == 5)
			{
				UI_ShowPairUnits();
			}
			else if(g_current_menu_level == 6)
			{
				UI_Info();
			}
			else if(g_current_menu_level == 7)
			{
				UI_ShowSleepTimer();
			}
	}

	OSD_IMG_INFO tFakeInfo = {0};
  tOSD_Img2(&tFakeInfo, OSD_UPDATE);
}

