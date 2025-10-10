/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI_VBMBU.c
	\brief		User Interface of VBM Baby Unit (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.34
	\date		2021/07/01
	\copyright	Copyright (C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include "UI_VBMBU.h"
#include "SEN.h"
#include "RTC_API.h"
#include "ISP_API.h"
#include "MD_API.h"
#include "SF_API.h"
#include "Buzzer.h"
#include "FWU_API.h"
#include "ADO.h"
#include "RC.h"

#define osUI_SIGNALS	0x6A

/**
 * Key event mapping table
 *
 * @param ubKeyID  			Key ID
 * @param ubKeyCnt 			Key count	(100ms per 1 count, ex.long press 5s, the count set to 50)
 * @param KeyEventFuncPtr 	Key event mapping to function
 */
const UI_KeyEventMap_t UiKeyEventMap[] =
{
	{NULL,				0,					NULL},
#if (defined(BSP_SN93710_FHD_REC_TX_V4_1) || defined(BSP_D_SN93714_TX_V1) || defined(BSP_D_SN93716_TX_V1))
	{PKEY_ID0, 			0,					UI_PairingKey},
    #if (APP_SD_FUNC_ENABLE && APP_REC_FUNC_ENABLE)
    {PKEY_ID0, 			80,					KNL_TXRecordFormat},
    #else
	{PKEY_ID0, 			20,					UI_PowerKey},
	#endif
#endif
#ifdef BSP_D_SN93712_VBM_TX_V2
	{GKEY_ID6, 			0,					UI_PairingKey},
#endif
#ifdef BSP_D_SNCC72_TX_V1
    {GKEY_ID8,          0,                  UI_PairingKey},
#endif
};
const UI_SettingFuncPtr_t tUiSettingMap2Func[] =
{
	[UI_PTZ_SETTING] 			= NULL,	
	[UI_RECMODE_SETTING] 		= NULL,
	[UI_RECRES_SETTING] 		= NULL,
	[UI_SDCARD_SETTING] 		= NULL,
	[UI_PHOTOMODE_SETTING] 		= NULL,
	[UI_PHOTORES_SETTING] 		= NULL,
	[UI_SYSINFO_SETTING] 		= NULL,
	[UI_VOXMODE_SETTING] 		= UI_PowerSaveSetting,
	[UI_ECOMODE_SETTING] 		= UI_PowerSaveSetting,
	[UI_WORMODE_SETTING] 		= UI_PowerSaveSetting,
	[UI_ADOANR_SETTING]			= UI_ANRSetting,
	[UI_ADOAEC_SETTING]			= UI_AECSetting,
	[UI_IMGPROC_SETTING]		= UI_ImageProcSetting,
	[UI_MD_SETTING]				= UI_MDSetting,
	[UI_VOICETRIG_SETTING]		= UI_VoiceTrigSetting,
	[UI_PERDBGMODE_SETTING]		= UI_PerDebugModeSetting,
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
	[UI_FS_EVENT_SETTING]		= UI_FS_EventSetting,	
	[UI_PLAY_EVENT_SETTING]		= UI_TXPLY_EventSetting,	
#else	
	[UI_FS_EVENT_SETTING]		= NULL,	
	[UI_PLAY_EVENT_SETTING]		= NULL,
#endif	
};

const ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n32p4DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n8p2DB};

static UI_BUStatus_t tUI_BuStsInfo;
static APP_State_t tUI_SyncAppState;
static UI_ThreadNotify_t tosUI_Notify;
static uint8_t ubUI_ClearThdCntFlag;
static uint8_t ubUI_WorModeEnFlag;
static uint8_t ubUI_WorWakeUpCnt;
static uint8_t ubUI_SyncDisVoxFlag;
osMutexId UI_BUMutex;
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
static UI_RecFoldersInfo_t pUI_RecFoldersInfo;
static UI_RecFilesInfo_t pUI_RecFilesInfo;
static UI_RecPlayAct_t tUI_RecPlayAct;
static uint8_t ubUI_RemotePlayTwcEn=1;
uint8_t ubUI_RemotePlayTrigger=0;
#endif
//------------------------------------------------------------------------------
void UI_KeyEventExec(void *pvKeyEvent)
{
	static uint8_t ubUI_KeyEventIdx = 0;
	uint16_t uwUiKeyEvent_Cnt = 0, uwIdx;

	KEY_Event_t *ptKeyEvent = (KEY_Event_t *)pvKeyEvent;
	uwUiKeyEvent_Cnt = sizeof UiKeyEventMap / sizeof(UI_KeyEventMap_t);
	if(ptKeyEvent->ubKeyAction == KEY_UP_ACT)
	{
		if((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt))
		{
			if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
		}
		ubUI_KeyEventIdx = 0;
		return;
	}
	for(uwIdx = 1; uwIdx < uwUiKeyEvent_Cnt; uwIdx++)
	{
		if((ptKeyEvent->ubKeyID  == UiKeyEventMap[uwIdx].ubKeyID) &&
		   (ptKeyEvent->uwKeyCnt == UiKeyEventMap[uwIdx].uwKeyCnt))
		{
			ubUI_KeyEventIdx = uwIdx;
			if((ptKeyEvent->uwKeyCnt != 0) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr))
			{
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
				ubUI_KeyEventIdx = 0;
			}
		}
	}
}
//------------------------------------------------------------------------------
#if (APP_ADO_FUNC_ENABLE == 1)
KNL_ROLE PcConn_AdoRole;
#endif
void UI_ShowPcCnnPic(void)
{
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
	osMutexDef(UI_BUMutex);
	UI_BUMutex = osMutexCreate(osMutex(UI_BUMutex));
	if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvPUResponse, UI_RecvPURequest) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI setting 2way command fail !\n");
	ubUI_ClearThdCntFlag = FALSE;
	ubUI_SyncDisVoxFlag  = FALSE;
	UI_LoadDevStatusInfo();
	ubUI_WorModeEnFlag	 = (PS_WOR_MODE == tUI_BuStsInfo.tCamPsMode)?TRUE:FALSE;
	ubUI_WorWakeUpCnt 	 = 0;
	tUI_BuStsInfo.tCamScanMode = CAMSET_OFF;
