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
#include <stdio.h>
#include <string.h>
#include "UI.h"
#include "APP_HS.h"
#include "LCD.h"
#include "OSD.h"
#include "BUF.h"
#include "SF_API.h"


#ifdef VBM_PU
#include "UI_VBMPU.h"
#endif
#ifdef VBM_BU
#include "UI_VBMBU.h"
#endif

#define MENU_LEVEL_SLEEP_TIMER 7
#define USER_SETTINGS_FLASH_ADDR 0xFFA000

osThreadId osUI_ThreadId;
osMessageQId UI_EventQueue;
osMessageQId *pAPP_MessageQH;
static void UI_Thread(void const *argument);
static void UI_EventThread(void const *argument);
static uint16_t g_rectangle_x = 40; // box X
static uint16_t g_rectangle_y = 110; // box Y
static uint8_t g_is_menu_visible = 1; //0 not visible
static uint8_t g_current_menu_level = 0; //0 mainmenu, 1,2,3 submenu
static uint8_t g_current_menu_index = 0; // menu index
// Menu(Level 0)
const uint16_t MAIN_MENU_Y_POS[] = {110, 150, 190, 230, 270, 310, 350};
const uint8_t MAIN_MENU_ITEM_COUNT = sizeof(MAIN_MENU_Y_POS) / sizeof(uint16_t);
// KeyLock(Level 1)
const uint16_t KEYLOCK_MENU_Y_POS[] = {110, 150, 190};
const uint8_t KEYLOCK_MENU_ITEM_COUNT = sizeof(KEYLOCK_MENU_Y_POS) / sizeof(uint16_t);
// Zoom(Level 2)
const uint16_t ZOOM_MENU_Y_POS[] = {110, 150};
const uint8_t ZOOM_MENU_ITEM_COUNT = sizeof(ZOOM_MENU_Y_POS) / sizeof(uint16_t);
// Settings(Level 4)
const uint16_t SETTINGS_MENU_Y_POS[] = {110, 150, 190, 230, 270, 310, 350, 390};
const uint8_t SETTINGS_MENU_ITEM_COUNT = sizeof(SETTINGS_MENU_Y_POS) / sizeof(uint16_t);
// Pair Units(Level 5)
const uint16_t PAIRUNITS_MENU_Y_POS[] = {110, 150, 190, 230};
const uint8_t PAIRUNITS_MENU_ITEM_COUNT = sizeof(PAIRUNITS_MENU_Y_POS) / sizeof(uint16_t);
// Sleep Timer(Level 7)
static uint16_t SLEEPTIMER_MENU_Y_POS[] = {110, 150, 190, 230};
static uint8_t SLEEPTIMER_MENU_ITEM_COUNT = sizeof(SLEEPTIMER_MENU_Y_POS) / sizeof(uint16_t);

// Save
static uint8_t g_settings_changed = 0;

typedef struct {
    uint32_t magic_number; 
    uint8_t sensitivity;
    uint8_t volume;
    uint8_t zoom_level;
    uint8_t language;
} UserSettings_t;

UserSettings_t g_user_settings;

// Language dot
typedef struct {
    uint16_t x;
    uint16_t y;
} Point;

const Point LANGUAGE_MENU_POS[] = {
    {40, 110}, // 0: ENGLISH
    {40, 150}, // 1: DANSK
    {40, 190}, // 2: SUOMI
    {40, 230}, // 3: FRANCAIS
    {320, 110}, // 4: NORSK
    {320, 150}, // 5: SVENSKA
    {320, 190}, // 6: DEUTCH
    {320, 230}, // 7: NEDERLANDS
    {40,  372}  // 8: EXIT
};
const uint8_t LANGUAGE_MENU_ITEM_COUNT = sizeof(LANGUAGE_MENU_POS) / sizeof(Point);


//static uint8_t g_edit_mode = 0;  //0: navigation, 1: edit ±à¼­Ä£Ê½ÔÝ²»ÆôÓÃ

//sleeptimer items
static uint8_t g_sleeptimer_level = 0; //0: ON

typedef struct{
	uint8_t is_connected; 
  char name[20];                  
  uint32_t sleep_time_ms;         
  uint32_t last_update_time;      
}BabyUnit_t;

static BabyUnit_t g_baby_units[3] = {
	{0, "", 0, 0},
  {0, "", 0, 0}, 
  {0, "", 0, 0}
};

