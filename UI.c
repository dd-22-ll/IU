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
#include "PAIR.h"
#include "TRX_IF.h"
#include "KNL.h"

#ifdef VBM_PU
#include "UI_VBMPU.h"
extern UI_BUStatus_t tUI_CamStatus[CAM_4T];
#endif
#ifdef VBM_BU
#include "UI_VBMBU.h"
#endif

#define MENU_LEVEL_SLEEP_TIMER 				7
#define MENU_MICROPHONE_SENSITIVITY 	8
#define MENU_DISPLAYSETTINGS 					9
#define MENU_ADVANCEDDISPLAYSETTINGS 10
#define MENU_BABYUNITSETTINGS        11
#define MENU_BABYUNITDETAIL					 12
#define MENU_CHANGE_NAME 						 13
#define MENU_LEVEL_TEMP_ALARM 			 14
#define MENU_LEVEL_SET_TEMP_HIGH 		 15
#define MENU_LEVEL_SET_TEMP_LOW  		 16
#define MENU_PAIR_NEW_BABY_UNIT 		 17
#define MENU_ABOUT_THIS_MODEL		 		 18
#define MENU_CONTACT_INFORMATION		 19
#define MENU_FEATURES								 20
#define MENU_RANGE_AND_TRANSMISSION	 21
#define MENU_BATTERY							 	 22
#define MENU_SPECIAL_FEATURES				 23
#define MENU_PAIRING_INFO						 24
#define MENU_ABOUT_PAIRING					 25
#define MENU_COMPATIBLE_UNITS				 26
#define MENU_PAIRING_WITH_N65				 27
#define MENU_BABY_UNIT_INFO					 28
#define MENU_PAIR_MODE					 		 29
#define MENU_SLEEP_HISTORY           30
#define MENU_SLEEP_HISTORY_VIEW      31
#define MENU_SLEEP_HISTORY_OPTIONS   32
#define MENU_SLEEP_HISTORY_SELECT    33


#define KEYBOARD_ROWS 3
#define KEYBOARD_ROW_0_COLS 13
#define KEYBOARD_ROW_1_COLS 13
#define KEYBOARD_ROW_2_COLS 6
#define TOTAL_KEYBOARD_KEYS (KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS + KEYBOARD_ROW_2_COLS)
#define TOTAL_MENU_ITEMS (TOTAL_KEYBOARD_KEYS + 2)

const char* ENGLISH_KEYBOARD[KEYBOARD_ROWS][13] = {
    { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M" },
    { "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" },
		{ "\xC6", "\xD8", "\xC5" "-" "SPACE", "DELETE", NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};

//baby unit settings
static uint8_t s_prev_buset_count = 0xFF;
static uint8_t g_bu_idx_map[CAM_4T] = {0,0,0,0};

//name
static char g_edit_name_buffer[32];
uint8_t g_edit_name_len = 0;
static uint8_t g_name_cursor_pos = 0;

//Language, Degree
typedef struct {
    uint16_t x;
    uint16_t y;
} Point;
const Point TEMP_HIGH_MENU_POS[] = {
    {40, 140}, {180, 140},{320, 140}, {460, 140},
    {40, 180}, {180, 180},{320, 180}, {460, 180},
    {40, 220}, {180, 220},{320, 220}, {460, 220},
    {50,  260}
};
const uint8_t TEMP_HIGH_VALUES[] = {40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20, 18};
const uint8_t TEMP_HIGH_MENU_ITEM_COUNT = sizeof(TEMP_HIGH_MENU_POS) / sizeof(Point);


const Point TEMP_LOW_MENU_POS[] = {
    {40, 140}, {180, 140},{320, 140}, {460, 140},
    {40, 180}, {180, 180},{320, 180}, {460, 180},
    {40, 220}, {180, 220},{320, 220}, {460, 220},
    {40, 260}, {180, 260},
    {50,  300}
};
const int8_t TEMP_LOW_VALUES[] = {16, 14, 12, 10, 8, 6, 4, 2, -0, -2, -4, -6, -8, -10};
const uint8_t TEMP_LOW_MENU_ITEM_COUNT = sizeof(TEMP_LOW_MENU_POS) / sizeof(Point);


osThreadId osUI_ThreadId;
osMessageQId UI_EventQueue;
osMessageQId *pAPP_MessageQH;
static void UI_Thread(void const *argument);
static void UI_EventThread(void const *argument);

static uint16_t g_rectangle_x = 40; // box X
static uint16_t g_rectangle_y = 110; // box Y
uint8_t g_is_menu_visible = 0; //0 not visible    static
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
static uint16_t PAIRUNITS_MENU_Y_POS[] = {122, 162, 202, 242};
static uint8_t PAIRUNITS_MENU_ITEM_COUNT = sizeof(PAIRUNITS_MENU_Y_POS) / sizeof(uint16_t);
// Info
static uint16_t INFO_MENU_Y_POS[] = {110, 150, 190, 230};
static uint8_t INFO_MENU_ITEM_COUNT = sizeof(INFO_MENU_Y_POS) / sizeof(uint16_t);
// Sleep Timer(Level 7)
static uint16_t SLEEPTIMER_MENU_Y_POS[] = {110, 150, 190, 230};
static uint8_t SLEEPTIMER_MENU_ITEM_COUNT = sizeof(SLEEPTIMER_MENU_Y_POS) / sizeof(uint16_t);
// Sleep history
static const uint16_t SLEEPHIS_MAIN_Y_POS[] = {110, 150, 190};   // VIEW / OPTIONS / EXIT
static const uint8_t  SLEEPHIS_MAIN_ITEM_COUNT = sizeof(SLEEPHIS_MAIN_Y_POS)/sizeof(uint16_t);
// Sleep history option
static const uint16_t SLEEPHIS_OPT_Y_POS[]  = {110, 150};        // DELETE / EXIT
static const uint8_t  SLEEPHIS_OPT_ITEM_COUNT = sizeof(SLEEPHIS_OPT_Y_POS)/sizeof(uint16_t);
// Sleep history select
static const uint16_t SLEEPHIS_BUSEL_Y_POS[] = {110, 150, 190, 372};
static const uint8_t  SLEEPHIS_BUSEL_ITEM_COUNT = sizeof(SLEEPHIS_BUSEL_Y_POS)/sizeof(uint16_t);
// Display Settings(Level 9)
static uint16_t DISPLAYSETTINGS_MENU_Y_POS[] = {110, 150, 190, 230, 270};
static uint8_t DISPLAYSETTINGS_MENU_ITEM_COUNT = sizeof(DISPLAYSETTINGS_MENU_Y_POS) / sizeof(uint16_t);
// Advanced Display Settings(Level 10)
static uint16_t ADVANCEDDISPLAYSETTINGS_MENU_Y_POS[] = {110, 150};
static uint8_t ADVANCEDDISPLAYSETTINGS_MENU_ITEM_COUNT = sizeof(ADVANCEDDISPLAYSETTINGS_MENU_Y_POS) / sizeof(uint16_t);
// Baby Unit Settings(Level 11)
static uint8_t BABYUNITSETTINGS_MENU_ITEM_COUNT = 0;
static uint16_t BABYUNITSETTINGS_MENU_Y_POS[10];
// Features
static uint16_t FEATURES_MENU_Y_POS[] = {110, 150, 190, 230};
static uint8_t FEATURES_MENU_ITEM_COUNT = sizeof(FEATURES_MENU_Y_POS) / sizeof(uint16_t);
// PairingInfo
static uint16_t PAIRINGINFO_MENU_Y_POS[] = {110, 150, 190};
static uint8_t PAIRINGINFO_MENU_ITEM_COUNT = sizeof(PAIRINGINFO_MENU_Y_POS) / sizeof(uint16_t);
// CompatibleUnits
static uint16_t COMPATIBLEUNITS_MENU_Y_POS[] = {325, 365};
static uint8_t COMPATIBLEUNITS_MENU_ITEM_COUNT = sizeof(COMPATIBLEUNITS_MENU_Y_POS) / sizeof(uint16_t);
// Save
static uint8_t g_settings_changed = 0;

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

//static uint8_t g_edit_mode = 0;  	0: navigation, 1: edit ±à¼­Ä£Ê½ÔÝ²»ÆôÓÃ

static uint8_t g_sleeptimer_level = 1; //0: ON    sleeptimer items
static uint32_t g_sleep_timer_start_ms[CAM_4T] = {0};  //SLEEPTIME
//Sleep history
#define SLEEP_HISTORY_MAX 8

static uint32_t g_sleep_history_ms[CAM_4T][SLEEP_HISTORY_MAX] = {0};
static uint8_t  g_sleep_hist_count[CAM_4T] = {0};
static uint8_t  g_sleep_hist_head[CAM_4T]  = {0};
static uint8_t  g_hist_show_unit = 0;

static void SleepHistory_Push(uint8_t unit, uint32_t ms)
{
    if (unit >= CAM_4T || ms == 0) return;
    if (g_sleep_hist_count[unit] == 0)
		{
        g_sleep_hist_head[unit] = 0;
    } 
		else
		{
        g_sleep_hist_head[unit] = (uint8_t)((g_sleep_hist_head[unit] + 1) % SLEEP_HISTORY_MAX);
    }
    g_sleep_history_ms[unit][g_sleep_hist_head[unit]] = ms;
    if (g_sleep_hist_count[unit] < SLEEP_HISTORY_MAX) 
		{
        g_sleep_hist_count[unit]++;
    }
}

static void UI_SleepHist_Push(UI_CamNum_t cam, uint32_t dur_ms)
{
    if (dur_ms == 0) return;
    UI_BUStatus_t *st = &tUI_CamStatus[cam];
    uint8_t next = (st->sleep_hist_head + 1) % UI_SLEEP_HISTORY_DEPTH;
    st->sleep_hist_head      = next;
    st->sleep_hist_ms[next]  = dur_ms;
    if (st->sleep_hist_count < UI_SLEEP_HISTORY_DEPTH)
        st->sleep_hist_count++;
}

static void UI_SleepHist_Clear(UI_CamNum_t cam)
{
    UI_BUStatus_t *st = &tUI_CamStatus[cam];
    memset(st->sleep_hist_ms, 0, sizeof(st->sleep_hist_ms));
    st->sleep_hist_count = 0;
    st->sleep_hist_head  = UI_SLEEP_HISTORY_DEPTH - 1;
}

static uint8_t g_selected_bu_index = 0;
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
extern uint8_t demo_menu;
extern UI_State_t tUI_State;
uint8_t ui_show_delay = 15;
uint8_t demo_engmode = 0;
void UI_ShowMenuKey(void)
{
    g_is_menu_visible = !g_is_menu_visible; 
		/*
    if (g_is_menu_visible) {
        g_rectangle_x = 40;
        g_rectangle_y = 110;
    }
		*/
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
	UI_DrawWhiteSquare(425, 150);
	
  const char* auto_activate_str;
  switch (tUI_PuSetting.ubKeyLockAutoActivate)
  {
    case 0:
        auto_activate_str = "NEVER";
        UI_DrawReverseString23(auto_activate_str, 480, 160);
        break;
    case 1:
        auto_activate_str = "AFTER 10 SEC";
        UI_DrawReverseString23(auto_activate_str, 435, 160);
        break;
    case 2:
        auto_activate_str = "AFTER 30 SEC";
        UI_DrawReverseString23(auto_activate_str, 435, 160);
        break;
  }

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
	
	if(tUI_PuSetting.ubPairedBuNum <= 1)
	{
	sprintf(value_str, "%d", tUI_CamStatus[CAM1].ubMicroSensitivity);
  UI_DrawReverseString23(value_str, 545, 120);
	}

	sprintf(value_str, "%d", tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
  UI_DrawReverseString23(value_str, 545, 160);
	
	const char* vibration_str = "";
	switch(tUI_PuSetting.ubVibration)
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
	
	if(strcmp(vibration_str, "LOW") == 0)
	{	
		UI_DrawReverseString23(vibration_str, 527, 200);
	}
	else if(strcmp(vibration_str, "HIGH") == 0)
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
void UI_UpdatePairUnitsMenuPositions(void)
{
    uint8_t connected_count = 0;
    for(uint8_t i = 0; i < 3; i++) 
    {
        if(ubTRX_GetLinkStatus(i))// g_baby_units[i].is_connected
        {
            connected_count++;
        }
    }
    
    uint16_t base_y;
    uint8_t menu_item_count;
    
    if(connected_count == 0) {
        base_y = 127;
        menu_item_count = 3;
        PAIRUNITS_MENU_Y_POS[0] = base_y;       // CONNECT NEW BABY UNIT
        PAIRUNITS_MENU_Y_POS[1] = base_y + 40; // INFO  
        PAIRUNITS_MENU_Y_POS[2] = base_y + 80; // EXIT
    } else if(connected_count == 1) {
        base_y = 127 + 20;
        menu_item_count = 4;
        PAIRUNITS_MENU_Y_POS[0] = base_y;       // CONNECT NEW BABY UNIT
        PAIRUNITS_MENU_Y_POS[1] = base_y + 40; // UNPAIR BABY UNIT
        PAIRUNITS_MENU_Y_POS[2] = base_y + 80; // INFO
        PAIRUNITS_MENU_Y_POS[3] = base_y + 120; // EXIT
    } else if(connected_count == 2) {
        base_y = 127 + 40;
        menu_item_count = 4;
        PAIRUNITS_MENU_Y_POS[0] = base_y;       // CONNECT NEW BABY UNIT
        PAIRUNITS_MENU_Y_POS[1] = base_y + 40; // UNPAIR BABY UNIT
        PAIRUNITS_MENU_Y_POS[2] = base_y + 80; // INFO
        PAIRUNITS_MENU_Y_POS[3] = base_y + 120; // EXIT
    } else { // connected_count == 3
        base_y = 127 + 60;
        menu_item_count = 3;
        PAIRUNITS_MENU_Y_POS[0] = base_y;       // UNPAIR BABY UNIT
        PAIRUNITS_MENU_Y_POS[1] = base_y + 40; // INFO
        PAIRUNITS_MENU_Y_POS[2] = base_y + 80; // EXIT
    }
    
    PAIRUNITS_MENU_ITEM_COUNT = menu_item_count;
}

void UI_ShowPairUnits(void)
{
    MenuBackground();
    UI_DrawString("PAIR UNITS", 240, 52);
    
    OSD_IMG_INFO tOsdImgInfoLine;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
    tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
    
    uint16_t current_y = 100;
    uint8_t connected_count = 0;
    
    // Count connected baby units
    for(uint8_t i = 0; i < 3; i++)
		{
        if(ubTRX_GetLinkStatus(i))
				{
            connected_count++;
        }
    }
    
    // Display interface based on connection count
    if (connected_count == 0) 
		{
        // No baby units paired - Show simple interface
        UI_DrawString23("NO BABY UNITS PAIRED", 52, current_y);
        current_y += 30;
        
        UI_DrawString("CONNECT NEW BABY UNIT", 52, current_y + 5);//
        current_y += 40;
        UI_DrawString("INFO", 52, current_y + 5);
        current_y += 40;
        UI_DrawString("EXIT", 52, current_y + 5);
    }
    else if (connected_count == 1) 
		{
        // One baby unit paired
        UI_DrawString23("THIS UNIT IS PAIRED WITH:", 52, current_y);
        current_y += 20;
        
        // Display the connected baby unit
        for(uint8_t i = 0; i < 3; i++)
				{
            if(ubTRX_GetLinkStatus(i))// g_baby_units[i].is_connected
						{
                UI_DrawString23(tUI_CamStatus[i].name, 52, current_y + 5);
                current_y += 20;
                break;
            }
        }
        current_y += 10; // Extra spacing before menu items
				
        UI_DrawString("CONNECT NEW BABY UNIT", 52, current_y + 5);//
        current_y += 40;
        UI_DrawString("UNPAIR BABY UNIT", 52, current_y + 5);
        current_y += 40;
        UI_DrawString("INFO", 52, current_y + 5);
        current_y += 40;
        UI_DrawString("EXIT", 52, current_y + 5);
    }
    else if (connected_count == 2)
		{
        // Two baby units paired
        UI_DrawString23("THIS UNIT IS PAIRED WITH:", 52, current_y);
        current_y += 20;
        
        // Display connected baby units
        for(uint8_t i = 0; i < 3; i++) 
				{
            if(ubTRX_GetLinkStatus(i))// g_baby_units[i].is_connected
						{
                UI_DrawString23(tUI_CamStatus[i].name, 52, current_y + 5);
                current_y += 20;
            }
        }
        current_y += 10; // Extra spacing before menu items
				
        UI_DrawString("CONNECT NEW BABY UNIT", 52, current_y + 5);//
        current_y += 40;
        UI_DrawString("UNPAIR BABY UNIT", 52, current_y + 5);
        current_y += 40;
        UI_DrawString("INFO", 52, current_y + 5);
        current_y += 40;
        UI_DrawString("EXIT", 52, current_y + 5);
        
    }
    else if (connected_count == 3) 
		{
        // Three baby units paired (maximum)
        UI_DrawString23("THIS UNIT IS PAIRED WITH:", 52, current_y);
        current_y += 20;
        
        // Display all connected baby units
        for(uint8_t i = 0; i < 3; i++) 
				{
            if(ubTRX_GetLinkStatus(i))// g_baby_units[i].is_connected
						{
                UI_DrawString23(tUI_CamStatus[i].name, 52, current_y + 5);
                current_y += 20;
            }
        }
				
        current_y += 10;
        
        // No "CONNECT NEW BABY UNIT" option when at maximum capacity
        UI_DrawString("UNPAIR BABY UNIT", 52, current_y + 5);//
        current_y += 40;
        UI_DrawString("INFO", 52, current_y + 5);
        current_y += 40;
        UI_DrawString("EXIT", 52, current_y + 5);
    }
    
    // bottom line
    OSD_IMG_INFO tOsdImgInfoLine2;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine2);
    tOsdImgInfoLine2.uwXStart = 40;
    tOsdImgInfoLine2.uwYStart = 395;
    tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);
    
    UI_DrawString23("YOU CAN CONNECT UP TO 3 BABY UNITS", 52, 405);
    UI_DrawString23("TO THIS PARENT UNIT", 52, 430);
		
		UI_UpdatePairUnitsMenuPositions();
    
    UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_PairNewBabyUnit(void)
{
    MenuBackground();
    UI_DrawString("PAIR NEW BABY UNIT", 160, 52);
    
    OSD_IMG_INFO tOsdImgInfoLine;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
    tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
    
    uint16_t current_y = 100;
    uint8_t connected_count = 0;
    
    // Count connected baby units
    for(uint8_t i = 0; i < 3; i++)
		{
        if(ubTRX_GetLinkStatus(i))// g_baby_units[i].is_connected
				{
            connected_count++;
        }
    }
    
    // Display connection status based on connected units
    if (connected_count == 0)
		{
        // No baby units connected
        UI_DrawString23("NO BABY UNITS PAIRED", 52, current_y);
        current_y += 30;
			
			  UI_DrawString("GO TO PAIR MODE", 52, current_y + 5);
				current_y += 40;
				UI_DrawString("EXIT", 52, current_y + 5);
    }
    else if(connected_count == 3)
		{
        // All three units connected - show the list
        UI_DrawString23("THIS UNIT IS PAIRED WITH:", 52, current_y);
        current_y += 20;
        
        // Display all connected baby units
        for(uint8_t i = 0; i < 3; i++)
				{
            if(ubTRX_GetLinkStatus(i))// g_baby_units[i].is_connected
						{
                UI_DrawString23(tUI_CamStatus[i].name, 52, current_y + 5);
                current_y += 20;
            }
        }
        current_y += 10; // Extra spacing before menu items
    }
    else
		{
        // 1 or 2 units connected - show which ones
        UI_DrawString23("THIS UNIT IS PAIRED WITH:", 52, current_y);
        current_y += 20;
        
        // Display connected baby units
        for(uint8_t i = 0; i < 3; i++)
				{
            if(ubTRX_GetLinkStatus(i))// g_baby_units[i].is_connected
						{
                UI_DrawString23(tUI_CamStatus[i].name, 52, current_y + 5);
                current_y += 20;
            }
        }
				current_y += 10; // Extra spacing before menu items
				
				UI_DrawString("GO TO PAIR MODE", 52, current_y + 5);
				current_y += 40;
				UI_DrawString("EXIT", 52, current_y + 5);
    }
    
    OSD_IMG_INFO tOsdImgInfoLine2;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine2);
    tOsdImgInfoLine2.uwXStart = 40;
    tOsdImgInfoLine2.uwYStart = 345;
    tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);
    
    UI_DrawString23("ENTERING PAIR MODE WILL UNPAIR THE", 52, 355);
    UI_DrawString23("CONNECTED BABY UNITS. YOU HAVE TO", 52, 380);
    UI_DrawString23("PAIR ALL THE BABY UNITS YOU WISH TO", 52, 405);
    UI_DrawString23("USE IN ONE SETTING.", 52, 430);
		
		UI_UpdatePairUnitsMenuPositions();
    
    UI_Drawbox();
}
void UI_PairMode(void)
{
	  MenuBackground();
    UI_DrawString("PAIR MODE", 160, 52);
    
    OSD_IMG_INFO tOsdImgInfoLine;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
    tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
		UI_DrawString23("HOW MANY BABY UNITS DO YOU WISH", 52, 100);
    UI_DrawString23("TO PAIR WITH THIS PARENT UNIT?", 52, 125);
	
		UI_DrawString("1", 52, 155);
		UI_DrawString("2", 52, 195);
		UI_DrawString("3", 52, 235);
		UI_DrawString("CANCEL", 52, 275);
	
    OSD_IMG_INFO tOsdImgInfoLine2;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine2);
    tOsdImgInfoLine2.uwYStart = 345;
    tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);
		UI_DrawString23("ENTERING PAIR MODE WILL UNPAIR THE", 52, 355);
    UI_DrawString23("CONNECTED BABY UNITS. YOU HAVE TO", 52, 380);	
		UI_DrawString23("PAIR ALL THE BABY UNITS YOU WISH TO", 52, 405);
    UI_DrawString23("USE IN ONE SETTING.", 52, 430);
}
void UI_PairInfo(void)
{
	MenuBackground();
	UI_DrawString("PAIRING INFO", 220, 52);

	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString("ABOUT PAIRING", 52, 118);
	UI_DrawString("COMPATIBLE UNITS", 52, 158);
	UI_DrawString("EXIT", 52, 198);
	
	UI_Drawbox();
}