#if APP_PC_CONNECT_EN
	KNL_PCCONN_INIT_t UI_PcConnInitInfo = {0};
	
	UI_PcConnInitInfo.EnterPcConnCb   = UI_ShowPcCnnPic;
	UI_PcConnInitInfo.LeavePcConnCb   = NULL;
	UI_PcConnInitInfo.OtherAction     = NULL;
	UI_PcConnInitInfo.SdCardPlugoutCb = UI_PcConn_SdCardPlugout;
	
	UI_PcConnInitInfo.SdFwuFileName.ubLen = 8;
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
}
//------------------------------------------------------------------------------
void UI_UpdateAppStatus(void *ptAppStsReport)
{
	APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;
	static uint8_t ubUI_BuSysSetFlag = FALSE;

	osMutexWait(UI_BUMutex, osWaitForever);
	switch(pAppStsRpt->tAPP_ReportType)
	{
		case APP_PAIRSTS_RPT:
		{
			UI_Result_t tPair_Result = (UI_Result_t)pAppStsRpt->ubAPP_Report[0];

			if(rUI_SUCCESS == tPair_Result)
				UI_ResetDevSetting();
			PAIRING_LED_IO = 0;
			break;
		}
		case APP_LINKSTS_RPT:
			break;
		default:
			break;
	}
	ubUI_ClearThdCntFlag = (tUI_SyncAppState == pAppStsRpt->tAPP_State)?FALSE:TRUE;
	tUI_SyncAppState = pAppStsRpt->tAPP_State;
	if((FALSE == ubUI_BuSysSetFlag) && (APP_IDLE_STATE == tUI_SyncAppState))
	{
		UI_SystemSetup();
		ubUI_BuSysSetFlag = TRUE;
	}
	osMutexRelease(UI_BUMutex);
}
//------------------------------------------------------------------------------
void UI_UpdateStatus(uint16_t *pThreadCnt)
{
	APP_EventMsg_t tUI_GetLinkStsMsg = {0};

	//osMutexWait(UI_BUMutex, osWaitForever);
	osStatus uiMutexState = osMutexWait(UI_BUMutex, 0);
	if (uiMutexState != osOK)
	{
		/* Skip the refresh when the UI mutex is owned by the key handler. */
		return;
	}
	UI_CLEAR_THREADCNT(ubUI_ClearThdCntFlag, *pThreadCnt);
	switch(tUI_SyncAppState)
	{
		case APP_LINK_STATE:
			if((TRUE == ubUI_WorModeEnFlag) && ((*pThreadCnt % UI_CHKWORSTS_PERIOD) != 0))
				UI_ChangePsModeToNormalMode();
			if(PS_VOX_MODE == tUI_BuStsInfo.tCamPsMode)
				UI_VoxTrigger();
			if(CAMSET_ON == tUI_BuStsInfo.tCamScanMode)
				UI_VoiceTrigger();
			if(MD_ON == tUI_BuStsInfo.MdParam.ubMD_Mode)
				UI_SetMotionEvent();
			if((*pThreadCnt % UI_UPDATESTS_PERIOD) != 0)
				UI_UpdateBUStatusToPU();
			if(!ubRC_GetFlg(0))
			{
				if(ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_MASTER_AP)>3)
					ubRC_SetFlg(ENCODE_0, TRUE);
			}
			(*pThreadCnt)++;
			break;
		case APP_LOSTLINK_STATE:
			if(PS_WOR_MODE == tUI_BuStsInfo.tCamPsMode)
			{
				if(FALSE == ubUI_WorModeEnFlag)
					UI_ChangePsModeToWorMode();
				if(!ubUI_WorWakeUpCnt)
					UI_VoiceTrigger();
				else if(++ubUI_WorWakeUpCnt > (6000 / UI_TASK_PERIOD))
					UI_PowerSaveSetting(&tUI_BuStsInfo.tCamPsMode);
			}
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
            if(KNL_VIDEO_PLAY == tKNL_GetRecordFunc() && UI_ChkRemotePlayTrigger() == 0)
            {
                UI_RemotePlayTrigger(1);   
                UI_RemotePlayStopTwc();
                KNL_TXRecordResume(KNL_TXREC_BUSY_PLAYSTOPVDO,1);
                KNL_VideoPlayStop();
            }   
#endif
						if(ubRC_GetFlg(0))
						ubRC_SetFlg(ENCODE_0, FALSE);  //µôÏßºó¹Ø±ÕRC£¬ÉÔÎ¢ÖØÑ¹
			break;
		case APP_PAIRING_STATE:
			if((*pThreadCnt % UI_PAIRINGLED_PERIOD) == 0)
				PAIRING_LED_IO = ~PAIRING_LED_IO;
			(*pThreadCnt)++;
			//osMutexRelease(UI_BUMutex);
			if (uiMutexState == osOK)
					osMutexRelease(UI_BUMutex);
			return;
		default:
			break;
	}
	PAIRING_LED_IO = 0;
	tUI_GetLinkStsMsg.ubAPP_Event = APP_LINKSTATUS_REPORT_EVENT;
	UI_SendMessageToAPP(&tUI_GetLinkStsMsg);
	//osMutexRelease(UI_BUMutex);
	if (uiMutexState == osOK)
		osMutexRelease(UI_BUMutex);
}
//------------------------------------------------------------------------------
void UI_EventHandles(UI_Event_t *ptEventPtr)
{
	switch(ptEventPtr->tEventType)
	{
		case AKEY_EVENT:
		case PKEY_EVENT:
		case GKEY_EVENT:
			UI_KeyEventExec(ptEventPtr->pvEvent);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PowerKey(void)
{
	POWER_LED_IO   = 0;
	PAIRING_LED_IO = 0;
#if (!defined(BSP_D_SN93716_TX_V1) && (!defined(BSP_D_SN93714_TX_V1) || !APP_SD_FUNC_ENABLE))
	SIGNAL_LED_IO  = 0;
#endif	
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
	RTC_SetGPO_1(0, RTC_PullDownEnable);
	printd(DBG_Debug1Lvl, "Power OFF !\n");
	RTC_PowerDisable();
	while(1);
}
//------------------------------------------------------------------------------
void UI_PairingKey(void)
{
	APP_EventMsg_t tUI_PairMessage = {0};

	tUI_PairMessage.ubAPP_Event = (APP_PAIRING_STATE == tUI_SyncAppState)?APP_PAIRING_STOP_EVENT:APP_PAIRING_START_EVENT;
	UI_SendMessageToAPP(&tUI_PairMessage);
	BUZ_PlaySingleSound();
}
//------------------------------------------------------------------------------
void UI_UpdateBUStatusToPU(void)
{
//	UI_BUReqCmd_t tUI_BuSts;

//	tUI_BuSts.ubCmd[UI_TWC_TYPE]		= UI_REPORT;
//	tUI_BuSts.ubCmd[UI_REPORT_ITEM] 	= UI_UPDATE_BUSTS;
//	tUI_BuSts.ubCmd[UI_REPORT_DATA] 	= 100;
//	tUI_BuSts.ubCmd_Len  				= 3;
//	UI_SendRequestToPU(NULL, &tUI_BuSts);
}
//------------------------------------------------------------------------------
UI_Result_t UI_SendRequestToPU(osThreadId thread_id, UI_BUReqCmd_t *ptReqCmd)
{
	UI_Result_t tReq_Result = rUI_SUCCESS;
	osEvent tReq_Event;
	uint8_t ubUI_TwcRetry = 5;
	
	tosUI_Notify.thread_id = thread_id;
	tosUI_Notify.iSignals  = osUI_SIGNALS;
	while(--ubUI_TwcRetry)
	{
		if(tTWC_Send(TWC_AP_MASTER, TWC_UI_SETTING, ptReqCmd->ubCmd, ptReqCmd->ubCmd_Len, 10) == TWC_SUCCESS)
			break;
		osDelay(10);
	}
	if(!ubUI_TwcRetry)
	{
		tTWC_StopTwcSend(TWC_AP_MASTER, TWC_UI_SETTING);
		return rUI_FAIL;
	}
	if(tosUI_Notify.thread_id != NULL)
	{
		tReq_Event = osSignalWait(tosUI_Notify.iSignals, UI_TWC_TIMEOUT);
		tReq_Result = (tReq_Event.status == osEventSignal)?(tReq_Event.value.signals == tosUI_Notify.iSignals)?tosUI_Notify.tReportSts:rUI_FAIL:rUI_FAIL;
		tTWC_StopTwcSend(TWC_AP_MASTER, TWC_UI_SETTING);
		tosUI_Notify.thread_id  = NULL;
		tosUI_Notify.iSignals   = NULL;
		tosUI_Notify.tReportSts = rUI_SUCCESS;
	}
	return tReq_Result;
}
//------------------------------------------------------------------------------
void UI_RecvPUResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus)
{
//	UI_CamNum_t tCamNum;
//	TWC_TAG tTWC_StaNum;

//	APP_KNLRoleMap2CamNum(ubKNL_GetRole(), tCamNum);
//	tTWC_StaNum = APP_GetSTANumMappingTable(tCamNum)->tTWC_StaNum;
//	if((tRecv_StaNum != tTWC_StaNum) || (NULL == tosUI_Notify.thread_id))
//		return;
	if((tRecv_StaNum != TWC_AP_MASTER) || (NULL == tosUI_Notify.thread_id))
		return;
	tosUI_Notify.tReportSts = (tStatus == TWC_SUCCESS)?rUI_SUCCESS:rUI_FAIL;
	if(osSignalSet(tosUI_Notify.thread_id, osUI_SIGNALS) != osOK)
		printd(DBG_ErrorLvl, "UI thread notify fail !\n");
}
//------------------------------------------------------------------------------
void UI_RecvPURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data)
{
//	UI_CamNum_t tCamNum;
//	TWC_TAG tTWC_StaNum;

//	APP_KNLRoleMap2CamNum(ubKNL_GetRole(), tCamNum);
//	tTWC_StaNum = APP_GetSTANumMappingTable(tCamNum)->tTWC_StaNum;
//	if(tRecv_StaNum != tTWC_StaNum)
//		return;
	if(tRecv_StaNum != TWC_AP_MASTER)
		return;
	switch(pTwc_Data[UI_TWC_TYPE])
	{
		case UI_SETTING:
			if(tUiSettingMap2Func[pTwc_Data[UI_SETTING_ITEM]].pvAction)
				tUiSettingMap2Func[pTwc_Data[UI_SETTING_ITEM]].pvAction((uint8_t *)(&pTwc_Data[UI_SETTING_DATA]));
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SetMotionEvent(void)
{       
	if(ubMD_EvenStateFunc())
	{
		UI_BUReqCmd_t tUI_MdMsg;

		tUI_MdMsg.ubCmd[UI_TWC_TYPE]	= UI_REPORT;
		tUI_MdMsg.ubCmd[UI_REPORT_ITEM] = UI_MD_TRIG;
		tUI_MdMsg.ubCmd[UI_REPORT_DATA] = TRUE;
		tUI_MdMsg.ubCmd_Len  			= 3;
		UI_SendRequestToPU(NULL, &tUI_MdMsg);
	}
}
//------------------------------------------------------------------------------
void UI_SystemSetup(void)
{
	UI_IspSetup();
#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_HwAecNr_I2C1_Init();
#endif
	UI_ANRSetting(&tUI_BuStsInfo.tCamAnrMode);
	UI_AECSetting(&tUI_BuStsInfo.tCamAecMode);	
	UI_MDSetting(&tUI_BuStsInfo.MdParam.ubMD_Param[0]);
	UI_VoiceTrigSetting(&tUI_BuStsInfo.tCamScanMode);
	if(PS_VOX_MODE == tUI_BuStsInfo.tCamPsMode)
		ubUI_SyncDisVoxFlag = TRUE;
	else
		UI_PowerSaveSetting(&tUI_BuStsInfo.tCamPsMode);
    MD_SetMdState(MD_UNSTABLE);
}
#define ADC_SUMRPT_VOX_THL			5000
#define ADC_SUMRPT_VOX_THH			5000
#define ADC_SUMRPT_VOICETRIG_THL	7000
#define ADC_SUMRPT_VOICETRIG_THH	7000
//------------------------------------------------------------------------------
void UI_PowerSaveSetting(void *pvPS_Mode)
{
	APP_EventMsg_t tUI_PsMessage = {0};
	UI_PowerSaveMode_t *pPS_Mode = (UI_PowerSaveMode_t *)pvPS_Mode;

	switch(pPS_Mode[0])
	{
		case PS_VOX_MODE:
			tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
			tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
			tUI_PsMessage.ubAPP_Message[1] = pPS_Mode[0];
			tUI_PsMessage.ubAPP_Message[2] = TRUE;
			UI_SendMessageToAPP(&tUI_PsMessage);
			if(CAMSET_ON == tUI_BuStsInfo.tCamScanMode)
			{
				ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_OFF);
				tUI_BuStsInfo.tCamScanMode = CAMSET_OFF;
			}
			ADO_SetAdcRpt(ADC_SUMRPT_VOX_THL, ADC_SUMRPT_VOX_THH, ADO_ON);
			tUI_BuStsInfo.tCamPsMode = PS_VOX_MODE;
			UI_UpdateDevStatusInfo();
			printd(DBG_InfoLvl, "		=> VOX Mode Enable\n");
			break;
#ifdef A7130
		case PS_ECO_MODE:
			tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
			tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
			tUI_PsMessage.ubAPP_Message[1] = pPS_Mode[0];
			tUI_PsMessage.ubAPP_Message[2] = TRUE;
			UI_SendMessageToAPP(&tUI_PsMessage);
			tUI_BuStsInfo.tCamPsMode = PS_ECO_MODE;
			break;
		case PS_WOR_MODE:
			ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_ON);
			ubUI_WorWakeUpCnt = 0;
			ubUI_WorModeEnFlag = FALSE;
			if(PS_WOR_MODE == tUI_BuStsInfo.tCamPsMode)
				break;
			tUI_BuStsInfo.tCamPsMode   = PS_WOR_MODE;
			tUI_BuStsInfo.tCamScanMode = CAMSET_OFF;
			UI_UpdateDevStatusInfo();
			printd(DBG_InfoLvl, "		=> WOR Mode\n");
			break;
#endif
		case POWER_NORMAL_MODE:
			if(PS_VOX_MODE == tUI_BuStsInfo.tCamPsMode)
				UI_DisableVox();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ChangePsModeToWorMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_WOR_MODE;
	tUI_PsMessage.ubAPP_Message[2] = FALSE;
	tUI_PsMessage.ubAPP_Message[3] = FALSE;
	tUI_PsMessage.ubAPP_Message[4] = TRUE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	ubUI_WorModeEnFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_ChangePsModeToNormalMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_WOR_MODE;
	tUI_PsMessage.ubAPP_Message[2] = TRUE;
	tUI_PsMessage.ubAPP_Message[3] = FALSE;
	tUI_PsMessage.ubAPP_Message[4] = (!ubUI_WorWakeUpCnt)?TRUE:FALSE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_OFF);
	tUI_BuStsInfo.tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
	ubUI_WorModeEnFlag = FALSE;
	ubUI_WorWakeUpCnt  = 0;
	printd(DBG_InfoLvl, "		=> WOR Disable\n");
}
//------------------------------------------------------------------------------
void UI_DisableVox(void)
{
	APP_EventMsg_t tUI_VoxMsg = {0};

	ADO_SetAdcRpt(ADC_SUMRPT_VOX_THL, ADC_SUMRPT_VOX_THH, ADO_OFF);
	tUI_VoxMsg.ubAPP_Event 	    = APP_POWERSAVE_EVENT;
	tUI_VoxMsg.ubAPP_Message[0] = 2;		//! Message Length
	tUI_VoxMsg.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_VoxMsg.ubAPP_Message[2] = FALSE;
	UI_SendMessageToAPP(&tUI_VoxMsg);
	tUI_BuStsInfo.tCamPsMode = POWER_NORMAL_MODE;
//	tUI_BuStsInfo.tCamScanMode = CAMSET_ON;
//	UI_VoiceTrigSetting(&tUI_BuStsInfo.tCamScanMode);
	UI_UpdateDevStatusInfo();
	printd(DBG_InfoLvl, "		=> VOX Mode Disable\n");
}
//------------------------------------------------------------------------------
void UI_VoxTrigger(void)
{
	UI_BUReqCmd_t tUI_VoxReqCmd;
	uint32_t ulUI_AdcRpt = 0;

	tUI_VoxReqCmd.ubCmd[UI_TWC_TYPE]	= UI_REPORT;
	tUI_VoxReqCmd.ubCmd[UI_REPORT_ITEM] = UI_VOX_TRIG;
	tUI_VoxReqCmd.ubCmd[UI_REPORT_DATA] = FALSE;
	tUI_VoxReqCmd.ubCmd_Len  			= 3;
	if(TRUE == ubUI_SyncDisVoxFlag)
	{
		UI_SendRequestToPU(NULL, &tUI_VoxReqCmd);
		tUI_BuStsInfo.tCamPsMode = POWER_NORMAL_MODE;
		UI_UpdateDevStatusInfo();
		ubUI_SyncDisVoxFlag = FALSE;
		return;
	}
	ulUI_AdcRpt = ulADO_GetAdcSumHigh();
	if(ulUI_AdcRpt > ADC_SUMRPT_VOX_THH)
	{
		UI_SendRequestToPU(NULL, &tUI_VoxReqCmd);
		UI_DisableVox();
	}
}
//------------------------------------------------------------------------------
void UI_VoiceTrigSetting(void *pvTrigMode)
{
	UI_CamsSetMode_t *pVoiceTrigMode = (UI_CamsSetMode_t *)pvTrigMode;

	tUI_BuStsInfo.tCamScanMode = *pVoiceTrigMode;
	ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, (CAMSET_ON == tUI_BuStsInfo.tCamScanMode)?ADO_ON:ADO_OFF);
}
//------------------------------------------------------------------------------
void UI_VoiceTrigger(void)
{
	UI_BUReqCmd_t tUI_VoiceReqCmd;
	uint32_t ulUI_AdcRpt = 0;

	ulUI_AdcRpt = ulADO_GetAdcSumHigh();
	if(PS_WOR_MODE == tUI_BuStsInfo.tCamPsMode)
	{
		if(ulUI_AdcRpt > ADC_SUMRPT_VOICETRIG_THH)
		{
			APP_EventMsg_t tUI_PsMessage = {0};

			tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
			tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
			tUI_PsMessage.ubAPP_Message[1] = PS_WOR_MODE;
			tUI_PsMessage.ubAPP_Message[2] = TRUE;
			tUI_PsMessage.ubAPP_Message[3] = TRUE;
			tUI_PsMessage.ubAPP_Message[4] = TRUE;
			UI_SendMessageToAPP(&tUI_PsMessage);
			ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_OFF);
			ubUI_WorWakeUpCnt++;
			printd(DBG_InfoLvl, "		=> Voice Trigger\n");
		}
	}
	else if((PS_VOX_MODE == tUI_BuStsInfo.tCamPsMode) ||
		    (CAMSET_ON == tUI_BuStsInfo.tCamScanMode))
	{
		if(ulUI_AdcRpt > ADC_SUMRPT_VOICETRIG_THH)
		{
			tUI_VoiceReqCmd.ubCmd[UI_TWC_TYPE]	  = UI_REPORT;
			tUI_VoiceReqCmd.ubCmd[UI_REPORT_ITEM] = UI_VOICE_TRIG;
			tUI_VoiceReqCmd.ubCmd[UI_REPORT_DATA] = TRUE;
			tUI_VoiceReqCmd.ubCmd_Len  			  = 3;
			UI_SendRequestToPU(NULL, &tUI_VoiceReqCmd);
			printd(DBG_InfoLvl, "		=> Voice Trigger\n");
		}
	}
}
//------------------------------------------------------------------------------
uint8_t ubGetAecNrCommand(UI_CamsSetMode_t AecSwitch, UI_CamsSetMode_t NrSwitch)
{
	//printf("		=> aec:%d,  nr:%d\n",AecSwitch,NrSwitch);
	if( AecSwitch==CAMSET_ON && NrSwitch==CAMSET_OFF )
	{
		return 0x80;
	}
	else if( AecSwitch==CAMSET_OFF && NrSwitch==CAMSET_ON )
	{
		return 0x08;
	}
	else if( AecSwitch==CAMSET_ON && NrSwitch==CAMSET_ON )
	{
		return 0x88;
	}
	else if( AecSwitch==CAMSET_OFF && NrSwitch==CAMSET_OFF )
	{
		return 0x00;
	}
	return 0;
}
//------------------------------------------------------------------------------
void UI_ANRSetting(void *pvAnrMode)
{
	uint8_t *pUI_AnrMode = (uint8_t *)pvAnrMode;
	
	//! AEC and NR Setting
#if(ADO_ENC_TYPE==AUDIO32_ENC)
#if APP_ADO_AEC_NR_TYPE == AEC_NR_SW
	ADO_Noise_Process_Type((CAMSET_ON == (UI_CamsSetMode_t)pUI_AnrMode[0])?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
#elif APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
#endif
#elif(ADO_ENC_TYPE==HW_ALAW_ENC)
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
#elif(ADO_ENC_TYPE==SW_ALAW_ENC)
#if APP_ADO_AEC_NR_TYPE == AEC_NR_SW
	ADO_Noise_Process_Type((CAMSET_ON == (UI_CamsSetMode_t)pUI_AnrMode[0])?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
#elif APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
#endif
#elif(ADO_ENC_TYPE==SW_AAC_ENC)
#if APP_ADO_AEC_NR_TYPE == AEC_NR_SW
	ADO_Noise_Process_Type((CAMSET_ON == (UI_CamsSetMode_t)pUI_AnrMode[0])?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
#elif APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
#endif
#endif

	if(tUI_BuStsInfo.tCamAnrMode != (UI_CamsSetMode_t)pUI_AnrMode[0])
	{
		tUI_BuStsInfo.tCamAnrMode = (UI_CamsSetMode_t)pUI_AnrMode[0];
		UI_UpdateDevStatusInfo();
	}
	printd(DBG_InfoLvl, "		=> ANR %s\n", (CAMSET_ON == tUI_BuStsInfo.tCamAnrMode)?"ON":"OFF");

#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_HwAecNr_Command(ubGetAecNrCommand(tUI_BuStsInfo.tCamAecMode,tUI_BuStsInfo.tCamAnrMode));
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
#endif
}
//------------------------------------------------------------------------------
void UI_AECSetting(void *pvAecMode)
{
	uint8_t *pUI_AecMode = (uint8_t *)pvAecMode;
	
#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
		ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
		ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
#endif

	//! Setting AEC
	if(tUI_BuStsInfo.tCamAecMode != (UI_CamsSetMode_t)pUI_AecMode[0])
	{
		tUI_BuStsInfo.tCamAecMode = (UI_CamsSetMode_t)pUI_AecMode[0];
		if(tUI_BuStsInfo.tCamAecMode ==  CAMSET_ON)
		{
			ADO_Noise_Process_Type(NOISE_AEC,AEC_NR_16kHZ);
		}
		else
		{
			ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
		}
//		UI_UpdateDevStatusInfo();
	}
	printd(DBG_InfoLvl, "		=> AEC %s\n", (CAMSET_ON == tUI_BuStsInfo.tCamAecMode)?"ON":"OFF");
	
#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_HwAecNr_Command(ubGetAecNrCommand(tUI_BuStsInfo.tCamAecMode,tUI_BuStsInfo.tCamAnrMode));
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
#endif
}
//------------------------------------------------------------------------------
void UI_IspSetup(void)
{
	UI_IspSettingFuncPtr_t tUI_IspFunc[UI_IMGSETTING_MAX] = 
	{
		[UI_IMG3DNR_SETTING] 		= {ISP_NR3DSwitch, (uint8_t *)&tUI_BuStsInfo.tCam3DNRMode},
		[UI_IMGvLDC_SETTING] 		= {ISP_VLDCSwitch, (uint8_t *)&tUI_BuStsInfo.tCamvLDCMode},
		[UI_IMGWDR_SETTING] 		= {NULL, NULL},
		[UI_IMGDIS_SETTING] 		= {NULL, NULL},
		[UI_IMGCBR_SETTING] 		= {NULL, NULL},
		[UI_IMGCONDENSE_SETTING] 	= {NULL, NULL},
		[UI_FLICKER_SETTING] 		= {ISP_SetAePwrFreq, 	(uint8_t *)&tUI_BuStsInfo.tCamFlicker},
		[UI_IMGBL_SETTING] 			= {ISP_SetIQBrightness, (uint8_t *)&tUI_BuStsInfo.tCamColorParam.ubColorBL},
		[UI_IMGCONTRAST_SETTING] 	= {ISP_SetIQContrast, 	(uint8_t *)&tUI_BuStsInfo.tCamColorParam.ubColorContrast},
		[UI_IMGSATURATION_SETTING] 	= {ISP_SetIQSaturation, (uint8_t *)&tUI_BuStsInfo.tCamColorParam.ubColorSaturation},
		[UI_IMGHUE_SETTING]			= {ISP_SetIQChroma, 	(uint8_t *)&tUI_BuStsInfo.tCamColorParam.ubColorHue},
	};
	uint8_t ubUI_IspItem, ubUI_IspParam;

	for(ubUI_IspItem = UI_IMG3DNR_SETTING; ubUI_IspItem < UI_IMGSETTING_MAX; ubUI_IspItem++)
	{
		if(tUI_IspFunc[ubUI_IspItem].pvImgFunc)
		{
			ubUI_IspParam = (*tUI_IspFunc[ubUI_IspItem].pImgParam) * (((UI_IMGBL_SETTING        == ubUI_IspItem) ||
																	  (UI_IMGCONTRAST_SETTING   == ubUI_IspItem) ||
														              (UI_IMGSATURATION_SETTING == ubUI_IspItem) ||
															          (UI_IMGHUE_SETTING 		== ubUI_IspItem))?2:1);
			if(UI_FLICKER_SETTING == ubUI_IspItem)
				ubUI_IspParam = ((CAMFLICKER_50HZ == ubUI_IspParam)?SENSOR_PWR_FREQ_50HZ:SENSOR_PWR_FREQ_60HZ);
			tUI_IspFunc[ubUI_IspItem].pvImgFunc(ubUI_IspParam);
		}
	}
}
//------------------------------------------------------------------------------
void UI_ImageProcSetting(void *pvImgProc)
{
	uint8_t *pUI_ImgProc = (uint8_t *)pvImgProc;

	switch(pUI_ImgProc[0])
	{
		case UI_IMG3DNR_SETTING:
			tUI_BuStsInfo.tCam3DNRMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			ISP_NR3DSwitch(tUI_BuStsInfo.tCam3DNRMode);
			break;
		case UI_IMGvLDC_SETTING:
			tUI_BuStsInfo.tCamvLDCMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			ISP_VLDCSwitch(tUI_BuStsInfo.tCamvLDCMode);
			break;
		case UI_IMGWDR_SETTING:
			break;
		case UI_IMGDIS_SETTING:
			tUI_BuStsInfo.tCamDisMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			break;
		case UI_IMGCBR_SETTING:
			tUI_BuStsInfo.tCamCbrMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			break;
		case UI_IMGCONDENSE_SETTING:
			tUI_BuStsInfo.tCamCondenseMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			break;
		case UI_FLICKER_SETTING:
			tUI_BuStsInfo.tCamFlicker = (UI_CamFlicker_t)pUI_ImgProc[1];
			ISP_SetAePwrFreq((CAMFLICKER_50HZ == tUI_BuStsInfo.tCamFlicker)?SENSOR_PWR_FREQ_50HZ:SENSOR_PWR_FREQ_60HZ);
			printd(DBG_InfoLvl, "		=> Flicker: %s\n", (CAMFLICKER_50HZ == tUI_BuStsInfo.tCamFlicker)?"50Hz":"60Hz");
			break;
		case UI_IMGBL_SETTING:
			tUI_BuStsInfo.tCamColorParam.ubColorBL = pUI_ImgProc[1];
			ISP_SetIQBrightness((tUI_BuStsInfo.tCamColorParam.ubColorBL*2));
			break;
		case UI_IMGCONTRAST_SETTING:
			tUI_BuStsInfo.tCamColorParam.ubColorContrast = pUI_ImgProc[1];
			ISP_SetIQContrast((tUI_BuStsInfo.tCamColorParam.ubColorContrast*2));
			break;
		case UI_IMGSATURATION_SETTING:
			tUI_BuStsInfo.tCamColorParam.ubColorSaturation = pUI_ImgProc[1];
			ISP_SetIQSaturation((tUI_BuStsInfo.tCamColorParam.ubColorSaturation*2));
			break;
		case UI_IMGHUE_SETTING:
			tUI_BuStsInfo.tCamColorParam.ubColorHue = pUI_ImgProc[1];
			ISP_SetIQChroma(tUI_BuStsInfo.tCamColorParam.ubColorHue*2);
			break;
		default:
			return;
	}
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
#define MD_TRIG_LVL	16
void UI_MDTrigger(void)
{
	uint32_t ulUI_MdTrig = 0;
    uint32_t ulUI_MdTrig_LV = 0;
    
	ulUI_MdTrig = uwMD_GetCnt(MD_REG1_CNT_01);    
	printd(DBG_InfoLvl, "=>MD Trig1: %d\n", ulUI_MdTrig);
   	//ulUI_MdTrig_LV = (((tUI_BuStsInfo.MdParam.ubMD_Param[2]+1) * (tUI_BuStsInfo.MdParam.ubMD_Param[3]+1)) < 30)? 
   	//     ((tUI_BuStsInfo.MdParam.ubMD_Param[2]+1) * (tUI_BuStsInfo.MdParam.ubMD_Param[3]+1) * MD_TRIG_LVL) : (30 * MD_TRIG_LVL);
    
	if(ulUI_MdTrig > ulUI_MdTrig_LV)
	{
		UI_BUReqCmd_t tUI_MdMsg;

		tUI_MdMsg.ubCmd[UI_TWC_TYPE]	= UI_REPORT;
		tUI_MdMsg.ubCmd[UI_REPORT_ITEM] = UI_MD_TRIG;
		tUI_MdMsg.ubCmd[UI_REPORT_DATA] = TRUE;
		tUI_MdMsg.ubCmd_Len  			= 3;
		UI_SendRequestToPU(NULL, &tUI_MdMsg);
		printd(DBG_InfoLvl, "=>MD Trig2: %d\n", ulUI_MdTrig);
	}
}
//------------------------------------------------------------------------------
void UI_MDSetting(void *pvMdParam)
{
#define MD_H_WINDOWSIZE		64
#define MD_V_WINDOWSIZE		48
	uint16_t uwMD_X          = 0;
	uint16_t uwMD_Y          = 0;
	uint16_t uwMD_BlockSIdx  = 0;
	uint16_t uwMD_BlockEIdx  = 0;
	uint16_t uwMD_BlockH     = sensor_cfg.xtSENWin.uwHSize / 30;
	uint16_t uwMD_BlockV     = sensor_cfg.xtSENWin.uwVSize / 23;
	uint16_t uwMD_H_WinNum   = sensor_cfg.xtSENWin.uwHSize / MD_H_WINDOWSIZE;
	uint16_t uwMD_StartIdx   = 0, i, j;
	uint16_t uwMD_TotalBlock = (30 * 23) / 2;	//1 block is 4 bits data, 1 byte is 2 block
	uint16_t uwMD_BlockNum1  = 0, uwMD_BlockNum2 = 0;
	uint8_t ubMD_BlockCnt    = 0;
	uint8_t *pMD_Param       = (uint8_t *)pvMdParam;
	uint8_t *pMD_BlockValue, *pMD_BlockGroup;
	static uint8_t ubUI_MdUpdateFlag = FALSE;

	if((pMD_Param[2] == 0) && (pMD_Param[3] == 0))
	{
		tUI_BuStsInfo.MdParam.ubMD_Mode = MD_OFF;
		if(TRUE == ubUI_MdUpdateFlag)
		{
			MD_Switch(tUI_BuStsInfo.MdParam.ubMD_Mode);
			UI_UpdateDevStatusInfo();
			printd(DBG_InfoLvl, "=>MD OFF\n");
		}
		else
			ubUI_MdUpdateFlag = TRUE;
		return;
	}
	printd(DBG_InfoLvl, "=>MD %d_%d_%d\n", ((pMD_Param[1] << 8) | pMD_Param[0]),  pMD_Param[2],  pMD_Param[3]);
	ubMD_BlockCnt  = pMD_Param[3] + 1;
	pMD_BlockValue = malloc(uwMD_TotalBlock);
	pMD_BlockGroup = malloc(uwMD_TotalBlock);
	for(i = 0; i < uwMD_TotalBlock; i++)
	{
		pMD_BlockValue[i] = MD_REG1_CNT_00;
		pMD_BlockGroup[i] = 0x88;
	}
	uwMD_StartIdx  = ((pMD_Param[1] << 8) | pMD_Param[0]);
	uwMD_X = (uwMD_StartIdx % uwMD_H_WinNum) * MD_H_WINDOWSIZE;
	uwMD_Y = (uwMD_StartIdx / uwMD_H_WinNum) * MD_V_WINDOWSIZE;
	uwMD_BlockSIdx = ((uwMD_X / uwMD_BlockH) + ((uwMD_Y / uwMD_BlockV) * 30));
	printd(DBG_InfoLvl, "=>MD x=%d,y=%d,S=%d\n",uwMD_X,uwMD_Y,uwMD_BlockSIdx);
	if(!((uwMD_StartIdx + pMD_Param[2] + 1) % uwMD_H_WinNum))
	{
		uwMD_Y = ((uwMD_StartIdx + pMD_Param[2]) / uwMD_H_WinNum) * MD_V_WINDOWSIZE;
		uwMD_BlockEIdx = (((uwMD_Y / uwMD_BlockV) + 1) * 30) - 1;
		printd(DBG_InfoLvl, "1=>MD y=%d,e=%d\n",uwMD_Y,uwMD_BlockEIdx);
	}
	else
	{
		uwMD_X = ((uwMD_StartIdx + pMD_Param[2]) % uwMD_H_WinNum) * MD_H_WINDOWSIZE;
		uwMD_Y = ((uwMD_StartIdx + pMD_Param[2]) / uwMD_H_WinNum) * MD_V_WINDOWSIZE;
		uwMD_BlockEIdx = ((uwMD_X / uwMD_BlockH) + ((uwMD_Y / uwMD_BlockV) * 30));
		printd(DBG_InfoLvl, "2=>MD x=%d,y=%d,e=%d\n",uwMD_X,uwMD_Y,uwMD_BlockEIdx);
	}
	ubMD_BlockCnt  = ((((ubMD_BlockCnt * MD_V_WINDOWSIZE) / uwMD_BlockV) + 1) > 23) ? 23 : (((ubMD_BlockCnt * MD_V_WINDOWSIZE) / uwMD_BlockV) + 1);
	printd(DBG_InfoLvl, "3=>MD c=%d\n",ubMD_BlockCnt);

	MD_Init();
    MD_SetUserThreshold(MD_TRIG_LVL);
	MD_Switch(MD_OFF);
	for(i = uwMD_BlockSIdx; i <= uwMD_BlockEIdx; i++)
	{
		for(j = 0; j < ubMD_BlockCnt; j++)
		{
			uwMD_BlockNum1 = (i + (j * 30)) / 2;
			uwMD_BlockNum2 = (i + (j * 30)) % 2;
			if(!uwMD_BlockNum2)
			{
				pMD_BlockValue[uwMD_BlockNum1] = (pMD_BlockValue[uwMD_BlockNum1] & 0xF0) | MD_REG1_CNT_01;
				pMD_BlockGroup[uwMD_BlockNum1] = (pMD_BlockGroup[uwMD_BlockNum1] & 0xF0) | 2;
			}
			else
			{
				pMD_BlockValue[uwMD_BlockNum1] = (pMD_BlockValue[uwMD_BlockNum1] & 0x0F) | (MD_REG1_CNT_01 << 4);
				pMD_BlockGroup[uwMD_BlockNum1] = (pMD_BlockGroup[uwMD_BlockNum1] & 0x0F) | (2 << 4);
			}
		}
	}
	MD_SetROIindex((uint32_t *)pMD_BlockValue);
	MD_SetROIweight((uint32_t *)pMD_BlockGroup);
	MD_SetSensitivity(80);
	tUI_BuStsInfo.MdParam.ubMD_Mode = MD_ON;
	MD_Switch(tUI_BuStsInfo.MdParam.ubMD_Mode);
#if defined(OP_STA)	
	KNL_MDResChBackup(tUI_BuStsInfo.MdParam.ubMD_Mode);
#endif
	free(pMD_BlockValue);
	free(pMD_BlockGroup);
	if(TRUE == ubUI_MdUpdateFlag)
	{
		for(i = 0; i < 4; i++)
			tUI_BuStsInfo.MdParam.ubMD_Param[i] = pMD_Param[i];
		UI_UpdateDevStatusInfo();
	}
	else
		ubUI_MdUpdateFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_ResetDevSetting(void)
{
	uint8_t i;

	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamAnrMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCam3DNRMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamvLDCMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamAecMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamDisMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamFlicker,		CAMFLICKER_60HZ);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamCbrMode,  		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamCondenseMode, 	CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamColorParam.ubColorBL, 		  64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamColorParam.ubColorContrast,	  64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamColorParam.ubColorSaturation, 64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamColorParam.ubColorHue, 		  64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.MdParam.ubMD_Mode,	MD_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamPsMode,			POWER_NORMAL_MODE);
	
	for(i = 0; i < 4; i++)
		tUI_BuStsInfo.MdParam.ubMD_Param[i] = 0;
	UI_UpdateDevStatusInfo();
	UI_SystemSetup();
}
//------------------------------------------------------------------------------
void UI_LoadDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	uint8_t i;

	memset(&tUI_BuStsInfo, 0xFF, sizeof(UI_BUStatus_t));
	osMutexWait(APP_UpdateMutex, osWaitForever);
	SF_Read(ulUI_SFAddr, sizeof(UI_BUStatus_t), (uint8_t *)&tUI_BuStsInfo);
	ADO_GetNrSettingFromUiInfo((ADO_NOISE_PROCESS_TYPE)tUI_BuStsInfo.tCamAnrMode);
	ADO_GetAecSettingFromUiInfo((ADO_NOISE_PROCESS_TYPE)tUI_BuStsInfo.tCamAecMode);
	osMutexRelease(APP_UpdateMutex);
	printd(DBG_InfoLvl, "UI TAG:%s\n",tUI_BuStsInfo.cbUI_DevStsTag);
	printd(DBG_InfoLvl, "UI VER:%s\n",tUI_BuStsInfo.cbUI_FwVersion);
	if ((strncmp(tUI_BuStsInfo.cbUI_DevStsTag, SF_STA_UI_SECTOR_TAG, sizeof(tUI_BuStsInfo.cbUI_DevStsTag) - 1) == 0)
	&& (strncmp(tUI_BuStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_BuStsInfo.cbUI_FwVersion) - 1) == 0)) {

	} else {
		printd(DBG_ErrorLvl, "TAG no match, Reset UI\n");
	}
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamAnrMode,  		CAMSET_OFF);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCam3DNRMode, 		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamvLDCMode, 		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamAecMode,  		CAMSET_OFF);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamDisMode,  		CAMSET_OFF);
	UI_CHK_CAMFLICER(tUI_BuStsInfo.tCamFlicker);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamCbrMode,  		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamCondenseMode, 	CAMSET_OFF);
	UI_CHK_CAMPARAM(tUI_BuStsInfo.tCamColorParam.ubColorBL, 		64);
	UI_CHK_CAMPARAM(tUI_BuStsInfo.tCamColorParam.ubColorContrast, 	64);
	UI_CHK_CAMPARAM(tUI_BuStsInfo.tCamColorParam.ubColorSaturation, 64);
	UI_CHK_CAMPARAM(tUI_BuStsInfo.tCamColorParam.ubColorHue, 		64);
	UI_CHK_MDMODE(tUI_BuStsInfo.MdParam.ubMD_Mode,		MD_OFF);
	UI_CHK_PSMODE(tUI_BuStsInfo.tCamPsMode,				POWER_NORMAL_MODE);
	for(i = 0; i < 4; i++)
	{
		if(MD_OFF == tUI_BuStsInfo.MdParam.ubMD_Mode)
			tUI_BuStsInfo.MdParam.ubMD_Param[i] = 0;
		else
			UI_CHK_CAMPARAM(tUI_BuStsInfo.MdParam.ubMD_Param[i], 0);
	}
	ADO_SetDacR2RVol(tUI_VOLTable[VOL_LVL4]);
}
//------------------------------------------------------------------------------
void UI_UpdateDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);

	osMutexWait(APP_UpdateMutex, osWaitForever);
	memcpy(tUI_BuStsInfo.cbUI_DevStsTag, SF_STA_UI_SECTOR_TAG, sizeof(tUI_BuStsInfo.cbUI_DevStsTag) - 1);
	memcpy(tUI_BuStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_BuStsInfo.cbUI_FwVersion) - 1);
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulUI_SFAddr, pSF_Info->ulSecSize, 1);
	SF_Write(ulUI_SFAddr, sizeof(UI_BUStatus_t), (uint8_t *)&tUI_BuStsInfo);
	SF_EnableWrProtect();
	osMutexRelease(APP_UpdateMutex);
}
//------------------------------------------------------------------------------
void UI_PerDebugModeSetting(void *pvDbgMode)
{
	uint8_t ubPerDbgMode;

	ubPerDbgMode = *(uint8_t *)pvDbgMode;
	KNL_EnPerDebugMode(ubPerDbgMode, NULL);
}
//------------------------------------------------------------------------------
UI_BUStatus_t *pUI_GetDevSetting(void) 
{
	return (UI_BUStatus_t *)&tUI_BuStsInfo;
}
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
//------------------------------------------------------------------------------
void UI_StopPlayRecordFile(uint8_t ubPlayRet)
{
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
    uint32_t ulFileIdx;
    static uint8_t ubUI_SDUnPlugfg=0;
	switch(ubPlayRet)
	{
		case KNL_VDOPLAY_STOP:
            if(tUI_RecPlayAct.tPlayDir != UI_PLY_ONCE)
            {
                ulFileIdx = ulKNL_PlayNextFile((KNL_PLY_NextItem_t)tUI_RecPlayAct.tPlayDir);
                if(ulFileIdx < 0xFFFFFFF0)
                {
                    pUI_RecFilesInfo.uwRecFileSelIdx = ulFileIdx;
                    UI_TXPLY_UpdateToCU(UI_TXPLY_UPDATE,NULL,NULL,pUI_RecFilesInfo.uwRecFileSelIdx);
                }
            }
            else
            {
                if(tUI_RecPlayAct.tPlaySts != UI_RECFILE_STOP)
                {
                    KNL_VideoPlayStop();
        			tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
                }
            }
			break;

        case KNL_ErrorNoCard:
            ubUI_SDUnPlugfg = 1;
            break;
		default:
			if(UI_RECFILE_PLAY == tUI_RecPlayAct.tPlaySts)
			{
                if(ubUI_SDUnPlugfg)
                {
                    ubUI_SDUnPlugfg = 0;
                    if(TRUE == ubKNL_GetBackupDispInfoFlag())
                    {
                        KNL_ROLE tPlayRole;
                        KNL_SRC tVdoSrcNum;
                        for(tPlayRole = KNL_STA1; tPlayRole <= KNL_STA4; tPlayRole++)
                        {
                            tVdoSrcNum = (KNL_SRC)(KNL_SRC_1_MAIN + tPlayRole);
                            KNL_VdoStop(tVdoSrcNum);
                        }
                        ubKNL_WaitNodeFinish(KNL_SRC_1_MAIN);
                        KNL_ResetVdoProc();
                    }
                    KNL_RevertDisplayMode();
                    KNL_TXRecordResume(KNL_TXREC_BUSY_NONE,0); 
                }
				tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
				tUI_PlayAct.pRecordStsNtyCb = NULL;
				tKNL_ExecRecordFunc(tUI_PlayAct);
				tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
				if(ubUI_RemotePlayTwcEn == 1)
			        UI_TXPLY_UpdateToCU(UI_TXPLY_STOPCU,NULL,NULL,pUI_RecFilesInfo.uwRecFileSelIdx);
			    else
				    ubUI_RemotePlayTwcEn = 1;
				UI_RemotePlayTrigger(0);
			}
			break;
	}
}
//------------------------------------------------------------------------------
void UI_StartPlayRecordFile(KNL_FldType_t tSimFld,UI_RecPlayDispType_t tPlayDispType, UI_RecFilesInfo_t* pFileInfo)
{
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
	KNL_ROLE tRecRoleNum;
	UI_CamNum_t tRecCamNum;
	uint16_t uwRecFileIndex;

	tUI_RecPlayAct.ubPlayMode = tPlayDispType;

	uwRecFileIndex 				= pFileInfo->uwRecFileSelIdx;
    tUI_PlayAct.tSimFolder     = tSimFld;
	tUI_PlayAct.tRecordFunc  	= KNL_VIDEO_PLAY;
	tUI_PlayAct.pRecordStsNtyCb = UI_StopPlayRecordFile;

	tUI_PlayAct.tPlayDispTye = KNL_DISP_SINGLE;
	tUI_PlayAct.ulVideoPlayIdx[0] = uwRecFileIndex;
	tUI_PlayAct.ubPlayFileNum = 1;
	tRecRoleNum = (KNL_ROLE)pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SrcNum;	
	APP_KNLRoleMap2CamNum(tRecRoleNum, tRecCamNum);
	tUI_RecPlayAct.tAdoPlayCam = tRecCamNum;
    ubPLY_AdoChannelSet(tUI_RecPlayAct.tAdoPlayCam);
	if(KNL_OK == tKNL_ExecRecordFunc(tUI_PlayAct))
	{
		tUI_RecPlayAct.tPlaySts = UI_RECFILE_PLAY;
        tUI_RecPlayAct.tSimFld = tSimFld;
	}
	else
    {      
		tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
        tUI_RecPlayAct.tSimFld = KNL_REAL_FLD;
    }
}
//------------------------------------------------------------------------------
void UI_PlayRecordFile(KNL_FldType_t tSimFld,uint16_t uwRecFileIndex,UI_RecFoldersInfo_t *pFldInfo,UI_RecFilesInfo_t *pFileInfo)
{
    KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};

    if(ubPLY_GetOpMode() == PLY_MODE_R)
    {
        printd(DBG_ErrorLvl, "TX Play Fail,Recording!\n");
        return;
    }

	if(!memcmp(pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName.chExt, "JPG", 3))
	{
	    tUI_PlayAct.tSimFolder     = tSimFld;
		tUI_PlayAct.tRecordFunc  	= KNL_PHOTO_PLAY;
		tUI_PlayAct.pRecordStsNtyCb = NULL;
		memset(&tUI_PlayAct.tPhotoPlayInfo.FileName, 0, sizeof(tUI_PlayAct.tPhotoPlayInfo.FileName));
		memcpy(&tUI_PlayAct.tPhotoPlayInfo.FileName, &pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName, sizeof(pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName));
		tUI_PlayAct.tPhotoPlayInfo.SrcNum =(FS_SRC_NUM)(pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SrcNum + FS_JPG_SRC_0);
		tUI_PlayAct.tPhotoPlayInfo.ulFirstClus = pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.ulFirstClus;
		tUI_PlayAct.tPhotoPlayInfo.ulFileSize = pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.ullFileSize;
		tUI_PlayAct.tPhotoPlayInfo.NoFatChainFlag = pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.NoFatChainFlag;
		if(KNL_OK != tKNL_ExecRecordFunc(tUI_PlayAct))
			printd(DBG_ErrorLvl, "Photo play err!\n");
	}
	else if(!memcmp(pFileInfo->tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName.chExt, "MP4", 3))
        UI_StartPlayRecordFile(tSimFld,UI_RECPLAY_SINGLEVIEW,pFileInfo);
}
//------------------------------------------------------------------------------
// pUI_FS_Event[0] Event
// pUI_FS_Event[1] FS Type
// pUI_FS_Event[2] Sort Mode
// pUI_FS_Event[3] Select Idx
void UI_FS_EventSetting(void *pvEventPtr)
{
    uint8_t *pUI_FS_Event = (uint8_t *)pvEventPtr;
    UI_BUReqCmd_t tUI_FSMsg;
    KNL_Status_t tKNL_SDSts;
    //printd(DBG_CriticalLvl,"FS_EVT=%x %x %x %x %x %x\n",pUI_FS_Event[0],pUI_FS_Event[1],pUI_FS_Event[2],pUI_FS_Event[3],pUI_FS_Event[4],pUI_FS_Event[5]);
    tKNL_SDSts = tKNL_ChkSdCardSts();
    if( tKNL_SDSts == KNL_OK )
    {
        if(ubKNL_ChkFreeSizeValid(KNL_FILE) != 1)
            tKNL_SDSts = KNL_ERR;
        else if((ubKNL_ChkFreeSizeValid(KNL_PHOTO) != 1))
            tKNL_SDSts = KNL_ErrorCap;
		
		if(ubKNL_ChkGarbageUsageValid(KNL_GARBAGE_USAGE_THRESHOLD) != 1)
			tKNL_SDSts = KNL_ERR;
    }
	if( KNL_OK != tKNL_SDSts )
	{
	    printd(DBG_CriticalLvl, "RemoteStorageEmptyFs!\n");
	    pUI_FS_Event[5] = 1;
	}
    else
    {
        switch(pUI_FS_Event[0])
        {
            case UI_FS_ENTER:
                if(pUI_FS_Event[1] == UI_FS_DCIM_FLD)
                {
                    // Stop Record
                    KNL_TXFSSetFldLayer(KNL_TX_FLDSUB1);
                    KNL_TXRecordStop();
                    // Enter DCIM and Sort Folder
                    pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
                    pUI_RecFoldersInfo.uwTotalRecFolderNum = ulKNL_GetSortingFolders(KNL_REAL_FLD,(KNL_SORTMODE_t)pUI_FS_Event[2],&pUI_RecFoldersInfo.tRecFolderInfo[0]);
                    KNL_TXFSSort(1,pUI_FS_Event[1],(uint32_t)&pUI_RecFoldersInfo,sizeof(pUI_RecFoldersInfo));
                }
                else if (pUI_FS_Event[1] == UI_FS_REC_FLD)
                {
                    // Enter Folder and Sort File
                    KNL_TXFSSetFldLayer(KNL_TX_FLDSUB2);
                    pUI_RecFoldersInfo.uwRecFolderSelIdx = (pUI_FS_Event[4]<<8)+pUI_FS_Event[3];
                    pUI_RecFilesInfo.uwRecFileSelIdx = 0;
                    pUI_RecFilesInfo.uwTotalRecFileNum = uwKNL_GetSortingFiles(pUI_RecFoldersInfo.uwRecFolderSelIdx, (KNL_SORTMODE_t)pUI_FS_Event[2], &pUI_RecFilesInfo.tRecFilesInfo[0]);
                    KNL_TXFSSort(1,pUI_FS_Event[1],(uint32_t)&pUI_RecFilesInfo,sizeof(pUI_RecFilesInfo));
                }
                else if (pUI_FS_Event[1] == UI_FS_FILE)
                {
                    // Play File
                    pUI_RecFilesInfo.uwRecFileSelIdx = (pUI_FS_Event[4]<<8)+pUI_FS_Event[3];
                    KNL_TXFSHidenInfo(1,pUI_FS_Event[1],pUI_RecFilesInfo.uwRecFileSelIdx);
                }
            break;

            case UI_FS_EXIT:
                if(pUI_FS_Event[1] == UI_FS_DCIM_FLD)	// Resume Record
                {
                    KNL_SetRecordFunc(KNL_RECORDFUNC_LOOP);
                    KNL_TXRecordResume(KNL_TXREC_BUSY_PLAYSTOPPHOTO,1);
                    KNL_TXFSSetFldLayer(KNL_TX_FLDROOT);
                }
            break;

            case UI_FS_FORMAT:
                FS_MediaFormat(KNL_GetFsMedia());
            break;

            case UI_FS_DEL:
                if (pUI_FS_Event[1] == UI_FS_REC_FLD)
                {
                    // Delet Selected Folder
                }
                else if (pUI_FS_Event[1] == UI_FS_FILE)
                {
                    // Delet Selected File
                }
            break;
        }
    }
    tUI_FSMsg.ubCmd[UI_TWC_TYPE] = UI_REPORT;
    tUI_FSMsg.ubCmd[UI_REPORT_ITEM] = UI_FS_TRIG;
    tUI_FSMsg.ubCmd[UI_REPORT_DATA] = pUI_FS_Event[0];
    tUI_FSMsg.ubCmd[UI_REPORT_DATA+1] = pUI_FS_Event[1];
    tUI_FSMsg.ubCmd[UI_REPORT_DATA+2] = pUI_FS_Event[2];
    tUI_FSMsg.ubCmd[UI_REPORT_DATA+3] = pUI_FS_Event[3];
    tUI_FSMsg.ubCmd[UI_REPORT_DATA+4] = pUI_FS_Event[4];
    tUI_FSMsg.ubCmd[UI_REPORT_DATA+5] = pUI_FS_Event[5];
    tUI_FSMsg.ubCmd_Len = 8;
    UI_SendRequestToPU(NULL, &tUI_FSMsg);
}
//------------------------------------------------------------------------------
// pUI_PLY_Event[0] Event
// pUI_PLY_Event[1] FS Type
void UI_TXPLY_EventSetting(void *pvEventPtr)
{
    uint8_t *pUI_TXPLY_Event = (uint8_t *)pvEventPtr;
    UI_BUReqCmd_t tUI_PLYMsg;
    KNL_Status_t tKNL_SDSts;
    //printd(DBG_CriticalLvl, "PLY_EVT=%x %x %x %x %x %x\n",pUI_TXPLY_Event[0],pUI_TXPLY_Event[1],pUI_TXPLY_Event[2],pUI_TXPLY_Event[3],pUI_TXPLY_Event[4],pUI_TXPLY_Event[5]);
    tUI_PLYMsg.ubCmd[UI_TWC_TYPE] = UI_REPORT;
    tUI_PLYMsg.ubCmd[UI_REPORT_ITEM] = UI_PLY_TRIG;
    tUI_PLYMsg.ubCmd[UI_REPORT_DATA] = pUI_TXPLY_Event[0];
    tUI_PLYMsg.ubCmd[UI_REPORT_DATA+1] = pUI_TXPLY_Event[1];
    tUI_PLYMsg.ubCmd[UI_REPORT_DATA+2] = pUI_TXPLY_Event[2];
    tUI_PLYMsg.ubCmd[UI_REPORT_DATA+3] = pUI_TXPLY_Event[3];
    tUI_PLYMsg.ubCmd[UI_REPORT_DATA+4] = pUI_TXPLY_Event[4];
    tUI_PLYMsg.ubCmd[UI_REPORT_DATA+5] = pUI_TXPLY_Event[5];
    tUI_PLYMsg.ubCmd_Len = 8;
    switch(pUI_TXPLY_Event[0])
    {
        case UI_TXPLY_PLAYCAM:
            tKNL_SDSts = tKNL_ChkSdCardSts();
            if( tKNL_SDSts == KNL_OK )
            {
                if(ubKNL_ChkFreeSizeValid(KNL_FILE) != 1)
                    tKNL_SDSts = KNL_ERR;
                else if((ubKNL_ChkFreeSizeValid(KNL_PHOTO) != 1))
                    tKNL_SDSts = KNL_ErrorCap;
				
				if(ubKNL_ChkGarbageUsageValid(KNL_GARBAGE_USAGE_THRESHOLD) != 1)
					tKNL_SDSts = KNL_ErrorCap;
            }
    		if( KNL_OK != tKNL_SDSts )
    		{
    		    printd(DBG_CriticalLvl, "RemoteStorageEmptyPly!\n");
    		    pUI_TXPLY_Event[5] = 1;
    		    UI_SendRequestToPU(NULL, &tUI_PLYMsg);
    		}
    		else
    		{
    		    pUI_TXPLY_Event[5] = 0;
    		    UI_SendRequestToPU(NULL, &tUI_PLYMsg);
                tUI_RecPlayAct.tPlayDir = (UI_PLY_NextItem_t)pUI_TXPLY_Event[1];
                pUI_RecFilesInfo.uwRecFileSelIdx = (pUI_TXPLY_Event[4]<<8)+pUI_TXPLY_Event[3];
                osDelay(200);
                UI_PlayRecordFile(KNL_REAL_FLD,pUI_RecFilesInfo.uwRecFileSelIdx,&pUI_RecFoldersInfo,&pUI_RecFilesInfo);
            }
        break;
        
        case UI_TXPLY_STOPCAM:
            UI_SendRequestToPU(NULL, &tUI_PLYMsg);
            if(UI_ChkRemotePlayTrigger() == 0 && tKNL_GetRecordFunc() == KNL_VIDEO_PLAY)
            {
                osDelay(100);
                UI_RemotePlayTrigger(1);
                UI_RemotePlayStopTwc();
                KNL_VideoPlayStop();
            }
        break;

        case UI_TXPLY_JMPPAUSE:
            if(ubPLY_GetPauseStatus() == PLY_PAUSE_OFF)
            {
                if(ubPLY_Pause(PLY_PAUSE_ON) != 0)
                    pUI_TXPLY_Event[1] = 1;
            }
            else if(ubPLY_GetPauseStatus() == PLY_PAUSE_ON)
            {
                if(ubPLY_Pause(PLY_PAUSE_OFF) != 0)
                    pUI_TXPLY_Event[1] = 1;
            }
            UI_SendRequestToPU(NULL, &tUI_PLYMsg);
        break;

        case UI_TXPLY_JMPBW:
            if(ubPLY_Jump(PLY_JUMP_BWD) != 0)
                pUI_TXPLY_Event[1] = 1;
            UI_SendRequestToPU(NULL, &tUI_PLYMsg);
        break;

        case UI_TXPLY_JMPFW:
            if(ubPLY_Jump(PLY_JUMP_FWD) != 0)
                pUI_TXPLY_Event[1] = 1;
            UI_SendRequestToPU(NULL, &tUI_PLYMsg);
        break;
    }
}
//------------------------------------------------------------------------------
void UI_TXFS_UpdateToCU(UI_FS_EventItem_t tEvent,uint8_t ubData1,uint8_t ubData2,uint16_t uwIdx)
{
    UI_BUReqCmd_t   tUI_ReqMsg;
    tUI_ReqMsg.ubCmd[UI_TWC_TYPE]     = UI_REPORT;
    tUI_ReqMsg.ubCmd[UI_SETTING_ITEM] = UI_FS_TRIG;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA] = tEvent;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+1] = ubData1;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+2] = ubData2;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+3] = (uwIdx&0x00FF);
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+4] = (uwIdx&0xFF00)>>8;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+5] = 0;
    tUI_ReqMsg.ubCmd_Len  = 8;    
    UI_SendRequestToPU(NULL, &tUI_ReqMsg);
}
//------------------------------------------------------------------------------
void UI_TXPLY_UpdateToCU(UI_TXPLY_EventItem_t tEvent,uint8_t ubData1,uint8_t ubData2,uint16_t uwIdx)
{
    UI_BUReqCmd_t   tUI_ReqMsg;
    tUI_ReqMsg.ubCmd[UI_TWC_TYPE]     = UI_REPORT;
    tUI_ReqMsg.ubCmd[UI_SETTING_ITEM] = UI_PLY_TRIG;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA] = tEvent;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+1] = ubData1;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+2] = ubData2;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+3] = (uwIdx&0x00FF);
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+4] = (uwIdx&0xFF00)>>8;
    tUI_ReqMsg.ubCmd[UI_SETTING_DATA+5] = 0;
    tUI_ReqMsg.ubCmd_Len  = 8;    
    UI_SendRequestToPU(NULL, &tUI_ReqMsg);
}
void UI_RemotePlayStopTwc(void)
{
    ubUI_RemotePlayTwcEn = 0;
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