static uint8_t g_connected_baby_units = 0;  //when baby connect or disconnect update
//------------------------------------------------------------------------------
void UI_Init(osMessageQId *pvMsgQId)
{
	UI_StateReset();
	printf("DEBUG: The ORIGINAL size of UI_PUSetting_t is %u bytes.\n", sizeof(UI_PUSetting_t));
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
	UI_DrawWhiteSquare(425, 190);
	
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
	
	if(tUI_PuSetting.ubLanguage < sizeof(language_strings) / sizeof(language_strings[0])) 
	{
		if(tUI_PuSetting.ubLanguage == 7)
		{
			UI_DrawReverseString23(language_strings[tUI_PuSetting.ubLanguage], 440, 200);
		}
		else if(tUI_PuSetting.ubLanguage == 3)
		{
			UI_DrawReverseString23(language_strings[tUI_PuSetting.ubLanguage], 455, 200);
		}
		else
		{
			UI_DrawReverseString23(language_strings[tUI_PuSetting.ubLanguage], 465, 200);
		}
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
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
	tOsdImgInfo.uwHSize = 588;
  tOsdImgInfo.uwVSize = 185;
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
	
	//OSD_IMG_INFO tInfor;
	//tInfor.uwHSize = 588;	//width
	//tInfor.uwVSize = 243;
	//tInfor.uwXStart = 26;
	//tInfor.uwYStart = 211;
	//OSD_EraserImg1(&tInfor);

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
  if (tUI_PuSetting.ubZoomLevel == 0)
  {
      UI_DrawReverseString23("OFF", 530, 120);
  }
  else // g_zoom_level == 1
  {
      UI_DrawReverseString23("X2", 537, 120);
  }
	
	UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_ShowLanguage(void)
{
	MenuBackground();

  UI_DrawString("LANGUAGE", 250, 52);

  UI_DrawString("ENGLISH", 72, 118);
  UI_DrawString("DANSK", 72, 158);
  UI_DrawString("SUOMI", 72, 198);
  UI_DrawString("FRANCAIS", 72, 238);

  UI_DrawString("NORSK", 352, 118);
  UI_DrawString("SVENSKA", 352, 158);
  UI_DrawString("DEUTCH", 352, 198);
	UI_DrawString("NEDERLANDS", 352, 238);

  UI_DrawString("EXIT", 52, 380);


	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	OSD_IMG_INFO tDotInfo;
  uint16_t dot_x = 0, dot_y = 0;
    
  if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DOT, 1, &tDotInfo) == OSD_OK)
  {
      switch(tUI_PuSetting.ubLanguage)
      {
          case 0: dot_x = 55; dot_y = 123; break; // ENGLISH
          case 1: dot_x = 55; dot_y = 163; break; // DANSK
          case 2: dot_x = 55; dot_y = 203; break; // SUOMI
          case 3: dot_x = 55; dot_y = 243; break; // FRANCAIS
          case 4: dot_x = 335; dot_y = 123; break; // NORSK
          case 5: dot_x = 335; dot_y = 163; break; // SVENSKA
          case 6: dot_x = 335; dot_y = 203; break; // DEUTCH
          case 7: dot_x = 335; dot_y = 243; break; // NEDERLANDS
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
	UI_DrawHalfWhiteSquare(508, 150);
	UI_DrawHalfWhiteSquare(508, 190);
	
	if(g_connected_baby == 1)
	{
	sprintf(value_str, "%d", tUI_PuSetting.ubMicroSensitivity);
  UI_DrawReverseString23(value_str, 545, 120);
	}
	sprintf(value_str, "%d", tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
  UI_DrawReverseString23(value_str, 545, 160);
	
	const char* vibration_str = "";
	switch (tUI_PuSetting.ubVibration)
  {
    case 0:
        vibration_str = "OFF";
        break;
    case 1:
        vibration_str = "LOW";
        break;
    case 2:
        vibration_str = "HIGH";
        break;
  }
	
	if(vibration_str == "LOW")
	{	
		UI_DrawReverseString23(vibration_str, 527, 200);
	}
	else if(vibration_str == "HIGH")
	{
		UI_DrawReverseString23(vibration_str, 526, 200);
	}
	else
	{
		UI_DrawReverseString23(vibration_str, 530, 200);
	}		
	
	UI_Drawbox(); 
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
void UI_CalculateSleepTimerMenuPositions(void)
{
    uint16_t start_y = 110;
    
    if (g_connected_baby_units == 0) {
        SLEEPTIMER_MENU_Y_POS[0] = start_y;      // ON/OFF
        SLEEPTIMER_MENU_Y_POS[1] = start_y + 40; // STOP
        SLEEPTIMER_MENU_Y_POS[2] = start_y + 80; // SLEEP HISTORY
        SLEEPTIMER_MENU_Y_POS[3] = start_y + 120; // EXIT
        SLEEPTIMER_MENU_ITEM_COUNT = 4;
    }
    else if (g_connected_baby_units == 1) {
        start_y += 25;
        SLEEPTIMER_MENU_Y_POS[0] = start_y;      // ON/OFF
        SLEEPTIMER_MENU_Y_POS[1] = start_y + 40; // STOP
        SLEEPTIMER_MENU_Y_POS[2] = start_y + 80; // SLEEP HISTORY
        SLEEPTIMER_MENU_Y_POS[3] = start_y + 120; // EXIT
        SLEEPTIMER_MENU_ITEM_COUNT = 4;
    }
    else {
        start_y += (g_connected_baby_units * 25 + 25); // ALL
        SLEEPTIMER_MENU_Y_POS[0] = start_y;      // ON/OFF
        SLEEPTIMER_MENU_Y_POS[1] = start_y + 40; // STOP
        SLEEPTIMER_MENU_Y_POS[2] = start_y + 80; // SLEEP HISTORY
        SLEEPTIMER_MENU_Y_POS[3] = start_y + 120; // EXIT
        SLEEPTIMER_MENU_ITEM_COUNT = 4;
    }
}
void UI_ShowSleepTimer(void)
{
  MenuBackground(); 

  UI_DrawString("SLEEP TIMER", 220, 52);

  OSD_IMG_INFO tOsdImgInfoLine;
  tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
  tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	
	uint16_t current_y = 118;

  if (g_connected_baby_units == 0) 
	{
		//no one connect
	}
  else if (g_connected_baby_units == 1) 
	{
      for(uint8_t i = 0; i < 3; i++)
			{
          if(g_baby_units[i].is_connected) 
					{
              OSD_IMG_INFO tDotInfo;
              if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DOT, 1, &tDotInfo) == OSD_OK) 
							{
                  tDotInfo.uwXStart = 52;
                  tDotInfo.uwYStart = current_y;
                  tOSD_Img2(&tDotInfo, OSD_QUEUE);
              }
                
              UI_DrawString23(g_baby_units[i].name, 72, current_y);
                
              UI_DrawString23("TIME", 320, current_y);
                
              char time_str[10];
              uint32_t total_seconds = g_baby_units[i].sleep_time_ms / 1000;
              uint8_t hours = total_seconds / 3600;
              uint8_t minutes = (total_seconds % 3600) / 60;
              uint8_t seconds = total_seconds % 60;
              sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
              UI_DrawString23(time_str, 380, current_y);
                
              current_y += 25;
              break;
          }
      }
  }
  else 
	{
      OSD_IMG_INFO tDotInfo;
      if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DOT, 1, &tDotInfo) == OSD_OK) {
          tDotInfo.uwXStart = 52;
          tDotInfo.uwYStart = current_y;
          tOSD_Img2(&tDotInfo, OSD_QUEUE);
      }
        
      UI_DrawString23("ALL", 72, current_y);
      current_y += 25;

      for(uint8_t i = 0; i < 3; i++) {
          if(g_baby_units[i].is_connected) {

              UI_DrawString23(g_baby_units[i].name, 72, current_y);
                
              UI_DrawString23("TIME", 320, current_y);
                
              char time_str[10];
              uint32_t total_seconds = g_baby_units[i].sleep_time_ms / 1000;
              uint8_t hours = total_seconds / 3600;
              uint8_t minutes = (total_seconds % 3600) / 60;
              uint8_t seconds = total_seconds % 60;
              sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
              UI_DrawString23(time_str, 380, current_y);
                
              current_y += 25;
          }
      }
  }

  UI_CalculateSleepTimerMenuPositions();

  UI_DrawString("ON/OFF", 52, SLEEPTIMER_MENU_Y_POS[0] + 8);
  UI_DrawString("STOP", 52, SLEEPTIMER_MENU_Y_POS[1] + 8);
  UI_DrawString("SLEEP HISTORY", 52, SLEEPTIMER_MENU_Y_POS[2] + 8);
  UI_DrawString("EXIT", 52, SLEEPTIMER_MENU_Y_POS[3] + 8);

  UI_DrawHalfWhiteSquare(508, SLEEPTIMER_MENU_Y_POS[0]);
  if (g_sleeptimer_level == 0) 
	{
      UI_DrawReverseString23("ON", 530, SLEEPTIMER_MENU_Y_POS[0] + 10);
  } 
	else 
	{
      UI_DrawReverseString23("OFF", 530, SLEEPTIMER_MENU_Y_POS[0] + 10);
  }

  UI_Drawbox();
}