void UI_AboutPairing(void)
{
	MenuBackground();
	UI_DrawString("ABOUT PAIRING", 190, 52);
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString23("PAIRING REFERS TO ESTABLISHING A", 52, 115);
  UI_DrawString23("CONNECTION BETWEEN THE PARENT UNIT", 52, 140);
  UI_DrawString23("AND ONE, OR MORE, BABY UNITS.", 52, 165);
	UI_DrawString23("THE UNITS ARE PRE PAIRED, BUT WE", 52, 200);
  UI_DrawString23("RECOMMEND PAIRING THEM AGAIN TO", 52, 225);
  UI_DrawString23("MAKE SURE THAT THE CONNECTION IS", 52, 250);
	UI_DrawString23("SAFE AND SECURE.", 52, 275);
	
  UI_DrawString("EXIT", 52, 372);
	
	UI_Drawbox();
}

void UI_CompatibleUnits(void)
{
	MenuBackground();
	UI_DrawString("COMPATIBLE UNITS", 175, 52);
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString23("THE PARENT UNIT OF NEONATE N80 IS", 52, 115);
  UI_DrawString23("COMPATIBLE WITH:", 52, 140);
  UI_DrawString23("N80 BABY UNITS", 52, 175);
	UI_DrawString23("N65 BABY UNITS (AUDIO ONLY)", 52, 210);
	
  UI_DrawString("PAIRING WITH N65", 52, 332);
  UI_DrawString("EXIT", 52, 372);
	
	UI_Drawbox();
}

