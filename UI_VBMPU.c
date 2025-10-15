/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI_VBMPU.c
	\brief		User Interface of VBM Parent Unit (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.16
	\date		2020/06/24
	\copyright	Copyright (C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <string.h>
#include "UI_VBMPU.h"
#include "SF_API.h"
#include "EN_API.h"
#include "FWU_API.h"
#include "TIMER.h"
#include "VDO.h"
#include "LCD.h"
#include "Buzzer.h"
#include "PLY_API.h"
#include "REC_API.h"
#include "ADO.h"

#define osUI_SIGNALS	0x66

/**
 * Key event mapping table
 *
 * @param ubKeyID  			Key ID
 * @param ubKeyCnt 			Key count	(100ms per 1 count, ex.long press 5s, the count set to 50)
 * @param KeyEventFuncPtr 	Key event mapping to function
 */
const UI_KeyEventMap_t UiKeyEventMap[] =
{
	{NULL,					0,			NULL,						NULL},
	{AKEY_MENU, 		25,			UI_MenuKey, 					BUZ_PlaySingleSound},
	//{AKEY_MENU, 		0,			UI_EngModeKey,			BUZ_PlaySingleSound},				//UI_LeftArrowKey(menu)		//UI_CameraSettingMenu1Key
	{AKEY_MENU, 		0,			UI_ShowSleepTimer,			BUZ_PlaySingleSound},
	{AKEY_LEFT, 		0,			UI_ShowMenuKey,			BUZ_PlaySingleSound},
	//{AKEY_LEFT, 		30,			UI_LeftArrowLongKey,		BUZ_PlaySingleSound},
	//{AKEY_LEFT, 		20,			UI_EngModeKey,				BUZ_PlaySingleSound},


	{AKEY_ENTER, 		0,			EnterKeyHandler,				BUZ_PlaySingleSound},
	{AKEY_UP, 			0,			MoveboxUp,				BUZ_PlaySingleSound},
	{AKEY_UP, 			30,			UI_FwUpgViaSdCard,   		NULL},
	{AKEY_DOWN, 		0,			MoveboxDown,			BUZ_PlaySingleSound},
	
//	{AKEY_LEFT, 		0,			UI_LeftArrowKey,			BUZ_PlaySingleSound},
//	{AKEY_LEFT, 		30,			UI_LeftArrowLongKey,		BUZ_PlaySingleSound},
	{AKEY_RIGHT, 		0,			UI_RightArrowKey,			BUZ_PlaySingleSound},
	{AKEY_RIGHT, 		30,			UI_EnPerDebugMode,			BUZ_PlaySingleSound},
	
	{AKEY_ENTER, 		20,			UI_CameraSettingMenu2Key, 	BUZ_PlaySingleSound},
#ifdef S2019A
	{AKEY_PS,			0,			UI_PuPowerSaveKey,			BUZ_PlaySingleSound},
	{AKEY_PS,			20,			UI_SetSPRfWorkCh,			BUZ_PlaySingleSound},
#else
	{AKEY_PS,			0,			UI_BuPowerSaveKey,			BUZ_PlaySingleSound},
	{AKEY_PS,			20,			UI_PuPowerSaveKey,			BUZ_PlaySingleSound},
#endif
	{AKEY_PTT,			0,			UI_PushTalkKey,				NULL},
	{PKEY_ID0, 			20,			UI_PowerKey,				BUZ_PlayPowerOffSound},
};
// static UI_State_t tUI_State;
UI_State_t tUI_State;
static APP_State_t tUI_SyncAppState;
UI_BUStatus_t tUI_CamStatus[CAM_4T];
//static UI_PUSetting_t tUI_PuSetting;  //original
UI_PUSetting_t tUI_PuSetting;

const static UI_MenuFuncPtr_t tUI_StateMap2MenuFunc[UI_STATE_MAX] =
{
	[UI_DISPLAY_STATE]			= UI_DisplayArrowKeyFunc,
	[UI_MAINMENU_STATE] 		= UI_Menu,
	[UI_SUBMENU_STATE]  		= UI_SubMenu,
	[UI_SUBSUBMENU_STATE]  		= UI_SubSubMenu,
	[UI_SUBSUBSUBMENU_STATE]  	= UI_SubSubSubMenu,
	[UI_CAM_SEL_STATE]			= UI_CameraSelection,
	[UI_SET_VDOMODE_STATE]		= UI_ChangeVideoMode,
	[UI_SET_ADOSRC_STATE]		= UI_ChangeAudioSource,
	[UI_SET_PUPSMODE_STATE] 	= UI_PuPowerSaveModeSelection,
	[UI_SET_BUECOMODE_STATE] 	= UI_BuPowerSaveModeSelection,
	[UI_CAMSETTINGMENU_STATE]	= UI_CameraSettingMenu,
	[UI_SET_CAMCOLOR_STATE] 	= UI_CameraColorSetting,
	[UI_DPTZ_CONTROL_STATE]		= UI_DPTZ_Control,
	[UI_MD_WINDOW_STATE]		= UI_MD_Window,
	[UI_PAIRING_STATE]			= UI_PairingControl,
	[UI_DUALVIEW_CAMSEL_STATE]  = UI_CameraSelection4DualView,
	[UI_MULTIVIEWSEL_STATE]     = UI_CameraMultiViewSelection,
	[UI_SDFWUPG_STATE]			= UI_FwUpgExecSel,
	[UI_RECFOLDER_SEL_STATE]	= UI_DCIMFolderSelection,
	[UI_RECFILES_SEL_STATE]		= UI_RecordFileSelection,
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    [UI_RECTXFOLDER_SEL_STATE]	= UI_DCIMTXFolderSelection,	
	[UI_RECTXFILES_SEL_STATE]   = UI_RecordTXFileSelection,
#endif	
	[UI_RECPLAYLIST_STATE]      = UI_RecordPlayListSelection,
	[UI_RECPLAYADOSRC_SEL_STATE]= UI_RecordPlayAdoSrcSelection,
	[UI_PHOTOPLAYNRDY_STATE]	= NULL,
	[UI_PHOTOPLAYLIST_STATE]	= UI_PhotoPlayListSelection,
	[UI_SDCARDFMT_STATE]		= UI_SdCardFormatFunc,
	[UI_ENGMODE_STATE]			= UI_EngModeCtrl,
	[UI_SPRF_SEL_STATE]			= UI_sPRfChSelection,
	[UI_DT_SEL_STATE]			= UI_DriveMdSelection,
};
static UI_MenuItem_t tUI_MenuItem;
static UI_SubMenuItem_t tUI_SubMenuItem[MENUITEM_MAX] =
{
	{CAMSSELCAM_ITEM,	CAMSITEM_MAX	},
	{PAIRCAM_ITEM,		PAIRITEM_MAX	},
	{RECMODE_ITEM,		RECITEM_MAX		},
	{PHOTOSELCAM_ITEM, 	PHOTOITEM_MAX	},
	{NULL},
	{NULL},
	{DATETIME_ITEM, 	SETTINGITEM_MAX	},
};
const UI_ReportFuncPtr_t tUiReportMap2Func[] =
{
	[UI_UPDATE_BUSTS] 			= UI_UpdateBuStatus,
	[UI_VOX_TRIG]				= UI_VoxTrigger,
	[UI_MD_TRIG]				= UI_MDTrigger,
	[UI_VOICE_TRIG]				= UI_VoiceTrigger,
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    [UI_FS_TRIG]				= UI_FSTrigger,
    [UI_PLY_TRIG]				= UI_TXPLYTrigger,
#endif	
};

const ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n32p4DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n8p2DB};

osMutexId UI_PUMutex;
static UI_SubMenuCamNum_t tCamSelect;
static UI_CamViewSelect_t tCamViewSel;
//static UI_CamViewSelect_t tCamPreViewSel;
static UI_PairingInfo_t tPairInfo;
static UI_ThreadNotify_t tosUI_Notify;
static UI_DPTZParam_t tUI_DptzParam;
static UI_CamNum_t tUI_BuEcoCamNum;
static UI_CamNum_t tUI_CamNumSel;
static UI_SdSts_t tUI_SdCardSts;
static uint32_t ulUI_LogoIndex;
static uint32_t ulUI_MonitorPsFlag[CAM_4T];
static uint8_t ubUI_PuStartUpFlag;
static uint8_t ubUI_PttStartFlag;
static uint8_t ubUI_ResetPeriodFlag;
static uint8_t ubUI_FastStateFlag;
static uint8_t ubUI_ShowTimeFlag;
static uint8_t ubUI_ScanStartFlag;
static uint8_t ubUI_RecSubMenuFlag;
static uint8_t ubUI_DualViewExFlag;
static uint8_t ubUI_StopUpdateStsBarFlag;
uint8_t *pUI_BuConnectFlag[CAM_STSMAX];
static uint8_t ubUI_DisScanMdFunc;

//! Record
static void UI_OsdLoadingDisplayThread(void const *argument);
osThreadId osUI_OsdLdDispThdId;
osMessageQId osUI_OsdLdDispQueue;
osMessageQId osUI_RecRptQueue;
osMutexId osUI_OsdLdStsUpdMutex;
static UI_OsdLdDispSts_t tUI_OsdLdDispSts;
static UI_RecFoldersInfo_t pUI_RecFoldersInfo;
static UI_RecFilesInfo_t pUI_RecFilesInfo;
static UI_RecOsdImgDb_t *pUI_RecOsdImgDB;
static OSD_IMGIDXARRARY_t tUI_RecOsdImgInfo;
static UI_RecPlayAct_t tUI_RecPlayAct;
static uint8_t ubUI_VdoRecChkFlag;
//! Functions Execute
static void UI_FuncsExecuteThread(void const *argument);
osThreadId osUI_FuncsExecThdId;
osMessageQId osUI_FuncsExecQue;
osMessageQId osUI_FuncsFinExecQue;
#ifdef S2019A
//! For SPRF
static uint8_t ubUI_sPRfChSel = 0;
#endif
//! Performance Debug
#define MAX_DBG_ITEM	3
osMutexId osUI_PerDbgMutex;
static OSD_IMG_INFO tUI_PerDbgOsdImgInfo[17];
static uint32_t ulUI_FrameErrCnt[CAM_4T][MAX_DBG_ITEM];
static uint8_t ubUI_PerDebugEn;
static uint8_t ubUI_SwtichViewTypeFg=0;
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
static uint8_t ubUI_TXGrayoutFg=0;
static uint8_t ubUI_RecSubMenuTXFldFlag;
static UI_RecFoldersInfo_t pUI_RecTXFoldersInfo;
static UI_RecFilesInfo_t pUI_RecTXFilesInfo;
static osThreadId osUIThreadIdBackup;
static uint8_t ubUI_FsEventSt=0;
static uint8_t ubUI_PlayEventSt=0;
uint8_t ubUI_RemotePlayTrigger=0;
#define osUI_TXFsFldFinSignal		0x41
#define osUI_TXFsFileFinSignal		0x42
#define osUI_TXFsHiddenFinSignal	0x43
#define osUI_TXPlyFinSignal	        0x44
#endif
#ifdef S2019A
//! For WiFi Direct
UI_DevDrvStsInfo_t tUI_DrvStsInfo;
#endif
//------------------------------------------------------------------------------
void UI_KeyEventExec(void *pvKeyEvent)
{
	static uint8_t ubUI_KeyEventIdx = 0;
	KEY_Event_t *ptKeyEvent;
	uint16_t uwUiKeyEvent_Cnt = 0, uwIdx;
#ifdef S2019A
	sPRF_DrvMode_t tCurDrvMd = sPRF_TRX_MODE;

	tCurDrvMd = tsPRF_GetDrvMode();
#endif
	ptKeyEvent = (KEY_Event_t *)pvKeyEvent;
	uwUiKeyEvent_Cnt = sizeof UiKeyEventMap / sizeof(UI_KeyEventMap_t);
	if(ptKeyEvent->ubKeyAction == KEY_UP_ACT)
	{
		if((UI_DPTZ_CONTROL_STATE == tUI_State) &&
		   ((ptKeyEvent->ubKeyID == AKEY_UP)   || (ptKeyEvent->ubKeyID == AKEY_DOWN) ||
		    (ptKeyEvent->ubKeyID == AKEY_LEFT) || (ptKeyEvent->ubKeyID == AKEY_RIGHT)))
		{
			UI_DPTZ_KeyRelease(ptKeyEvent->ubKeyID);
			return;
		}
		if(((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt)) ||
		   (ptKeyEvent->ubKeyID == AKEY_PTT))
		{
			if(ptKeyEvent->ubKeyID == AKEY_PTT)
				ubUI_PttStartFlag = FALSE;
			if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
			{
			#ifdef S2019A
				if((sPRF_APDIRECT_MODE == tCurDrvMd) &&
				   (ptKeyEvent->ubKeyID != AKEY_PS)  && (UI_DT_SEL_STATE != tUI_State))
				{
					ubUI_KeyEventIdx = 0;
					return;
				}
			#endif
			    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
				if(UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone)
					UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
			}
		}
		ubUI_KeyEventIdx = 0;
		return;
	}
	for(uwIdx = 1; uwIdx < uwUiKeyEvent_Cnt; uwIdx++)
	{
		if((UI_DPTZ_CONTROL_STATE == tUI_State) &&
		   ((ptKeyEvent->ubKeyID == AKEY_UP)   || (ptKeyEvent->ubKeyID == AKEY_DOWN) ||
		    (ptKeyEvent->ubKeyID == AKEY_LEFT) || (ptKeyEvent->ubKeyID == AKEY_RIGHT)))
		{
			if(UI_DPTZ_KeyPress(ptKeyEvent->ubKeyID, uwIdx) == rUI_FAIL)
				continue;
			if(UiKeyEventMap[uwIdx].KeyEventFuncPtr)
			{
			#ifdef S2019A
				if(sPRF_APDIRECT_MODE == tCurDrvMd)
				{
					ubUI_KeyEventIdx = 0;
					break;
				}
			#endif
			    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
				UiKeyEventMap[uwIdx].KeyEventFuncPtr();
				if(UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone)
					UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
			}
			ubUI_KeyEventIdx = 0;
			break;
		}
		if((ptKeyEvent->ubKeyID  == UiKeyEventMap[uwIdx].ubKeyID) &&
		   (ptKeyEvent->uwKeyCnt == UiKeyEventMap[uwIdx].uwKeyCnt))
		{
			ubUI_KeyEventIdx = uwIdx;
			if(((ptKeyEvent->uwKeyCnt) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)) ||
			   (ptKeyEvent->ubKeyID == AKEY_PTT))
			{
				if(ptKeyEvent->ubKeyID == AKEY_PTT)
					ubUI_PttStartFlag = TRUE;
			#ifdef S2019A
				if(sPRF_APDIRECT_MODE == tCurDrvMd)
				{
					ubUI_KeyEventIdx = 0;
					break;
				}
			#endif
				KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
				if(UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone)
					UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
				ubUI_KeyEventIdx = (ptKeyEvent->ubKeyID == AKEY_PTT)?ubUI_KeyEventIdx:0;
			}
		}
	}
}
//------------------------------------------------------------------------------
void UI_OnInitDialog(void)
{
	if(NULL == osUI_OsdLdDispQueue)
	{
		osMessageQDef(UiLdDispQue, 10, uint16_t);
		osUI_OsdLdDispQueue = osMessageCreate(osMessageQ(UiLdDispQue), NULL);
	}
	if(NULL == osUI_RecRptQueue)
	{
		osMessageQDef(UiRecRptQue, 1, uint8_t);
		osUI_RecRptQueue = osMessageCreate(osMessageQ(UiRecRptQue), NULL);
	}
	if(NULL == osUI_OsdLdDispThdId)
	{
		osThreadDef(UiOsdLdDisp, UI_OsdLoadingDisplayThread, THREAD_PRIO_UIEVENT_HANDLER, 1, 512);
		osUI_OsdLdDispThdId = osThreadCreate(osThread(UiOsdLdDisp), NULL);
	}
	if(NULL == osUI_FuncsExecQue)
	{
		osMessageQDef(UiFuncExecQue, 300, UI_FuncExecMsg_t);
		osUI_FuncsExecQue = osMessageCreate(osMessageQ(UiFuncExecQue), NULL);
	}
	if(NULL == osUI_FuncsFinExecQue)
	{
		osMessageQDef(UiFuncsFinExecQue, 1, uint8_t);
		osUI_FuncsFinExecQue = osMessageCreate(osMessageQ(UiFuncsFinExecQue), NULL);
	}	
	if(NULL == osUI_FuncsExecThdId)
	{
		osThreadDef(UiFuncsExec, UI_FuncsExecuteThread, osPriorityBelowNormal, 1, 8192);
		osUI_FuncsExecThdId = osThreadCreate(osThread(UiFuncsExec), NULL);
	}

	if ((DISPLAY_MODE != DISPLAY_1T1R) && wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) == RTC_WATCHDOG_CHK_TAG) {
		printd(DBG_CriticalLvl, "PWR STS: Keep\n");
		tCamViewSel.tCamViewType    = (UI_CamViewType_t)wRTC_ReadUserRam(RTC_RECORD_VIEW_MODE_ADDR);
		tCamViewSel.tCamViewPool[0] = (UI_CamNum_t)(wRTC_ReadUserRam(RTC_RECORD_VIEW_CAM_ADDR) >> 4);
		tCamViewSel.tCamViewPool[1] = (UI_CamNum_t)(wRTC_ReadUserRam(RTC_RECORD_VIEW_CAM_ADDR) & 0x0f);
	} else {
		tCamViewSel.tCamViewType    = (VDO_DISP_TYPE == KNL_DISP_QUAD)?QUAD_VIEW:
									  (VDO_DISP_TYPE == KNL_DISP_DUAL_C)?DUAL_VIEW:
									  (VDO_DISP_SCAN == TRUE)?SCAN_VIEW:SINGLE_VIEW;
		tCamViewSel.tCamViewPool[0] = (VDO_DISP_TYPE == KNL_DISP_QUAD)?CAM_4T:CAM1;
		tCamViewSel.tCamViewPool[1] = (VDO_DISP_TYPE == KNL_DISP_DUAL_C)?CAM2:NO_CAM;
	}

	if(iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar)) != RTC_OK)
		printd(DBG_ErrorLvl, "Calendar base setting fail!\n");

	if (((wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) & 0xF) != RTC_PWRSTS_KEEP_TAG) &&
		((wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) & 0xF) != RTC_WATCHDOG_CHK_TAG))
		RTC_SetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));

	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, ((wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) & 0xF0) | RTC_WATCHDOG_CHK_TAG));
#ifdef S2019A
	if(sPRF_APDIRECT_MODE == tsPRF_GetDrvMode())
		ulUI_LogoIndex = OSDLOGO_WIFIDT_ENY;
#endif
	OSD_LogoJpeg(ulUI_LogoIndex);
	OSD_IMG_INFO tOsdImgInfo;	  //Logo Image
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tOsdImgInfo);
  tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
	osDelay(3000);
	OSD_EraserImg1(&tOsdImgInfo);
	GPIO->GPIO_O0 	= 0;
	GPIO->GPIO_O13 	= 0;
	BUZ_PlayPowerOnSound();
}
//------------------------------------------------------------------------------
#if (APP_ADO_FUNC_ENABLE == 1)
KNL_ROLE PcConn_AdoRole;
#endif
void UI_ShowPcCnnPic(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	ubUI_StopUpdateStsBarFlag = TRUE;
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
	UI_ClearStatusBarOsdIcon();
	UI_ClearBuConnectStatusFlag();
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo);
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_FWUSTARTBG, 1, &tOsdImgInfo);
	tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
	
	KEY_Suspend();
	UI_StopUpdateThread();
#if (APP_ADO_FUNC_ENABLE == 1)
	PcConn_AdoRole = ADO_Stop();
#endif
	VDO_Stop();
}
//------------------------------------------------------------------------------
void UI_PcConn_SdCardPlugout(void)
{
	UI_ClearOsdImage();
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
	ubUI_StopUpdateStsBarFlag = FALSE;
	if(FALSE == tUI_PuSetting.IconSts.ubShowLostLogoFlag)
		UI_DrawPUStatusIcon();
	tUI_State = UI_DISPLAY_STATE;

	VDO_Start();
#if (APP_ADO_FUNC_ENABLE == 1)
	ADO_Start(PcConn_AdoRole);
#endif
	UI_StartUpdateThread();
	KEY_Resume();
}
//------------------------------------------------------------------------------
void UI_StateReset(void)
{
	osMutexDef(UI_PUMutex);
	UI_PUMutex 	= osMutexCreate(osMutex(UI_PUMutex));
	osMutexDef(UI_OsdLdStsUpdMutex);
	osUI_OsdLdStsUpdMutex	  	 = osMutexCreate(osMutex(UI_OsdLdStsUpdMutex));
	tosUI_Notify.thread_id	  	 = NULL;
	tosUI_Notify.iSignals	  	 = 0;
	ubUI_PuStartUpFlag		  	 = FALSE;
	ubUI_ResetPeriodFlag		 = FALSE;
	ubUI_FastStateFlag		 	 = FALSE;
	ubUI_ShowTimeFlag		 	 = FALSE;
	ubUI_PttStartFlag		  	 = TRUE;
	ubUI_ScanStartFlag    	  	 = FALSE;
	ubUI_DisScanMdFunc			 = FALSE;
	ubUI_DualViewExFlag		  	 = FALSE;
	ubUI_StopUpdateStsBarFlag 	 = FALSE;
	tUI_SdCardSts				 = UI_SD_CFM;
	osUI_OsdLdDispQueue		  	 = NULL;
	osUI_OsdLdDispThdId		  	 = NULL;
	osUI_RecRptQueue			 = NULL;
	tUI_OsdLdDispSts		  	 = UI_OSDLDDISP_OFF;
	osUI_FuncsExecThdId			 = NULL;
	osUI_FuncsExecQue			 = NULL;
	osUI_FuncsFinExecQue		 = NULL;
	ubUI_RecSubMenuFlag		  	 = FALSE;
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
	ubUI_RecSubMenuTXFldFlag     = FALSE;
#endif
	memset(&pUI_RecFoldersInfo, 0, sizeof(UI_RecFoldersInfo_t));
	memset(&pUI_RecFilesInfo,   0, sizeof(UI_RecFilesInfo_t));
	tUI_RecPlayAct.tRecAct 		 = UI_REC_STOP;
	tUI_RecPlayAct.tPlaySts 	 = UI_RECFILE_STOP;
    tUI_RecPlayAct.tSimFld       = KNL_REAL_FLD;
	ubUI_VdoRecChkFlag			 = TRUE;
	ubUI_PerDebugEn				 = FALSE;
	osMutexDef(UiPerDbgMutex);
	osUI_PerDbgMutex 			 = osMutexCreate(osMutex(UiPerDbgMutex));
	tUI_State 		 		  	 = UI_DISPLAY_STATE;
	tUI_SyncAppState		  	 = APP_STATE_NULL;
	ulUI_LogoIndex			  	 = OSDLOGO_BOOT;
	tUI_MenuItem.ubItemIdx 	  	 = CAMERAS_ITEM;
	tUI_MenuItem.ubItemPreIdx 	 = CAMERAS_ITEM;
	UI_ResetSubMenuInfo();
	UI_ResetSubSubMenuInfo();
	if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvBUResponse, UI_RecvBURequest) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI Setting 2-way command fail!\n");
	UI_LoadDevStatusInfo();
	UI_LoadDrvStatusInfo();
#if APP_PC_CONNECT_EN
	KNL_PCCONN_INIT_t UI_PcConnInitInfo = {0};
	
	UI_PcConnInitInfo.EnterPcConnCb   = UI_ShowPcCnnPic;
	UI_PcConnInitInfo.LeavePcConnCb   = NULL;
	UI_PcConnInitInfo.OtherAction     = NULL;
	UI_PcConnInitInfo.SdCardPlugoutCb = UI_PcConn_SdCardPlugout;
	
	UI_PcConnInitInfo.SdFwuFileName.ubLen = 9;
	memcpy(&UI_PcConnInitInfo.SdFwuFileName.chName, "SN937XXFW", UI_PcConnInitInfo.SdFwuFileName.ubLen);
	memcpy(&UI_PcConnInitInfo.SdFwuFileName.chExt, "BIN", 3);
	
	UI_PcConnInitInfo.EngModeFileName.ubLen = 2;
	memcpy(&UI_PcConnInitInfo.EngModeFileName.chName, "EM", UI_PcConnInitInfo.EngModeFileName.ubLen);
	memcpy(&UI_PcConnInitInfo.EngModeFileName.chExt, "BIN", 3);
	
	KNL_PcConnectInit(&UI_PcConnInitInfo);
#endif
}
//------------------------------------------------------------------------------
void UI_UpdateFwUpgStatus(void *ptUpgStsReport)
{
	APP_StatusReport_t *pFWU_StsRpt = (APP_StatusReport_t *)ptUpgStsReport;
	OSD_IMG_INFO tOsdImgInfo;
	static UI_State_t tUI_PreState;

	switch(pFWU_StsRpt->ubAPP_Report[0])
	{
		case FWU_UPG_INPROGRESS:
            if(UI_REC_START == tUI_RecPlayAct.tRecAct)
                UI_VideoRecordingExec(UI_REC_STOP);
			if(TRUE == ubUI_PerDebugEn)
				UI_EnPerDebugMode();
			ubUI_StopUpdateStsBarFlag = TRUE;
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			UI_ClearStatusBarOsdIcon();
			UI_ClearBuConnectStatusFlag();
			OSD_Weight(OSD_WEIGHT_8DIV8);
			tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
			tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdImgInfo);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_FWUSTARTBG, 1, &tOsdImgInfo);
			tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
			tUI_PreState = tUI_State;
			tUI_State = UI_FWUPG_STATE;
			break;
		case FWU_UPG_SUCCESS:
		case FWU_UPG_FAIL:
		case FWU_UPG_DEVTAG_FAIL:
		{
			uint16_t uwImgIdx = (FWU_UPG_SUCCESS == pFWU_StsRpt->ubAPP_Report[0])?1:0;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_FWUFAILED_ICON + uwImgIdx), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			osDelay(2000);
			if(FWU_UPG_SUCCESS != pFWU_StsRpt->ubAPP_Report[0])
			{
				tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
				tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
			}
			ubUI_StopUpdateStsBarFlag = FALSE;
			tUI_State = tUI_PreState;
			break;
		}
		default:
		{
			uint8_t ubProgScaleIdx = (pFWU_StsRpt->ubAPP_Report[0] / pFWU_StsRpt->ubAPP_Report[1]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_FWUPROG0P_ICON + ubProgScaleIdx), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			if(100 == pFWU_StsRpt->ubAPP_Report[0])
				osDelay(1000);
			break;
		}
	}
}
//------------------------------------------------------------------------------
void UI_UpdateAppStatus(void *ptAppStsReport)
{
	APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;

	osMutexWait(UI_PUMutex, osWaitForever);
	switch(pAppStsRpt->tAPP_ReportType)
	{
		case APP_PAIRSTS_RPT:
		{
			UI_Result_t tPair_Result  = (UI_Result_t)pAppStsRpt->ubAPP_Report[0];
			UI_CamNum_t tAppAdoSrcNum = (UI_CamNum_t)pAppStsRpt->ubAPP_Report[1];
			uint8_t ubAppPairFlag	  = pAppStsRpt->ubAPP_Report[2];
			void (*UI_RptPairFunc[])(UI_Result_t) =
			{
				UI_ReportPairingResult,
				UI_ReportAppPairingResult,
			};

			if(TRUE == ubAppPairFlag)
			{
				tPairInfo.tPairSelCam   = (UI_CamNum_t)pAppStsRpt->ubAPP_Report[3];
				tPairInfo.tDispLocation = (UI_DisplayLocation_t)pAppStsRpt->ubAPP_Report[4];
			}
			UI_RptPairFunc[ubAppPairFlag](tPair_Result);
			if((rUI_SUCCESS == tPair_Result) &&
			   (tUI_PuSetting.tAdoSrcCamNum != tAppAdoSrcNum))
				pAppStsRpt->ubAPP_Report[1] = tUI_PuSetting.tAdoSrcCamNum;
			break;
		}
		case APP_LINKSTS_RPT:
			UI_ReportBuConnectionStatus(pAppStsRpt->ubAPP_Report);
			break;
		case APP_VWMODESTS_RPT:
			if (pAppStsRpt->ubAPP_Report[0] != SCAN_VIEW)
				UI_DisableScanMode();
			break;
		case APP_VOXMODESTS_RPT:
			ubUI_StopUpdateStsBarFlag = pAppStsRpt->ubAPP_Report[0];
			if(TRUE == ubUI_StopUpdateStsBarFlag)
				break;
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			UI_ClearStatusBarOsdIcon();
			UI_ClearBuConnectStatusFlag();
			UI_RemoveLostLinkLogo();
			break;
		case APP_PAIRUDBU_PRT:
		{
			UI_CamNum_t tDelCam 	= (UI_CamNum_t)pAppStsRpt->ubAPP_Report[0];
			uint8_t ubAppRrefUiFlag = pAppStsRpt->ubAPP_Report[1];

			if(TRUE == ubAppRrefUiFlag)
			{
				tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
				UI_ClearStatusBarOsdIcon();
				UI_ClearBuConnectStatusFlag();
			}
			UI_UnBindBu(tDelCam);
			break;
		}
		case APP_DISPPAIRICON_RPT:
		{
			UI_DisplayAppPairingScreen();
			osMutexRelease(UI_PUMutex);
			return;
		}
		default:
			break;
	}
	if(pAppStsRpt->tAPP_State == APP_LINK_STATE)
	{
		if(tUI_SyncAppState != pAppStsRpt->tAPP_State)
			UI_RemoveLostLinkLogo();
		ubUI_ResetPeriodFlag = TRUE;
		if((SCAN_VIEW == tCamViewSel.tCamViewType) && (FALSE == ubUI_ScanStartFlag) && (FALSE == ubUI_DisScanMdFunc))
			UI_EnableScanMode();

	}
	if(FALSE == ubUI_PuStartUpFlag)
	{
		ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
		if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
			UI_EnableVox();
        if((UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode) &&
		   (POWER_NORMAL_MODE == tUI_PuSetting.tPsMode)  && (SCAN_VIEW != tCamViewSel.tCamViewType))
			UI_VideoRecordingExec(UI_REC_START);
		ubUI_PuStartUpFlag = TRUE;
	}
	tUI_PuSetting.IconSts.ubClearThdCntFlag = (tUI_SyncAppState == pAppStsRpt->tAPP_State)?FALSE:TRUE;
	tUI_SyncAppState = pAppStsRpt->tAPP_State;
	osMutexRelease(UI_PUMutex);
}
//------------------------------------------------------------------------------
void UI_UpdateStatus(uint16_t *pThreadCnt)
{
	APP_EventMsg_t tUI_GetLinkStsMsg = {0};
	uint8_t ubUI_SendMsg2AppFlag = FALSE;
#ifdef S2019A
	static sPRF_DrvMode_t tUI_sPRFdrv = sPRF_TRX_MODE;
	static uint8_t ubUI_UpdDtStsCnt = 0;
	sPRF_DrvMode_t tCurDrvMd = sPRF_TRX_MODE;
	UI_CamNum_t tCamNum;
#endif
	//osMutexWait(UI_PUMutex, osWaitForever);
	//UI_CLEAR_THREADCNT(tUI_PuSetting.IconSts.ubClearThdCntFlag, *pThreadCnt);
    osStatus uiMutexState = osMutexWait(UI_PUMutex, 0);
    if (uiMutexState != osOK)
    {
            /*
             * The UI event thread uses the same mutex when dispatching key
             * presses.  If we block here waiting forever we can starve the
             * key handler and make the keypad look dead.  By skipping this
             * refresh when the mutex is busy we let the key task finish and
             * retry the status update on the next tick.
             */
            return;
    }
    UI_CLEAR_THREADCNT(tUI_PuSetting.IconSts.ubClearThdCntFlag, *pThreadCnt);
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    if( UI_RECFILE_PLAY == tUI_RecPlayAct.tPlaySts && KNL_SIM_FLD == tUI_RecPlayAct.tSimFld &&        
        ubKNL_GetCommLinkStatus(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx)) == BB_LOST_LINK )
    {    
        if(UI_ChkRemotePlayTrigger()==0)
        {
            UI_RemotePlayTrigger(1);
            UI_StopRemotePlay();
            UI_RemotePlayTrigger(0);
        }
    }
    
    if( (tUI_State == UI_RECTXFOLDER_SEL_STATE || tUI_State == UI_RECTXFILES_SEL_STATE )&&
        ubKNL_GetCommLinkStatus(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx)) == BB_LOST_LINK )
    {
        OSD_IMG_INFO tRecPlayOsdImgInfo[2];
    	tRecPlayOsdImgInfo[0].uwHSize  = 100;
    	tRecPlayOsdImgInfo[0].uwVSize  = uwOSD_GetVSize();
    	tRecPlayOsdImgInfo[0].uwXStart = 620;
    	tRecPlayOsdImgInfo[0].uwYStart = 0;
    	OSD_EraserImg1(&tRecPlayOsdImgInfo[0]);
    	OSD_Weight(OSD_WEIGHT_8DIV8);
    	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
    	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
    	tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
    	tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_UPDATE);
        pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
		pUI_RecFilesInfo.uwRecFileSelIdx = 0;
        UI_DrawDCIMFolderMenu();
    }    

    if(KNL_VIDEO_PLAY != tKNL_GetRecordFunc() && ubKNL_GetRemotePlayFrmType() == 0xAA && ubKNL_CheckVdoTransmit(ubKNL_GetRemotePlayStaNump()) )
    {
        UI_CamNum_t tRecCamNum;
        APP_KNLRoleMap2CamNum(ubKNL_GetRemotePlayStaNump(), tRecCamNum);
        UI_TXPlayEvent(tRecCamNum,UI_TXPLY_STOPCAM,NULL,NULL,NULL);       
    }
#endif    
#ifdef S2019A
	tCurDrvMd = tsPRF_GetDrvMode();
	if(sPRF_APDIRECT_MODE == tCurDrvMd)
	{
		tUI_sPRFdrv = sPRF_APDIRECT_MODE;
		if(UI_DISPLAY_STATE == tUI_State)
		{
			if(!(++ubUI_UpdDtStsCnt % 3))
			{
				uint8_t ubDtRdy = 0;
				ubDtRdy = ubsPRF_GetDtDevRdy();
				for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
				{
					if(((ubDtRdy >> tCamNum) & 0x1) != (tUI_DrvStsInfo.ubDtRdy[tCamNum]))
					{
						OSD_IMG_INFO tOsdImgInfo;
						tUI_DrvStsInfo.ubDtRdy[tCamNum] = (ubDtRdy >> tCamNum) & 0x1;
						tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_WIFIDTCAM1NRDY_ICON + (tCamNum * 2) + tUI_DrvStsInfo.ubDtRdy[tCamNum]), 1, &tOsdImgInfo);
						tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					}
				}
				ubUI_UpdDtStsCnt = 0;
			}
		}
		else
			ubUI_UpdDtStsCnt = 0;
			osMutexRelease(UI_PUMutex);
			return;
	}
	else if(sPRF_APDIRECT_MODE == tUI_sPRFdrv)
	{
		uint8_t ubClrFlag = TRUE;
		tUI_SyncAppState = APP_LOSTLINK_STATE;
		ubUI_ResetPeriodFlag = FALSE;
		UI_ClearOsdImage();
		UI_CLEAR_THREADCNT(ubClrFlag, *pThreadCnt);
	}
	if(tUI_sPRFdrv != tCurDrvMd)
	{
		UI_UpdateDrvStatusInfo();
		tUI_sPRFdrv = tCurDrvMd;
	}
#endif
	
	switch(tUI_SyncAppState)
	{
		case APP_IDLE_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			break;
		case APP_LOSTLINK_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			if(FALSE == tUI_PuSetting.IconSts.ubShowLostLogoFlag)
			{
				//UI_ShowLostLinkLogo(pThreadCnt);
				(*pThreadCnt)++;
				goto END_UPDATESTS;
			}
			break;
		case APP_LINK_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			if(tUI_State == UI_DISPLAY_STATE)
			{
				UI_RedrawStatusBar(pThreadCnt);
				(*pThreadCnt)++;
				goto END_UPDATESTS;
			}
			break;
		case APP_PAIRING_STATE:
			UI_DrawPairingStatusIcon();
			osMutexRelease(UI_PUMutex);
			return;
		default:
			break;
	}
	*pThreadCnt 	    					= 0;
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = (UI_SHOWSTSICON_STATE == tUI_State)?tUI_PuSetting.IconSts.ubDrawStsIconFlag:FALSE;
END_UPDATESTS:
	UI_UpdateBriLvlIcon();
	UI_UpdateVolLvlIcon();
	UI_UpdateRecStsIcon();
	UI_UpdateWarningNoteIcon();
	tUI_PuSetting.IconSts.ubRdPairIconFlag 	= FALSE;
	if(ubUI_SendMsg2AppFlag == TRUE)
	{
		tUI_GetLinkStsMsg.ubAPP_Event 		= APP_LINKSTATUS_REPORT_EVENT;
		UI_SendMessageToAPP(&tUI_GetLinkStsMsg);
	}
  osMutexRelease(UI_PUMutex);
}
//------------------------------------------------------------------------------
void UI_EventHandles(UI_Event_t *ptEventPtr)
{
    //printf("tUI_State %d\n",tUI_State);
	if(FALSE == ubUI_PuStartUpFlag)
		return;
	switch(ptEventPtr->tEventType)
	{
		case AKEY_EVENT:
		case PKEY_EVENT:
			osMutexWait(UI_PUMutex, osWaitForever);
			UI_KeyEventExec(ptEventPtr->pvEvent);
			osMutexRelease(UI_PUMutex);
			break;
		case SCANMODE_EVENT:
			UI_ScanModeExec();
			break;
		case FWUPG_EVENT:
			KNL_SDUpgradeFwFunc();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
APP_EventMsg_t *tUI_ViewTypeSetup(UI_CamViewType_t tViewType)
{
	static APP_EventMsg_t tUI_ViewTypeParam = {0};
	UI_CamNum_t tCamNum;

	tCamViewSel.tCamViewType = tViewType;
	if((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
	   ((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum) && (DUAL_VIEW == tViewType)))
		return NULL;
	switch(tViewType)
	{
		case SINGLE_VIEW:
		case DUAL_VIEW:
			tCamViewSel.tCamViewPool[0] = NO_CAM;
			tCamViewSel.tCamViewPool[1] = NO_CAM;
			tCamViewSel.tCamViewPool[2] = NO_CAM;
			for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
			{
				if(tUI_CamStatus[tCamNum].ulCAM_ID != INVALID_ID)
				{
					if(NO_CAM == tCamViewSel.tCamViewPool[0])
					{
						tCamViewSel.tCamViewPool[0] = tCamNum;
					}
					else if(NO_CAM == tCamViewSel.tCamViewPool[1])
					{
						tCamViewSel.tCamViewPool[1] = tCamNum;
						break;
					}
				}
			}
			if(NO_CAM == tCamViewSel.tCamViewPool[0])
				return NULL;
			if(NO_CAM == tCamViewSel.tCamViewPool[1])
				tCamViewSel.tCamViewPool[1] = ((tCamViewSel.tCamViewPool[0] + 1) >= ((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?CAM_2T:CAM_4T))?CAM1:
											   ((UI_CamNum_t)(tCamViewSel.tCamViewPool[0] + 1));
			break;
		case TRIPLE_2L1R_VIEW:
		case TRIPLE_1L2R_VIEW:
		case TRIPLE_2T1B_VIEW:
		case TRIPLE_1T2B_VIEW:
		case TRIPLE_3COL_VIEW:
			for(tCamNum = CAM1; tCamNum <= (CAM4-1); tCamNum++)
			{
				tCamViewSel.tCamViewPool[tCamNum] = tCamNum;
				tUI_ViewTypeParam.ubAPP_Message[tCamNum] = tCamViewSel.tCamViewPool[tCamNum];
			}			
			break;
		default:
			return NULL;
	}
	tUI_ViewTypeParam.ubAPP_Message[0] = tCamViewSel.tCamViewPool[0];
	tUI_ViewTypeParam.ubAPP_Message[1] = tCamViewSel.tCamViewPool[1];
	tUI_PuSetting.tAdoSrcCamNum 	   = tCamViewSel.tCamViewPool[0];

	return &tUI_ViewTypeParam;
}
//------------------------------------------------------------------------------
void UI_ClearOsdImage(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo);
}
//------------------------------------------------------------------------------
void UI_PowerKey(void)
{
	BUZ_PlayPowerOffSound();
	osDelay(600);			//wait buzzer play finish
	LCDBL_ENABLE(UI_DISABLE);
	POWER_LED_IO  = 0;
	SIGNAL_LED_IO = 0;
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
	RTC_SetGPO_1(0, RTC_PullDownEnable);
	if(UI_REC_START == tUI_RecPlayAct.tRecAct)
		UI_VideoRecordingExec(UI_REC_STOP);
	printd(DBG_Debug1Lvl, "Power OFF!\n");
	RTC_PowerDisable();
	while(1);
}
//------------------------------------------------------------------------------


void UI_MenuKey(void)
{
	
	if (UI_PAIRING_STATE != tUI_State)
	{
		APP_EventMsg_t tUI_PairMessage = {0};

		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRINGSINGLE_ICON, 1, &tOsdImgInfo);
		//tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
		tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
		tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam = CAM1;
		tUI_PairMessage.ubAPP_Message[2] = DISP_1T;
		tUI_PairMessage.ubAPP_Message[3] = FALSE;
		UI_SendMessageToAPP(&tUI_PairMessage);
		tPairInfo.ubDrawFlag 			 = TRUE;
		UI_DisableScanMode();
		tUI_State = UI_PAIRING_STATE;	
	}
	
	/*
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
			tUI_State = UI_MAINMENU_STATE;
			tUI_MenuItem.ubItemPreIdx = CAMERAS_ITEM;
			tUI_MenuItem.ubItemIdx 	  = CAMERAS_ITEM;
			UI_DrawMenuPage();
			break;
		case UI_MAINMENU_STATE:
		{
			OSD_IMG_INFO tOsdInfo;

			tOsdInfo.uwHSize  = uwOSD_GetHSize();
			tOsdInfo.uwVSize  = uwOSD_GetVSize();
			tOsdInfo.uwXStart = 0;
			tOsdInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdInfo);
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
			if(FALSE == tUI_PuSetting.IconSts.ubShowLostLogoFlag)
				UI_DrawPUStatusIcon();
			tUI_State = UI_DISPLAY_STATE;
			break;
		}
		case UI_SUBMENU_STATE:
		{
			tUI_State = UI_MAINMENU_STATE;
			UI_ResetSubMenuInfo();
			if(TRUE == ubUI_FastStateFlag)
			{
				OSD_IMG_INFO tOsdInfo;

				ubUI_FastStateFlag = FALSE;
				UI_ClearBuConnectStatusFlag();
				tOsdInfo.uwHSize  = uwOSD_GetHSize();
				tOsdInfo.uwVSize  = uwOSD_GetVSize();
				tOsdInfo.uwXStart = 0;
				tOsdInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdInfo);
				tUI_State = UI_DISPLAY_STATE;
				break;
			}
			ubUI_RecSubMenuFlag = FALSE;
			UI_DrawMenuPage();
			break;
		}
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_PUPSMODE_STATE:
		case UI_SET_BUECOMODE_STATE:
		case UI_ENGMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_MULTIVIEWSEL_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
        case UI_RECTXFOLDER_SEL_STATE:
        case UI_RECTXFILES_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_PHOTOPLAYLIST_STATE:
		case UI_SDCARDFMT_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(EXIT_ARROW);
			break;
		default:
			break;
	}*/
}
//------------------------------------------------------------------------------
void UI_UpArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
        case UI_RECTXFOLDER_SEL_STATE:
        case UI_RECTXFILES_SEL_STATE:	
		case UI_ENGMODE_STATE:
		case UI_SPRF_SEL_STATE:	
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(UP_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DownArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_MULTIVIEWSEL_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
        case UI_RECTXFOLDER_SEL_STATE:            
        case UI_RECTXFILES_SEL_STATE:
		case UI_ENGMODE_STATE:
		case UI_SPRF_SEL_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(DOWN_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------


uint8_t demo_menu = 0;
void UI_LeftArrowKey(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	if (tUI_State == UI_ENGMODE_STATE)
		return;
	
	
	if (demo_menu < 6)
		 demo_menu++;
	else 
			demo_menu = 0;
	
	
	if (demo_menu == 1)
	{
		OSD_Weight(OSD_WEIGHT_8DIV8);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PU_STSICON, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	}
	else if (demo_menu == 2)
	{
		OSD_Weight(OSD_WEIGHT_8DIV8);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAM1_STSICON, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	}	
	else if (demo_menu == 3) //-----
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_MENU3_UP, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		
		OSD_Weight(OSD_WEIGHT_4DIV8);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_MENU3_DOWN, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_Transparency, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_A, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);			
	}	
	else if (demo_menu == 4)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_MENU3_UP, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		
		OSD_Weight(OSD_WEIGHT_5DIV8);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_MENU3_DOWN, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_Transparency, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_B, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);			
	}
	else if (demo_menu == 5)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_MENU3_UP, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		
		OSD_Weight(OSD_WEIGHT_6DIV8);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_MENU3_DOWN, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_Transparency, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_C, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);			
	}	
	else if (demo_menu == 6)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_MENU3_UP, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		
		OSD_Weight(OSD_WEIGHT_7DIV8);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_MENU3_DOWN, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_Transparency, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_N80_D, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);			
	}	
	else 
	{
//		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAM2_STSICON, 1, &tOsdImgInfo);
//		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
//		OSD_ClearImg2Buf();
		
		tOsdImgInfo.uwHSize  = 640;
		tOsdImgInfo.uwVSize  = 480;
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 0;		
		OSD_EraserImg2(&tOsdImgInfo);
		OSD_Weight(OSD_WEIGHT_8DIV8);
	}
	
	
	
	
	printf("demo_menu = %d\n", demo_menu);		//huang
	
//	switch(tUI_State)
//	{
//		case UI_DISPLAY_STATE:
//		case UI_MAINMENU_STATE:
//		case UI_SUBMENU_STATE:
//		case UI_SUBSUBMENU_STATE:
//		case UI_SUBSUBSUBMENU_STATE:
//		case UI_CAM_SEL_STATE:
//		case UI_SET_VDOMODE_STATE:
//		case UI_SET_ADOSRC_STATE:
//		case UI_SET_PUPSMODE_STATE:
//		case UI_SET_BUECOMODE_STATE:
//		case UI_CAMSETTINGMENU_STATE:
//		case UI_SET_CAMCOLOR_STATE:
//		case UI_DPTZ_CONTROL_STATE:
//		case UI_MD_WINDOW_STATE:
//		case UI_SDFWUPG_STATE:
//		case UI_RECFOLDER_SEL_STATE:
//		case UI_RECFILES_SEL_STATE:
//        case UI_RECTXFOLDER_SEL_STATE:		
//        case UI_RECTXFILES_SEL_STATE:
//		case UI_RECPLAYLIST_STATE:
//		case UI_RECPLAYADOSRC_SEL_STATE:
//		case UI_SDCARDFMT_STATE:
//			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
//				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(LEFT_ARROW);
//			break;
//		default:
//			break;
//	}
}
//------------------------------------------------------------------------------
void UI_RightArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_PUPSMODE_STATE:
		case UI_SET_BUECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
        case UI_RECTXFOLDER_SEL_STATE:
        case UI_RECTXFILES_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_SDCARDFMT_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(RIGHT_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_EnterKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_PUPSMODE_STATE:
		case UI_SET_BUECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_MULTIVIEWSEL_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
        case UI_RECTXFOLDER_SEL_STATE:
        case UI_RECTXFILES_SEL_STATE:	
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_SDCARDFMT_STATE:
		case UI_ENGMODE_STATE:
		case UI_PAIRING_STATE:
		case UI_SPRF_SEL_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(ENTER_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ShowColorSettingValue(uint8_t ubValue)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t ubYStart = 0;
	uint8_t ubUnits = 0, ubTens = 0, ubHunds = 0;

	ubHunds = ubValue / 100;
	ubTens  = (ubValue - (ubHunds * 100)) / 10;
	ubUnits = ubValue - (ubHunds * 100 + ubTens * 10);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORVALUEMASK_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	if(ubHunds)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = ubYStart = 530 - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = 530;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = ubYStart - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else if(ubTens)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = ubYStart = 543 - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = ubYStart - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = 530 - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu1Key(void)
{
//	if(UI_DISPLAY_STATE != tUI_State)
//	{
//		UI_MenuKey();
//		return;
//	}
//	if((DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum) && (APP_LINK_STATE == tUI_SyncAppState))
//	{
////		tCamPreViewSel.tCamViewType = tCamViewSel.tCamViewType;
//		UI_CameraSelectionKey();
//		return;
//	}
//#if APP_FS_FILE_LIST_STYLE
//	KNL_ThmShowInfo.ubInFldListFlg = 1;
//#endif
//	UI_DrawDCIMFolderMenu();
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu2Key(void)
{
	if((APP_LOSTLINK_STATE == tUI_SyncAppState) ||
	   (SINGLE_VIEW != tCamViewSel.tCamViewType) ||
	   (UI_DISPLAY_STATE != tUI_State))
		return;

	UI_DrawCameraSettingMenu(UI_CAMFUNC_SETUP);
}
//------------------------------------------------------------------------------
void UI_DrawColorSettingMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSET_BG, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRGB_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORBL_ITEM, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETUPNOR_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETDNNOR_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	UI_ShowColorSettingValue(tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorBL);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRIGHT_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawMDSettingScreen(void)
{
#define MD_H_WINDOWSIZE		48
#define MD_V_WINDOWSIZE		64
	OSD_IMG_INFO tOsdImgInfo[3];
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint8_t ubMD_V_WinNum = ulLcd_VSize / MD_V_WINDOWSIZE;
	uint8_t ubMD_H_WinNum = ulLcd_HSize / MD_H_WINDOWSIZE;
	uint8_t ubMD_Idx;

	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MDROWLINE, 2, &tOsdImgInfo[0]);
	for(ubMD_Idx = 0; ubMD_Idx < ubMD_H_WinNum; ubMD_Idx++)
	{
		tOsdImgInfo[0].uwXStart = (ubMD_Idx * MD_H_WINDOWSIZE);
		tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
	}
	tOsdImgInfo[0].uwXStart = ulLcd_HSize - 5;
	tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
	for(ubMD_Idx = 0; ubMD_Idx < ubMD_V_WinNum; ubMD_Idx++)
	{
		tOsdImgInfo[1].uwYStart = (ubMD_Idx * MD_V_WINDOWSIZE);
		tOSD_Img1(&tOsdImgInfo[1], OSD_QUEUE);
	}
	tOsdImgInfo[1].uwYStart = ulLcd_VSize - 5;
	tOSD_Img1(&tOsdImgInfo[1], OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MDBLOCK_ICON, 1, &tOsdImgInfo[2]);
	tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
	OSD_Weight(OSD_WEIGHT_6DIV8);
}
//------------------------------------------------------------------------------
void UI_DrawCameraSettingMenu(UI_CameraSettingMenu_t tCamSetMenu)
{
	OSD_IMG_INFO tOsdImgInfo;

	switch(tCamSetMenu)
	{
		case UI_CAMISP_SETUP:
			tUI_State = UI_SUBMENU_STATE;
//			tOsdImgInfo.uwXStart = 0;
//			tOsdImgInfo.uwYStart = 0;
//			tOsdImgInfo.uwHSize  = 100;
//			tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
//			OSD_EraserImg1(&tOsdImgInfo);
			OSD_Weight(OSD_WEIGHT_7DIV8);
			tUI_MenuItem.ubItemIdx = CAMERAS_ITEM;
			UI_DrawSubMenuPage(CAMERAS_ITEM);
			ubUI_FastStateFlag = TRUE;
			break;
		case UI_CAMFUNC_SETUP:
		{
			OSD_IMG_INFO tCamSetOsdImgInfo[6];

			tUI_State = UI_CAMSETTINGMENU_STATE;
			OSD_Weight(OSD_WEIGHT_8DIV8);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tOsdImgInfo);
			tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
			if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSCOLORMENUNOR_ITEM, 6, &tCamSetOsdImgInfo[0]) != OSD_OK)
			{
				printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
				return;
			}
			tOSD_Img2(&tCamSetOsdImgInfo[1], OSD_QUEUE);
			tOSD_Img2(&tCamSetOsdImgInfo[2], OSD_QUEUE);
			tOSD_Img2(&tCamSetOsdImgInfo[4], OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_SUBMENUICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu(UI_ArrowKey_t tArrowKey)
{
	static UI_CameraSettingItem_t tUI_CamSetItem = UI_COLOR_ITEM;
	UI_CameraSettingItem_t tUI_PrevCamSetItem = UI_DPTZ_ITEM;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwOsdImgIdx[3] = {OSD2IMG_CAMSCOLORMENUNOR_ITEM, OSD2IMG_CAMSDPTZMENUNOR_ITEM, OSD2IMG_CAMSMDV2MENUNOR_ITEM};

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(UI_COLOR_ITEM == tUI_CamSetItem)
				return;
			tUI_PrevCamSetItem = tUI_CamSetItem;
			--tUI_CamSetItem;
			break;
		case RIGHT_ARROW:
			if(UI_MD_ITEM == tUI_CamSetItem)
				return;
			tUI_PrevCamSetItem = tUI_CamSetItem;
			++tUI_CamSetItem;
			break;
		case ENTER_ARROW:
		case EXIT_ARROW:
			UI_ClearBuConnectStatusFlag();
			tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
			tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdImgInfo);
			if(ENTER_ARROW == tArrowKey)
			{
				if(UI_COLOR_ITEM == tUI_CamSetItem)
				{
					UI_DrawColorSettingMenu();
					tUI_State = UI_SET_CAMCOLOR_STATE;
					return;
				}
				if(UI_DPTZ_ITEM == tUI_CamSetItem)
				{
					uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
					uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
					uint8_t ubArrowNum;

					tUI_DptzParam.tScaleParam							 = UI_SCALEUP_2X;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = uwKNL_GetVdoV(ubKNL_SearchSrcByDispLoc(KNL_DISP_LOCATION1));
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = uwKNL_GetVdoH(ubKNL_SearchSrcByDispLoc(KNL_DISP_LOCATION1));
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize/tUI_DptzParam.tScaleParam;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize/tUI_DptzParam.tScaleParam;
					tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize	   	 = ulLcd_HSize;
					tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize	   	 = ulLcd_VSize;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
					tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
					for(ubArrowNum = 0; ubArrowNum < 4; ubArrowNum++)
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_DPTZUPARROWNOR_ICON+(ubArrowNum*2)), 1, &tOsdImgInfo);
						tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
					}
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DPTZZOOMMSG_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tUI_CamSetItem = UI_COLOR_ITEM;
					tUI_State = UI_DPTZ_CONTROL_STATE;
					return;
				}
				if(UI_MD_ITEM == tUI_CamSetItem)
				{
					UI_DrawMDSettingScreen();
					tUI_CamSetItem = UI_COLOR_ITEM;
					tUI_State = UI_MD_WINDOW_STATE;
					return;
				}
			}
			tUI_CamSetItem = UI_COLOR_ITEM;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwOsdImgIdx[tUI_CamSetItem]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx[tUI_PrevCamSetItem], 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_CameraColorSetting(UI_ArrowKey_t tArrowKey)
{
	static uint8_t ubUI_ColorSetItem = UI_IMGBL_SETTING;
	OSD_IMG_INFO tOsdImgInfo;
	UI_PUReqCmd_t tCamSetColorCmd;
	uint8_t *pUI_ColorParm[4] = {(uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorBL,
								 (uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorContrast,
								 (uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorSaturation,
								 (uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorHue};

	tCamSetColorCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
	tCamSetColorCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamSetColorCmd.ubCmd[UI_SETTING_ITEM]  = UI_IMGPROC_SETTING;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(UI_IMGBL_SETTING == ubUI_ColorSetItem)
				return;
			if(UI_IMGHUE_SETTING == ubUI_ColorSetItem)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRIGHTMASK_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwYStart = 1181;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRIGHT_ICON, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			--ubUI_ColorSetItem;
			break;
		case RIGHT_ARROW:
			if(UI_IMGHUE_SETTING == ubUI_ColorSetItem)
				return;
			if(++ubUI_ColorSetItem == UI_IMGHUE_SETTING)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRIGHTMASK_ICON, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORLEFT_ICON, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			break;
		case DOWN_ARROW:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETDNHL_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			if(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING] > 0)
			{
				tCamSetColorCmd.ubCmd[UI_SETTING_DATA]   = ubUI_ColorSetItem;
				tCamSetColorCmd.ubCmd[UI_SETTING_DATA+1] = --(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
				tCamSetColorCmd.ubCmd_Len				 = 4;
				if(UI_SendRequestToBU(osThreadGetId(), &tCamSetColorCmd) == rUI_SUCCESS)
				{
					UI_ShowColorSettingValue(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
					UI_UpdateDevStatusInfo();
				}
				else
					++(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETDNNOR_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			return;
		case UP_ARROW:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETUPHL_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			if(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING] < 127)
			{
				tCamSetColorCmd.ubCmd[UI_SETTING_DATA]   = ubUI_ColorSetItem;
				tCamSetColorCmd.ubCmd[UI_SETTING_DATA+1] = ++(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
				tCamSetColorCmd.ubCmd_Len				 = 4;
				if(UI_SendRequestToBU(osThreadGetId(), &tCamSetColorCmd) == rUI_SUCCESS)
				{
					UI_ShowColorSettingValue(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
					UI_UpdateDevStatusInfo();
				}
				else
					--(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETUPNOR_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			return;
		case EXIT_ARROW:
			UI_ClearBuConnectStatusFlag();
			tOsdImgInfo.uwXStart = 580;
			tOsdImgInfo.uwYStart = 0;
			tOsdImgInfo.uwHSize  = 140;
			tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_ColorSetItem = UI_IMGBL_SETTING;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	UI_ShowColorSettingValue(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_COLORBL_ITEM + (ubUI_ColorSetItem - UI_IMGBL_SETTING)), 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
UI_Result_t UI_DPTZ_KeyPress(uint8_t ubKeyID, uint8_t ubKeyMapIdx)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwUI_ArrowOsdImgIdx[] = {[AKEY_UP]    = OSD2IMG_DPTZUPARROWHL_ICON,
							          [AKEY_DOWN]  = OSD2IMG_DPTZDNARROWHL_ICON,
									  [AKEY_LEFT]  = OSD2IMG_DPTZLEFTARROWHL_ICON,
		                              [AKEY_RIGHT] = OSD2IMG_DPTZRIGHTARROWHL_ICON};

	if(ubKeyID == UiKeyEventMap[ubKeyMapIdx].ubKeyID)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwUI_ArrowOsdImgIdx[ubKeyID], 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		return rUI_SUCCESS;
	}
	return rUI_FAIL;
}
//------------------------------------------------------------------------------
void UI_DPTZ_KeyRelease(uint8_t ubKeyID)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwUI_ArrowOsdImgIdx[] = {[AKEY_UP]    = OSD2IMG_DPTZUPARROWNOR_ICON,
							          [AKEY_DOWN]  = OSD2IMG_DPTZDNARROWNOR_ICON,
									  [AKEY_LEFT]  = OSD2IMG_DPTZLEFTARROWNOR_ICON,
		                              [AKEY_RIGHT] = OSD2IMG_DPTZRIGHTARROWNOR_ICON};

	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwUI_ArrowOsdImgIdx[ubKeyID], 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DPTZ_Control(UI_ArrowKey_t tArrowKey)
{
#define PT_STEP		10
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubArrowNum;

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart == 0)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart -= PT_STEP;
			break;
		case DOWN_ARROW:
			if((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart + 
			    tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize) >= tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart += PT_STEP;
			break;
		case LEFT_ARROW:
			if((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart + 
			    tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize) >= tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart += PT_STEP;
			break;
		case RIGHT_ARROW:
			if(tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart == 0)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart -= PT_STEP;
			break;
		case ENTER_ARROW:
			if(tUI_DptzParam.tScaleParam == UI_SCALEUP_2X)
			{
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize /= UI_SCALEUP_2X;
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize /= UI_SCALEUP_2X;
				tUI_DptzParam.tScaleParam = UI_SCALEUP_4X;
			}
			else if(tUI_DptzParam.tScaleParam == UI_SCALEUP_4X)
			{
				uint16_t uwCropHsize     = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize*UI_SCALEUP_2X;
				uint16_t uwCropVsize     = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize*UI_SCALEUP_2X;
				uint16_t uwPrevCropHsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize;
				uint16_t uwPrevCropVsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize;
				if(tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart < ((uwCropHsize - uwPrevCropHsize)/2))
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart = 0;
				if((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart + ((uwCropVsize - uwPrevCropVsize)/2)) >= uwCropVsize)
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart = uwCropVsize;
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize = uwCropHsize;
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize = uwCropVsize;
				tUI_DptzParam.tScaleParam = UI_SCALEUP_2X;
			}
			break;
		case EXIT_ARROW:
		{
			UI_ClearStatusBarOsdIcon();
			for(ubArrowNum = 0; ubArrowNum < 4; ubArrowNum++)
			{
				if(ubArrowNum < 2)
				{
					tOsdImgInfo.uwXStart = 300;
					tOsdImgInfo.uwYStart = 40 + (ubArrowNum * 1080);
				}
				else
				{
					tOsdImgInfo.uwXStart = 40 + ((ubArrowNum - 2) * 570);
					tOsdImgInfo.uwYStart = 590;
				}
				tOsdImgInfo.uwHSize  = 100;
				tOsdImgInfo.uwVSize  = 110;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 20;
			tOsdImgInfo.uwHSize  = 50;
			tOsdImgInfo.uwVSize  = 125;
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_DptzParam.tScaleParam						   = UI_SCALEUP_2X;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart = 0;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart = 0;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize  = uwLCD_GetLcdHoSize();
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize  = uwLCD_GetLcdVoSize();
			tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
			tUI_State = UI_DISPLAY_STATE;
			return;
		}
		default:
			return;
	}
	tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
}
//------------------------------------------------------------------------------
void UI_MD_Window(UI_ArrowKey_t tArrowKey)
{
	uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
	uint8_t ubMD_V_WinNum = ulLcd_VSize / MD_V_WINDOWSIZE;
	uint8_t ubMD_H_WinNum = ulLcd_HSize / MD_H_WINDOWSIZE;
	static uint16_t uwMD_StartIdx = 0;
	static uint8_t ubMD_DownIdx = 0, ubMD_RightIdx = 0, ubMD_RightCnt = 0, ubMD_DownCnt = 0;
	static uint8_t ubMD_1stFlag = FALSE, ubMD_2ndFlag = FALSE;
	uint8_t ubMD_PrevDownIdx, ubMD_PrevRightIdx, i;
	OSD_IMG_INFO tOsdImgInfo[2];

	ubMD_PrevDownIdx  = ubMD_DownIdx;
	ubMD_PrevRightIdx = ubMD_RightIdx;
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(((TRUE == ubMD_1stFlag) && (FALSE == ubMD_2ndFlag)) ||
			   ((TRUE == ubMD_2ndFlag) && !ubMD_DownCnt))
				return;
			if(ubMD_DownIdx == 0)
				return;
			--ubMD_DownIdx;
			if(TRUE == ubMD_2ndFlag)
				--ubMD_DownCnt;
			break;
		case DOWN_ARROW:
			if((TRUE == ubMD_1stFlag) && (FALSE == ubMD_2ndFlag))
				return;
			if((ubMD_DownIdx + 1) == ubMD_H_WinNum)
				return;
			++ubMD_DownIdx;
			if(TRUE == ubMD_2ndFlag)
				++ubMD_DownCnt;
			break;
		case LEFT_ARROW:
			if(((TRUE == ubMD_1stFlag) && !ubMD_RightCnt) || (TRUE == ubMD_2ndFlag))
				return;
			if(ubMD_RightIdx == 0)
				return;
			--ubMD_RightIdx;
			if(TRUE == ubMD_1stFlag)
				--ubMD_RightCnt;
			break;
		case RIGHT_ARROW:
			if(((ubMD_RightIdx + 1) == ubMD_V_WinNum) || (TRUE == ubMD_2ndFlag))
				return;
			++ubMD_RightIdx;
			if(TRUE == ubMD_1stFlag)
				++ubMD_RightCnt;
			break;
		case ENTER_ARROW:
			if(TRUE == ubMD_2ndFlag)
			{
				UI_PUReqCmd_t tMdCmd;

				tMdCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
				tMdCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
				tMdCmd.ubCmd[UI_SETTING_ITEM]   = UI_MD_SETTING;
				tMdCmd.ubCmd[UI_SETTING_DATA]   = (uwMD_StartIdx & 0xFF);
				tMdCmd.ubCmd[UI_SETTING_DATA+1] = (uwMD_StartIdx >> 8);
				tMdCmd.ubCmd[UI_SETTING_DATA+2] = ubMD_RightCnt;
				tMdCmd.ubCmd[UI_SETTING_DATA+3] = ubMD_DownCnt;
				tMdCmd.ubCmd_Len  				= 6;
				if(UI_SendRequestToBU(osThreadGetId(), &tMdCmd) != rUI_SUCCESS)
				{
					printd(DBG_ErrorLvl, "MD Setting Fail !\n");
					return;
				}
			}
			else
			{
				if(FALSE == ubMD_1stFlag)
				{
					uwMD_StartIdx = ((ubMD_DownIdx * ubMD_V_WinNum) + ubMD_RightIdx);
					ubMD_1stFlag  = TRUE;
				}
				else if(FALSE == ubMD_2ndFlag)
				{
					ubMD_2ndFlag  = TRUE;
				}
				return;
			}
		case EXIT_ARROW:
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
			uwMD_StartIdx = ubMD_DownIdx  = ubMD_RightIdx = 0;
			ubMD_RightCnt = ubMD_DownCnt = 0;
			ubMD_1stFlag  = ubMD_2ndFlag = FALSE;
			tOsdImgInfo[1].uwXStart = 0;
			tOsdImgInfo[1].uwYStart = 0;
			tOsdImgInfo[1].uwHSize  = ulLcd_HSize;
			tOsdImgInfo[1].uwVSize  = ulLcd_VSize;
			OSD_EraserImg1(&tOsdImgInfo[1]);
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MDBLOCK_ICON, 1, &tOsdImgInfo[0]);
	if(TRUE == ubMD_2ndFlag)
	{
		uint16_t uwTmpYStart = 0;

		if(UP_ARROW == tArrowKey)
		{
			tOsdImgInfo[1].uwHSize  = tOsdImgInfo[0].uwHSize;
			tOsdImgInfo[1].uwVSize  = tOsdImgInfo[0].uwVSize;
			tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart + (ubMD_PrevDownIdx * MD_H_WINDOWSIZE);
			for(i = 0; i <= ubMD_RightCnt; i++)
			{
				tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart - ((ubMD_PrevRightIdx - i) * MD_V_WINDOWSIZE);
				OSD_EraserImg2(&tOsdImgInfo[1]);
			}
		}
		tOsdImgInfo[0].uwXStart += (ubMD_DownIdx * MD_H_WINDOWSIZE);
		for(i = 0; i <= ubMD_RightCnt; i++)
		{
			uwTmpYStart = tOsdImgInfo[0].uwYStart;
			tOsdImgInfo[0].uwYStart -= ((ubMD_RightIdx - i) * MD_V_WINDOWSIZE);
			tOSD_Img2(&tOsdImgInfo[0], (i == ubMD_RightCnt)?OSD_UPDATE:OSD_QUEUE);
			tOsdImgInfo[0].uwYStart = uwTmpYStart;
		}
		return;
	}
	if((FALSE == ubMD_1stFlag) || (LEFT_ARROW == tArrowKey))
	{
		tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart + (ubMD_PrevDownIdx * MD_H_WINDOWSIZE);
		tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart - (ubMD_PrevRightIdx * MD_V_WINDOWSIZE);
		tOsdImgInfo[1].uwHSize  = tOsdImgInfo[0].uwHSize;
		tOsdImgInfo[1].uwVSize  = tOsdImgInfo[0].uwVSize;
		OSD_EraserImg2(&tOsdImgInfo[1]);
	}
	tOsdImgInfo[0].uwXStart += (ubMD_DownIdx * MD_H_WINDOWSIZE);
	tOsdImgInfo[0].uwYStart -= (ubMD_RightIdx * MD_V_WINDOWSIZE);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_CameraSelectionKey(void)
{
	OSD_IMG_INFO tOsdImgInfo[(OSD2IMG_SELCAM2TDISABLE_ICON-OSD2IMG_SELCAM1ONLINE_ICON)+1] = {0};
	uint16_t uwDisplayImgIdx = 0, uwStartIdx;
	uint16_t uwXOffset  = 0;
	uint16_t uwYOffset  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?230:0;
	uint8_t ubSelItem;
	UI_CamNum_t tCamNum;
	static uint8_t ubUI_UpdateCamSelFlag = FALSE;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAM1ONLINE_ICON, (OSD2IMG_SELCAM2TDISABLE_ICON-OSD2IMG_SELCAM1ONLINE_ICON)+1, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	if(FALSE == ubUI_UpdateCamSelFlag)
	{
		tUI_CamNumSel = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(UI_CamNum_t)QUAD_TYPE_ITEM:(UI_CamNum_t)tUI_PuSetting.ubTotalBuNum;
		ubUI_UpdateCamSelFlag = TRUE;
	}
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		uwDisplayImgIdx = (((tCamNum == CAM4)?(tCamNum+1):tCamNum)*2) +
		                  //(((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) || (tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE))?1:0);
		                  ((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID)?1:0);
		tOsdImgInfo[uwDisplayImgIdx].uwYStart -= uwYOffset;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
	}
	if (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) {
		OSD_IMG_INFO tOsdImg4TInfo[4];

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SEL4TDUALONLINE_ICON, 4, &tOsdImg4TInfo[0]);
		if(IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType))
		{
			OSD_IMG_INFO tOsdImg3TInfo;
			uwDisplayImgIdx = OSD2IMG_SEL3T2L1RENNOR_ICON+(tCamViewSel.tCamViewType-TRIPLE_2L1R_VIEW);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwDisplayImgIdx, 1, &tOsdImg3TInfo);
			tOSD_Img2(&tOsdImg3TInfo, OSD_QUEUE);
		}
		else
		{
			uwStartIdx = OSD2IMG_SELCAM4TENABLE_ICON;
			uwDisplayImgIdx = (uwStartIdx - OSD2IMG_SELCAM1ONLINE_ICON);
			tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
		}
		tOSD_Img2(&tOsdImg4TInfo[((tUI_PuSetting.ubPairedBuNum >= 1)?0:1)], OSD_QUEUE);
		tOSD_Img2(&tOsdImg4TInfo[((tUI_PuSetting.ubPairedBuNum > 1)?2:3)], OSD_QUEUE);
		ubSelItem = ((tCamViewSel.tCamViewType == QUAD_VIEW) ||
		             (IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType)) ||
		             (TRIPLE_1T2B_VIEW == tCamViewSel.tCamViewType))?QUAD_TYPE_ITEM:
		             (tCamViewSel.tCamViewType == DUAL_VIEW)?DUAL_TYPE_ITEM:
					 (tCamViewSel.tCamViewType == SCAN_VIEW)?SCAN_TYPE_ITEM:tCamViewSel.tCamViewPool[0];
		tUI_CamNumSel = (UI_CamNum_t)ubSelItem;
	} else if(DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum) {
		uwStartIdx = OSD2IMG_SELCAM2TENABLE_ICON;
		uwDisplayImgIdx = (uwStartIdx - OSD2IMG_SELCAM1ONLINE_ICON);
		tOsdImgInfo[uwDisplayImgIdx].uwYStart -= 80;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
		ubSelItem = (tCamViewSel.tCamViewType == DUAL_VIEW)?CAM_2T:tCamViewSel.tCamViewPool[0];
		tUI_CamNumSel = (UI_CamNum_t)ubSelItem;
	} else {
		ubSelItem = tCamViewSel.tCamViewPool[0];
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo[0]);
	tOsdImgInfo[0].uwXStart += uwXOffset;
	tOsdImgInfo[0].uwYStart -= ((ubSelItem*111) + uwYOffset);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	tUI_State = UI_CAM_SEL_STATE;
	if(UI_REC_START == tUI_RecPlayAct.tRecAct)
	{
		OSD_IMG_INFO tRecOsdImgInfo[3];

		osDelay(20);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECORDINGNACT_ICON, 3, &tRecOsdImgInfo);
		tOSD_Img2(&tRecOsdImgInfo[1], OSD_QUEUE);
		tOSD_Img2(&tRecOsdImgInfo[2], OSD_UPDATE);
	}
}
//------------------------------------------------------------------------------
UI_Result_t tUI_SwitchViewType(UI_CamViewType_t tViewType)
{
	UI_Result_t tUI_ChkResult = rUI_SUCCESS;

	if(DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)
	{
		tCamViewSel.tCamViewPool[0] = (tUI_CamNumSel == CAM_2T)?(DISP_LEFT == tUI_CamStatus[CAM1].tCamDispLocation)?CAM1:CAM2:tUI_CamNumSel;
		tCamViewSel.tCamViewPool[1] = ((tCamViewSel.tCamViewPool[0] + 1) > CAM2)?CAM1:CAM2;
	}
	else
	{
		UI_CamNum_t tNextCamNum;

		tCamViewSel.tCamViewPool[0] = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel == DUAL_TYPE_ITEM)?tCamViewSel.tCamViewPool[0]:tUI_CamNumSel:CAM1;
		tNextCamNum = ((tUI_CamNumSel + 1) > CAM4)?CAM1:(UI_CamNum_t)(tUI_CamNumSel + 1);
		tCamViewSel.tCamViewPool[1] = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel == DUAL_TYPE_ITEM)?tCamViewSel.tCamViewPool[1]:tNextCamNum:tNextCamNum;
	}
	if(DUAL_TYPE_ITEM == tUI_CamNumSel)
	{
		UI_CameraSelection4DualView(ENTER_ARROW);
		return tUI_ChkResult;
	}
	else
		ubUI_DualViewExFlag = FALSE;
	tCamViewSel.tCamViewType = tViewType;
	if(SCAN_VIEW == tCamViewSel.tCamViewType)
	{
		tCamViewSel.tCamViewPool[0] = CAM1;
		tUI_ChkResult = UI_CheckCameraSource4SV();
		if((rUI_SUCCESS == tUI_ChkResult) && (UI_PHOTOCAP_MODE != tUI_PuSetting.tVdoMode))
		{
			tUI_PuSetting.RecInfo.tREC_Mode = REC_OFF;
			tUI_PuSetting.tVdoMode = UI_PHOTOCAP_MODE;
			UI_UpdateDevStatusInfo();
		}
	}
	if(rUI_SUCCESS == tUI_ChkResult)
	{
		if(IS_UI_DISP3T_VIEW(tViewType))
		{
			tCamViewSel.tCamViewPool[0] = CAM1;
			tCamViewSel.tCamViewPool[1] = CAM2;
			tCamViewSel.tCamViewPool[2] = CAM3;
			UI_SwitchCameraSource4TripleView();
		}
		else
			UI_SwitchCameraSource();
	}
	return tUI_ChkResult;
}
//------------------------------------------------------------------------------
void UI_CameraMultiViewSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	static UI_CamViewType_t tUI_CamMultiViewTypeSel = QUAD_VIEW;
	static uint8_t ubUI_CamMultiVwFlag = FALSE;
	uint16_t uwOsdImgIdx;

	if(DISPLAY_4T1R != tUI_PuSetting.ubTotalBuNum)
		return;
	if(FALSE == ubUI_CamMultiVwFlag)
	{
		tUI_CamMultiViewTypeSel = (tCamViewSel.tCamViewType < QUAD_VIEW)?QUAD_VIEW:tCamViewSel.tCamViewType;
		ubUI_CamMultiVwFlag = TRUE;
	}
	switch(tArrowKey)
	{
		case DOWN_ARROW:
			tUI_CamMultiViewTypeSel = (TRIPLE_3COL_VIEW == tUI_CamMultiViewTypeSel)?QUAD_VIEW:(UI_CamViewType_t)(tUI_CamMultiViewTypeSel+1);
			uwOsdImgIdx = OSD2IMG_SEL4TQUADENHL_ICON + (tUI_CamMultiViewTypeSel - QUAD_VIEW);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		case ENTER_ARROW:
			UI_CameraSelection(EXIT_ARROW);
		case EXIT_ARROW:
			tOsdImgInfo.uwXStart  = 53;
			tOsdImgInfo.uwYStart  = 0;
			tOsdImgInfo.uwHSize   = 95;
			tOsdImgInfo.uwVSize   = 350;
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_CamMultiVwFlag = FALSE;
			if(ENTER_ARROW == tArrowKey)
			{
				tUI_SwitchViewType(tUI_CamMultiViewTypeSel);
				tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
				UI_ClearStatusBarOsdIcon();
				break;
			}
			uwOsdImgIdx = (QUAD_VIEW == tCamViewSel.tCamViewType)?OSD2IMG_SELCAM4TENABLE_ICON:(OSD2IMG_SEL3T2L1RENNOR_ICON+(tUI_CamMultiViewTypeSel-TRIPLE_2L1R_VIEW));
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwYStart -= (QUAD_TYPE_ITEM*111);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_State = UI_CAM_SEL_STATE;
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSelection(UI_ArrowKey_t tArrowKey)
{
	UI_CamNum_t tPreCamNum = tUI_CamNumSel;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwXOffset  = 0;
	uint16_t uwYOffset  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?230:0;
	UI_CamViewType_t tCamViewTypeSel;
	uint8_t ubWarnNoteShowFlag = FALSE;

	if(DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum)
		return;

	tPreCamNum = tUI_CamNumSel;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
		case RIGHT_ARROW:
			tUI_CamNumSel = UI_ChangeSelectCamNum4UiMenu(&tPreCamNum, &tArrowKey);
			if(tUI_CamNumSel == NO_CAM)
			{
				tUI_CamNumSel = tPreCamNum;
				break;
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwXOffset;
			tOsdImgInfo.uwYStart -= ((tPreCamNum*111) + uwYOffset);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwXOffset;
			tOsdImgInfo.uwYStart -= ((tUI_CamNumSel*111) + uwYOffset);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		case DOWN_ARROW:
#if APP_FS_FILE_LIST_STYLE
			KNL_ThmShowInfo.ubInFldListFlg = 1;
#endif		
			if((SINGLE_VIEW == tCamViewSel.tCamViewType) || (SCAN_VIEW == tCamViewSel.tCamViewType))
			{
				UI_DrawDCIMFolderMenu();
			}
			else
			{
				tOsdImgInfo.uwXStart  = 100;
				tOsdImgInfo.uwYStart  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?400:0;
				tOsdImgInfo.uwHSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?300:255;
				tOsdImgInfo.uwVSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?500:uwLCD_GetLcdVoSize();
				OSD_EraserImg2(&tOsdImgInfo);
				UI_ChangeAudioSourceKey();
			}
			break;
		case ENTER_ARROW:
			tCamViewTypeSel = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel == QUAD_TYPE_ITEM)?QUAD_VIEW:
																	       (tUI_CamNumSel == DUAL_TYPE_ITEM)?DUAL_VIEW:/*tCamPreViewSel.tCamViewType:*/
																		   (tUI_CamNumSel == SCAN_TYPE_ITEM)?SCAN_VIEW:SINGLE_VIEW:
							  (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel == CAM_2T)?DUAL_VIEW:SINGLE_VIEW:SINGLE_VIEW;
//			if((tCamViewSel.tCamViewType != tCamViewTypeSel) || ((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (DUAL_VIEW == tCamViewTypeSel)))
			{
				if(UI_REC_START == tUI_RecPlayAct.tRecAct)
				{
					ubWarnNoteShowFlag = TRUE;
				}
				else
				{
					if((tUI_CamNumSel == SCAN_TYPE_ITEM) && (tCamViewSel.tCamViewType == SCAN_VIEW))
						goto EXIT_CAMSELECT_MENU;
					if(SCAN_VIEW == tCamViewSel.tCamViewType)
						UI_DisableScanMode();
					if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (UI_CAM_SEL_STATE == tUI_State) &&
					   ((IS_UI_DISP3T_VIEW(tCamViewTypeSel)) || (QUAD_VIEW == tCamViewTypeSel)))
					{
						OSD_IMG_INFO tOsdMultiVwImgInfo[8];
						uint8_t ubImgIdx = 0;

						ubImgIdx = (       QUAD_VIEW == tCamViewSel.tCamViewType)?2:
								   (TRIPLE_2L1R_VIEW == tCamViewSel.tCamViewType)?3:
							       (TRIPLE_1L2R_VIEW == tCamViewSel.tCamViewType)?4:
								   (TRIPLE_2T1B_VIEW == tCamViewSel.tCamViewType)?5:
								   (TRIPLE_1T2B_VIEW == tCamViewSel.tCamViewType)?6:
						           (TRIPLE_3COL_VIEW == tCamViewSel.tCamViewType)?7:2;
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MULTIVIEWSELINFO_ICON, 8, &tOsdMultiVwImgInfo);
						tOSD_Img2(&tOsdMultiVwImgInfo[ubImgIdx], OSD_QUEUE);
						tOSD_Img2(&tOsdMultiVwImgInfo[0], OSD_QUEUE);
						tOSD_Img2(&tOsdMultiVwImgInfo[1], OSD_UPDATE);
						tUI_State = UI_MULTIVIEWSEL_STATE;
						break;
					}
					tUI_SwitchViewType(tCamViewTypeSel);
					if((DISPLAY_2T1R != tUI_PuSetting.ubTotalBuNum) && (DUAL_VIEW == tCamViewTypeSel))
						break;
				}
			}
		case EXIT_ARROW:
EXIT_CAMSELECT_MENU:
			tOsdImgInfo.uwXStart  = 100;
			tOsdImgInfo.uwYStart  = 0;
			tOsdImgInfo.uwHSize   = 255;
			tOsdImgInfo.uwVSize   = uwLCD_GetLcdVoSize();
			if(ENTER_ARROW == tArrowKey)
			{
				if(TRUE == ubWarnNoteShowFlag)
				{
					OSD_EraserImg2(&tOsdImgInfo);
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECORDINGWARN_ICON, 1, &tOsdImgInfo);
					osDelay(80);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tUI_PuSetting.WarnIcon.tWarnNote 	   = UI_WARN_RECMODE;
					tUI_PuSetting.WarnIcon.ubWarnUpdateCnt = UI_WARNINGNOTE_PERIOD;
					tUI_State = UI_SHOWSTSICON_STATE;
					ubWarnNoteShowFlag = FALSE;
					break;
				}
				else
				{
					tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
					UI_ClearStatusBarOsdIcon();
				}
			}
			UI_ClearBuConnectStatusFlag();
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = UI_DISPLAY_STATE;
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSelection4DualView(UI_ArrowKey_t tArrowKey)
{
	static UI_DualViewCamSel_t tCamSelIdx = DUAL_CAM1_CAM2;
	OSD_IMG_INFO tDualSelOsdImgInfo[8];
	UI_Result_t tUI_ChkResult = rUI_SUCCESS;
	uint16_t uwUI_DualCamSelIdx = 0;
	uint8_t ubUI_CamSelTable[DUAL_VIEWCAMSEL_MAX] = {[DUAL_CAM1_CAM2] = 0x1,
													 [DUAL_CAM1_CAM3] = 0x2,
													 [DUAL_CAM1_CAM4] = 0x3,
													 [DUAL_CAM2_CAM1] = 0x10,
													 [DUAL_CAM2_CAM3] = 0x12,
													 [DUAL_CAM2_CAM4] = 0x13,
													 [DUAL_CAM3_CAM1] = 0x20,
													 [DUAL_CAM3_CAM2] = 0x21,
													 [DUAL_CAM3_CAM4] = 0x23,
													 [DUAL_CAM4_CAM1] = 0x30,
													 [DUAL_CAM4_CAM2] = 0x31,
													 [DUAL_CAM4_CAM3] = 0x32};	
	if(UI_DUALVIEW_CAMSEL_STATE != tUI_State)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DUALVIEWSELWIN, 8, &tDualSelOsdImgInfo[0]);
		tOSD_Img2(&tDualSelOsdImgInfo[0], OSD_QUEUE);
		uwUI_DualCamSelIdx = (TRUE == ubUI_DualViewExFlag)?((ubUI_CamSelTable[tCamSelIdx] >> 4) + 1):1;
		tOSD_Img2(&tDualSelOsdImgInfo[uwUI_DualCamSelIdx], OSD_QUEUE);
		uwUI_DualCamSelIdx = (TRUE == ubUI_DualViewExFlag)?((ubUI_CamSelTable[tCamSelIdx] & 0xF) + 1):2;
		tDualSelOsdImgInfo[uwUI_DualCamSelIdx].uwYStart -= 48;
		tOSD_Img2(&tDualSelOsdImgInfo[uwUI_DualCamSelIdx], OSD_QUEUE);
		tCamSelIdx = (TRUE == ubUI_DualViewExFlag)?tCamSelIdx:DUAL_CAM1_CAM2;
		tOSD_Img2(&tDualSelOsdImgInfo[6], OSD_QUEUE);
		tOSD_Img2(&tDualSelOsdImgInfo[7], OSD_UPDATE);
		tUI_State = UI_DUALVIEW_CAMSEL_STATE;
		return;
	}
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(DUAL_CAM1_CAM2 == tCamSelIdx)
				return;
			--tCamSelIdx;
			break;
		case DOWN_ARROW:
			if(DUAL_CAM4_CAM3 == tCamSelIdx)
				return;
			++tCamSelIdx;
			break;
		case ENTER_ARROW:
			tUI_ChkResult = ((tCamViewSel.tCamViewPool[0] == (UI_CamNum_t)(ubUI_CamSelTable[tCamSelIdx] >> 4)) &&
							 (tCamViewSel.tCamViewPool[1] == (UI_CamNum_t)(ubUI_CamSelTable[tCamSelIdx] & 0xF)) &&
		                     (DUAL_VIEW == tCamViewSel.tCamViewType))?rUI_FAIL:rUI_SUCCESS;
			tCamViewSel.tCamViewType = DUAL_VIEW;
			if(rUI_SUCCESS == tUI_ChkResult)
			{
				tCamViewSel.tCamViewPool[0] = (UI_CamNum_t)(ubUI_CamSelTable[tCamSelIdx] >> 4);
				tCamViewSel.tCamViewPool[1] = (UI_CamNum_t)(ubUI_CamSelTable[tCamSelIdx] & 0xF);
				UI_SwitchCameraSource();
				ubUI_DualViewExFlag = TRUE;
			}
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			UI_ClearStatusBarOsdIcon();
			UI_ClearBuConnectStatusFlag();
			tDualSelOsdImgInfo[0].uwXStart	= 50;
			tDualSelOsdImgInfo[0].uwYStart	= 0;
			tDualSelOsdImgInfo[0].uwHSize	= 305;
			tDualSelOsdImgInfo[0].uwVSize	= uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tDualSelOsdImgInfo[0]);
			tUI_State = UI_DISPLAY_STATE;
			return;
		case EXIT_ARROW:
			tDualSelOsdImgInfo[0].uwXStart  = 50;
			tDualSelOsdImgInfo[0].uwYStart  = 0;
			tDualSelOsdImgInfo[0].uwHSize   = 100;
			tDualSelOsdImgInfo[0].uwVSize   = uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tDualSelOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DUALVIEWSELDELWIN, 1, &tDualSelOsdImgInfo[0]);
			tOSD_Img2(&tDualSelOsdImgInfo[0], OSD_UPDATE);
			tUI_State = UI_CAM_SEL_STATE;
			return;
		default:
			break;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DUALVIEWSELCAM1, 4, &tDualSelOsdImgInfo[0]);
	uwUI_DualCamSelIdx = (ubUI_CamSelTable[tCamSelIdx] >> 4);
	tOSD_Img2(&tDualSelOsdImgInfo[uwUI_DualCamSelIdx], OSD_QUEUE);
	uwUI_DualCamSelIdx = (ubUI_CamSelTable[tCamSelIdx] & 0xF);
	tDualSelOsdImgInfo[uwUI_DualCamSelIdx].uwYStart -= 48;
	tOSD_Img2(&tDualSelOsdImgInfo[uwUI_DualCamSelIdx], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_ChangeAudioSourceKey(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwDisp2TImgIdx[4] = {OSD2IMG_SELADOCAM1ONLINE_ICON,   OSD2IMG_SELADOCAM1ONLINE_ICON,
								  OSD2IMG_SELADOCAM2_1ONLINE_ICON, OSD2IMG_SELADOCAM2_1OFFLINE_ICON};
	uint16_t uwDisp4TImgIdx[8] = {OSD2IMG_SELADOCAM1ONLINE_ICON, OSD2IMG_SELADOCAM1OFFLINE_ICON,
								  OSD2IMG_SELADOCAM2ONLINE_ICON, OSD2IMG_SELADOCAM2OFFLINE_ICON,
								  OSD2IMG_SELADOCAM3ONLINE_ICON, OSD2IMG_SELADOCAM3OFFLINE_ICON,
								  OSD2IMG_SELADOCAM4ONLINE_ICON, OSD2IMG_SELADOCAM4OFFLINE_ICON};
	uint16_t uwDisplayImgIdx;
	uint16_t uwXOffset = 0, uwYOffset = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?150:0;
	uint16_t uwYItemOffset = 150;
	UI_CamNum_t tCamNum;

	if((APP_LOSTLINK_STATE == tUI_SyncAppState) || (tUI_State != UI_CAM_SEL_STATE) ||
	   (DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum))
		return;

	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		uwDisplayImgIdx  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?uwDisp2TImgIdx[tCamNum*2]:uwDisp4TImgIdx[tCamNum*2];
		uwDisplayImgIdx += (((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) || (tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE))?1:0);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwDisplayImgIdx, 1, &tOsdImgInfo);
		tOsdImgInfo.uwYStart -= uwYOffset;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tUI_PuSetting.tAdoSrcCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_State = UI_SET_ADOSRC_STATE;
}
//------------------------------------------------------------------------------
void UI_ChangeAudioSource(UI_ArrowKey_t tArrowKey)
{
	static UI_CamNum_t tAdoCamNumSel;
	static uint8_t ubUI_UpdateAdoCamSelFlag = FALSE;
	UI_CamNum_t tPreAdoCamNum = tAdoCamNumSel;
	uint16_t uwXOffset = 0, uwYOffset = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?150:0;
	uint16_t uwYItemOffset = 150;
	uint8_t ubCamNum;
	OSD_IMG_INFO tOsdImgInfo;

	if(FALSE == ubUI_UpdateAdoCamSelFlag)
	{
		tAdoCamNumSel = tUI_PuSetting.tAdoSrcCamNum;
		ubUI_UpdateAdoCamSelFlag = TRUE;
	}
	tPreAdoCamNum = tAdoCamNumSel;
	switch(tArrowKey)
	{
		case UP_ARROW:
			UI_CameraSelectionKey();
			return;
		case DOWN_ARROW:
			if(SINGLE_VIEW != tCamViewSel.tCamViewType)
				UI_DrawDCIMFolderMenu();	//!UI_DrawCameraSettingMenu(UI_CAMISP_SETUP);
			ubUI_UpdateAdoCamSelFlag = FALSE;
			return;
		case LEFT_ARROW:
			if(tAdoCamNumSel == CAM1)
				return;
			for(ubCamNum = tAdoCamNumSel; ubCamNum > CAM1; ubCamNum--)
			{
				if((tUI_CamStatus[ubCamNum - 1].ulCAM_ID != INVALID_ID) &&
				   (tUI_CamStatus[ubCamNum - 1].tCamConnSts == CAM_ONLINE))
				{
					tAdoCamNumSel = (UI_CamNum_t)(ubCamNum - 1);
					break;
				}
				if((ubCamNum - 1) == CAM1)
					return;
			}
			break;
		case RIGHT_ARROW:
			if((tAdoCamNumSel + 1) >= tUI_PuSetting.ubTotalBuNum)
				return;
			for(ubCamNum = (tAdoCamNumSel + 1); ubCamNum < tUI_PuSetting.ubTotalBuNum; ubCamNum++)
			{
				if((tUI_CamStatus[ubCamNum].ulCAM_ID != INVALID_ID) &&
				   (tUI_CamStatus[ubCamNum].tCamConnSts == CAM_ONLINE))
				{
					tAdoCamNumSel = (UI_CamNum_t)ubCamNum;
					break;
				}
			}
			if(ubCamNum == tUI_PuSetting.ubTotalBuNum)
				return;
			break;
		case ENTER_ARROW:
			UI_SwitchAudioSource(tAdoCamNumSel);
			UI_UpdateDevStatusInfo();
		case EXIT_ARROW:
			if(ENTER_ARROW == tArrowKey)
				UI_ClearStatusBarOsdIcon();
			UI_ClearBuConnectStatusFlag();
			tOsdImgInfo.uwXStart  = 100;
			tOsdImgInfo.uwYStart  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?450:0;
			tOsdImgInfo.uwHSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?300:255;
			tOsdImgInfo.uwVSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?335:uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_UpdateAdoCamSelFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tPreAdoCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tAdoCamNumSel*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
#define UI_MENUICON_NUM		7
#define UI_WRICON_OFFSET	2
//------------------------------------------------------------------------------
void UI_ChangeVideoMode(UI_ArrowKey_t tArrowKey)
{
	static UI_VdoModeList_t tUI_VdoModeSet = UI_VDOPHOTO_MODE;
	static UI_VdoModeList_t tVdoModeSel = UI_VDOPHOTO_MODE;
	static uint8_t ubUI_UpdateVdoModeFlag = FALSE;
	UI_VdoModeList_t tPreVdoMode = UI_VDOPHOTO_MODE;
	uint16_t uwVdoModeOsdImg[UI_VDOMODELIST_MAX] = {OSD2IMG_RECLOOPMODENOR_ICON, OSD2IMG_RECMANUMODENOR_ICON,
												    OSD2IMG_RECTRIGMODENOR_ICON, OSD2IMG_PHOTOSHOOTMODENOR_ICON};
	OSD_IMG_INFO tOsdImgInfo;

	if(FALSE == ubUI_UpdateVdoModeFlag)
	{
		tUI_VdoModeSet = tVdoModeSel = tPreVdoMode = (UI_PHOTOCAP_MODE == tUI_PuSetting.tVdoMode)?UI_VDOPHOTO_MODE:
												     (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode)?UI_VDORECLOOP_MODE:
												     (REC_MANUAL  == tUI_PuSetting.RecInfo.tREC_Mode)?UI_VDORECMANU_MODE:UI_VDORECTRIG_MODE;
		ubUI_UpdateVdoModeFlag = TRUE;
	}
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(tVdoModeSel == UI_VDORECLOOP_MODE)
				return;
			tPreVdoMode = tVdoModeSel;
			tVdoModeSel--;
			break;
		case RIGHT_ARROW:
			if(tVdoModeSel == UI_VDOPHOTO_MODE)
				return;
			tPreVdoMode = tVdoModeSel;
			tVdoModeSel++;
			break;
		case ENTER_ARROW:
			if(tVdoModeSel == UI_VDORECTRIG_MODE)
				return;
			if((tUI_VdoModeSet != tVdoModeSel) &&
			   ((UI_VDOPHOTO_MODE == tVdoModeSel) || (SCAN_VIEW != tCamViewSel.tCamViewType)))
			{
			#if (!APP_SD_FUNC_ENABLE || !APP_REC_FUNC_ENABLE)
				if((UI_VDORECLOOP_MODE == tVdoModeSel) || (UI_VDORECMANU_MODE == tVdoModeSel))
					return;
			#endif
			#if !APP_PHOTOGRAPH_FUNC_ENABLE
				if(UI_VDOPHOTO_MODE == tVdoModeSel)
					return;
			#endif
				if(UI_RECORDING_MODE == tUI_PuSetting.tVdoMode)
				{
					ubUI_VdoRecChkFlag = ((UI_REC_STOP == tUI_RecPlayAct.tRecAct) && (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode))?FALSE:TRUE;
					UI_VideoRecordingExec(UI_REC_STOP);
				}
				tUI_PuSetting.RecInfo.tREC_Mode = (UI_VDORECLOOP_MODE == tVdoModeSel)?REC_LOOPING:
												  (UI_VDORECMANU_MODE == tVdoModeSel)?REC_MANUAL:
												  (UI_VDORECTRIG_MODE == tVdoModeSel)?REC_TRIGGER:REC_OFF;
				tUI_PuSetting.tVdoMode = (REC_OFF == tUI_PuSetting.RecInfo.tREC_Mode)?UI_PHOTOCAP_MODE:UI_RECORDING_MODE;
				if((UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode))
					UI_VideoRecordingExec(UI_REC_START);
                else
                    KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
				tUI_VdoModeSet = tVdoModeSel;
				UI_UpdateDevStatusInfo();
			}
		case EXIT_ARROW:
			if(UI_SDCARDFMT_STATE != tUI_State)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PHOTOSHOOTMODEHL_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwHSize = 110;
				tOsdImgInfo.uwVSize = 650;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			tUI_State = (UI_SET_VDOMODE_STATE == tUI_State)?UI_DISPLAY_STATE:tUI_State;
			ubUI_UpdateVdoModeFlag = FALSE;
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwVdoModeOsdImg[tPreVdoMode]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
	OSD_EraserImg2(&tOsdImgInfo);
	UI_DrawHLandNormalIcon(uwVdoModeOsdImg[tPreVdoMode], (uwVdoModeOsdImg[tVdoModeSel]+UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_PuPowerSaveKey(void)
{
	OSD_IMG_INFO tOsdImgInfo[6];

	if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
	{
		UI_DisableVox();
		return;
	}
	if(PS_ADOONLY_MODE == tUI_PuSetting.tPsMode)
	{
		UI_DisablePuAdoOnlyMode();
		return;
	}
	if((APP_LOSTLINK_STATE == tUI_SyncAppState)
	|| (tUI_State != UI_DISPLAY_STATE)
//	|| (DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum)
	|| (PS_VOX_MODE == tUI_PuSetting.tPsMode))
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_VOXOPT_ICON, 6, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
#ifdef A7130
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
#else
	tOSD_Img2(&tOsdImgInfo[3], OSD_QUEUE);
#endif
	tOSD_Img2(&tOsdImgInfo[5], OSD_UPDATE);
	tUI_State = UI_SET_PUPSMODE_STATE;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupPuVoxMode(void)
{
	UI_PUReqCmd_t tPsCmd;
	UI_CamNum_t tCamNum;
	UI_Result_t tVoxRet = rUI_FAIL, tBuNotifyRet = rUI_SUCCESS;

	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
			continue;
		if(CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts)
			continue;
		tPsCmd.tDS_CamNum 				= tCamNum;
		tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
		tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_VOX_MODE;
		tPsCmd.ubCmd_Len  				= 3;
		tBuNotifyRet = UI_SendRequestToBU(osThreadGetId(), &tPsCmd);
		if(rUI_SUCCESS == tBuNotifyRet)
			tVoxRet = rUI_SUCCESS;
		else
			printd(DBG_ErrorLvl, "CAM%d:VOX Notify Fail !\n", (tCamNum + 1));
		tUI_CamStatus[tCamNum].tCamPsMode = PS_VOX_MODE;
	}
	if(rUI_SUCCESS == tVoxRet)
		UI_EnableVox();
	return tVoxRet;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupPuAdoOnlyMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	LCDBL_ENABLE(UI_DISABLE);
	UI_DisableScanMode();
	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_ADOONLY_MODE;
	tUI_PsMessage.ubAPP_Message[2] = TRUE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_PuSetting.tPsMode = PS_ADOONLY_MODE;
	UI_EnableScanMode();

	return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupPuWorMode(void)
{
	UI_PUReqCmd_t tPsCmd;
	UI_CamNum_t tCamNum;
	UI_Result_t tWorRet = rUI_FAIL, tBuNotifyRet = rUI_SUCCESS;

	tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_WORMODE_SETTING;
	tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_WOR_MODE;
	tPsCmd.ubCmd_Len  				= 3;
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		if(CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts)
			continue;
		tPsCmd.tDS_CamNum = tCamNum;
		tBuNotifyRet = UI_SendRequestToBU(osThreadGetId(), &tPsCmd);
		if(rUI_SUCCESS == tBuNotifyRet)
			tWorRet = rUI_SUCCESS;
		else
			printd(DBG_ErrorLvl, "CAM%d:WOR Setting Fail !\n", (tCamNum + 1));
	}
	if(rUI_SUCCESS == tWorRet)
	{
		APP_EventMsg_t tUI_PsMessage = {0};

		tUI_PuSetting.tPsMode = PS_WOR_MODE;
		UI_UpdateDevStatusInfo();
		tUI_PsMessage.ubAPP_Event 	    = APP_POWERSAVE_EVENT;
		tUI_PsMessage.ubAPP_Message[0]  = 3;		//! Message Length
		tUI_PsMessage.ubAPP_Message[1]  = PS_WOR_MODE;
		tUI_PsMessage.ubAPP_Message[2]  = FALSE;
		tUI_PsMessage.ubAPP_Message[3]  = CAM1;
		UI_SendMessageToAPP(&tUI_PsMessage);
	}
	return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
static uint8_t ubUI_WakeUpFromPsFlag = FALSE;
UI_Result_t UI_SetupBuEcoMode(UI_CamNum_t tECO_CamNum)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	tUI_PsMessage.ubAPP_Event 	    = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0]  = 4;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1]  = PS_ECO_MODE;
	if((POWER_NORMAL_MODE == tUI_CamStatus[tECO_CamNum].tCamPsMode) &&
	   (FALSE == ulUI_MonitorPsFlag[tECO_CamNum]))
	{
		UI_PUReqCmd_t tPsCmd;

		tPsCmd.tDS_CamNum 				= tECO_CamNum;
		tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_ECOMODE_SETTING;
		tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_ECO_MODE;
		tPsCmd.ubCmd_Len  				= 3;
		if(tUI_CamStatus[tECO_CamNum].tCamConnSts == CAM_ONLINE)
		{
			UI_SendRequestToBU(NULL, &tPsCmd);
			tUI_CamStatus[tECO_CamNum].tCamPsMode = PS_ECO_MODE;
			UI_UpdateDevStatusInfo();
			tUI_PsMessage.ubAPP_Message[2]  = FALSE;
			tUI_PsMessage.ubAPP_Message[3]  = tECO_CamNum;
			tUI_PsMessage.ubAPP_Message[4]  = FALSE;
			UI_SendMessageToAPP(&tUI_PsMessage);
		}
	}
	else if(PS_ECO_MODE == tUI_CamStatus[tECO_CamNum].tCamPsMode)
	{
		tUI_PsMessage.ubAPP_Message[2]  = TRUE;
		tUI_PsMessage.ubAPP_Message[3]  = tECO_CamNum;
		tUI_PsMessage.ubAPP_Message[4]  = TRUE;
		UI_SendMessageToAPP(&tUI_PsMessage);
		ubUI_WakeUpFromPsFlag = TRUE;
	}
	
	return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
void UI_BuPowerSaveKey(void)
{
#ifdef A7130
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwDisp2TImgIdx[4] = {OSD2IMG_SELECOCAM1ONLINE_ICON,   OSD2IMG_SELECOCAM1ONLINE_ICON,
								  OSD2IMG_SELECOCAM2_1ONLINE_ICON, OSD2IMG_SELECOCAM2_1OFFLINE_ICON};
	uint16_t uwDisp4TImgIdx[8] = {OSD2IMG_SELECOCAM1ONLINE_ICON, OSD2IMG_SELECOCAM1OFFLINE_ICON,
								  OSD2IMG_SELECOCAM2ONLINE_ICON, OSD2IMG_SELECOCAM2OFFLINE_ICON,
								  OSD2IMG_SELECOCAM3ONLINE_ICON, OSD2IMG_SELECOCAM3OFFLINE_ICON,
								  OSD2IMG_SELECOCAM4ONLINE_ICON, OSD2IMG_SELECOCAM4OFFLINE_ICON};
	uint16_t uwDisplayImgIdx;
	uint16_t uwXOffset = 5, uwYOffset = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?155:0;
	uint16_t uwYItemOffset = 150, uwYEcoOffset = 150;
	UI_CamNum_t tCamNum;

	if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
	{
		UI_DisableVox();
		return;
	}
	if(PS_ADOONLY_MODE == tUI_PuSetting.tPsMode)
	{
		UI_DisablePuAdoOnlyMode();
		return;
	}
	if((UI_DISPLAY_STATE != tUI_State) ||
	   (TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag))
		return;
	if((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
	   (SINGLE_VIEW == tCamViewSel.tCamViewType))
	{
		UI_SetupBuEcoMode(tCamViewSel.tCamViewPool[0]);
		return;
	}
	tUI_BuEcoCamNum = NO_CAM;
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		uwYEcoOffset     = 0;
		uwDisplayImgIdx  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?uwDisp2TImgIdx[tCamNum*2]:uwDisp4TImgIdx[tCamNum*2];
		if((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) ||
		   ((tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE) && (PS_ECO_MODE != tUI_CamStatus[tCamNum].tCamPsMode)))
		{
			uwDisplayImgIdx = ((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum) && (CAM2 == tCamNum))?OSD2IMG_SELCAM2_1OFFLINE_ICON:(uwDisplayImgIdx+1);
		}
		else
		{
			if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
			{
				uwDisplayImgIdx = ((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum) && (CAM2 == tCamNum))?OSD2IMG_SELCAM2_1ONLINE_ICON:(uwDisplayImgIdx-30);
				uwYEcoOffset    = (((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum) && (CAM2 == tCamNum)) ||
								   ((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (CAM4 == tCamNum)))?0:150;
			}
			tUI_BuEcoCamNum = (NO_CAM == tUI_BuEcoCamNum)?tCamNum:tUI_BuEcoCamNum;
		}
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwDisplayImgIdx, 1, &tOsdImgInfo);	
		tOsdImgInfo.uwYStart -= (uwYOffset + uwYEcoOffset);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	if(NO_CAM != tUI_BuEcoCamNum)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart += uwXOffset;
		tOsdImgInfo.uwYStart -= ((tUI_BuEcoCamNum*111) + uwYOffset + uwYItemOffset);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	}
	tUI_State = UI_SET_BUECOMODE_STATE;
#endif
}
//------------------------------------------------------------------------------
void UI_PuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	static UI_PowerSaveMode_t tUI_PsMode = PS_VOX_MODE;
#ifdef S2019A
	UI_PowerSaveMode_t tUI_PrePsMode = PS_ADOONLY_MODE;
#else
	UI_PowerSaveMode_t tUI_PrePsMode = PS_WOR_MODE;
#endif
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(PS_VOX_MODE == tUI_PsMode)
				return;
			tUI_PrePsMode = tUI_PsMode;
			tUI_PsMode--;
			break;
		case RIGHT_ARROW:
#ifdef S2019A
			if(PS_ADOONLY_MODE == tUI_PsMode)
#else
			if(PS_WOR_MODE == tUI_PsMode)
#endif
				return;
			tUI_PrePsMode = tUI_PsMode;			
			tUI_PsMode++;
			break;
		case ENTER_ARROW:
			if((tUI_PsMode <= PS_WOR_MODE) && (UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (UI_REC_START == tUI_RecPlayAct.tRecAct))
        		UI_VideoRecordingExec(UI_REC_STOP);
			if(PS_VOX_MODE == tUI_PsMode)
			{
				tUI_PsMode = (rUI_SUCCESS != UI_SetupPuVoxMode())?POWER_NORMAL_MODE:tUI_PsMode;
			}
			else if(PS_ADOONLY_MODE == tUI_PsMode)
			{
				UI_SetupPuAdoOnlyMode();
			}
#ifdef A7130
			else if(PS_WOR_MODE == tUI_PsMode)
			{
				UI_SetupPuWorMode();
			}
#endif
		case EXIT_ARROW:
			UI_ClearBuConnectStatusFlag();
			tOsdImgInfo.uwXStart = 153;
			tOsdImgInfo.uwYStart = 300;
			tOsdImgInfo.uwHSize  = 210;
			tOsdImgInfo.uwVSize  = 600;
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = (ENTER_ARROW == tArrowKey)?(PS_VOX_MODE == tUI_PsMode)?UI_VOXPS_STATE:
												   (PS_ADOONLY_MODE == tUI_PsMode)?UI_ADOONLYPS_STATE:UI_DISPLAY_STATE:UI_DISPLAY_STATE;
			tUI_PsMode = PS_VOX_MODE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PSSELECTNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwYStart -= (tUI_PrePsMode * 183);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PSSELECTHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwYStart -= (tUI_PsMode * 183);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_BuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_CamNum_t tEcoPrevCamNum = tUI_BuEcoCamNum;
	uint16_t uwXOffset = 5, uwYOffset = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?155:0;
	uint16_t uwYItemOffset = 150;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
		case RIGHT_ARROW:
			tUI_BuEcoCamNum = UI_ChangeSelectCamNum4UiMenu(&tEcoPrevCamNum, &tArrowKey);
			if(tUI_BuEcoCamNum >= tUI_PuSetting.ubTotalBuNum)
			{
				tUI_BuEcoCamNum = tEcoPrevCamNum;
				return;
			}
			break;
		case ENTER_ARROW:
			if(UI_SetupBuEcoMode(tUI_BuEcoCamNum) == rUI_FAIL)
				return;
		case EXIT_ARROW:
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			if(DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
				UI_ClearBuConnectStatusFlag();
			tOsdImgInfo.uwXStart  = 150;
			tOsdImgInfo.uwYStart  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?450:400;
			tOsdImgInfo.uwHSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?300:210;
			tOsdImgInfo.uwVSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?335:500;
			OSD_EraserImg2(&tOsdImgInfo);
			if(ENTER_ARROW == tArrowKey)
				osDelay(800);
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tEcoPrevCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tUI_BuEcoCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_UpdatePushTalkIcon(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint16_t uwXOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
							  [DISP_LOWER_LEFT] = (ulLcd_HSize/2), [DISP_LOWER_RIGHT] = (ulLcd_HSize/2),
							  [DISP_LEFT] 	    = 0,			   [DISP_RIGHT]	 	  = 0,
							  [DISP_3T_TOP] 	= 0,			   [DISP_3T_BOTTOM]	  = (ulLcd_HSize/2),
							  [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = 0,
	                          [DISP_3T_3CRIGHT] = 0};
	uint16_t uwYOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
							  [DISP_LOWER_LEFT] = 0, 			   [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
							  [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT]		  = (ulLcd_VSize/2),
							  [DISP_3T_TOP] 	= (ulLcd_VSize/2), [DISP_3T_BOTTOM]	  = (ulLcd_VSize/2),
							  [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = (ulLcd_VSize/3),
	                          [DISP_3T_3CRIGHT] = ((ulLcd_VSize/3)*2)};
	UI_DisplayLocation_t tUI_DispLoc;
	uint16_t uw3TColYOffset = 0;

	if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType)))
	{
		tUI_DispLoc = tUI_GetTripViewDispLoc(tUI_PuSetting.tAdoSrcCamNum);
		if((DISP_LEFT == tUI_DispLoc) || (DISP_RIGHT == tUI_DispLoc) ||
		   (DISP_3T_3CLEFT == tUI_DispLoc) || (DISP_3T_3CMID == tUI_DispLoc) || (DISP_3T_3CRIGHT == tUI_DispLoc))
			uwXOffset[tUI_DispLoc] += (ulLcd_HSize/2);
		uw3TColYOffset = (((DISP_3T_3CLEFT == tUI_DispLoc) || (DISP_3T_3CMID == tUI_DispLoc))?214:(DISP_3T_3CRIGHT == tUI_DispLoc)?212:0);
	}
	else
		tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) ||
					   (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_LOWER_RIGHT:tUI_CamStatus[tUI_PuSetting.tAdoSrcCamNum].tCamDispLocation;
	if(tCamViewSel.tCamViewType == DUAL_VIEW)
	{
		if((tUI_PuSetting.tAdoSrcCamNum != tCamViewSel.tCamViewPool[0]) && (tUI_PuSetting.tAdoSrcCamNum != tCamViewSel.tCamViewPool[1]))
			return;
		tUI_DispLoc = (tUI_PuSetting.tAdoSrcCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT;
	}
	if(OSD_OK != tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PTTMIC_ICON, 1, &tOsdImgInfo))
		return;
	tOsdImgInfo.uwXStart += (uwXOffset[tUI_DispLoc] + ((tCamViewSel.tCamViewType == DUAL_VIEW)?(ulLcd_HSize/2):0));
	tOsdImgInfo.uwYStart += uw3TColYOffset;
	tOsdImgInfo.uwYStart -= uwYOffset[tUI_DispLoc];
	switch(ubUI_PttStartFlag)
	{
		case TRUE:
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		case FALSE:
			OSD_EraserImg2(&tOsdImgInfo);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PushTalkKey(void)
{
	APP_EventMsg_t tUI_PttMessage = {0};

	if(APP_LOSTLINK_STATE == tUI_SyncAppState)
		return;
	tUI_PttMessage.ubAPP_Event 	 	= APP_PTT_EVENT;
	tUI_PttMessage.ubAPP_Message[0] = 1;		//! Message Length
	tUI_PttMessage.ubAPP_Message[1] = ubUI_PttStartFlag;
	UI_SendMessageToAPP(&tUI_PttMessage);
	SPEAKER_EN(((TRUE == ubUI_PttStartFlag)?UI_DISABLE:UI_ENABLE));
	if(UI_DISPLAY_STATE == tUI_State)
		UI_UpdatePushTalkIcon();
}
//------------------------------------------------------------------------------
void UI_DisplayArrowKeyFunc(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwOsdImgIdx = 0;

	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
		return;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
		{
			tUI_PuSetting.VolLvL.ubVOL_UpdateCnt = UI_UPDATEVOLLVL_PERIOD;
			if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
			{
				uwOsdImgIdx = OSD2IMG_VOLUME_LVL0_ICON;
				break;
			}
			uwOsdImgIdx = OSD2IMG_VOLUME_LVL0_ICON+(--tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
			if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
			{
				ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
				break;
			}
            ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
			break;
		}
		case RIGHT_ARROW:
		{
			tUI_PuSetting.VolLvL.ubVOL_UpdateCnt = UI_UPDATEVOLLVL_PERIOD;
			if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL5)
			{
				uwOsdImgIdx = OSD2IMG_VOLUME_LVL5_ICON;
				break;
			}
			if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
				ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
			uwOsdImgIdx = OSD2IMG_VOLUME_LVL0_ICON+(++tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
            ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
			break;
		}
		case UP_ARROW:
		{
			uint32_t ulUI_BLTable[] = {0x150, 0x230, 0x400, 0x600, 0x0800, 0xA00};
			tUI_PuSetting.BriLvL.ubBL_UpdateCnt = UI_UPDATEBRILVL_PERIOD;
			if(tUI_PuSetting.BriLvL.tBL_UpdateLvL == BL_LVL5)
			{
				uwOsdImgIdx = OSD2IMG_BRIGHT_LVL5_ICON;
				break;
			}
			uwOsdImgIdx = OSD2IMG_BRIGHT_LVL0_ICON+(++tUI_PuSetting.BriLvL.tBL_UpdateLvL);
			LCD_BACKLIGHT_CTRL(ulUI_BLTable[tUI_PuSetting.BriLvL.tBL_UpdateLvL]);
			break;
		}
		case DOWN_ARROW:
		{
			uint32_t ulUI_BLTable[] = {0x150, 0x230, 0x400, 0x600, 0x0800, 0xA00};
			tUI_PuSetting.BriLvL.ubBL_UpdateCnt = UI_UPDATEBRILVL_PERIOD;
			if(tUI_PuSetting.BriLvL.tBL_UpdateLvL == BL_LVL0)
			{
				uwOsdImgIdx = OSD2IMG_BRIGHT_LVL0_ICON;
				break;
			}
			uwOsdImgIdx = OSD2IMG_BRIGHT_LVL0_ICON+(--tUI_PuSetting.BriLvL.tBL_UpdateLvL);
			LCD_BACKLIGHT_CTRL(ulUI_BLTable[tUI_PuSetting.BriLvL.tBL_UpdateLvL]);
			break;
		}
		case ENTER_ARROW:
			if(UI_PHOTOCAP_MODE == tUI_PuSetting.tVdoMode)
			{
				KNL_RecordAct_t tUI_CapAct = {KNL_RECORDFUNC_DISABLE, NULL,};

				tUI_CapAct.tRecordFunc  	= KNL_PHOTO_CAPTURE;
				tUI_CapAct.pRecordStsNtyCb 	= UI_PhotoCaptureFinish;
				tKNL_ExecRecordFunc(tUI_CapAct);
			}
            else if((UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (REC_MANUAL == tUI_PuSetting.RecInfo.tREC_Mode))
            {
                UI_VideoRecordingExec(((UI_REC_START == tUI_RecPlayAct.tRecAct)?UI_REC_STOP:UI_REC_START));
            }
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_State = UI_SHOWSTSICON_STATE;
}

//------------------------------------------------------------------------------
void UI_DrawMenuPage(void)
{
	uint16_t uwMenuOsdImg[UI_MENUICON_NUM] = {OSD2IMG_CAMNOR_MENUICON,      OSD2IMG_PAIRNOR_MENUICON, OSD2IMG_RECNOR_MENUICON,
											  OSD2IMG_PHOTONOR_MENUICON,    OSD2IMG_PLYNOR_MENUICON,  OSD2IMG_PSNOR_MENUICON,
											  OSD2IMG_SETTINGNOR_MENUICON};
	uint16_t uwDisplayImgIdx = 0;
	uint8_t i;
	OSD_IMG_INFO tOsdImgInfo[UI_MENUICON_NUM*4];

	UI_ClearBuConnectStatusFlag();
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo[0]);
	tOsdImgInfo[0].uwXStart = tOsdImgInfo[0].uwYStart = 0;
	tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
	uwMenuOsdImg[tUI_MenuItem.ubItemIdx] += UI_ICON_HIGHLIGHT;
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMNOR_MENUICON, (UI_MENUICON_NUM*4), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	for(i = 0; i < UI_MENUICON_NUM; i++)
	{
		uwDisplayImgIdx = uwMenuOsdImg[i] - OSD2IMG_CAMNOR_MENUICON;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
		uwDisplayImgIdx += UI_WRICON_OFFSET;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENUWR_ICON, 1, &tOsdImgInfo[0]);	
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	OSD_Weight(OSD_WEIGHT_8DIV8);
}
//------------------------------------------------------------------------------
void UI_Menu(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwMenuOsdImg[UI_MENUICON_NUM] = {OSD2IMG_CAMNOR_MENUICON,      OSD2IMG_PAIRNOR_MENUICON, OSD2IMG_RECNOR_MENUICON,
											  OSD2IMG_PHOTONOR_MENUICON,    OSD2IMG_PLYNOR_MENUICON,  OSD2IMG_PSNOR_MENUICON,
											  OSD2IMG_SETTINGNOR_MENUICON};
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(tUI_MenuItem.ubItemIdx < PLAYBACK_ITEM)
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx 	 -= PLAYBACK_ITEM;
			break;
		case DOWN_ARROW:
			if(tUI_MenuItem.ubItemIdx >= PHOTO_ITEM)
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx 	 += PLAYBACK_ITEM;
			break;
		case LEFT_ARROW:
			if((tUI_MenuItem.ubItemIdx == CAMERAS_ITEM) ||
			   (tUI_MenuItem.ubItemIdx == PLAYBACK_ITEM))
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx--;
			break;
		case RIGHT_ARROW:
			if((tUI_MenuItem.ubItemIdx == PHOTO_ITEM) ||
			   (tUI_MenuItem.ubItemIdx == SETTING_ITEM))
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx++;
			break;
		case ENTER_ARROW:
			//! Check tUI_MenuItem.ubItemIdx
			//! Draw Sub menu page (if record and photo page, check select camera number
			UI_SubMenu(ENTER_ARROW);
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemPreIdx], 1, &tOsdImgInfo);		
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemPreIdx]+UI_WRICON_OFFSET, 1, &tOsdImgInfo);			
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	//! Draw highlight icon
	uwMenuOsdImg[tUI_MenuItem.ubItemIdx] += UI_ICON_HIGHLIGHT;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemIdx], 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemIdx]+UI_WRICON_OFFSET, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
#define CAMS_SET_ITEM_OFFSET	50
void UI_DrawCamsSubMenuPage(void)
{
	UI_CamNum_t tCamSelNum = tCamSelect.tCamNum4CamSetSub, tCamNum;
	OSD_IMG_INFO tOsdImgInfo[26];
	uint16_t uwCamOsdImg[CAM_4T] = {OSD2IMG_RECCAM1NOR_ICON, OSD2IMG_RECCAM2NOR_ICON, OSD2IMG_RECCAM3NOR_ICON, OSD2IMG_RECCAM4NOR_ICON};
	uint16_t uwDisplayImgIdx[8]  = {OSD2IMG_ANRNOR_ITEM,   OSD2IMG_THREEDNRNOR_ITEM, OSD2IMG_vLDSNOR_ITEM, 
	                                OSD2IMG_BUAECNOR_ITEM, OSD2IMG_DISNOR_ITEM, OSD2IMG_CBRNOR_ITEM,
								    OSD2IMG_CONDENSENOR_ITEM, OSD2IMG_FLICKERNOR_ITEM};
	uint16_t uwCamModeImgIdx[7]  = {OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamAnrMode,
								    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCam3DNRMode,
								    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamvLDCMode,
								    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamAecMode,
								    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamDisMode,
									OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamCbrMode,
									OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamCondenseMode};
									
	uint16_t uwCamImgIdx, uwModeImgIdx, uwXStart[2];
	uint8_t i;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON, (tUI_PuSetting.ubTotalBuNum*2), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwCamOsdImg[tCamSelNum] += UI_ICON_HIGHLIGHT;
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		uwCamImgIdx = uwCamOsdImg[tCamNum] - OSD2IMG_RECCAM1NOR_ICON;
		tOSD_Img2(&tOsdImgInfo[uwCamImgIdx], OSD_QUEUE);
	}
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOPTMARKNOR_ICON, (OSD2IMG_FLICKERHL_ITEM-OSD2IMG_CAMSOPTMARKNOR_ICON+1), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwXStart[0] = tOsdImgInfo[0].uwXStart;
	for(i = 0; i < (CAMSITEM_MAX - 2); i++)
	{
		uwDisplayImgIdx[i] -= OSD2IMG_ANRNOR_ITEM;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[i] + 10], OSD_QUEUE);
		uwModeImgIdx = uwCamModeImgIdx[i] - OSD2IMG_CAMSOPTMARKNOR_ICON;
		uwXStart[1]  = tOsdImgInfo[uwModeImgIdx].uwXStart;
		tOsdImgInfo[uwModeImgIdx].uwXStart += (i * CAMS_SET_ITEM_OFFSET);
		tOSD_Img2(&tOsdImgInfo[uwModeImgIdx], OSD_QUEUE);
		tOsdImgInfo[uwModeImgIdx].uwXStart  = uwXStart[1];
		tOsdImgInfo[0].uwXStart += ( i * CAMS_SET_ITEM_OFFSET);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		tOsdImgInfo[0].uwXStart  = uwXStart[0];
	}
	uwDisplayImgIdx[7] -= OSD2IMG_ANRNOR_ITEM;	//! CAMSFLICKER_ITEM - 1
	tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[7] + 10], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[((tUI_CamStatus[tCamSelNum].tCamFlicker == CAMFLICKER_50HZ)?0:1) + 6], OSD_QUEUE);
	tOsdImgInfo[0].uwXStart += (CAMS_SET_ITEM_OFFSET * 7);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_SUBMENUICON, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawPairingSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubMenuItemOsdImg[PAIRITEM_MAX] = {OSD2IMG_PAIRCAMHL_ITEM, OSD2IMG_DELCAMNOR_ITEM};
	uint8_t i;
	for(i = 0; i < PAIRITEM_MAX; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);		
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIR_SUBMENUICON, 1, &tOsdImgInfo);	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawRecordSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo[15];
	uint16_t uwDisplayImgIdx = 0;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFNOR_ITEM, 7, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[5], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1LOOPWR_ICON, 8, &tOsdImgInfo[7]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwDisplayImgIdx = 7 + tUI_PuSetting.RecInfo.tREC_Mode;
	tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
	uwDisplayImgIdx = 11 + tUI_PuSetting.RecInfo.tREC_Time;
	tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKNOR_ICON, 2, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOsdImgInfo[0].uwXStart += 0x40;
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawPhotoSubMenuPage(void)
{
	uint16_t uwCamOsdImg[CAM_4T] = {OSD2IMG_RECCAM1NOR_ICON, OSD2IMG_RECCAM2NOR_ICON, OSD2IMG_RECCAM3NOR_ICON, OSD2IMG_RECCAM4NOR_ICON};
	OSD_IMG_INFO tOsdImgInfo[OSD2IMG_PRES12MHL_ICON - OSD2IMG_PHOTO_SUBMENUICON + 1] = {0};
	UI_CamNum_t tCamSelNum = tCamSelect.tCamNum4PhotoSub, tCamNum;
	uint16_t uwDisplayImgIdx[5]  = {0, (OSD2IMG_PHOTOMODENOR_ITEM-OSD2IMG_PHOTO_SUBMENUICON), ((OSD2IMG_PHOTOOFFWR_ICON-OSD2IMG_PHOTO_SUBMENUICON)+tUI_CamStatus[tCamSelNum].tPHOTO_Func),
								   (OSD2IMG_PRESNOR_ITEM-OSD2IMG_PHOTO_SUBMENUICON), ((OSD2IMG_PRES3MWR_ICON-OSD2IMG_PHOTO_SUBMENUICON)+tUI_CamStatus[tCamSelNum].tPHOTO_Resolution)};
	uint8_t i;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON, (CAM_4T*2), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwCamOsdImg[tCamSelNum] += UI_ICON_HIGHLIGHT;
	for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
	{
		uwDisplayImgIdx[0] = uwCamOsdImg[tCamNum] - OSD2IMG_RECCAM1NOR_ICON;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[0]], OSD_QUEUE);
	}
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PHOTO_SUBMENUICON, (OSD2IMG_PRES12MHL_ICON-OSD2IMG_PHOTO_SUBMENUICON+1), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	for(i = 1; i < 6; i++)
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[i]], OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo[1]);
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOsdImgInfo[1].uwXStart += 0x40;
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawPlaybackSubMenuPage(void)
{
	ubUI_RecSubMenuFlag = TRUE;
	UI_DrawDCIMFolderMenu();
}
//------------------------------------------------------------------------------
void UI_DrawPowerSaveSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PS_SUBMENUICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawSettingSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubMenuItemOsdImg[SETTINGITEM_MAX] = {OSD2IMG_DTHL_ITEM, OSD2IMG_AECNOR_ITEM, OSD2IMG_CCANOR_ITEM,
	                                                 OSD2IMG_STORAGENOR_ITEM, OSD2IMG_LANGUAGENOR_ITEM, OSD2IMG_DEFUNOR_ITEM, OSD2IMG_SWUSBDMODENOR_ITEM};
	uint8_t i;
	for(i = 0; i < SETTINGITEM_MAX; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AECOFFWR_ICON+tUI_PuSetting.ubAEC_Mode, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AECOFFWR_ICON+tUI_PuSetting.ubCCA_Mode, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += 65;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_STORAGATBU_ICON+tUI_PuSetting.ubSTORAGE_Mode, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart -= 0xE;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOsdImgInfo.uwXStart += (0xE + 0x32);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SETTING_SUBMENUICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawSubMenuPage(UI_MenuItemList_t MenuItem)
{
	const static UI_DrawSubMenuFuncPtr_t DrawSubMenuFunc[MENUITEM_MAX] =
	{
		UI_DrawCamsSubMenuPage,
		UI_DrawPairingSubMenuPage,
		UI_DrawRecordSubMenuPage,
		NULL,						//! UI_DrawPhotoSubMenuPage,
		UI_DrawPlaybackSubMenuPage,
		UI_DrawPowerSaveSubMenuPage,
		UI_DrawSettingSubMenuPage
	};
	OSD_IMG_INFO tOsdImgInfo;

	if(NULL == DrawSubMenuFunc[MenuItem].pvFuncPtr)
		return;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
	DrawSubMenuFunc[MenuItem].pvFuncPtr();
	tUI_State = UI_SUBMENU_STATE;
}
//------------------------------------------------------------------------------
void UI_SubMenu(UI_ArrowKey_t tArrowKey)
{
	const static UI_MenuFuncPtr_t SubMenuFunc[MENUITEM_MAX] =
	{
		UI_CameraSettingSubMenuPage,
		UI_PairingSubMenuPage,
		UI_RecordSubMenuPage,
		NULL,
		UI_PlaybackSubMenuPage,
		UI_PowerSaveSubMenuPage,
		UI_SettingSubMenuPage,
	};
	if(SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr)
		SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_SubSubMenu(UI_ArrowKey_t tArrowKey)
{
	const static UI_MenuFuncPtr_t SubSubMenuFunc[MENUITEM_MAX] =
	{
		NULL,
		UI_PairingSubSubMenuPage,
		UI_RecordSubSubMenuPage,
		UI_PhotoSubSubMenuPage,
		NULL,
		NULL,
		UI_SettingSubSubMenuPage
	};
	if(SubSubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr)
		SubSubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_SubSubSubMenu(UI_ArrowKey_t tArrowKey)
{
	switch(tUI_MenuItem.ubItemIdx)
	{
		case PAIRING_ITEM:
			UI_PairingSubSubSubMenuPage(tArrowKey);
			break;
		case SETTING_ITEM:
			UI_SettingSysDateTimeSubSubMenuPage(tArrowKey);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_CamsSubMenuItemList_t tSubMenuItem = (UI_CamsSubMenuItemList_t)tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;
	OSD_IMG_INFO tOsdImgInfo[8];
	UI_CamNum_t tCamSelNum;
	UI_CamsSetMode_t tCamsModeSts[CAMSITEM_MAX];

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Cameras sub menu page
		UI_DrawSubMenuPage(CAMERAS_ITEM);
		return;
	}
	if(tSubMenuItem == CAMSSELCAM_ITEM)
	{
		if((tArrowKey == LEFT_ARROW) || (tArrowKey == RIGHT_ARROW))
		{
			UI_CamNum_t tPreCamNum = tCamSelect.tCamNum4CamSetSub;
			UI_CamNum_t tNexCamNum = NO_CAM;
			tNexCamNum = UI_ChangeSelectCamNum4UiMenu(&tCamSelect.tCamNum4CamSetSub, &tArrowKey);
			if(tNexCamNum < tUI_PuSetting.ubTotalBuNum)
			{
				uint16_t uwTmpXStart = 0;
				uint8_t i;
				//! Change camera number
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON+(tPreCamNum*2), 1, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1HL_ICON+(tNexCamNum*2), 1, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOFFWR_ICON, 2, &tOsdImgInfo[0]);
				tCamsModeSts[CAMSANR_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamAnrMode;
				tCamsModeSts[CAMS3DNR_ITEM] 	= tUI_CamStatus[tNexCamNum].tCam3DNRMode;
				tCamsModeSts[CAMSvLDS_ITEM] 	= tUI_CamStatus[tNexCamNum].tCamvLDCMode;
				tCamsModeSts[CAMSAEC_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamAecMode;
				tCamsModeSts[CAMSDIS_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamDisMode;
				tCamsModeSts[CAMSCBR_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamCbrMode;
				tCamsModeSts[CAMSCONDENSE_ITEM] = tUI_CamStatus[tNexCamNum].tCamCondenseMode;
				for(i = CAMSANR_ITEM; i <= CAMSCONDENSE_ITEM; i++)
				{
					uwTmpXStart = tOsdImgInfo[tCamsModeSts[i]].uwXStart;
					tOsdImgInfo[tCamsModeSts[i]].uwXStart += (i - CAMSANR_ITEM) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsModeSts[i]], OSD_QUEUE);
					tOsdImgInfo[tCamsModeSts[i]].uwXStart = uwTmpXStart;
				}
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS50WR_ICON, 2, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[(tUI_CamStatus[tNexCamNum].tCamFlicker == CAMFLICKER_50HZ)?0:1], OSD_UPDATE);
				tCamSelect.tCamNum4CamSetSub = tNexCamNum;
			}
			else if((tArrowKey == LEFT_ARROW) && (TRUE == ubUI_FastStateFlag))
			{
				OSD_IMG_INFO tOsdInfo;

				tOsdInfo.uwHSize  = 600;
				tOsdInfo.uwVSize  = 1180;
				tOsdInfo.uwXStart = 100;
				tOsdInfo.uwYStart = 50;
				OSD_EraserImg2(&tOsdInfo);
				ubUI_FastStateFlag = FALSE;
				UI_DrawDCIMFolderMenu();
			}
			return;
		}
	}
	tCamSelNum = tCamSelect.tCamNum4CamSetSub;
	if((tUI_CamStatus[tCamSelNum].ulCAM_ID == INVALID_ID) ||
	   (tUI_CamStatus[tCamSelNum].tCamConnSts == CAM_OFFLINE))
		return;
	tCamsModeSts[CAMSANR_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamAnrMode;
	tCamsModeSts[CAMS3DNR_ITEM] 	= tUI_CamStatus[tCamSelNum].tCam3DNRMode;
	tCamsModeSts[CAMSvLDS_ITEM] 	= tUI_CamStatus[tCamSelNum].tCamvLDCMode;
	tCamsModeSts[CAMSAEC_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamAecMode;
	tCamsModeSts[CAMSDIS_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamDisMode;
	tCamsModeSts[CAMSCBR_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamCbrMode;
	tCamsModeSts[CAMSCONDENSE_ITEM] = tUI_CamStatus[tCamSelNum].tCamCondenseMode;
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[CAMERAS_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[CAMSITEM_MAX] = {OSD2IMG_ANRNOR_ITEM,  OSD2IMG_ANRNOR_ITEM,      OSD2IMG_THREEDNRNOR_ITEM,
														  OSD2IMG_vLDSNOR_ITEM, OSD2IMG_BUAECNOR_ITEM,    OSD2IMG_DISNOR_ITEM, 
														  OSD2IMG_CBRNOR_ITEM,  OSD2IMG_CONDENSENOR_ITEM, OSD2IMG_FLICKERNOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx 	= tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemIdx;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOFFWR_ICON, 8, &tOsdImgInfo[0]);
			if((ubSubMenuItemIdx != CAMSANR_ITEM) || (ubSubMenuItemPreIdx == CAMS3DNR_ITEM))
			{
				if(ubSubMenuItemPreIdx == CAMSFLICKER_ITEM)
				{
					tOSD_Img2(&tOsdImgInfo[(4 + ((tUI_CamStatus[tCamSelNum].tCamFlicker == CAMFLICKER_50HZ)?0:1))], OSD_QUEUE);
				}
				else
				{
					tOsdImgInfo[tCamsModeSts[ubSubMenuItemPreIdx]].uwXStart += (ubSubMenuItemPreIdx - 1) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsModeSts[ubSubMenuItemPreIdx]], OSD_QUEUE);
				}
			}
			if(ubSubMenuItemIdx > CAMSSELCAM_ITEM)
			{
				if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
				{
					tOSD_Img2(&tOsdImgInfo[(6 + ((tUI_CamStatus[tCamSelNum].tCamFlicker == CAMFLICKER_50HZ)?0:1))], OSD_QUEUE);
				}
				else
				{
					tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx] + 2].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx] + 2], OSD_QUEUE);
				}
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOPTMARKNOR_ICON, 2, &tOsdImgInfo[0]);
			if((ubSubMenuItemIdx != CAMSANR_ITEM) || (ubSubMenuItemPreIdx == CAMS3DNR_ITEM))
			{
				tOsdImgInfo[0].uwXStart += (ubSubMenuItemPreIdx - 1) * CAMS_SET_ITEM_OFFSET;
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			}
			if(ubSubMenuItemIdx > CAMSSELCAM_ITEM)
			{
				tOsdImgInfo[1].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
				tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
			}
			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + ((!ubSubMenuItemIdx)?0:UI_ICON_HIGHLIGHT)));
			break;
		}
		case DRAW_MENUPAGE:
			if(tArrowKey == ENTER_ARROW)
			{
				UI_PUReqCmd_t tCamSetCmd;
				UI_CamsSetMode_t tCamsSetMode;
				UI_CamFlicker_t tCamsFlicker = tUI_CamStatus[tCamSelNum].tCamFlicker;
				uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemIdx;
				uint8_t tUI_CamsSetItem[CAMSITEM_MAX] = {UI_ADOANR_SETTING,  UI_ADOANR_SETTING, UI_IMG3DNR_SETTING,
														 UI_IMGvLDC_SETTING, UI_ADOAEC_SETTING, UI_IMGDIS_SETTING, 
														 UI_IMGCBR_SETTING, UI_IMGCONDENSE_SETTING, UI_FLICKER_SETTING};
				UI_CamsSetMode_t *pCamsModeSts[CAMSITEM_MAX] = {&tUI_CamStatus[tCamSelNum].tCamAnrMode, &tUI_CamStatus[tCamSelNum].tCamAnrMode,
																&tUI_CamStatus[tCamSelNum].tCam3DNRMode, &tUI_CamStatus[tCamSelNum].tCamvLDCMode,
																&tUI_CamStatus[tCamSelNum].tCamAecMode, &tUI_CamStatus[tCamSelNum].tCamDisMode,
															    &tUI_CamStatus[tCamSelNum].tCamCbrMode, &tUI_CamStatus[tCamSelNum].tCamCondenseMode};

				if((ubSubMenuItemIdx == CAMSSELCAM_ITEM) || (ubSubMenuItemIdx == CAMSDIS_ITEM) ||				   
				   (ubSubMenuItemIdx == CAMSCBR_ITEM) || (ubSubMenuItemIdx == CAMSCONDENSE_ITEM))
					break;
				tCamSetCmd.tDS_CamNum 				= tCamSelNum;
				tCamSetCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
				tCamSetCmd.ubCmd[UI_SETTING_ITEM]   = (ubSubMenuItemIdx == CAMSANR_ITEM)?UI_ADOANR_SETTING:(ubSubMenuItemIdx == CAMSAEC_ITEM)?UI_ADOAEC_SETTING:UI_IMGPROC_SETTING;
				if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
				{
					tCamsFlicker  						= (CAMFLICKER_50HZ == tUI_CamStatus[tCamSelNum].tCamFlicker)?CAMFLICKER_60HZ:CAMFLICKER_50HZ;
					tCamSetCmd.ubCmd[UI_SETTING_DATA]   = UI_FLICKER_SETTING;
					tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tCamsFlicker;
				}
				else
				{
					tCamsSetMode  						= (CAMSET_OFF == tCamsModeSts[ubSubMenuItemIdx])?CAMSET_ON:CAMSET_OFF;
					tCamSetCmd.ubCmd[UI_SETTING_DATA]   = ((ubSubMenuItemIdx == CAMSANR_ITEM) || (ubSubMenuItemIdx == CAMSAEC_ITEM))?tCamsSetMode:tUI_CamsSetItem[ubSubMenuItemIdx];
					tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ((ubSubMenuItemIdx == CAMSANR_ITEM) || (ubSubMenuItemIdx == CAMSAEC_ITEM))?0:tCamsSetMode;
				}
				tCamSetCmd.ubCmd_Len  				= ((ubSubMenuItemIdx == CAMSANR_ITEM) || (ubSubMenuItemIdx == CAMSAEC_ITEM))?3:4;
				if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS50HL_ICON, 2, &tOsdImgInfo[0]);
					tOSD_Img2(&tOsdImgInfo[tCamsFlicker], OSD_UPDATE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOFFHL_ICON, 2, &tOsdImgInfo[0]);
					tOsdImgInfo[tCamsSetMode].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsSetMode], OSD_UPDATE);
				}
				if(UI_SendRequestToBU(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
				{
					printd(DBG_ErrorLvl, "Camera Setting fail !\n");
					if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
					{
						tOSD_Img2(&tOsdImgInfo[tCamsFlicker], OSD_UPDATE);
					}
					else
					{
						tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx]].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
						tOSD_Img2(&tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx]], OSD_UPDATE);
					}
					return;
				}
				*pCamsModeSts[ubSubMenuItemIdx] 		= tCamsSetMode;
				tUI_CamStatus[tCamSelNum].tCamFlicker   = tCamsFlicker;
				UI_UpdateDevStatusInfo();
			}
			break;
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
#if 0
void UI_CameraSettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_CamsSubSubMenuItem_t tCamsSubSubMenuItem = 
	{
		{CAM1VIEW_ITEM, CAMVIEWITEM_MAX, {QUALVIEW_ITEM, QUALVIEW_ITEM}}
	};
	static uint8_t ubUI_CamSelStsUpdateFlag = FALSE;
	OSD_IMG_INFO tOsdImgInfo;
	UI_MenuAct_t tMenuAct;
	
	if(FALSE == ubUI_CamSelStsUpdateFlag)
	{
		tCamsSubSubMenuItem.tCameraViewPage.ubItemCount = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?CAMVIEWITEM_MAX:
														  (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?DUALVIEW_ITEM:SINGLEVIEW_ITEM;
		tCamsSubSubMenuItem.tCameraViewPage.tSubMenuInfo.ubItemIdx 	= tCamViewSel.tCamViewPool[0];
		ubUI_CamSelStsUpdateFlag = TRUE;
	}
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tCamsSubSubMenuItem.tCameraViewPage);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubSubMenuItemOsdImg[5] = {OSD2IMG_CAM1VIEWNOR_ICON, OSD2IMG_CAM2VIEWNOR_ICON, OSD2IMG_CAM3VIEWNOR_ICON,
												  OSD2IMG_CAM4VIEWNOR_ICON, OSD2IMG_CAMQUALVIEWNOR_ICON};
			uint8_t ubSubSubMenuItemPreIdx 	= tCamsSubSubMenuItem.tCameraViewPage.tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubSubMenuItemIdx 	= tCamsSubSubMenuItem.tCameraViewPage.tSubMenuInfo.ubItemIdx;

			if(tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
				uwSubSubMenuItemOsdImg[2] = OSD2IMG_CAMDUALVIEWNOR_ICON;
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMVIEWSUS_ICON, 1, &tOsdImgInfo);
			OSD_EraserImg2(&tOsdImgInfo);
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
			break;
		}
		case EXECUTE_MENUFUNC:
		{
			UI_CamNum_t tCamNumSel = (UI_CamNum_t)tCamsSubSubMenuItem.tCameraViewPage.tSubMenuInfo.ubItemIdx;

			tCamViewSel.tCamViewPool[0] = tCamNumSel;
			tCamViewSel.tCamViewType 	= (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(tCamNumSel == CAM_4T)?QUAD_VIEW:SINGLE_VIEW:
									      (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?(tCamNumSel == CAM_2T)?DUAL_VIEW:SINGLE_VIEW:SINGLE_VIEW;
			tUI_PuSetting.ubScanModeEn	= FALSE;
			UI_SwitchCameraSource();
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMVIEWSUS_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewPool[0]*68);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAM1VIEWNOR_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwYStart -= 150;
			tOsdImgInfo.uwHSize   = 335;
			tOsdImgInfo.uwVSize   = 420;
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_CamSelStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
#endif
//------------------------------------------------------------------------------
void UI_PairingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;

    if(UI_REC_START == tUI_RecPlayAct.tRecAct)
    {
        return;
    }        
	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Cameras sub menu page
		UI_DrawSubMenuPage(PAIRING_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[PAIRING_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[PAIRITEM_MAX] = {OSD2IMG_PAIRCAMNOR_ITEM, OSD2IMG_DELCAMNOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
			break;
		}
		case DRAW_MENUPAGE:
		{
			//! Draw sub sub menu page
			OSD_IMG_INFO tOsdImgInfo, tCamRdyOsdImgInfo = {0};
			uint16_t uwSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1NOR_ICON, OSD2IMG_PAIRCAM2NOR_ICON, OSD2IMG_PAIRCAM3NOR_ICON, OSD2IMG_PAIRCAM4NOR_ICON};
			uint8_t ubBuNum = tUI_PuSetting.ubTotalBuNum, i, ubUI_FirstHL = FALSE;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
			uint16_t uwIconOffset = (ubSubMenuItemIdx == DELCAM_ITEM)?80:0;
			uint16_t uwRDY_XStart = 0, uwOsdImgIdx;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tCamRdyOsdImgInfo);
			uwRDY_XStart = tCamRdyOsdImgInfo.uwXStart;
			if(ubSubMenuItemIdx == PAIRCAM_ITEM)
			{
				uint16_t uwItemOffset = ubBuNum * 55;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DELCAMNOR_ITEM, 1, &tOsdImgInfo);
				OSD_EraserImg2(&tOsdImgInfo);
				tOsdImgInfo.uwXStart += uwItemOffset;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				for(i = CAM1; i < ubBuNum; i++)
				{
					if(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)
					{
						if(FALSE == ubUI_FirstHL)
						{
							uwSubMenuItemOsdImg[i] += UI_ICON_HIGHLIGHT;
							ubUI_FirstHL = TRUE;
						}
						tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
					}
					else
					{
						uwOsdImgIdx = uwSubMenuItemOsdImg[i]+UI_ICON_READY;
						if(FALSE == ubUI_FirstHL)
						{
							uwOsdImgIdx  = uwSubMenuItemOsdImg[i]+UI_ICON_HIGHLIGHT;
							ubUI_FirstHL = TRUE;
						}
						tCamRdyOsdImgInfo.uwXStart = uwRDY_XStart + (i*55);
						tOSD_Img2(&tCamRdyOsdImgInfo, OSD_QUEUE);
						tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
					}
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			if(ubSubMenuItemIdx == DELCAM_ITEM)
			{
				for(i = CAM1; i < ubBuNum; i++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSubMenuItemOsdImg[i]+((i == CAM1)?UI_ICON_HIGHLIGHT:(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)?UI_ICON_NORMAL:UI_ICON_READY)), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart += uwIconOffset;
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
					if(tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
					{
						tCamRdyOsdImgInfo.uwXStart = uwIconOffset + uwRDY_XStart + (i*55);
						tOSD_Img2(&tCamRdyOsdImgInfo, OSD_QUEUE);						
					}
				}
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIROPTMASK_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwIconOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_State = UI_SUBSUBMENU_STATE;
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingUpdateSubSubMenuItemIndex(UI_SubMenuItem_t *ptSubSubMenuItem, uint8_t *Update_Flag)
{
	ptSubSubMenuItem->ubFirstItem 			 = CAM1;
	ptSubSubMenuItem->tSubMenuInfo.ubItemIdx = CAM1;
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_PairingDrawSubSbuMenuItem(UI_PairSubMenuItemList_t *ptSubMenuItem, UI_SubMenuItem_t *ptSubSubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1NOR_ICON, OSD2IMG_PAIRCAM2NOR_ICON, OSD2IMG_PAIRCAM3NOR_ICON, OSD2IMG_PAIRCAM4NOR_ICON};
	uint16_t uwIconOffset = (*ptSubMenuItem == DELCAM_ITEM)?80:0;
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx    = ptSubSubMenuItem->tSubMenuInfo.ubItemIdx;
	switch(*ptSubMenuItem)
	{
		case PAIRCAM_ITEM:
		case DELCAM_ITEM:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx]+((tUI_CamStatus[ubSubSubMenuItemPreIdx].ulCAM_ID != INVALID_ID)?UI_ICON_READY:UI_ICON_NORMAL)), 1, &tOsdImgInfo);								
			tOsdImgInfo.uwXStart += uwIconOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT, 1, &tOsdImgInfo);											
			tOsdImgInfo.uwXStart += uwIconOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_PairSubSubMenuItem_t tPairSubSubMenuItem =
	{
		{
		   { 0, CAM_4T, { 0, 0 } },
		   { 0, CAM_4T, { 0, 0 } },
		},
	};
	static uint8_t ubUI_PairStsUpdateFlag = FALSE;
	UI_PairSubMenuItemList_t tSubMenuItem = (UI_PairSubMenuItemList_t)tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;
	OSD_IMG_INFO tOsdImgInfo;

	if((FALSE == ubUI_PairStsUpdateFlag) && (tSubMenuItem == PAIRCAM_ITEM))
		UI_PairingUpdateSubSubMenuItemIndex(&tPairSubSubMenuItem.tPairS[tSubMenuItem], &ubUI_PairStsUpdateFlag);
	tPairSubSubMenuItem.tPairS[tSubMenuItem].ubItemCount = tUI_PuSetting.ubTotalBuNum;
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tPairSubSubMenuItem.tPairS[tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_PairingDrawSubSbuMenuItem(&tSubMenuItem, &tPairSubSubMenuItem.tPairS[tSubMenuItem]);
			break;
		case EXECUTE_MENUFUNC:
		{
			if(tSubMenuItem == PAIRCAM_ITEM)
			{
				if(tUI_PuSetting.ubTotalBuNum == DISPLAY_1T1R)
				{
					APP_EventMsg_t tUI_PairMessage = {0};

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRINGSINGLE_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
					tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
					tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam = CAM1;
					tUI_PairMessage.ubAPP_Message[2] = DISP_1T;
					tUI_PairMessage.ubAPP_Message[3] = FALSE;
					UI_SendMessageToAPP(&tUI_PairMessage);
					tPairInfo.ubDrawFlag 			 = TRUE;
					UI_DisableScanMode();
					tUI_State = UI_PAIRING_STATE;
				}
				else if(tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
				{
					tPairInfo.tDispLocation = DISP_LEFT;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLEFTWINDOW_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tPairInfo.tPairSelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;
					tUI_State = UI_SUBSUBSUBMENU_STATE;
				}
				else if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
				{
					tPairInfo.tDispLocation = DISP_UPPER_LEFT;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tPairInfo.tPairSelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;
					tUI_State = UI_SUBSUBSUBMENU_STATE;
				}
			}
			if(tSubMenuItem == DELCAM_ITEM)
			{
				OSD_IMG_INFO tDelOsdInfo;
				uint16_t uwSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1HL_ICON, OSD2IMG_PAIRCAM2HL_ICON, OSD2IMG_PAIRCAM3HL_ICON, OSD2IMG_PAIRCAM4HL_ICON};
				uint16_t uwIconOffset = 80;
				UI_CamNum_t tUI_DelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx, tSwCamNum;

				if(INVALID_ID == tUI_CamStatus[tUI_DelCam].ulCAM_ID)
					break;
				tUI_PuSetting.tAdoSrcCamNum  = (tUI_PuSetting.tAdoSrcCamNum == tUI_DelCam)?NO_CAM:tUI_PuSetting.tAdoSrcCamNum;
				for(tSwCamNum = CAM1; tSwCamNum < tUI_PuSetting.ubTotalBuNum; tSwCamNum++)
				{
					if((tSwCamNum != tUI_DelCam) &&
					   (tUI_CamStatus[tSwCamNum].ulCAM_ID != INVALID_ID) &&
					   (tUI_CamStatus[tSwCamNum].tCamConnSts == CAM_ONLINE))
					{
						UI_ClearBuConnectStatusFlag();
						if(SINGLE_VIEW == tCamViewSel.tCamViewType)
						{
							tCamViewSel.tCamViewPool[0] = tSwCamNum;
							tUI_PuSetting.tAdoSrcCamNum = tSwCamNum;
							UI_SwitchCameraSource();
							break;
						}
						else
						{
							UI_SwitchAudioSource(tSwCamNum);
							break;
						}
					}
				}
				UI_UnBindBu(tUI_DelCam);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[tUI_DelCam], 1, &tOsdImgInfo);
				tDelOsdInfo.uwHSize  = 50;
				tDelOsdInfo.uwVSize  = 80;
				tDelOsdInfo.uwXStart = tOsdImgInfo.uwXStart + uwIconOffset;
				tDelOsdInfo.uwYStart = tOsdImgInfo.uwYStart - 80;
				OSD_EraserImg2(&tDelOsdInfo);
				tOsdImgInfo.uwXStart += uwIconOffset;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			break;
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo;
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRCAM1NOR_ICON, 1, &tOsdImgInfo);
			if(tSubMenuItem == PAIRCAM_ITEM)
			{
				tOsdImgInfo.uwHSize   = 335;
				tOsdImgInfo.uwVSize   = 450;
				tOsdImgInfo.uwYStart -= 80;
				OSD_EraserImg2(&tOsdImgInfo);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DELCAMNOR_ITEM, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			if(tSubMenuItem == DELCAM_ITEM)
			{
				tOsdImgInfo.uwHSize   = 335;
				tOsdImgInfo.uwVSize   = 330;
				tOsdImgInfo.uwXStart += 80;
				tOsdImgInfo.uwYStart -= 80;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			memset(&tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
			ubUI_PairStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_UPPER_LEFT) || (tPairInfo.tDispLocation == DISP_UPPER_RIGHT))
					return;
				tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_LOWER_LEFT)?DISP_UPPER_LEFT:DISP_UPPER_RIGHT;
				break;
			}
			return;
		case DOWN_ARROW:
			if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))
					return;
				tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_UPPER_LEFT)?DISP_LOWER_LEFT:DISP_LOWER_RIGHT;
				break;
			}
			return;
		case LEFT_ARROW:
			if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_UPPER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_LEFT))
					return;
				tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_UPPER_RIGHT)?DISP_UPPER_LEFT:DISP_LOWER_LEFT;
			}
			else if(tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
			{
				if(tPairInfo.tDispLocation == DISP_LEFT)
					return;
				tPairInfo.tDispLocation = DISP_LEFT;
			}
			break;
		case RIGHT_ARROW:
			if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))
					return;
				tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_UPPER_LEFT)?DISP_UPPER_RIGHT:DISP_LOWER_RIGHT;
			}
			else if(tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
			{
				if(tPairInfo.tDispLocation == DISP_RIGHT)
					return;
				tPairInfo.tDispLocation = DISP_RIGHT;
			}
			break;
		case ENTER_ARROW:
		{
			APP_EventMsg_t tUI_PairMessage = {0};
			uint16_t uwIconXoffset = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?88:0:
									 (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?40:0;
			uint16_t uwIconYoffset = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?130:0:
									 (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?(tPairInfo.tDispLocation == DISP_RIGHT)?130:0:0;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRINGMULTI_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwIconXoffset;
			tOsdImgInfo.uwYStart -= uwIconYoffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
			tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
			tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam;
			tUI_PairMessage.ubAPP_Message[2] = tPairInfo.tDispLocation;
			tUI_PairMessage.ubAPP_Message[3] = FALSE;
			UI_SendMessageToAPP(&tUI_PairMessage);
			tPairInfo.ubDrawFlag 			 = TRUE;
			UI_DisableScanMode();
			tUI_State = UI_PAIRING_STATE;
			return;
		}
		case EXIT_ARROW:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON, 1, &tOsdImgInfo);			
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = UI_SUBSUBMENU_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON+tPairInfo.tDispLocation, 1, &tOsdImgInfo);	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
#define PAIRING_ICON_NUM	2
void UI_DrawPairingStatusIcon(void)
{
	static OSD_IMG_INFO tPairOsdImgInfo[PAIRING_ICON_NUM] = {0};
	uint16_t uwPairIconOsdImgIdx;
	uint16_t uwIconXoffset = 0;
	uint16_t uwIconYoffset = 0;
	uint16_t uwIconXStart  = 0;
	uint16_t uwIconYStart  = 0;
	static uint8_t ubIconOffset = 0;

	if(FALSE == tPairInfo.ubDrawFlag)
		return;
	if(FALSE == tUI_PuSetting.IconSts.ubRdPairIconFlag)
	{
		uwPairIconOsdImgIdx = (tUI_PuSetting.ubTotalBuNum == DISPLAY_1T1R)?OSD2IMG_PAIRINGSINGLE_ICON:
							  (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?OSD2IMG_PAIRINGMULTI_ICON:
							  (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?OSD2IMG_PAIRINGMULTI_ICON:OSD2IMG_PAIRINGSINGLE_ICON;
		if(tOSD_GetOsdImgInfor(1, OSD_IMG2, uwPairIconOsdImgIdx, PAIRING_ICON_NUM, &tPairOsdImgInfo[0]) != OSD_OK)
		{
			printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
			return;
		}
		tUI_PuSetting.IconSts.ubRdPairIconFlag 	= TRUE;
		ubIconOffset     						= 1;
	}
	uwIconXoffset = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?88:0:
					(tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?40:0;
	uwIconYoffset = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?130:0:
					(tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?(tPairInfo.tDispLocation == DISP_RIGHT)?130:0:0;
	uwIconXStart = tPairOsdImgInfo[ubIconOffset].uwXStart;
	uwIconYStart = tPairOsdImgInfo[ubIconOffset].uwYStart;
	tPairOsdImgInfo[ubIconOffset].uwXStart += uwIconXoffset;
	tPairOsdImgInfo[ubIconOffset].uwYStart -= uwIconYoffset;
	tOSD_Img2(&tPairOsdImgInfo[ubIconOffset], OSD_UPDATE);
	tPairOsdImgInfo[ubIconOffset].uwXStart = uwIconXStart;
	tPairOsdImgInfo[ubIconOffset].uwYStart = uwIconYStart;
	ubIconOffset = (++ubIconOffset == PAIRING_ICON_NUM)?0:ubIconOffset;
}
//------------------------------------------------------------------------------
void UI_ReportPairingResult(UI_Result_t tResult)
{
	OSD_IMG_INFO tOsdImgInfo, tEraseOsdImgInfo;
	APP_EventMsg_t tUI_UnindBuMsg = {0};
	UI_CamNum_t tCamNum;
	uint16_t uwPairIconOsdImgIdx = 0;
	uint16_t uwIconXStart  = 0;

	tPairInfo.ubDrawFlag = FALSE;
	uwPairIconOsdImgIdx = (tUI_PuSetting.ubTotalBuNum == DISPLAY_1T1R)?OSD2IMG_PAIRINGSINGLE_ICON:OSD2IMG_PAIRLUWINDOW_ICON;
	uwIconXStart		= (tUI_PuSetting.ubTotalBuNum == DISPLAY_1T1R)?(tPairInfo.tPairSelCam*55):0;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwPairIconOsdImgIdx, 1, &tEraseOsdImgInfo);
	tEraseOsdImgInfo.uwXStart += uwIconXStart;
	switch(tResult)
	{
		case rUI_SUCCESS:
			if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) ||
			   (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum))
			{
				if(!tUI_PuSetting.ubPairedBuNum)
				{
					if((SINGLE_VIEW == tCamViewSel.tCamViewType) || (DUAL_VIEW == tCamViewSel.tCamViewType))
					{
						tCamViewSel.tCamViewPool[0] = tPairInfo.tPairSelCam;
						tCamViewSel.tCamViewPool[1] = ((tPairInfo.tPairSelCam + 1) >= ((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?CAM_2T:CAM_4T))?CAM1:
													   ((UI_CamNum_t)(tPairInfo.tPairSelCam + 1));
						if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (DUAL_VIEW == tCamViewSel.tCamViewType))
							UI_SwitchCameraSource();
					}
				}
				else if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (DUAL_VIEW == tCamViewSel.tCamViewType))
				{
					UI_CamNum_t tCamPoolNum = tCamViewSel.tCamViewPool[1];

					if((INVALID_ID == tUI_CamStatus[tCamPoolNum].ulCAM_ID) && (tCamPoolNum != tPairInfo.tPairSelCam))
					{
						tCamViewSel.tCamViewPool[1] = tPairInfo.tPairSelCam;
						UI_SwitchCameraSource();
					}
				}
			}
			tUI_PuSetting.ubPairedBuNum += (tUI_PuSetting.ubPairedBuNum >= tUI_PuSetting.ubTotalBuNum)?0:1;
			tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
			tUI_CamStatus[tPairInfo.tPairSelCam].tCamDispLocation = tPairInfo.tDispLocation;
			for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
			{
				if(tCamNum == tPairInfo.tPairSelCam)
					continue;
				if((INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID) &&
				   (tUI_CamStatus[tCamNum].tCamDispLocation == tPairInfo.tDispLocation))
				{
					tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_CAM_EVENT;
					tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
					tUI_UnindBuMsg.ubAPP_Message[1] = tCamNum;
					UI_SendMessageToAPP(&tUI_UnindBuMsg);
					tUI_CamStatus[tCamNum].ulCAM_ID    = INVALID_ID;
					tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
					tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart += (tCamNum*55);
					OSD_EraserImg2(&tOsdImgInfo);
					tOSD_GetOsdImgInfor(1, OSD_IMG2, ((OSD2IMG_PAIRCAM1NOR_ICON+(tCamNum*3))), 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
					UI_ResetDevSetting(tCamNum);
				}
			}
			if((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
			   (NO_CAM == tUI_PuSetting.tAdoSrcCamNum) ||
			   (INVALID_ID == tUI_CamStatus[tUI_PuSetting.tAdoSrcCamNum].ulCAM_ID))
				tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;
#if ((LCD_PANEL != LCD_H5024A) && (LCD_PANEL != LCD_FTD50B7))				
			tOSD_GetOsdImgInfor(1, OSD_IMG2, ((OSD2IMG_PAIRCAM1NOR_ICON+(tPairInfo.tPairSelCam*3))+UI_ICON_READY), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += (tPairInfo.tPairSelCam*55);
			OSD_EraserImg2(&tEraseOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			UI_PairingSubSubMenuPage(DOWN_ARROW);
#endif
			UI_ResetDevSetting(tPairInfo.tPairSelCam);
			UI_UpdateDevStatusInfo();
			break;
		case rUI_FAIL:
			OSD_EraserImg2(&tEraseOsdImgInfo);
			break;
		default:
			break;
	}
	tUI_State = UI_SUBSUBMENU_STATE;
}
//------------------------------------------------------------------------------
void UI_ReportAppPairingResult(UI_Result_t tResult)
{
	UI_CamNum_t tCamNum;
	OSD_IMG_INFO tOsdImgInfo;

	tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo);
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
	UI_ClearBuConnectStatusFlag();
	switch(tResult)
	{
		case rUI_SUCCESS:
			tUI_PuSetting.ubPairedBuNum += (tUI_PuSetting.ubPairedBuNum >= tUI_PuSetting.ubTotalBuNum)?0:1;
			tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
			tUI_CamStatus[tPairInfo.tPairSelCam].tCamDispLocation = tPairInfo.tDispLocation;
			for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
			{
				if(tCamNum == tPairInfo.tPairSelCam)
					continue;
				if((INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID) &&
				   (tUI_CamStatus[tCamNum].tCamDispLocation == tPairInfo.tDispLocation))
				{
					APP_EventMsg_t tUI_UnindBuMsg = {0};

					tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_CAM_EVENT;
					tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
					tUI_UnindBuMsg.ubAPP_Message[1] = tCamNum;
					UI_SendMessageToAPP(&tUI_UnindBuMsg);
					tUI_CamStatus[tCamNum].ulCAM_ID    = INVALID_ID;
					tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
					tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
					UI_ResetDevSetting(tCamNum);
				}
			}
			if((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
			   (NO_CAM == tUI_PuSetting.tAdoSrcCamNum) ||
			   (INVALID_ID == tUI_CamStatus[tUI_PuSetting.tAdoSrcCamNum].ulCAM_ID))
				tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;
			UI_ResetDevSetting(tPairInfo.tPairSelCam);
			UI_UpdateDevStatusInfo();
			break;
		default:
			break;
	}
	ubUI_StopUpdateStsBarFlag = FALSE;
	tUI_State = UI_DISPLAY_STATE;
}
//------------------------------------------------------------------------------
void UI_SetLdDispStatus(UI_OsdLdDispSts_t tDispSts)
{
	osMutexWait(osUI_OsdLdStsUpdMutex, osWaitForever);
	tUI_OsdLdDispSts = tDispSts;
	osMutexRelease(osUI_OsdLdStsUpdMutex);
}
//------------------------------------------------------------------------------
UI_OsdLdDispSts_t UI_GetLdDispStatus(void)
{
	UI_OsdLdDispSts_t tDispSts;

	osMutexWait(osUI_OsdLdStsUpdMutex, osWaitForever);
	tDispSts = tUI_OsdLdDispSts;
	osMutexRelease(osUI_OsdLdStsUpdMutex);
	return tDispSts;
}
//------------------------------------------------------------------------------
void UI_ClearRecordStatusOsdImg(void)
{
	OSD_IMG_INFO tRecEreOsdImgInfo;

	tRecEreOsdImgInfo.uwHSize  = 50;
	tRecEreOsdImgInfo.uwVSize  = 150;
	tRecEreOsdImgInfo.uwXStart = 40;
	tRecEreOsdImgInfo.uwYStart = uwOSD_GetVSize() - 150;
	OSD_EraserImg2(&tRecEreOsdImgInfo);
}
//------------------------------------------------------------------------------
static void UI_OsdLoadingDisplayThread(void const *argument)
{
	uint32_t ulUI_OsdLdWaitTickTime = osWaitForever;
	uint16_t uwUI_OsdLdDispSte;
	uint8_t ubLdOsdIdx = 0;
	uint8_t ubUI_RecChgImgFlag = TRUE;
	uint8_t ubFlag = 0;
	OSD_IMG_INFO tLdOsdImg[17], tWorkOsdImg[8], tRecOsdImgInfo[3];
	OSD_RESULT tLdOsdImgRdRet = OSD_OK, tWorkOsdImgRdRet = OSD_OK, tRecOsdImgRdRet = OSD_OK;

	tLdOsdImgRdRet 	 = tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_LOADING1_P1_ICON, 16, &tLdOsdImg);
	tWorkOsdImgRdRet = tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_WORKING_P1_ICON, 8, &tWorkOsdImg);
	tRecOsdImgRdRet  = tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECORDINGNACT_ICON, 3, &tRecOsdImgInfo);
	if((OSD_OK != tLdOsdImgRdRet) || (OSD_OK != tWorkOsdImgRdRet) || (OSD_OK != tRecOsdImgRdRet))
		printd(DBG_ErrorLvl, "[%d:%d:%d]Load OSD Image FAIL, pls check (%d) !\n", tLdOsdImgRdRet, tWorkOsdImgRdRet, tRecOsdImgRdRet, __LINE__);
	while(1)
	{
		osMessageGet(osUI_OsdLdDispQueue, &uwUI_OsdLdDispSte, ulUI_OsdLdWaitTickTime);
		switch(uwUI_OsdLdDispSte)
		{
			case UI_SEARCH_DCIMFOLDER:
			case UI_SEARCH_RECFILES:
				while(UI_OSDLDDISP_ON == UI_GetLdDispStatus())
				{
					tOSD_Img2(&tLdOsdImg[ubLdOsdIdx++], OSD_UPDATE);
					ubLdOsdIdx = (ubLdOsdIdx >= 16)?0:ubLdOsdIdx;
					osDelay(250);
				}
				tLdOsdImg[16].uwHSize  = 200;
				tLdOsdImg[16].uwVSize  = 250;
				tLdOsdImg[16].uwXStart = 275;
				tLdOsdImg[16].uwYStart = 550;
				OSD_EraserImg2(&tLdOsdImg[16]);
				uwUI_OsdLdDispSte = UI_LD_DEFU;
				break;
			case UI_SDCARD_FORMAT:
			{
				uint16_t uwTmpXStart, uwTmpYStart;
				ubLdOsdIdx = 0;
				uwTmpXStart = tWorkOsdImg[ubLdOsdIdx].uwXStart;
				uwTmpYStart = tWorkOsdImg[ubLdOsdIdx].uwYStart;
				while(UI_OSDLDDISP_ON == UI_GetLdDispStatus())
				{
					if(UI_SD_NRDY == tUI_SdCardSts)
					{
						tWorkOsdImg[ubLdOsdIdx].uwXStart = 353;
						tWorkOsdImg[ubLdOsdIdx].uwYStart = 563;
					}
					tOSD_Img2(&tWorkOsdImg[ubLdOsdIdx], OSD_UPDATE);
					tWorkOsdImg[ubLdOsdIdx].uwXStart = uwTmpXStart;
					tWorkOsdImg[ubLdOsdIdx].uwYStart = uwTmpYStart;
					ubLdOsdIdx = (++ubLdOsdIdx >= 8)?0:ubLdOsdIdx;
					osDelay(250);
				}
				uwUI_OsdLdDispSte = UI_LD_DEFU;
				break;
			}
			default:
				if((APP_LINK_STATE == tUI_SyncAppState) && (UI_DISPLAY_STATE == tUI_State) &&
				   (LCD_JPEG_DISABLE == tLCD_GetJpegDecoderStatus()))
				{
					if(UI_REC_START == tUI_RecPlayAct.tRecAct)
					{
						tOSD_Img2(((FALSE == ubUI_RecChgImgFlag)?&tRecOsdImgInfo[0]:&tRecOsdImgInfo[1]), OSD_QUEUE);
						ubUI_RecChgImgFlag = (FALSE == ubUI_RecChgImgFlag)?TRUE:FALSE;
						if (!ubFlag)
						{
							tOSD_Img2(&tRecOsdImgInfo[2], OSD_UPDATE);
							ubFlag = 1;
						}
						else
							OSD_UpdateQueueBuf();
						ulUI_OsdLdWaitTickTime = 500;
					}
					else
					{
						UI_ClearRecordStatusOsdImg();
						ubUI_RecChgImgFlag	   = FALSE;
						ubFlag = 0;
						ulUI_OsdLdWaitTickTime = osWaitForever;
					}
				}
				else
				{
					ulUI_OsdLdWaitTickTime = 200;
					ubUI_RecChgImgFlag 	   = TRUE;
					ubFlag = 0;
				}
				break;
		}
	}
}
//------------------------------------------------------------------------------
static void UI_FuncsExecuteThread(void const *argument)
{
	UI_FuncExecMsg_t tUI_FuncsExecMsg;
	uint8_t ubUI_FuncsExecRet = rUI_SUCCESS;
	while(1)
	{
		osMessageGet(osUI_FuncsExecQue, &tUI_FuncsExecMsg, osWaitForever);
		switch(tUI_FuncsExecMsg.uwFunc)
		{
			case UI_SDCARDFMT_ACT:
			{
				uint16_t uwLdState = UI_SDCARD_FORMAT;

				if(UI_SD_NRDY == tUI_SdCardSts)
				{
					OSD_IMG_INFO tOsdImgInfo;

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFING_BG, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				UI_SetLdDispStatus(UI_OSDLDDISP_ON);
				osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
				ubUI_FuncsExecRet = (FORMAT_FAIL == FS_MediaFormat(KNL_GetFsMedia()))?rUI_FAIL:rUI_SUCCESS;
				UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
				if((rUI_SUCCESS == ubUI_FuncsExecRet) &&
				   ((UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode)))
					UI_VideoRecordingExec(UI_REC_START);
				break;
			}
			case UI_PERDBGRPT_ACT:
				UI_DisplayTrxInfo(tUI_FuncsExecMsg);
				continue;
			default:
				continue;
		}
		osMessagePut(osUI_FuncsFinExecQue, &ubUI_FuncsExecRet, 0);
	}
}
//------------------------------------------------------------------------------
void UI_SetRecImgColor(UI_RecImgColor_t tColorNum)
{
	static UI_RecImgColor_t tUI_RecImgColor = UI_RECIMG_DEFU;	
	uint8_t i;
	
	if(NULL == pUI_RecOsdImgDB)
	{
		pUI_RecOsdImgDB = osMalloc(sizeof(UI_RecOsdImgDb_t));
		memset(pUI_RecOsdImgDB, 0, sizeof(UI_RecOsdImgDb_t));
	}
	if(tUI_RecImgColor == tColorNum)
		return;
	for(i = 0; i < 10; i++)
		pUI_RecOsdImgDB->uwUI_RecNumArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_NUM0 + (tColorNum * 10)) + i):
																				(OSD2IMG_FILE_C6_NUM0 + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
	tUI_RecOsdImgInfo.pNumImgIdxArray = pUI_RecOsdImgDB->uwUI_RecNumArray;
	for(i = 0; i < 26; i++)
	{
		pUI_RecOsdImgDB->uwUI_RecUpperLetterArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_UPA + (tColorNum * 26)) + i):
																						(OSD2IMG_FILE_C6_UPA + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
		pUI_RecOsdImgDB->uwUI_RecLowerLetterArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_LWA + (tColorNum * 26)) + i):
																						(OSD2IMG_FILE_C6_LWA + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
	}
	tUI_RecOsdImgInfo.pUpperLetterImgIdxArray = pUI_RecOsdImgDB->uwUI_RecUpperLetterArray;
	tUI_RecOsdImgInfo.pLowerLetterImgIdxArray = pUI_RecOsdImgDB->uwUI_RecLowerLetterArray;
	for(i = 0; i < 4; i++)
		pUI_RecOsdImgDB->uwUI_RecSymbolArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_COLON + (tColorNum * 4)) + i):
																				   (OSD2IMG_FILE_C6_COLON + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
	tUI_RecOsdImgInfo.pSymbolImgIdxArray = pUI_RecOsdImgDB->uwUI_RecSymbolArray;
	tUI_RecImgColor = tColorNum;
}	
//------------------------------------------------------------------------------
void UI_SearchDCIMFolder(void)
{
	uint16_t uwLdState = UI_SEARCH_DCIMFOLDER;
	
	UI_SetLdDispStatus(UI_OSDLDDISP_ON);
	osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
	osDelay(200);
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    pUI_RecFoldersInfo.uwTotalRecFolderNum = ulKNL_GetSortingFolders(KNL_SIM_FLD,SORT_BY_NAME_DESCENDING,&pUI_RecFoldersInfo.tRecFolderInfo[0]);
#else
    pUI_RecFoldersInfo.uwTotalRecFolderNum = ulKNL_GetSortingFolders(KNL_REAL_FLD,SORT_BY_NAME_DESCENDING,&pUI_RecFoldersInfo.tRecFolderInfo[0]);
#endif
	UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
	osDelay(300);
}
//------------------------------------------------------------------------------
void UI_ListDCIMFolderInfo(UI_RecFoldersInfo_t *pFldInfo,uint16_t uwStartFileIdx, uint16_t uwEndFileIdx, OSD_UPDATE_TYP tUpdateMode)
{
	OSD_IMG_INFO tRecOsdImgInfo[2];
	uint16_t uwRecXStart[2] = {0, 0}, uwRecYStart[2] = {0, 0};
	uint8_t ubRecFolderIdx, ubOpenOsdIdx = 0;
	uint16_t uwIdx;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_FOLDERCLOSE_ICON, 2, &tRecOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwRecXStart[0] = tRecOsdImgInfo[0].uwXStart;
	uwRecYStart[0] = tRecOsdImgInfo[0].uwYStart;
	uwRecXStart[1] = tRecOsdImgInfo[1].uwXStart;
	uwRecYStart[1] = tRecOsdImgInfo[1].uwYStart;
	for(uwIdx = uwStartFileIdx; uwIdx < uwEndFileIdx; uwIdx++)
	{
        UI_SetRecImgColor((pFldInfo->uwRecFolderSelIdx == uwIdx)?UI_RECIMG_COLOR6:UI_RECIMG_COLOR5);
		ubOpenOsdIdx = (pFldInfo->uwRecFolderSelIdx == uwIdx)?1:0;
		ubRecFolderIdx = uwIdx % REC_FOLDER_LIST_MAXNUM;
		tRecOsdImgInfo[ubOpenOsdIdx].uwYStart -= ((ubRecFolderIdx % 5) * 185);
		tRecOsdImgInfo[ubOpenOsdIdx].uwXStart += ((ubRecFolderIdx / 5) * 265);
		tOSD_Img2(&tRecOsdImgInfo[ubOpenOsdIdx], OSD_QUEUE);
	    OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubRecFolderIdx % 5) * 185)), (263 + ((ubRecFolderIdx / 5) * 265)), tUI_RecOsdImgInfo, tUpdateMode, pFldInfo->tRecFolderInfo[uwIdx].FldName.chName);
		tRecOsdImgInfo[ubOpenOsdIdx].uwXStart = uwRecXStart[ubOpenOsdIdx];
		tRecOsdImgInfo[ubOpenOsdIdx].uwYStart = uwRecYStart[ubOpenOsdIdx];
	}
}
//------------------------------------------------------------------------------
void UI_DrawDCIMFolderMenu(void)
{
	OSD_IMG_INFO tRecBgOsdImgInfo, tRecOsdImgInfo;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecBgOsdImgInfo) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecOsdImgInfo) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tUI_State = (TRUE == ubUI_RecSubMenuFlag)?UI_SUBMENU_STATE:UI_RECFOLDER_SEL_STATE;
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tRecBgOsdImgInfo.uwXStart = 0;
	tRecBgOsdImgInfo.uwYStart = 0;
#if !APP_FS_FILE_LIST_STYLE
	tOSD_Img1(&tRecBgOsdImgInfo, OSD_QUEUE);
#else
	if(KNL_ThmShowInfo.ubInFldListFlg==1)
	{
		KNL_ThmShowInfo.ubInFldListFlg = 0;
		tOSD_Img1(&tRecBgOsdImgInfo, OSD_QUEUE);
	}
#endif
	tOSD_Img2(&tRecOsdImgInfo, OSD_UPDATE);
	if(SCAN_VIEW == tCamViewSel.tCamViewType)
	{
		ubUI_DisScanMdFunc = TRUE;
		UI_DisableScanMode();
	}
	UI_SearchDCIMFolder();
	if(pUI_RecFoldersInfo.uwTotalRecFolderNum)
		UI_ListDCIMFolderInfo(&pUI_RecFoldersInfo,0, ((pUI_RecFoldersInfo.uwTotalRecFolderNum < REC_FOLDER_LIST_MAXNUM)?pUI_RecFoldersInfo.uwTotalRecFolderNum:REC_FOLDER_LIST_MAXNUM), OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DCIMFolderSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tFolderSelOsdImgInfo[2], tOsdImgInfo;
	uint8_t ubUI_RecFolderIdx = 0, ubUI_PrevRecFolderIdx = 0;
	uint8_t ubRefreshFldIconFlag = FALSE, ubRefreshFldIdx, ubFldChkIdx;
	uint16_t uwRecSelIdx;

	if((!pUI_RecFoldersInfo.uwTotalRecFolderNum) && (EXIT_ARROW != tArrowKey))
		return;
    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_FOLDERCLOSE_ICON, 2, &tFolderSelOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwRecSelIdx = pUI_RecFoldersInfo.uwRecFolderSelIdx;
	ubUI_RecFolderIdx = (uwRecSelIdx % REC_FOLDER_LIST_MAXNUM);
	switch(tArrowKey)
	{
		case LEFT_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			if(!(ubUI_RecFolderIdx % 5))
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx     = (ubUI_PrevRecFolderIdx - 1);
			--pUI_RecFoldersInfo.uwRecFolderSelIdx;
			break;
		case RIGHT_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			if((((ubUI_RecFolderIdx % 5) + 1) >= 5) || ((uwRecSelIdx + 1) >= pUI_RecFoldersInfo.uwTotalRecFolderNum)) 
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx     = (ubUI_PrevRecFolderIdx + 1);
			++pUI_RecFoldersInfo.uwRecFolderSelIdx;
			break;
		case UP_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			if(pUI_RecFoldersInfo.uwRecFolderSelIdx < 5)
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx    -= 5;
			ubRefreshFldIdx = (pUI_RecFoldersInfo.uwRecFolderSelIdx > REC_FOLDER_LIST_MAXNUM)?((pUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM) >= 5)?0:(pUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM):0;
			ubRefreshFldIconFlag  = (!((pUI_RecFoldersInfo.uwRecFolderSelIdx - ubRefreshFldIdx) % REC_FOLDER_LIST_MAXNUM))?TRUE:FALSE;
			pUI_RecFoldersInfo.uwRecFolderSelIdx -= 5;
			break;
		case DOWN_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			ubFldChkIdx = (ubUI_RecFolderIdx / 5)?(uwRecSelIdx - (uwRecSelIdx % 5) + 5):(uwRecSelIdx + 5);
        	if(ubFldChkIdx >= pUI_RecFoldersInfo.uwTotalRecFolderNum)
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx    += 5;
			pUI_RecFoldersInfo.uwRecFolderSelIdx = ((pUI_RecFoldersInfo.uwRecFolderSelIdx + 5) >= pUI_RecFoldersInfo.uwTotalRecFolderNum)?
													 (((pUI_RecFoldersInfo.uwRecFolderSelIdx + 5) / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM):(pUI_RecFoldersInfo.uwRecFolderSelIdx + 5);
			ubRefreshFldIdx = (pUI_RecFoldersInfo.uwRecFolderSelIdx > REC_FOLDER_LIST_MAXNUM)?((pUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM) >= 5)?0:(pUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM):0;
			ubRefreshFldIconFlag  = (!((pUI_RecFoldersInfo.uwRecFolderSelIdx - ubRefreshFldIdx) % REC_FOLDER_LIST_MAXNUM))?TRUE:FALSE;
			break;
		case ENTER_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
#if !APP_FS_FILE_LIST_STYLE
			tOsdImgInfo.uwHSize  = 600;
			tOsdImgInfo.uwVSize  = 955;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
            if( pUI_RecFoldersInfo.uwRecFolderSelIdx < ubKNL_GetTXFldNum())
            {
                if(ubUI_RecSubMenuFlag == TRUE)
                    ubUI_RecSubMenuTXFldFlag = TRUE;
                UI_DrawTXFolderMenu();
            }
            else
#endif
    			UI_DrawRecordFileMenu();
#else
			tOsdImgInfo.uwHSize  = 0;
			tOsdImgInfo.uwVSize  = 0;
			tOsdImgInfo.uwXStart = 720;
			tOsdImgInfo.uwYStart = 1280;
			OSD_EraserImg2(&tOsdImgInfo);
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
            if( pUI_RecFoldersInfo.uwRecFolderSelIdx < ubKNL_GetTXFldNum())
            {
                if(ubUI_RecSubMenuFlag == TRUE)
                    ubUI_RecSubMenuTXFldFlag = TRUE;
                UI_DrawTXFolderMenu();
            }
            else
#endif
            {
                if((UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (UI_REC_START == tUI_RecPlayAct.tRecAct))
                {
                    UI_VideoRecordingExec(UI_REC_STOP);
                }
                KNL_ThumbnailSwtichView();
    		    UI_ClearOsdImage();
       			UI_DrawRecordFileMenu();
   			}
#endif
			return;
		case EXIT_ARROW:
			ubUI_DisScanMdFunc = FALSE;
			pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
			if(APP_LOSTLINK_STATE == tUI_SyncAppState)
			{
				tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
				tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
				UI_ClearBuConnectStatusFlag();
				tUI_State = UI_DISPLAY_STATE;
				return;
			}
			tOsdImgInfo.uwHSize  = 600;
			tOsdImgInfo.uwVSize  = 1180;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
			UI_DrawCameraSettingMenu(UI_CAMISP_SETUP);
			return;
		default:
			return;
	}
	if(TRUE == ubRefreshFldIconFlag)
	{
		uint16_t uwFldStartIdx = 0, uwFldEndIdx = 0;

		tOsdImgInfo.uwHSize  = 600;
		tOsdImgInfo.uwVSize  = 955;
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 50;
		OSD_EraserImg2(&tOsdImgInfo);
		uwFldStartIdx = (pUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;
		if(DOWN_ARROW == tArrowKey)
		{
			uwFldEndIdx = pUI_RecFoldersInfo.uwTotalRecFolderNum - pUI_RecFoldersInfo.uwRecFolderSelIdx;
			uwFldEndIdx = (uwFldEndIdx < REC_FOLDER_LIST_MAXNUM)?pUI_RecFoldersInfo.uwTotalRecFolderNum:(uwFldStartIdx + REC_FOLDER_LIST_MAXNUM);
		}
		else
		{
			uwFldEndIdx = uwFldStartIdx + REC_FOLDER_LIST_MAXNUM;
			ubRefreshFldIdx = pUI_RecFoldersInfo.uwRecFolderSelIdx;			
		}
		if(ubRefreshFldIdx)
		{
			ubUI_PrevRecFolderIdx = 0;
			ubUI_RecFolderIdx 	  = ubRefreshFldIdx % REC_FOLDER_LIST_MAXNUM;
			ubRefreshFldIconFlag  = FALSE;
		}
        UI_ListDCIMFolderInfo(&pUI_RecFoldersInfo,uwFldStartIdx, uwFldEndIdx, (ubRefreshFldIdx)?OSD_QUEUE:OSD_UPDATE);
	}
	if(FALSE == ubRefreshFldIconFlag)
	{
		uwRecSelIdx = (pUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;
		UI_SetRecImgColor(UI_RECIMG_COLOR5);
		tFolderSelOsdImgInfo[0].uwYStart -= ((ubUI_PrevRecFolderIdx % 5) * 185);
		tFolderSelOsdImgInfo[0].uwXStart += ((ubUI_PrevRecFolderIdx / 5) * 265);
		tOSD_Img2(&tFolderSelOsdImgInfo[0], OSD_QUEUE);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubUI_PrevRecFolderIdx % 5) * 185)), (263 + ((ubUI_PrevRecFolderIdx / 5) * 265)),
						tUI_RecOsdImgInfo, OSD_QUEUE, pUI_RecFoldersInfo.tRecFolderInfo[ubUI_PrevRecFolderIdx + uwRecSelIdx].FldName.chName);
		UI_SetRecImgColor(UI_RECIMG_COLOR6);
		tFolderSelOsdImgInfo[1].uwYStart -= ((ubUI_RecFolderIdx % 5) * 185);
		tFolderSelOsdImgInfo[1].uwXStart += ((ubUI_RecFolderIdx / 5) * 265);
		tOSD_Img2(&tFolderSelOsdImgInfo[1], OSD_QUEUE);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubUI_RecFolderIdx % 5) * 185)), (263 + ((ubUI_RecFolderIdx / 5) * 265)),
						tUI_RecOsdImgInfo, OSD_UPDATE, pUI_RecFoldersInfo.tRecFolderInfo[ubUI_RecFolderIdx + uwRecSelIdx].FldName.chName);
	}
}
//------------------------------------------------------------------------------
void UI_StopPlayRecordFile(uint8_t ubPlayRet)
{
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};

	switch(ubPlayRet)
	{
		case KNL_VDOPLAY_STOP:
            if(tUI_RecPlayAct.tPlaySts != UI_RECFILE_STOP)
            {
    			tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
    			UI_RecordPlayListSelection(DOWN_ARROW);
            }
			break;
		default:
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
		    UI_RemotePlayTrigger(0);
#endif
			if(UI_RECFILE_PLAY == tUI_RecPlayAct.tPlaySts)
			{
				printd(DBG_ErrorLvl, "Video Play Err !\n");
				tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
                tUI_PlayAct.pRecordStsNtyCb = NULL;
    			tKNL_ExecRecordFunc(tUI_PlayAct);
				UI_RecordPlayListSelection(EXIT_ARROW);
			}
			else if(UI_RECFILES_SEL_STATE == tUI_State || UI_RECTXFILES_SEL_STATE == tUI_State)
			{
				OSD_IMG_INFO tRecPlayOsdImgInfo;

				tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
                tUI_PlayAct.pRecordStsNtyCb = NULL;
    			tKNL_ExecRecordFunc(tUI_PlayAct);
				tRecPlayOsdImgInfo.uwHSize  = 415;
				tRecPlayOsdImgInfo.uwVSize  = 955;
				tRecPlayOsdImgInfo.uwXStart = 195;
				tRecPlayOsdImgInfo.uwYStart = 50;
				OSD_EraserImg2(&tRecPlayOsdImgInfo);
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
                if(tUI_State == UI_RECTXFILES_SEL_STATE)
                    UI_ReDrawTXRecordFileMenu();
                else
#endif
                    UI_DrawRecordFileMenu();
			}
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PlayRecordFile(KNL_FldType_t tSimFld,uint16_t uwRecFileIndex,UI_RecFoldersInfo_t *pFldInfo,UI_RecFilesInfo_t *pFileInfo)
{
    KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
    uint16_t uwIdx;
    uint8_t ubPlayIdx = 0;

    if(ubPLY_GetOpMode() == PLY_MODE_R)
    {
        printd(DBG_ErrorLvl, "PLY Fail(Now Recording) \n");
        return;
    }

    if(!memcmp(pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName.chExt, "JPG", 3))
	{
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
	    if(tSimFld == KNL_SIM_FLD)
	    {
	        printd(DBG_ErrorLvl,"Tx Remote Photo play not Support\n");
                return;
	    }
#endif
	    tUI_PlayAct.tSimFolder     = tSimFld;
		tUI_PlayAct.tRecordFunc  	= KNL_PHOTO_PLAY;
		tUI_PlayAct.pRecordStsNtyCb = UI_PhotoPlayFinish;
		memset(&tUI_PlayAct.tPhotoPlayInfo.FileName, 0, sizeof(tUI_PlayAct.tPhotoPlayInfo.FileName));
		memcpy(&tUI_PlayAct.tPhotoPlayInfo.FileName, &pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName, sizeof(pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName));
		tUI_PlayAct.tPhotoPlayInfo.SrcNum =(FS_SRC_NUM)(pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SrcNum + FS_JPG_SRC_0);        
		tUI_PlayAct.tPhotoPlayInfo.ulFirstClus = pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.ulFirstClus;
		tUI_PlayAct.tPhotoPlayInfo.ulFileSize = pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.ullFileSize;
		tUI_PlayAct.tPhotoPlayInfo.NoFatChainFlag = pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.NoFatChainFlag;
		VDO_Stop();
		if(KNL_OK == tKNL_ExecRecordFunc(tUI_PlayAct))
			tUI_State = UI_PHOTOPLAYNRDY_STATE;
		else
			printd(DBG_ErrorLvl, "Photo play err !\n");
	}
    else if(!memcmp(pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName.chExt, "MP4", 3))
	{
		OSD_IMG_INFO tRecPlayOsdImgInfo[12];
		KNL_ROLE tRecRoleNum;
		UI_CamNum_t tRecCamNum;
		uint8_t ubImgIdx = 0;

#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
        if(tSimFld == KNL_SIM_FLD)
        {
            osEvent osUI_TXPLYFinSig;
			uint8_t ubStatus;
            APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecCamNum);
            ubStatus = UI_TXFsDrawEvent(tRecCamNum,UI_FS_ENTER,UI_FS_FILE,SORT_BY_NAME_DESCENDING,pFileInfo->uwRecFileSelIdx);
            if( ubStatus == 0 )
            {
                UI_TXPlayEvent(tRecCamNum,UI_TXPLY_PLAYCAM,(uint8_t)UI_PLY_NEXT,NULL,pFileInfo->uwRecFileSelIdx);
                osUIThreadIdBackup = osThreadGetId();
                osUI_TXPLYFinSig = osSignalWait(osUI_TXPlyFinSignal, UI_TWC_TIMEOUT);
                if((osUI_TXPLYFinSig.status == osEventSignal) || (osUI_TXPLYFinSig.value.signals == osUI_TXPlyFinSignal))
                {
                    if(ubUI_PlayEventSt)
                    {
						printd(DBG_ErrorLvl,"Tx Remote play fail\n");
                        return;
                    }
                }
                else
                {
                    printd(DBG_ErrorLvl,"PlayEvent TimeOut\n");
                    return;
                }
            }
            else if (ubStatus == 1)
            {
                UI_SetRecImgColor(UI_RECIMG_COLOR2);
                OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Please Check Remote SD");
                return;
            }
            else if (ubStatus == 2)
            {
                UI_SetRecImgColor(UI_RECIMG_COLOR2);
                OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Enter File Timeout");
                return;
            }
            else if (ubStatus == 3)
            {
                UI_SetRecImgColor(UI_RECIMG_COLOR2);
                OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Data Channel Not Ready");
                return;
            }
        }
#endif
		if(tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_RECPLAYLISTBG, 1, &tRecPlayOsdImgInfo[0]) != OSD_OK)
		{
			printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
			return;
		}
		if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PAUSENOR_ICON, 10, &tRecPlayOsdImgInfo[1]) != OSD_OK)
		{
			printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
			return;
		}
		if(UI_RECPLAYLIST_STATE == tUI_State)
		{
		    tUI_RecPlayAct.tPlaySts = UI_RECFILE_PLAY;
            ubPLY_Jump(PLY_JUMP_RESTART);
			KNL_ResetLcdChannel();
			tOSD_Img2(&tRecPlayOsdImgInfo[2], OSD_UPDATE);
		}
		else
		{
			for(tRecCamNum = CAM1; tRecCamNum <= CAM4; tRecCamNum++)
				tUI_RecPlayAct.tVdoPlayCam[tRecCamNum] = NO_CAM;
            tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SrcNum);
			if(KNL_NONE != tRecRoleNum)
			{
				APP_KNLRoleMap2CamNum(tRecRoleNum, tRecCamNum);
				tUI_RecPlayAct.tVdoPlayCam[tRecCamNum] = tRecCamNum;
				tUI_RecPlayAct.tAdoPlayCam = tRecCamNum;
				ubPLY_AdoChannelSet(tUI_RecPlayAct.tAdoPlayCam);
			}
            tUI_PlayAct.tSimFolder      = tSimFld;
            tUI_PlayAct.uwSimFldSelIdx  = pUI_RecFoldersInfo.uwRecFolderSelIdx;
			tUI_PlayAct.tRecordFunc  	= KNL_VIDEO_PLAY;
			tUI_PlayAct.pRecordStsNtyCb = UI_StopPlayRecordFile;
			tUI_PlayAct.tPlayDispTye	= (KNL_DISP_TYPE)pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SubHidnInfo.ubPreviewMode;
			tUI_RecPlayAct.ubPlayMode	= tUI_PlayAct.tPlayDispTye;
			if(KNL_DISP_SINGLE == tUI_PlayAct.tPlayDispTye)
			{
				tUI_PlayAct.ulVideoPlayIdx[0] = uwRecFileIndex;
				tUI_PlayAct.ubPlayFileNum = 1;
			}
			else
			{
				tUI_PlayAct.ubPlayFileNum = 0;
				for(uwIdx = 0; uwIdx < pFileInfo->uwTotalRecFileNum; uwIdx++)
				{
					if(tUI_PlayAct.ubPlayFileNum)
					{
						 if(++ubPlayIdx > 4)
							break;
					}
					if(pFileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.uwGroupIdx == pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.uwGroupIdx)
					{
						tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)pFileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.SrcNum);
						if(KNL_NONE != tRecRoleNum)
						{
							APP_KNLRoleMap2CamNum(tRecRoleNum, tRecCamNum);
							tUI_RecPlayAct.tVdoPlayCam[tRecCamNum] = tRecCamNum;
						}
						tUI_PlayAct.ulVideoPlayIdx[tUI_PlayAct.ubPlayFileNum++] = uwIdx;
					}
				}
			}
			UI_DisableScanMode();
			if(KNL_OK == tKNL_ExecRecordFunc(tUI_PlayAct))
			{
				tUI_RecPlayAct.tPlaySts = UI_RECFILE_PLAY;
                tUI_RecPlayAct.tSimFld = tSimFld;
				tUI_State = UI_RECPLAYLIST_STATE;
				tRecPlayOsdImgInfo[11].uwHSize  = uwOSD_GetHSize();
				tRecPlayOsdImgInfo[11].uwVSize  = uwOSD_GetVSize();
				tRecPlayOsdImgInfo[11].uwXStart = 0;
				tRecPlayOsdImgInfo[11].uwYStart = 0;
				OSD_EraserImg1(&tRecPlayOsdImgInfo[11]);
				OSD_Weight(OSD_WEIGHT_6DIV8);
				tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
				for(ubImgIdx = 3; ubImgIdx < 10; ubImgIdx+=2)
					tOSD_Img2(&tRecPlayOsdImgInfo[ubImgIdx], OSD_QUEUE);
				tOSD_Img2(&tRecPlayOsdImgInfo[2], OSD_UPDATE);
			}
			else
            {         
				tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
                tUI_RecPlayAct.tSimFld = KNL_REAL_FLD;
            }
		}
	}
}
//------------------------------------------------------------------------------
uint8_t UI_DeleteRecordFile(uint16_t uwRecFileIndex)
{
    FS_KNL_MANUAL_DEL_PROCESS_t tUI_DelAct; 
    uint8_t ubFsTimeout = 150;
	FS_MEDIA_SEL MediaSel;
	
	MediaSel = KNL_GetFsMedia();
	
	tUI_DelAct.MediaSel = MediaSel;
    memset(&tUI_DelAct.FldName, 0, sizeof(tUI_DelAct.FldName));
    memset(&tUI_DelAct.FileName, 0, sizeof(tUI_DelAct.FileName));
	memcpy(&tUI_DelAct.FldName, &pUI_RecFoldersInfo.tRecFolderInfo[pUI_RecFoldersInfo.uwRecFolderSelIdx].FldName, sizeof(pUI_RecFoldersInfo.tRecFolderInfo[pUI_RecFoldersInfo.uwRecFolderSelIdx].FldName));
	memcpy(&tUI_DelAct.FileName, &pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName, sizeof(pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName));
	if( memcmp(tUI_DelAct.FldName.chName,"EMG", 3)==0 )
		tUI_DelAct.FilePath = FILE_PATH3;
	else if( memcmp(tUI_DelAct.FldName.chName,"TIMELAPS", 8)==0 )
		tUI_DelAct.FilePath = FILE_PATH4;
	else
    tUI_DelAct.FilePath = pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FilePath;
    FS_ManualDeleteFile(&tUI_DelAct);
    while(FS_ChkManualDelStatus(MediaSel) != FS_MANUAL_DEL_OK)
	{
		osDelay(20);
		if(!--ubFsTimeout)
		{
			printd(DBG_ErrorLvl, "Delete File Err !!\n");
			return FALSE;
		}
	}
	
    return TRUE;
}
//------------------------------------------------------------------------------
void UI_PhotoPlayListSelection(UI_ArrowKey_t tArrowKey)
{
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
#if !APP_FS_FILE_LIST_STYLE
	OSD_IMG_INFO tRecPlayOsdImgInfo[2];
#else
	OSD_IMG_INFO tOsdImgInfo;
#endif
	switch(tArrowKey)
	{
		case EXIT_ARROW:
#if !APP_FS_FILE_LIST_STYLE
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
			tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
			tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_QUEUE);
#else
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
			tOsdImgInfo.uwHSize  = 720;
			tOsdImgInfo.uwVSize  = 1280;
			OSD_EraserImg2(&tOsdImgInfo);
#endif
			UI_DrawRecordFileMenu();
			tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
            tUI_PlayAct.pRecordStsNtyCb = NULL;
			tKNL_ExecRecordFunc(tUI_PlayAct);
#if !APP_FS_FILE_LIST_STYLE
			if(TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag)
			{
			#ifdef S2019A
				tLCD_JpegDecodeDisable();
				OSD_LogoJpeg((sPRF_APDIRECT_MODE == tsPRF_GetDrvMode())?OSDLOGO_WIFIDT_ENY:OSDLOGO_LOSTLINK);
			#else
				OSD_LogoJpeg(OSDLOGO_LOSTLINK);
			#endif
			}
#else
			tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;
#endif
            VDO_Start();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DrawRecordPlayAudioSource(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwDisp2TImgIdx[4] = {OSD2IMG_SELADOCAM1ONLINE_ICON,   OSD2IMG_SELADOCAM1ONLINE_ICON,
								  OSD2IMG_SELADOCAM2_1ONLINE_ICON, OSD2IMG_SELADOCAM2_1OFFLINE_ICON};
	uint16_t uwDisp4TImgIdx[8] = {OSD2IMG_SELADOCAM1ONLINE_ICON, OSD2IMG_SELADOCAM1OFFLINE_ICON,
								  OSD2IMG_SELADOCAM2ONLINE_ICON, OSD2IMG_SELADOCAM2OFFLINE_ICON,
								  OSD2IMG_SELADOCAM3ONLINE_ICON, OSD2IMG_SELADOCAM3OFFLINE_ICON,
								  OSD2IMG_SELADOCAM4ONLINE_ICON, OSD2IMG_SELADOCAM4OFFLINE_ICON};
	uint16_t uwDisplayImgIdx;
	uint16_t uwXOffset = 0, uwYOffset;
	uint16_t uwYItemOffset = 150;
	UI_CamNum_t tCamNum;

	uwYOffset = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?150:0;
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		uwDisplayImgIdx  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?uwDisp2TImgIdx[tCamNum*2]:uwDisp4TImgIdx[tCamNum*2];
		uwDisplayImgIdx += ((NO_CAM == tUI_RecPlayAct.tVdoPlayCam[tCamNum])?1:0);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwDisplayImgIdx, 1, &tOsdImgInfo);
		tOsdImgInfo.uwYStart -= uwYOffset;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tUI_RecPlayAct.tAdoPlayCam*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_State = UI_RECPLAYADOSRC_SEL_STATE;
}
//------------------------------------------------------------------------------
static UI_RecPlayStatus_t tUI_RecPlaySts = UI_RECFILE_PLAY;
static uint16_t uwRecPlayListImgIdx[UI_RECPLAYLISTITEM_MAX] = {OSD2IMG_REC_SKIPBACKWARDNOR_ICON, OSD2IMG_REC_PAUSENOR_ICON,
											                   OSD2IMG_REC_SKIPFORWARDNOR_ICON, OSD2IMG_REC_ADOSRCSELNOR_ICON};
void UI_RecordPlayListSelection(UI_ArrowKey_t tArrowKey)
{
	static UI_RecPlayListItem_t tUI_RecPlayListItem = UI_RECPLAYPAUSE_ITEM;
	UI_RecPlayListItem_t tPrevRecListItem = UI_RECPLAYPAUSE_ITEM;
	OSD_IMG_INFO tRecPlayOsdImgInfo[2];

	if(UI_RECPLAYLIST_STATE != tUI_State)
		return;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if((UI_RECSKIPBKFWD_ITEM == tUI_RecPlayListItem)  || (UI_RECFILE_PAUSE == tUI_RecPlaySts) ||
			   (UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts))
				return;
			tPrevRecListItem = tUI_RecPlayListItem;
			tUI_RecPlayListItem--;
			break;
		case RIGHT_ARROW:
			if(((tUI_RecPlayListItem + 1) > UI_RECADOSRCSEL_ITEM) || (UI_RECFILE_PAUSE == tUI_RecPlaySts) ||
			   (UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts))
				return;
			tPrevRecListItem = tUI_RecPlayListItem;
			tUI_RecPlayListItem++;
			break;
		case ENTER_ARROW:
			if(UI_RECPLAYPAUSE_ITEM == tUI_RecPlayListItem)
			{
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
			    if(tUI_RecPlayAct.tSimFld == KNL_SIM_FLD)
			    {
			        UI_CamNum_t tRecCamNum;
                    APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecCamNum);
                    UI_TXPlayEvent(tRecCamNum,UI_TXPLY_JMPPAUSE,NULL,NULL,NULL);
			        return;
			    }
#endif
				if(UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts)
				{
					if(UI_RECPLAYPAUSE_ITEM == tUI_RecPlayListItem)
					{
						uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
                        UI_PlayRecordFile(KNL_REAL_FLD,pUI_RecFilesInfo.uwRecFileSelIdx,&pUI_RecFoldersInfo,&pUI_RecFilesInfo);
					}
					return;
				}
				if(ubPLY_GetPauseStatus() == PLY_PAUSE_OFF)
				{
                    if(ubPLY_Pause(PLY_PAUSE_ON) == 0)
                    	return;
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PLAYHL_ICON, 1, &tRecPlayOsdImgInfo[0]);
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
					tUI_RecPlaySts = UI_RECFILE_PAUSE;
				}
                else if(ubPLY_GetPauseStatus() == PLY_PAUSE_ON)
				{
                    if(ubPLY_Pause(PLY_PAUSE_OFF) == 0)
                        return;
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PAUSEHL_ICON, 1, &tRecPlayOsdImgInfo[0]);
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
					tUI_RecPlaySts = UI_RECFILE_PLAY;
				}
			}
            else if(UI_RECSKIPBKFWD_ITEM == tUI_RecPlayListItem)
			{
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)			
			    if(tUI_RecPlayAct.tSimFld == KNL_SIM_FLD)
			    {
			        UI_CamNum_t tRecCamNum;
                    APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecCamNum);
                    UI_TXPlayEvent(tRecCamNum,UI_TXPLY_JMPBW,NULL,NULL,NULL);
			    }
			    else
#endif
				    ubPLY_Jump(PLY_JUMP_BWD);
			}
            else if(UI_RECSKIPFRFWD_ITEM == tUI_RecPlayListItem)
			{
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    			if(tUI_RecPlayAct.tSimFld == KNL_SIM_FLD)
			    {
			        UI_CamNum_t tRecCamNum;
                    APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecCamNum);
                    UI_TXPlayEvent(tRecCamNum,UI_TXPLY_JMPFW,NULL,NULL,NULL);
			    }
			    else
#endif
				    ubPLY_Jump(PLY_JUMP_FWD);
			}
			else if(UI_RECADOSRCSEL_ITEM == tUI_RecPlayListItem)
			{
				if(KNL_DISP_SINGLE != tUI_RecPlayAct.ubPlayMode)
					UI_DrawRecordPlayAudioSource();
			}
			return;
		case EXIT_ARROW:
			tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
            if( tUI_RecPlayAct.tSimFld == KNL_SIM_FLD )
            {
                UI_CamNum_t tRecCamNum;
                APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecCamNum);
                UI_TXPlayEvent(tRecCamNum,UI_TXPLY_STOPCAM,NULL,NULL,NULL);
            }
#endif            
			KNL_VideoPlayStop();
			tRecPlayOsdImgInfo[0].uwHSize  = 100;
			tRecPlayOsdImgInfo[0].uwVSize  = uwOSD_GetVSize();
			tRecPlayOsdImgInfo[0].uwXStart = 620;
			tRecPlayOsdImgInfo[0].uwYStart = 0;
			OSD_EraserImg1(&tRecPlayOsdImgInfo[0]);
			tUI_RecPlaySts      = UI_RECFILE_PLAY;
			tUI_RecPlayListItem = UI_RECPLAYPAUSE_ITEM;
			uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
#if APP_FS_FILE_LIST_STYLE
            if(KNL_ThmShowInfo.ubEnFlg==0)
            {
                OSD_Weight(OSD_WEIGHT_8DIV8);
                tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
                tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
                tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
                tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_UPDATE);
            }
            else if(KNL_ThmShowInfo.ubEnFlg==1)
            {
                KNL_ROLE CamRole = KNL_STA1;
                VDO_RestartThumbnail(KNL_DISP_SINGLE, &CamRole);
            }
#else	
			OSD_Weight(OSD_WEIGHT_8DIV8);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
			tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
			tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_UPDATE);
#endif
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
            if (tUI_RecPlayAct.tSimFld == KNL_SIM_FLD)
            {
                KNL_RevertDisplayMode();
                if(tKNL_GetRecordFunc() != KNL_RECORDFUNC_LOOP && tKNL_GetRecordFunc() != KNL_RECORDFUNC_MANU)
                {
                    KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
        			tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
                    tUI_PlayAct.pRecordStsNtyCb = NULL;
        			tKNL_ExecRecordFunc(tUI_PlayAct);
                }
                UI_ReDrawTXRecordFileMenu();
            }
			else
#endif            
			    UI_DrawRecordFileMenu();
			return;
		case DOWN_ARROW:
			if(UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PLAYHL_ICON, 1, &tRecPlayOsdImgInfo[0]);
				if(UI_RECPLAYPAUSE_ITEM != tUI_RecPlayListItem)
				{
					tPrevRecListItem = tUI_RecPlayListItem;
					tUI_RecPlayListItem = UI_RECPLAYPAUSE_ITEM;
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
					break;
				}
				else
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
			}
		default:
			return;
	}
	UI_DrawHLandNormalIcon(uwRecPlayListImgIdx[tPrevRecListItem], (uwRecPlayListImgIdx[tUI_RecPlayListItem] + UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_RecordPlayAdoSrcSelection(UI_ArrowKey_t tArrowKey)
{
	UI_CamNum_t tPreRecAdoCamNum = tUI_RecPlayAct.tAdoPlayCam;
	uint16_t uwXOffset = 0, uwYOffset = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?150:0;
	uint16_t uwYItemOffset = 150;
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubCamNum;

	tPreRecAdoCamNum = tUI_RecPlayAct.tAdoPlayCam;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(CAM1 == tUI_RecPlayAct.tAdoPlayCam)
				return;
			for(ubCamNum = tUI_RecPlayAct.tAdoPlayCam; ubCamNum > CAM1; ubCamNum--)
			{
				if(NO_CAM != tUI_RecPlayAct.tVdoPlayCam[ubCamNum - 1])
				{
					tUI_RecPlayAct.tAdoPlayCam = (UI_CamNum_t)(ubCamNum - 1);
					break;
				}
				if((ubCamNum - 1) == CAM1)
					return;
			}
			break;
		case RIGHT_ARROW:
			if((tUI_RecPlayAct.tAdoPlayCam + 1) >= tUI_PuSetting.ubTotalBuNum)
				return;
			for(ubCamNum = (tUI_RecPlayAct.tAdoPlayCam + 1); ubCamNum < tUI_PuSetting.ubTotalBuNum; ubCamNum++)
			{
				if(NO_CAM != tUI_RecPlayAct.tVdoPlayCam[ubCamNum])
				{
					tUI_RecPlayAct.tAdoPlayCam = (UI_CamNum_t)ubCamNum;
					break;
				}
			}
			if(ubCamNum == tUI_PuSetting.ubTotalBuNum)
				return;
			break;
		case ENTER_ARROW:
			ubPLY_AdoChannelSet(tUI_RecPlayAct.tAdoPlayCam);
		case EXIT_ARROW:
			tOsdImgInfo.uwXStart  = 100;
			tOsdImgInfo.uwYStart  = 100;
			tOsdImgInfo.uwHSize   = 255;
			tOsdImgInfo.uwVSize   = 800;
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = UI_RECPLAYLIST_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tPreRecAdoCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tUI_RecPlayAct.tAdoPlayCam*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_SearchRecordFile(uint16_t uwRecFolderIndex)
{
	uint16_t uwLdState = UI_SEARCH_RECFILES;

	uwRecFolderIndex = uwRecFolderIndex;
	UI_SetLdDispStatus(UI_OSDLDDISP_ON);
	osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
	osDelay(200);
	pUI_RecFilesInfo.uwTotalRecFileNum = uwKNL_GetSortingFiles(uwRecFolderIndex, SORT_BY_TIME_DESCENDING, &pUI_RecFilesInfo.tRecFilesInfo[0]);
	UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
	osDelay(300);
}
//------------------------------------------------------------------------------
void UI_ListRecFileInfo(UI_RecFilesInfo_t *FileInfo,uint16_t uwStartFileIdx, uint16_t uwEndFileIdx, OSD_UPDATE_TYP tUpdateMode)
{
	UI_RecImgColor_t tRecCamColor[] = {[CAM1] = UI_RECIMG_COLOR1,
									   [CAM2] = UI_RECIMG_COLOR2,
									   [CAM3] = UI_RECIMG_COLOR3,
									   [CAM4] = UI_RECIMG_COLOR4,}, tRecColor;
	UI_CamNum_t tRecSelCamNum;
	KNL_ROLE tRecRoleNum;
#if !APP_FS_FILE_LIST_STYLE
	OSD_IMG_INFO tAsteriskOsdImgInfo;
#endif
	uint16_t uwIdx;
	uint8_t ubRecGrpLNum, ubRecGrpRNum, ubGrpFlag = FALSE;
	int iGrpRIdx = 1;
    char cRecFileName[32];
#if !APP_FS_FILE_LIST_STYLE
    char cRecFileCreated[20];
#endif

	for(uwIdx = uwStartFileIdx; uwIdx < uwEndFileIdx; uwIdx++)
	{
		ubRecGrpLNum = FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.uwGroupIdx;
		iGrpRIdx     = uwIdx + ((uwIdx)?-1:1);
		ubRecGrpRNum = FileInfo->tRecFilesInfo[iGrpRIdx].HidnFileInfo.uwGroupIdx;
		ubGrpFlag	 = (ubRecGrpLNum == ubRecGrpRNum)?TRUE:FALSE;
		if((uwIdx) && (FALSE == ubGrpFlag))
		{
			ubRecGrpRNum = FileInfo->tRecFilesInfo[uwIdx + 1].HidnFileInfo.uwGroupIdx;
			ubGrpFlag	 = (ubRecGrpLNum == ubRecGrpRNum)?TRUE:FALSE;
		}
		tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.SrcNum);
		if(KNL_NONE == tRecRoleNum)
		{
			printd(DBG_ErrorLvl, "Get kernel role info Err!\n");
			tRecRoleNum = KNL_STA1;
		}
		APP_KNLRoleMap2CamNum(tRecRoleNum, tRecSelCamNum);
		tRecColor = (TRUE == ubGrpFlag)?UI_RECIMG_COLOR5:tRecCamColor[tRecSelCamNum];
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
        if( pUI_RecFoldersInfo.uwRecFolderSelIdx < ubKNL_GetTXFldNum())
        {
            if(FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.SrcNum == 0)
            {
                APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecSelCamNum);
                tRecColor = tRecCamColor[tRecSelCamNum];
            }
            else
                tRecColor = UI_RECIMG_COLOR5;
        }
#endif
		UI_SetRecImgColor(tRecColor);
		memset(cRecFileName, 0, sizeof(cRecFileName));
		snprintf(cRecFileName, sizeof(cRecFileName), "%s.%s", FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chName, FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chExt);
		cRecFileName[(FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.FileName.ubLen + 4)] = 0;
#if APP_FS_FILE_LIST_STYLE
		if( memcmp(pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chExt, "MP4", 3)==0 )
		{
			KNL_SRC tVDO_KNLSrcNum;
			uint8_t ubDataPathSetupFlg = 0;
			
			KNL_ThmShowInfo.uwHSize_Now = pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.SubHidnInfo.uwRes_HSize;
			KNL_ThmShowInfo.uwVSize_Now = pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.SubHidnInfo.uwRes_VSize;
			
			if(KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg==0)
			{
				KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg = 1;
				ubDataPathSetupFlg = 1;
			}
			else if(KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg==1)
			{
				if(!( KNL_ThmShowInfo.uwHSize_Now==KNL_ThmShowInfo.uwHSize_Pre && 
					  KNL_ThmShowInfo.uwVSize_Now==KNL_ThmShowInfo.uwVSize_Pre ))
				{
					ubDataPathSetupFlg = 1;
				}
			}
			
			if(ubDataPathSetupFlg)
			{
				VDO_Stop();
				KNL_VdoPathReset();
				tVDO_KNLSrcNum = VDO_GetSourceNumber(KNL_MAIN_PATH, KNL_STA1);
				KNL_SetVdoResolution(tVDO_KNLSrcNum, KNL_ThmShowInfo.uwHSize_Now, KNL_ThmShowInfo.uwVSize_Now);
				VDO_DataPathSetup(KNL_STA1, VDO_MAIN_SRC);
				KNL_ImageDecodeSetup(tVDO_KNLSrcNum);
				KNL_VdoStart(tVDO_KNLSrcNum);
				
				KNL_ThmShowInfo.uwHSize_Pre = KNL_ThmShowInfo.uwHSize_Now;
				KNL_ThmShowInfo.uwVSize_Pre = KNL_ThmShowInfo.uwVSize_Now;
			}
		}
        KNL_ShowingThm(uwIdx%REC_FILE_LIST_MAXNUM, &pUI_RecFilesInfo.tRecFilesInfo[uwIdx]);
#else
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, 325, 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5), tUI_RecOsdImgInfo, OSD_QUEUE, cRecFileName);
#if (APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SF)
		ubGrpFlag = FALSE;
#endif
		if(TRUE == ubGrpFlag)
		{
			char cListGrp[9];

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_C5_ASTERISK, 1, &tAsteriskOsdImgInfo);
			tAsteriskOsdImgInfo.uwYStart = 280;	//! uwLCD_GetLcdVoSize() - 980 - 20
			tAsteriskOsdImgInfo.uwXStart = 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5);
			tOSD_Img2(&tAsteriskOsdImgInfo, OSD_QUEUE);
			snprintf(cListGrp, sizeof(cListGrp), "Grp%d", FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.uwGroupIdx);
			OSD_ImagePrintf(OSD_IMG_ROTATION_90, 1000, 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5), tUI_RecOsdImgInfo, OSD_QUEUE, cListGrp);
		}
		memset(cRecFileCreated, 0, sizeof(cRecFileCreated));
		snprintf(cRecFileCreated, sizeof(cRecFileCreated), "%02d-%02d-%02d %02d:%02d", FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.uwYear, FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMonth, FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubDay,
																					   FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubHour, FileInfo->tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMin);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, 643, 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5), tUI_RecOsdImgInfo, ((uwIdx + 1) == uwEndFileIdx)?tUpdateMode:OSD_QUEUE, cRecFileCreated);
#endif		
	}
}
//------------------------------------------------------------------------------
#if APP_FS_FILE_LIST_STYLE
void UI_RecordFileThmShowXYSetting(uint8_t ubOpIdx, OSD_IMG_INFO *Info)
{
	Info->uwXStart += (ubOpIdx/3)*150;
	Info->uwYStart -= ((ubOpIdx%3)*(pUI_RecFilesInfo.tRecFilesInfo[ubOpIdx].HidnFileInfo.FileName.ubLen+4)*25);
}
void UI_DrawFileTime(uint16_t uwIdx)
{
	char cRecFileCreated[60];
	memset(cRecFileCreated, 0, sizeof(cRecFileCreated));
	UI_SetRecImgColor(UI_RECIMG_COLOR5);
	snprintf(cRecFileCreated, sizeof(cRecFileCreated), "%s.%s  %02d.%02d.%04d  %02d:%02d:%02d", 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chName,
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chExt,
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMonth, 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubDay,
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.uwYear, 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubHour, 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMin, 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubSec);
	OSD_ImagePrintf(OSD_IMG_ROTATION_90, 20, 650, tUI_RecOsdImgInfo, OSD_UPDATE, cRecFileCreated);
}
void UI_DrawFileIdx(uint16_t uwIdx)
{
	char cRecFileNum[10];
	memset(cRecFileNum, 0, sizeof(cRecFileNum));
	UI_SetRecImgColor(UI_RECIMG_COLOR5);
	snprintf(cRecFileNum, sizeof(cRecFileNum), "%04d-%04d",uwIdx+1, pUI_RecFilesInfo.uwTotalRecFileNum);
	OSD_ImagePrintf(OSD_IMG_ROTATION_90, 780, 650, tUI_RecOsdImgInfo, OSD_UPDATE, cRecFileNum);
}
#endif
//------------------------------------------------------------------------------
void UI_DrawRecordFileMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo[9];
	uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
#if !APP_FS_FILE_LIST_STYLE
	uint8_t ubIdx;
#endif
    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_CAM1, 9, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
	UI_SearchRecordFile(pUI_RecFoldersInfo.uwRecFolderSelIdx);
	osDelay(20);
#if !APP_FS_FILE_LIST_STYLE
	for(ubIdx = 0; ubIdx < 8; ubIdx++)
		tOSD_Img2(&tOsdImgInfo[ubIdx], ((!pUI_RecFilesInfo.uwTotalRecFileNum) && (ubIdx == 7))?OSD_UPDATE:OSD_QUEUE);
#endif
	if(pUI_RecFilesInfo.uwTotalRecFileNum)
	{
		uwFileStartIdx = (pUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
		uwFileEndIdx   = pUI_RecFilesInfo.uwTotalRecFileNum - uwFileStartIdx;
		uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?pUI_RecFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
#if APP_FS_FILE_LIST_STYLE
		tOsdImgInfo[8].uwXStart = 143;
		tOsdImgInfo[8].uwYStart = 1020;
		UI_RecordFileThmShowXYSetting(pUI_RecFilesInfo.uwRecFileSelIdx%REC_FILE_LIST_MAXNUM, &tOsdImgInfo[8]);
		tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
        
		KNL_ThmShowInfo.ubLcdDispAddrKeepFlg = 0;
		KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg = 0;
        
        KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
        
		KNL_FillWhiteOnLcd(720, 1280);
#else	
		tOsdImgInfo[8].uwXStart += ((pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
		tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
#endif
        KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
        UI_ListRecFileInfo(&pUI_RecFilesInfo,uwFileStartIdx, uwFileEndIdx, OSD_UPDATE);
#if APP_FS_FILE_LIST_STYLE
		UI_DrawFileTime(pUI_RecFilesInfo.uwRecFileSelIdx);
		UI_DrawFileIdx(pUI_RecFilesInfo.uwRecFileSelIdx);
#endif
	}
#if APP_FS_FILE_LIST_STYLE
	else
	{
	    KNL_ThmShowInfo.ubLcdDispAddrKeepFlg = 0;
		KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg = 0;
		KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
		KNL_LcdDisplaySetting();
		osDelay(20);
		if(KNL_ThmShowInfo.ubLcdDispAddrKeepFlg==0)
		{
			KNL_ThmShowInfo.ubLcdDispAddrKeepFlg = 1;
			KNL_ThmShowInfo.ulLcdDispCurAddr = ulKNL_GetLcdDispAddr(KNL_SRC_1_MAIN);
			KNL_ThmShowInfo.ulLcdDispKeepAddr = KNL_ThmShowInfo.ulLcdDispCurAddr;
		}
		else if(KNL_ThmShowInfo.ubLcdDispAddrKeepFlg==1)
		{
			KNL_ThmShowInfo.ulLcdDispCurAddr = KNL_ThmShowInfo.ulLcdDispKeepAddr;
		}
		KNL_FillWhiteOnLcd(720, 1280);
		osDelay(20);
		KNL_ActiveLcdDispBuf(KNL_SRC_1_MAIN);
		osDelay(20);
	}
#endif
	tUI_State = UI_RECFILES_SEL_STATE;
}
//------------------------------------------------------------------------------
void UI_RecordFileSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tFileSelOsdImgInfo[3];
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t ubUI_RecFileIdx, ubUI_PrevRecFileIdx = 0;
	static uint8_t ubUI_RecFileDelFlag = FALSE;
	uint8_t ubUI_RecFileDelRdy = FALSE;
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
	if((!pUI_RecFilesInfo.uwTotalRecFileNum) && (EXIT_ARROW != tArrowKey))
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_FILESELECT, 3, &tFileSelOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
#if APP_FS_FILE_LIST_STYLE
    tFileSelOsdImgInfo[0].uwXStart = tFileSelOsdImgInfo[1].uwXStart = tFileSelOsdImgInfo[2].uwXStart = 143;
    tFileSelOsdImgInfo[0].uwYStart = tFileSelOsdImgInfo[1].uwYStart = tFileSelOsdImgInfo[2].uwYStart = 1020;
#endif
	switch(tArrowKey)
	{
		case RIGHT_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			if(FALSE == ubUI_RecFileDelFlag)
				return;
#if APP_FS_FILE_LIST_STYLE
            UI_RecordFileThmShowXYSetting(pUI_RecFilesInfo.uwRecFileSelIdx%REC_FILE_LIST_MAXNUM, &tFileSelOsdImgInfo[0]);
#else				
			tFileSelOsdImgInfo[0].uwXStart += ((pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
#endif						
			tOSD_Img2(&tFileSelOsdImgInfo[0], OSD_UPDATE);
			ubUI_RecFileDelFlag = FALSE;
			return;
		case LEFT_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			if(FALSE == ubUI_RecFileDelFlag)
			{
#if APP_FS_FILE_LIST_STYLE
                UI_RecordFileThmShowXYSetting(pUI_RecFilesInfo.uwRecFileSelIdx%REC_FILE_LIST_MAXNUM, &tFileSelOsdImgInfo[2]);
#else
				tFileSelOsdImgInfo[2].uwXStart += ((pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
#endif	
				tOSD_Img2(&tFileSelOsdImgInfo[2], OSD_UPDATE);
			}
			ubUI_RecFileDelFlag = TRUE;
			return;
		case UP_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			ubUI_RecFileDelFlag = FALSE;
			if(!pUI_RecFilesInfo.uwRecFileSelIdx)
				return;
			ubUI_PrevRecFileIdx = (pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx		= (--pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case DOWN_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			ubUI_RecFileDelFlag = FALSE;
			if((pUI_RecFilesInfo.uwRecFileSelIdx + 1) >= pUI_RecFilesInfo.uwTotalRecFileNum)
				return;
			ubUI_PrevRecFileIdx = (pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx		= (++pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case ENTER_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			if(TRUE == ubUI_RecFileDelFlag)
			{
				//! Delete Record File              
				ubUI_RecFileDelRdy = UI_DeleteRecordFile(pUI_RecFilesInfo.uwRecFileSelIdx);
			}
			else
			{
                UI_PlayRecordFile(KNL_REAL_FLD,pUI_RecFilesInfo.uwRecFileSelIdx,&pUI_RecFoldersInfo,&pUI_RecFilesInfo);
				return;
			}
		case EXIT_ARROW:
		    KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
#if !APP_FS_FILE_LIST_STYLE
			tOsdImgInfo.uwHSize  = 610;
			tOsdImgInfo.uwVSize  = 955;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
#else
			tOsdImgInfo.uwHSize  = 720;
			tOsdImgInfo.uwVSize  = 1280;
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
#endif
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_RecFileDelFlag = FALSE;
			if(TRUE == ubUI_RecFileDelRdy)
			{
                if(pUI_RecFilesInfo.uwRecFileSelIdx)
                {
                    ubUI_PrevRecFileIdx = (pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
                    ubUI_RecFileIdx		= (--pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
                }
				UI_DrawRecordFileMenu();
				return;
			}
#if APP_FS_FILE_LIST_STYLE
            KNL_ThmShowInfo.ubEnFlg = 0;
#endif
            KNL_RevertDisplayMode();
            if(tKNL_GetRecordFunc() != KNL_RECORDFUNC_LOOP && tKNL_GetRecordFunc() != KNL_RECORDFUNC_MANU)
            {
    			tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
                tUI_PlayAct.pRecordStsNtyCb = NULL;
    			tKNL_ExecRecordFunc(tUI_PlayAct);
            }
			pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
			pUI_RecFilesInfo.uwRecFileSelIdx = 0;
#if APP_FS_FILE_LIST_STYLE
			KNL_ThmShowInfo.ubInFldListFlg = 1;
#endif			
			UI_DrawDCIMFolderMenu();
			tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;

            if((UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode ))
    		    UI_VideoRecordingExec(UI_REC_START);			
			return;
		default:
			return;
	}
	if((((!(pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM)) && (pUI_RecFilesInfo.uwRecFileSelIdx) && (ubUI_PrevRecFileIdx == REC_FILE_LIST_MAXNUM - 1))) ||
	   ((ubUI_PrevRecFileIdx == 0) && (ubUI_RecFileIdx == REC_FILE_LIST_MAXNUM - 1)))
	{
		uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
#if !APP_FS_FILE_LIST_STYLE
		tOsdImgInfo.uwHSize  = 415;
		tOsdImgInfo.uwVSize  = 955;
		tOsdImgInfo.uwXStart = 195;
		tOsdImgInfo.uwYStart = 50;
		OSD_EraserImg2(&tOsdImgInfo);
#else
		tOsdImgInfo.uwHSize  = 720;
		tOsdImgInfo.uwVSize  = 1280;
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 0;
		OSD_EraserImg2(&tOsdImgInfo);
		
		KNL_FillWhiteOnLcd(720, 1280);
#endif
		if(DOWN_ARROW == tArrowKey)
		{
			uwFileStartIdx = pUI_RecFilesInfo.uwRecFileSelIdx;
			uwFileEndIdx   = pUI_RecFilesInfo.uwTotalRecFileNum - pUI_RecFilesInfo.uwRecFileSelIdx;
			uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?pUI_RecFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
		}
		else
		{
			uwFileStartIdx = (pUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
			uwFileEndIdx   = pUI_RecFilesInfo.uwRecFileSelIdx + 1;
		}
		UI_ListRecFileInfo(&pUI_RecFilesInfo,uwFileStartIdx, uwFileEndIdx, OSD_QUEUE);
	}
#if APP_FS_FILE_LIST_STYLE
	UI_RecordFileThmShowXYSetting(ubUI_RecFileIdx, &tFileSelOsdImgInfo[0]);	
	UI_RecordFileThmShowXYSetting(ubUI_PrevRecFileIdx, &tFileSelOsdImgInfo[1]);
#else			
	tFileSelOsdImgInfo[0].uwXStart += (ubUI_RecFileIdx * 40);
	tFileSelOsdImgInfo[1].uwXStart += (ubUI_PrevRecFileIdx * 40);
#endif		
	tOSD_Img2(&tFileSelOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tFileSelOsdImgInfo[1], OSD_UPDATE);
#if APP_FS_FILE_LIST_STYLE
	UI_DrawFileTime(pUI_RecFilesInfo.uwRecFileSelIdx);
	UI_DrawFileIdx(pUI_RecFilesInfo.uwRecFileSelIdx);
#endif
}
//------------------------------------------------------------------------------
void UI_RecordSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Record sub menu page
		//! Check Select Camera number
		UI_DrawSubMenuPage(RECORD_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[RECORD_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[RECITEM_MAX] = {OSD2IMG_REC1MODENOR_ITEM, OSD2IMG_REC1TIMENOR_ITEM, OSD2IMG_SDFNOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
			OSD_IMG_INFO tOsdImgInfo[2];

			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKNOR_ICON, 2, &tOsdImgInfo[0]);
			if(ubSubMenuItemPreIdx != SDCARD_ITEM)
			{
				tOsdImgInfo[0].uwXStart += (ubSubMenuItemPreIdx * 0x40);
				tOSD_Img2(&tOsdImgInfo[0], (ubSubMenuItemIdx == SDCARD_ITEM)?OSD_UPDATE:OSD_QUEUE);
			}
			if(ubSubMenuItemIdx == SDCARD_ITEM)
				break;
			tOsdImgInfo[1].uwXStart += (ubSubMenuItemIdx * 0x40);
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
			break;
		}
		case DRAW_MENUPAGE:
		{
			OSD_IMG_INFO tOsdImgInfo;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
			uint16_t uwOptMarkOffset = 0;
			uint8_t i;

			//! Draw sub sub menu page
			if(ubSubMenuItemIdx == RECMODE_ITEM)
			{
				uint16_t uwSubSubMenuItemOsdImg[REC_RECMODE_MAX] = {OSD2IMG_REC1LOOPNOR_ICON, OSD2IMG_REC1MANUNOR_ICON, OSD2IMG_REC1TRIGNOR_ICON, OSD2IMG_REC1OFFNOR_ICON};
				uint8_t i;
				uwSubSubMenuItemOsdImg[tUI_PuSetting.RecInfo.tREC_Mode] += UI_ICON_HIGHLIGHT;
				tOsdImgInfo.uwHSize  = 335;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 130;
				tOsdImgInfo.uwYStart = 300;
				OSD_EraserImg2(&tOsdImgInfo);
				for(i = 0; i < REC_RECMODE_MAX; i++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			else if(ubSubMenuItemIdx == RECTIME_ITEM)
			{
				uint16_t uwSubSubMenuItemOsdImg[RECTIME_MAX] = {OSD2IMG_REC1RT1MINNOR_ICON, OSD2IMG_REC1RT2MINNOR_ICON,
																OSD2IMG_REC1RT5MINNOR_ICON};
				uwSubSubMenuItemOsdImg[tUI_PuSetting.RecInfo.tREC_Time] += UI_ICON_HIGHLIGHT;
				tOsdImgInfo.uwHSize  = 335;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 198;
				tOsdImgInfo.uwYStart = 300;
				OSD_EraserImg2(&tOsdImgInfo);
				for(i = 0; i < RECTIME_MAX; i++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				uwOptMarkOffset = 0x40;
			}
			else if(ubSubMenuItemIdx == SDCARD_ITEM)
			{
				OSD_IMG_INFO tSdfOsdImgInfo[6];

				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFBG_ICON, 6, &tSdfOsdImgInfo);
				tOSD_Img2(&tSdfOsdImgInfo[0], OSD_QUEUE);
				tOSD_Img2(&tSdfOsdImgInfo[2], OSD_QUEUE);
				tOSD_Img2(&tSdfOsdImgInfo[3], OSD_UPDATE);
				tUI_SdCardSts = UI_SD_CFM;
				tUI_State = UI_SDCARDFMT_STATE;
				break;				
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKHL_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwOptMarkOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_State = UI_SUBSUBMENU_STATE;
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RecordUpdateSubSubMenuItemIndex(UI_RecSubSubMenuItem_t *ptSubSubMenuItem, UI_RecordSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	switch(*tSubMenuItem)
	{
		case RECMODE_ITEM:
			ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_PuSetting.RecInfo.tREC_Mode;
			break;
		case RECTIME_ITEM:
			ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_PuSetting.RecInfo.tREC_Time;
			break;
		default:
			return;
	}
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_RecordDrawSubSubMenuItem(UI_RecSubSubMenuItem_t *ptSubSubMenuItem, UI_RecordSubMenuItemList_t *tSubMenuItem)
{
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(*tSubMenuItem)
	{
		case RECMODE_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[REC_RECMODE_MAX] = {OSD2IMG_REC1LOOPNOR_ICON, OSD2IMG_REC1MANUNOR_ICON, OSD2IMG_REC1TRIGNOR_ICON, OSD2IMG_REC1OFFNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		case RECTIME_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[RECTIME_MAX] = {OSD2IMG_REC1RT1MINNOR_ICON, OSD2IMG_REC1RT2MINNOR_ICON,
															OSD2IMG_REC1RT5MINNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RecordSDFSubSubMenu(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSdfSelImgIdx[2] = {OSD2IMG_SDFNONOR_ICON, OSD2IMG_SDFYESNOR_ICON};
	static uint8_t ubUI_SdfSel = FALSE;
	uint8_t ubPrevSdfSel;
	FS_FMT_STATUS tSDF_Ret = FORMAT_FAIL;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(TRUE == ubUI_SdfSel)
				return;
			ubPrevSdfSel = ubUI_SdfSel;
			ubUI_SdfSel = TRUE;
			break;
		case RIGHT_ARROW:
			if(FALSE == ubUI_SdfSel)
				return;
			ubPrevSdfSel = ubUI_SdfSel;
			ubUI_SdfSel = FALSE;
			break;
		case ENTER_ARROW:
			if(TRUE == ubUI_SdfSel)
            {
				if(UI_REC_START == tUI_RecPlayAct.tRecAct)
					UI_VideoRecordingExec(UI_REC_STOP);
            }
		case EXIT_ARROW:
			tOsdImgInfo.uwHSize  = 310;
			tOsdImgInfo.uwVSize  = 600;
			tOsdImgInfo.uwXStart = 310;
			tOsdImgInfo.uwYStart = 400;
			OSD_EraserImg2(&tOsdImgInfo);
			if((ENTER_ARROW == tArrowKey) && (TRUE == ubUI_SdfSel))
			{
				OSD_IMG_INFO tSdfOsdImgInfo[3];
				UI_FuncExecMsg_t tWorkAct;
				uint8_t ubSdFmtRet;

				tWorkAct.uwFunc = UI_SDCARDFMT_ACT;
				osMessagePut(osUI_FuncsExecQue, &tWorkAct, 0);
				osMessageGet(osUI_FuncsFinExecQue, &ubSdFmtRet, osWaitForever);
				tSDF_Ret = (rUI_SUCCESS == ubSdFmtRet)?FORMAT_OK:FORMAT_FAIL;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFRET_BG, 3, &tSdfOsdImgInfo);
				tOSD_Img2(&tSdfOsdImgInfo[0], OSD_QUEUE);
				tOSD_Img2(&tSdfOsdImgInfo[1+tSDF_Ret], OSD_UPDATE);
				osDelay(1000);
				OSD_EraserImg2(&tOsdImgInfo);
			}
			ubUI_SdfSel = FALSE;
			tUI_State 	= UI_SUBMENU_STATE;
			return;
		default:
			return;
	}
	UI_DrawHLandNormalIcon(uwSdfSelImgIdx[ubPrevSdfSel], (uwSdfSelImgIdx[ubUI_SdfSel]+UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_RecordSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_RecSubSubMenuItem_t tRecSubSubMenuItem = 
	{
		{
		   { 0, REC_RECMODE_MAX },{ 0, RECTIME_MAX }, { 0, 2 }
		},
	};
	static uint8_t ubUI_RecStsUpdateFlag = FALSE;
	UI_RecordSubMenuItemList_t tSubMenuItem = (UI_RecordSubMenuItemList_t)tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;

	if(FALSE == ubUI_RecStsUpdateFlag)
		UI_RecordUpdateSubSubMenuItemIndex(&tRecSubSubMenuItem, &tSubMenuItem, &ubUI_RecStsUpdateFlag);
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tRecSubSubMenuItem.tRecordS[tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_RecordDrawSubSubMenuItem(&tRecSubSubMenuItem, &tSubMenuItem);
			break;
		case EXECUTE_MENUFUNC:
		{
			uint8_t *pUI_Mode = (tSubMenuItem == RECMODE_ITEM)?(uint8_t *)&tUI_PuSetting.RecInfo.tREC_Mode:
					            (tSubMenuItem == RECTIME_ITEM)?(uint8_t *)&tUI_PuSetting.RecInfo.tREC_Time:NULL;
			if(pUI_Mode)
			{
				uint8_t ubItemIdx = tRecSubSubMenuItem.tRecordS[tSubMenuItem].tSubMenuInfo.ubItemIdx;

				if((*pUI_Mode != ubItemIdx) && (SCAN_VIEW != tCamViewSel.tCamViewType))
				{
					if((tSubMenuItem == RECMODE_ITEM) && (REC_TRIGGER != ubItemIdx))
					{
					#if APP_SD_FUNC_ENABLE
						#if !APP_REC_FUNC_ENABLE
						if((UI_VDORECLOOP_MODE == ubItemIdx) || (UI_VDORECMANU_MODE == ubItemIdx))
							break;
						#endif
                        if(UI_REC_START == tUI_RecPlayAct.tRecAct)
							UI_VideoRecordingExec(UI_REC_STOP);
						*pUI_Mode = ubItemIdx;
						tUI_PuSetting.tVdoMode = (REC_OFF == *pUI_Mode)?UI_PHOTOCAP_MODE:UI_RECORDING_MODE;
						UI_UpdateDevStatusInfo();
						if((UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode))
						{
							UI_VideoRecordingExec(UI_REC_START);
							if((tUI_PuSetting.ubVdoRecStsCnt) || (UI_SDCARDFMT_STATE == tUI_State))
								break;
						}
                        else
                            KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
					#else
						break;
					#endif
					}
					else if((tSubMenuItem == RECTIME_ITEM) && (tUI_RecPlayAct.tRecAct != UI_REC_START))
					{
					#if (APP_SD_FUNC_ENABLE && APP_REC_FUNC_ENABLE)
						UI_RecordingAct_t tCurRecAct;

						tCurRecAct = tUI_RecPlayAct.tRecAct;
						if(UI_REC_START == tUI_RecPlayAct.tRecAct)
							UI_VideoRecordingExec(UI_REC_STOP);
						*pUI_Mode = ubItemIdx;
						if(tUI_PuSetting.RecInfo.tREC_Time == RECTIME_1MIN)
							REC_TimeSet(0,60);
						else if(tUI_PuSetting.RecInfo.tREC_Time == RECTIME_3MIN)
							REC_TimeSet(0,180);
						else if(tUI_PuSetting.RecInfo.tREC_Time == RECTIME_5MIN)
							REC_TimeSet(0,300);
						UI_UpdateDevStatusInfo();
						if(UI_REC_START == tCurRecAct)
							UI_VideoRecordingExec(UI_REC_START);
					#else
						break;
					#endif
					}
				}
			}
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo[2];

			tOsdImgInfo[0].uwHSize  = 335;
			tOsdImgInfo[0].uwVSize  = 250;
			tOsdImgInfo[0].uwXStart = 110;
			tOsdImgInfo[0].uwYStart = 300;
			OSD_EraserImg2(&tOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_REC1LOOPWR_ICON+tUI_PuSetting.RecInfo.tREC_Mode), 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_REC1RT1MINWR_ICON+tUI_PuSetting.RecInfo.tREC_Time), 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKNOR_ICON, 2, &tOsdImgInfo[0]);
			tOsdImgInfo[0].uwXStart += (((tSubMenuItem == RECMODE_ITEM)?1:0) * 0x40);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOsdImgInfo[1].uwXStart += (((tSubMenuItem == RECMODE_ITEM)?0:1) * 0x40);
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
			ubUI_RecStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PhotoSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(tUI_State == UI_MAINMENU_STATE)
	{
		ubUI_RecSubMenuFlag = TRUE;
		UI_DrawDCIMFolderMenu();
		return;
	}
	UI_DCIMFolderSelection(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_PhotoUpdateSubSubMenuItemIndex(UI_PhotoSubSubMenuItem_t *ptSubSubMenuItem, UI_PhotoSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	UI_CamNum_t tCamNum = tCamSelect.tCamNum4PhotoSub;
	switch(*tSubMenuItem)
	{
		case PHOTOFUNC_ITEM:
			ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CamStatus[tCamNum].tPHOTO_Func;
			break;
		case PHOTORES_ITEM:
			ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CamStatus[tCamNum].tPHOTO_Resolution;
			break;
		default:
			return;
	}
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_PhotoDrawSubSubMenuItem(UI_PhotoSubSubMenuItem_t *ptSubSubMenuItem, UI_PhotoSubMenuItemList_t *tSubMenuItem)
{
	UI_CamNum_t tCamNum = tCamSelect.tCamNum4PhotoSub;
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(*tSubMenuItem)
	{
		case PHOTOFUNC_ITEM:
			break;
		case PHOTORES_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[PHOTORES_MAX] = {OSD2IMG_PRES3MNOR_ICON, OSD2IMG_PRES5MNOR_ICON, OSD2IMG_PRES12MNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PhotoSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_PhotoSubSubMenuItem_t tPhotoSubSubMenuItem = 
	{
		{
		   {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }},
		   {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }},
		   {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }},
		   {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }}
		},
	};
	static uint8_t ubUI_PhotoStsUpdateFlag = FALSE;
	UI_PhotoSubMenuItemList_t tSubMenuItem = (UI_PhotoSubMenuItemList_t)tUI_SubMenuItem[PHOTO_ITEM].tSubMenuInfo.ubItemIdx;
	UI_CamNum_t tCamNum 				   = tCamSelect.tCamNum4PhotoSub;
	UI_MenuAct_t tMenuAct;

	if(FALSE == ubUI_PhotoStsUpdateFlag)
		UI_PhotoUpdateSubSubMenuItemIndex(&tPhotoSubSubMenuItem, &tSubMenuItem, &ubUI_PhotoStsUpdateFlag);
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tPhotoSubSubMenuItem.tPhotoS[tCamNum][tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_PhotoDrawSubSubMenuItem(&tPhotoSubSubMenuItem, &tSubMenuItem);
			break;
		case EXECUTE_MENUFUNC:
		{
			UI_PUReqCmd_t   tPhotoCmd;
			UI_PhotoFunction_t tPHOTO_Func = (tUI_CamStatus[tCamNum].tPHOTO_Func == PHOTOFUNC_OFF)?PHOTOFUNC_ON:PHOTOFUNC_OFF;

			if((tSubMenuItem != PHOTOFUNC_ITEM) && (tSubMenuItem != PHOTORES_ITEM))
				break;
			tPhotoCmd.tDS_CamNum = tCamNum;
			tPhotoCmd.ubCmd[UI_TWC_TYPE]	 = UI_SETTING;
			tPhotoCmd.ubCmd[UI_SETTING_ITEM] = (tSubMenuItem == PHOTOFUNC_ITEM)?PHOTOFUNC_ITEM:PHOTORES_ITEM;
			tPhotoCmd.ubCmd[UI_SETTING_DATA] = (tSubMenuItem == PHOTOFUNC_ITEM)?tPHOTO_Func:
												tPhotoSubSubMenuItem.tPhotoS[tCamNum][tSubMenuItem].tSubMenuInfo.ubItemIdx;
			tPhotoCmd.ubCmd_Len  = 3;
			if(UI_SendRequestToBU(osThreadGetId(), &tPhotoCmd) == rUI_SUCCESS)
			{
				if(tSubMenuItem == PHOTOFUNC_ITEM)
					tUI_CamStatus[tCamNum].tPHOTO_Func = tPHOTO_Func;
				if(tSubMenuItem == PHOTORES_ITEM)
					tUI_CamStatus[tCamNum].tPHOTO_Resolution = (UI_PhotoResolution_t)tPhotoSubSubMenuItem.tPhotoS[tCamNum][tSubMenuItem].tSubMenuInfo.ubItemIdx;
			}
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo;

			if(tSubMenuItem == PHOTOFUNC_ITEM)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_PHOTOOFFWR_ICON+tUI_CamStatus[tCamNum].tPHOTO_Func), 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			if(tSubMenuItem == PHOTORES_ITEM)
			{
				tOsdImgInfo.uwHSize  = 335;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 285;
				tOsdImgInfo.uwYStart = 312;
				OSD_EraserImg2(&tOsdImgInfo);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_PRES3MWR_ICON+tUI_CamStatus[tCamNum].tPHOTO_Resolution), 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 0x40;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
			}
			ubUI_PhotoStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PlaybackSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(UI_MAINMENU_STATE == tUI_State)
	{
		UI_DrawSubMenuPage(PLAYBACK_ITEM);
		return;
	}

#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    if( ubUI_RecSubMenuTXFldFlag )
        UI_DCIMTXFolderSelection(tArrowKey);
    else
#endif
	UI_DCIMFolderSelection(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_PowerSaveSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(UI_MAINMENU_STATE == tUI_State)
	{
		UI_DrawSubMenuPage(PS_ITEM);
		return;
	}
}
//------------------------------------------------------------------------------
void UI_DrawSysDateTime(UI_CalendarItem_t tShowItem, UI_IconType_t tIconType, RTC_Calendar_t *ptSysCalendar)
{
	static OSD_IMG_INFO tNumOsdImgArrayInfo[22] = {0};
	static uint8_t ubUI_RdNumOsdImgFlag = FALSE;
	uint8_t ubDateOffset[2] = {0}, i;

	if(FALSE == ubUI_RdNumOsdImgFlag)
	{
		uint16_t uwSubSubMenuItemOsdImg[22] = {OSD2IMG_NUMBER0NOR_ICON, OSD2IMG_NUMBER0HL_ICON,
											   OSD2IMG_NUMBER1NOR_ICON, OSD2IMG_NUMBER1HL_ICON,
											   OSD2IMG_NUMBER2NOR_ICON, OSD2IMG_NUMBER2HL_ICON,
											   OSD2IMG_NUMBER3NOR_ICON, OSD2IMG_NUMBER3HL_ICON,
											   OSD2IMG_NUMBER4NOR_ICON, OSD2IMG_NUMBER4HL_ICON,
											   OSD2IMG_NUMBER5NOR_ICON, OSD2IMG_NUMBER5HL_ICON,
											   OSD2IMG_NUMBER6NOR_ICON, OSD2IMG_NUMBER6HL_ICON,
											   OSD2IMG_NUMBER7NOR_ICON, OSD2IMG_NUMBER7HL_ICON,
											   OSD2IMG_NUMBER8NOR_ICON, OSD2IMG_NUMBER8HL_ICON,
											   OSD2IMG_NUMBER9NOR_ICON, OSD2IMG_NUMBER9HL_ICON,
											   OSD2IMG_COLONNOR_ICON, OSD2IMG_DIVISIONNOR_ICON};
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[21], 1, &tNumOsdImgArrayInfo[21]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[0], 21, &tNumOsdImgArrayInfo[0]);
		ubUI_RdNumOsdImgFlag = TRUE;
	}
	switch(tShowItem)
	{
		case ALLCALE_ITEM:
		case YEAR_ITEM:
			tNumOsdImgArrayInfo[4+tIconType].uwXStart = 170;
			tNumOsdImgArrayInfo[4+tIconType].uwYStart = 390;
			tOSD_Img2(&tNumOsdImgArrayInfo[4+tIconType], OSD_QUEUE);
			tNumOsdImgArrayInfo[tIconType].uwXStart = 170;
			tNumOsdImgArrayInfo[tIconType].uwYStart = 360;
			tOSD_Img2(&tNumOsdImgArrayInfo[tIconType], OSD_QUEUE);
			ubDateOffset[0] = (ptSysCalendar->uwYear - 2000) / 10;
			ubDateOffset[1] = (ptSysCalendar->uwYear - 2000) - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 330 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case MONTH_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubMonth / 10;
			ubDateOffset[1] = ptSysCalendar->ubMonth - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 248 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case DATE_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubDate / 10;
			ubDateOffset[1] = ptSysCalendar->ubDate - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 166 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case HOUR_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubHour / 10;
			ubDateOffset[1] = ptSysCalendar->ubHour - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 390 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case MIN_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubMin / 10;
			ubDateOffset[1] = ptSysCalendar->ubMin - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 306 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case SEC_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubSec / 10;
			ubDateOffset[1] = ptSysCalendar->ubSec - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 222 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			break;
		default:
			return;
	}
	if(tShowItem == ALLCALE_ITEM)
	{
		tNumOsdImgArrayInfo[21].uwXStart = 170;
		tNumOsdImgArrayInfo[21].uwYStart = 278;
		tOSD_Img2(&tNumOsdImgArrayInfo[21], OSD_QUEUE);
		
		tNumOsdImgArrayInfo[21].uwXStart = 170;
		tNumOsdImgArrayInfo[21].uwYStart = 196;
		tOSD_Img2(&tNumOsdImgArrayInfo[21], OSD_QUEUE);
		
		tNumOsdImgArrayInfo[20].uwXStart = 220;
		tNumOsdImgArrayInfo[20].uwYStart = 336;
		tOSD_Img2(&tNumOsdImgArrayInfo[20], OSD_QUEUE);
		
		tNumOsdImgArrayInfo[20].uwXStart = 220;
		tNumOsdImgArrayInfo[20].uwYStart = 252;
		tOSD_Img2(&tNumOsdImgArrayInfo[20], OSD_UPDATE);
	}
}
//------------------------------------------------------------------------------
int UI_DrawSettingSubSubMenuPage(UI_SettingSubMenuItemList_t *tSubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	int iRet = -1;
	uint8_t i;

	tOsdImgInfo.uwHSize  = 335;
	tOsdImgInfo.uwVSize  = 250;
	tOsdImgInfo.uwYStart = 312;
	switch(*tSubMenuItem)
	{
		case DATETIME_ITEM:
		{
			tOsdImgInfo.uwXStart = 150;
			OSD_EraserImg2(&tOsdImgInfo);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DT_SUBMENUICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			UI_DrawSysDateTime(ALLCALE_ITEM, UI_ICON_NORMAL, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
			break;
		}
		case AECSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[AECFUNC_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
			uwSubSubMenuItemOsdImg[tUI_PuSetting.ubAEC_Mode] += UI_ICON_HIGHLIGHT;
			tOsdImgInfo.uwXStart = 215;
			OSD_EraserImg2(&tOsdImgInfo);
			for(i = 0; i < AECFUNC_MAX; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0;
			break;
			}
		case CCASET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[CCAMODE_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
			uwSubSubMenuItemOsdImg[tUI_PuSetting.ubCCA_Mode] += UI_ICON_HIGHLIGHT;
			tOsdImgInfo.uwXStart = 275;
			OSD_EraserImg2(&tOsdImgInfo);
			for(i = 0; i < CCAMODE_MAX; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 65;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0x40;
			break;
		}
		case DEFUSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_DEFUNOTHL_ICON, OSD2IMG_DEFUEXECNOR_ICON};

			for(i = 0; i < 2; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0x100;
			break;
		}
		case SWUSBDMODE_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_FWUDISABLEHL_ICON, OSD2IMG_FWUENNOR_ICON};

			for(i = 0; i < 2; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0x140;
			break;
		}
		default:
			break;
	}
	return iRet;
}
//------------------------------------------------------------------------------
void UI_SettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Cameras sub menu page
		UI_DrawSubMenuPage(SETTING_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[SETTING_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[SETTINGITEM_MAX] = {OSD2IMG_DTNOR_ITEM, OSD2IMG_AECNOR_ITEM, OSD2IMG_CCANOR_ITEM,
															 OSD2IMG_STORAGENOR_ITEM, OSD2IMG_LANGUAGENOR_ITEM, OSD2IMG_DEFUNOR_ITEM, OSD2IMG_SWUSBDMODENOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
			if(ubSubMenuItemIdx == STORAGESTS_ITEM)
			{
				ubSubMenuItemIdx = (ubSubMenuItemPreIdx > ubSubMenuItemIdx)?--tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx:
																		    ++tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
				tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx = (ubSubMenuItemPreIdx > ubSubMenuItemIdx)?--tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx:
																												   ++tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx;
			}
			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
			break;
		}
		case DRAW_MENUPAGE:
		{
			//! Draw sub sub menu page
			UI_SettingSubMenuItemList_t tSubMenuItemIdx = (UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
			OSD_IMG_INFO tOsdImgInfo;
			int iOptMarkOffset = UI_DrawSettingSubSubMenuPage(&tSubMenuItemIdx);

			if(iOptMarkOffset != -1)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKHL_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart = (0xDA + iOptMarkOffset);
				tOsdImgInfo.uwYStart = 0x205;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			tUI_State = UI_SUBSUBMENU_STATE;
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SettingUpdateSubSubMenuItemIndex(UI_SettingSubSubMenuItem_t *ptSubSubMenuItem, UI_SettingSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	switch(*tSubMenuItem)
	{
		case AECSET_ITEM:
			ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_PuSetting.ubAEC_Mode;
			break;
		case CCASET_ITEM:
			ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_PuSetting.ubCCA_Mode;
			break;
		default:
			return;
	}
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_SettingDrawSubSubMenuItem(UI_SettingSubSubMenuItem_t *ptSubSubMenuItem, UI_SettingSubMenuItemList_t *tSubMenuItem)
{
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(*tSubMenuItem)
	{
		case AECSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[AECFUNC_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		case CCASET_ITEM:
		{
			OSD_IMG_INFO tOsdImgInfo;
			uint16_t uwSubSubMenuItemOsdImg[AECFUNC_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};

			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += 65;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += 65;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		}
		case DEFUSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_DEFUNOTNOR_ICON, OSD2IMG_DEFUEXECNOR_ICON};

			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		case SWUSBDMODE_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_FWUDISABLENOR_ICON, OSD2IMG_FWUENNOR_ICON};

			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_SettingSubSubMenuItem_t tSettingSubSubMenuItem = 
	{
		{
		   { 0, 0 			},
		   { 0, AECFUNC_MAX },
		   { 0, CCAMODE_MAX },
		   { 0, 0 			},
		   { 0, 0 			},
		   { 0, 2 			},
		   { 0, 2 			},
		},
	};
	static uint8_t ubUI_SettingStsUpdateFlag = FALSE;
	UI_SettingSubMenuItemList_t tSubMenuItem = (UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;
	OSD_IMG_INFO tOsdImgInfo;

	if(FALSE == ubUI_SettingStsUpdateFlag)
		UI_SettingUpdateSubSubMenuItemIndex(&tSettingSubSubMenuItem, &tSubMenuItem, &ubUI_SettingStsUpdateFlag);
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tSettingSubSubMenuItem.tSettingS[tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_SettingDrawSubSubMenuItem(&tSettingSubSubMenuItem, &tSubMenuItem);
			break;
		case EXECUTE_MENUFUNC:
			if(tSubMenuItem == SWUSBDMODE_ITEM)
			{
				if(tSettingSubSubMenuItem.tSettingS[SWUSBDMODE_ITEM].tSubMenuInfo.ubItemIdx)
					FWU_Enable();
				else
					FWU_Disable();
				tSettingSubSubMenuItem.tSettingS[SWUSBDMODE_ITEM].tSubMenuInfo.ubItemIdx = 0;
			}
		case EXIT_MENUFUNC:
			tUI_State = UI_SUBMENU_STATE;
			if((tSubMenuItem == DEFUSET_ITEM) || (tSubMenuItem == SWUSBDMODE_ITEM))
			{
				tOsdImgInfo.uwHSize  = 200;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 450 + ((tSubMenuItem == SWUSBDMODE_ITEM)?60:0);
				tOsdImgInfo.uwYStart = 312;
				OSD_EraserImg2(&tOsdImgInfo);
				break;
			}
			
			if(tSubMenuItem == AECSET_ITEM)
            {
				
				tUI_PuSetting.ubAEC_Mode = tSettingSubSubMenuItem.tSettingS[AECSET_ITEM].tSubMenuInfo.ubItemIdx;	
				
				if(tUI_PuSetting.ubAEC_Mode == 0)
				{
					ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
				}
				else
				{
					ADO_Noise_Process_Type(NOISE_AEC,AEC_NR_16kHZ);
				}
                                //AEC
				
            }
		
			tOsdImgInfo.uwHSize  = 350;
			tOsdImgInfo.uwVSize  = 480;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 100;
			OSD_EraserImg2(&tOsdImgInfo);
			//! Change camera setting
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_AECOFFWR_ICON+tUI_PuSetting.ubAEC_Mode), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_AECOFFWR_ICON+tUI_PuSetting.ubCCA_Mode), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += 65;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_STORAGATBU_ICON+tUI_PuSetting.ubSTORAGE_Mode, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart -= 0xE;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOsdImgInfo.uwXStart += (0xE + 0x32);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubUI_SettingStsUpdateFlag = FALSE;
			break;
		default:
			if((tSubMenuItem == DATETIME_ITEM) && (tArrowKey == RIGHT_ARROW))
			{
				UI_DrawSysDateTime(YEAR_ITEM, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
				tUI_State = UI_SUBSUBSUBMENU_STATE;
			}
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SettingSysDateTimeSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_SettingSubSubSubItem_t tSysDtSubSubSubItem = {YEAR_ITEM, YEAR_ITEM};
	static RTC_Calendar_t tUI_SetSysCalendar;
	static uint8_t ubUI_SetSysDtFlag = FALSE;
	uint16_t *pDT_YearNum;
	uint8_t *pDT_Num[ALLCALE_ITEM];
	uint16_t uwDT_MaxYearNum = 2098, uwDT_MinYearNum = 2015;
	uint8_t ubDT_MaxNum[ALLCALE_ITEM] = {0, 12, 31, 23, 59, 59};
	uint8_t ubDT_MinNum[ALLCALE_ITEM] = {0,  1,  1,  0,  0,  0};
	uint8_t ubDT_UpdateItem = tSysDtSubSubSubItem.ubItemIdx;

	if(FALSE == ubUI_SetSysDtFlag)
	{
		memcpy((RTC_Calendar_t *)&tUI_SetSysCalendar, (RTC_Calendar_t *)&tUI_PuSetting.tSysCalendar, sizeof(RTC_Calendar_t));
		ubUI_SetSysDtFlag	= TRUE;
	}
	pDT_YearNum 		= (uint16_t *)&tUI_SetSysCalendar.uwYear;
	pDT_Num[MONTH_ITEM] = (uint8_t *)&tUI_SetSysCalendar.ubMonth;
	pDT_Num[DATE_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubDate;
	pDT_Num[HOUR_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubHour;
	pDT_Num[MIN_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubMin;
	pDT_Num[SEC_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubSec;
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubDT_UpdateItem == YEAR_ITEM)
			{
				if((NULL == pDT_YearNum) || (*pDT_YearNum >= uwDT_MaxYearNum))
					return;
				(*pDT_YearNum)++;
			}
			else
			{
				if((NULL == pDT_Num[ubDT_UpdateItem]) || (*pDT_Num[ubDT_UpdateItem] >= ubDT_MaxNum[ubDT_UpdateItem]))
					return;
				(*pDT_Num[ubDT_UpdateItem])++;
			}
			UI_DrawSysDateTime((UI_CalendarItem_t)ubDT_UpdateItem, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_SetSysCalendar));
			return;
		case DOWN_ARROW:
			if(ubDT_UpdateItem == YEAR_ITEM)
			{
				if((NULL == pDT_YearNum) || (*pDT_YearNum == uwDT_MinYearNum))
					return;
				(*pDT_YearNum)--;
			}
			else
			{
				if((NULL == pDT_Num[ubDT_UpdateItem]) || (*pDT_Num[ubDT_UpdateItem] == ubDT_MinNum[ubDT_UpdateItem]))
					return;
				(*pDT_Num[ubDT_UpdateItem])--;
			}
			UI_DrawSysDateTime((UI_CalendarItem_t)ubDT_UpdateItem, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_SetSysCalendar));
			return;
		case ENTER_ARROW:
		case LEFT_ARROW:
			if((tSysDtSubSubSubItem.ubItemIdx == YEAR_ITEM) || (ENTER_ARROW == tArrowKey))
			{
				if(memcmp((RTC_Calendar_t *)&tUI_SetSysCalendar, (RTC_Calendar_t *)&tUI_PuSetting.tSysCalendar, sizeof(RTC_Calendar_t)))
				{
					memcpy((RTC_Calendar_t *)&tUI_PuSetting.tSysCalendar, (RTC_Calendar_t *)&tUI_SetSysCalendar, sizeof(RTC_Calendar_t));
					if(iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar)) != RTC_OK)
						printd(DBG_ErrorLvl, "Calendar base setting fail !\n");
					else
						RTC_SetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
					UI_UpdateDevStatusInfo();
				}
				UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemIdx, UI_ICON_NORMAL, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
#if APP_TIMESTAMP_FUNC_ENABLE
                KNL_SyncTimeStamp2TX();
#endif                
				tSysDtSubSubSubItem.ubItemPreIdx = YEAR_ITEM;
				tSysDtSubSubSubItem.ubItemIdx    = YEAR_ITEM;
				ubUI_SetSysDtFlag = FALSE;
				tUI_State = UI_SUBSUBMENU_STATE;
				return;
			}
			tSysDtSubSubSubItem.ubItemPreIdx = tSysDtSubSubSubItem.ubItemIdx;
			tSysDtSubSubSubItem.ubItemIdx--;
			break;
		case RIGHT_ARROW:
			if(tSysDtSubSubSubItem.ubItemIdx == SEC_ITEM)
				return;
			tSysDtSubSubSubItem.ubItemPreIdx = tSysDtSubSubSubItem.ubItemIdx;
			tSysDtSubSubSubItem.ubItemIdx++;
			break;
		default:
			return;
	}
	UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemIdx,    UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_SetSysCalendar));
	UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemPreIdx, UI_ICON_NORMAL,    (RTC_Calendar_t *)(&tUI_SetSysCalendar));
}
//------------------------------------------------------------------------------
void UI_ResetSubMenuInfo(void)
{
	uint8_t i;

	for(i = 0; i < MENUITEM_MAX; i++)
		memset(&tUI_SubMenuItem[i].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
}
//------------------------------------------------------------------------------
void UI_ResetSubSubMenuInfo(void)
{
}
//------------------------------------------------------------------------------
UI_CamNum_t UI_ChangeSelectCamNum4UiMenu(UI_CamNum_t *tCurrentCamNum, UI_ArrowKey_t *ptArrowKey)
{
	UI_CamNum_t tChangeCamNum = NO_CAM;
	UI_CamNum_t tCamNum 	  = (UI_CamNum_t)*tCurrentCamNum;
	UI_CamNum_t tMaxCamNum 	  = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?CAM_4T:
	                            (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?CAM_2T:CAM_4T;

	if(tCamNum > ((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?QUAD_TYPE_ITEM:tMaxCamNum))
		return NO_CAM;
	switch(*ptArrowKey)
	{
		case LEFT_ARROW:
			if(tCamNum == CAM1)
				return NO_CAM;
			if(DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
			{
				if((tUI_PuSetting.ubPairedBuNum > 1) && (tCamNum > DUAL_TYPE_ITEM))
					return (UI_CamNum_t)(tCamNum-1);
				else if((tUI_PuSetting.ubPairedBuNum == 1) && (tCamNum > DUAL_TYPE_ITEM))
					return (UI_CamNum_t)(tCamNum-2);
				if(tCamNum == DUAL_TYPE_ITEM)
					tCamNum = CAM_4T;
			}
			for(;tCamNum > CAM1; tCamNum--)
			{
				//if((tUI_CamStatus[tCamNum-1].ulCAM_ID != INVALID_ID) &&
				//   (tUI_CamStatus[tCamNum-1].tCamConnSts == CAM_ONLINE))
				if((tUI_CamStatus[tCamNum-1].ulCAM_ID != INVALID_ID))
				{
					tChangeCamNum = (UI_CamNum_t)(tCamNum-1);
					break;
				}
			}
			break;
		case RIGHT_ARROW:
			if((tCamNum+1) >= tMaxCamNum)
			{
				if(DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
					return ((tCamNum+1) > QUAD_TYPE_ITEM)?NO_CAM:(tUI_PuSetting.ubPairedBuNum > 1)?((UI_CamNum_t)(tCamNum+1)):(UI_CamNum_t)QUAD_TYPE_ITEM;
				else
					return tMaxCamNum;
			}
			tChangeCamNum = tMaxCamNum;
			for(;tCamNum < (tMaxCamNum - 1); tCamNum++)
			{
				//if((tUI_CamStatus[tCamNum+1].ulCAM_ID != INVALID_ID) &&
				//   (tUI_CamStatus[tCamNum+1].tCamConnSts == CAM_ONLINE))
				if((tUI_CamStatus[tCamNum+1].ulCAM_ID != INVALID_ID))
				{
					tChangeCamNum = (UI_CamNum_t)(tCamNum+1);
					break;
				}
			}
			if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (tChangeCamNum == tMaxCamNum))
				tChangeCamNum = (tUI_PuSetting.ubPairedBuNum >= 1)?(UI_CamNum_t)DUAL_TYPE_ITEM:(UI_CamNum_t)QUAD_TYPE_ITEM;
			break;
		default:
			break;
	}
	return tChangeCamNum;
}
//------------------------------------------------------------------------------
void UI_DrawHLandNormalIcon(uint16_t uwNormalOsdImgIdx, uint16_t uwHighLigthOsdImgIdx)
{
	OSD_IMG_INFO tOsdImgInfo;

	//! Draw normal item
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNormalOsdImgIdx, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	//! Draw highlight item
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwHighLigthOsdImgIdx, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
UI_MenuAct_t UI_KeyEventMap2SubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubMenu)
{
	uint8_t ubCurrentSubMenuItemIdx = ptSubMenu->tSubMenuInfo.ubItemIdx;
	uint8_t ubNextSubMenuItemIdx = 0;

	switch(*ptArrowKey)
	{
		case UP_ARROW:
			if(ubCurrentSubMenuItemIdx <= ptSubMenu->ubFirstItem)
				return NOT_ACTION;
			ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx - 1;
			break;
		case DOWN_ARROW:
			if((ubCurrentSubMenuItemIdx+1) >= ptSubMenu->ubItemCount)
				return NOT_ACTION;
			ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx + 1;
			break;
		case RIGHT_ARROW:
		case ENTER_ARROW:
			return DRAW_MENUPAGE;
		case EXIT_ARROW:
			return EXIT_MENUFUNC;
		default:
			return NOT_ACTION;
	}
	ptSubMenu->tSubMenuInfo.ubItemPreIdx = ubCurrentSubMenuItemIdx;
	ptSubMenu->tSubMenuInfo.ubItemIdx    = ubNextSubMenuItemIdx;
	return DRAW_HIGHLIGHT_MENUICON;
}
//------------------------------------------------------------------------------
UI_MenuAct_t UI_KeyEventMap2SubSubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubSubMenuItem)
{
	uint8_t ubCurrentSubSubMenuItemIdx = 0;
	uint8_t ubNextSubSubMenuItemIdx = 0;

	ubCurrentSubSubMenuItemIdx = ptSubSubMenuItem->tSubMenuInfo.ubItemIdx;
	switch(*ptArrowKey)
	{
		case UP_ARROW:
			if(ubCurrentSubSubMenuItemIdx <= ptSubSubMenuItem->ubFirstItem)
				return NOT_ACTION;
			ubNextSubSubMenuItemIdx = ubCurrentSubSubMenuItemIdx - 1;
			break;
		case DOWN_ARROW:
			if((ubCurrentSubSubMenuItemIdx+1) >= ptSubSubMenuItem->ubItemCount)
				return NOT_ACTION;
			ubNextSubSubMenuItemIdx = ubCurrentSubSubMenuItemIdx + 1;
			break;
		case ENTER_ARROW:
			return EXECUTE_MENUFUNC;
		case LEFT_ARROW:
			return EXIT_MENUFUNC;
		default:
			return NOT_ACTION;
	}
	ptSubSubMenuItem->tSubMenuInfo.ubItemPreIdx = ubCurrentSubSubMenuItemIdx;
	ptSubSubMenuItem->tSubMenuInfo.ubItemIdx    = ubNextSubSubMenuItemIdx;
	return DRAW_HIGHLIGHT_MENUICON;
}
//------------------------------------------------------------------------------
void UI_UpdateBriLvlIcon(void)
{
	if((UI_SHOWSTSICON_STATE != tUI_State) ||
	   (!tUI_PuSetting.BriLvL.ubBL_UpdateCnt))
		return;
	if(!(--tUI_PuSetting.BriLvL.ubBL_UpdateCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		UI_ClearBuConnectStatusFlag();
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 0;
		tOsdImgInfo.uwHSize  = 190;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
		tUI_State = UI_DISPLAY_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateVolLvlIcon(void)
{
	if((UI_SHOWSTSICON_STATE != tUI_State) ||
	   (!tUI_PuSetting.VolLvL.ubVOL_UpdateCnt))
		return;
	if(!(--tUI_PuSetting.VolLvL.ubVOL_UpdateCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		UI_ClearBuConnectStatusFlag();
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 0;
		tOsdImgInfo.uwHSize  = 190;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
		tUI_State = UI_DISPLAY_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateOsdImg4MultiView(UI_CamViewType_t tView_Type, OSD_RESULT(*pOsdImgFuncPtr)(OSD_IMG_INFO *, OSD_UPDATE_TYP), OSD_IMG_INFO *pOsdImgInfo)
{
	uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
	uint16_t uwOriXStart = pOsdImgInfo->uwXStart, uwOriYStart = pOsdImgInfo->uwYStart;

	if(TRIPLE_3COL_VIEW == tView_Type)
	{
		pOsdImgInfo->uwYStart -= (ulLcd_VSize/3);
	}
	else
	{
		if(TRIPLE_2L1R_VIEW == tView_Type)
		{
			pOsdImgInfo->uwXStart += (ulLcd_HSize/2);
			pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
			pOsdImgInfo->uwXStart = uwOriXStart;
		}
		pOsdImgInfo->uwYStart -= (ulLcd_VSize/2);
		if(QUAD_VIEW == tView_Type)
		{
			pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
			pOsdImgInfo->uwXStart += (ulLcd_HSize/2);
			pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
			pOsdImgInfo->uwYStart = uwOriYStart;
		}
		else if(TRIPLE_1L2R_VIEW == tView_Type)
		{
			pOsdImgInfo->uwXStart += (ulLcd_HSize/2);
			pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
			pOsdImgInfo->uwXStart = uwOriXStart;
		}
	}
	pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
	pOsdImgInfo->uwXStart = uwOriXStart;
	pOsdImgInfo->uwYStart = uwOriYStart;
	pOsdImgFuncPtr(pOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawPUStatusIcon(void)
{
	static OSD_IMG_INFO tPuOsdImgInfo[2];
	static uint8_t ubUI_RdPuOsdImgFlag = FALSE;
	OSD_IMG_INFO tOsdImgInfo;
	uint32_t ulQualMask_XStart = uwLCD_GetLcdHoSize()/2;
	uint32_t ulQualMask_YStart = uwLCD_GetLcdVoSize()/2;
	uint32_t ulBatLvL_YStart = 0x48D;
	uint16_t uwPuOriXStart[2] = {0, 0}, uwPuOriYStart[2] = {0, 0};
	UI_CamViewType_t tUI_CamViewType = tCamViewSel.tCamViewType;

	if(UI_SET_BUECOMODE_STATE == tUI_State)
		return;
	if(FALSE == ubUI_RdPuOsdImgFlag)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PU_STSICON, 1, &tPuOsdImgInfo[0]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BATLVL4_STSICON, 1, &tPuOsdImgInfo[1]);
		ubUI_RdPuOsdImgFlag = TRUE;
	}
	switch(tUI_CamViewType)
	{
		case SINGLE_VIEW:
		case SCAN_VIEW:
			tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
			if(TRUE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
			{
				tOSD_Img2(&tPuOsdImgInfo[1], OSD_UPDATE);
				break;
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_HDSTATUSMASK, 1, &tOsdImgInfo);
			tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
			tOSD_Img2(&tPuOsdImgInfo[0], OSD_QUEUE);
			tOSD_Img2(&tPuOsdImgInfo[1], OSD_UPDATE);
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			break;
		case DUAL_VIEW:
		case QUAD_VIEW:
		case TRIPLE_2L1R_VIEW:
		case TRIPLE_1L2R_VIEW:
			if(TRUE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
			{
				tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
				UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[1]);
				break;
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_QUALSTATUSMASK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwYStart = ulQualMask_YStart;
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img1, &tOsdImgInfo);
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[0]);
			tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[1]);
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			break;
		case TRIPLE_2T1B_VIEW:
		case TRIPLE_1T2B_VIEW:
			uwPuOriXStart[0] = tPuOsdImgInfo[0].uwXStart;
			uwPuOriYStart[0] = tPuOsdImgInfo[0].uwYStart;
			uwPuOriXStart[1] = tPuOsdImgInfo[1].uwXStart;
			uwPuOriYStart[1] = tPuOsdImgInfo[1].uwYStart;
			if(TRUE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
			{			
				tPuOsdImgInfo[1].uwXStart += (TRIPLE_2T1B_VIEW == tUI_CamViewType)?ulQualMask_XStart:0;
				tOSD_Img2(&tPuOsdImgInfo[1], OSD_QUEUE);
				tPuOsdImgInfo[1].uwXStart = uwPuOriXStart[1];
				tPuOsdImgInfo[1].uwXStart += (TRIPLE_2T1B_VIEW == tUI_CamViewType)?0:ulQualMask_XStart;
				tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
				UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[1]);
				tPuOsdImgInfo[1].uwXStart = uwPuOriXStart[1];
				tPuOsdImgInfo[1].uwYStart = uwPuOriYStart[1];
				osDelay(200);
				break;
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_HDSTATUSMASK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += (TRIPLE_2T1B_VIEW == tUI_CamViewType)?(uwLCD_GetLcdHoSize()/2):0;
			tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_QUALSTATUSMASK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += (TRIPLE_2T1B_VIEW == tUI_CamViewType)?0:ulQualMask_XStart;
			tOsdImgInfo.uwYStart  = ulQualMask_YStart;			
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img1, &tOsdImgInfo);
			//! 2T/2B PU
			tPuOsdImgInfo[0].uwXStart += (TRIPLE_2T1B_VIEW == tUI_CamViewType)?0:ulQualMask_XStart;
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[0]);
			tPuOsdImgInfo[0].uwXStart = uwPuOriXStart[0];
			//! 1T/1B PU/Battery Lvl
			tPuOsdImgInfo[0].uwXStart += (TRIPLE_2T1B_VIEW == tUI_CamViewType)?ulQualMask_XStart:0;
			tOSD_Img2(&tPuOsdImgInfo[0], OSD_QUEUE);
			tPuOsdImgInfo[0].uwXStart = uwPuOriXStart[0];
			tPuOsdImgInfo[1].uwXStart += (TRIPLE_2T1B_VIEW == tUI_CamViewType)?ulQualMask_XStart:0;
			tOSD_Img2(&tPuOsdImgInfo[1], OSD_QUEUE);
			tPuOsdImgInfo[1].uwXStart = uwPuOriXStart[1];			
			//! 2T/2B Battery Lvl
			tPuOsdImgInfo[1].uwXStart += (TRIPLE_2T1B_VIEW == tUI_CamViewType)?0:ulQualMask_XStart;
			tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[1]);
			tPuOsdImgInfo[1].uwXStart = uwPuOriXStart[1];
			tPuOsdImgInfo[1].uwYStart = uwPuOriYStart[1];
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			break;
		case TRIPLE_3COL_VIEW:
		{
			OSD_IMG_INFO tOsdColImgInfo[2];
			uwPuOriXStart[0] = tPuOsdImgInfo[0].uwXStart;
			uwPuOriYStart[0] = tPuOsdImgInfo[0].uwYStart;
			uwPuOriXStart[1] = tPuOsdImgInfo[1].uwXStart;
			uwPuOriYStart[1] = tPuOsdImgInfo[1].uwYStart;
			if(TRUE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
			{
				tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
				UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[1]);
				tPuOsdImgInfo[1].uwYStart -= ((uwLCD_GetLcdVoSize()/3)*2);
				tOSD_Img2(&tPuOsdImgInfo[1], OSD_UPDATE);
				tPuOsdImgInfo[1].uwYStart = uwPuOriYStart[1];
				break;
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_TRIP3COL1STATUSMASK_ICON, 2, &tOsdColImgInfo[0]);
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img1, &tOsdColImgInfo[0]);
			tOSD_Img1(&tOsdColImgInfo[1], OSD_QUEUE);
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[0]);
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[1]);
			tPuOsdImgInfo[0].uwYStart -= ((uwLCD_GetLcdVoSize()/3)*2);
			tOSD_Img2(&tPuOsdImgInfo[0], OSD_QUEUE);
			tPuOsdImgInfo[0].uwYStart = uwPuOriYStart[0];
			tPuOsdImgInfo[1].uwYStart -= ((uwLCD_GetLcdVoSize()/3)*2);
			tOSD_Img2(&tPuOsdImgInfo[1], OSD_UPDATE);
			tPuOsdImgInfo[1].uwYStart = uwPuOriYStart[1];
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateBuStatusOsdImg(OSD_IMG_INFO *pOsdImgInfo, OSD_UPDATE_TYP tUpdateMode, UI_OsdImgFnType_t tOsdImgFnType, UI_DisplayLocation_t tDispLoc)
{
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint16_t uwXOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
	                          [DISP_LOWER_LEFT] = (ulLcd_HSize/2), [DISP_LOWER_RIGHT] = (ulLcd_HSize/2),
							  [DISP_LEFT] 	    = 0,			   [DISP_RIGHT]		  = 0,
							  [DISP_3T_TOP]     = 0, 		       [DISP_3T_BOTTOM]	  = (ulLcd_HSize/2),
							  [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = 0,
	                          [DISP_3T_3CRIGHT] = 0};
	uint16_t uwYOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
	                          [DISP_LOWER_LEFT] = 0, 			   [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
							  [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT] 	  = (ulLcd_VSize/2),
							  [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = (ulLcd_VSize/3),
	                          [DISP_3T_3CRIGHT] = ((ulLcd_VSize/3)*2)};
	uint16_t uwXStart = 0, uwYStart = 0, uw3TColYOffset = 0;

	uw3TColYOffset = ((DISP_3T_3CLEFT == tDispLoc) || (DISP_3T_3CMID == tDispLoc))?214:(DISP_3T_3CRIGHT == tDispLoc)?212:0;	                 
	uwXStart = pOsdImgInfo->uwXStart;
	uwYStart = pOsdImgInfo->uwYStart;
	pOsdImgInfo->uwXStart += uwXOffset[tDispLoc];
	pOsdImgInfo->uwYStart += uw3TColYOffset;
	pOsdImgInfo->uwYStart -= uwYOffset[tDispLoc];
	if(UI_OsdUpdate == tOsdImgFnType)
		tOSD_Img2(pOsdImgInfo, tUpdateMode);
	else if(UI_OsdErase == tOsdImgFnType)
		OSD_EraserImg2(pOsdImgInfo);
	pOsdImgInfo->uwXStart = uwXStart;
	pOsdImgInfo->uwYStart = uwYStart;
}
//------------------------------------------------------------------------------
void UI_ClearBuConnectStatusFlag(void)
{
	UI_CamNum_t tCamNum;

	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		if(pUI_BuConnectFlag[0])
			*(pUI_BuConnectFlag[0]+tCamNum) = FALSE;
		if(pUI_BuConnectFlag[1])
			*(pUI_BuConnectFlag[1]+tCamNum) = FALSE;
	}
}
//------------------------------------------------------------------------------
UI_DisplayLocation_t tUI_GetTripViewDispLoc(UI_CamNum_t tCamNum)
{
	UI_DisplayLocation_t tDispLoc;

	tDispLoc = tUI_CamStatus[tCamNum].tCamDispLocation;
	switch(tCamViewSel.tCamViewType)
	{
		case TRIPLE_2L1R_VIEW:
			tDispLoc = (tCamNum == tCamViewSel.tCamViewPool[0])?DISP_RIGHT:
					   (tCamNum == tCamViewSel.tCamViewPool[1])?DISP_UPPER_LEFT:
					   (tCamNum == tCamViewSel.tCamViewPool[2])?DISP_LOWER_LEFT:tUI_CamStatus[tCamNum].tCamDispLocation;
			break;
		case TRIPLE_1L2R_VIEW:
			tDispLoc = (tCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:
					   (tCamNum == tCamViewSel.tCamViewPool[1])?DISP_UPPER_RIGHT:
					   (tCamNum == tCamViewSel.tCamViewPool[2])?DISP_LOWER_RIGHT:tUI_CamStatus[tCamNum].tCamDispLocation;
			break;
		case TRIPLE_2T1B_VIEW:
			tDispLoc = (tCamNum == tCamViewSel.tCamViewPool[0])?DISP_UPPER_LEFT:
					   (tCamNum == tCamViewSel.tCamViewPool[1])?DISP_UPPER_RIGHT:
					   (tCamNum == tCamViewSel.tCamViewPool[2])?DISP_3T_BOTTOM:tUI_CamStatus[tCamNum].tCamDispLocation;
			break;
		case TRIPLE_1T2B_VIEW:
			tDispLoc = (tCamNum == tCamViewSel.tCamViewPool[0])?DISP_LOWER_LEFT:
					   (tCamNum == tCamViewSel.tCamViewPool[1])?DISP_LOWER_RIGHT:
					   (tCamNum == tCamViewSel.tCamViewPool[2])?DISP_3T_TOP:tUI_CamStatus[tCamNum].tCamDispLocation;
			break;
		case TRIPLE_3COL_VIEW:
			tDispLoc = (tCamNum == tCamViewSel.tCamViewPool[0])?DISP_3T_3CLEFT:
					   (tCamNum == tCamViewSel.tCamViewPool[1])?DISP_3T_3CMID:
					   (tCamNum == tCamViewSel.tCamViewPool[2])?DISP_3T_3CRIGHT:tUI_CamStatus[tCamNum].tCamDispLocation;
			break;
		default:
			break;
	}
	return tDispLoc;
}
//------------------------------------------------------------------------------
void UI_RedrawBuConnectStatusIcon(UI_CamNum_t tCamNum)
{
	static OSD_IMG_INFO tCamNumOsdImgInfo[CAM_4T];
	static OSD_IMG_INFO tAdoSrcOsdImgInfo, tMarkAdoOsdImgInfo;
	static OSD_IMG_INFO tCamAntLvlOsdImgInfo[6], tCamBatLvlOsdImgInfo[6];
	static uint8_t ubUI_RdBuStsOsdImgFlag 	    = FALSE;
	static uint8_t ubUI_BuOnlineFlag[CAM_4T]    = {FALSE, FALSE, FALSE, FALSE};
	static uint8_t ubUI_BuOfflineFlag[CAM_4T]   = {FALSE, FALSE, FALSE, FALSE};
	static uint8_t ubUI_NoSignalOsdFlag[CAM_4T] = {FALSE, FALSE, FALSE, FALSE};
	uint16_t uwCamNumOsdIdx[CAM_4T] = {OSD2IMG_CAM1_STSICON, OSD2IMG_CAM2_STSICON,
									   OSD2IMG_CAM3_STSICON, OSD2IMG_CAM4_STSICON};
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;
	uint16_t uwAntLvLIdx = ANT_NOSIGNAL, uwBatLvLIdx = BAT_LVL0;

	if(FALSE == ubUI_RdBuStsOsdImgFlag)
	{
		UI_CamNum_t tSelCamNum;

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ADOSRC_STSICON, 1, &tAdoSrcOsdImgInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_STSMASK_STSICON, 1, &tMarkAdoOsdImgInfo);
		for(tSelCamNum = CAM1; tSelCamNum < tUI_PuSetting.ubTotalBuNum; tSelCamNum++)
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCamNumOsdIdx[tSelCamNum], 1, &tCamNumOsdImgInfo[tSelCamNum]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ANTLVL5_STSICON, 6, &tCamAntLvlOsdImgInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BATLVL4_STSICON, 6, &tCamBatLvlOsdImgInfo);
		pUI_BuConnectFlag[0] = &ubUI_BuOnlineFlag[0];
		pUI_BuConnectFlag[1] = &ubUI_BuOfflineFlag[0];
		ubUI_RdBuStsOsdImgFlag = TRUE;
	}
	if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType)))
		tUI_DispLoc = tUI_GetTripViewDispLoc(tCamNum);
	else if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (tCamViewSel.tCamViewType == DUAL_VIEW))
		tUI_DispLoc = (tCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT;
	else
		tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_RIGHT:tUI_CamStatus[tCamNum].tCamDispLocation;
	if((TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag) ||
       (UI_MAINMENU_STATE == tUI_State))
	{
		ubUI_BuOnlineFlag[tCamNum]  = FALSE;
		ubUI_BuOfflineFlag[tCamNum] = FALSE;
		if(UI_MAINMENU_STATE == tUI_State)
			return;
		//! No Signal
		if((DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum) && (TRUE == ubUI_NoSignalOsdFlag[tCamNum]) &&
		   (UI_RECFILES_SEL_STATE != tUI_State))
			UI_RedrawNoSignalOsdIcon(tCamNum, UI_OsdErase);
		return;
	}
	switch(tUI_CamStatus[tCamNum].tCamConnSts)
	{
		case CAM_ONLINE:
			uwAntLvLIdx = tUI_CamStatus[tCamNum].tCamAntLvl;
			uwBatLvLIdx = BAT_LVL4;
			if(TRUE == ubUI_BuOnlineFlag[tCamNum])
				break;
			if(PS_ECO_MODE != tUI_CamStatus[tCamNum].tCamPsMode)
			{
				//! Mask No Signal
				if(TRUE == ubUI_NoSignalOsdFlag[tCamNum])
					UI_RedrawNoSignalOsdIcon(tCamNum, UI_OsdErase);
				//! ANR
				if(tUI_PuSetting.tAdoSrcCamNum == tCamNum)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_NROFF_ICON+tUI_CamStatus[tUI_PuSetting.tAdoSrcCamNum].tCamAnrMode, 1, &tOsdImgInfo);
					UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
				}
			}
			//! Camera Number
			UI_UpdateBuStatusOsdImg(&tCamNumOsdImgInfo[tCamNum], OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			//! Audio Source
			if(tUI_PuSetting.tAdoSrcCamNum == tCamNum)
				UI_UpdateBuStatusOsdImg(&tAdoSrcOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			ubUI_NoSignalOsdFlag[tCamNum] 	= FALSE;
			ubUI_BuOnlineFlag[tCamNum]  	= TRUE;
			ubUI_BuOfflineFlag[tCamNum] 	= FALSE;
			break;
		case CAM_OFFLINE:
			if(TRUE == ubUI_BuOfflineFlag[tCamNum])
				return;
			//! ANR
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_NRMASK_ICON, 1, &tOsdImgInfo);
			UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			//! Camera Number
			UI_UpdateBuStatusOsdImg(&tCamNumOsdImgInfo[tCamNum], OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			//! Mask Audio Source
			if(tUI_PuSetting.tAdoSrcCamNum == tCamNum)
				UI_UpdateBuStatusOsdImg(&tMarkAdoOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			//! No Signal
			if(PS_ECO_MODE != tUI_CamStatus[tCamNum].tCamPsMode)
				UI_RedrawNoSignalOsdIcon(tCamNum, UI_OsdUpdate);
			ubUI_NoSignalOsdFlag[tCamNum] 	= TRUE;
			ubUI_BuOfflineFlag[tCamNum] 	= TRUE;
			ubUI_BuOnlineFlag[tCamNum]  	= FALSE;
			break;
		default:
			break;
	}
	if(TRUE == ubUI_PerDebugEn)
		UI_OsdDisplayFrmErrItem(tUI_DispLoc);
	//! Antenna Level
	UI_UpdateBuStatusOsdImg(&tCamAntLvlOsdImgInfo[uwAntLvLIdx], OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
	//! Battery Level
	UI_UpdateBuStatusOsdImg(&tCamBatLvlOsdImgInfo[uwBatLvLIdx], OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
}
//------------------------------------------------------------------------------
void UI_DrawBUStatusIcon(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_CamNum_t tCamNum, tDrawCamNum;
	UI_CamViewType_t tUI_CamViewType = tCamViewSel.tCamViewType;
	uint8_t ubUI_TotalBuNum = (tCamViewSel.tCamViewType == DUAL_VIEW)?CAM_2T:tUI_PuSetting.ubTotalBuNum;

	switch(tUI_CamViewType)
	{
		case SINGLE_VIEW:
		case SCAN_VIEW:
			if(tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID)
				UI_RedrawBuConnectStatusIcon(tCamViewSel.tCamViewPool[0]);
			if(PS_ECO_MODE == tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamPsMode)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ECOSTS_ICON, 1, &tOsdImgInfo);
				UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, DISP_UPPER_RIGHT);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 180;
				tOsdImgInfo.uwYStart -= 321;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			else
			{
				if((tUI_CamViewType == SCAN_VIEW) && (TRUE == (*(pUI_BuConnectFlag[0]+tCamViewSel.tCamViewPool[0]))))
				{
					tOsdImgInfo.uwXStart = 280;
					tOsdImgInfo.uwYStart = 480;
					tOsdImgInfo.uwHSize  = 160;
					tOsdImgInfo.uwVSize  = 310;
					OSD_EraserImg2(&tOsdImgInfo);
				}
			}
			break;
		case DUAL_VIEW:
		case QUAD_VIEW:
		case TRIPLE_2L1R_VIEW:
		case TRIPLE_1L2R_VIEW:
		case TRIPLE_2T1B_VIEW:
		case TRIPLE_1T2B_VIEW:
		case TRIPLE_3COL_VIEW:
			ubUI_TotalBuNum = (IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType))?3:ubUI_TotalBuNum;
			for(tCamNum = CAM1; tCamNum < ubUI_TotalBuNum; tCamNum++)
			{
				tDrawCamNum = tCamNum;
				if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (tCamViewSel.tCamViewType == DUAL_VIEW)) {
					tDrawCamNum = (tCamNum == CAM1)?tCamViewSel.tCamViewPool[0]:tCamViewSel.tCamViewPool[1];
				}
				if(tUI_CamStatus[tDrawCamNum].ulCAM_ID != INVALID_ID) {
					UI_RedrawBuConnectStatusIcon(tDrawCamNum);
				}
				if(PS_ECO_MODE == tUI_CamStatus[tDrawCamNum].tCamPsMode)
				{
					UI_DisplayLocation_t tUI_DispLoc;

					if(UI_SET_BUECOMODE_STATE == tUI_State)
						break;
					if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType)))
					{
						uint16_t uwXStart = 0, uwYStart = 0;
						tUI_DispLoc = tUI_GetTripViewDispLoc(tCamNum);
						if((DISP_LEFT == tUI_DispLoc) || (DISP_RIGHT == tUI_DispLoc) ||
						   ((DISP_3T_3CLEFT == tUI_DispLoc) || (DISP_3T_3CMID == tUI_DispLoc) || (DISP_3T_3CRIGHT == tUI_DispLoc)))
						{
							uwXStart += 180;
							uwYStart -= (((DISP_3T_3CLEFT == tUI_DispLoc) || (DISP_3T_3CMID == tUI_DispLoc))?104:(DISP_3T_3CRIGHT == tUI_DispLoc)?102:0);
						}
						if((DISP_3T_TOP == tUI_DispLoc) || (DISP_3T_BOTTOM == tUI_DispLoc))
							uwYStart = ((uwLCD_GetLcdVoSize()/2)-321);
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ECOSTS_ICON, 1, &tOsdImgInfo);
						UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
						tOsdImgInfo.uwXStart += uwXStart;
						tOsdImgInfo.uwYStart += uwYStart;
						UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
					}
					else
					{
						tUI_DispLoc = (tCamViewSel.tCamViewType == DUAL_VIEW)?((tDrawCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT):tUI_CamStatus[tDrawCamNum].tCamDispLocation;
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ECOSTS_ICON, 1, &tOsdImgInfo);
						UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
						tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewType == DUAL_VIEW)?180:(tCamViewSel.tCamViewType == QUAD_VIEW)?30:0;
						UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
					}
				}
			}
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RemoveLostLinkLogo(void)
{
	tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;
}
//------------------------------------------------------------------------------
void UI_ShowLostLinkLogo(uint16_t *pThreadCnt)
{
	UI_CamNum_t tCamNum;
	uint16_t uwUI_LostPeriod = (FALSE == ubUI_ResetPeriodFlag)?(UI_SHOWLOSTLOGO_PERIOD * 2):UI_SHOWLOSTLOGO_PERIOD;

	if(TRUE == ubUI_ResetPeriodFlag)
	{
		switch(tUI_PuSetting.tPsMode)
		{
			case PS_VOX_MODE:
				UI_DisableVox();
				break;
			case PS_ADOONLY_MODE:
				UI_DisablePuAdoOnlyMode();
				break;
			default:
				break;
		}
	}
	uint8_t total_bu = (tUI_PuSetting.ubTotalBuNum < CAM_4T) ? tUI_PuSetting.ubTotalBuNum : CAM_4T;
	for(tCamNum = CAM1; tCamNum < total_bu; tCamNum++)     //original  tUI_PuSetting.ubTotalBuNum
	{
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
		{
			if((GLB->LCD_FUNC_DIS == 1) ||
			   (TRUE == ubUI_StopUpdateStsBarFlag))
				return;
			//UI_DrawPUStatusIcon();	//huang
			//UI_DrawBUStatusIcon();	//huang
			return;
		}
	}
	if((FALSE == tUI_PuSetting.IconSts.ubShowLostLogoFlag) && (*pThreadCnt == uwUI_LostPeriod))
	{
		UI_CamNum_t tCamNum;

		tUI_PuSetting.IconSts.ubShowLostLogoFlag = TRUE;
		ubUI_ResetPeriodFlag = TRUE;
		if(TRUE == ubUI_StopUpdateStsBarFlag)
			return;
		switch(tUI_State)
		{
		    case UI_RECPLAYLIST_STATE:
			case UI_PHOTOPLAYLIST_STATE:
			case UI_PAIRING_STATE:
		#if APP_FS_FILE_LIST_STYLE
			case UI_RECFILES_SEL_STATE:
		#endif
				return;
			case UI_MAINMENU_STATE:
			case UI_SUBSUBSUBMENU_STATE:
			case UI_RECFOLDER_SEL_STATE:
		#if !APP_FS_FILE_LIST_STYLE
			case UI_RECFILES_SEL_STATE:
		#endif
            case UI_RECTXFOLDER_SEL_STATE: 
			case UI_RECTXFILES_SEL_STATE:   
			case UI_SDCARDFMT_STATE:
				break;
			case UI_SUBMENU_STATE:
			case UI_SUBSUBMENU_STATE:
				if(FALSE == ubUI_FastStateFlag)
					break;
			default:
			{
				OSD_IMG_INFO tOsdInfo;

				//for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)	//huang
				//	UI_RedrawBuConnectStatusIcon(tCamNum); //huang
				tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
				tOsdInfo.uwHSize  = uwOSD_GetHSize();
				tOsdInfo.uwVSize  = uwOSD_GetVSize();
				tOsdInfo.uwXStart = 0;
				tOsdInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdInfo);
				tUI_State = UI_DISPLAY_STATE;
				break;
			}
		}
#ifdef S2019A
		if(sPRF_APDIRECT_MODE == tsPRF_GetDrvMode())
		{
			OSD_IMG_INFO tOsdImgInfo;
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_WIFIDTENY_BG, 1, &tOsdImgInfo);
			tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
		}
		else
#endif
		{
			tLCD_JpegDecodeDisable();
			OSD_LogoJpeg(OSDLOGO_LOSTLINK);
		}
	}
}
//------------------------------------------------------------------------------
void UI_RedrawNoSignalOsdIcon(UI_CamNum_t tCamNum, UI_OsdImgFnType_t tOsdImgFnType)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_NOSIGNAL_STSICON, 1, &tOsdImgInfo);
	if ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))
	{
		tOsdImgInfo.uwXStart += 192;
		tOsdImgInfo.uwYStart -= 355;
		tUI_DispLoc = DISP_UPPER_LEFT;
	}
	else if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType)))
	{
		tUI_DispLoc = tUI_GetTripViewDispLoc(tCamNum);
		if((DISP_LEFT == tUI_DispLoc) || (DISP_RIGHT == tUI_DispLoc) ||
		   (DISP_3T_3CLEFT == tUI_DispLoc) || (DISP_3T_3CMID == tUI_DispLoc) || (DISP_3T_3CRIGHT == tUI_DispLoc))
		{
			tOsdImgInfo.uwXStart += 180;
			tOsdImgInfo.uwYStart -= 40;
			tOsdImgInfo.uwYStart -= (((DISP_3T_3CLEFT == tUI_DispLoc) || (DISP_3T_3CMID == tUI_DispLoc))?104:(DISP_3T_3CRIGHT == tUI_DispLoc)?102:0);
		}
		if((DISP_3T_TOP == tUI_DispLoc) || (DISP_3T_BOTTOM == tUI_DispLoc))
			tOsdImgInfo.uwYStart += ((uwLCD_GetLcdVoSize()/2)-355);
	}
	else
	{
		tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewType == DUAL_VIEW)?180:10;
		tOsdImgInfo.uwYStart -= 40;
		tUI_DispLoc = ((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (tCamViewSel.tCamViewType == DUAL_VIEW))?((tCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT):tUI_CamStatus[tCamNum].tCamDispLocation;
	}
	UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, tOsdImgFnType, tUI_DispLoc);
}
//------------------------------------------------------------------------------
void UI_ClearStatusBarOsdIcon(void)
{
	uint32_t ulLcd_HSize = (uwLCD_GetLcdHoSize() / 2);
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
	OSD_IMG_INFO tOsdImgInfo = {0};

	if(FALSE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
		return;

	tOsdImgInfo.uwHSize = 100;
	tOsdImgInfo.uwVSize = ulLcd_VSize;
	OSD_EraserImg1(&tOsdImgInfo);
	if(DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
	{
		tOsdImgInfo.uwXStart += ulLcd_HSize;
		OSD_EraserImg1(&tOsdImgInfo);
	}
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
}
//------------------------------------------------------------------------------
void UI_RedrawStatusBar(uint16_t *pThreadCnt)
{
	if(TRUE == ubUI_StopUpdateStsBarFlag)
		return;
	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
	{
		*pThreadCnt = (UI_UPDATESTS_PERIOD - 1);
		return;
	}
	if((*pThreadCnt % UI_UPDATESTS_PERIOD) == 0)
	{
		//UI_DrawPUStatusIcon(); //huang
		//UI_DrawBUStatusIcon(); //huang
	}
	if(TRUE == ubUI_ShowTimeFlag)
		UI_ShowSysTime();
}
//------------------------------------------------------------------------------
void UI_ChangeBuPsModeToNormalMode(UI_CamNum_t tPS_CamNum)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;

	tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_RIGHT:
				   (tCamViewSel.tCamViewType == DUAL_VIEW)?((tPS_CamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT):tUI_CamStatus[tPS_CamNum].tCamDispLocation;
	if (tCamViewSel.tCamViewType != SCAN_VIEW)
	{
		if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType)))
		{
			uint16_t uwXStart = 0, uwYStart = 0;
			tUI_DispLoc = tUI_GetTripViewDispLoc(tPS_CamNum);
			if((DISP_LEFT == tUI_DispLoc) || (DISP_RIGHT == tUI_DispLoc) ||
			   ((DISP_3T_3CLEFT == tUI_DispLoc) || (DISP_3T_3CMID == tUI_DispLoc) || (DISP_3T_3CRIGHT == tUI_DispLoc)))
			{
				uwXStart += 180;
				uwYStart -= (((DISP_3T_3CLEFT == tUI_DispLoc) || (DISP_3T_3CMID == tUI_DispLoc))?104:(DISP_3T_3CRIGHT == tUI_DispLoc)?102:0);
			}
			if((DISP_3T_TOP == tUI_DispLoc) || (DISP_3T_BOTTOM == tUI_DispLoc))
				uwYStart += ((uwLCD_GetLcdVoSize()/2)-321);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_NRMASK_ICON, 1, &tOsdImgInfo);
			UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwXStart;
			tOsdImgInfo.uwYStart += uwYStart;
			UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdErase, tUI_DispLoc);
		}		
		else
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_NRMASK_ICON, 1, &tOsdImgInfo);
			UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
			if (tCamViewSel.tCamViewType == SINGLE_VIEW)
			{
				tOsdImgInfo.uwXStart += 180;
				tOsdImgInfo.uwYStart -= 321;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			else
			{
				tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewType == DUAL_VIEW)?180:(tCamViewSel.tCamViewType == QUAD_VIEW)?30:0;
				UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdErase, tUI_DispLoc);
			}
		}
	}
	if(pUI_BuConnectFlag[0])
		*(pUI_BuConnectFlag[0]+tPS_CamNum) = FALSE;
	if(pUI_BuConnectFlag[1])
		*(pUI_BuConnectFlag[1]+tPS_CamNum) = FALSE;
	tUI_CamStatus[tPS_CamNum].tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_ReportBuConnectionStatus(void *pvConnectionSts)
{
	uint8_t *pCamConnSts = (uint8_t *)pvConnectionSts;
	UI_CamNum_t tCamNum;
	UI_PerMap2AntLvl_t tAntMap[] =
	{
		{ANT_NOSIGNAL, 		 10},
		{ANT_SIGNALLVL1, 	 20},
		{ANT_SIGNALLVL2, 	 40},
		{ANT_SIGNALLVL3, 	 60},
		{ANT_SIGNALLVL4, 	 80},
		{ANT_SIGNALLVL5, 	100},
	};
	uint8_t ubUI_AntLvlCnt = sizeof tAntMap / sizeof(UI_PerMap2AntLvl_t), ubIdx;
	static uint8_t ubUI_PsStsFlag = FALSE;
	static uint32_t ulUI_EcoStsCnt[CAM_4T] = {0, 0, 0, 0};
	static uint8_t ubUI_TotalBuEcoNum = 0, ubUI_WakeUpBuCnt = 0;

	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		tUI_CamStatus[tCamNum].tCamConnSts 	= (pCamConnSts[APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum] == rLINK)?CAM_ONLINE:CAM_OFFLINE;
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
		{
			switch(tUI_CamStatus[tCamNum].tCamConnSts)
			{
				case CAM_ONLINE:
					if(TRUE == ulUI_MonitorPsFlag[tCamNum])
					{
						APP_EventMsg_t tUI_PsMessage = {0};

						tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
						tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
						tUI_PsMessage.ubAPP_Message[1] = PS_ECO_MODE;
						tUI_PsMessage.ubAPP_Message[2] = FALSE;
						tUI_PsMessage.ubAPP_Message[3] = tCamNum;
						tUI_PsMessage.ubAPP_Message[4] = TRUE;
						UI_SendMessageToAPP(&tUI_PsMessage);
						UI_ChangeBuPsModeToNormalMode(tCamNum);
						ulUI_MonitorPsFlag[tCamNum] = FALSE;
						if(ubUI_TotalBuEcoNum)
							--ubUI_TotalBuEcoNum;
					}
					else
					{
					#define	UI_CHKBUECOSTS_PERIOD (2000 / UI_TASK_PERIOD)
						if(++ulUI_EcoStsCnt[tCamNum] > UI_CHKBUECOSTS_PERIOD)
						{
							ulUI_EcoStsCnt[tCamNum] = 0;
							UI_ChangeBuPsModeToNormalMode(tCamNum);
						}
					}
					break;
				case CAM_OFFLINE:
					if(FALSE == ulUI_MonitorPsFlag[tCamNum])
					{
						ulUI_EcoStsCnt[tCamNum] = 0;
						ulUI_MonitorPsFlag[tCamNum] = TRUE;
						++ubUI_TotalBuEcoNum;
						ubUI_WakeUpBuCnt = 0;
					}
					tUI_CamStatus[tCamNum].tCamConnSts = CAM_ONLINE;
					if(FALSE == ubUI_PsStsFlag)
					{
						tLCD_JpegDecodeDisable();
						//UI_DrawPUStatusIcon(); //huang
						//UI_DrawBUStatusIcon(); //huang
						ubUI_PsStsFlag = TRUE;
					}
					continue;
				default:
					break;
			}
		}
		else
			ulUI_EcoStsCnt[tCamNum] = 0;
#ifdef A7130
		if((BB_ENABLE_ALL_STA_WAKEUP == tKNL_GetWORMode()) && (TRUE == ubUI_WakeUpFromPsFlag))
		{
		#define	UI_PSWAKEUP_TIME (6000 / UI_TASK_PERIOD)
			if((!ubUI_TotalBuEcoNum) || (++ubUI_WakeUpBuCnt > UI_PSWAKEUP_TIME))
			{
				APP_EventMsg_t tUI_PsMessage = {0};

				tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
				tUI_PsMessage.ubAPP_Message[0] = 4;
				tUI_PsMessage.ubAPP_Message[1] = POWER_NORMAL_MODE;
				tUI_PsMessage.ubAPP_Message[2] = FALSE;
				tUI_PsMessage.ubAPP_Message[3] = CAM1;
				UI_SendMessageToAPP(&tUI_PsMessage);
				ubUI_WakeUpFromPsFlag = FALSE;
				ubUI_TotalBuEcoNum	  = 0;
				ubUI_WakeUpBuCnt	  = 0;
				printd(DBG_Debug1Lvl, "  _Stop Wake-Up !\n");
			}
		}
#else
		ubUI_WakeUpFromPsFlag = ubUI_WakeUpFromPsFlag;
		ubUI_WakeUpBuCnt = ubUI_WakeUpBuCnt;
#endif
		tUI_CamStatus[tCamNum].tCamAntLvl = ANT_NOSIGNAL;
//		printd(DBG_Debug3Lvl, "RSSI : %d\n", pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 8]);
		printd(DBG_Debug3Lvl, "PER: %d\n", pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 4]);
		for(ubIdx = 0; ubIdx < ubUI_AntLvlCnt; ubIdx++)
		{
//			if((pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 8]) < tAntMap[ubIdx].ubRssiValue)
			if((pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 4]) <= tAntMap[ubIdx].ubPerValue)
			{
				tUI_CamStatus[tCamNum].tCamAntLvl = tAntMap[ubIdx].tAntLvl;
				break;
			}
		}
	}
}
//------------------------------------------------------------------------------
void UI_UpdateBuStatus(UI_CamNum_t tCamNum, void *pvStatus)
{
//	uint8_t *pUI_BuSts = (uint8_t *)pvStatus;

//	if(tCamNum > CAM4)
//		return;

//	tUI_CamStatus[tCamNum].tCamBatLvl = pUI_BuSts[0];
}
//------------------------------------------------------------------------------
void UI_LeftArrowLongKey(void)
{
//	OSD_IMG_INFO tOsdImgInfo;
//	UI_VdoModeList_t tVdoModeList, tVdoModeHL;

//	if(UI_DISPLAY_STATE != tUI_State)
//		return;

//	tVdoModeHL = (UI_PHOTOCAP_MODE == tUI_PuSetting.tVdoMode)?UI_VDOPHOTO_MODE:
//	             (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode)?UI_VDORECLOOP_MODE:
//	             (REC_MANUAL  == tUI_PuSetting.RecInfo.tREC_Mode)?UI_VDORECMANU_MODE:UI_VDORECTRIG_MODE;
//	for(tVdoModeList = UI_VDORECLOOP_MODE; tVdoModeList < UI_VDOMODELIST_MAX; tVdoModeList++)
//	{
//		tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_RECLOOPMODENOR_ICON+(tVdoModeList*2)+((tVdoModeList == tVdoModeHL)?UI_ICON_HIGHLIGHT:0)), 1, &tOsdImgInfo);
//		tOSD_Img2(&tOsdImgInfo, ((tVdoModeList+1) == UI_VDOMODELIST_MAX)?OSD_UPDATE:OSD_QUEUE);
//	}
//	tUI_State = UI_SET_VDOMODE_STATE;
}
//------------------------------------------------------------------------------
void UI_RightArrowLongKey(void)
{
	if(UI_DISPLAY_STATE != tUI_State)
		return;
	if(TRUE == ubUI_ShowTimeFlag)
	{
		OSD_IMG_INFO tOsdImgInfo;

		ubUI_ShowTimeFlag = FALSE;
		tOsdImgInfo.uwXStart = 670;
		tOsdImgInfo.uwYStart = 980;
		tOsdImgInfo.uwHSize  = 50;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
		return;
	}
	ubUI_ShowTimeFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_ShowSysTime(void)
{
	static OSD_IMG_INFO tOsdImgInfo[11];
	static uint8_t ubUI_SysTimeFlag = FALSE;
	uint16_t uwYOffset = uwLCD_GetLcdVoSize();
	uint8_t *pDT_Num[3];
	uint8_t ubTen = 0, ubUnit = 0, ubSysTimeIdx = 0;
	RTC_Calendar_t tSysCalendar;

	if(FALSE == ubUI_SysTimeFlag)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CK_NUM0, 11, &tOsdImgInfo[0]);
		ubUI_SysTimeFlag = TRUE;
	}
	RTC_GetCalendar((RTC_Calendar_t *)(&tSysCalendar));
	printd(DBG_Debug2Lvl, "Time: %02d:%02d:%02d\n", tSysCalendar.ubHour, tSysCalendar.ubMin, tSysCalendar.ubSec);
	if(tUI_PuSetting.tSysCalendar.ubSec == tSysCalendar.ubSec)
		return;
	pDT_Num[0] = (uint8_t *)(&tSysCalendar.ubHour);
	pDT_Num[1] = (uint8_t *)(&tSysCalendar.ubMin);
	pDT_Num[2] = (uint8_t *)(&tSysCalendar.ubSec);
	for(ubSysTimeIdx = 0; ubSysTimeIdx < 3; ubSysTimeIdx++)
	{
		ubTen  = *pDT_Num[ubSysTimeIdx] / 10;
		ubUnit = *pDT_Num[ubSysTimeIdx] - (ubTen * 10);
		tOsdImgInfo[ubTen].uwXStart  = 680;
		tOsdImgInfo[ubTen].uwYStart  = uwYOffset - tOsdImgInfo[ubTen].uwVSize;
		tOSD_Img2(&tOsdImgInfo[ubTen], OSD_QUEUE);
		tOsdImgInfo[ubUnit].uwXStart = 680;
		tOsdImgInfo[ubUnit].uwYStart = tOsdImgInfo[ubTen].uwYStart - tOsdImgInfo[ubUnit].uwVSize;
		tOSD_Img2(&tOsdImgInfo[ubUnit], (ubSysTimeIdx == 2)?OSD_UPDATE:OSD_QUEUE);
		if(ubSysTimeIdx != 2)
		{
			tOsdImgInfo[10].uwXStart = 680;
			tOsdImgInfo[10].uwYStart = tOsdImgInfo[ubUnit].uwYStart - tOsdImgInfo[10].uwVSize;
			tOSD_Img2(&tOsdImgInfo[10], OSD_QUEUE);
			uwYOffset = tOsdImgInfo[10].uwYStart;
		}
		tOsdImgInfo[ubTen].uwXStart  = 0;
		tOsdImgInfo[ubTen].uwYStart  = 0;
		tOsdImgInfo[ubUnit].uwXStart = 0;
		tOsdImgInfo[ubUnit].uwYStart = 0;
		tOsdImgInfo[10].uwXStart	 = 0;
		tOsdImgInfo[10].uwYStart	 = 0;
	}
}
//------------------------------------------------------------------------------
void UI_UnBindBu(UI_CamNum_t tUI_DelCam)
{
	APP_EventMsg_t tUI_UnindBuMsg = {0};

	if(INVALID_ID == tUI_CamStatus[tUI_DelCam].ulCAM_ID)
		return;
	tUI_CamStatus[tUI_DelCam].ulCAM_ID 	  = INVALID_ID;
	tUI_CamStatus[tUI_DelCam].tCamConnSts = CAM_OFFLINE;
	tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
	UI_ResetDevSetting(tUI_DelCam);
	UI_UpdateDevStatusInfo();
	tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_CAM_EVENT;
	tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
	tUI_UnindBuMsg.ubAPP_Message[1] = tUI_DelCam;
	UI_SendMessageToAPP(&tUI_UnindBuMsg);
}
//------------------------------------------------------------------------------
void UI_VoxTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	if(tCamNum > CAM4)
		return;

	if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
		UI_DisableVox();
}
//------------------------------------------------------------------------------
void UI_EnableVox(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

//	if(DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum)
//	{
//		tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;
//		UI_UpdateDevStatusInfo();
//		return;
//	}

	LCDBL_ENABLE(UI_DISABLE);
	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_PsMessage.ubAPP_Message[2] = TRUE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_PuSetting.tPsMode = PS_VOX_MODE;
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_DisableVox(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};
	UI_PUReqCmd_t tPsCmd;
	UI_CamNum_t tCamNum;

	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
			continue;
		tUI_CamStatus[tCamNum].tCamPsMode = POWER_NORMAL_MODE;
		if(CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts)
			continue;
		tPsCmd.tDS_CamNum 				= tCamNum;
		tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
		tPsCmd.ubCmd[UI_SETTING_DATA]   = POWER_NORMAL_MODE;
		tPsCmd.ubCmd_Len  				= 3;
		if(UI_SendRequestToBU(osThreadGetId(), &tPsCmd) != rUI_SUCCESS)
			printd(DBG_ErrorLvl, "CAM%d:Disable VOX Notify Fail !\n", tCamNum);
	}
	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_PsMessage.ubAPP_Message[2] = FALSE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
	if((APP_LINK_STATE == tUI_SyncAppState) &&
	(UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode ))
		UI_VideoRecordingExec(UI_REC_START);
	tUI_State = UI_DISPLAY_STATE;
}
//------------------------------------------------------------------------------
void UI_DisablePuAdoOnlyMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	UI_DisableScanMode();
	UI_ClearStatusBarOsdIcon();
	UI_ClearBuConnectStatusFlag();
	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_ADOONLY_MODE;
	tUI_PsMessage.ubAPP_Message[2] = FALSE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;
	tUI_State = UI_DISPLAY_STATE;
	switch(tCamViewSel.tCamViewType)
	{
		case SINGLE_VIEW:
		case SCAN_VIEW:
			if(tCamViewSel.tCamViewPool[0] != tUI_PuSetting.tAdoSrcCamNum)
				UI_SwitchAudioSource(tCamViewSel.tCamViewPool[0]);
			if(SINGLE_VIEW == tCamViewSel.tCamViewType)
				break;
			UI_EnableScanMode();
			break;
		default:
			break;
	}
	if((APP_LINK_STATE == tUI_SyncAppState) &&
	   (UI_RECORDING_MODE == tUI_PuSetting.tVdoMode) && (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode ))
		UI_VideoRecordingExec(UI_REC_START);
}
//------------------------------------------------------------------------------
void UI_MDTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint16_t uwXOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
	                          [DISP_LOWER_LEFT] = (ulLcd_HSize/2), [DISP_LOWER_RIGHT] = (ulLcd_HSize/2),
							  [DISP_LEFT]	    = 0,			   [DISP_RIGHT]		  = 0,
							  [DISP_3T_TOP]		= 0,			   [DISP_3T_BOTTOM]	  = (ulLcd_HSize/2),
							  [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = 0,
	                          [DISP_3T_3CRIGHT] = 0};
	uint16_t uwYOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
	                          [DISP_LOWER_LEFT] = 0, 			   [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
							  [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT]	   	  = (ulLcd_VSize/2),
						      [DISP_3T_TOP]	    = 0,			   [DISP_3T_BOTTOM]	  = (ulLcd_VSize/2),
							  [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = (ulLcd_VSize/3),
	                          [DISP_3T_3CRIGHT] = ((ulLcd_VSize/3)*2)};
	uint16_t uw3TColYOffset = 0;

	if(tCamNum > CAM4)
		return;
	if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType)))
	{
		tUI_DispLoc = tUI_GetTripViewDispLoc(tCamNum);
		uw3TColYOffset = (((DISP_3T_3CLEFT == tUI_DispLoc) || (DISP_3T_3CMID == tUI_DispLoc))?214:(DISP_3T_3CRIGHT == tUI_DispLoc)?212:0);
	}
	else
		tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_LEFT:tUI_CamStatus[tCamNum].tCamDispLocation;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MDTRIG_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset[tUI_DispLoc];
	tOsdImgInfo.uwYStart += uw3TColYOffset;
	tOsdImgInfo.uwYStart -= uwYOffset[tUI_DispLoc];
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_PuSetting.IconSts.ubDrawMdTrigFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_VoiceTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	if(DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum)
		return;

	if(SCAN_VIEW == tCamViewSel.tCamViewType)
		UI_DisableScanMode();
	if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
	{
		UI_DisableVox();
		osDelay(50);
	}
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
	UI_ClearStatusBarOsdIcon();
	UI_ClearBuConnectStatusFlag();
	tUI_CamNumSel = tCamViewSel.tCamViewPool[0];
	tCamViewSel.tCamViewType = SINGLE_VIEW;
	tCamViewSel.tCamViewPool[0] = tCamNum;
	UI_SwitchCameraSource();	
	tUI_State = UI_DISPLAY_STATE;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SendRequestToBU(osThreadId thread_id, UI_PUReqCmd_t *ptReqCmd)
{
	UI_Result_t tReq_Result = rUI_SUCCESS;
	osEvent tReq_Event;
	APP_StaNumMap_t *pUI_CamNumMap = APP_GetSTANumMappingTable(ptReqCmd->tDS_CamNum);
	uint8_t ubUI_TwcRetry = 5;

	tosUI_Notify.thread_id = thread_id;
	tosUI_Notify.iSignals  = osUI_SIGNALS;
	while(--ubUI_TwcRetry)
	{
		if(tTWC_Send(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING, ptReqCmd->ubCmd, ptReqCmd->ubCmd_Len, 10) == TWC_SUCCESS)
			break;
		osDelay(10);
	}
	if(!ubUI_TwcRetry)
	{
		tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING);
		return rUI_FAIL;
	}
	if(tosUI_Notify.thread_id != NULL)
	{
		tReq_Event = osSignalWait(tosUI_Notify.iSignals, UI_TWC_TIMEOUT);
		tReq_Result = (tReq_Event.status == osEventSignal)?(tReq_Event.value.signals == tosUI_Notify.iSignals)?tosUI_Notify.tReportSts:rUI_FAIL:rUI_FAIL;
		tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING);
		tosUI_Notify.thread_id  = NULL;
		tosUI_Notify.iSignals   = NULL;
		tosUI_Notify.tReportSts = rUI_SUCCESS;
	}
	return tReq_Result;
}
//------------------------------------------------------------------------------
void UI_RecvBUResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus)
{
	if(NULL == tosUI_Notify.thread_id)
		return;
	tosUI_Notify.tReportSts = (tStatus == TWC_SUCCESS)?rUI_SUCCESS:rUI_FAIL;
	if(osSignalSet(tosUI_Notify.thread_id, osUI_SIGNALS) != osOK)
		printd(DBG_ErrorLvl, "UI thread notify fail !\n");
}
//------------------------------------------------------------------------------
void UI_RecvBURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data)
{
	switch(pTwc_Data[UI_TWC_TYPE])
	{
		case UI_REPORT:
		{
			UI_CamNum_t tCamNum = NO_CAM;

			APP_TwcTagMap2CamNum(tRecv_StaNum, tCamNum);
			if(tUiReportMap2Func[pTwc_Data[UI_REPORT_ITEM]].pvAction)
				tUiReportMap2Func[pTwc_Data[UI_REPORT_ITEM]].pvAction(tCamNum, (uint8_t *)(&pTwc_Data[UI_REPORT_DATA]));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ResetDevSetting(UI_CamNum_t tCamNum)
{
	ulUI_MonitorPsFlag[tCamNum] = FALSE;
	tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
	tUI_CamStatus[tCamNum].tCamPsMode = POWER_NORMAL_MODE;
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamAnrMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCam3DNRMode, 		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamvLDCMode, 		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamAecMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamDisMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamFlicker, 		CAMFLICKER_50HZ);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamCbrMode,  		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamCondenseMode,  CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL, 		 	64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast,    64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation, 	64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue, 		64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tREC_Mode, 		REC_MANUAL);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tREC_Resolution, 	RECRES_HD);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tPHOTO_Func, 		PHOTOFUNC_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tPHOTO_Resolution, PHOTORES_3M);
	//zhu
	tUI_CamStatus[tCamNum].sleep_time_ms = 0;
  tUI_CamStatus[tCamNum].priority      = tCamNum;

  tUI_CamStatus[tCamNum].sleep_hist_count = 0;
  tUI_CamStatus[tCamNum].sleep_hist_head  = UI_SLEEP_HISTORY_DEPTH - 1;
  memset(tUI_CamStatus[tCamNum].sleep_hist_ms, 0,
         sizeof(tUI_CamStatus[tCamNum].sleep_hist_ms));
}
//------------------------------------------------------------------------------
void UI_LoadDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DeviceStatusInfo_t tUI_DevStsInfo = {{0}, {0}, {0}, {0}};
	UI_CamNum_t tCamNum;

	SF_Read(ulUI_SFAddr, sizeof(UI_DeviceStatusInfo_t), (uint8_t *)&tUI_DevStsInfo);
	memcpy(tUI_CamStatus, tUI_DevStsInfo.tBU_StatusInfo, (CAM_4T * sizeof(UI_BUStatus_t)));
	memcpy(&tUI_PuSetting, &tUI_DevStsInfo.tPU_SettingInfo, sizeof(UI_PUSetting_t));
  // ----------------------------------------------------
	for (tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
  {
     tUI_CamStatus[tCamNum].name[sizeof(tUI_CamStatus[tCamNum].name) - 1] = '\0';
  }
  // ----------------------------------------------------
	printd(DBG_InfoLvl, "UI TAG:%s\n",tUI_DevStsInfo.cbUI_DevStsTag);
	printd(DBG_InfoLvl, "UI VER:%s\n",tUI_DevStsInfo.cbUI_FwVersion);
	if((strncmp(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1)) ||
	   (strncmp(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION,  sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1)))
	{
		printd(DBG_ErrorLvl, "UI Default Setting !\n");
		for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
			memset(&tUI_CamStatus[tCamNum], 0xFF, sizeof(UI_BUStatus_t));
		memset(&tUI_PuSetting, 0xFF, sizeof(UI_PUSetting_t));
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.uwYear, 2018);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubMonth,   1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubDate,    1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubHour,    0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubMin,     0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubSec,     0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.RecInfo.tREC_Mode, REC_OFF);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.RecInfo.tREC_Time, RECTIME_5MIN);
        REC_TimeSet(0,300);
		tUI_PuSetting.tVdoMode = UI_PHOTOCAP_MODE;
		tUI_PuSetting.VolLvL.tVOL_UpdateLvL		 = VOL_LVL3;		//VOLUME
		tUI_PuSetting.ubVibration = 1;      //  LOW
		tUI_PuSetting.ubZoomLevel = 0;      //  OFF
		tUI_PuSetting.ubLanguage = 0;       //  ENGLISH
		tUI_PuSetting.ubTempAlarmOn = 0; 		//	CLOSE
		tUI_PuSetting.ubTempMax = 30;    		//  High Temp
		tUI_PuSetting.ubTempMin = 10;    		//  Low Temp
		tUI_PuSetting.ubKeyLockAutoActivate = 0; //NEVER
		for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
    {
        tUI_CamStatus[tCamNum].ubMicroSensitivity = 3;
    }
	}
	tUI_PuSetting.ubTotalBuNum 				 = DISPLAY_MODE;
	tUI_PuSetting.tAdoSrcCamNum				 = (tUI_PuSetting.tAdoSrcCamNum > CAM4)?CAM1:tUI_PuSetting.tAdoSrcCamNum;
	//tUI_PuSetting.BriLvL.tBL_UpdateLvL		 = BL_LVL5;
	//tUI_PuSetting.VolLvL.tVOL_UpdateLvL		 = VOL_LVL3;		//close default config
	tUI_PuSetting.IconSts.ubDrawStsIconFlag  = FALSE;
	tUI_PuSetting.IconSts.ubRdPairIconFlag   = FALSE;
	tUI_PuSetting.IconSts.ubClearThdCntFlag	 = FALSE;
	tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;
	tUI_PuSetting.IconSts.ubDrawMdTrigFlag 	 = FALSE;
	tUI_PuSetting.ubPairedBuNum				 = 0;
	UI_CHK_PUSYS(tUI_PuSetting.ubAEC_Mode, AECFUNC_MAX, AECFUNC_OFF);
	UI_CHK_PUSYS(tUI_PuSetting.ubCCA_Mode, CCAMODE_MAX, CCAFUNC_OFF);
	UI_CHK_PUSYS(tUI_PuSetting.tPsMode, POWER_NORMAL_MODE, POWER_NORMAL_MODE);
	UI_CHK_PUSYS(tUI_PuSetting.RecInfo.tREC_Mode, REC_RECMODE_MAX, REC_OFF);
	UI_CHK_PUSYS(tUI_PuSetting.RecInfo.tREC_Time, RECTIME_MAX, RECTIME_5MIN);
	UI_CHK_PUSYS(tUI_PuSetting.tVdoMode, UI_VDOMODE_MAX, UI_PHOTOCAP_MODE);
	if(UI_PHOTOCAP_MODE == tUI_PuSetting.tVdoMode)
		tUI_PuSetting.RecInfo.tREC_Mode = REC_OFF;

	if(PS_WOR_MODE == tUI_PuSetting.tPsMode)
	{
		ulUI_LogoIndex = OSDLOGO_WORBOOT;
		tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;
		UI_UpdateDevStatusInfo();
	}
#ifdef S2019A
	else if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
	{
		tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;
		UI_UpdateDevStatusInfo();
	}
#endif

	tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;
		UI_UpdateDevStatusInfo();
    if(tUI_PuSetting.RecInfo.tREC_Time == RECTIME_1MIN)
        REC_TimeSet(0,60);
    else if(tUI_PuSetting.RecInfo.tREC_Time == RECTIME_3MIN)
        REC_TimeSet(0,180);
    else if(tUI_PuSetting.RecInfo.tREC_Time == RECTIME_5MIN)
        REC_TimeSet(0,300);

	for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
	{
		if ((strncmp(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1) == 0)
		&& (strncmp(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1) == 0)) {
			ulUI_MonitorPsFlag[tCamNum]   = FALSE;
			tUI_CamStatus[tCamNum].tCamConnSts = CAM_ONLINE;
			if(tCamNum >= tUI_PuSetting.ubTotalBuNum)
				tUI_CamStatus[tCamNum].ulCAM_ID = INVALID_ID;
			if(INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID)
				tUI_PuSetting.ubPairedBuNum += 1;
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamAnrMode,  		CAMSET_OFF);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCam3DNRMode, 		CAMSET_OFF);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamvLDCMode, 		CAMSET_OFF);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamAecMode,  		CAMSET_OFF);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamDisMode,  		CAMSET_OFF);
			UI_CHK_CAMFLICKER(tUI_CamStatus[tCamNum].tCamFlicker);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamCbrMode,  		CAMSET_ON);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamCondenseMode,   CAMSET_OFF);
			UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL, 		 64);
			UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast,   64);
			UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation, 64);
			UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue, 		 64);
			UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tCamPsMode, POWER_NORMAL_MODE, POWER_NORMAL_MODE);
			UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tREC_Mode, REC_RECMODE_MAX, REC_MANUAL);
			UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tREC_Resolution, RECRES_MAX, RECRES_HD);
			UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tPHOTO_Func, PHOTOFUNC_MAX, PHOTOFUNC_OFF);
			UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tPHOTO_Resolution, PHOTORES_MAX, PHOTORES_3M);
		} else {
				tUI_CamStatus[tCamNum].ulCAM_ID = INVALID_ID;
			UI_ResetDevSetting(tCamNum);
			sprintf(tUI_CamStatus[tCamNum].name, "BABY UNIT %d", tCamNum + 1); //zhu
			tUI_CamStatus[tCamNum].ubMicroSensitivity = 3;
			tUI_CamStatus[tCamNum].sleep_time_ms = 0;
			tUI_CamStatus[tCamNum].priority = tCamNum;
		}
	}
  for (tCamNum = CAM1; tCamNum < DISPLAY_MODE; tCamNum++)
  {
      if (tUI_CamStatus[tCamNum].sleep_hist_count > UI_SLEEP_HISTORY_DEPTH)
			{
          tUI_CamStatus[tCamNum].sleep_hist_count = 0;
          tUI_CamStatus[tCamNum].sleep_hist_head  = UI_SLEEP_HISTORY_DEPTH - 1;
          memset(tUI_CamStatus[tCamNum].sleep_hist_ms, 0,
                 sizeof(tUI_CamStatus[tCamNum].sleep_hist_ms));
      }
			else
			{
          tUI_CamStatus[tCamNum].sleep_hist_head %= UI_SLEEP_HISTORY_DEPTH;
      }
  }
}
//------------------------------------------------------------------------------
void UI_UpdateDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DeviceStatusInfo_t tUI_DevStsInfo = {{0}, {0}, {0}, {0}};

	osMutexWait(APP_UpdateMutex, osWaitForever);
	memcpy(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1);
	memcpy(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1);
	memcpy(tUI_DevStsInfo.tBU_StatusInfo, tUI_CamStatus, (CAM_4T * sizeof(UI_BUStatus_t)));
	memcpy(&tUI_DevStsInfo.tPU_SettingInfo, &tUI_PuSetting, sizeof(UI_PUSetting_t));
	printf("\n--- SAVING TO FLASH ---\n");//zhu
    printf("Tag to be saved: [%s]\n", tUI_DevStsInfo.cbUI_DevStsTag);
    printf("Version to be saved: [%s]\n", tUI_DevStsInfo.cbUI_FwVersion);
    printf("Vibration to be saved: %d\n", tUI_DevStsInfo.tPU_SettingInfo.ubVibration);
    printf("-----------------------\n\n");
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulUI_SFAddr, pSF_Info->ulSecSize, 1);
	SF_Write(ulUI_SFAddr, sizeof(UI_DeviceStatusInfo_t), (uint8_t *)&tUI_DevStsInfo);
	SF_EnableWrProtect();
	osMutexRelease(APP_UpdateMutex);
}
//------------------------------------------------------------------------------
void UI_TimerEventStart(uint32_t ulTime_ms, void *pvRegCb)
{
	TIMER_SETUP_t tUI_TimerParam;

	tUI_TimerParam.tCLK 		= TIMER_CLK_EXTCLK;
	tUI_TimerParam.ulTmLoad 	= 10000 * ulTime_ms;
	tUI_TimerParam.ulTmCounter 	= tUI_TimerParam.ulTmLoad;
	tUI_TimerParam.ulTmMatch1 	= tUI_TimerParam.ulTmLoad + 1;
	tUI_TimerParam.ulTmMatch2 	= tUI_TimerParam.ulTmLoad + 1;
	tUI_TimerParam.tOF 			= TIMER_OF_ENABLE;
	tUI_TimerParam.tDIR 		= TIMER_DOWN_CNT;
	tUI_TimerParam.tEM 			= TIMER_CB;
	tUI_TimerParam.pvEvent 		= pvRegCb;
	TIMER_Start(TIMER1_3, tUI_TimerParam);
}
//------------------------------------------------------------------------------
void UI_TimerEventStop(void)
{
	TIMER_Stop(TIMER1_3);
}
//------------------------------------------------------------------------------
void UI_ScanModeTimerEvent(void)
{
	UI_Event_t tScanEvent;
	osMessageQId *pUI_ScanEventQH = NULL;

	UI_TimerEventStop();
	tScanEvent.tEventType = SCANMODE_EVENT;
	tScanEvent.pvEvent 	  = NULL;
	pUI_ScanEventQH 	  = pUI_GetEventQueueHandle();
    osMessagePut(*pUI_ScanEventQH, &tScanEvent, 0);
}
//------------------------------------------------------------------------------
void UI_SetupScanModeTimer(uint8_t ubTimerEn)
{
	ubUI_ScanStartFlag = ubTimerEn;
	if(TRUE == ubTimerEn)
		UI_TimerEventStart(10000, UI_ScanModeTimerEvent);
	else
		UI_TimerEventStop();
}
//------------------------------------------------------------------------------
void UI_EnableScanMode(void)
{
	if(PS_ADOONLY_MODE != tUI_PuSetting.tPsMode)
		UI_CheckCameraSource4SV();
	UI_SetupScanModeTimer(TRUE);
}
//------------------------------------------------------------------------------
void UI_DisableScanMode(void)
{
	if(FALSE == ubUI_ScanStartFlag)
		return;
	UI_SetupScanModeTimer(FALSE);
}
//------------------------------------------------------------------------------
void UI_ScanModeExec(void)
{
	UI_CamNum_t tSearchCam = tCamViewSel.tCamViewPool[0];
	uint8_t ubSearchCnt;

	tSearchCam = (PS_ADOONLY_MODE == tUI_PuSetting.tPsMode)?tUI_PuSetting.tAdoSrcCamNum:tCamViewSel.tCamViewPool[0];
	for(ubSearchCnt = 0; ubSearchCnt < tUI_PuSetting.ubTotalBuNum; ubSearchCnt++)
	{
		tSearchCam = ((tSearchCam + 1) >= ((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?CAM_2T:CAM_4T))?CAM1:((UI_CamNum_t)(tSearchCam + 1));
		if((tUI_CamStatus[tSearchCam].ulCAM_ID != INVALID_ID) &&
		   //(tUI_CamStatus[tSearchCam].tCamConnSts == CAM_ONLINE) &&
		   (tUI_CamStatus[tSearchCam].tCamPsMode == POWER_NORMAL_MODE))
		{
			if(PS_ADOONLY_MODE == tUI_PuSetting.tPsMode)
			{
				UI_SwitchAudioSource(tSearchCam);
				break;
			}
			else if(tSearchCam != tCamViewSel.tCamViewPool[0])
			{
				tCamViewSel.tCamViewType	= SCAN_VIEW;
				tCamViewSel.tCamViewPool[0] = tSearchCam;
				UI_SwitchCameraSource();
				UI_ClearBuConnectStatusFlag();
				break;
			}
		}
	}
	UI_SetupScanModeTimer(TRUE);
}
//------------------------------------------------------------------------------
UI_Result_t UI_CheckCameraSource4SV(void)
{
	UI_CamNum_t tCamViewNum;

	//if((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
	//   (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
	if((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID))
		return rUI_SUCCESS;

	for(tCamViewNum = CAM1; tCamViewNum < tUI_PuSetting.ubTotalBuNum; tCamViewNum++)
	{
		//if((tUI_CamStatus[tCamViewNum].ulCAM_ID != INVALID_ID) &&
	    //   (tUI_CamStatus[tCamViewNum].tCamConnSts == CAM_ONLINE))
		if((tUI_CamStatus[tCamViewNum].ulCAM_ID != INVALID_ID))
		{
			tCamViewSel.tCamViewType	= SCAN_VIEW;
			tCamViewSel.tCamViewPool[0] = tCamViewNum;
			tUI_PuSetting.tAdoSrcCamNum = tCamViewNum;
			return rUI_SUCCESS;
		}
	}
	return rUI_FAIL;
}
//------------------------------------------------------------------------------
void UI_SwitchCameraSource(void)
{
	APP_EventMsg_t tUI_SwitchBuMsg = {0};

	tUI_SwitchBuMsg.ubAPP_Event 	 = APP_VIEWTYPECHG_EVENT;
	tUI_SwitchBuMsg.ubAPP_Message[0] = 3;		//! Message Length
	tUI_SwitchBuMsg.ubAPP_Message[1] = tCamViewSel.tCamViewType;
	tUI_SwitchBuMsg.ubAPP_Message[2] = tCamViewSel.tCamViewPool[0];
	tUI_SwitchBuMsg.ubAPP_Message[3] = tCamViewSel.tCamViewPool[1];
	UI_SendMessageToAPP(&tUI_SwitchBuMsg);
	if ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))
		tUI_PuSetting.tAdoSrcCamNum = tCamViewSel.tCamViewPool[0];
	RTC_WriteUserRam(RTC_RECORD_VIEW_MODE_ADDR, tCamViewSel.tCamViewType);	
	RTC_WriteUserRam(RTC_RECORD_VIEW_CAM_ADDR, (tCamViewSel.tCamViewPool[0] << 4) | tCamViewSel.tCamViewPool[1]);
}
//------------------------------------------------------------------------------
void UI_SwitchViewTypeFg(uint8_t ubFg)
{
    ubUI_SwtichViewTypeFg = ubFg;
}
//------------------------------------------------------------------------------
void UI_SwitchAudioSource(UI_CamNum_t tCamNum)
{
	APP_EventMsg_t tUI_SwitchAdoSrcMsg = {0};

	tUI_PuSetting.tAdoSrcCamNum			 = tCamNum;
	tUI_SwitchAdoSrcMsg.ubAPP_Event 	 = APP_ADOSRCSEL_EVENT;
	tUI_SwitchAdoSrcMsg.ubAPP_Message[0] = 1;		//! Message Length
	tUI_SwitchAdoSrcMsg.ubAPP_Message[1] = tUI_PuSetting.tAdoSrcCamNum;
	tUI_SwitchAdoSrcMsg.ubAPP_Message[2] = (PS_ADOONLY_MODE == tUI_PuSetting.tPsMode)?FALSE:TRUE;
	UI_SendMessageToAPP(&tUI_SwitchAdoSrcMsg);
}
//------------------------------------------------------------------------------
void UI_SwitchCameraSource4TripleView(void)
{
	APP_EventMsg_t tUI_SwitchBuMsg = {0};

	tUI_SwitchBuMsg.ubAPP_Event 	 = APP_VIEWTYPECHG_EVENT;
	tUI_SwitchBuMsg.ubAPP_Message[0] = 4;		//! Message Length
	tUI_SwitchBuMsg.ubAPP_Message[1] = tCamViewSel.tCamViewType;
	tUI_SwitchBuMsg.ubAPP_Message[2] = tCamViewSel.tCamViewPool[0];
	tUI_SwitchBuMsg.ubAPP_Message[3] = tCamViewSel.tCamViewPool[1];
	tUI_SwitchBuMsg.ubAPP_Message[4] = tCamViewSel.tCamViewPool[2];
	UI_SendMessageToAPP(&tUI_SwitchBuMsg);
	RTC_WriteUserRam(RTC_RECORD_VIEW_MODE_ADDR, tCamViewSel.tCamViewType);	
	RTC_WriteUserRam(RTC_RECORD_VIEW_CAM_ADDR, (tCamViewSel.tCamViewPool[0] << 4) | tCamViewSel.tCamViewPool[1]);
}
//------------------------------------------------------------------------------
extern uint8_t demo_engmode;
void UI_EngModeKey(void)
{
	printf("1235");
	
	/*
	OSD_IMG_INFO tOsdImgInfo;
	
	if (1 == demo_engmode)
	{		
			EN_OpenEnMode(FALSE);
			UI_ClearBuConnectStatusFlag();
			tUI_State = UI_DISPLAY_STATE;	
		
		
			tOsdImgInfo.uwHSize  = 640;
			tOsdImgInfo.uwVSize  = 480;
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;		
			OSD_EraserImg2(&tOsdImgInfo);
			OSD_Weight(OSD_WEIGHT_8DIV8);

			demo_engmode = 0;
			return;
	}
	//else
	
	
	if (demo_menu || tUI_State != UI_ENGMODE_STATE || 0 == demo_engmode)  
	{
		demo_menu = 0;	
		
		tOsdImgInfo.uwHSize  = 640;
		tOsdImgInfo.uwVSize  = 480;
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 0;		
		OSD_EraserImg2(&tOsdImgInfo);
		OSD_Weight(OSD_WEIGHT_8DIV8);
	}	
	printf(">>¡¡UI_EngModeKey \n");
	
	
	
#ifdef A7130
	//OSD_IMG_INFO tOsdImgInfo;
	OSD_IMGIDXARRARY_t tUI_EngOsdImg;
	uint16_t uwUI_NumArray[10];
	uint16_t uwUI_UpperLetterArray[26];
	uint16_t uwUI_LowerLetterArray[26];
	uint16_t uwUI_SymbolArray[4];
	uint8_t i;

// 	if(UI_DISPLAY_STATE != tUI_State)
// 		return;
	
	
// #if ((LCD_PANEL != LCD_H5024A) && (LCD_PANEL != LCD_FTD50B7))
// 	tOsdImgInfo.uwXStart = 670;
// 	tOsdImgInfo.uwYStart = 980;
// 	tOsdImgInfo.uwHSize  = 50;
// 	tOsdImgInfo.uwVSize  = 300;
// #else
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	tOsdImgInfo.uwHSize  = 0;
	tOsdImgInfo.uwVSize  = 0;
// #endif
		
	OSD_EraserImg2(&tOsdImgInfo);
	
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;	//0
	tOsdImgInfo.uwHSize  = uwLCD_GetLcdHoSize();	//40
	tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
	OSD_EraserImg1(&tOsdImgInfo);
	
	for(i = 0; i < 10; i++)
		uwUI_NumArray[i] = OSD2IMG_ENG_D0 + i;		//OSD2IMG_ENG_D0
		
	tUI_EngOsdImg.pNumImgIdxArray = uwUI_NumArray;
	for(i = 0; i < 26; i++)
	{
		uwUI_UpperLetterArray[i] = OSD2IMG_ENG_UPA + i;
		uwUI_LowerLetterArray[i] = OSD2IMG_ENG_LWA + i;
	}
	
	tUI_EngOsdImg.pUpperLetterImgIdxArray = uwUI_UpperLetterArray;
	tUI_EngOsdImg.pLowerLetterImgIdxArray = uwUI_LowerLetterArray;
	for(i = 0; i < 4; i++)
		uwUI_SymbolArray[i] = OSD2IMG_ENG_COLONSYM + i;
	
	tUI_EngOsdImg.pSymbolImgIdxArray = uwUI_SymbolArray;
	
	printf(">>¡¡UI_EngModeKey---- \n");
	
	
	//EN_SetupOsdImgRotate(OSD_IMG_ROTATION_90);
	EN_SetupOsdImgInfo(&tUI_EngOsdImg);
	EN_OpenEnMode(TRUE);
	tUI_State = UI_ENGMODE_STATE;
	demo_engmode = 1;
	
#endif
*/
}
//------------------------------------------------------------------------------
void UI_EngModeCtrl(UI_ArrowKey_t tArrowKey)
{
#ifdef A7130
	pvUiFuncPtr UI_EngFuncPrt[] = {[UP_ARROW] 	 = EN_UpKey,
								   [DOWN_ARROW]	 = EN_DownKey,
								   NULL, NULL,
								   [ENTER_ARROW] = EN_EnterKey,
								   NULL};
	switch(tArrowKey)
	{
		case EXIT_ARROW:
		{
			EN_OpenEnMode(FALSE);
			UI_ClearBuConnectStatusFlag();
			tUI_State = UI_DISPLAY_STATE;
			break;
		}
		default:
			if(UI_EngFuncPrt[tArrowKey])
				UI_EngFuncPrt[tArrowKey]();
			break;
	}
#endif
}
//------------------------------------------------------------------------------
void UI_DisplayAppPairingScreen(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	ubUI_StopUpdateStsBarFlag = TRUE;
	tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo);
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_APPPAIRBG_ICON, 1, &tOsdImgInfo);
	tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
	tUI_State = UI_PAIRING_STATE;
}
//------------------------------------------------------------------------------
void UI_PairingControl(UI_ArrowKey_t tArrowKey)
{
	APP_EventMsg_t tUI_PairMessage = {0};

	tUI_PairMessage.ubAPP_Event = APP_PAIRING_STOP_EVENT;
	UI_SendMessageToAPP(&tUI_PairMessage);
}
//------------------------------------------------------------------------------
void UI_FwUpgViaSdCard(void)
{
	OSD_IMG_INFO tOsdImgInfo[6];

	if(UI_DISPLAY_STATE != tUI_State)
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FWUSDUPGDIAG_ICON, 5, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tOsdImgInfo[5].uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo[5].uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo[5].uwXStart = 0;
	tOsdImgInfo[5].uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo[5]);
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[4], OSD_UPDATE);
	tUI_State = UI_SDFWUPG_STATE;
}
//------------------------------------------------------------------------------
void UI_FwUpgExecSel(UI_ArrowKey_t tArrowKey)
{
	static uint8_t ubUI_FwuSelIdx = TRUE;
	uint8_t ubPrevFwuSelIdx = TRUE;
	uint16_t uwFwuSelImgIdx[2] = {OSD2IMG_FWUSDUPGNONOR_ICON, OSD2IMG_FWUSDUPGYESNOR_ICON};
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(TRUE == ubUI_FwuSelIdx)
				return;
			ubPrevFwuSelIdx = ubUI_FwuSelIdx;
			ubUI_FwuSelIdx = TRUE;
			break;
		case RIGHT_ARROW:
			if(FALSE == ubUI_FwuSelIdx)
				return;
			ubPrevFwuSelIdx = ubUI_FwuSelIdx;
			ubUI_FwuSelIdx = FALSE;
			break;
		case ENTER_ARROW:
			if(TRUE == ubUI_FwuSelIdx)
			{
				UI_Event_t tSdFwUpgEvent;
				osMessageQId *pUI_EventQH = NULL;

				tSdFwUpgEvent.tEventType = FWUPG_EVENT;
				tSdFwUpgEvent.pvEvent  	 = NULL;
				pUI_EventQH 	  	 	 = pUI_GetEventQueueHandle();
				osMessagePut(*pUI_EventQH, &tSdFwUpgEvent, 0);
				ubUI_FwuSelIdx = TRUE;
				tUI_State = UI_DISPLAY_STATE;
				return;
			}
		case EXIT_ARROW:
		{
			OSD_IMG_INFO tOsdImgInfo;

			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			UI_ClearStatusBarOsdIcon();
			UI_ClearBuConnectStatusFlag();
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
			tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdImgInfo);
			ubUI_FwuSelIdx = TRUE;
			tUI_State = UI_DISPLAY_STATE;
			return;
		}
		default:
			return;
	}
	UI_DrawHLandNormalIcon(uwFwuSelImgIdx[ubPrevFwuSelIdx], (uwFwuSelImgIdx[ubUI_FwuSelIdx]+UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_UpdateRecStsIcon(void)
{
	if((UI_SHOWSTSICON_STATE != tUI_State) ||
	   (!tUI_PuSetting.ubVdoRecStsCnt))
		return;
	if(!(--tUI_PuSetting.ubVdoRecStsCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		UI_ClearBuConnectStatusFlag();
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 0;
		tOsdImgInfo.uwHSize  = 190;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
		tUI_State = UI_DISPLAY_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateWarningNoteIcon(void)
{
	if((UI_SHOWSTSICON_STATE != tUI_State) ||
	   (!tUI_PuSetting.WarnIcon.ubWarnUpdateCnt))
		return;
	if(!(--tUI_PuSetting.WarnIcon.ubWarnUpdateCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		UI_ClearBuConnectStatusFlag();
		tOsdImgInfo.uwXStart = 160;
		tOsdImgInfo.uwYStart = 100;
		tOsdImgInfo.uwHSize  = 150;
		tOsdImgInfo.uwVSize  = 800;
		OSD_EraserImg2(&tOsdImgInfo);
		tUI_State = UI_DISPLAY_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_SdCardFormatMenu(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSdfSelImgIdx[2] = {OSD2IMG_SDFNONOR_ICON, OSD2IMG_SDFYESNOR_ICON};
	static uint8_t ubUI_SdfItmSel = FALSE;
	uint8_t ubPrevSdfSel;
	FS_FMT_STATUS tSDF_Ret = FORMAT_FAIL;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(TRUE == ubUI_SdfItmSel)
				return;
			ubPrevSdfSel = ubUI_SdfItmSel;
			ubUI_SdfItmSel = TRUE;
			break;
		case RIGHT_ARROW:
			if(FALSE == ubUI_SdfItmSel)
				return;
			ubPrevSdfSel = ubUI_SdfItmSel;
			ubUI_SdfItmSel = FALSE;
			break;
		case ENTER_ARROW:
			if(TRUE == ubUI_SdfItmSel)
            {
				OSD_IMG_INFO tSdfOsdImgInfo[3];
				UI_FuncExecMsg_t tWorkAct;
				uint8_t ubSdFmtRet;

				tWorkAct.uwFunc = UI_SDCARDFMT_ACT;
				osMessagePut(osUI_FuncsExecQue, &tWorkAct, 0);
				osMessageGet(osUI_FuncsFinExecQue, &ubSdFmtRet, osWaitForever);
				tSDF_Ret = (rUI_SUCCESS == ubSdFmtRet)?FORMAT_OK:FORMAT_FAIL;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFRET_BG, 3, &tSdfOsdImgInfo);
				tSdfOsdImgInfo[0].uwXStart = 260;
				tSdfOsdImgInfo[0].uwYStart = 473;
				tSdfOsdImgInfo[1+tSDF_Ret].uwXStart = 336;
				tSdfOsdImgInfo[1+tSDF_Ret].uwYStart = 499;
				tOSD_Img2(&tSdfOsdImgInfo[0], OSD_QUEUE);
				tOSD_Img2(&tSdfOsdImgInfo[1+tSDF_Ret], OSD_UPDATE);
				osDelay(1000);
            }
		case EXIT_ARROW:
			UI_ClearBuConnectStatusFlag();
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tOsdImgInfo.uwHSize  = 300;
			tOsdImgInfo.uwVSize  = 600;
			tOsdImgInfo.uwXStart = 220;
			tOsdImgInfo.uwYStart = 400;
			OSD_EraserImg2(&tOsdImgInfo);
			if(UI_REC_START == tUI_RecPlayAct.tRecAct)
			{
				tUI_RecPlayAct.tRecAct = UI_REC_STOP;
				if((TRUE == ubUI_SdfItmSel) && (FORMAT_OK == tSDF_Ret))
					UI_VideoRecordingExec(UI_REC_START);
			}
			ubUI_SdfItmSel = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSdfSelImgIdx[ubPrevSdfSel], 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 405;
	tOsdImgInfo.uwYStart = (TRUE == ubPrevSdfSel)?670:470;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSdfSelImgIdx[ubUI_SdfItmSel]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 405;
	tOsdImgInfo.uwYStart = (TRUE == ubUI_SdfItmSel)?670:470;
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_SdCardFormatFunc(UI_ArrowKey_t tArrowKey)
{
	switch(tUI_SdCardSts)
	{
		case UI_SD_NRDY:
			UI_SdCardFormatMenu(tArrowKey);
			break;
		case UI_SD_CFM:
			UI_RecordSDFSubSubMenu(tArrowKey);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SdCardFsFmtErrMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo[6];

	tUI_State = UI_SDCARDFMT_STATE;
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
	UI_ClearStatusBarOsdIcon();
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFBG_ICON, 6, &tOsdImgInfo);
	tOsdImgInfo[0].uwXStart = 230;
	tOsdImgInfo[0].uwYStart = 416;
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOsdImgInfo[2].uwXStart = 405;
	tOsdImgInfo[2].uwYStart = 470;
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOsdImgInfo[3].uwXStart = 405;
	tOsdImgInfo[3].uwYStart = 670;
	tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
	tUI_SdCardSts = UI_SD_NRDY;
}
//------------------------------------------------------------------------------
void UI_PhotoCaptureFinish(uint8_t ubPhotoCapRet)
{
	OSD_IMG_INFO tOsdImgInfo;
	KNL_Status_t tPhotoCapSts = (KNL_Status_t)ubPhotoCapRet;

	if(KNL_ErrorFsFmt == tPhotoCapSts)
	{
		UI_SdCardFsFmtErrMenu();
		return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_PHOTOCAPOK_ICON + tPhotoCapSts), 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_PuSetting.ubVdoRecStsCnt = UI_PHOTOGRAPHSTS_PERIOD;
	tUI_State = UI_SHOWSTSICON_STATE;
}
//------------------------------------------------------------------------------
void UI_PhotoPlayFinish(uint8_t ubPhtoCapRet)
{
	if(!ubPhtoCapRet)
	{
		OSD_IMG_INFO tOsdImgInfo;

		tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
		tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 0;
		OSD_EraserImg1(&tOsdImgInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PHOTOPLAY_ICON, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		tUI_State = UI_PHOTOPLAYLIST_STATE;
	}
	else
	{
		printd(DBG_ErrorLvl, "Play Err !\n");
		tUI_State = UI_RECFILES_SEL_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_VideoRecordingStsRpt(uint8_t ubRecSts)
{
	switch((KNL_Status_t)ubRecSts)
	{
		case KNL_ErrorNoCard:
        case KNL_ErrorTimeout:
		case KNL_ErrorCardNRdy:
		{
            uint8_t ubRecRpt = ubRecSts;
			OSD_IMG_INFO tOsdImgInfo;

			if(UI_DISPLAY_STATE != tUI_State)
				break;
			if((UI_SUBSUBMENU_STATE == tUI_State) || (UI_SUBMENU_STATE == tUI_State))
			{
				tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
				tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
				osDelay(100);
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PHOTOCAPNOCARD_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_PuSetting.ubVdoRecStsCnt = UI_RECGRAPHSTS_PERIOD;
			tUI_State = UI_SHOWSTSICON_STATE;
			if(UI_REC_START == tUI_RecPlayAct.tRecAct)
			{
                osMessagePut(osUI_RecRptQueue, &ubRecRpt, 0);
				UI_ClearRecordStatusOsdImg();
			}
			break;
		}
		case KNL_ErrorFsFmt:
			if((UI_SUBSUBMENU_STATE == tUI_State) || (UI_SUBMENU_STATE == tUI_State))
			{
				OSD_IMG_INFO tOsdImgInfo;

				tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
				tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
				osDelay(100);
			}
			UI_SdCardFsFmtErrMenu();
			break;
        case KNL_VDOREC_STOP_TIMEOUT: 
        {
			uint8_t ubRecRpt = ubRecSts;
            tUI_RecPlayAct.tRecAct = UI_REC_STOP;
			osMessagePut(osUI_RecRptQueue, &ubRecRpt, 0);
            break;
		}
		case KNL_VDOREC_STOP:
		{
			uint8_t ubRecRpt = ubRecSts;
            tUI_RecPlayAct.tRecAct = UI_REC_STOP;
			osMessagePut(osUI_RecRptQueue, &ubRecRpt, 0);
			break;
		}
		case KNL_VDOREC_START:
		{
			uint16_t uwLdState = UI_LD_DEFU;
			tUI_RecPlayAct.tRecAct = UI_REC_START;
			if(UI_RECORDING_MODE != tUI_PuSetting.tVdoMode)
			{
				ubUI_VdoRecChkFlag = FALSE;
				UI_VideoRecordingExec(UI_REC_STOP);
				break;
			}
			osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_VideoRecordingExec(UI_RecordingAct_t tRecAct)
{
	KNL_RecordAct_t tUI_RecAct = {KNL_RECORDFUNC_DISABLE, NULL,};
	KNL_Status_t tUI_RecSts;

	if((TRUE == ubUI_VdoRecChkFlag) &&
	   ((tUI_RecPlayAct.tRecAct == tRecAct) || (UI_PHOTOCAP_MODE == tUI_PuSetting.tVdoMode)))
		return;

	tUI_RecAct.tRecordFunc     = (REC_LOOPING == tUI_PuSetting.RecInfo.tREC_Mode)?KNL_RECORDFUNC_LOOP:KNL_RECORDFUNC_MANU;
	tUI_RecAct.pRecordStsNtyCb = UI_VideoRecordingStsRpt;
	tUI_RecSts = tKNL_ExecRecordFunc(tUI_RecAct);
	if(KNL_ERR != tUI_RecSts)
	{
        tUI_RecPlayAct.tRecAct = tRecAct;
		if(UI_REC_STOP == tUI_RecPlayAct.tRecAct)
		{
			uint8_t ubRecRpt;
            osMessageReset(osUI_RecRptQueue);
			osMessageGet(osUI_RecRptQueue, &ubRecRpt, osWaitForever);
		}
	}
	ubUI_VdoRecChkFlag = TRUE;
}
//------------------------------------------------------------------------------
#ifdef S2019A
void UI_SetSPRfWorkCh(void)
{
	OSD_IMG_INFO tChOsdImgInfo[11];
	uint8_t ubTen = 0, ubUnit = 0;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CK_NUM0, 11, &tChOsdImgInfo[0]);
	tChOsdImgInfo[10].uwXStart = 500;
	tChOsdImgInfo[10].uwYStart = uwOSD_GetVSize() - tChOsdImgInfo[10].uwVSize;
	tOSD_Img2(&tChOsdImgInfo[10], OSD_QUEUE);
	ubTen  = ubUI_sPRfChSel / 10;
	ubUnit = ubUI_sPRfChSel - (ubTen * 10);
	tChOsdImgInfo[ubTen].uwXStart = 500;
	tChOsdImgInfo[ubTen].uwYStart = tChOsdImgInfo[10].uwYStart - tChOsdImgInfo[10].uwVSize;
	tOSD_Img2(&tChOsdImgInfo[ubTen], OSD_QUEUE);
	tChOsdImgInfo[ubUnit].uwXStart = 500;
	tChOsdImgInfo[ubUnit].uwYStart = tChOsdImgInfo[ubTen].uwYStart - tChOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tChOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_State = UI_SPRF_SEL_STATE;
}
#endif	//! End of #ifdef S2019A
//------------------------------------------------------------------------------
void UI_sPRfChSelection(UI_ArrowKey_t tArrowKey)
{
#ifdef S2019A
	static uint8_t ubUI_RecordsPRfCh = 0;
	OSD_IMG_INFO tChOsdImgInfo[11];
	uint8_t ubTen = 0, ubUnit = 0;

	switch(tArrowKey)
	{
		case UP_ARROW:
			if((ubUI_sPRfChSel + 1) > 14)
				ubUI_sPRfChSel = 0;
			ubUI_RecordsPRfCh = ubUI_sPRfChSel;
			ubUI_sPRfChSel++;
			break;
		case DOWN_ARROW:
			if(!ubUI_sPRfChSel)
				ubUI_sPRfChSel = 15;
			ubUI_RecordsPRfCh = ubUI_sPRfChSel;
			ubUI_sPRfChSel--;
			break;
		case ENTER_ARROW:
			if((ubUI_RecordsPRfCh == ubUI_sPRfChSel) && (ubUI_sPRfChSel))
				return;
			osMutexWait(osUI_PerDbgMutex, osWaitForever);
			if(!ubUI_sPRfChSel)
			{
				sPRF_EnAutoSwCh(TRUE);
				sPRF_SetWorkCh(1);
			}
			else
			{
				sPRF_EnAutoSwCh(FALSE);
				sPRF_SetWorkCh(ubUI_sPRfChSel);
			}
			UI_ClearBuConnectStatusFlag();
			tChOsdImgInfo[10].uwHSize  = 40;
			tChOsdImgInfo[10].uwVSize  = uwOSD_GetVSize();
			tChOsdImgInfo[10].uwXStart = 500;
			tChOsdImgInfo[10].uwYStart = 0;
			OSD_EraserImg2(&tChOsdImgInfo[10]);
			tUI_State = UI_DISPLAY_STATE;
			osMutexRelease(osUI_PerDbgMutex);
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CK_NUM0, 10, &tChOsdImgInfo[0]);
	ubTen  = ubUI_sPRfChSel / 10;
	ubUnit = ubUI_sPRfChSel - (ubTen * 10);
	tChOsdImgInfo[ubTen].uwXStart = 500;
	tChOsdImgInfo[ubTen].uwYStart = uwOSD_GetVSize() - 40;
	tOSD_Img2(&tChOsdImgInfo[ubTen], OSD_QUEUE);
	tChOsdImgInfo[ubUnit].uwXStart = 500;
	tChOsdImgInfo[ubUnit].uwYStart = tChOsdImgInfo[ubTen].uwYStart - tChOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tChOsdImgInfo[ubUnit], OSD_UPDATE);
#endif
}
//------------------------------------------------------------------------------
//! Performance Debug
void UI_OsdDisplayFrmErrItem(UI_DisplayLocation_t tDispLoc)
{
#define DBGDISP_ICON_OFFSET 40
	uint16_t uwLcd_HSize  = uwOSD_GetHSize();
	uint16_t uwLcd_VSize  = uwOSD_GetVSize();
	uint16_t uwXOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
							  [DISP_LOWER_LEFT] = (uwLcd_HSize/2), [DISP_LOWER_RIGHT] = (uwLcd_HSize/2),
							  [DISP_LEFT] 	    = 0,			   [DISP_RIGHT]		  = 0,
							  [DISP_3T_TOP]     = 0,			   [DISP_3T_BOTTOM]	  = (uwLcd_HSize/2),
						      [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = 0,
	                          [DISP_3T_3CRIGHT] = 0};
	uint16_t uwYOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (uwLcd_VSize/2),
							  [DISP_LOWER_LEFT] = 0, 			   [DISP_LOWER_RIGHT] = (uwLcd_VSize/2),
							  [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT]		  = (uwLcd_VSize/2),
							  [DISP_3T_TOP] 	= 0,			   [DISP_3T_BOTTOM]	  = (uwLcd_VSize/2),
							  [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = (uwLcd_VSize/3),
	                          [DISP_3T_3CRIGHT] = ((uwLcd_VSize/3)*2)};
	uint16_t uwXStart = 0, uwYStart = 0;
	uint8_t ubDbgItem;

	tDispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_LEFT:tDispLoc;
	for(ubDbgItem = 0; ubDbgItem < MAX_DBG_ITEM; ubDbgItem++)
	{
		uwXStart = tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwXStart;
		uwYStart = tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwYStart;
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwXStart += uwXOffset[tDispLoc];
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwYStart -= uwYOffset[tDispLoc];
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[11+ubDbgItem], OSD_QUEUE);
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwXStart = uwXStart;
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwYStart = uwYStart;
	}
}
//------------------------------------------------------------------------------
void UI_OsdDisplayFrameSeqFunc(uint8_t ubCamNum, uint16_t uwFrameSeq, uint16_t uwXPos, uint16_t uwYPos)
{
	static uint8_t ubFrameSeq[CAM4 + 1][4]={{0xFF, 0xFF, 0xFF, 0xFF},
											{0xFF, 0xFF, 0xFF, 0xFF},
											{0xFF, 0xFF, 0xFF, 0xFF},
											{0xFF, 0xFF, 0xFF, 0xFF}};
	uint8_t ubTen = 0, ubUnit = 0, ubHun = 0, ubThus = 0;
	uint16_t uwXStart = 0, uwYStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
	{
		memset(ubFrameSeq, 0xFF, sizeof(ubFrameSeq));
		return;
	}
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	ubThus =  uwFrameSeq / 1000;
//	ubHun  = (uwFrameSeq - (ubThus * 1000)) / 100;
//	ubTen  = (uwFrameSeq - ((ubThus * 1000) + (ubHun * 100))) / 10;
//	ubUnit = (uwFrameSeq - ((ubThus * 1000) + (ubHun * 100) + (ubTen * 10)));
	ubHun  = (uwFrameSeq % 1000) / 100;
	ubTen  = (uwFrameSeq % 100) / 10;
	ubUnit = uwFrameSeq % 10;
	uwXStart = 60 + uwXPos;	
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[ubThus].uwVSize;
	if (ubThus != ubFrameSeq[ubCamNum][3])
	{
		tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubThus], OSD_QUEUE);
		ubFrameSeq[ubCamNum][3] = ubThus;
	}
	uwYStart = tUI_PerDbgOsdImgInfo[ubThus].uwYStart;
//	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = 0;
//	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubThus].uwVSize;
	if (ubHun != ubFrameSeq[ubCamNum][2])
	{
		tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
		ubFrameSeq[ubCamNum][2] = ubHun;
	}
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
//	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = 0;
//	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
	if (ubTen != ubFrameSeq[ubCamNum][1])
	{
		tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
		ubFrameSeq[ubCamNum][1] = ubTen;
	}
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
//	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
//	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;	
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
//	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = 0;
//	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 0;
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_OsdDisplayFpsFunc(uint8_t ubCamNum, uint16_t uwFps, uint16_t uwXPos, uint16_t uwYPos)
{
	static uint8_t ubFps[CAM4 + 1][2]={{0xFF, 0xFF},
									   {0xFF, 0xFF},
									   {0xFF, 0xFF},
									   {0xFF, 0xFF}};
	uint8_t ubTen = 0, ubUnit = 0;
	uint16_t uwXStart = 0, uwYStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
	{
		memset(ubFps, 0xFF, sizeof(ubFps));	
		return;
	}
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	uwXStart = 60 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[10].uwVSize - 240);
	if (0xFF == ubFps[ubCamNum][1])
	{
		tUI_PerDbgOsdImgInfo[10].uwXStart = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	}
	uwYStart = tUI_PerDbgOsdImgInfo[10].uwYStart;
//	tUI_PerDbgOsdImgInfo[10].uwXStart = 0;
//	tUI_PerDbgOsdImgInfo[10].uwYStart = 0;
	ubTen  = uwFps / 10;
	ubUnit = uwFps % 10;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	if (ubTen != ubFps[ubCamNum][1])
	{
		tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	}
	if (ubUnit != ubFps[ubCamNum][0])
	{
		uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	//	tUI_PerDbgOsdImgInfo[ubTen].uwXStart = 0;
	//	tUI_PerDbgOsdImgInfo[ubTen].uwYStart = 0;
		tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
		tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	//	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = 0;
	//	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 0;
		ubFps[ubCamNum][0] = ubUnit;
		ubFps[ubCamNum][1] = ubTen;
	}
	else if (ubTen != ubFps[ubCamNum][1])
	{
		OSD_UpdateQueueBuf();
		ubFps[ubCamNum][1] = ubTen;
	}
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_OsdDisplayRfBwFunc(uint8_t ubCamNum, uint16_t uwBw, uint16_t uwXPos, uint16_t uwYPos)
{
	uint8_t ubHun = 0, ubTen = 0, ubUnit = 0;
	uint16_t uwXStart = 0, uwYStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	uwXStart = 60 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[10].uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[10].uwVSize - 135);
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[10].uwYStart;
	tUI_PerDbgOsdImgInfo[10].uwXStart = 0;
	tUI_PerDbgOsdImgInfo[10].uwYStart = 0;
	ubHun  = uwBw / 100;
	ubTen  = (uwBw - (ubHun * 100)) / 10;
	ubUnit = uwBw - ((ubHun * 100) + (ubTen * 10));
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 0;
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_OsdDisplayFrameErrType(uint8_t ubCamNum, uint16_t uwErrorType, uint16_t uwXPos, uint16_t uwYPos)
{
	uint8_t ubTen = 0, ubUnit = 0, ubHun = 0, ubThus = 0;
	uint16_t uwXStart = 0 ,uwYStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	ubThus = ulUI_FrameErrCnt[ubCamNum][uwErrorType] / 1000;
	ubHun  = (ulUI_FrameErrCnt[ubCamNum][uwErrorType] - (ubThus * 1000)) / 100;
	ubTen  = (ulUI_FrameErrCnt[ubCamNum][uwErrorType] - ((ubThus * 1000) + (ubHun * 100))) / 10;
	ubUnit = (ulUI_FrameErrCnt[ubCamNum][uwErrorType] - ((ubThus * 1000) + (ubHun * 100) + (ubTen * 10)));
	uwXStart = 105 + (uwErrorType * DBGDISP_ICON_OFFSET) + uwXPos;
	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[ubThus].uwVSize - 40;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubThus], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubThus].uwYStart;
	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart   = uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart   = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart   = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart   = uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart   = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart   = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart   = 0;
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_OsdDisplaySPRfStatus(uint8_t ubCamNum, uint16_t uwValue, uint16_t uwXPos, uint16_t uwYPos)
{
#ifdef S2019A
	uint8_t ubHun = 0, ubTen = 0, ubUnit = 0;
	uint8_t ubCh = 0, ubRssi = 0;
	uint16_t uwXStart = 0, uwYStart = 0;
	OSD_IMG_INFO tUI_sPRfDbgOsdImgInfo;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SPRFCHDBGITEM_ICON, 1, &tUI_sPRfDbgOsdImgInfo);
	//! CH
	ubCh = ubsPRF_GetWorkCh();
	uwXStart = 60 + uwXPos;
	tUI_sPRfDbgOsdImgInfo.uwXStart = uwXStart;
	tUI_sPRfDbgOsdImgInfo.uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_sPRfDbgOsdImgInfo.uwVSize - 135);
	tOSD_Img2(&tUI_sPRfDbgOsdImgInfo, OSD_QUEUE);
	uwYStart = tUI_sPRfDbgOsdImgInfo.uwYStart;
	ubTen  = ubCh / 10;
	ubUnit = ubCh - (ubTen * 10);
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 0;
	//! RSSI
	ubRssi = tsPRF_GetRssi();
	uwXStart = 100 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[10].uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[10].uwVSize - 240);
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[10].uwYStart;
	tUI_PerDbgOsdImgInfo[10].uwXStart = 0;
	tUI_PerDbgOsdImgInfo[10].uwYStart = 0;
	ubHun  = ubRssi / 100;
	ubTen  = (ubRssi - (ubHun * 100)) / 10;
	ubUnit = ubRssi - ((ubHun * 100) + (ubTen * 10));
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 0;
	osMutexRelease(osUI_PerDbgMutex);
#endif
}
//------------------------------------------------------------------------------
void UI_DisplayTrxInfo(UI_FuncExecMsg_t tPerRpt)
{
	static void (*pvUI_PerRptFunc[])(uint8_t, uint16_t, uint16_t, uint16_t) =
	{
		[PER_FRMSTS_RPT] = UI_OsdDisplayFrameErrType,
		[PER_FRMSEQ_RPT] = UI_OsdDisplayFrameSeqFunc,
		[PER_FPS_RPT] 	 = UI_OsdDisplayFpsFunc,
		[PER_RFBW_RPT] 	 = UI_OsdDisplayRfBwFunc,
		[PER_SPRFSTS_RPT] = UI_OsdDisplaySPRfStatus,
	};
	uint16_t uwLcd_HSize  = uwOSD_GetHSize();
	uint16_t uwLcd_VSize  = uwOSD_GetVSize();
	uint16_t uwXOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
							  [DISP_LOWER_LEFT] = (uwLcd_HSize/2), [DISP_LOWER_RIGHT] = (uwLcd_HSize/2),
							  [DISP_LEFT] 	    = 0,			   [DISP_RIGHT]		  = 0,
							  [DISP_3T_TOP]		= 0,			   [DISP_3T_BOTTOM]	  = (uwLcd_HSize/2),
							  [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = 0,
	                          [DISP_3T_3CRIGHT] = 0};
	uint16_t uwYOffset[12] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (uwLcd_VSize/2),
							  [DISP_LOWER_LEFT] = 0, 			   [DISP_LOWER_RIGHT] = (uwLcd_VSize/2),
							  [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT]		  = (uwLcd_VSize/2),
						      [DISP_3T_TOP]	    = 0,			   [DISP_3T_BOTTOM]	  = (uwLcd_VSize/2),
						      [DISP_3T_3CLEFT]  = 0, 		       [DISP_3T_3CMID]	  = (uwLcd_VSize/3),
	                          [DISP_3T_3CRIGHT] = ((uwLcd_VSize/3)*2)};
	UI_DisplayLocation_t tUI_DispLoc;

	if((SINGLE_VIEW == tCamViewSel.tCamViewType) && (((UI_CamNum_t)tPerRpt.ubCamNum != tCamViewSel.tCamViewPool[0])))
		return;
	if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (IS_UI_DISP3T_VIEW(tCamViewSel.tCamViewType)))
		tUI_DispLoc = tUI_GetTripViewDispLoc((UI_CamNum_t)tPerRpt.ubCamNum);
	else if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (tCamViewSel.tCamViewType == DUAL_VIEW))
		tUI_DispLoc = (tPerRpt.ubCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT;
	else
		tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_LEFT:tUI_CamStatus[tPerRpt.ubCamNum].tCamDispLocation;
	if(pvUI_PerRptFunc[tPerRpt.ubType])
		pvUI_PerRptFunc[tPerRpt.ubType](tPerRpt.ubCamNum, tPerRpt.uwFuncMsg, uwXOffset[tUI_DispLoc], uwYOffset[tUI_DispLoc]);
}
//------------------------------------------------------------------------------
void UI_PerReportFunc(uint8_t ubRptType, uint8_t ubCamNum, uint16_t uwValue)
{
	UI_FuncExecMsg_t tWorkAct;

	if(INVALID_ID == tUI_CamStatus[ubCamNum].ulCAM_ID || ubUI_SwtichViewTypeFg == 1)
		return;
	switch(ubRptType)
	{
		case PER_FRMSTS_RPT:
		case PER_FRMSEQ_RPT:
		case PER_FPS_RPT:
		case PER_RFBW_RPT:
		case PER_SPRFSTS_RPT:
			if(PER_FRMSTS_RPT == ubRptType)
			{
				osMutexWait(osUI_PerDbgMutex, osWaitForever);
				++ulUI_FrameErrCnt[ubCamNum][uwValue];
				osMutexRelease(osUI_PerDbgMutex);
			}
			if((SINGLE_VIEW == tCamViewSel.tCamViewType) && (((UI_CamNum_t)ubCamNum != tCamViewSel.tCamViewPool[0])))
				return;
			tWorkAct.uwFunc 	= UI_PERDBGRPT_ACT;
			tWorkAct.ubCamNum 	= ubCamNum;
			tWorkAct.ubType 	= ubRptType;
			tWorkAct.uwFuncMsg  = uwValue;
			osMessagePut(osUI_FuncsExecQue, &tWorkAct, 0);
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_EnPerDebugMode(void)
{
	static uint8_t ubUI_LdPerDbgOsdImgFlag = FALSE;
	UI_CamNum_t tCamNum;
	UI_PUReqCmd_t tPerDbgCmd;
	UI_Result_t tPerDbgNotifyRet = rUI_SUCCESS;
	uint8_t ubCnt;

	if((APP_LOSTLINK_STATE == tUI_SyncAppState) ||
	   (UI_DISPLAY_STATE != tUI_State))
		return;
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		for(ubCnt = 0; ubCnt < MAX_DBG_ITEM; ubCnt++)
			ulUI_FrameErrCnt[tCamNum][ubCnt] = 0;
	}
	if(FALSE == ubUI_LdPerDbgOsdImgFlag)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CK_NUM0,    11, &tUI_PerDbgOsdImgInfo[0]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRXERR1TYPE, 6, &tUI_PerDbgOsdImgInfo[11]);
		ubUI_LdPerDbgOsdImgFlag = TRUE;
	}
	ubUI_PerDebugEn	= !ubUI_PerDebugEn;
	KNL_EnPerDebugMode(((ubUI_PerDebugEn)?KNL_PERDBG_ON:KNL_PERDBG_OFF), UI_PerReportFunc);
	if(FALSE == ubUI_PerDebugEn)
	{
		OSD_IMG_INFO tOsdImgInfo;

		tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
		tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 0;
		OSD_EraserImg1(&tOsdImgInfo);
		tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
		UI_ClearBuConnectStatusFlag();
	}
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		if((INVALID_ID == tUI_CamStatus[tCamNum].ulCAM_ID) ||
		   (CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts))
			continue;
		tPerDbgCmd.tDS_CamNum 				= tCamNum;
		tPerDbgCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPerDbgCmd.ubCmd[UI_SETTING_ITEM]   = UI_PERDBGMODE_SETTING;
		tPerDbgCmd.ubCmd[UI_SETTING_DATA]   = ubUI_PerDebugEn;
		tPerDbgCmd.ubCmd_Len  				= 3;
		tPerDbgNotifyRet = UI_SendRequestToBU(osThreadGetId(), &tPerDbgCmd);
		if(rUI_SUCCESS != tPerDbgNotifyRet)
			printd(DBG_ErrorLvl, "CAM%d:Per Debug Notify Fail !\n", (tCamNum + 1));
	}
}
//------------------------------------------------------------------------------
void UI_LoadDrvStatusInfo(void)
{
#ifdef S2019A
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (DRV_MD_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DevDrvStsInfo_t tDrvStsInfo = {sPRF_TRX_MODE};
	UI_CamNum_t tCamNum;

	SF_Read(ulUI_SFAddr, sizeof(UI_DevDrvStsInfo_t), (uint8_t *)&tDrvStsInfo);
	if(tDrvStsInfo.tUI_DrvMd > sPRF_BRIDGE_MODE)
		tDrvStsInfo.tUI_DrvMd = sPRF_TRX_MODE;
	KNL_SetDrvMode(tDrvStsInfo.tUI_DrvMd);
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		tUI_DrvStsInfo.ubDtRdy[tCamNum]  = 2;
		tUI_DrvStsInfo.ubDtLink[tCamNum] = 2;
	}
#endif
}
//------------------------------------------------------------------------------
void UI_DriveMdSelection(UI_ArrowKey_t tArrowKey)
{
#ifdef S2019A
	static uint8_t ubUI_DrvMdStpEn = FALSE;
	OSD_IMG_INFO tOsdImgInfo;
	sPRF_DrvMode_t tCurDrvMd = sPRF_TRX_MODE, tDrvSel = sPRF_TRX_MODE;
	UI_CamNum_t tCamNum;

	switch(tArrowKey)
	{
		case ENTER_ARROW:
			if(TRUE == ubUI_DrvMdStpEn)
				break;
			tCurDrvMd = tsPRF_GetDrvMode();
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_WIFIDTOFFHL_ITEM + tCurDrvMd), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubUI_DrvMdStpEn = TRUE;
			break;
		case LEFT_ARROW:
		case RIGHT_ARROW:
			if(FALSE == ubUI_DrvMdStpEn)
				break;
			tCurDrvMd = tsPRF_GetDrvMode();
			if(sPRF_TRX_MODE == tCurDrvMd)
			{
				if(LEFT_ARROW == tArrowKey)
					break;
			}
			else if(sPRF_APDIRECT_MODE == tCurDrvMd)
			{
				if(RIGHT_ARROW == tArrowKey)
					break;
			}
			tDrvSel = (LEFT_ARROW == tArrowKey)?sPRF_TRX_MODE:sPRF_APDIRECT_MODE;
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_WIFIDTOFFHL_ITEM + tDrvSel), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			if(sPRF_APDIRECT_MODE == tCurDrvMd)
			{
				tLCD_JpegDecodeDisable();
				OSD_LogoJpeg(OSDLOGO_WIFIDT_STP);
				UI_ClearOsdImage();
				sPRF_SetDrvMode(tDrvSel);
			}
			else
			{
				sPRF_SetDrvMode(tDrvSel);
				tLCD_JpegDecodeDisable();
				OSD_LogoJpeg(OSDLOGO_WIFIDT_STP);
			}
		case EXIT_ARROW:
			if(EXIT_ARROW != tArrowKey)
			{
				UI_ClearBuConnectStatusFlag();
				UI_ClearOsdImage();
				tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
				if(sPRF_APDIRECT_MODE == tDrvSel)
				{
					osDelay(600);
					tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_WIFIDTENY_BG, 1, &tOsdImgInfo);
					tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
					for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_WIFIDTCAM1NRDY_ICON + (tCamNum * 2)), 1, &tOsdImgInfo);
						tOSD_Img2(&tOsdImgInfo, ((tUI_PuSetting.ubTotalBuNum == (tCamNum+1))?OSD_UPDATE:OSD_QUEUE));
					}
				}
				UI_UpdateDrvStatusInfo();
			}
			else
			{
				if(sPRF_APDIRECT_MODE == tsPRF_GetDrvMode())
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_WIFIDTENY_BG, 1, &tOsdImgInfo);
					tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
					for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_WIFIDTCAM1NRDY_ICON + (tCamNum * 2)), 1, &tOsdImgInfo);
						tOSD_Img2(&tOsdImgInfo, ((tUI_PuSetting.ubTotalBuNum == (tCamNum+1))?OSD_UPDATE:OSD_QUEUE));
					}
				}
				else
				{
					UI_ClearBuConnectStatusFlag();
					UI_ClearOsdImage();
					tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
				}
			}
			for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
			{
				tUI_DrvStsInfo.ubDtRdy[tCamNum]  = 2;
				tUI_DrvStsInfo.ubDtLink[tCamNum] = 2;
			}
			tUI_State = UI_DISPLAY_STATE;
			ubUI_DrvMdStpEn = FALSE;
			break;
		default:
			break;
	}
#endif
}
//------------------------------------------------------------------------------
void UI_SwitchDrvMd(void)
{
#ifdef S2019A
	OSD_IMG_INFO tOsdImgInfo[2];
	sPRF_DrvMode_t tUI_sPRFdrv = sPRF_TRX_MODE;

	if(APP_LOSTLINK_STATE == tUI_SyncAppState)
		return;
	tUI_sPRFdrv = tsPRF_GetDrvMode();
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_WIFIDTBG_ICON, 1, &tOsdImgInfo[0]);
	tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_WIFIDTOFFNOR_ITEM + tUI_sPRFdrv), 1, &tOsdImgInfo[1]);
	tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
	tUI_State = UI_DT_SEL_STATE;
#endif
}
//------------------------------------------------------------------------------
void UI_UpdateDrvStatusInfo(void)
{
#ifdef S2019A
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (DRV_MD_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DevDrvStsInfo_t tDrvStsInfo = {sPRF_TRX_MODE};

	tDrvStsInfo.tUI_DrvMd = tsPRF_GetDrvMode();
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulUI_SFAddr, pSF_Info->ulSecSize, 1);
	SF_Write(ulUI_SFAddr, sizeof(UI_DevDrvStsInfo_t), (uint8_t *)&tDrvStsInfo);
	SF_EnableWrProtect();
#endif
}
void UI_PairingDirectlyCAM1(void)
{
	APP_EventMsg_t tUI_PairMessage = {0};
	tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
	tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam = CAM1;
	tUI_PairMessage.ubAPP_Message[2] = tPairInfo.tDispLocation = DISP_LEFT;
	tUI_PairMessage.ubAPP_Message[3] = FALSE;
	UI_SendMessageToAPP(&tUI_PairMessage);
	tPairInfo.ubDrawFlag 			 = TRUE;
	UI_DisableScanMode();
	tUI_State = UI_PAIRING_STATE;
}
void UI_PairingDirectlyCAM2(void)
{
	APP_EventMsg_t tUI_PairMessage = {0};
	tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
	tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam = CAM2;
	tUI_PairMessage.ubAPP_Message[2] = tPairInfo.tDispLocation = DISP_RIGHT;
	tUI_PairMessage.ubAPP_Message[3] = FALSE;
	UI_SendMessageToAPP(&tUI_PairMessage);
	tPairInfo.ubDrawFlag 			 = TRUE;
	UI_DisableScanMode();
	tUI_State = UI_PAIRING_STATE;
}
//------------------------------------------------------------------------------
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
void UI_DrawTXFolderMenu(void)
{
	OSD_IMG_INFO tRecBgOsdImgInfo, tRecOsdImgInfo;
    UI_CamNum_t tRecCamNum;
    uint8_t ubStatus;
	if(tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecBgOsdImgInfo) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecOsdImgInfo) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tUI_State = UI_RECTXFOLDER_SEL_STATE;
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tRecBgOsdImgInfo.uwXStart = 0;
	tRecBgOsdImgInfo.uwYStart = 0;
	tOSD_Img1(&tRecBgOsdImgInfo, OSD_QUEUE);
	tOSD_Img2(&tRecOsdImgInfo, OSD_UPDATE);
	
    APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecCamNum);

    if( tCamViewSel.tCamViewType == SCAN_VIEW ||
        (tCamViewSel.tCamViewType == SINGLE_VIEW && tRecCamNum != tCamViewSel.tCamViewPool[0])||
        (tCamViewSel.tCamViewType == DUAL_VIEW && (tRecCamNum != tCamViewSel.tCamViewPool[0] && tRecCamNum != tCamViewSel.tCamViewPool[1] )))
    {
        ubUI_TXGrayoutFg = 1;
        UI_SetRecImgColor(UI_RECIMG_COLOR2);
        OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Selected Camara DisConnect");
        return;
    }
    
    ubStatus = UI_TXFsDrawEvent(tRecCamNum,UI_FS_ENTER,UI_FS_DCIM_FLD,SORT_BY_NAME_DESCENDING,0);
    if(ubStatus==0)
    {
        if(pUI_RecTXFoldersInfo.uwTotalRecFolderNum)
    	    UI_ListDCIMFolderInfo(&pUI_RecTXFoldersInfo,0, ((pUI_RecTXFoldersInfo.uwTotalRecFolderNum < REC_FOLDER_LIST_MAXNUM)?pUI_RecTXFoldersInfo.uwTotalRecFolderNum:REC_FOLDER_LIST_MAXNUM), OSD_UPDATE);
    }
    else if (ubStatus == 1)
    {
        ubUI_TXGrayoutFg = 1;
        UI_SetRecImgColor(UI_RECIMG_COLOR2);
        OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Please Check Remote SD");
    }
    else if (ubStatus == 2)
    {
        ubUI_TXGrayoutFg = 1;
        UI_SetRecImgColor(UI_RECIMG_COLOR2);
        OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Enter Folder List Timeout");    
    }
    else if (ubStatus == 3)
    {
        ubUI_TXGrayoutFg = 1;
        UI_SetRecImgColor(UI_RECIMG_COLOR2);
        OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Data Channel Not Ready");
    }
}

void UI_DCIMTXFolderSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tFolderSelOsdImgInfo[2], tOsdImgInfo;
	uint8_t ubUI_RecFolderIdx = 0, ubUI_PrevRecFolderIdx = 0;
	uint8_t ubRefreshFldIconFlag = FALSE, ubRefreshFldIdx, ubFldChkIdx;
	uint16_t uwRecSelIdx;
    UI_CamNum_t tRecCamNum;

	if((!pUI_RecTXFoldersInfo.uwTotalRecFolderNum) && (EXIT_ARROW != tArrowKey))
		return;
	if(ubUI_TXGrayoutFg ==1 && EXIT_ARROW != tArrowKey)
	    return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_FOLDERCLOSE_ICON, 2, &tFolderSelOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwRecSelIdx = pUI_RecTXFoldersInfo.uwRecFolderSelIdx;
	ubUI_RecFolderIdx = (uwRecSelIdx % REC_FOLDER_LIST_MAXNUM);
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(!(ubUI_RecFolderIdx % 5))
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx     = (ubUI_PrevRecFolderIdx - 1);
			--pUI_RecTXFoldersInfo.uwRecFolderSelIdx;
			break;
		case RIGHT_ARROW:
			if((((ubUI_RecFolderIdx % 5) + 1) >= 5) || ((uwRecSelIdx + 1) >= pUI_RecTXFoldersInfo.uwTotalRecFolderNum)) 
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx     = (ubUI_PrevRecFolderIdx + 1);
			++pUI_RecTXFoldersInfo.uwRecFolderSelIdx;
			break;
		case UP_ARROW:
			if(pUI_RecTXFoldersInfo.uwRecFolderSelIdx < 5)
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx    -= 5;
			ubRefreshFldIdx = (pUI_RecTXFoldersInfo.uwRecFolderSelIdx > REC_FOLDER_LIST_MAXNUM)?((pUI_RecTXFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM) >= 5)?0:(pUI_RecTXFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM):0;
			ubRefreshFldIconFlag  = (!((pUI_RecTXFoldersInfo.uwRecFolderSelIdx - ubRefreshFldIdx) % REC_FOLDER_LIST_MAXNUM))?TRUE:FALSE;
			pUI_RecTXFoldersInfo.uwRecFolderSelIdx -= 5;
			break;
		case DOWN_ARROW:
			ubFldChkIdx = (ubUI_RecFolderIdx / 5)?(uwRecSelIdx - (uwRecSelIdx % 5) + 5):(uwRecSelIdx + 5);
        	if(ubFldChkIdx >= pUI_RecTXFoldersInfo.uwTotalRecFolderNum)
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx    += 5;
			pUI_RecTXFoldersInfo.uwRecFolderSelIdx = ((pUI_RecTXFoldersInfo.uwRecFolderSelIdx + 5) >= pUI_RecTXFoldersInfo.uwTotalRecFolderNum)?
													 (((pUI_RecTXFoldersInfo.uwRecFolderSelIdx + 5) / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM):(pUI_RecTXFoldersInfo.uwRecFolderSelIdx + 5);
			ubRefreshFldIdx = (pUI_RecTXFoldersInfo.uwRecFolderSelIdx > REC_FOLDER_LIST_MAXNUM)?((pUI_RecTXFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM) >= 5)?0:(pUI_RecTXFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM):0;
			ubRefreshFldIconFlag  = (!((pUI_RecTXFoldersInfo.uwRecFolderSelIdx - ubRefreshFldIdx) % REC_FOLDER_LIST_MAXNUM))?TRUE:FALSE;
			break;
		case ENTER_ARROW:
			tOsdImgInfo.uwHSize  = 600;
			tOsdImgInfo.uwVSize  = 955;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
            UI_DrawTXRecordFileMenu();
			return;
		case EXIT_ARROW:
    		ubUI_TXGrayoutFg=0;
		    if(ubUI_RecSubMenuFlag == TRUE)
		        ubUI_RecSubMenuTXFldFlag = FALSE;
            APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecCamNum);
            UI_TXFsDrawEvent(tRecCamNum,UI_FS_EXIT,UI_FS_DCIM_FLD,SORT_BY_NAME_DESCENDING,pUI_RecTXFoldersInfo.uwRecFolderSelIdx);
			tOsdImgInfo.uwHSize  = 600;
			tOsdImgInfo.uwVSize  = 955;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);            
			pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
			pUI_RecFilesInfo.uwRecFileSelIdx = 0;
			UI_DrawDCIMFolderMenu();
			return;
		default:
			return;
	}
	if(TRUE == ubRefreshFldIconFlag)
	{
		uint16_t uwFldStartIdx = 0, uwFldEndIdx = 0;

		tOsdImgInfo.uwHSize  = 600;
		tOsdImgInfo.uwVSize  = 955;
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 50;
		OSD_EraserImg2(&tOsdImgInfo);
		uwFldStartIdx = (pUI_RecTXFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;
		if(DOWN_ARROW == tArrowKey)
		{
			uwFldEndIdx = pUI_RecTXFoldersInfo.uwTotalRecFolderNum - pUI_RecTXFoldersInfo.uwRecFolderSelIdx;
			uwFldEndIdx = (uwFldEndIdx < REC_FOLDER_LIST_MAXNUM)?pUI_RecTXFoldersInfo.uwTotalRecFolderNum:(uwFldStartIdx + REC_FOLDER_LIST_MAXNUM);
		}
		else
		{
			uwFldEndIdx = uwFldStartIdx + REC_FOLDER_LIST_MAXNUM;
			ubRefreshFldIdx = pUI_RecTXFoldersInfo.uwRecFolderSelIdx;			
		}
		if(ubRefreshFldIdx)
		{
			ubUI_PrevRecFolderIdx = 0;
			ubUI_RecFolderIdx 	  = ubRefreshFldIdx % REC_FOLDER_LIST_MAXNUM;
			ubRefreshFldIconFlag  = FALSE;
		}
		UI_ListDCIMFolderInfo(&pUI_RecTXFoldersInfo,uwFldStartIdx, uwFldEndIdx, (ubRefreshFldIdx)?OSD_QUEUE:OSD_UPDATE);
	}
	if(FALSE == ubRefreshFldIconFlag)
	{
		uwRecSelIdx = (pUI_RecTXFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;
		UI_SetRecImgColor(UI_RECIMG_COLOR5);
		tFolderSelOsdImgInfo[0].uwYStart -= ((ubUI_PrevRecFolderIdx % 5) * 185);
		tFolderSelOsdImgInfo[0].uwXStart += ((ubUI_PrevRecFolderIdx / 5) * 265);
		tOSD_Img2(&tFolderSelOsdImgInfo[0], OSD_QUEUE);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubUI_PrevRecFolderIdx % 5) * 185)), (263 + ((ubUI_PrevRecFolderIdx / 5) * 265)),
						tUI_RecOsdImgInfo, OSD_QUEUE, pUI_RecTXFoldersInfo.tRecFolderInfo[ubUI_PrevRecFolderIdx + uwRecSelIdx].FldName.chName);
		UI_SetRecImgColor(UI_RECIMG_COLOR6);
		tFolderSelOsdImgInfo[1].uwYStart -= ((ubUI_RecFolderIdx % 5) * 185);
		tFolderSelOsdImgInfo[1].uwXStart += ((ubUI_RecFolderIdx / 5) * 265);
		tOSD_Img2(&tFolderSelOsdImgInfo[1], OSD_QUEUE);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubUI_RecFolderIdx % 5) * 185)), (263 + ((ubUI_RecFolderIdx / 5) * 265)),
						tUI_RecOsdImgInfo, OSD_UPDATE, pUI_RecTXFoldersInfo.tRecFolderInfo[ubUI_RecFolderIdx + uwRecSelIdx].FldName.chName);
	}
}

void UI_DrawTXRecordFileMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo[9];
	uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
	uint8_t ubIdx;
    UI_CamNum_t tRecCamNum;
    uint8_t ubStatus;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_CAM1, 9, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}

    APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecCamNum);
    ubStatus = UI_TXFsDrawEvent(tRecCamNum,UI_FS_ENTER,UI_FS_REC_FLD,SORT_BY_NAME_DESCENDING,pUI_RecTXFoldersInfo.uwRecFolderSelIdx);
    if( ubStatus == 0)
    {
        for(ubIdx = 0; ubIdx < 8; ubIdx++)
    		tOSD_Img2(&tOsdImgInfo[ubIdx], ((!pUI_RecTXFilesInfo.uwTotalRecFileNum) && (ubIdx == 7))?OSD_UPDATE:OSD_QUEUE);
    	if(pUI_RecTXFilesInfo.uwTotalRecFileNum)
    	{
    		uwFileStartIdx = (pUI_RecTXFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
    		uwFileEndIdx   = pUI_RecTXFilesInfo.uwTotalRecFileNum - uwFileStartIdx;
    		uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?pUI_RecTXFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
    		tOsdImgInfo[8].uwXStart += ((pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
    		tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
    		UI_ListRecFileInfo(&pUI_RecTXFilesInfo,uwFileStartIdx, uwFileEndIdx, OSD_UPDATE);
    	}
    }
    else if (ubStatus == 1)
    {
        UI_SetRecImgColor(UI_RECIMG_COLOR2);
        OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Please Check Remote SD");
    }
    else if (ubStatus == 2)
    {
        UI_SetRecImgColor(UI_RECIMG_COLOR2);
        OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Enter Folder List Timeout");
    }
    else if (ubStatus == 3)
    {
        UI_SetRecImgColor(UI_RECIMG_COLOR2);
        OSD_ImagePrintf(OSD_IMG_ROTATION_90, 300, 370, tUI_RecOsdImgInfo, OSD_UPDATE, "Data Channel Not Ready");
    }
    tUI_State = UI_RECTXFILES_SEL_STATE;
}

void UI_ReDrawTXRecordFileMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo[9];
	uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
	uint8_t ubIdx;    
    if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_CAM1, 9, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}    
    for(ubIdx = 0; ubIdx < 8; ubIdx++)
		tOSD_Img2(&tOsdImgInfo[ubIdx], ((!pUI_RecTXFilesInfo.uwTotalRecFileNum) && (ubIdx == 7))?OSD_UPDATE:OSD_QUEUE);
	if(pUI_RecTXFilesInfo.uwTotalRecFileNum)
	{
		uwFileStartIdx = (pUI_RecTXFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
		uwFileEndIdx   = pUI_RecTXFilesInfo.uwTotalRecFileNum - uwFileStartIdx;
		uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?pUI_RecTXFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
		tOsdImgInfo[8].uwXStart += ((pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
		tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
		UI_ListRecFileInfo(&pUI_RecTXFilesInfo,uwFileStartIdx, uwFileEndIdx, OSD_UPDATE);
	}
    tUI_State = UI_RECTXFILES_SEL_STATE;
}

void UI_RecordTXFileSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tFileSelOsdImgInfo[3];
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t ubUI_RecFileIdx, ubUI_PrevRecFileIdx = 0;
	static uint8_t ubUI_RecFileDelFlag = FALSE;
	uint8_t ubUI_RecFileDelRdy = FALSE;

	if((!pUI_RecTXFilesInfo.uwTotalRecFileNum) && (EXIT_ARROW != tArrowKey))
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_FILESELECT, 3, &tFileSelOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	switch(tArrowKey)
	{
		case RIGHT_ARROW:
			if(FALSE == ubUI_RecFileDelFlag)
				return;
			tFileSelOsdImgInfo[0].uwXStart += ((pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
			tOSD_Img2(&tFileSelOsdImgInfo[0], OSD_UPDATE);
			ubUI_RecFileDelFlag = FALSE;
			return;
		case LEFT_ARROW:
			if(FALSE == ubUI_RecFileDelFlag)
			{
				tFileSelOsdImgInfo[2].uwXStart += ((pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
				tOSD_Img2(&tFileSelOsdImgInfo[2], OSD_UPDATE);
			}
			ubUI_RecFileDelFlag = TRUE;
			return;
		case UP_ARROW:
			ubUI_RecFileDelFlag = FALSE;
			if(!pUI_RecTXFilesInfo.uwRecFileSelIdx)
				return;
			ubUI_PrevRecFileIdx = (pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx		= (--pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case DOWN_ARROW:
			ubUI_RecFileDelFlag = FALSE;
			if((pUI_RecTXFilesInfo.uwRecFileSelIdx + 1) >= pUI_RecTXFilesInfo.uwTotalRecFileNum)
				return;
			ubUI_PrevRecFileIdx = (pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx		= (++pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case ENTER_ARROW:
			if(TRUE == ubUI_RecFileDelFlag)
			{
				//! Delete Record File
				ubUI_RecFileDelRdy = UI_DeleteRecordFile(pUI_RecTXFilesInfo.uwRecFileSelIdx);
			}
			else
			{
				UI_PlayRecordFile(KNL_SIM_FLD,pUI_RecTXFilesInfo.uwRecFileSelIdx,&pUI_RecTXFoldersInfo,&pUI_RecTXFilesInfo);
				return;
			}
		case EXIT_ARROW:
			tOsdImgInfo.uwHSize  = 610;
			tOsdImgInfo.uwVSize  = 955;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_RecFileDelFlag = FALSE;
			if(TRUE == ubUI_RecFileDelRdy)
			{
                if(pUI_RecTXFilesInfo.uwRecFileSelIdx)
                {
                    ubUI_PrevRecFileIdx = (pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
                    ubUI_RecFileIdx		= (--pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
                }
				UI_DrawTXRecordFileMenu();
				return;
			}
			pUI_RecTXFoldersInfo.uwRecFolderSelIdx = 0;
			pUI_RecTXFilesInfo.uwRecFileSelIdx = 0;
			UI_DrawTXFolderMenu();
			tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;
			return;
		default:
			return;
	}
	if((((!(pUI_RecTXFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM)) && (pUI_RecTXFilesInfo.uwRecFileSelIdx) && (ubUI_PrevRecFileIdx == REC_FILE_LIST_MAXNUM - 1))) ||
	   ((ubUI_PrevRecFileIdx == 0) && (ubUI_RecFileIdx == REC_FILE_LIST_MAXNUM - 1)))
	{
		uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;

		tOsdImgInfo.uwHSize  = 415;
		tOsdImgInfo.uwVSize  = 955;
		tOsdImgInfo.uwXStart = 195;
		tOsdImgInfo.uwYStart = 50;
		OSD_EraserImg2(&tOsdImgInfo);
		if(DOWN_ARROW == tArrowKey)
		{
			uwFileStartIdx = pUI_RecTXFilesInfo.uwRecFileSelIdx;
			uwFileEndIdx   = pUI_RecTXFilesInfo.uwTotalRecFileNum - pUI_RecTXFilesInfo.uwRecFileSelIdx;
			uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?pUI_RecTXFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
		}
		else
		{
			uwFileStartIdx = (pUI_RecTXFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
			uwFileEndIdx   = pUI_RecTXFilesInfo.uwRecFileSelIdx + 1;
		}
		UI_ListRecFileInfo(&pUI_RecTXFilesInfo,uwFileStartIdx, uwFileEndIdx, OSD_QUEUE);
	}
	tFileSelOsdImgInfo[0].uwXStart += (ubUI_RecFileIdx * 40);
	tFileSelOsdImgInfo[1].uwXStart += (ubUI_PrevRecFileIdx * 40);
	tOSD_Img2(&tFileSelOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tFileSelOsdImgInfo[1], OSD_UPDATE);
}

void UI_StopRemotePlay(void)
{
	OSD_IMG_INFO tRecPlayOsdImgInfo[2];
    
    tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
	KNL_VideoPlayStop();

#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    if (tUI_RecPlayAct.tSimFld == KNL_SIM_FLD)
    {
        UI_CamNum_t tRecCamNum;
        APP_KNLRoleMap2CamNum(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx), tRecCamNum);
        UI_TXPlayEvent(tRecCamNum,UI_TXPLY_STOPCAM,NULL,NULL,NULL);        
        osDelay(200);
    	tRecPlayOsdImgInfo[0].uwHSize  = uwOSD_GetHSize();
    	tRecPlayOsdImgInfo[0].uwVSize  = uwOSD_GetVSize();
    	tRecPlayOsdImgInfo[0].uwXStart = 0;
    	tRecPlayOsdImgInfo[0].uwYStart = 0;
    	OSD_EraserImg1(&tRecPlayOsdImgInfo[0]);    
        KNL_RevertDisplayMode();
        if(tKNL_GetRecordFunc() != KNL_RECORDFUNC_LOOP && tKNL_GetRecordFunc() != KNL_RECORDFUNC_MANU)
        {
            KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
			tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
            tUI_PlayAct.pRecordStsNtyCb = NULL;
			tKNL_ExecRecordFunc(tUI_PlayAct);
        }
        pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
		pUI_RecFilesInfo.uwRecFileSelIdx = 0;
        UI_DrawDCIMFolderMenu();
    }
	else
#endif
    {
    	tRecPlayOsdImgInfo[0].uwHSize  = 100;
    	tRecPlayOsdImgInfo[0].uwVSize  = uwOSD_GetVSize();
    	tRecPlayOsdImgInfo[0].uwXStart = 620;
    	tRecPlayOsdImgInfo[0].uwYStart = 0;
    	OSD_EraserImg1(&tRecPlayOsdImgInfo[0]);    
        OSD_Weight(OSD_WEIGHT_8DIV8);
        tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
        tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
        tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
        tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_UPDATE);
	    UI_DrawRecordFileMenu();
    }
}

void UI_FSTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
    uint8_t *pUI_FSTrig = (uint8_t *)pvTrig;
    pUI_FSTrig = pUI_FSTrig;   

    //printd(DBG_CriticalLvl,"TXFS_Trig = %d %d %d %d %d %d\n",pUI_FSTrig[0],pUI_FSTrig[1],pUI_FSTrig[2],pUI_FSTrig[3],pUI_FSTrig[4],pUI_FSTrig[5]);
    switch(pUI_FSTrig[0])    
    {
        case UI_FS_ENTER:
        case UI_FS_EXIT:
        case UI_FS_FORMAT:
        case UI_FS_DEL:
            if(pUI_FSTrig[5] == 1)
            {
                printd(DBG_CriticalLvl, "RemoteStorageEmptyFs!\n");
                ubUI_FsEventSt = 1;
                UI_TxFsCbFunc(pUI_FSTrig[1]);
                return;
            }
        break;

        case UI_FS_SD_IN:
        case UI_FS_SD_OUT:
            printf("REMOTE SD INOUT %d\n",pUI_FSTrig[0]);
            if( tUI_State == UI_RECFOLDER_SEL_STATE || tUI_State == UI_RECTXFOLDER_SEL_STATE || tUI_State == UI_RECTXFILES_SEL_STATE )
            {
            	OSD_IMG_INFO tRecPlayOsdImgInfo[2];
            	tRecPlayOsdImgInfo[0].uwHSize  = 100;
            	tRecPlayOsdImgInfo[0].uwVSize  = uwOSD_GetVSize();
            	tRecPlayOsdImgInfo[0].uwXStart = 620;
            	tRecPlayOsdImgInfo[0].uwYStart = 0;
            	OSD_EraserImg1(&tRecPlayOsdImgInfo[0]);
            	OSD_Weight(OSD_WEIGHT_8DIV8);
            	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
            	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
            	tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
            	tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_UPDATE);
                pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
        		pUI_RecFilesInfo.uwRecFileSelIdx = 0;
                UI_DrawDCIMFolderMenu();
            }            
        break;
    }
}
//------------------------------------------------------------------------------
void UI_TXPLYTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
    uint8_t *pUI_PLYTrig = (uint8_t *)pvTrig;
    pUI_PLYTrig = pUI_PLYTrig;
    //printd(DBG_CriticalLvl,"TXPLY_Trig = %d %d %d %d %d %d\n",pUI_PLYTrig[0],pUI_PLYTrig[1],pUI_PLYTrig[2],pUI_PLYTrig[3],pUI_PLYTrig[4],pUI_PLYTrig[5]);
    if(pUI_PLYTrig[5] == 1)
    {
        printd(DBG_CriticalLvl, "RemoteStorageEmptyPly!\n");
        return;
    }    
    switch(pUI_PLYTrig[0])
    {
        case UI_TXPLY_PLAYCAM:
            ubUI_PlayEventSt = pUI_PLYTrig[5];
            UI_TxFsCbFunc(3);
            break;
        case UI_TXPLY_STOPCU:
            if(UI_ChkRemotePlayTrigger()==0)
            {
            	OSD_IMG_INFO tRecPlayOsdImgInfo[2];
            	UI_RemotePlayTrigger(1);
            	pUI_RecTXFilesInfo.uwRecFileSelIdx = (pUI_PLYTrig[4]<<8)+pUI_PLYTrig[3];                
                tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
            	KNL_VideoPlayStop();
            	tRecPlayOsdImgInfo[0].uwHSize  = 100;
            	tRecPlayOsdImgInfo[0].uwVSize  = uwOSD_GetVSize();
            	tRecPlayOsdImgInfo[0].uwXStart = 620;
            	tRecPlayOsdImgInfo[0].uwYStart = 0;
            	OSD_EraserImg1(&tRecPlayOsdImgInfo[0]);
            	OSD_Weight(OSD_WEIGHT_8DIV8);
            	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
            	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
            	tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
            	tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_UPDATE);

                KNL_RevertDisplayMode();
                if(tKNL_GetRecordFunc() != KNL_RECORDFUNC_LOOP && tKNL_GetRecordFunc() != KNL_RECORDFUNC_MANU)
                {
                    KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
        			tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
                    tUI_PlayAct.pRecordStsNtyCb = NULL;
        			tKNL_ExecRecordFunc(tUI_PlayAct);
                }
                pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
        		pUI_RecFilesInfo.uwRecFileSelIdx = 0;
                UI_DrawDCIMFolderMenu();
                UI_RemotePlayTrigger(0);
            }            
        break;

        case UI_TXPLY_UPDATE:
            pUI_RecTXFilesInfo.uwRecFileSelIdx = (pUI_PLYTrig[4]<<8)+pUI_PLYTrig[3];
        break;

        case UI_TXPLY_JMPPAUSE:
        {    
            OSD_IMG_INFO tRecPlayOsdImgInfo[2];
            //if(pUI_PLYTrig[1] != 1) printf("UI_TXPLY_JMPPAUSE Err\n");
			if(UI_RECFILE_PLAY == tUI_RecPlaySts)
			{
				uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PLAYHL_ICON, 1, &tRecPlayOsdImgInfo[0]);
				tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
				tUI_RecPlaySts = UI_RECFILE_PAUSE;
			}
			else
			{
				uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PAUSEHL_ICON, 1, &tRecPlayOsdImgInfo[0]);
				tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
				tUI_RecPlaySts = UI_RECFILE_PLAY;
			}
		}
        break;

        case UI_TXPLY_JMPBW:
            //if(pUI_PLYTrig[1] != 1) printf("PLY_JUMP_BWD Err\n");
        break;

        case UI_TXPLY_JMPFW:
            //if(pUI_PLYTrig[1] != 1) printf("PLY_JUMP_FWD Err\n");
        break;
    }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// pUI_FS_Event[0] Event
// pUI_FS_Event[1] FS Type
// pUI_FS_Event[2] Sort Mode
// pUI_FS_Event[3-4] Select Idx
void UI_GetTXFsInfo(UI_CamNum_t tCamNum,UI_FS_EventItem_t tEvent,UI_FSSettingItem_t tType,KNL_SORTMODE_t tSortMode,uint16_t uwIdx)
{
    UI_PUReqCmd_t   tFSCmd;
    tFSCmd.tDS_CamNum = tCamNum;
    tFSCmd.ubCmd[UI_TWC_TYPE]     = UI_SETTING;
    tFSCmd.ubCmd[UI_SETTING_ITEM] = UI_FS_EVENT_SETTING;
    tFSCmd.ubCmd[UI_SETTING_DATA] = tEvent;
    tFSCmd.ubCmd[UI_SETTING_DATA+1] = tType;
    tFSCmd.ubCmd[UI_SETTING_DATA+2] = tSortMode;
    tFSCmd.ubCmd[UI_SETTING_DATA+3] = (uwIdx&0x00FF);
    tFSCmd.ubCmd[UI_SETTING_DATA+4] = (uwIdx&0xFF00)>>8;
    tFSCmd.ubCmd[UI_SETTING_DATA+5] = 0;
    tFSCmd.ubCmd_Len  = 8;
    UI_SendRequestToBU(osThreadGetId(), &tFSCmd);
}

typedef struct
{
	int32_t signals;
	uint8_t ubRetry;
	uint32_t ulTimeout;
}UI_TXFsDraw_t;

uint8_t UI_TXFsDrawEvent(UI_CamNum_t tCamNum,UI_FS_EventItem_t tEvent,UI_FSSettingItem_t tType,KNL_SORTMODE_t tSortMode,uint16_t uwIdx)
{
    osEvent osUI_TXFSFinSig;
    uint8_t ubRetry;
    UI_TXFsDraw_t UI_TXFsDraw[] = {
    [UI_FS_DCIM_FLD] = {osUI_TXFsFldFinSignal,3,2000},
    [UI_FS_REC_FLD] = {osUI_TXFsFileFinSignal,2,3000},
    [UI_FS_FILE] = {osUI_TXFsHiddenFinSignal,2,3000},
    };    
    osUIThreadIdBackup = osThreadGetId();
    if(tEvent == UI_FS_ENTER)
    {
        ubRetry = 4;
        while(ubRetry)
        {
            if(ubKNL_CheckVdoTransmit(ubKNL_GetTXFldRole(pUI_RecFoldersInfo.uwRecFolderSelIdx))==1)
                break;
            osDelay(500);
            if(--ubRetry==0)    return 3;
        }
        ubRetry = UI_TXFsDraw[tType].ubRetry;
        while(ubRetry)
        {
            ubUI_FsEventSt = 0;
            UI_GetTXFsInfo(tCamNum,tEvent,tType,tSortMode,uwIdx);
            osUI_TXFSFinSig = osSignalWait(UI_TXFsDraw[tType].signals, UI_TXFsDraw[tType].ulTimeout);
            if((osUI_TXFSFinSig.status == osEventSignal) && (osUI_TXFSFinSig.value.signals == UI_TXFsDraw[tType].signals))
            {
                if(ubUI_FsEventSt == 0) 
                	return 0;
            	else
            	{
            	    printd(DBG_ErrorLvl,"TXFsDrawEventErr %d %d\n",tType,ubRetry);
            	    ubUI_FsEventSt = 0;
            	    return 1;
        	    }
            }
            else
                printd(DBG_ErrorLvl,"TXFsDrawEventTimeOut %d %d\n",tType,ubRetry);
            ubUI_FsEventSt = 0;
            ubRetry--;
        }
        return 2;
    }
    else if(tEvent == UI_FS_EXIT)
        UI_GetTXFsInfo(tCamNum,tEvent,tType,tSortMode,uwIdx);
    return 0;
}

void UI_TXPlayEvent(UI_CamNum_t tCamNum,UI_TXPLY_EventItem_t tEvent,uint8_t ubData1,uint8_t ubData2,uint16_t uwIdx)
{
    UI_PUReqCmd_t   tPlyCmd;
    tPlyCmd.tDS_CamNum = tCamNum;
    tPlyCmd.ubCmd[UI_TWC_TYPE]     = UI_SETTING;
    tPlyCmd.ubCmd[UI_SETTING_ITEM] = UI_PLAY_EVENT_SETTING;
    tPlyCmd.ubCmd[UI_SETTING_DATA] = tEvent;
    tPlyCmd.ubCmd[UI_SETTING_DATA+1] = ubData1;
    tPlyCmd.ubCmd[UI_SETTING_DATA+2] = ubData2;
    tPlyCmd.ubCmd[UI_SETTING_DATA+3] = (uwIdx&0x00FF);
    tPlyCmd.ubCmd[UI_SETTING_DATA+4] = (uwIdx&0xFF00)>>8;
    tPlyCmd.ubCmd[UI_SETTING_DATA+5] = 0;
    tPlyCmd.ubCmd_Len  = 8;
    UI_SendRequestToBU(osThreadGetId(), &tPlyCmd);
}

void UI_FSInfoInstall(void)
{
    KNL_TxFSInfoInstall((uint32_t*)&pUI_RecTXFoldersInfo,(uint32_t*)&pUI_RecTXFilesInfo);
}

void UI_TxFsCbFunc(uint8_t ubType)
{
    uint8_t ubSignal;
    if(osUIThreadIdBackup != NULL)
    {
        if(ubType==0) 
            ubSignal = osUI_TXFsFldFinSignal;
        if(ubType==1) 
            ubSignal = osUI_TXFsFileFinSignal;
        if(ubType==2)
            ubSignal = osUI_TXFsHiddenFinSignal;
        if(ubType==3)
            ubSignal = osUI_TXPlyFinSignal;
        osSignalSet(osUIThreadIdBackup, ubSignal);
    }
}
void UI_RemotePlayTrigger(uint8_t ubEn)
{
    ubUI_RemotePlayTrigger = ubEn;
}
uint8_t UI_ChkRemotePlayTrigger(void)
{
    return ubUI_RemotePlayTrigger;
}

#endif