void UI_ConnectBabyUnit(uint8_t unit_index, const char* unit_name, uint32_t initial_sleep_time_ms)
{
    if(unit_index < 3) {
        g_baby_units[unit_index].is_connected = 1;
        strncpy(g_baby_units[unit_index].name, unit_name, sizeof(g_baby_units[unit_index].name) - 1);
        g_baby_units[unit_index].name[sizeof(g_baby_units[unit_index].name) - 1] = '\0';
        g_baby_units[unit_index].sleep_time_ms = initial_sleep_time_ms;
        g_baby_units[unit_index].last_update_time = osKernelSysTick();
        
        g_connected_baby_units = 0;
        for(uint8_t i = 0; i < 3; i++) {
            if(g_baby_units[i].is_connected) {
                g_connected_baby_units++;
            }
        }
    }
}

void UI_DisconnectBabyUnit(uint8_t unit_index)
{
    if(unit_index < 3) {
        g_baby_units[unit_index].is_connected = 0;
        g_baby_units[unit_index].name[0] = '\0';
        g_baby_units[unit_index].sleep_time_ms = 0;
        g_baby_units[unit_index].last_update_time = 0;
        
        g_connected_baby_units = 0;
        for(uint8_t i = 0; i < 3; i++) {
            if(g_baby_units[i].is_connected) {
                g_connected_baby_units++;
            }
        }
    }
}