void UI_PairingWithN65(void)
{
	MenuBackground();
	UI_DrawString("PAIRING WITH N65", 180, 52);
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString23("THE NEONATE N65 UNIT USES THE UP-BUTTON", 52, 115);
  UI_DrawString23("ON THE BABY UNIT AS THE PAIR BUTTON.", 52, 140);
  UI_DrawString23("WHEN THE PAIR INSTRUCTIONS READS\"PAIR", 52, 165);
	UI_DrawString23("BUTTON\", PRESS THE\"UP BUTTON\" ON THE N65", 52, 190);
  UI_DrawString23("BABY UNIT.", 52, 215);
	
  UI_DrawString("EXIT", 52, 372);
	
	UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_Info(void)
{
	MenuBackground();

  UI_DrawString("INFO", 280, 52);

	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
  UI_DrawString("ABOUT THIS MODEL", 52, 118);
  UI_DrawString("FEATURES", 52, 158);
  UI_DrawString("CONTACT INFORMATION", 52, 198);
  UI_DrawString("EXIT", 52, 238);
	
	UI_Drawbox();
}
void UI_AboutThisModel(void)
{
	MenuBackground();

  UI_DrawString("ABOUT THIS MODEL", 170, 52);

	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString23("MODEL: NEONATE N80", 52, 115);
  UI_DrawString23("BATCH NUMBER: XXXX", 52, 140);
  UI_DrawString23("PRODUCTION DATE: XX.XX.XX", 52, 165);
	UI_DrawString23("NEONATE", 52, 195);
  UI_DrawString23("THE SCANDINAVIAN BABY MONITOR SYSTEM", 52, 220);
  UI_DrawString23("DESIGNED IN NORWAY", 52, 250);	
  UI_DrawString23("MADE IN CHINA", 52, 275);
	
  UI_DrawString("EXIT", 52, 372);
	
	UI_Drawbox();
}
void UI_ContactInformation(void)
{
	MenuBackground();

  UI_DrawString("CONTACT INFORMATION", 140, 52);

	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString23("IF YOU HAVE QUESTIONS ABOUT THIS BABY", 52, 115);
  UI_DrawString23("MONITOR YOU CAN CONTACT NEONATE ON", 52, 140);
  UI_DrawString23("FACEBOOK OR HROUGH OUR WEBPAGE.", 52, 165);
	UI_DrawString23("WWW.NEONATE.NO", 52, 200);
  UI_DrawString23("WWW.FACEBOOK.COM/NEONATEBABYMONITOR", 52, 235);

  UI_DrawString("EXIT", 52, 372);
	
	UI_Drawbox();
}

void UI_Features(void)
{
	MenuBackground();
	UI_DrawString("FEATURES", 235, 52);

	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString("RANGE AND TRANSMISSION", 52, 118);
	UI_DrawString("BATTERY", 52, 158);
	UI_DrawString("SPECIAL FEATURES", 52, 198);
	UI_DrawString("EXIT", 52, 238);
	
	UI_Drawbox();
}
void UI_RangeAndTransmission(void)
{
	MenuBackground();
	UI_DrawString("RANGE AND TRANSMISSION", 125, 52);
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString23("MODE:VIDEO AND AUDIO", 52, 115);
  UI_DrawString23("VIDEO RANGE: XXX METERS", 52, 140);
  UI_DrawString23("AUDIO RANGE: XXX METERS ", 52, 165);
	UI_DrawString23("TRANSMISSION TECHNOLOGY:", 52, 190);
  UI_DrawString23("SECURE DIGITAL TRANSMISSION.", 52, 215);

  UI_DrawString("EXIT", 52, 372);
	
	UI_Drawbox();
}
void UI_Battery(void)
{
	MenuBackground();
	UI_DrawString("BATTERY", 235, 52);
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString23("BATTERY PARENT UNIT:", 52, 115);
  UI_DrawString23("STAND-BY-TIME : MAX XXX HOURS", 52, 140);
  UI_DrawString23("TYPE : 1X XXXX RECHARGEABLE BATTERY", 52, 165);
	UI_DrawString23("CAPACITY : 5000 MAH", 52, 190);
  UI_DrawString23("BATTERY BABY UNIT:", 52, 225);
  UI_DrawString23("STAND-BY-TIME : MAX XXX HOURS", 52, 250);
	UI_DrawString23("TYPE :2X 18650 RECHARGEABLE BATTERY", 52, 275);
  UI_DrawString23("CAPACITY : 3500 MAH (TOTAL 7000 MAH)", 52, 300);
	
  UI_DrawString("EXIT", 52, 372);
	
	UI_Drawbox();
}
void UI_SpecialFeatures(void)
{
	MenuBackground();
  UI_DrawString("SPECIAL FEATURES", 170, 52);
	
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	UI_DrawString23("CONNECT 3 TO 1", 52, 115);
  UI_DrawString23("RANGE CONTROL", 52, 140);
  UI_DrawString23("OUT OF RANGE ALARM", 52, 165);
	UI_DrawString23("TEMPERATURE ALARM", 52, 190);
  UI_DrawString23("TALCK BACK FUNCTION", 52, 215);
  UI_DrawString23("READ MORE ABOUT THE FEATURES", 52, 250);
	UI_DrawString23("ON OUR WEB PAGE:", 52, 275);
  UI_DrawString23("WWW.NEONATE.NO", 52, 310);
	
  UI_DrawString("EXIT", 52, 372);
	
	UI_Drawbox();
}
//------------------------------------------------------------------------------
static uint8_t s_prev_connected_count = 0xFF; 
void UI_CalculateSleepTimerMenuPositions(uint8_t connected_count)
{
    uint16_t start_y = 110;
    uint8_t  show_all = (connected_count > 1);

    uint16_t header_lines = (show_all ? 1 : 0) + connected_count;
    start_y += (header_lines * 20) + 3;

    SLEEPTIMER_MENU_Y_POS[0] = start_y;          // ON/OFF
    SLEEPTIMER_MENU_Y_POS[1] = start_y + 40;     // STOP
    SLEEPTIMER_MENU_Y_POS[2] = start_y + 80;     // SLEEP HISTORY
    SLEEPTIMER_MENU_Y_POS[3] = start_y + 120;    // EXIT
}
//------------------------------------------------------------------------------
void UI_ShowSleepTimer(void)
{
    MenuBackground();
    UI_DrawString("SLEEP TIMER", 220, 52);

    OSD_IMG_INFO ln;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &ln);
    tOSD_Img2(&ln, OSD_QUEUE);

    uint8_t connected_count = 0;
    for (uint8_t i = 0; i < CAM_4T; i++)
		{
        if (ubTRX_GetLinkStatus(i)) connected_count++;
    }

    uint16_t y = 100;
    uint8_t show_all = (connected_count > 1);

    if (show_all) {
        UI_DrawString23("ALL", 52, y);
        y += 20;
    }

    uint32_t now_ms = KNL_TIMER_Get1ms();
    for (uint8_t i = 0; i < CAM_4T; i++)
		{
        if (!ubTRX_GetLinkStatus(i)) continue;

        UI_DrawString23(tUI_CamStatus[i].name, 72, y);
        UI_DrawString23("TIME", 320, y);

        uint32_t disp_ms = tUI_CamStatus[i].sleep_time_ms;
        if (g_sleeptimer_level == 0 && g_sleep_timer_start_ms[i] > 0)
				{
            disp_ms += (now_ms - g_sleep_timer_start_ms[i]);
        }
        char ts[10];
        uint32_t s = disp_ms / 1000;
        uint8_t hh = s / 3600, mm = (s % 3600) / 60, ss = s % 60;
        sprintf(ts, "%02u:%02u:%02u", hh, mm, ss);
        UI_DrawString23(ts, 380, y);

        y += 20;
    }

		if (connected_count > 0)
		{
				if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &ln) == OSD_OK) 
				{
						ln.uwYStart = y + 5;
						tOSD_Img2(&ln, OSD_QUEUE);
				}
		}

    UI_CalculateSleepTimerMenuPositions(connected_count);

		if (s_prev_connected_count != connected_count)
		{
				if (g_current_menu_level == MENU_LEVEL_SLEEP_TIMER)
				{
						if (g_current_menu_index > 3) g_current_menu_index = 0;
						g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
						g_rectangle_x = 40;
				}
				s_prev_connected_count = connected_count;
		}

    UI_DrawString("ON/OFF",         52, SLEEPTIMER_MENU_Y_POS[0] + 8);
    UI_DrawString("STOP",           52, SLEEPTIMER_MENU_Y_POS[1] + 8);
    UI_DrawString("SLEEP HISTORY",  52, SLEEPTIMER_MENU_Y_POS[2] + 8);
    UI_DrawString("EXIT",           52, SLEEPTIMER_MENU_Y_POS[3] + 8);

    UI_DrawHalfWhiteSquare(508, SLEEPTIMER_MENU_Y_POS[0]);
    UI_DrawReverseString23((g_sleeptimer_level == 0) ? "ON" : "OFF",
                           530, SLEEPTIMER_MENU_Y_POS[0] + 8);

		printf("ubPairedBuNum: %d\n", tUI_PuSetting.ubPairedBuNum);
    UI_Drawbox();
}
//------------------------------------------------------------------------------
static uint8_t UI_GetSleepHistorySelectItemCount(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < CAM_4T; i++)
    {
        if (ubTRX_GetLinkStatus(i))
            count++;
    }
    return count == 0 ? 1 : count + 1; // +1 for EXIT
}

void UI_SleepHistorySelect(void)
{
    MenuBackground();
    UI_DrawString("SLEEP HISTORY", 205, 52);

    OSD_IMG_INFO ln;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &ln);
    tOSD_Img2(&ln, OSD_QUEUE);

    uint8_t bu_count = 0;

    for (uint8_t i = 0; i < CAM_4T; i++)
    {
        if (ubTRX_GetLinkStatus(i))
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "BABY UNIT %d", i + 1);
            UI_DrawString(buf, 52, SLEEPHIS_BUSEL_Y_POS[bu_count]);
            bu_count++;
        }
    }

    UI_DrawString("EXIT", 52, SLEEPHIS_BUSEL_Y_POS[bu_count] + 3);

    if (g_current_menu_index > bu_count)
        g_current_menu_index = bu_count;

    g_rectangle_y = SLEEPHIS_BUSEL_Y_POS[g_current_menu_index];
    UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_SleepHistoryMain(void)
{
    MenuBackground();
    UI_DrawString("SLEEP HISTORY", 205, 52);

    OSD_IMG_INFO ln;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &ln);
    tOSD_Img2(&ln, OSD_QUEUE);

    UI_DrawString("VIEW SLEEP HISTORY", 52, SLEEPHIS_MAIN_Y_POS[0] + 8);
    UI_DrawString("OPTIONS",            52, SLEEPHIS_MAIN_Y_POS[1] + 8);
    UI_DrawString("EXIT",               52, SLEEPHIS_MAIN_Y_POS[2] + 8);

    UI_Drawbox();
}

void UI_SleepHistoryView(void)
{
    MenuBackground();
    UI_DrawString("SLEEP HISTORY", 205, 52);

    OSD_IMG_INFO ln;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &ln);
    tOSD_Img2(&ln, OSD_QUEUE);

    extern UI_BUStatus_t tUI_CamStatus[];
    uint8_t unit = 0;

    for (int i = 0; i < CAM_4T; ++i) 
		{
        if (tUI_CamStatus[i].sleep_hist_count > 0) { unit = i; break; }
    }
		
    if (tUI_CamStatus[unit].sleep_hist_count == 0) 
		{
        for (int i = 0; i < CAM_4T; ++i) 
				{
            if (ubTRX_GetLinkStatus(i)) { unit = i; break; }
        }
    }

    char line[64];
    snprintf(line, sizeof(line), "NAME: %s", tUI_CamStatus[unit].name);
    UI_DrawString23(line, 52, 108);

    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &ln);
    ln.uwYStart = 160;
    tOSD_Img2(&ln, OSD_QUEUE);

    uint16_t y = 175;
    UI_BUStatus_t *st = &tUI_CamStatus[unit];
    uint8_t cnt = st->sleep_hist_count;

    if (cnt == 0) 
		{
        UI_DrawString23("NO SLEEP TIMES RECORDED", 52, 130);
    } 
		else 
		{
        UI_DrawString23("NEWEST SLEEP TIMES", 52, 130);
        uint8_t idx = st->sleep_hist_head;
        for (uint8_t i = 0; i < cnt && i < 7; ++i) 
				{
            uint32_t total = st->sleep_hist_ms[idx] / 1000;
            uint8_t h = total / 3600;
            uint8_t m = (total % 3600) / 60;
            uint8_t s = total % 60;
            snprintf(line, sizeof(line), "%02u.%02u.%02u%s",
                     h, m, s, (i == 0 ? "  NEW" : ""));
            UI_DrawString23(line, 52, y);
            y += 25;
            idx = (uint8_t)((idx + UI_SLEEP_HISTORY_DEPTH - 1) % UI_SLEEP_HISTORY_DEPTH);
        }
    }

    UI_DrawString("EXIT", 52, 378);
    UI_Drawbox();
}

void UI_SleepHistoryOptions(void)
{
    MenuBackground();
    UI_DrawString("SLEEP HISTORY", 205, 52);

    OSD_IMG_INFO ln;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &ln);
    tOSD_Img2(&ln, OSD_QUEUE);

    UI_DrawString("DELETE SLEEP HISTORY", 52, SLEEPHIS_OPT_Y_POS[0] + 8);
    UI_DrawString("EXIT",                 52, SLEEPHIS_OPT_Y_POS[1] + 8);

    UI_Drawbox();
}


//------------------------------------------------------------------------------
void UI_ShowMicroSensitivity(void)
{
    char value_str[4];
    MenuBackground();
    UI_DrawString("MICROPHONE SENSITIVITY", 120, 52);

		OSD_IMG_INFO tOsdImgInfoLine;
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
    tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
    uint16_t current_y = 118;
    uint8_t displayed_units = 0;
				
    for (uint8_t i = 0; i < CAM_4T; i++)
    {
				//uint32_t *Id = (uint32_t *)PAIR_GetId(i);
        if(ubTRX_GetLinkStatus(i))// *Id != PAIR_INVALID_ID && *Id != 0 && *Id != 0xFFFFFFFF
        {            
            sprintf(value_str, "BABY UNIT %d", i + 1);
            UI_DrawString(value_str, 52, current_y);

            sprintf(value_str, "%d", tUI_CamStatus[i].ubMicroSensitivity);
            UI_DrawString(value_str, 530, current_y);

            current_y += 40;
            displayed_units++;
        }
    }

    UI_Drawbox(); 
}
//------------------------------------------------------------------------------
void DisplaySettings(void)
{
	char value_str[10];
	
	OSD_IMG_INFO tInfor;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DISPLAYSETTINGSMENU, 1, &tInfor);
	tInfor.uwXStart = 26;
	tInfor.uwYStart = 26;
  tOSD_Img1(&tInfor, OSD_QUEUE);
	
	OSD_IMG_INFO tOsdImgInfoLine1;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine1);
	tOsdImgInfoLine1.uwHSize = 343;
  tOSD_Img2(&tOsdImgInfoLine1, OSD_QUEUE);
	
	UI_DrawString("DISPLAY SETTINGS", 42, 52);
  UI_DrawString("LCD BRIGHTNESS", 52, 118);
  UI_DrawString("DIGITAL ZOOM", 52, 158);
	UI_DrawString("FLIP IMAGE", 52, 198);
	UI_DrawString("ADVANCED", 52, 238);
	UI_DrawString("EXIT", 52, 278);
	
	//UI_DrawHalfWhiteSquare68(331, 110);
	//UI_DrawHalfWhiteSquare68(331, 150);
	//UI_DrawHalfWhiteSquare68(331, 190);
	
	sprintf(value_str, "%d", tUI_PuSetting.ubLcdBrightness);
  UI_DrawReverseString23(value_str, 340, 120);

  if (tUI_PuSetting.ubZoomLevel == 0)
	{
      UI_DrawReverseString23("OFF", 330, 160);
  }
	else
	{
      UI_DrawReverseString23("2X", 335, 160);
  }
  switch (tUI_PuSetting.ubFlipImage)
	{
      case 0: UI_DrawReverseString23("0""\xB0", 340, 200); break;
      case 1: UI_DrawReverseString23("90""\xB0", 335, 200); break;
      case 2: UI_DrawReverseString23("180""\xB0", 330, 200); break;
      case 3: UI_DrawReverseString23("270""\xB0", 330, 200); break;
  }
	
	UI_Drawbox341();
}
//------------------------------------------------------------------------------
void AdvancedDisplaySettings(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo);
	tOsdImgInfo.uwHSize = 588;
  tOsdImgInfo.uwVSize = 185;
  tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);

  UI_DrawString("ADVANCE DISPLAY SETTINGS", 80, 52);
  UI_DrawString("TRANSPARENT TEXT FIELD", 52, 118);
  UI_DrawString("EXIT", 52, 158);

	OSD_IMG_INFO tOsdImgInfoLine1;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine1);
  tOSD_Img2(&tOsdImgInfoLine1, OSD_QUEUE);
	
	OSD_IMG_INFO tOsdImgInfoLine2;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENUHLINE, 1, &tOsdImgInfoLine2);
	tOsdImgInfoLine2.uwXStart = 28;
	tOsdImgInfoLine2.uwYStart = 209;	
  tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);
	
	UI_Drawbox();
}

//------------------------------------------------------------------------------
void UI_CalculateBabyUnitSettingsMenuPositions(void)
{
    uint8_t bu_count = 0;
    for (uint8_t i = 0; i < CAM_4T; i++)
		{
        if (ubTRX_GetLinkStatus(i)) 
				{
            g_bu_idx_map[bu_count] = i;
            bu_count++;
        }
    }

    if (bu_count == 0) 
		{
        BABYUNITSETTINGS_MENU_ITEM_COUNT = 1;
    } 
		else
		{
        BABYUNITSETTINGS_MENU_ITEM_COUNT = bu_count + 2;
    }

    uint16_t y = 110;
    for (uint8_t i = 0; i < BABYUNITSETTINGS_MENU_ITEM_COUNT; i++) 
		{
        BABYUNITSETTINGS_MENU_Y_POS[i] = y;
        y += 40;
    }
		
    if (g_current_menu_index >= BABYUNITSETTINGS_MENU_ITEM_COUNT) 
		{
        g_current_menu_index = BABYUNITSETTINGS_MENU_ITEM_COUNT - 1;
    }

    uint8_t need_realign = (s_prev_buset_count != bu_count);
    if (need_realign && g_current_menu_level == MENU_BABYUNITSETTINGS)
		{
        g_rectangle_x = 40;
        g_rectangle_y = BABYUNITSETTINGS_MENU_Y_POS[g_current_menu_index];
    }
    s_prev_buset_count = bu_count;
}