void ShowMicrphoneSensitivity(void)
{
	
}

void DisplaySettings(void)
{
	
}
	
void BabyUnitSettings(void)
{
	
}
	
void TemperatureAlarm(void)
{

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
			
			case MENU_LEVEL_SLEEP_TIMER:
				 g_current_menu_index = (g_current_menu_index + 1) % SLEEPTIMER_MENU_ITEM_COUNT;
				 g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
					break;

     default:
         return; 
    }
		
    UI_RefreshScreen();	
}

//------------------------------------------------------------------------------
void MoveboxUp(void)
{
    if(!g_is_menu_visible) return;
	
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
    
		 case MENU_LEVEL_SLEEP_TIMER:
				 g_current_menu_index = (g_current_menu_index + SLEEPTIMER_MENU_ITEM_COUNT - 1) % SLEEPTIMER_MENU_ITEM_COUNT;
				 g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
				 break;
		
     default:
         return;
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
							
                switch(g_current_menu_level)  //original position
                {
                    case 1: g_rectangle_y = KEYLOCK_MENU_Y_POS[0]; break;
										case 2: g_rectangle_y = ZOOM_MENU_Y_POS[0]; break;
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
						switch(g_current_menu_index)
            {
                case 0: // DIGITAL ZOOM
                    //ZOOM: OFF -> X2 -> OFF
                    tUI_PuSetting.ubZoomLevel++;
                    if(tUI_PuSetting.ubZoomLevel > 1) 
										{
                        tUI_PuSetting.ubZoomLevel = 0;
                    }
										g_settings_changed = 1;
                    break;
                    
                case 1: // EXIT
                    MenuExitHandler();
                    break;
                    
                default:
                    break;
            }
            break;
						
				case 3:
						switch(g_current_menu_index)
						{
								case 0: // ENGLISH
                    tUI_PuSetting.ubLanguage = 0;
										g_settings_changed = 1;
                    break;
                case 1: // DANSK
                    tUI_PuSetting.ubLanguage = 1;
										g_settings_changed = 1;
                    break;
                case 2: // SUOMI
                    tUI_PuSetting.ubLanguage = 2;
										g_settings_changed = 1;
                    break;
                case 3: // FRANCAIS
                    tUI_PuSetting.ubLanguage = 3;
										g_settings_changed = 1;
                    break;
                case 4: // NORSK
                    tUI_PuSetting.ubLanguage = 4;
										g_settings_changed = 1;
                    break;
                case 5: // SVENSKA
                    tUI_PuSetting.ubLanguage = 5;
										g_settings_changed = 1;
                    break;
                case 6: // DEUTCH
                    tUI_PuSetting.ubLanguage = 6;
										g_settings_changed = 1;
                    break;
                case 7: // NEDERLANDS
                    tUI_PuSetting.ubLanguage = 7;
										g_settings_changed = 1;
                    break;
                case 8: // EXIT
                    MenuExitHandler();
                    break;
                default:
                    break;
            }
						
            break;
						
        case 4: 
             switch(g_current_menu_index)
            {
                case 0: // MICROPHONE SENSITIVITY
                    
                    tUI_PuSetting.ubMicroSensitivity++;
                    if(tUI_PuSetting.ubMicroSensitivity > 5) 
										{
                        tUI_PuSetting.ubMicroSensitivity = 1;
                    }
										g_settings_changed = 1;
                    break;
                    
                case 1: // VOLUME
                    //0-6
                    tUI_PuSetting.VolLvL.tVOL_UpdateLvL++;
                    if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL > 6) 
										{
                        tUI_PuSetting.VolLvL.tVOL_UpdateLvL = 0;
                    }
										g_settings_changed = 1;
										
										//change underlying volume
                    break;
                    
                case 2: // VIBRATION
                    //OFF->LOW->HIGH->OFF
                    tUI_PuSetting.ubVibration++;
                    if(tUI_PuSetting.ubVibration > 2) 
										{
                        tUI_PuSetting.ubVibration = 0;
                    }
										g_settings_changed = 1;
                    break;
                    
                case 3: // SLEEP TIMER
                    g_current_menu_level = MENU_LEVEL_SLEEP_TIMER;
                    g_current_menu_index = 0;
										UI_CalculateSleepTimerMenuPositions();
										g_rectangle_x = 40;
										g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
                    break;
                    
                case 4: // DISPLAY SETTINGS
                    
                    break;
                    
                case 5: // BABY UNIT SETTINGS
                    
                    break;
                    
                case 6: // TEMPERATURE ALARM
                    
                    break;
                    
                case 7: // EXIT
                    MenuExitHandler();
                    break;
                    
                default:
                    break;
            }
            break;

        case 5: 
            if (g_current_menu_index == PAIRUNITS_MENU_ITEM_COUNT - 1)
            {
                MenuExitHandler(); 
            }
            else {  }//pair
            break;
						
				case MENU_LEVEL_SLEEP_TIMER:
						switch(g_current_menu_index)
            {
                case 0: // ON/OFF
                    g_sleeptimer_level = !g_sleeptimer_level;
                    break;
                    
                case 1: // STOP
                    break;
                    
                case 2: // SLEEP HISTORY
                    break;
                    
                case 3: // EXIT SETTINGS SLEEP TIMER
										MenuExitHandler();
                    break;
                    
                default:
                    break;
            }
            break;
    }
		
    UI_RefreshScreen();
}
//------------------------------------------------------------------------------
void MenuExitHandler(void)
{
    if(!g_is_menu_visible) {
        return;
    }
		
		if(g_settings_changed == 1)
		{
			UI_UpdateDevStatusInfo();
			g_settings_changed = 0;
		}
		
		
		if(g_current_menu_level == MENU_LEVEL_SLEEP_TIMER)
		{
				g_current_menu_level = 4; // Settings menu level
        g_current_menu_index = 3; // cursor positioning "SLEEP TIMER"
        g_rectangle_y = SETTINGS_MENU_Y_POS[g_current_menu_index];
		}
    else if(g_current_menu_level > 0)
    {
        uint8_t previous_menu = g_current_menu_level;

        g_current_menu_level = 0;

        if(previous_menu >= 1 && previous_menu <= 6)
        {
            g_current_menu_index = previous_menu - 1;
						g_rectangle_x = 40;
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
			else if(g_current_menu_level == MENU_LEVEL_SLEEP_TIMER)
			{
				UI_ShowSleepTimer();
			}
	}

	OSD_IMG_INFO tFakeInfo = {0};
  tOSD_Img2(&tFakeInfo, OSD_UPDATE);
}