void UI_ShowBabyUnitSettings(void)
{
    MenuBackground();
    UI_DrawString("BABY UNIT SETTINGS", 160, 52);
		printf("ulAp_ID: %d, ulSTA_ID[0]: %d, %d, %d, %d, %d\n", PAIR_IdTable.ulAp_ID, PAIR_IdTable.ulSTA_ID[0], PAIR_IdTable.ulSTA_ID[1],PAIR_IdTable.ulSTA_ID[2], PAIR_IdTable.ulSTA_ID[3]);
		printf("%d\n", PAIR_GetStaNumber());
		printf("zubpairedBuNum: %d, zubTotalBuNum: %d\n", tUI_PuSetting.ubPairedBuNum, tUI_PuSetting.ubTotalBuNum);
		printf("%d\n", tUI_CamStatus[0].ulCAM_ID);
    //printf("%s\n", tUI_CamStatus[0].tCamConnSts);
		PAIR_ShowDeviceID();
		printf("%X,%X,%X,%X", *((uint32_t *)PAIR_GetId(0)), *((uint32_t *)PAIR_GetId(1)), *((uint32_t *)PAIR_GetId(2)), *((uint32_t *)PAIR_GetId(3)));

		OSD_IMG_INFO tOsdImgInfoLine;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
		tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
		UI_CalculateBabyUnitSettingsMenuPositions();

    char line[32];
    uint8_t bu_count = (s_prev_buset_count == 0xFF) ? 0 : s_prev_buset_count; // ????????
    uint8_t displayed = 0;

    for (uint8_t k = 0; k < bu_count; k++) {
        uint8_t ch = g_bu_idx_map[k];
        uint16_t y = BABYUNITSETTINGS_MENU_Y_POS[k];

        snprintf(line, sizeof(line), "%.*s", 20, tUI_CamStatus[ch].name);
        UI_DrawString(line, 52, y + 8);

        snprintf(line, sizeof(line), "PRIORITY : %d", k + 1);
        UI_DrawString23(line, 390, y + 10);

        displayed++;
    }

    if (displayed > 0)
		{
        //CHANGE PRIORITY
        UI_DrawString("CHANGE PRIORITY", 52,
                      BABYUNITSETTINGS_MENU_Y_POS[displayed] + 8);
        //EXIT
        UI_DrawString("EXIT", 52,
                      BABYUNITSETTINGS_MENU_Y_POS[displayed + 1] + 8);
    } 
		else
		{
        //EXIT
        UI_DrawString("EXIT", 52,
                      BABYUNITSETTINGS_MENU_Y_POS[0] + 8);
    }

    UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_ShowBabyUnitDetail(uint8_t unit_index)
{
    MenuBackground();
    UI_DrawString("BABY UNIT SETTINGS", 160, 52);
	
		OSD_IMG_INFO tOsdImgInfoLine;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
		tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);

    uint16_t y = 180;
    char str[64];

    sprintf(str, "NAME : %.*s", 20, tUI_CamStatus[unit_index].name);
		//snprintf(str, sizeof(str), "NAME : %s", tUI_CamStatus[unit_index].name);
    UI_DrawString23(str, 52, 118);
	
		OSD_IMG_INFO tOsdImgInfoLine1;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine1);
		tOsdImgInfoLine1.uwXStart = 40;
		tOsdImgInfoLine1.uwYStart = 160;
		tOSD_Img2(&tOsdImgInfoLine1, OSD_QUEUE);
		
		UI_DrawWhiteSquare(425, 180);
    UI_DrawString("ACTIVE/INACTIVE", 52, y + 8);
    if(ubTRX_GetLinkStatus(unit_index))
		{
        UI_DrawReverseString23("ACTIVE", 455, y + 10);y += 40;
				UI_DrawString("CHANGE NAME", 52, y + 8); y += 40;
				UI_DrawString("INFO", 52, y + 8); y += 40;
				UI_DrawString("EXIT", 52, y + 8);
    }
		else
		{
        UI_DrawReverseString23("INACTIVE", 455, y + 10);y += 40;
				UI_DrawString("EXIT", 52, y + 8);
    }

		if(g_current_menu_index == 0) 
			{
        if(ubTRX_GetLinkStatus(unit_index))
				{
            OSD_IMG_INFO tOsdImgInfoLine2;
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine2);
						tOsdImgInfoLine2.uwXStart = 40;
						tOsdImgInfoLine2.uwYStart = 370;
						tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);
						UI_DrawString23("IF YOU ONLY WANT TO USE 1 BABY UNIT,", 52, 380);
						UI_DrawString23("YOU CAN SET THIS BABY UNIT AS INACTIVE,", 52, 405);
						UI_DrawString23("BUT YOU HAVE TO TURN IT OFF FIRST.", 52, 430);
        }
				else 
				{
            OSD_IMG_INFO tOsdImgInfoLine3;
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine3);
						tOsdImgInfoLine3.uwXStart = 40;
						tOsdImgInfoLine3.uwYStart = 395;
						tOSD_Img2(&tOsdImgInfoLine3, OSD_QUEUE);
						UI_DrawString23("THIS BABY UNIT IS NOW INACTIVE.", 52, 405);
						UI_DrawString23("TURN THE BABY UNIT ON TO ACTVATE.", 52, 430);
        }
			}
    UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_ShowBabyUnitInfo(void)
{
	char temp_str[32];
	MenuBackground();
  UI_DrawString("BABY UNIT INFO", 185, 52);
	
	OSD_IMG_INFO tOsdImgInfoLine;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
	tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
	sprintf(temp_str, "NAME:%s", g_edit_name_buffer);
  UI_DrawString23(temp_str, 52, 110);
  UI_DrawString23("MODEL:N80BU", 52, 135);
  UI_DrawString23("COMPATIBLE WITH:N80", 52, 160);

	OSD_IMG_INFO tOsdImgInfoLine2;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine2);
	tOsdImgInfoLine2.uwYStart = 190;
	tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);
	
  UI_DrawString("EXIT", 52, 215);
	
	UI_Drawbox();
}

//------------------------------------------------------------------------------
uint8_t UI_GetActualUnitIndexFromMenuIndex(uint8_t menu_index)
{
    uint8_t current_item = 0;
    for (uint8_t i = 0; i < CAM_4T; i++)
    {
				//uint32_t *Id = (uint32_t *)PAIR_GetId(i);
        if(ubTRX_GetLinkStatus(i)) // *Id != PAIR_INVALID_ID && *Id != 0 && *Id != 0xFFFFFFFF
        {
            if (current_item == menu_index)
            {
                return i;
            }
            current_item++;
        }
    }
    return 0xFF;
}
//------------------------------------------------------------------------------
void UI_ShowChangeName(void)
{
		char temp_str[32];
		MenuBackground();
		UI_DrawString("CHANGE NAME", 200, 52);
	
		OSD_IMG_INFO tOsdImgInfoLine;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
		tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);

		sprintf(temp_str, "NEW NAME : %s", g_edit_name_buffer);
    UI_DrawString23(temp_str, 52, 115);

		OSD_IMG_INFO tOsdImgInfoLine1;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine1);
		tOsdImgInfoLine1.uwXStart = 40;
		tOsdImgInfoLine1.uwYStart = 145;
		tOSD_Img2(&tOsdImgInfoLine1, OSD_QUEUE);
		OSD_IMG_INFO tOsdImgInfoLine2;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine2);
		tOsdImgInfoLine2.uwXStart = 40;
		tOsdImgInfoLine2.uwYStart = 295;
		tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);
		
		uint16_t x = 52;
    uint16_t y = 175;
    for(int col = 0; col < KEYBOARD_ROW_0_COLS; col++)
    {
        UI_DrawString23(ENGLISH_KEYBOARD[0][col], x, y);
        x += 40;
    }
    
    x = 52;
    y += 40;
    for(int col = 0; col < KEYBOARD_ROW_1_COLS; col++)
    {
        UI_DrawString23(ENGLISH_KEYBOARD[1][col], x, y);
        x += 40;
    }
    
    x = 52;
    y += 40;
    for(int col = 0; col < 4; col++)
    {
        UI_DrawString23(ENGLISH_KEYBOARD[2][col], x, y);
        x += 40;
    }
		
    for(int col = 0; col < 2; col++)
    {
        UI_DrawString23(ENGLISH_KEYBOARD[2][col], x, y);
        x += 100;
    }
		
		UI_DrawString23("SET AS NEW NAME", 52, 315);
		UI_DrawString23("EXIT", 52, 355);
		
		if (g_current_menu_index < 30) 
		{
				UI_Drawbox40();
		}
		else if (g_current_menu_index == 30 || g_current_menu_index == 31) 
		{
				UI_Drawhalfbox();
		}
		else 
		{
				UI_Drawbox();
		}
}
//------------------------------------------------------------------------------	
void TemperatureAlarm(void)
{
		MenuBackground();
		UI_DrawString("TEMPERATURE ALARM", 150, 52);
	  UI_DrawString("ON/OFF", 52, 118);
		char temp_str[8];
	
		OSD_IMG_INFO tOsdImgInfoLine;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine);
		tOSD_Img2(&tOsdImgInfoLine, OSD_QUEUE);
	
		UI_DrawHalfWhiteSquare(508, 110);
    if (tUI_PuSetting.ubTempAlarmOn) 
		{
        UI_DrawReverseString23("ON", 537, 120);
    } 
		else 
		{
        UI_DrawReverseString23("OFF", 530, 120);
    }
		
		if(tUI_PuSetting.ubTempAlarmOn)
		{
				UI_DrawString("TEMP LIMIT HIGH", 52, 158);
				UI_DrawHalfWhiteSquare(508, 150);
				sprintf(temp_str, "%d" "\xB0" "C", tUI_PuSetting.ubTempMax);
        UI_DrawReverseString23(temp_str, 530, 158);
			
				UI_DrawString("TEMP LIMIT LOW", 52, 198);
				UI_DrawHalfWhiteSquare(508, 190);
				sprintf(temp_str, "%d" "\xB0" "C", tUI_PuSetting.ubTempMin);
        UI_DrawReverseString23(temp_str, 530, 198);
		}
		
		uint16_t exit_y = tUI_PuSetting.ubTempAlarmOn ? 238 : 158;
		UI_DrawString("EXIT", 52, exit_y);
	
		UI_Drawbox();
}
//------------------------------------------------------------------------------
void UI_ShowSetLimitHigh(void)
{
    char temp_str[8];
    MenuBackground();
    UI_DrawString("SET LIMIT HIGH", 180, 52);

    OSD_IMG_INFO tOsdImgInfoLine1;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine1);
		tOSD_Img2(&tOsdImgInfoLine1, OSD_QUEUE);
		
		sprintf(temp_str, "ALARM LIMIT:%d" "\xB0", tUI_PuSetting.ubTempMax);
		UI_DrawString23(temp_str, 55, 100);

		OSD_IMG_INFO tOsdImgInfoLine2;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine2);
		tOsdImgInfoLine2.uwYStart = 125;
		tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);
	
		osDelay(1);
	
    const int BATCH_SIZE = 4; // Process 4 items at a time
    int processed = 0;
    
    for (int batch = 0; batch < (TEMP_HIGH_MENU_ITEM_COUNT - 1 + BATCH_SIZE - 1) / BATCH_SIZE; batch++) {
        int start = batch * BATCH_SIZE;
        int end = start + BATCH_SIZE;
        if (end > TEMP_HIGH_MENU_ITEM_COUNT - 1) {
            end = TEMP_HIGH_MENU_ITEM_COUNT - 1;
        }
        
        for (int i = start; i < end; i++) {
            sprintf(temp_str, "%d" "\xB0" "C", TEMP_HIGH_VALUES[i]);
            UI_DrawString(temp_str, TEMP_HIGH_MENU_POS[i].x + 55, TEMP_HIGH_MENU_POS[i].y + 8);
        }
        
        if (batch < ((TEMP_HIGH_MENU_ITEM_COUNT - 1 + BATCH_SIZE - 1) / BATCH_SIZE) - 1) {
            osDelay(1);
        }
    }
    
    UI_DrawString("EXIT", 60, 270);

    g_rectangle_x = TEMP_HIGH_MENU_POS[g_current_menu_index].x;
    g_rectangle_y = TEMP_HIGH_MENU_POS[g_current_menu_index].y;
		
		osDelay(1);
    UI_Drawbox140();
}
//------------------------------------------------------------------------------
void UI_ShowSetLimitLow(void)
{
    char temp_str[8];
    MenuBackground();
    UI_DrawString("SET LIMIT LOW", 180, 52);

		OSD_IMG_INFO tOsdImgInfoLine1;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine1);
		tOSD_Img2(&tOsdImgInfoLine1, OSD_QUEUE);
	
		sprintf(temp_str, "ALARM LIMIT:%d" "\xB0", tUI_PuSetting.ubTempMin);
		UI_DrawString23(temp_str, 55, 100);
	
		OSD_IMG_INFO tOsdImgInfoLine2;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OSD2HORIZONTALLINE, 1, &tOsdImgInfoLine2);
		tOsdImgInfoLine2.uwYStart = 125;
		tOSD_Img2(&tOsdImgInfoLine2, OSD_QUEUE);

		osDelay(1);
	
    const int BATCH_SIZE = 4; // Process 4 items at a time
    
    for (int batch = 0; batch < (TEMP_LOW_MENU_ITEM_COUNT - 1 + BATCH_SIZE - 1) / BATCH_SIZE; batch++) {
        int start = batch * BATCH_SIZE;
        int end = start + BATCH_SIZE;
        if (end > TEMP_LOW_MENU_ITEM_COUNT - 1) {
            end = TEMP_LOW_MENU_ITEM_COUNT - 1;
        }
        
        for (int i = start; i < end; i++) {
            sprintf(temp_str, "%d" "\xB0" "C", TEMP_LOW_VALUES[i]);
            UI_DrawString(temp_str, TEMP_LOW_MENU_POS[i].x + 55, TEMP_LOW_MENU_POS[i].y + 8);
        }
        
        // Yield to other tasks between batches
        if (batch < ((TEMP_LOW_MENU_ITEM_COUNT - 1 + BATCH_SIZE - 1) / BATCH_SIZE) - 1) {
            osDelay(1);
        }
    }
    
    UI_DrawString("EXIT", 60, 310);

    g_rectangle_x = TEMP_LOW_MENU_POS[g_current_menu_index].x;
    g_rectangle_y = TEMP_LOW_MENU_POS[g_current_menu_index].y;
		osDelay(1);
    UI_Drawbox140();
}
//------------------------------------------------------------------------------
void UI_TimerShow(void)
{
}
//------------------------------------------------------------------------------
void UI_InitEditNameBuffer(uint8_t unit_index)
{
    if (unit_index < 3)
    {
        strcpy(g_edit_name_buffer, tUI_CamStatus[unit_index].name);
				//snprintf(g_edit_name_buffer, sizeof(g_edit_name_buffer), "%s", tUI_CamStatus[unit_index].name);
        g_edit_name_len = strlen(g_edit_name_buffer);
        g_name_cursor_pos = g_edit_name_len;
    }
}

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
static void UI_Thread(void const *argument)
{
	static uint16_t uwUI_TaskCnt = 0;
	static uint32_t lastStatusUpdateTime = 0;
	printf(">>> UI_Thread has started! <<<\n");
	while(1)
	{
		    if ((KNL_TIMER_Get1ms() - lastStatusUpdateTime) >= 1000) 
        {
            lastStatusUpdateTime = KNL_TIMER_Get1ms();
            if (g_is_menu_visible == 0)
            {
                UI_UpdateStatus(&uwUI_TaskCnt);
            }
						printf("Runtime name for CAM 1: [%s]\n", tUI_CamStatus[CAM1].name); 
        }
		//UI_UpdateStatus(&uwUI_TaskCnt);		refresh 
		#ifdef VBM_PU
			//printf("UI_Thread:  AntLvl[%d]\n", ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_1_MAIN));
			//if (g_is_menu_visible == 0)
			//{
				//UI_UpdateStatus(&uwUI_TaskCnt);
			//}
		#endif
		
		osDelay(UI_TASK_PERIOD);

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
    uint16_t i = 0;
		uint8_t char_count = 0;

    for (i = 0; str[i] != '\0'; ++i)
    {
        unsigned char character = str[i];
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
						case '%': imageIndex = OSD2IMG_PERCENT31; break;
            case '-': imageIndex = OSD2IMG_NEGATIVE31; break;
						case '/': imageIndex = OSD2IMG_BACKSLASH31; break;
						case ':': imageIndex = OSD2IMG_COLON31; break;
						case 0xB0: imageIndex = OSD2IMG_TEMP31; break;
            default: continue; 
        }

        if (tOSD_GetOsdImgInfor(1, OSD_IMG2, imageIndex, 1, &tCharImgInfo) == OSD_OK)
        {
            tCharImgInfo.uwXStart = currentX;
            tCharImgInfo.uwYStart = startY;

            tOSD_Img2(&tCharImgInfo, OSD_QUEUE);

            currentX += tCharImgInfo.uwHSize;
					
						if (++char_count >= 15)
            {
                osDelay(1);
                char_count = 0;
            }
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
        unsigned char character = str[i];
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
						case ' ': imageIndex = OSD2IMG_SPACE3; break;
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
						case '%': imageIndex = OSD2IMG_PERCENTR23; break;
            case '-': imageIndex = OSD2IMG_NEGATIVER23; break;
						case '/': imageIndex = OSD2IMG_BACKSLASHR23; break;
						case ':': imageIndex = OSD2IMG_COLONR23; break;
						case 0xB0: imageIndex = OSD2IMG_TEMPR23; break;
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
		uint8_t char_count = 0;
	
    for (i = 0; str[i] != '\0'; ++i)
    {
        unsigned char character = str[i];
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
						case ' ': imageIndex = OSD2IMG_SPACE2; break;
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
            case '%': imageIndex = OSD2IMG_PERCENT23; break;
            case '-': imageIndex = OSD2IMG_NEGATIVE23; break;
            case '.': imageIndex = OSD2IMG_PERIOD23; break;
						case ',': imageIndex = OSD2IMG_COMMA23; break;
						case '?': imageIndex = OSD2IMG_QUESTION23; break;
						case '(': imageIndex = OSD2IMG_BRACKETLEFT23; break;
						case ')': imageIndex = OSD2IMG_BRACKETRIGHT23; break;
						case '/': imageIndex = OSD2IMG_BACKSLASH23; break;
						case ':': imageIndex = OSD2IMG_COLON23; break;
						case 0xB0: imageIndex = OSD2IMG_TEMP23; break;
						case 0xC6: imageIndex = OSD2IMG_AE23; break;
            case 0xD8: imageIndex = OSD2IMG_O_SLASH23; break;
            case 0xC5: imageIndex = OSD2IMG_A_RING23;   break;
            default: continue; 
        }

        if (tOSD_GetOsdImgInfor(1, OSD_IMG2, imageIndex, 1, &tCharImgInfo) == OSD_OK)
        {
            tCharImgInfo.uwXStart = currentX;
            tCharImgInfo.uwYStart = startY;

            tOSD_Img2(&tCharImgInfo, OSD_QUEUE);

            currentX += tCharImgInfo.uwHSize; 
						
						if (++char_count >= 15)
            {
                osDelay(1);
                char_count = 0;
            }
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
void UI_Drawbox40(void)
{
		OSD_IMG_INFO horizontal_line_info;
    OSD_IMG_INFO vertical_line_info;

    const uint16_t BOX_WIDTH = 40;
    const uint16_t BOX_HEIGHT = 40;

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
    uint16_t horizontal_thickness = horizontal_line_info.uwVSize;
    uint16_t vertical_thickness = vertical_line_info.uwHSize;

    horizontal_line_info.uwHSize = BOX_WIDTH;
    horizontal_line_info.uwXStart = g_rectangle_x;
    horizontal_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    horizontal_line_info.uwYStart = g_rectangle_y + 38;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    vertical_line_info.uwVSize = BOX_HEIGHT;
    vertical_line_info.uwXStart = g_rectangle_x;
    vertical_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&vertical_line_info, OSD_QUEUE);

    vertical_line_info.uwXStart = g_rectangle_x + BOX_WIDTH - vertical_thickness;
    tOSD_Img2(&vertical_line_info, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_Drawbox140(void)
{
    OSD_IMG_INFO horizontal_line_info;
    OSD_IMG_INFO vertical_line_info;

    const uint16_t box_width = 140;
    const uint16_t box_height = 40;

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

    horizontal_line_info.uwHSize = box_width;
    horizontal_line_info.uwXStart = g_rectangle_x;
    horizontal_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    horizontal_line_info.uwYStart = g_rectangle_y + 38;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    vertical_line_info.uwVSize = box_height;
    vertical_line_info.uwXStart = g_rectangle_x;
    vertical_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&vertical_line_info, OSD_QUEUE);

    vertical_line_info.uwXStart = g_rectangle_x + box_width - vertical_line_info.uwHSize;
    tOSD_Img2(&vertical_line_info, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_Drawbox341(void)
{
    OSD_IMG_INFO horizontal_line_info;
    OSD_IMG_INFO vertical_line_info;

    // --- Define the target dimensions for the new box ---
    const uint16_t target_width = 341;
    const uint16_t target_height = 40;

    // Fetch the image info for the line assets
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
    // --- Draw the four sides of the box with the new dimensions ---

    // 1. Draw the TOP line
    horizontal_line_info.uwHSize = target_width; // Set the width to our target
    horizontal_line_info.uwXStart = g_rectangle_x;
    horizontal_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    // 2. Draw the BOTTOM line
    horizontal_line_info.uwHSize = target_width; // Set the width again
    horizontal_line_info.uwXStart = g_rectangle_x;
    // Position it at the bottom edge
    horizontal_line_info.uwYStart = g_rectangle_y + target_height - horizontal_line_info.uwVSize;
    tOSD_Img2(&horizontal_line_info, OSD_QUEUE);

    // 3. Draw the LEFT line
    vertical_line_info.uwVSize = target_height; // Set the height to our target
    vertical_line_info.uwXStart = g_rectangle_x;
    vertical_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&vertical_line_info, OSD_QUEUE);

    // 4. Draw the RIGHT line and trigger the screen update
    vertical_line_info.uwVSize = target_height; // Set the height again
    // Position it at the right edge
    vertical_line_info.uwXStart = g_rectangle_x + target_width - vertical_line_info.uwHSize;
    vertical_line_info.uwYStart = g_rectangle_y;
    tOSD_Img2(&vertical_line_info, OSD_UPDATE); // Use OSD_UPDATE on the last call
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
void UI_DrawHalfWhiteSquare68(uint16_t x, uint16_t y)
{
    OSD_IMG_INFO tOsdImgInfoblank;

    if (tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_WHITESQUARE, 1, &tOsdImgInfoblank) != OSD_OK)
    {
        printf("Error: Failed to load WHITE SQUARE image\n");
        return;
    }

    tOsdImgInfoblank.uwXStart = x;
    tOsdImgInfoblank.uwYStart = y;

    tOsdImgInfoblank.uwHSize = 68;

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

			case 2: // ZOOM
         g_current_menu_index = (g_current_menu_index + 1) % ZOOM_MENU_ITEM_COUNT;
         g_rectangle_y = ZOOM_MENU_Y_POS[g_current_menu_index];
         break;
			
			case 3: // Language
         g_current_menu_index = (g_current_menu_index + 1) % LANGUAGE_MENU_ITEM_COUNT;
         break;

			case 4: // Settings
         g_current_menu_index = (g_current_menu_index + 1) % SETTINGS_MENU_ITEM_COUNT;
         g_rectangle_y = SETTINGS_MENU_Y_POS[g_current_menu_index];
         break;

			case 5: // Pair Units
         UI_UpdatePairUnitsMenuPositions();
         g_current_menu_index = (g_current_menu_index + 1) % PAIRUNITS_MENU_ITEM_COUNT;
         g_rectangle_y = PAIRUNITS_MENU_Y_POS[g_current_menu_index];
        break;
			
			case 6:
         g_current_menu_index = (g_current_menu_index + 1) % INFO_MENU_ITEM_COUNT;
         g_rectangle_y = INFO_MENU_Y_POS[g_current_menu_index];
         break;
			
			case MENU_LEVEL_SLEEP_TIMER:
				 g_current_menu_index = (g_current_menu_index + 1) % SLEEPTIMER_MENU_ITEM_COUNT;
				 g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
					break;
			
			case MENU_MICROPHONE_SENSITIVITY:
				 if (tUI_PuSetting.ubPairedBuNum == 0) break;
				 g_current_menu_index = (g_current_menu_index + 1) % tUI_PuSetting.ubPairedBuNum;
				 g_rectangle_y = 110 + (g_current_menu_index * 40);
					break;
			
      case MENU_SLEEP_HISTORY_SELECT:
          uint8_t max = UI_GetSleepHistorySelectItemCount();
          g_current_menu_index = (g_current_menu_index + 1) % max;
          g_rectangle_y = SLEEPHIS_BUSEL_Y_POS[g_current_menu_index];
          break;
			
			case MENU_SLEEP_HISTORY:
					g_current_menu_index = (g_current_menu_index + 1) % SLEEPHIS_MAIN_ITEM_COUNT;
					g_rectangle_y = SLEEPHIS_MAIN_Y_POS[g_current_menu_index];
					break;
			
			case MENU_SLEEP_HISTORY_OPTIONS:
					g_current_menu_index = (g_current_menu_index + 1) % SLEEPHIS_OPT_ITEM_COUNT;
					g_rectangle_y = SLEEPHIS_OPT_Y_POS[g_current_menu_index];
					break;
			
			case MENU_SLEEP_HISTORY_VIEW:
					g_current_menu_index = 0;
					g_rectangle_y = 372;
					break;
			
			case MENU_DISPLAYSETTINGS:
				 g_current_menu_index = (g_current_menu_index + DISPLAYSETTINGS_MENU_ITEM_COUNT + 1) % DISPLAYSETTINGS_MENU_ITEM_COUNT;
				 g_rectangle_y = DISPLAYSETTINGS_MENU_Y_POS[g_current_menu_index];
				 break;
		
			case MENU_ADVANCEDDISPLAYSETTINGS:
				 g_current_menu_index = (g_current_menu_index + ADVANCEDDISPLAYSETTINGS_MENU_ITEM_COUNT + 1) % ADVANCEDDISPLAYSETTINGS_MENU_ITEM_COUNT;
				 g_rectangle_y = ADVANCEDDISPLAYSETTINGS_MENU_Y_POS[g_current_menu_index];
				 break;
		
			case MENU_BABYUNITSETTINGS:
				 g_current_menu_index = (g_current_menu_index + 1) % BABYUNITSETTINGS_MENU_ITEM_COUNT;
				 g_rectangle_y = BABYUNITSETTINGS_MENU_Y_POS[g_current_menu_index];
				 break;
			
			case MENU_BABYUNITDETAIL:
				{
            uint8_t item_count = ubTRX_GetLinkStatus(g_selected_bu_index) ? 4 : 2;
            g_current_menu_index = (g_current_menu_index + 1) % item_count;
            g_rectangle_y = 180 + (g_current_menu_index * 40);
            break;
				}
				 
			case MENU_CHANGE_NAME:
        {
            g_current_menu_index = (g_current_menu_index + 1) % TOTAL_MENU_ITEMS;
            
            if (g_current_menu_index < KEYBOARD_ROW_0_COLS) // 1
            {
                g_rectangle_x = 40 + g_current_menu_index * 40;
                g_rectangle_y = 160;
            }
            else if(g_current_menu_index < KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS) // 2
            {
                g_rectangle_x = 40 + (g_current_menu_index - KEYBOARD_ROW_0_COLS) * 40;
                g_rectangle_y = 200;
            }
						else if(g_current_menu_index < KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS + 4)
						{
								g_rectangle_x = 40 + (g_current_menu_index - (KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS)) * 40;
                g_rectangle_y = 240;
						}
            else if(g_current_menu_index < TOTAL_KEYBOARD_KEYS) // (SPACE DELETE)
            {
                g_rectangle_x = 40*5 + (g_current_menu_index - (KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS + 4)) * 100;
                g_rectangle_y = 240;
            }
            else if(g_current_menu_index == TOTAL_KEYBOARD_KEYS) // SET AS NEW NAME
            {
                g_rectangle_x = 40;
                g_rectangle_y = 305;
            }
            else if (g_current_menu_index == TOTAL_KEYBOARD_KEYS + 1) // EXIT
            {
                g_rectangle_x = 40;
                g_rectangle_y = 345;
            }
            break;
        }
		
		case MENU_LEVEL_TEMP_ALARM:
		{
				uint8_t item_count = tUI_PuSetting.ubTempAlarmOn ? 4 : 2;
				g_current_menu_index = (g_current_menu_index + 1) % item_count;
				g_rectangle_y = 110 + (g_current_menu_index * 40);
				break;
		}
		
    case MENU_LEVEL_SET_TEMP_HIGH:
        g_current_menu_index = (g_current_menu_index + 1) % TEMP_HIGH_MENU_ITEM_COUNT;
        break;
        
    case MENU_LEVEL_SET_TEMP_LOW:
        g_current_menu_index = (g_current_menu_index + 1) % TEMP_LOW_MENU_ITEM_COUNT;
        break;

    case MENU_PAIR_NEW_BABY_UNIT:
        g_current_menu_index = (g_current_menu_index + 1) % 2;
        g_rectangle_y = PAIRUNITS_MENU_Y_POS[g_current_menu_index];
        break;
		
		case MENU_PAIRING_INFO:
        g_current_menu_index = (g_current_menu_index + 1) % PAIRINGINFO_MENU_ITEM_COUNT;
        g_rectangle_y = PAIRINGINFO_MENU_Y_POS[g_current_menu_index];
        break;
		
		case MENU_COMPATIBLE_UNITS:
        g_current_menu_index = (g_current_menu_index + 1) % COMPATIBLEUNITS_MENU_ITEM_COUNT;
        g_rectangle_y = COMPATIBLEUNITS_MENU_Y_POS[g_current_menu_index];
        break;
		
		case MENU_FEATURES:
        g_current_menu_index = (g_current_menu_index + 1) % FEATURES_MENU_ITEM_COUNT;
        g_rectangle_y = FEATURES_MENU_Y_POS[g_current_menu_index];
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
		 
		 case 2:
         g_current_menu_index = (g_current_menu_index + ZOOM_MENU_ITEM_COUNT - 1) % ZOOM_MENU_ITEM_COUNT;
         g_rectangle_y = ZOOM_MENU_Y_POS[g_current_menu_index];
         break;
    
     case 3: 
         g_current_menu_index = (g_current_menu_index + LANGUAGE_MENU_ITEM_COUNT - 1) % LANGUAGE_MENU_ITEM_COUNT;
         break;
    
     case 4:
         g_current_menu_index = (g_current_menu_index + SETTINGS_MENU_ITEM_COUNT - 1) % SETTINGS_MENU_ITEM_COUNT;
         g_rectangle_y = SETTINGS_MENU_Y_POS[g_current_menu_index];
         break;
		 
     case 5:
				 UI_UpdatePairUnitsMenuPositions();
         g_current_menu_index = (g_current_menu_index + PAIRUNITS_MENU_ITEM_COUNT - 1) % PAIRUNITS_MENU_ITEM_COUNT;
         g_rectangle_y = PAIRUNITS_MENU_Y_POS[g_current_menu_index];
        break;
		 
     case 6:
         g_current_menu_index = (g_current_menu_index + INFO_MENU_ITEM_COUNT - 1) % INFO_MENU_ITEM_COUNT;
         g_rectangle_y = INFO_MENU_Y_POS[g_current_menu_index];
				 break;
    
		 case MENU_LEVEL_SLEEP_TIMER:
				 g_current_menu_index = (g_current_menu_index + SLEEPTIMER_MENU_ITEM_COUNT - 1) % SLEEPTIMER_MENU_ITEM_COUNT;
				 g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
				 break;
		 
		 case MENU_MICROPHONE_SENSITIVITY:
			 if (tUI_PuSetting.ubPairedBuNum == 0) break;
					g_current_menu_index = (g_current_menu_index + tUI_PuSetting.ubPairedBuNum - 1) % tUI_PuSetting.ubPairedBuNum;
					g_rectangle_y = 110 + (g_current_menu_index * 40);
					break;
		 
      case MENU_SLEEP_HISTORY_SELECT:
          uint8_t max = UI_GetSleepHistorySelectItemCount();
          g_current_menu_index = (g_current_menu_index + max - 1) % max;
          g_rectangle_y = SLEEPHIS_BUSEL_Y_POS[g_current_menu_index];
          break;
			
			case MENU_SLEEP_HISTORY:
					g_current_menu_index = (g_current_menu_index + SLEEPHIS_MAIN_ITEM_COUNT - 1) % SLEEPHIS_MAIN_ITEM_COUNT;
					g_rectangle_y = SLEEPHIS_MAIN_Y_POS[g_current_menu_index];
					break;
			
			case MENU_SLEEP_HISTORY_OPTIONS:
					g_current_menu_index = (g_current_menu_index + SLEEPHIS_OPT_ITEM_COUNT - 1) % SLEEPHIS_OPT_ITEM_COUNT;
					g_rectangle_y = SLEEPHIS_OPT_Y_POS[g_current_menu_index];
					break;
			
			case MENU_SLEEP_HISTORY_VIEW:
					g_current_menu_index = 0;
					g_rectangle_y = 372;
					break;
		
		case MENU_DISPLAYSETTINGS:
				 g_current_menu_index = (g_current_menu_index + DISPLAYSETTINGS_MENU_ITEM_COUNT - 1) % DISPLAYSETTINGS_MENU_ITEM_COUNT;
				 g_rectangle_y = DISPLAYSETTINGS_MENU_Y_POS[g_current_menu_index];
				 break;
		
		case MENU_ADVANCEDDISPLAYSETTINGS:
				 g_current_menu_index = (g_current_menu_index + ADVANCEDDISPLAYSETTINGS_MENU_ITEM_COUNT - 1) % ADVANCEDDISPLAYSETTINGS_MENU_ITEM_COUNT;
				 g_rectangle_y = ADVANCEDDISPLAYSETTINGS_MENU_Y_POS[g_current_menu_index];
				 break;
		
		case MENU_BABYUNITSETTINGS:
				 g_current_menu_index = (g_current_menu_index + BABYUNITSETTINGS_MENU_ITEM_COUNT - 1) % BABYUNITSETTINGS_MENU_ITEM_COUNT;
				 g_rectangle_y = BABYUNITSETTINGS_MENU_Y_POS[g_current_menu_index];
				 break;
		
		case MENU_BABYUNITDETAIL:
        {
            uint8_t item_count = ubTRX_GetLinkStatus(g_selected_bu_index) ? 4 : 2;
            g_current_menu_index = (g_current_menu_index + item_count - 1) % item_count;
            g_rectangle_y = 180 + (g_current_menu_index * 40);
            break;
        }
				
		case MENU_CHANGE_NAME:
        {
            g_current_menu_index = (g_current_menu_index + TOTAL_MENU_ITEMS - 1) % TOTAL_MENU_ITEMS;
            
            if (g_current_menu_index < KEYBOARD_ROW_0_COLS) //1
            {
                g_rectangle_x = 40 + g_current_menu_index * 40;
                g_rectangle_y = 160;
            }
            else if (g_current_menu_index < KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS) //2
            {
                g_rectangle_x = 40 + (g_current_menu_index - KEYBOARD_ROW_0_COLS) * 40;
                g_rectangle_y = 200;
            }
            else if (g_current_menu_index < KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS + 4) //3
            {
                g_rectangle_x = 40 + (g_current_menu_index - (KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS)) * 40;
                g_rectangle_y = 240;
            }
            else if(g_current_menu_index < TOTAL_KEYBOARD_KEYS) // (SPACE DELETE)
            {
                g_rectangle_x = 40*5 + (g_current_menu_index - (KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS + 4)) * 100;
                g_rectangle_y = 240;
            }
            else if (g_current_menu_index == TOTAL_KEYBOARD_KEYS) //SET AS NEW NAME
            {
                g_rectangle_x = 40;
                g_rectangle_y = 305;
            }
            else if (g_current_menu_index == TOTAL_KEYBOARD_KEYS - 1) //EXIT
            {
                g_rectangle_x = 40;
                g_rectangle_y = 345;
            }
            break;
        }
				
		case MENU_LEVEL_TEMP_ALARM:
		{
				uint8_t item_count = tUI_PuSetting.ubTempAlarmOn ? 4 : 2;
				g_current_menu_index = (g_current_menu_index + item_count - 1) % item_count;
				g_rectangle_y = 110 + (g_current_menu_index * 40);
				break;
		}
		case MENU_LEVEL_SET_TEMP_HIGH:
				g_current_menu_index = (g_current_menu_index + TEMP_HIGH_MENU_ITEM_COUNT - 1) % TEMP_HIGH_MENU_ITEM_COUNT;
				break;

		case MENU_LEVEL_SET_TEMP_LOW:
				g_current_menu_index = (g_current_menu_index + TEMP_LOW_MENU_ITEM_COUNT - 1) % TEMP_LOW_MENU_ITEM_COUNT;
				break;
		
    case MENU_PAIR_NEW_BABY_UNIT:
        g_current_menu_index = (g_current_menu_index + 1) % 2;
        g_rectangle_y = PAIRUNITS_MENU_Y_POS[g_current_menu_index];
        break;
		
		case MENU_PAIRING_INFO:
        g_current_menu_index = (g_current_menu_index + PAIRINGINFO_MENU_ITEM_COUNT - 1) % PAIRINGINFO_MENU_ITEM_COUNT;
        g_rectangle_y = PAIRINGINFO_MENU_Y_POS[g_current_menu_index];
        break;
		
		case MENU_COMPATIBLE_UNITS:
        g_current_menu_index = (g_current_menu_index + COMPATIBLEUNITS_MENU_ITEM_COUNT - 1) % COMPATIBLEUNITS_MENU_ITEM_COUNT;
        g_rectangle_y = COMPATIBLEUNITS_MENU_Y_POS[g_current_menu_index];
        break;
		
		case MENU_FEATURES:
        g_current_menu_index = (g_current_menu_index + FEATURES_MENU_ITEM_COUNT - 1) % FEATURES_MENU_ITEM_COUNT;
        g_rectangle_y = FEATURES_MENU_Y_POS[g_current_menu_index];
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
                    case 5: // PAIR UNITS
                    {
                        uint8_t connected_count = 0;
                        for(uint8_t i = 0; i < 3; i++) 
                        {
                            if(ubTRX_GetLinkStatus(i))// g_baby_units[i].is_connected
                            {
                                connected_count++;
                            }
                        }
                        /*
                        if(connected_count == 0) 
                        {
                            g_rectangle_y = 127;
                        }
                        else if(connected_count == 1) 
                        {
                            g_rectangle_y = 127 + 20;
                        }
                        else if(connected_count == 2) 
                        {
                            g_rectangle_y = 127 + 40;
                        }
                        else
                        {
                            g_rectangle_y = 127 + 60;
                        }*/
												UI_UpdatePairUnitsMenuPositions();
												g_rectangle_y = PAIRUNITS_MENU_Y_POS[0];
                    }
                    break;
										case 6: g_rectangle_y = INFO_MENU_Y_POS[0]; break;
                }
            }
            break;

        case 1:  //KeyLock
            switch (g_current_menu_index)
            {
                case 0: // ACTIVATE KEY LOCK
                    // TODO Í£Ö¹°´¼üÊ¹ÓÃ
                    break;
                
                case 1: // AUTO ACTIVATE
                    tUI_PuSetting.ubKeyLockAutoActivate = (tUI_PuSetting.ubKeyLockAutoActivate + 1) % 3;
                    g_settings_changed = 1;
                    break;

                case 2:
                    MenuExitHandler();
                    break;
            }
            break;
						
				case 2:  //ZOOM
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
						
				case 3:		//LANGUAGE
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
						
        case 4: //SETTINGS MENU
             switch(g_current_menu_index)
            {
                case 0: // MICROPHONE SENSITIVITY
                    if(tUI_PuSetting.ubPairedBuNum <= 1)
										{
												tUI_CamStatus[CAM1].ubMicroSensitivity++;
												if(tUI_CamStatus[CAM1].ubMicroSensitivity > 5) 
												{
														tUI_CamStatus[CAM1].ubMicroSensitivity = 1;
												}
												g_settings_changed = 1;
										}
										else
										{
												g_current_menu_level = MENU_MICROPHONE_SENSITIVITY;
												g_current_menu_index = 0;
												g_rectangle_x = 40;
												g_rectangle_y = 110; 
										}
                    break;
                    
                case 1: // VOLUME
                    //0-6
                    tUI_PuSetting.VolLvL.tVOL_UpdateLvL++;
                    if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL > 6) 
										{
                        tUI_PuSetting.VolLvL.tVOL_UpdateLvL = 0;
                    }
										g_settings_changed = 1;
										
										//change underlying volume todotodo;
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
								    uint8_t connected_count = 0;
										for (uint8_t i = 0; i < CAM_4T; ++i)
										{
											if (ubTRX_GetLinkStatus(i)) connected_count++;
										}
										UI_CalculateSleepTimerMenuPositions(connected_count);
										g_rectangle_x = 40;
										g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
                    break;
                    
                case 4: // DISPLAY SETTINGS
                    g_current_menu_level = MENU_DISPLAYSETTINGS;
										g_current_menu_index = 0;
										g_rectangle_x = 40;
										g_rectangle_y = DISPLAYSETTINGS_MENU_Y_POS[0];
                    break;
                    
                case 5: // BABY UNIT SETTINGS
                    g_current_menu_level = MENU_BABYUNITSETTINGS;
										g_current_menu_index = 0;
										UI_CalculateBabyUnitSettingsMenuPositions();
										g_rectangle_y = BABYUNITSETTINGS_MENU_Y_POS[0];
                    break;
                    
                case 6: // TEMPERATURE ALARM
										g_current_menu_level = MENU_LEVEL_TEMP_ALARM;
										g_current_menu_index = 0;
										g_rectangle_y = 110;
                    break;
                    
                case 7: // EXIT
                    MenuExitHandler();
                    break;
                    
                default:
                    break;
            }
            break;

        case 5: //PAIR
        { 
            uint8_t connected_count = 0;
            for(uint8_t i = 0; i < 3; i++) 
            {
                if(ubTRX_GetLinkStatus(i)) //g_baby_units[i].is_connected
                {
                    connected_count++;
                }
            }
						
            if(connected_count == 0) 
            {
                switch(g_current_menu_index) 
                {
                    case 0: /* connect new baby unit */
												UI_UpdatePairUnitsMenuPositions();
                        g_current_menu_level = MENU_PAIR_NEW_BABY_UNIT;
                        g_current_menu_index = 0;
                        g_rectangle_x = 40;
                        g_rectangle_y = PAIRUNITS_MENU_Y_POS[0];
												break;
                    case 1: //INFO
												g_current_menu_level = MENU_PAIRING_INFO;
                        g_current_menu_index = 0;
                        g_rectangle_y = PAIRINGINFO_MENU_Y_POS[0];
												break;
                    case 2: MenuExitHandler(); break;
                }
            }
            else if(connected_count == 1 || connected_count == 2)
            {
                switch(g_current_menu_index)
                {
                    case 0: /* connect new baby unit */ 
												UI_UpdatePairUnitsMenuPositions();
                        g_current_menu_level = MENU_PAIR_NEW_BABY_UNIT;
                        g_current_menu_index = 0;
                        g_rectangle_x = 40;
                        g_rectangle_y = PAIRUNITS_MENU_Y_POS[0];
												break;
                    case 1: /* TODO: unpair baby unit */ 
												break;
                    case 2: 
												g_current_menu_level = MENU_PAIRING_INFO;
                        g_current_menu_index = 0;
                        g_rectangle_y = PAIRINGINFO_MENU_Y_POS[0];
												break;
                    case 3: MenuExitHandler(); break;
                }
            }
            else // connected_count == 3
            {
                switch(g_current_menu_index) 
                {
                    case 0: /* TODO: unpair baby unit */ 
												break;
                    case 1: 
												g_current_menu_level = MENU_PAIRING_INFO;
                        g_current_menu_index = 0;
                        g_rectangle_y = PAIRINGINFO_MENU_Y_POS[0];
												break;
                    case 2: MenuExitHandler(); break;
                }
            }
            break; 
				}
				
				case 6:  //info
						switch(g_current_menu_index)
            {
                case 0:
										g_current_menu_level = MENU_ABOUT_THIS_MODEL;
										g_current_menu_index = 0;
										g_rectangle_y = 365;
                    break;
								case 1:
										g_current_menu_level = MENU_FEATURES;
										g_current_menu_index = 0;
										g_rectangle_y = FEATURES_MENU_Y_POS[g_current_menu_index];
                    break;
                case 2:
										g_current_menu_level = MENU_CONTACT_INFORMATION;
										g_current_menu_index = 0;
										g_rectangle_y = 365;
                    break;
                case 3:
                    MenuExitHandler();
                    break;  
								
								default:
                    break;
            }
						break;
						
				case MENU_LEVEL_SLEEP_TIMER:
						switch(g_current_menu_index)
            {
                case 0: // ON/OFF
                    g_sleeptimer_level = !g_sleeptimer_level;
										if (g_sleeptimer_level == 0) //ON
										{ 
												//start
												for (int i = 0; i < CAM_4T; i++) 
												{
														if (ubTRX_GetLinkStatus(i))
														{
																g_sleep_timer_start_ms[i] = KNL_TIMER_Get1ms();
														}
												}
										} 
										else //OFF
										{
												for (int i = 0; i < CAM_4T; i++) 
												{
														if (g_sleep_timer_start_ms[i] > 0) 
														{ // timer running
																uint32_t elapsed_ms = KNL_TIMER_Get1ms() - g_sleep_timer_start_ms[i];
																tUI_CamStatus[i].sleep_time_ms += elapsed_ms;
																//UI_SleepHist_Push(i, elapsed_ms);
																g_sleep_timer_start_ms[i] = 0;		//reset start time
														}
												}
												//g_settings_changed = 1; // save flag
										}
										break;
                    
                case 1: // STOP
										g_sleeptimer_level = 1; // OFF
										//stop and save timer
										for (int i = 0; i < CAM_4T; i++) 
										{
												if (g_sleep_timer_start_ms[i] > 0) 
												{
														uint32_t elapsed_ms = KNL_TIMER_Get1ms() - g_sleep_timer_start_ms[i];
														tUI_CamStatus[i].sleep_time_ms += elapsed_ms;
														//UI_SleepHist_Push(i, elapsed_ms);
														g_sleep_timer_start_ms[i] = 0;
												}
										}
										
										for (int i = 0; i < CAM_4T; i++) 
										{
												if (tUI_CamStatus[i].sleep_time_ms > 0) 
												{
														SleepHistory_Push(i, tUI_CamStatus[i].sleep_time_ms);
														tUI_CamStatus[i].sleep_time_ms = 0;
												}
										}
										
										g_settings_changed = 1;
                    break;
                    
                case 2: // SLEEP HISTORY
										g_current_menu_level = MENU_SLEEP_HISTORY_SELECT;
										g_current_menu_index = 0;
										g_rectangle_x = 40;
										g_rectangle_y = SLEEPHIS_MAIN_Y_POS[0];
										break;
                    
                case 3: // EXIT SETTINGS SLEEP TIMER
										MenuExitHandler();
                    break;
                    
                default:
                    break;
            }
            break;
						
				case MENU_SLEEP_HISTORY_SELECT:
						uint8_t bu_count = 0;
						for (uint8_t i = 0; i < CAM_4T; i++)
						{
								if (ubTRX_GetLinkStatus(i))
								{
										if (g_current_menu_index == bu_count)
										{
												g_hist_show_unit = i;
												g_current_menu_level = MENU_SLEEP_HISTORY;
												g_current_menu_index = 0;
												g_rectangle_x = 40;
												g_rectangle_y = SLEEPHIS_MAIN_Y_POS[0];
										}
										bu_count++;
								}
						}

						// EXIT
						if (g_current_menu_index == bu_count || bu_count == 0)
						{
								g_current_menu_level = MENU_LEVEL_SLEEP_TIMER;
								g_current_menu_index = 2;
								g_rectangle_x = 40;
								g_rectangle_y = SLEEPTIMER_MENU_Y_POS[2];
						}
						break;

						
				case MENU_SLEEP_HISTORY:
						if (g_current_menu_index == 0)
						{
							g_current_menu_level = MENU_SLEEP_HISTORY_VIEW;
							g_current_menu_index = 0;
							g_rectangle_x = 40;
							g_rectangle_y = 372;                  
						} 
						else if (g_current_menu_index == 1) 
						{
								g_current_menu_level = MENU_SLEEP_HISTORY_OPTIONS;
								g_current_menu_index = 0;
								g_rectangle_x = 40;
								g_rectangle_y = SLEEPHIS_OPT_Y_POS[0];
						} 
						else 
						{                                  
								g_current_menu_level = MENU_LEVEL_SLEEP_TIMER;
								g_current_menu_index = 2;
								g_rectangle_x = 40;
								g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
						}
						break;

				case MENU_SLEEP_HISTORY_VIEW:
						g_current_menu_level = MENU_SLEEP_HISTORY;
						g_current_menu_index = 0;                        
						g_rectangle_x = 40;
						g_rectangle_y = SLEEPHIS_MAIN_Y_POS[g_current_menu_index];
						break;

				case MENU_SLEEP_HISTORY_OPTIONS:
						if (g_current_menu_index == 0) 
						{
								for (int i = 0; i < CAM_4T; ++i) {
										UI_SleepHist_Clear(i);
								}
								g_settings_changed = 1; 
						} 
						else 
						{                               
								g_current_menu_level = MENU_SLEEP_HISTORY;
								g_current_menu_index = 1;
								g_rectangle_x = 40;
								g_rectangle_y = SLEEPHIS_MAIN_Y_POS[g_current_menu_index];
						}
						break;

				case MENU_DISPLAYSETTINGS:
						switch(g_current_menu_index)
						{
								case 0: // LCD BRIGHTNESS
										tUI_PuSetting.ubLcdBrightness++;
										if (tUI_PuSetting.ubLcdBrightness > 3) 
										{
												tUI_PuSetting.ubLcdBrightness = 1;
										}
										const uint32_t brightness_levels[] = { 0x230, 0x600, 0xA00 };
										LCD_BACKLIGHT_CTRL(brightness_levels[tUI_PuSetting.ubLcdBrightness - 1]);
										g_settings_changed = 1;
										break;

								case 1: // DIGITAL ZOOM
										tUI_PuSetting.ubZoomLevel = !tUI_PuSetting.ubZoomLevel;
										g_settings_changed = 1;
										break;

								case 2: // FLIP IMAGE
										tUI_PuSetting.ubFlipImage++;
										if (tUI_PuSetting.ubFlipImage > 3)
										{
												tUI_PuSetting.ubFlipImage = 0;
										}
										g_settings_changed = 1;
										break;

								case 3: // ADVANCED
										g_current_menu_level = MENU_ADVANCEDDISPLAYSETTINGS;
										g_current_menu_index = 0;
										g_rectangle_x = 40;
										g_rectangle_y = ADVANCEDDISPLAYSETTINGS_MENU_Y_POS[g_current_menu_index];
										break;

								case 4: // EXIT
										MenuExitHandler();
										break;

								default:
										break;
						}
						break;
						
				case MENU_ADVANCEDDISPLAYSETTINGS:
						switch(g_current_menu_index)
						{
								case 0:
            
										break;

								case 1: 
										MenuExitHandler();
										break;

								default:
										break;
						}
						break;
						
				case MENU_BABYUNITSETTINGS:
				{
						uint8_t bu_count = 0;
						for(uint8_t i = 0; i < CAM_4T; i++)
						{
							if(ubTRX_GetLinkStatus(i))
							{
								bu_count++;
							}
						}
						
						
            if (bu_count == 0)
            {
                if (g_current_menu_index == 0)
                {
                     MenuExitHandler();
                }
                break;
            }
						
						if(g_current_menu_index < bu_count)
						{
								uint8_t actual_unit_index = UI_GetActualUnitIndexFromMenuIndex(g_current_menu_index);
								if(actual_unit_index != 0XFF)
								{
									g_selected_bu_index = actual_unit_index;
									g_current_menu_level = MENU_BABYUNITDETAIL;
									g_current_menu_index = 0;
									g_rectangle_y = 180;
								}
						}
						else if(g_current_menu_index == bu_count) //PAIR_GetPairedDevCnt(),tUI_PuSetting.ubPairedBuNum
						{
								// CHANGE PRIORITY
							
						}
						else if(g_current_menu_index == bu_count + 1) //PAIR_GetPairedDevCnt(),tUI_PuSetting.ubPairedBuNum
						{
								MenuExitHandler();
						}
						break;
					}
					
					case MENU_BABYUNITDETAIL:
					{
							if(ubTRX_GetLinkStatus(g_selected_bu_index))
							{
									switch(g_current_menu_index)
									{
											case 0: // ACTIVE/INACTIVE
												//todo: break down connect
													/*ubTRX_GetLinkStatus(g_selected_bu_index) = 
                    					!g_baby_units[g_selected_bu_index].is_connected;*/
													break;

											case 1: // CHANGE NAME
													UI_InitEditNameBuffer(g_selected_bu_index);
                          g_current_menu_level = MENU_CHANGE_NAME;
                          g_current_menu_index = 0;
                          g_rectangle_x = 40;
                          g_rectangle_y = 160;
                          break;
          
                      case 2: // INFO
                          g_current_menu_level = MENU_BABY_UNIT_INFO;
													g_rectangle_x = 40;
                          g_rectangle_y = 208;
                          break;
          
                      case 3: // EXIT
                          MenuExitHandler();
                          break;
                  }
              }
              else
              {
                  if (g_current_menu_index == 1) 
									{
                      MenuExitHandler();
                  }
              }
              break;
          }
					
					case MENU_CHANGE_NAME:
          {
							const char* selected = NULL;
            
							if (g_current_menu_index < KEYBOARD_ROW_0_COLS) // 1
							{
									selected = ENGLISH_KEYBOARD[0][g_current_menu_index];
							}
							else if (g_current_menu_index < KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS) // 2
							{
									selected = ENGLISH_KEYBOARD[1][g_current_menu_index - KEYBOARD_ROW_0_COLS];
							}
							else if (g_current_menu_index < TOTAL_KEYBOARD_KEYS) // 3
							{
									selected = ENGLISH_KEYBOARD[2][g_current_menu_index - (KEYBOARD_ROW_0_COLS + KEYBOARD_ROW_1_COLS)];
							}
							else if (g_current_menu_index == TOTAL_KEYBOARD_KEYS) // SET AS NEW NAME
							{
									strcpy(tUI_CamStatus[g_selected_bu_index].name, g_edit_name_buffer);
									//snprintf(tUI_CamStatus[g_selected_bu_index].name, sizeof(tUI_CamStatus[g_selected_bu_index].name), "%s", g_edit_name_buffer);
									g_settings_changed = 1;
									MenuExitHandler();
									break;
							}
							else if (g_current_menu_index == TOTAL_KEYBOARD_KEYS + 1) // EXIT
							{
									MenuExitHandler();
									break;
							}
            
							if (selected != NULL)
							{
									if (strcmp(selected, "DELETE") == 0)
									{
											if (g_edit_name_len > 0)
											{
													g_edit_name_len--;
													g_edit_name_buffer[g_edit_name_len] = '\0';
											}
									}
									else if (strcmp(selected, "SPACE") == 0)
									{
											if (g_edit_name_len < sizeof(g_edit_name_buffer) - 1)
											{
													g_edit_name_buffer[g_edit_name_len++] = ' ';
													g_edit_name_buffer[g_edit_name_len] = '\0';
											}
									}
									else
									{
											if (g_edit_name_len < sizeof(g_edit_name_buffer) - 1)
											{
													g_edit_name_buffer[g_edit_name_len++] = selected[0];
													g_edit_name_buffer[g_edit_name_len] = '\0';
											}
									}
							}
							break;
           }
					 
					case MENU_LEVEL_TEMP_ALARM:
          {
              uint8_t is_alarm_on = tUI_PuSetting.ubTempAlarmOn;
              uint8_t exit_index = is_alarm_on ? 3 : 1; 
          
              if (g_current_menu_index == 0)
              {
                  tUI_PuSetting.ubTempAlarmOn = !tUI_PuSetting.ubTempAlarmOn;
                  g_current_menu_index = 0;
                  g_rectangle_y = 110;
                  g_settings_changed = 1;
              }
              else if (g_current_menu_index == exit_index)
              {
                  MenuExitHandler();
              }
              else if (is_alarm_on)
              {
                  if (g_current_menu_index == 1)
                  {
                      g_current_menu_level = MENU_LEVEL_SET_TEMP_HIGH;
                  }
                  else if (g_current_menu_index == 2)
                  {
                      g_current_menu_level = MENU_LEVEL_SET_TEMP_LOW;
                  }
              }
              break;
          }
					
					case MENU_LEVEL_SET_TEMP_HIGH:
					{
							if(g_current_menu_index == TEMP_HIGH_MENU_ITEM_COUNT - 1) 
							{
                 // EXIT selected
                 MenuExitHandler();
							} 
							else 
							{
                 // Temperature value selected
                 tUI_PuSetting.ubTempMax = TEMP_HIGH_VALUES[g_current_menu_index];
                 g_settings_changed = 1;
                 UI_RefreshScreen();
							}
							break;
         
         case MENU_LEVEL_SET_TEMP_LOW:
							if(g_current_menu_index == TEMP_LOW_MENU_ITEM_COUNT - 1) 
							{
                 // EXIT selected
                 MenuExitHandler();
							} 
							else 
							{
                 // Temperature value selected
                 tUI_PuSetting.ubTempMin = TEMP_LOW_VALUES[g_current_menu_index];
                 g_settings_changed = 1;
                 UI_RefreshScreen();
							}
             break;
					}
					
				 case MENU_PAIRING_INFO:
							switch(g_current_menu_index)
							{
									case 0:
											g_current_menu_level = MENU_ABOUT_PAIRING;
											g_rectangle_y = 365;
											break;
									case 1:
											g_current_menu_level = MENU_COMPATIBLE_UNITS;
											g_current_menu_index = 0;
											g_rectangle_y = 325;
											break;
									case 2:
											MenuExitHandler();
											break;
							}
							break;
							
				 case MENU_PAIR_NEW_BABY_UNIT:
              switch(g_current_menu_index) 
              {
                  case 0: 
											UI_UpdatePairUnitsMenuPositions();
                      g_current_menu_level = MENU_PAIR_MODE;
											g_current_menu_index = 0;
                      g_rectangle_x = 40;
                      g_rectangle_y = PAIRUNITS_MENU_Y_POS[0];
											break;
                  case 1:
											MenuExitHandler(); 
											break;
                }
							break;
				
				 case MENU_ABOUT_PAIRING:
							MenuExitHandler();
							break;
				 
				 case MENU_COMPATIBLE_UNITS:
							if(g_current_menu_index == 0)
							{
									g_current_menu_level = MENU_PAIRING_WITH_N65;
									g_rectangle_y = 365;									
							}
							else
							{
									MenuExitHandler();
							}
							break;
					
				 case MENU_PAIRING_WITH_N65:
							MenuExitHandler();
							break;							
				 
				 case MENU_ABOUT_THIS_MODEL:
							MenuExitHandler();
							break;
				 
				 case MENU_CONTACT_INFORMATION:
							MenuExitHandler();
							break;
				 
				 case MENU_FEATURES:
							switch(g_current_menu_index)
							{
								case 0:
										g_current_menu_level = MENU_RANGE_AND_TRANSMISSION;
										g_current_menu_index = 0;
										g_rectangle_y = 365;
										break;
								case 1:
										g_current_menu_level = MENU_BATTERY;
										g_current_menu_index = 0;
										g_rectangle_y = 365;
										break;
								case 2:
										g_current_menu_level = MENU_SPECIAL_FEATURES;
										g_current_menu_index = 0;
										g_rectangle_y = 365;
										break;
								case 3:
										MenuExitHandler();
										break;
								
								default:
                    break;
							}
							break;
							
				 case MENU_RANGE_AND_TRANSMISSION:
							MenuExitHandler();
							break;
				 
				 case MENU_BATTERY:
							MenuExitHandler();
							break;
				 
				 case MENU_SPECIAL_FEATURES:
							MenuExitHandler();
							break;
				 
				 case MENU_BABY_UNIT_INFO:
							MenuExitHandler();
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
		else if (g_current_menu_level == MENU_SLEEP_HISTORY_VIEW)
		{
				g_current_menu_level = MENU_SLEEP_HISTORY;
				g_current_menu_index = 0;
				g_rectangle_x = 40;
				g_rectangle_y = SLEEPHIS_MAIN_Y_POS[g_current_menu_index];
		}
		else if (g_current_menu_level == MENU_SLEEP_HISTORY_OPTIONS)
		{
				g_current_menu_level = MENU_SLEEP_HISTORY;
				g_current_menu_index = 1;
				g_rectangle_x = 40;
				g_rectangle_y = SLEEPHIS_MAIN_Y_POS[g_current_menu_index];
		}
		else if (g_current_menu_level == MENU_SLEEP_HISTORY)
		{
				g_current_menu_level = MENU_SLEEP_HISTORY_SELECT;
				g_current_menu_index = 0;
				g_rectangle_x = 40;
				g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
		}
		else if (g_current_menu_level == MENU_SLEEP_HISTORY_SELECT)
		{
				g_current_menu_level = MENU_LEVEL_SLEEP_TIMER;
				g_current_menu_index = 2;
				g_rectangle_x = 40;
				g_rectangle_y = SLEEPTIMER_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_DISPLAYSETTINGS)
		{
				g_current_menu_level = 4; // Back SETTINGS
				g_current_menu_index = 4; // positioning "DISPLAY SETTINGS"
				g_rectangle_x = 40;
				g_rectangle_y = SETTINGS_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_BABYUNITSETTINGS)
		{
				g_current_menu_level = 4; // back SETTINGS
				g_current_menu_index = 5; // position "BABY UNIT SETTINGS"
				g_rectangle_x = 40;
				g_rectangle_y = SETTINGS_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_LEVEL_TEMP_ALARM)
		{
				g_current_menu_level = 4; // back SETTINGS
				g_current_menu_index = 6; // position "MENU_LEVEL_TEMP_ALARM"
				g_rectangle_y = SETTINGS_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_LEVEL_SET_TEMP_HIGH)
		{
				g_current_menu_level = MENU_LEVEL_TEMP_ALARM;
				g_current_menu_index = 1;
				g_rectangle_x = 40;
				g_rectangle_y = 150;
		}
		else if(g_current_menu_level == MENU_LEVEL_SET_TEMP_LOW)
		{
				g_current_menu_level = MENU_LEVEL_TEMP_ALARM;
				g_current_menu_index = 2;
				g_rectangle_x = 40;
				g_rectangle_y = 190;
		}
		else if(g_current_menu_level == MENU_ADVANCEDDISPLAYSETTINGS)
		{
				g_current_menu_level = MENU_DISPLAYSETTINGS;
				g_current_menu_index = 3; // positioning "ADVANCED"
				g_rectangle_x = 40;
				g_rectangle_y = DISPLAYSETTINGS_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_BABYUNITDETAIL)
		{
				g_current_menu_level = MENU_BABYUNITSETTINGS;
				g_current_menu_index = g_selected_bu_index;
				g_rectangle_y = BABYUNITSETTINGS_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_CHANGE_NAME)
		{
				g_current_menu_level = MENU_BABYUNITDETAIL;
				g_current_menu_index = 1;
				g_rectangle_x = 40;
				g_rectangle_y = 220;
		}
		else if(g_current_menu_level == MENU_BABY_UNIT_INFO)
		{
				g_current_menu_level = MENU_BABYUNITDETAIL;
				g_current_menu_index = 2;
				g_rectangle_x = 40;
				g_rectangle_y = 260;
		}		
		else if(g_current_menu_level == MENU_PAIR_NEW_BABY_UNIT)
    {
        g_current_menu_level = 5; // Back to PAIR UNITS
        g_current_menu_index = 0; // Position on "CONNECT NEW BABY UNIT"
        g_rectangle_x = 40;
        UI_UpdatePairUnitsMenuPositions();
        g_rectangle_y = PAIRUNITS_MENU_Y_POS[g_current_menu_index];
    }
		else if(g_current_menu_level == MENU_PAIRING_INFO)
    {
        g_current_menu_level = 5; // Back to PAIR UNITS
			
				uint8_t connected_count = 0;
				for(uint8_t i = 0; i < 3; i++)
				{
						if(ubTRX_GetLinkStatus(i)) //g_baby_units[i].is_connected
						{
								connected_count++;
						}
				}
				if(connected_count == 0)
				{
						g_current_menu_index = 1;
				}
				else if(connected_count == 1 || connected_count == 2)
				{
						g_current_menu_index = 2;
				}
				else if(connected_count == 3)
				{
						g_current_menu_index = 1;
				}
        g_rectangle_y = PAIRUNITS_MENU_Y_POS[g_current_menu_index];
    }
		else if(g_current_menu_level == MENU_ABOUT_PAIRING)
    {
				g_current_menu_level = MENU_PAIRING_INFO;
				g_current_menu_index = 0;
        g_rectangle_y = PAIRINGINFO_MENU_Y_POS[g_current_menu_index];
    }
		else if(g_current_menu_level == MENU_COMPATIBLE_UNITS)
    {
				g_current_menu_level = MENU_PAIRING_INFO;
				g_current_menu_index = 1;
        g_rectangle_y = PAIRINGINFO_MENU_Y_POS[g_current_menu_index];
    }
		else if(g_current_menu_level == MENU_PAIRING_WITH_N65)
    {
        g_current_menu_level = MENU_COMPATIBLE_UNITS;
        g_current_menu_index = 0;
        g_rectangle_y = COMPATIBLEUNITS_MENU_Y_POS[g_current_menu_index];
    }
		else if(g_current_menu_level == MENU_ABOUT_THIS_MODEL)
		{
				g_current_menu_level = 6;
				g_current_menu_index = 0;
				g_rectangle_y = INFO_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_FEATURES)
		{
				g_current_menu_level = 6;
				g_current_menu_index = 1;
				g_rectangle_y = INFO_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_CONTACT_INFORMATION)
		{
				g_current_menu_level = 6;
				g_current_menu_index = 2;
				g_rectangle_y = INFO_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_RANGE_AND_TRANSMISSION)
		{
				g_current_menu_level = MENU_FEATURES;
				g_current_menu_index = 0;
				g_rectangle_y = FEATURES_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_BATTERY)
		{
				g_current_menu_level = MENU_FEATURES;
				g_current_menu_index = 1;
				g_rectangle_y = FEATURES_MENU_Y_POS[g_current_menu_index];
		}
		else if(g_current_menu_level == MENU_SPECIAL_FEATURES)
		{
				g_current_menu_level = MENU_FEATURES;
				g_current_menu_index = 2;
				g_rectangle_y = FEATURES_MENU_Y_POS[g_current_menu_index];
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
	
	osDelay(1);
	
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
			else if(g_current_menu_level == MENU_MICROPHONE_SENSITIVITY)
			{
					UI_ShowMicroSensitivity();
			}
			else if(g_current_menu_level == MENU_DISPLAYSETTINGS)
			{
					DisplaySettings();
			}
			else if(g_current_menu_level == MENU_ADVANCEDDISPLAYSETTINGS)
			{
					AdvancedDisplaySettings();
			}
			else if(g_current_menu_level == MENU_BABYUNITSETTINGS)
			{
					UI_ShowBabyUnitSettings();
			}
			else if(g_current_menu_level == MENU_BABYUNITDETAIL) 
			{
					UI_ShowBabyUnitDetail(g_selected_bu_index);
			}
			else if(g_current_menu_level == MENU_CHANGE_NAME)
			{
					UI_ShowChangeName();
			}
			else if(g_current_menu_level == MENU_LEVEL_TEMP_ALARM)
			{
					TemperatureAlarm();
			}
			else if(g_current_menu_level == MENU_LEVEL_SET_TEMP_HIGH)
			{
					osDelay(1);
					UI_ShowSetLimitHigh();
			}
			else if(g_current_menu_level == MENU_LEVEL_SET_TEMP_LOW)
			{
					osDelay(1);
					UI_ShowSetLimitLow();
			}
			else if(g_current_menu_level == MENU_PAIR_NEW_BABY_UNIT)
			{
					UI_PairNewBabyUnit();
			}
			else if(g_current_menu_level == MENU_ABOUT_THIS_MODEL)
			{
					UI_AboutThisModel();
			}
			else if(g_current_menu_level == MENU_CONTACT_INFORMATION)
			{
					UI_ContactInformation();
			}
			else if(g_current_menu_level == MENU_FEATURES)
			{
					UI_Features();
			}
			else if(g_current_menu_level == MENU_RANGE_AND_TRANSMISSION)
			{
					UI_RangeAndTransmission();
			}
			else if(g_current_menu_level == MENU_BATTERY)
			{
					UI_Battery();
			}
			else if(g_current_menu_level == MENU_SPECIAL_FEATURES)
			{
					UI_SpecialFeatures();
			}
			else if(g_current_menu_level == MENU_PAIRING_INFO)
			{
					UI_PairInfo();
			}
			else if(g_current_menu_level == MENU_ABOUT_PAIRING)
			{
					UI_AboutPairing();
			}
			else if(g_current_menu_level == MENU_COMPATIBLE_UNITS)
			{
					UI_CompatibleUnits();
			}
			else if(g_current_menu_level == MENU_PAIRING_WITH_N65)
			{
					UI_PairingWithN65();
			}
			else if(g_current_menu_level == MENU_BABY_UNIT_INFO)
			{
					UI_ShowBabyUnitInfo();
			}
			else if(g_current_menu_level == MENU_SLEEP_HISTORY)
			{
					UI_SleepHistoryMain();
			}
			else if(g_current_menu_level == MENU_SLEEP_HISTORY_VIEW)
			{
					UI_SleepHistoryView();
			}
			else if(g_current_menu_level == MENU_SLEEP_HISTORY_OPTIONS)
			{
					UI_SleepHistoryOptions();
			}
      else if(g_current_menu_level == MENU_SLEEP_HISTORY_SELECT)
      {
          UI_SleepHistorySelect();
      }
	}

	OSD_IMG_INFO tFakeInfo = {0};
  tOSD_Img2(&tFakeInfo, OSD_UPDATE);
}

