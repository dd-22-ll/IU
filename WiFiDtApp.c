/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		WiFiDtApp.c
	\brief		WiFiDtApp function
	\author		Chris 
	\version	0.20
	\date		2021/09/29
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#if (defined(S2019A) && defined(RVCS_APP))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "WiFiDtApp.h"
#include "ADO_API.h"
#include "VDO.h"
#ifdef OP_STA
#include "SEN.h"
#include "RC.h"
#include "ISP_API.h"
#include "UI_VBMBU.h"

#if APP_BLE_SN9380_FUNC_ENABLE
#include "WDT.h"
#include "Buzzer.h"
#include "UI_UART2.h"
#include "UI_CMD.h"
#include "APP_BLE.h" 
#endif
#endif	//! End of #ifdef OP_STA
#ifdef OP_AP
#include "WiFiBdgApp.h"
#ifdef BSP_D_SNCC71_GM8285C_RX_V2
#include "UI_RVCSCU[WSVGA].h"
#endif
#endif	//! End of #ifdef OP_AP	

#define DT_TYPE_AUD32	 		0x55

void WiFiDt_Thread(void const *pvParameters);	
	
uint8_t ubUpdateFWinfo = 0;
uint8_t ubDT_AppSyncPktFlag = 0;
uint8_t ubDT_AppCnnTimeOSts = 0;
uint16_t ubSyncTimerCnt = 0;

uint8_t  g_ubFwVersion[E_FW_VER_LEN] = {0};
uint8_t  ubDesIP[4] = {192, 168, 0, 2};

uint8_t ubIsBLE_ProcFOTA = 0;
uint8_t ubUI_PushTalkEnable = 0;
#ifdef OP_STA
static uint32_t ulPTTimeStamp = 0;
#endif

osMutexId xSemaphoreUpdateMutex = NULL;
osMutexId xSemaphoreSysMutex = NULL;

osThreadId osWiFiDt_MainThdId = NULL;
osMessageQId *pUIApp_QRptLinkStatus = NULL;
#if defined(OP_AP) || (defined (OP_STA) && (E_RVCS_ALL_I_FRM == 0))
static uint8_t ubWiFiDtApp_PbCnt = 0;
#endif
//------------------------------------------------------------------------------
void WiFiDt_AppInit(osMessageQId *pLinkRptQue) 
{
	WiFiDt_CbFunc_t tRegCb;

	ubUpdateFWinfo 			= 0;
	WiFiDt_UpSynCnnSts(0);
	ubIsBLE_ProcFOTA 	= 0;
	ubUI_PushTalkEnable		= 0;

	tRegCb.pvTwcProc 	= WiFiDt_AppTwcProc;
	tRegCb.pvAdoProc	= NULL;
#if (APP_ADO_FUNC_ENABLE == 1)
	tRegCb.pvAdoProc 	= WiFiDt_AudioProc;	
#endif
	tRegCb.pvBTProc  	= ubWiFiDt_BLEProc;
	tRegCb.pvFwStsProc 	= ubWiFiDt_FwStatusProc;
	tRegCb.pvRcRstFunc  = WiFiDt_ResetRateCtrl;
	WiFiDt_RegCbFunc(tRegCb);
	pUIApp_QRptLinkStatus = pLinkRptQue;

	if(NULL == xSemaphoreUpdateMutex)
	{
		osMutexDef(xSemaphoreUpdateMutex);
		xSemaphoreUpdateMutex = osMutexCreate(osMutex(xSemaphoreUpdateMutex)); 
	}	
	if(NULL == xSemaphoreSysMutex)
	{
		osMutexDef(xSemaphoreSysMutex);
		xSemaphoreSysMutex = osMutexCreate(osMutex(xSemaphoreSysMutex)); 
	}	
	
	if(NULL == osWiFiDt_MainThdId)
	{
		osThreadDef(WiFiDtThd, WiFiDt_Thread, osPriorityNormal/*osPriorityAboveNormal*/, 1, 4096);
		osWiFiDt_MainThdId = osThreadCreate(osThread(WiFiDtThd), NULL);
	}
#if (APP_ADO_FUNC_ENABLE == 1)
	ADO_SetDacR2RVol(R2R_VOL_n3DB);   
#endif

	//! For 510 UART FWU SN9380
	#if APP_BLE_SN9380_FUNC_ENABLE	
	tBLE_RegFwuWriteCbFunc(UART2_Send);	
	#endif  
}

//------------------------------------------------------------------------------
void WiFiDt_AppUninit(void)
{
	WiFiDt_CbFunc_t tUnCb;
	uint8_t ubDstMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	if(osWiFiDt_MainThdId)
	{
		osThreadTerminate(osWiFiDt_MainThdId);
		osWiFiDt_MainThdId = NULL;
	}

	sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);
	osDelay(70);	
#if (MANUAL_DT_ENCRY_TYPE == 1) 
	Hostapd_onUninit();
#endif

	if(xSemaphoreUpdateMutex)
	{
		osMutexDelete(xSemaphoreUpdateMutex);
		xSemaphoreUpdateMutex = NULL;
	}	
	if(xSemaphoreSysMutex)
	{
		osMutexDelete(xSemaphoreSysMutex);
		xSemaphoreSysMutex = NULL;
	}	
#if  (APP_BLE_SN9380_FUNC_ENABLE == 1) 
	UI_UART2_Uninit();
#endif	
	if(pUIApp_QRptLinkStatus)
		pUIApp_QRptLinkStatus = NULL;
	tUnCb.pvTwcProc 	= NULL;
	tUnCb.pvBTProc  	= NULL;
	tUnCb.pvFwStsProc 	= NULL;
	tUnCb.pvAdoProc		= NULL;
	tUnCb.pvRcRstFunc	= NULL;
	WiFiDt_RegCbFunc(tUnCb);
}

//------------------------------------------------------------------------------
void WiFiDt_ResetRateCtrl(uint8_t ubRstFlag)
{
#ifdef OP_STA
	RC_DtModeReset(0, ubRstFlag);
	KNL_SetReEncIfrmFlag(TRUE);
#endif
}

//----------------------------------------------------------
void WiFiDt_UpSynCnnSts(uint8_t ubStatus)
{
	osMutexWait(xSemaphoreUpdateMutex, osWaitForever);
	ubDT_AppSyncPktFlag = ubStatus;
	osMutexRelease(xSemaphoreUpdateMutex);
}
//------------------------------------------------------------------------------
uint8_t ubWiFiDt_VdoStartProc(DP* pPtr, uint8_t *SrcIP, uint8_t *SrcMac, uint8_t ubLinked)
{
	uint8_t ubData[8] = {0};
	uint8_t ubRet = 0;
	uint8_t ubErrorFlag = 0;
	uint8_t i = 0;
	uint8_t *ubTwcDataAddr;
	uint32_t ulFrmSize;

#ifdef OP_STA
	UI_BUStatus_t *pUI_DevSts;
#else
	UI_CUSetting_t *pUI_DevSts;
#endif
	pUI_DevSts = pUI_DevSts;
	pUI_DevSts = pUI_GetDevSetting();	
	ulFrmSize  = ulFrmSize;
	if (ubLinked == 1) {
		ubTwcDataAddr = (uint8_t *)pPtr->DP_Hdr.TwcData;

		printd(DBG_ErrorLvl, "VDO_ST[%d], CnnTimeO[%d] EN[%d] L[%d], DA[", ubLinked, ubDT_AppCnnTimeOSts,
			*(ubTwcDataAddr+APP_VDO_OFFSET_PWD_ENABLE), 
			*(ubTwcDataAddr+APP_VDO_OFFSET_PWD_LEN));
		for(i = 0; i<*(ubTwcDataAddr+APP_VDO_OFFSET_PWD_LEN); i++) {
			printd(DBG_ErrorLvl, "%c", *(ubTwcDataAddr+APP_VDO_OFFSET_PWD_DATA+i));
		}
		printd(DBG_ErrorLvl, "]\n");
	}
	else {
		ubTwcDataAddr = (uint8_t *)&pPtr->DP_Hdr.TwcData[4];

		printd(DBG_ErrorLvl, "VDO_ST[%d], CnnTimeO[%d] DA[", ubLinked, ubDT_AppCnnTimeOSts);
		for(i = 0; i<12; i++) {
			printd(DBG_ErrorLvl, "%X ", *(ubTwcDataAddr+i));
		}
		printd(DBG_ErrorLvl, "]\n");
		printd(DBG_ErrorLvl, "VDO_ST[P], PWD[");
		for(i = 0; i<*(ubTwcDataAddr+APP_VDO_OFFSET_PWD_LEN); i++) {
			printd(DBG_ErrorLvl, "%X ", *(ubTwcDataAddr+APP_VDO_OFFSET_PWD_DATA+i));
		}
		printd(DBG_ErrorLvl, "]\n");
	}
	
#if (MANUAL_DT_ENCRY_TYPE == 1)  
#if 0
	// to check the password
	ubErrorFlag = 0;
	if (*(ubTwcDataAddr+APP_VDO_OFFSET_PWD_ENABLE) == 1) {
		if (pUI_DevSts->tCamsSecurity.ubSec_PWDLen == *(ubTwcDataAddr+APP_VDO_OFFSET_PWD_LEN) ) {
			memcpy(ubData, MANUAL_VDO_ST_PWD, E_DT_WPA_PWD_LEN);
			for (i = 0; i<pUI_DevSts->tCamsSecurity.ubSec_PWDLen; i++) {
				if (ubData[i] != *(ubTwcDataAddr+APP_VDO_OFFSET_PWD_DATA+i) ) {
					ubErrorFlag = 1;
					break;
				}
			}
		}
		else 
			ubErrorFlag = 2;
	}
	else 
		ubErrorFlag = 3;
#endif
#else
	// to check the password
	ubErrorFlag = 0;
	if (*(ubTwcDataAddr+APP_VDO_OFFSET_PWD_ENABLE) ==  E_DT_PWD_SET_OLD_FW) {
		if (pUI_DevSts->tCamsSecurity.ubSec_PWDLen == *(ubTwcDataAddr+APP_VDO_OFFSET_PWD_LEN) ) {
			if (pUI_DevSts->tCamsSecurity.ubSec_PWDLen > 0) {
				for (i = 0; i<pUI_DevSts->tCamsSecurity.ubSec_PWDLen; i++) {
					if (pUI_DevSts->tCamsSecurity.ubSec_PWD[i] != *(ubTwcDataAddr+APP_VDO_OFFSET_PWD_DATA+i) ) {
						ubErrorFlag = 1;
						break;
					}
				}
			}
		}
		else 
			ubErrorFlag = 1;
	}
	else 
		ubErrorFlag = 1;
#endif
	if (ubErrorFlag > 0) {  // VIDEO_START CMD --> Fail
		ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_STOP_CERTIFICATE_FAIL);
#ifdef OP_STA	
		WiFiDt_UpAppConnInfo(0, SrcMac, ubTwcDataAddr);
#endif
		printd(DBG_ErrorLvl, "-->[VDO_ST] fail[%d] F[%d], Link[%d].\n", E_TWC_STOP_CERTIFICATE_FAIL, ubErrorFlag, ubLinked);

		ubData[0] = E_TWC_STOP_CERTIFICATE_FAIL;
		WiFiDt_SendTwc(UI_APPTWC_VDO_STOP, TARGET_STA0, SrcIP, ubData, 1, 1);
		return 0;
	}
	
	osMutexWait(xSemaphoreSysMutex, osWaitForever);
	#ifdef OP_STA
	WiFiDt_SetDispFps(VDO_FRC_FPS);
	#endif

	if (ubLinked == 1) 
		ubRet = ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK);
	else
		ubRet = ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_RECONNECT);
	if (ubRet == 0) {
		osMutexRelease(xSemaphoreSysMutex);
		ubData[0] = E_TWC_STOP_NO_RESOURCE_CONNECT;
		WiFiDt_SendTwc(UI_APPTWC_VDO_STOP, TARGET_STA0, SrcIP, ubData, 1, 3);
		return 0;
	}
	else if (ubRet == 1) {

	}
	else {						
		osMutexRelease(xSemaphoreSysMutex);
		ubData[0] = E_TWC_STOP_NO_RESOURCE_CONNECT;
		WiFiDt_SendTwc(UI_APPTWC_VDO_STOP, TARGET_STA1, SrcIP, ubData, 1, 3);	
		return 0;
	}

#if (APP_BLE_SN9380_FUNC_ENABLE == 1) 
	printd(DBG_ErrorLvl, "VdoStartProc, ConnSts[%d] Rst[%d].\n", UIcmd_GetConnSts(), UIcmd_GetRstFunc());
	if (UIcmd_GetConnSts() == 1)
		UIcmd_SetRstFunc(1);
	else
		if (UIcmd_GetRstFunc() == 1)
			UI_UpdateWORsts(E_EVENT_RST, 1, E_510_PWN_ON);
#endif

	KNL_sPRFLinkReportFunc(sPRF_AP, sPRF_LINK);
	ubDT_AppCnnTimeOSts = 0;
	memcpy(ubDesIP, SrcIP, 4);
	#ifdef OP_STA
	ulPTTimeStamp = 0;
	WiFiDt_ResetRateCtrl(1);
	#else
	osDelay(20);
	#endif				
	osMutexRelease(xSemaphoreSysMutex);
	return 1;
}

#ifdef OP_STA
#if (APP_PLYBK_ENABLE == 1)   
void WiFiDt_StartKNLVideo()
{
	if( tKNL_GetRecordFunc() == KNL_PHOTO_PLAY )
    {
    	KNL_ROLE tPlayRole;
        KNL_SRC tVdoSrcNum;
        printd(DBG_Debug3Lvl, "===>>> UI_FS_EXIT, PhotoPlyStop!\n");
        KNL_ResetVdoProc();
        for (tPlayRole = KNL_STA1; tPlayRole < DISPLAY_MODE; tPlayRole++)
        {
        	tVdoSrcNum = (KNL_SRC)(KNL_SRC_1_MAIN + tPlayRole);
            KNL_VdoStart(tVdoSrcNum);
        }

        for (tPlayRole = KNL_STA1; tPlayRole < DISPLAY_MODE; tPlayRole++)
        {
        	tVdoSrcNum = (KNL_SRC)(KNL_SRC_1_AUX + tPlayRole);
            KNL_VdoStart(tVdoSrcNum);
        }                     
        //if(pUI_FS_Event[1] == UI_FS_DCIM_FLD)    // Resume Record
        {
        	KNL_TXRecordResume(KNL_TXREC_BUSY_PLAYSTOPPHOTO,1);
            KNL_TXFSSetFldLayer(KNL_TX_FLDROOT);
        }
        KNL_PhotoPlayStop();
    }
    else
    {
        KNL_TXRecordResume(KNL_TXREC_BUSY_NONE,0);
        KNL_TXFSSetFldLayer(KNL_TX_FLDROOT);
    }
}
#endif

void fill_UITWC_SETTING(UI_BUStatus_t *pUI_DevSts, uint8_t * pSetting_Buf)
{
	uint8_t ubMirrorflip = 0;
				
	if ( (pUI_DevSts->tCamImgFlip == CAMIMGFLIP_ENABLE)&&(pUI_DevSts->tCamImgMirror == CAMIMGMIRROR_ENABLE) ) {
		ubMirrorflip = VDO_SEN_MIRROR_FLIP;
	}
	else if (pUI_DevSts->tCamImgFlip == CAMIMGFLIP_ENABLE) {
		ubMirrorflip = VDO_SEN_FLIP;
	}
	else if (pUI_DevSts->tCamImgMirror == CAMIMGMIRROR_ENABLE) {
		ubMirrorflip = VDO_SEN_MIRROR;
	}
	else {
		ubMirrorflip = VDO_SEN_NOCHANGE;
	}

	*(pSetting_Buf+0) = ubMirrorflip;

	*(pSetting_Buf+1) = pUI_DevSts->tCamFlicker+1;

	*(pSetting_Buf+2) = pUI_DevSts->tCamColorParam.ubColorBL;

	*(pSetting_Buf+3) = pUI_DevSts->tCamColorParam.ubColorHue;

	*(pSetting_Buf+4) = pUI_DevSts->tCamColorParam.ubColorContrast;

	*(pSetting_Buf+5) = pUI_DevSts->tCamColorParam.ubColorSaturation;
}

void send_VDO_START_ack(UI_BUStatus_t *pUI_DevSts, uint8_t *SrcIP)
{
	uint8_t ubAckBuf[7] = {0};	// 0:playback, 1~6
	#if (APP_PLYBK_ENABLE == 1)
	KNL_Status_t tKNL_SDSts;
	#endif

	#if (APP_PLYBK_ENABLE == 1)
	ubAckBuf[0] = 0;
	ubAckBuf[0] |= 0x01;
	tKNL_SDSts = tKNL_ChkSdCardSts();
	if( tKNL_SDSts == KNL_OK )
	ubAckBuf[0] |= 0x02;
	#else
	ubAckBuf[0] = 0;
	#endif
	fill_UITWC_SETTING(pUI_DevSts, &ubAckBuf[1]);
	printd(DBG_ErrorLvl, "[VDO START ACK]: [0x%x] [0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n", 
		ubAckBuf[0], ubAckBuf[1], ubAckBuf[2], ubAckBuf[3], ubAckBuf[4], ubAckBuf[5], ubAckBuf[6]);
	WiFiDt_SendTwc(UI_APPTWC_VDO_START, TARGET_STA0, SrcIP, ubAckBuf, 7, 1);
}
#endif
//------------------------------------------------------------------------------
#define E_CAMSTS_LEN	4
static uint8_t ubHBBuf[E_FW_VER_LEN+E_PF_SSID_LEN+E_PB_MAX_LAN+E_CAMSTS_LEN] = {0};
static uint8_t ubQuerySSIDIsOK = 0;
void WiFiDt_AppTwcProc(DP* pPtr, uint8_t *SrcIP, uint8_t *SrcMac)
{
#ifdef OP_STA
	static uint8_t ubVdoStartAckSendTime = 0;
#endif
	uint32_t ulFrmSize;
	uint8_t *ubTwcDataAddr;
#ifdef OP_STA
	UI_BUStatus_t *pUI_DevSts;
#else
	UI_CUSetting_t *pUI_DevSts;
#endif
	pUI_DevSts = pUI_DevSts;
	pUI_DevSts = pUI_GetDevSetting();	
	ulFrmSize  = ulFrmSize;
	ubTwcDataAddr = (uint8_t *)pPtr->DP_Hdr.TwcData;
/*	
	if ( (ubWiFiDt_GetRssi(0) < 187) ||(ubWiFiDt_QueryAppConnectStatus() == 0) ||(ubSyncTimerCnt > E_BUC_APPS_SYNC_MIN_TIMEOUT)){
		if (pPtr->DP_Hdr.TwcOpc != UI_APPTWC_PROP_BEACON)
			printd(DBG_ErrorLvl, "Twc, O[%X]\n", pPtr->DP_Hdr.TwcOpc);
	}
*/
	switch(pPtr->DP_Hdr.TwcOpc)	// check op code
	{
		case UI_APPTWC_VDO_START:
		{
			printd(DBG_ErrorLvl, "VDO_ST[S]: CnnSts[%d %d %d] Scnt[%d]\n", ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP), ubWiFiDt_QueryAppConnectStatus(), ubDT_AppCnnTimeOSts, ubSyncTimerCnt);
			if (/*(ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))&&*/(ubWiFiDt_QueryAppConnectStatus())) {		
			//if ((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))&&(ubWiFiDt_QueryAppConnectStatus())) {		
				printd(DBG_ErrorLvl, "VDO_ST[S1]: CnnSts(%d)\n", ubWiFiDt_QueryAppConnectStatus());

				if (ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_STOP_NO_RESOURCE_CONNECT) == 0) {	
					uint8_t ubData[4] = {0};

					ubData[0] = E_TWC_STOP_NO_RESOURCE_CONNECT;
					WiFiDt_SendTwc(UI_APPTWC_VDO_STOP, TARGET_STA1, SrcIP, ubData, 1, 3);
				}
#ifdef OP_STA
				if (ubVdoStartAckSendTime == 0) {
					ubVdoStartAckSendTime = 7;
					// Start KNL video to handle the case of losting exit commands of PHOTO playabck
				  #if (APP_PLYBK_ENABLE == 1)	
					WiFiDt_StartKNLVideo();
				  #endif
				}
#endif
			}	
			else {
#ifdef OP_STA
				 ubTWC_WiFiDt_GetData(TWC_STA1, TWC_UI_SETTING, SrcIP, NULL); 	
				 if (ubWiFiDt_VdoStartProc( pPtr, SrcIP, SrcMac, 1) == 1)
				 {
					 if (ubVdoStartAckSendTime == 0) {
					 	ubVdoStartAckSendTime = 7;
						// Start KNL video to handle the case of losting exit commands of PHOTO playabck
					  #if (APP_PLYBK_ENABLE == 1)
						WiFiDt_StartKNLVideo();
					  #endif
					 }
				 }
#else
				 ubWiFiDt_VdoStartProc( pPtr, SrcIP, SrcMac, 1);
#endif
			}
			break;
		}	
		case UI_APPTWC_VDO_STOP:
			printd(DBG_ErrorLvl, "VDO_SP[0]: CnnSts(%d), LS(%d), M[%02X:%02X:%02X:%02X:%02X:%02X]\n", 
				ubWiFiDt_QueryAppConnectStatus(), ubKNL_GetCommLinkStatus(KNL_MASTER_AP),
				SrcMac[0], SrcMac[1], SrcMac[2], SrcMac[3], SrcMac[4], SrcMac[5]);

			#ifdef OP_STA
			ubVdoStartAckSendTime = 0;
			#endif

			osMutexWait(xSemaphoreSysMutex, osWaitForever);
			if (ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK) == 1) {

				printd(DBG_ErrorLvl, "=PV STOP=\n");
				KNL_sPRFLinkReportFunc(sPRF_AP, sPRF_LOST_LINK);
			}
			osMutexRelease(xSemaphoreSysMutex);
			break;

		case UI_APPTWC_ADO_START:
			printd(DBG_ErrorLvl, "ADO START\n\r");
			break;
			
		case UI_APPTWC_ADO_STOP:
			printd(DBG_ErrorLvl, "ADO STOP\n\r");
			break;
		
		case UI_APPTWC_ADO_RESTART:
			printd(DBG_ErrorLvl, "ADO RESTART\n\r");
			break;
		case UI_APPTWC_FRM_OK:					
		case UI_APPTWC_FRM_LOST:
			WiFiDt_UpSynCnnSts(1);
#ifdef OP_STA
			ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK);
#endif
			break;
		case UI_APPTWC_STA_ALIVE:	// 100ms
			if (ubWiFiDt_QueryAppConnectStatus()) {				
				WiFiDt_UpSynCnnSts(1);
				ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK);
			}
			else {
				printd(DBG_Debug1Lvl, "- ALIVE,  LS[%d %d] CnnTo[%d] D[%x %x]\r\n", ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP), 
					ubWiFiDt_QueryAppConnectStatus(), ubDT_AppCnnTimeOSts, pPtr->DP_Hdr.TwcData[2], pPtr->DP_Hdr.TwcData[3]);
				if ((pPtr->DP_Hdr.TwcData[2] == 0x53)&&(pPtr->DP_Hdr.TwcData[3] == 0x4E)&&(ubDT_AppCnnTimeOSts == 1) ) { // new Apps format.
					pPtr->DP_Hdr.TwcOpc = UI_APPTWC_VDO_START;
					ubWiFiDt_VdoStartProc( pPtr, SrcIP, SrcMac, 0);
				}
			}
			break;	
#ifdef OP_STA
		case UI_APPTWC_NOTIFY_FW_UPDATE:	// 0x14
			{
				//printd(DBG_ErrorLvl, "%s(): UI_APPTWC_NOTIFY_FW_UPDATE \n", __FUNCTION__);
				osMutexWait(xSemaphoreSysMutex, osWaitForever);
				if (ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK) == 1) {	
					LINK_REPORT tLinkRpt;
					tLinkRpt.tRole 	 = (LINK_ROLE)KNL_MASTER_AP;
					tLinkRpt.tStatus = (LINK_STATUS)KNL_LOST_LINK;
					osMessagePut(*pUIApp_QRptLinkStatus, &tLinkRpt, 0);  

					WiFiDt_StartUpdateFw(ubHBBuf);
				}
				osMutexRelease(xSemaphoreSysMutex);
			}
			break;
		case UI_APPTWC_VDO_PARM_MIRROR_FLIP: 	// 0x30,
			{
				uint8_t ubMirrorflip = 0;
				
				printd(DBG_Debug3Lvl, "[MIRROR_FLIP] data[%d : %d]\n", *(ubTwcDataAddr), *(ubTwcDataAddr+1));
				if (*(ubTwcDataAddr) == E_OPCODE_SET) {
					
					printd(DBG_ErrorLvl, "[MIRROR_FLIP]: SET D[%d/%d: %d]\n", pUI_DevSts->tCamImgFlip, 
							pUI_DevSts->tCamImgMirror, *(ubTwcDataAddr+1));
					if ( (pUI_DevSts->tCamImgFlip == CAMIMGFLIP_ENABLE)&&(pUI_DevSts->tCamImgMirror == CAMIMGMIRROR_ENABLE) ) {
						ubMirrorflip = VDO_SEN_MIRROR_FLIP;
					}
					else if (pUI_DevSts->tCamImgFlip == CAMIMGFLIP_ENABLE) {
						ubMirrorflip = VDO_SEN_FLIP;
					}
					else if (pUI_DevSts->tCamImgMirror == CAMIMGMIRROR_ENABLE) {
						ubMirrorflip = VDO_SEN_MIRROR;
					}
					else {
						ubMirrorflip = VDO_SEN_NOCHANGE;
					}

					if (ubMirrorflip != *(ubTwcDataAddr+1)) {
						if (*(ubTwcDataAddr+1) == VDO_SEN_MIRROR_FLIP) {
							pUI_DevSts->tCamImgFlip = CAMIMGFLIP_ENABLE;
							pUI_DevSts->tCamImgMirror = CAMIMGMIRROR_ENABLE;
						}
						else if (*(ubTwcDataAddr+1) == VDO_SEN_FLIP) {
							pUI_DevSts->tCamImgFlip = CAMIMGFLIP_ENABLE;
							pUI_DevSts->tCamImgMirror = CAMIMGMIRROR_DISABLE;
						}
						else if (*(ubTwcDataAddr+1) == VDO_SEN_MIRROR) {
							pUI_DevSts->tCamImgFlip = CAMIMGFLIP_DISABLE;
							pUI_DevSts->tCamImgMirror = CAMIMGMIRROR_ENABLE;
						}
						else {	// VDO_SEN_NOCHANGE
							pUI_DevSts->tCamImgFlip = CAMIMGFLIP_DISABLE;
							pUI_DevSts->tCamImgMirror = CAMIMGMIRROR_DISABLE;
						}
						ISP_SetMirrorFlip(pUI_DevSts->tCamImgMirror, pUI_DevSts->tCamImgFlip);
					}
				}
				else {
					if ( (pUI_DevSts->tCamImgFlip == CAMIMGFLIP_ENABLE)&&(pUI_DevSts->tCamImgMirror == CAMIMGMIRROR_ENABLE) ) {
						ubMirrorflip = VDO_SEN_MIRROR_FLIP;
					}
					else if (pUI_DevSts->tCamImgFlip == CAMIMGFLIP_ENABLE) {
						ubMirrorflip = VDO_SEN_FLIP;
					}
					else if (pUI_DevSts->tCamImgMirror == CAMIMGMIRROR_ENABLE) {
						ubMirrorflip = VDO_SEN_MIRROR;
					}
					else {
						ubMirrorflip = VDO_SEN_NOCHANGE;
					}
					
					printd(DBG_Debug3Lvl, "[MIRROR_FLIP]: Get, D[%d]\n", ubMirrorflip);
					WiFiDt_SendTwc(UI_APPTWC_VDO_PARM_MIRROR_FLIP, TARGET_STA0, SrcIP, &ubMirrorflip, 1, 1);
				}
			}
			break;	
		case UI_APPTWC_VDO_SENSOR_POWER_FREQ: 	// 0x40,	
			if (*(ubTwcDataAddr) == E_OPCODE_SET) {
				if ( (*(ubTwcDataAddr) < VDO_SEN_POWER_FREQ_50HZ)||(*(ubTwcDataAddr) > VDO_SEN_POWER_FREQ_60HZ) ) {
					printd(DBG_InfoLvl, "[SENSOR_POWER_FREQ]: Unknow param[%d]\n", *(ubTwcDataAddr));
					break;	
				}
	
				printd(DBG_ErrorLvl, "[SENSOR_POWER_FREQ]: SET D[%d : %d]\n", pUI_DevSts->tCamFlicker, *(ubTwcDataAddr+1));
				if (pUI_DevSts->tCamFlicker != (UI_CamFlicker_t)(*(ubTwcDataAddr+1))) {
					pUI_DevSts->tCamFlicker = (UI_CamFlicker_t)(*(ubTwcDataAddr+1));
					ISP_SetAePwrFreq((CAMFLICKER_50HZ == pUI_DevSts->tCamFlicker)?SENSOR_PWR_FREQ_50HZ:SENSOR_PWR_FREQ_60HZ);
					printd(DBG_InfoLvl, "		=> Flicker: %s\n", (CAMFLICKER_50HZ == pUI_DevSts->tCamFlicker)?"50Hz":"60Hz");
					UI_UpdateDevStatusInfo();
				}
			}
			else {
				printd(DBG_Debug3Lvl, "[SENSOR_POWER_FREQ]: Get, D[%d]\n", pUI_DevSts->tCamFlicker+1);
				WiFiDt_SendTwc(UI_APPTWC_VDO_SENSOR_POWER_FREQ, TARGET_STA0, SrcIP, (uint8_t *)pUI_DevSts->tCamFlicker+1, 1, 1);
			}	
			break;
#endif	//! End of #ifdef OP_STA
		case UI_APPTWC_PROP_BEACON: 			// 0x7F
			{
				WiFiDt_UpSynCnnSts(1);
				if (ubQuerySSIDIsOK == 0) {
					ubHBBuf[0] = sizeof(UI_CODE_VERSION) - 1;
					memcpy(&ubHBBuf[1], UI_CODE_VERSION, sizeof(UI_CODE_VERSION));

					if (ubWiFiDt_GetSsid(&ubHBBuf[E_FW_VER_LEN]) > 0) 
						ubQuerySSIDIsOK = 1;					
				} 

				if (ubUpdateFWinfo == 1) { 
					memcpy(ubHBBuf, g_ubFwVersion, E_FW_VER_LEN);
					ubUpdateFWinfo = 0;
				}
#if  (APP_BLE_SN9380_FUNC_ENABLE == 1) 
				// to check if update the BLE version
				UIcmd_GetBLEVersion(&ubHBBuf[E_FW_VER_LEN+E_PF_SSID_LEN], E_PB_MAX_LAN);
#endif
			#if E_CAMSTS_LEN
				KNL_ROLE tSta;
				uint8_t ubLink = 0;
				for(tSta = KNL_STA1; tSta <= KNL_STA4; tSta++)
				{
				#ifdef OP_AP
					ubLink = ubKNL_GetCommLinkStatus(tSta);
				#endif
					ubHBBuf[E_FW_VER_LEN+E_PF_SSID_LEN+E_PB_MAX_LAN+tSta] = ubLink;
				}
			#endif
				if (ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK) == 1) 
				{
				#ifdef OP_AP
					if(!(ubWiFiDtApp_PbCnt++ % 4))
				#elif defined (OP_STA) && (E_RVCS_ALL_I_FRM == 0)
					if(!(ubWiFiDtApp_PbCnt++ % 5))
				#endif
					{
						WiFiDt_SendTwc(UI_APPTWC_PROP_BEACON, TARGET_STA0, SrcIP, ubHBBuf, (E_FW_VER_LEN+E_PF_SSID_LEN+E_PB_MAX_LAN+E_CAMSTS_LEN), 1);
					#ifdef OP_STA	
						if (ubVdoStartAckSendTime > 0)
						{
							printd(DBG_ErrorLvl, "==>> vdoSnd:%d\n", ubVdoStartAckSendTime);
							ubVdoStartAckSendTime--;
							send_VDO_START_ack(pUI_DevSts, SrcIP);
						}
					#endif	
				#if defined(OP_AP) || (defined (OP_STA) && (E_RVCS_ALL_I_FRM == 0))
						ubWiFiDtApp_PbCnt = 1;
				#endif
					}
				}
/*
				if (ubWiFiDt_QueryAppConnectStatus() == 0) {
					uint8_t ubData[2] = {0};

					printd(DBG_ErrorLvl, "=> PROP_BEACON, AppCnn[%d]<=\r\n", ubWiFiDt_QueryAppConnectStatus() );
					ubData[0] = 0xA;
					WiFiDt_SendTwc(UI_APPTWC_STA_ALIVE_AGAIN, TARGET_STA0, SrcIP, ubData, 1, 2);			
				}
*/
			}
			break;
#ifdef OP_STA
		case UI_APPTWC_VDO_IQ_BRIGHTNESS:   	// 0x80,
			if (*(ubTwcDataAddr) == E_OPCODE_SET) {
				printd(DBG_ErrorLvl, "[IQ_BRIGHTNESS]: SET D[%d : %d]\n", pUI_DevSts->tCamColorParam.ubColorBL, *(ubTwcDataAddr+1));
				if (pUI_DevSts->tCamColorParam.ubColorBL != *(ubTwcDataAddr+1)) {
					pUI_DevSts->tCamColorParam.ubColorBL = *(ubTwcDataAddr+1);
					ISP_SetIQBrightness(pUI_DevSts->tCamColorParam.ubColorBL);
					UI_UpdateDevStatusInfo();
				}			
			}
			else {
				printd(DBG_Debug3Lvl, "[IQ_BRIGHTNESS]: Get, D[%d]\n", pUI_DevSts->tCamColorParam.ubColorBL);
				WiFiDt_SendTwc(UI_APPTWC_VDO_IQ_BRIGHTNESS, TARGET_STA0, SrcIP, &pUI_DevSts->tCamColorParam.ubColorBL, 1, 1);
			}		
			break;
		case UI_TWC_VDO_IQ_CHROMA:   		// 0x81,
			if (*(ubTwcDataAddr) == E_OPCODE_SET) {
				printd(DBG_ErrorLvl, "[IQ_CHROMA]: SET D[%d : %d]\n", pUI_DevSts->tCamColorParam.ubColorHue, *(ubTwcDataAddr+1));
				if (pUI_DevSts->tCamColorParam.ubColorHue != *(ubTwcDataAddr+1)) {
					pUI_DevSts->tCamColorParam.ubColorHue = *(ubTwcDataAddr+1);
					ISP_SetIQChroma(pUI_DevSts->tCamColorParam.ubColorHue);
					UI_UpdateDevStatusInfo();
				}
			}
			else {
				printd(DBG_Debug3Lvl, "[IQ_CHROMA]: Get, D[%d]\n", pUI_DevSts->tCamColorParam.ubColorHue);
				WiFiDt_SendTwc(UI_TWC_VDO_IQ_CHROMA, TARGET_STA0, SrcIP, &pUI_DevSts->tCamColorParam.ubColorHue, 1, 1);
			}
			break;
		case UI_TWC_VDO_IQ_CONTRACT:   		// 0x82,
			if (*(ubTwcDataAddr) == E_OPCODE_SET) {
				printd(DBG_ErrorLvl, "[IQ_CONTRACT]: SET D[%d : %d]\n", pUI_DevSts->tCamColorParam.ubColorContrast, *(ubTwcDataAddr+1));
				if (pUI_DevSts->tCamColorParam.ubColorContrast != *(ubTwcDataAddr+1)) {
					pUI_DevSts->tCamColorParam.ubColorContrast = *(ubTwcDataAddr+1);
					ISP_SetIQContrast(pUI_DevSts->tCamColorParam.ubColorContrast);
					UI_UpdateDevStatusInfo();
				}
			}
			else {
				printd(DBG_Debug3Lvl, "[IQ_CONTRACT]: Get, D[%d]\n", pUI_DevSts->tCamColorParam.ubColorContrast);
				WiFiDt_SendTwc(UI_TWC_VDO_IQ_CONTRACT, TARGET_STA0, SrcIP, &pUI_DevSts->tCamColorParam.ubColorContrast, 1, 1);
			}
			break;
		case UI_TWC_VDO_IQ_SATURATION:   	// 0x83,
			if (*(ubTwcDataAddr) == E_OPCODE_SET) {
				printd(DBG_ErrorLvl, "[IQ_SATURATION]: SET D[%d : %d]\n", pUI_DevSts->tCamColorParam.ubColorSaturation, *(ubTwcDataAddr+1));
				if (pUI_DevSts->tCamColorParam.ubColorSaturation != *(ubTwcDataAddr+1)) {
					pUI_DevSts->tCamColorParam.ubColorSaturation = *(ubTwcDataAddr+1);
					ISP_SetIQSaturation(pUI_DevSts->tCamColorParam.ubColorSaturation);
					UI_UpdateDevStatusInfo();
				}
			}
			else {
				printd(DBG_Debug3Lvl, "[IQ_SATURATION]: Get, D[%d]\n", pUI_DevSts->tCamColorParam.ubColorSaturation);
				WiFiDt_SendTwc(UI_TWC_VDO_IQ_SATURATION, TARGET_STA0, SrcIP, &pUI_DevSts->tCamColorParam.ubColorSaturation, 1, 1);
			}
			break;
		case UI_TWC_ADO_VOL_CTL:		// 0x8C (R2R_VOL_n18p2DB[0]~R2R_VOL_n0DB[6])
			{
				uint8_t ubValue = 0;
				
				printd(DBG_Debug3Lvl, "[ADO_VOL_CTL]: D[%d, %d]\n", *(ubTwcDataAddr), *(ubTwcDataAddr+1));
				if (*(ubTwcDataAddr) == E_OPCODE_SET) {
					ubValue = *(ubTwcDataAddr+1);
					if (ubValue <= 6)
						ADO_SetDacR2RVol((ADO_R2R_VOL)(ubValue + R2R_VOL_n18p2DB));  
				}
				else {
					ubValue = ((uint8_t)ADO_GetDacR2RVol() - R2R_VOL_n18p2DB);
					WiFiDt_SendTwc(UI_TWC_ADO_VOL_CTL, TARGET_STA0, SrcIP, &ubValue, 1, 1);
				}
			}
			break;
#endif	//! End of #ifdef OP_STA
		case UI_TWC_PUSHTALK_FUNC:      // 0x8D
			printd(DBG_ErrorLvl, "[PUSHTALK]: Enable[%d, %d]\n", *(ubTwcDataAddr), *(ubTwcDataAddr+1));
			if ( (*(ubTwcDataAddr+1) == 0)||(*(ubTwcDataAddr+1) == 1) ) {
				ubUI_PushTalkEnable = *(ubTwcDataAddr+1);
				WiFiDt_SetPushTalkFunc(ubUI_PushTalkEnable);
				//printd(DBG_ErrorLvl, "[UI_TWC_PUSHTALK_FUNC]: PushTalkEnable[%d]\n", ubUI_PushTalkEnable);
			}
		    break;
#ifdef OP_STA
		case UI_TWC_CHANGE_RF_MODE:		// 0x8E
			{
				uint8_t ubData[1] = {0};
				uint8_t ubRxMac[6] = {0,0,0,0,0,0}, ubValid = 0;
				uint8_t ubDstMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
				uint8_t ubCH;
			
				printd(DBG_ErrorLvl, "[CHANGE_RF_MODE]: oper[%s]\n", (*(ubTwcDataAddr) == E_OPCODE_SET)?"SET":"GET");
				ubValid = sPRF_GetRxIdInfo(ubRxMac);
				if(ubValid)	{
					ubData[0] = 0x0;
					WiFiDt_SendTwc(UI_TWC_CHANGE_RF_MODE, TARGET_STA0, SrcIP, ubData, 1, 1);
					sPRF_QueryAndSet(SET_BEACON_OFF, &ubCH, 0);
					sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);
					sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);
					sPRF_QueryAndSet(SET_AP_DIS_ACT, SrcMac, 0);
					sPRF_ForceLeavByDev();
					
					if ( ubWiFiDt_QueryAppConnectStatus() ) {
						osMutexWait(xSemaphoreSysMutex, osWaitForever);
						pPtr->DP_Hdr.TwcOpc = UI_APPTWC_VDO_STOP;
						if (ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK) == 1) {
							LINK_REPORT tLinkRpt;

							printd(DBG_ErrorLvl, "=PV STOP=\n");
							tLinkRpt.tRole 	 = (LINK_ROLE)KNL_MASTER_AP;
							tLinkRpt.tStatus = (LINK_STATUS)KNL_LOST_LINK;
							osMessagePut(*pUIApp_QRptLinkStatus, &tLinkRpt, 0); 
						}
						osMutexRelease(xSemaphoreSysMutex);
					}
					osDelay(30);
					sPRF_ChkTxDrvMode(sPRF_TRX_MODE);
				}
				else {
					ubData[0] = 0x1;
					WiFiDt_SendTwc(UI_TWC_CHANGE_RF_MODE, TARGET_STA0, SrcIP, ubData, 1, 1);
				}		
			}
			break;
		case UI_TWC_SET_PF_PASSWORD:		// 0x90
			{
				uint8_t i = 0;
				uint8_t ubData[1] = {0};
#if (MANUAL_DT_ENCRY_TYPE == 1) 
				uint8_t ssidLen;
				uint8_t ubDstMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
				char cSsid[32] = {0};
#endif
				uint8_t ubErrorFlag = 0;	
				
				// data[0]: length, data[1]~ : real data.
				printd(DBG_ErrorLvl, "[SET_PF_PASSWORD] Len[%d] D[", *(ubTwcDataAddr));
				for(i = 0; i<*(ubTwcDataAddr); i++) {
					printd(DBG_ErrorLvl, "%c", *(ubTwcDataAddr+i+1));
				}
				printd(DBG_ErrorLvl, "]\n");
				
				ubErrorFlag = 0;
				if (*(ubTwcDataAddr) == E_DT_WPA_PWD_LEN) {
					for (i = 0; i<E_DT_WPA_PWD_LEN; i++) {
						if (pUI_DevSts->tCamsSecurity.ubSec_PWD[i] != *(ubTwcDataAddr+APP_PWDCHG_OFFSET_PWD_DATA+i) ) {
							ubErrorFlag = 1;
							break;
						}
					}

					if (ubErrorFlag == 0)
						ubData[0] = 0x1;	
					else 
						ubData[0] = 0x0;
				}
				else 
					ubData[0] = 0x1;
				WiFiDt_SendTwc(UI_TWC_SET_PF_PASSWORD, TARGET_STA0, SrcIP, ubData, 1, 1);

				if (ubErrorFlag == 0) {
					printd(DBG_ErrorLvl, "The password is the same, no change.\n");
					break;
				}
				memcpy(&pUI_DevSts->tCamsSecurity.ubSec_PWD, (ubTwcDataAddr+APP_PWDCHG_OFFSET_PWD_DATA), *(ubTwcDataAddr));
				pUI_DevSts->tCamsSecurity.ubSec_PWDLen = *(ubTwcDataAddr);
#if (MANUAL_DT_ENCRY_TYPE == 1) 
				if ( ubWiFiDt_QueryAppConnectStatus() ) {
					osMutexWait(xSemaphoreSysMutex, osWaitForever);
					VDO_Stop();
					KNL_SenStop(KNL_STA1);

					pPtr->DP_Hdr.TwcOpc = UI_APPTWC_VDO_STOP;
					if (ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK) == 1) {
						printd(DBG_ErrorLvl, "=PV STOP=\n");
						KNL_sPRFLinkReportFunc(sPRF_AP, sPRF_LOST_LINK);
					}
					osMutexRelease(xSemaphoreSysMutex);
				}
					
				sPRF_QueryAndSet(SET_BEACON_OFF, ubData, 0);
				// update SF --> DeAuth pkt --> reboot
				ssidLen = ubWiFiDt_GetSsid((uint8_t *)cSsid);
				Hostapd_setupParam(cSsid, ssidLen, 
									pUI_DevSts->tCamsSecurity.ubSec_PWD,
									pUI_DevSts->tCamsSecurity.ubSec_PWDLen,
									pUI_DevSts->tCamsSecurity.ubHostap_Param);
				UI_UpdateDevStatusInfo();
				//sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);
				sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);
				sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);
				sPRF_ForceLeavByDev();

				osDelay(30);
				printd(DBG_ErrorLvl, "Change WiFi password, reboot system !!!\n");
				SYS_Reboot();
#else 
				UI_UpdateDevStatusInfo();
#endif
			}
			break;
		case UI_TWC_PWIFI_PAIRING_MODE: 
			{
				uint8_t ubRxMac[6] = {0,0,0,0,0,0}, ubValid = 0;
				uint8_t ubData[2] = {0};

				ubValid = sPRF_GetRxIdInfo(ubRxMac);
				ubData[0] = ubValid;
				printd(DBG_ErrorLvl, "[PAIRING_MODE] D[%d] V[%d %d]\n", *(ubTwcDataAddr), ubValid, ubData[0]);
				WiFiDt_SendTwc(UI_TWC_PWIFI_PAIRING_MODE, TARGET_STA0, SrcIP, ubData, 1, 1);
				if (*(ubTwcDataAddr) == E_OPCODE_SET) {
					if (ubValid) {
						printd(DBG_ErrorLvl, "The TX cam is paired, do nothing.\n");
						break;
					}

					if ( ubWiFiDt_QueryAppConnectStatus() ) {
						pPtr->DP_Hdr.TwcOpc = UI_APPTWC_VDO_STOP;
						osMutexWait(xSemaphoreSysMutex, osWaitForever);
						if (ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK) == 1) {
							LINK_REPORT tLinkRpt;
							APP_EventMsg_t tUI_PairMessage = {0};

							printd(DBG_ErrorLvl, "=PV STOP=\n");
							tLinkRpt.tRole 	 = (LINK_ROLE)KNL_MASTER_AP;
							tLinkRpt.tStatus = (LINK_STATUS)KNL_LOST_LINK;
							osMessagePut(*pUIApp_QRptLinkStatus, &tLinkRpt, 0);

							tUI_PairMessage.ubAPP_Event = APP_PAIRING_START_EVENT;
							UI_SendMessageToAPP(&tUI_PairMessage);
						}
						osMutexRelease(xSemaphoreSysMutex);
					}
				}
			}
			break;
		case UI_APPTWC_SNPSHT_PLYBK_FUNC:
			printd(DBG_ErrorLvl, "----------->>> [SNPSHT_PLYBK]\n");
			if (ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK) == 1) {
				ubTWC_WiFiDt_GetData(TWC_STA1, TWC_UI_SETTING, SrcIP, ubTwcDataAddr);

			}
			break;
#endif	//! End of #ifdef OP_STA
		default:
		{
#ifdef OP_AP
			uint8_t ubAct = 0;
			ubAct = ubWiFiBdg_AppTwcFunc(pPtr, SrcIP, SrcMac);
			if(ubAct)
#endif
				ubWiFiDt_TwcProc(pPtr, SrcIP, SrcMac, E_TWC_CMD_OK);
			break;
		}
	}
}

//------------------------------------------------------------------------------
#if (APP_ADO_FUNC_ENABLE == 1)
void WiFiDt_AudioProc(ADOP *pPtr)
{
#ifdef OP_STA
	uint16_t	uwCurPktSz;
	static uint32_t ulTS = 0;
	ADO_Queue_INFO EN_INFO;	
	ADO_ENC_PACK_INFO_t *EncPackInfo;

	if (ubUI_PushTalkEnable == 0) {
		printd(DBG_ErrorLvl, "%s() -> PushTalkEnable[%d], skip data.\n", __FUNCTION__, ubUI_PushTalkEnable);
		return;
	}
	uwCurPktSz = FLIPW_word(pPtr->PacketIdex.PktSize);

	//printd(DBG_ErrorLvl, "ADO in, OP[%X] L[%d] \n", pPtr->ADOP_Hdr.TwcOpc, uwCurPktSz);
	if (uwCurPktSz > 0) {
		if (pPtr->ADOP_Hdr.TwcOpc == DT_TYPE_AUD32) {
			EncPackInfo = (ADO_ENC_PACK_INFO_t*)pPtr->ADOP_Hdr.DataPayload;
			if (ulPTTimeStamp == 0) {
				ulTS = ulPTTimeStamp = EncPackInfo->ulTimestamp;
			}
			else {
				ulPTTimeStamp = EncPackInfo->ulTimestamp - ulPTTimeStamp;
				printd(DBG_Debug3Lvl, "ADO, T[%d - %d] L[%d] \n", ulPTTimeStamp, (EncPackInfo->ulTimestamp - ulTS), uwCurPktSz);
				if (ulPTTimeStamp == 0) {
					ulPTTimeStamp = EncPackInfo->ulTimestamp;
					return;
				}
				ulPTTimeStamp = EncPackInfo->ulTimestamp;
			}			
	
			EN_INFO.PlyType = NORMAL_PLY;
			EN_INFO.EncType = ADO32;
			EN_INFO.SrcAddr = (uint32_t)pPtr->ADOP_Hdr.DataPayload;
			EN_INFO.SrcSize = (uint32_t)(uwCurPktSz);
			EN_INFO.ubSrcNum = 0;
			EN_INFO.ubPlaySrcNum = 0;
		}
		else {	// pPtr->ADOP_Hdr.TwcOpc = 0
			uint8_t	ubAdoDataBuf[320] = {0};

			EncPackInfo = (ADO_ENC_PACK_INFO_t*)ubAdoDataBuf;
			// add sonix tag
			memcpy(EncPackInfo->chADO_StreamTag, ADO_STREAM_TAG, sizeof(ADO_STREAM_TAG)-1);
			EncPackInfo->ulStartupFlag = 0;
			EncPackInfo->ulTimestamp   = KNL_TIMER_Get1ms();
			EncPackInfo->ulEncodedSize = uwCurPktSz;
			EncPackInfo->EncType       = ADO32;
			EncPackInfo->SampleRate    = SAMPLERATE_16kHZ;
			EncPackInfo->Channel       = MONO;
			EncPackInfo->ubReserved    = 0;
		
			memcpy(ubAdoDataBuf+sizeof(ADO_ENC_PACK_INFO_t), pPtr->ADOP_Hdr.DataPayload, uwCurPktSz);
			EN_INFO.PlyType = NORMAL_PLY;
			EN_INFO.EncType = ADO32;
			EN_INFO.SrcAddr = (uint32_t)ubAdoDataBuf;
			EN_INFO.SrcSize = (uint32_t)(uwCurPktSz+sizeof(ADO_ENC_PACK_INFO_t));
			EN_INFO.ubSrcNum = 0;
			EN_INFO.ubPlaySrcNum = 0;
		}

		if(ADO_DecBufWrtInChk(EN_INFO.PlyType, EN_INFO.EncType, EN_INFO.SrcSize) == DEC_BUF_EMPTY)
		{
			if(ubADO_DecBufWrtIn(&EN_INFO)==0)
			{
				printf("%s() -> DecBufWrt fail\n", __FUNCTION__);
			}
		}
	}
#endif	//! End of #ifdef OP_STA
#ifdef OP_AP
	uint16_t uwPktSize = 0;
	if (ubUI_PushTalkEnable == 0) {
		printd(DBG_ErrorLvl, "%s() -> PushTalkEnable[%d], skip data.\n", __FUNCTION__, ubUI_PushTalkEnable);
		return;
	}
	uwPktSize = FLIPW_word(pPtr->PacketIdex.PktSize);
	if((uwPktSize > 0) && (pPtr->ADOP_Hdr.TwcOpc == DT_TYPE_AUD32))
		ubKNL_ApBdgTxProc(sPRF_ADO_PKT, (uint8_t *)pPtr->ADOP_Hdr.DataPayload, (uint32_t)uwPktSize);
#endif	//! End of #ifdef OP_AP
	return;
}
#endif	//! End of #if (APP_ADO_FUNC_ENABLE == 1)

//------------------------------------------------------------------------------
uint8_t ubWiFiDt_BLEProc(uint8_t *ubDataAddr, uint32_t ubDataLen, uint8_t ubFlag)
{
#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
#if (E_BLE_FOTA_CUST_DEFINED == 1)
	if (ubFlag == 1) {
		BLE_FWU_RETURN_FLAG_t tState;

		ubWiFiDt_FwStatusProc(E_STATUS_FW_BT_UPDATING, NULL);
		printd(DBG_ErrorLvl, "BLEProc, Adr(0x%x) L[%ld] .\n\r", ubDataAddr, ubDataLen);
		if (ubIsBLE_ProcFOTA == 0) {
			ubIsBLE_ProcFOTA = 1;
			UIcmd_UpdateBLESts(1);
			
			tState = tAPP_BleUartFwu((uint8_t*)ubDataAddr, 0x2010, ubDataLen);	// 133120
			printd(DBG_ErrorLvl, "\r\n---> APP_BleUartFwu[%d]\r\n", tState);
			
			UIcmd_UpdateBLESts(0);
			ubIsBLE_ProcFOTA = 0;
		}
		if (tState == BLE_FWU_OK) {
			ubWiFiDt_FwStatusProc(E_STATUS_FW_BT_UPDATE_OK, NULL);
			return E_BT_FOTA_OK;
		}
		else {
			return E_BT_FOTA_FAIL;
		}
	}
	else 
		return E_BT_FOTA_OK;
#else 
	if (ubFlag == 0) {
		BLE_FWU_RETURN_FLAG_t tState;

		ubWiFiDt_FwStatusProc(E_STATUS_FW_BT_UPDATING, NULL);
		printd(DBG_ErrorLvl, "BLEProc, Adr(0x%x) L[%ld] .\n\r", ubDataAddr, ubDataLen);
		if (ubIsBLE_ProcFOTA == 0) {
			ubIsBLE_ProcFOTA = 1;
			UIcmd_UpdateBLESts(1);
			
			tState = tAPP_BleUartFwu((uint8_t*)ubDataAddr, 0x2010, ubDataLen);	// 133120
			printd(DBG_ErrorLvl, "\r\n---> APP_BleUartFwu[%d]\r\n", tState);
			
			UIcmd_UpdateBLESts(0);
			ubIsBLE_ProcFOTA = 0;
		}
		if (tState == BLE_FWU_OK) {
			ubWiFiDt_FwStatusProc(E_STATUS_FW_BT_UPDATE_OK, NULL);
			return E_BT_FOTA_OK;
		}
		else {
			return E_BT_FOTA_FAIL;
		}
	}
	else 
		return E_BT_FOTA_OK;
#endif

#else 
	return E_BT_FOTA_FAIL;
#endif	
}

//------------------------------------------------------------------------------
uint8_t ubWiFiDt_FwStatusProc(uint8_t ubStatus, uint8_t *ubData)
{
	uint8_t ubDstMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t ubAckBuf[6] = {0};

	switch (ubStatus)
	{
		case E_STATUS_FW_STATUS:
			printd(DBG_ErrorLvl, "FwStatusProc, [FW_STATUS] type(%x)\n\r", ubData[0]);	

			break;
		case E_STATUS_FW_BEGIN_UPDATE:
			{
				LINK_REPORT tLinkRpt;

				printd(DBG_ErrorLvl, "FwStatusProc, [FW_BEGIN_UPDATE]\n\r");

				tLinkRpt.tRole 	 = (LINK_ROLE)KNL_MASTER_AP;
				tLinkRpt.tStatus = (LINK_STATUS)KNL_LOST_LINK;
				osMessagePut(*pUIApp_QRptLinkStatus, &tLinkRpt, 0);  

				WiFiDt_StartUpdateFw(ubHBBuf);
			}
			break;
		case E_STATUS_FW_TX_UPDATING:
			printd(DBG_ErrorLvl, "FwStatusProc, [FW_TX_UPDATING]\n\r");
#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
			ubUI_RstBleTimer();
#endif
			break;
		case E_STATUS_FW_TX_UPDATE_OK:
			{
#if (E_BLE_FOTA_CUST_DEFINED == 1)
				printd(DBG_ErrorLvl, "FwStatusProc, [FW_TX_UPDATE_OK], M[%d %d]\n\r", ubData[0], ubData[1]);
				if ((ubData[1] & E_FW_DATA_MASK) == E_FW_DATA_TX) {

					sPRF_QueryAndSet(SET_BEACON_OFF, ubAckBuf, 0);
					printd(DBG_ErrorLvl, "2. FwStatusProc, [FW_TX_UPDATE_OK], M[%d %d]\n\r", ubData[0], ubData[1]);
					ubAckBuf[4] = APP_OTA_UPDT_SUCCESS;
					WiFiDt_SendTwc(UI_APPTWC_SEND_FWUPDATE_OK, TARGET_STA0, ubDesIP, &ubData[4], 1, 5);	
					osDelay(50);
					sPRF_ForceLeavByDev();
					sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);
					osDelay(30);
					
					printf("\n<<< Reset System >>>\n\n\n");
					sPRF_DevPwrDown();
					SYS_Reboot();
				}

#else 
				sPRF_QueryAndSet(SET_BEACON_OFF, ubAckBuf, 0);
				printd(DBG_ErrorLvl, "FwStatusProc, [FW_TX_UPDATE_OK], M[%d.%d.%d.%d]\n\r", ubData[0], ubData[1], ubData[2], ubData[3]);
				ubAckBuf[4] = APP_OTA_UPDT_SUCCESS;
				WiFiDt_SendTwc(UI_APPTWC_SEND_FWUPDATE_OK, TARGET_STA0, ubDesIP, &ubData[4], 1, 5);	
				osDelay(50);
				sPRF_ForceLeavByDev();
				sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);
				osDelay(30);
				
				printf("\n<<< Reset System >>>\n\n\n");
				sPRF_DevPwrDown();
				SYS_Reboot();
#endif
			}
			break;
		case E_STATUS_FW_TX_UPDATE_FAIL:
			{
				if (ubData != NULL) {
					printd(DBG_ErrorLvl, "FwStatusProc, [FW_TX_UPDATE_FAIL], D[%d]\n\r", ubData[0]);
					// -----STA update fail will send message to AP----- //
					WiFiDt_SendTwc(UI_APPTWC_SEND_FWUPDATE_FAIL, TARGET_STA0, ubDesIP, ubData, 1, 3);
					
					osDelay(80);
					printd(DBG_ErrorLvl, "<Burn_FW_Failed>, ErrCode:%d\n\r", ubData[4]);
				}
			}
			break;
		case E_STATUS_FW_BT_UPDATING:
			printd(DBG_ErrorLvl, "FwStatusProc, [FW_BT_UPDATING]\n\r");

			break;
		case E_STATUS_FW_BT_UPDATE_OK:
			printd(DBG_ErrorLvl, "FwStatusProc, [FW_BT_UPDATE_OK]\n\r");
#if (E_BLE_FOTA_CUST_DEFINED == 1)
			sPRF_QueryAndSet(SET_BEACON_OFF, ubAckBuf, 0);
			printd(DBG_ErrorLvl, "FwStatusProc, [FW_TX_UPDATE_OK]\n\r");
			ubAckBuf[0] = APP_OTA_UPDT_SUCCESS;
			WiFiDt_SendTwc(UI_APPTWC_SEND_FWUPDATE_OK, TARGET_STA0, ubDesIP, ubAckBuf, 1, 5);	
			osDelay(50);
			sPRF_ForceLeavByDev();
			sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);
			osDelay(30);
			
			printf("\n<<< Reset System >>>\n\n\n");
			sPRF_DevPwrDown();
			SYS_Reboot();
#endif
			break;
		case E_STATUS_FW_BT_UPDATE_FAIL:	
			printd(DBG_ErrorLvl, "FwStatusProc, [FW_BT_UPDATE_FAIL]\n\r");

			break;
		case E_STATUS_APP_PREVIEW_TIMEOUT:
			printd(DBG_ErrorLvl, "FwStatusProc, [APP_PREVIEW_TIMEOUT]\n\r");

			break;
		case E_STATUS_WIFI_INITIAL_TIMEOUT:
			printd(DBG_ErrorLvl, "FwStatusProc, [WIFI_INITIAL_TIMEOUT]\n\r");

			break;
		default:
			printd(DBG_ErrorLvl, "FwStatusProc, unsupport command\n\r");

			break;
	}
	return 1;
}


//--------------------------------------------------------
//                                                       |
//         	    PF 10 ms Task                            |
//                                                       |
//--------------------------------------------------------
void WiFiDt_Thread(void const *pvParameters)	
{
	uint8_t ubPrevLinkStatus = AP_NO_LINK, ubLinkStatus = AP_NO_LINK;
	uint8_t ubCRLTimerCnt = 0, ubCKLinkTimer = 0;
	uint8_t ubData[2] = {0};
#if (APP_BLE_SN9380_FUNC_ENABLE == 1) 
	uint16_t uwAppTimerCnt = 0;
	uint32_t ulPwnTimerCnt = 0;
#endif
	
	printd(DBG_ErrorLvl, "--> FW Ver[%s]\n\r", UI_CODE_VERSION);
	while(1)
	{
		osDelay(10);	

		ubCKLinkTimer++;			
		if ((ubCKLinkTimer % 20) == 0) {
			ubLinkStatus = ubWiFiDt_GetLinkStatus();
			if (ubPrevLinkStatus != ubLinkStatus) {
				if ((ubPrevLinkStatus == AP_LINKED) && (ubLinkStatus == AP_NO_LINK)) {
					printd(DBG_ErrorLvl, "-- Client disconnect, status(%d ->%d).\n", ubPrevLinkStatus, ubLinkStatus);
					
					osMutexWait(xSemaphoreSysMutex, osWaitForever);
					WiFiDt_UpdateAppConnectStatus(0);
					WiFiDt_ResetVdoAdoQue(1);
					KNL_sPRFLinkReportFunc(sPRF_AP, sPRF_LOST_LINK);
					osMutexRelease(xSemaphoreSysMutex);
								
					#if defined(OP_AP) || (defined (OP_STA) && (E_RVCS_ALL_I_FRM == 0))
					ubWiFiDtApp_PbCnt = 0;
					#endif
					printd(DBG_ErrorLvl, "=PV STOP OK=\n");
				}
				else {
					printd(DBG_ErrorLvl, "-- Client connect, status(%d ->%d).\n", ubPrevLinkStatus, ubLinkStatus);
					ubDT_AppCnnTimeOSts = 0;
				}
				ubPrevLinkStatus = ubLinkStatus;
			}
			
			if (ubLinkStatus == AP_LINKED) {
				if ((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))&&(ubWiFiDt_QueryAppConnectStatus())) {
				}
				else {
					if (ubDT_AppCnnTimeOSts == 1) {
						if ((ubCKLinkTimer % 50) == 0) 
							printd(DBG_ErrorLvl, "-- Cnn OK, Apps Lost link.\n");
	
						ubData[0] = 0xA;
						WiFiDt_SendTwc(UI_APPTWC_STA_ALIVE_AGAIN, TARGET_STA0, ubDesIP, ubData, 1, 1);
					}
				}
			}	

			if (ubCKLinkTimer == 200)
				ubCKLinkTimer = 0;
		}

		if ((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))&&(ubWiFiDt_QueryAppConnectStatus())) {

			ubCRLTimerCnt++;
			if (ubCRLTimerCnt == 10) { 	
				ubCRLTimerCnt = 0;
			}

			ubSyncTimerCnt++;
		 	if (ubDT_AppSyncPktFlag == 0) {
		 		if ( ((ubSyncTimerCnt%100) == 0)&&(ubSyncTimerCnt >= 500) ) 
					printd(DBG_ErrorLvl, "- Syn(%d) Sts[%d].\n", ubSyncTimerCnt, ubDT_AppCnnTimeOSts);
				
				if (ubSyncTimerCnt == E_BUC_APPS_SYNC_MAX_TIMEOUT) {
					printd(DBG_ErrorLvl, "-- [SYNC_TIMEOUT], Cnt[%d] -- STOP VDO.\n", ubWiFiDt_QueryAppConnectStatus());
					osMutexWait(xSemaphoreSysMutex, osWaitForever);
					WiFiDt_UpdateAppConnectStatus(0);
					ubDT_AppCnnTimeOSts = 1;
					WiFiDt_ResetVdoAdoQue(1);
					ubWiFiDt_FwStatusProc(E_STATUS_APP_PREVIEW_TIMEOUT, NULL);
					
					KNL_sPRFLinkReportFunc(sPRF_AP, sPRF_LOST_LINK);
					osMutexRelease(xSemaphoreSysMutex);
					#if defined(OP_AP) || (defined (OP_STA) && (E_RVCS_ALL_I_FRM == 0))
					ubWiFiDtApp_PbCnt = 0;
					#endif
					printd(DBG_ErrorLvl, "=PV STOP OK=\n");

					ubSyncTimerCnt = 0;
				}
		 	}
		 	else {	// ubDT_AppSyncPktFlag = 1
				WiFiDt_UpSynCnnSts(0);
				ubSyncTimerCnt = 0;
		 	}
		}
		else {
			ubSyncTimerCnt = 0;
		}
		
#if  (APP_BLE_SN9380_FUNC_ENABLE == 1) 
		if (ubUI_QueryBleSts() == E_BLE_ADVERT_EVENT) {
			if (ubUI_QueryBleTimer() == 1)
				uwAppTimerCnt = 0;
			else 
				uwAppTimerCnt++;

			if ((uwAppTimerCnt%400) == 0) {
				BUZ_PlayFailSound();
			}			
			if ( ((uwAppTimerCnt%100) == 0)&&(uwAppTimerCnt != 0) )
				printd(DBG_ErrorLvl, "--> BLE Event timer (%d)\n", uwAppTimerCnt/100);
			
			if (uwAppTimerCnt >= E_WOR_ADVERT_TIMEOUT) {
				uwAppTimerCnt = 0;
				printd(DBG_ErrorLvl, "--> Timout, Change BLE to Normal mode.\n");
				UI_UpdateWORsts(E_EVENT_RST, 0, E_510_PWN_ON);
			}
		}
		else {
			uwAppTimerCnt = 0;
		}
			
		if ( (ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))&&(ubWiFiDt_QueryAppConnectStatus()) ) {
			ulPwnTimerCnt = 0;			
		} 
		else {
			ulPwnTimerCnt++;
			
	 		if ( ((ulPwnTimerCnt%100) == 0)&&(ulPwnTimerCnt >= E_WOR_APPS_CHK_TIME) ) { 
				printd(DBG_ErrorLvl, "- PWN_Timeout(%d).\n", ulPwnTimerCnt/100);
			}

			if (ubUI_QueryBleStsTimer() == 1) {
				ulPwnTimerCnt = 0;	
			}
			else {		
				if (ulPwnTimerCnt >= E_WOR_APPS_ALIVE_TIME) {
					uint8_t ubDstMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

					UI_UpdateWORsts(E_EVENT_RST, 1, E_510_PWN_SAVING);
					ulPwnTimerCnt = 0;
					UI_EnableUART2Setting(0);
					sPRF_QueryAndSet(SET_AP_DIS_ACT, ubDstMac, 0);	
					printd(DBG_ErrorLvl, "--> Timout[%d], No connection, go to Pwn-off mode.\n", E_WOR_APPS_ALIVE_TIME/100);
					WDT_Disable(WDT_RST);
					osDelay(40);
					UI_PowerKey();
				}
			}
		}
#endif
	}
}

//------------------------------------------------------------------------------
void WiFiDt_SetupDevInfo(void)
{
#ifdef OP_STA
#define DT_SSID_XOR1				'r'
#define DT_SSID_XOR2				'v'
#define DT_SSID_XOR3				's'
#define DT_PWD_XOR1					'C'
#define DT_PWD_XOR2					'V'
#define DT_PWD_XOR3					'S'
#define DT_PWD_XOR4					'R'

	uint8_t ubApMac[6] = {0,0,0,0,0,0};
	uint8_t ubRxMac[6] = {0,0,0,0,0,0}, ubValid = 0;
	char cSsid[32] = {0};
#if (MANUAL_DT_ENCRY_TYPE == 1) 
#if (MANUAL_DT_PWD_USER_SET == 0)
	uint8_t ubTestMac[6] = {0};
#endif
	uint8_t ssidLen = 0;
	uint8_t ubUpPwd = 0;
	uint8_t ubUpdateHostapParam = 0;
#endif
	UI_BUStatus_t *pUI_CamStsInfo;

	WiFiDt_GetMacAddr(ubApMac);
	printd(DBG_CriticalLvl, "SetupDevInfo, Mac[%02X:%02X:%02X:%02X:%02X:%02X]\n\r", ubApMac[0], ubApMac[1], ubApMac[2], ubApMac[3], ubApMac[4], ubApMac[5]);
	ubValid = sPRF_GetRxIdInfo(ubRxMac);
	if(ubValid)
	{
		sPRF_DevId_t tTxId = sPRF_STA1;
		memset(cSsid, 0, sizeof(cSsid));
		tTxId = tsPRF_GetDevId();
		sprintf(cSsid, "%sCAM%X_%02X%02X%02X", MANUAL_DT_SSID, ((tTxId <= sPRF_STA4)?(tTxId + 1):sPRF_ID_INVAILD), 
												   ubRxMac[5] ^ DT_SSID_XOR1,
												   ubRxMac[4] ^ DT_SSID_XOR2,
												   ubRxMac[3] ^ DT_SSID_XOR3);
		WiFiDt_SetSsid(E_MAG_ENABLE, cSsid, strlen(cSsid), 0);
	}
	else
	{
		memset(cSsid, 0, sizeof(cSsid));
		sprintf(cSsid, "%sRVCS_%02X%02X%02X", MANUAL_DT_SSID, 
													ubApMac[5] ^ DT_SSID_XOR1,
											   		ubApMac[4] ^ DT_SSID_XOR2,
											   		ubApMac[3] ^ DT_SSID_XOR3);

		WiFiDt_SetSsid(E_MAG_ENABLE, cSsid, strlen(cSsid), 0);
	}

	pUI_CamStsInfo = (UI_BUStatus_t *)pUI_GetDevSetting();
#if (MANUAL_DT_ENCRY_TYPE == 1) 
	printd(DBG_ErrorLvl, "SetupDevInfo(0), EN[0x%X], L[0%X], PWD[%s] P[%d]\n", pUI_CamStsInfo->tCamsSecurity.ubSec_En,
			pUI_CamStsInfo->tCamsSecurity.ubSec_PWDLen, pUI_CamStsInfo->tCamsSecurity.ubSec_PWD, pUI_CamStsInfo->tCamsSecurity.ubPaired);

	if ((pUI_CamStsInfo->tCamsSecurity.ubSec_En == E_DT_PWD_NULL)||(pUI_CamStsInfo->tCamsSecurity.ubSec_En == E_DT_PWD_EMPTY)) {
		printd(DBG_ErrorLvl, "SetupDevInfo(1): EN[%d], update SF parameter\n", pUI_CamStsInfo->tCamsSecurity.ubSec_En);
		ubUpPwd = 1;
	}
	else if ((pUI_CamStsInfo->tCamsSecurity.ubSec_En == E_DT_PWD_SET_OLD_FW)&&(pUI_CamStsInfo->tCamsSecurity.ubSec_PWDLen > 0)) {
		printd(DBG_ErrorLvl, "SetupDevInfo(2): EN[%d], L[0%X], update SF parameter\n", pUI_CamStsInfo->tCamsSecurity.ubSec_En,
						pUI_CamStsInfo->tCamsSecurity.ubSec_PWDLen);
		// to check if the user has changed the password ? If he changed,
		ubUpPwd = 1;
	}
	else if ((pUI_CamStsInfo->tCamsSecurity.ubSec_En == E_DT_PWD_SET_NEW_FW)&&(pUI_CamStsInfo->tCamsSecurity.ubSec_PWDLen > 0)) {
		ubUpPwd = 0;
	}
	else {
		printd(DBG_ErrorLvl, "SetupDevInfo(3): EN[0x%X], unknown value, to update SF parameter\n", pUI_CamStsInfo->tCamsSecurity.ubSec_En);
		ubUpPwd = 1;
	}

	if (pUI_CamStsInfo->tCamsSecurity.ubHostap_Param[0] == 0x00 || pUI_CamStsInfo->tCamsSecurity.ubHostap_Param[0] == 0xFF) {
		printd(DBG_ErrorLvl, "SetupDevInfo(4), param:0x%x\n", pUI_CamStsInfo->tCamsSecurity.ubHostap_Param[0]); 
		ubUpdateHostapParam = 1;
	}
	else {
		printd(DBG_ErrorLvl, "SetupDevInfo(5), param:0x%x\n", pUI_CamStsInfo->tCamsSecurity.ubHostap_Param[0]); 
	}

	if ((ubValid != pUI_CamStsInfo->tCamsSecurity.ubPaired)&&(pUI_CamStsInfo->tCamsSecurity.ubPaired != 0xFF)) {
		printd(DBG_ErrorLvl, "SetupDevInfo(6), pairing status[%d -> %d] is incorrect, to re-calcuate the key\n", pUI_CamStsInfo->tCamsSecurity.ubPaired, ubValid); 
		ubUpdateHostapParam = 1;
	}
	
	if ((ubUpPwd == 1)||(ubUpdateHostapParam == 1)) {

		memset(cSsid, 0, 32);
		ssidLen = ubWiFiDt_GetSsid((uint8_t *)cSsid);
		printd(DBG_CriticalLvl, "SetupDevInfo, L[%d] S[%s]\n", ssidLen, cSsid);
		if (ubUpPwd == 1) {		
			pUI_CamStsInfo->tCamsSecurity.ubSec_En = E_DT_PWD_SET_NEW_FW;
			pUI_CamStsInfo->tCamsSecurity.ubSec_PWDLen = E_DT_WPA_PWD_LEN; 
			memset(&pUI_CamStsInfo->tCamsSecurity.ubSec_PWD, 0, 20);
#if (MANUAL_DT_PWD_USER_SET == 1)		
			strcpy((char *)pUI_CamStsInfo->tCamsSecurity.ubSec_PWD, MANUAL_DT_PWD);
#else

			memset(ubTestMac, 0, 6);
			ubTestMac[2] = ((cSsid[ssidLen-5]<<4)|cSsid[ssidLen-6]);
			ubTestMac[1] = ((cSsid[ssidLen-3]<<4)|cSsid[ssidLen-4]);
			ubTestMac[0] = ((cSsid[ssidLen-1]<<4)|cSsid[ssidLen-2]);
			printd(DBG_CriticalLvl, "SetupDevInfo, TM[%02X:%02X:%02X:%02X:%02X:%02X]\n\r", ubTestMac[0], ubTestMac[1], ubTestMac[2], ubTestMac[3], ubTestMac[4], ubTestMac[5]);
			
			sprintf((char *)pUI_CamStsInfo->tCamsSecurity.ubSec_PWD, "%02X%02X%02X%02X", 
								ubTestMac[2] ^ DT_PWD_XOR1,
								ubTestMac[0] ^ DT_PWD_XOR2,
								ubTestMac[1] ^ DT_PWD_XOR3,
							 	ubTestMac[2] ^ DT_PWD_XOR4);

			printd(DBG_CriticalLvl, "SetupDevInfo, TP[%s]\n", (char *)pUI_CamStsInfo->tCamsSecurity.ubSec_PWD);

#endif
		}

		if (ubUpdateHostapParam == 1) {
			pUI_CamStsInfo->tCamsSecurity.ubPaired = ubValid;
			VDO_Stop();
			KNL_SenStop(KNL_STA1);
			Hostapd_setupParam(cSsid, ssidLen, 
							pUI_CamStsInfo->tCamsSecurity.ubSec_PWD,
							pUI_CamStsInfo->tCamsSecurity.ubSec_PWDLen,
							pUI_CamStsInfo->tCamsSecurity.ubHostap_Param);
			VDO_Start();
			KNL_SenStart(KNL_STA1);		
			if (Hostapd_check_lauched())
				Hostapd_onUninit();
		}
		UI_UpdateDevStatusInfo();
	}	
	printd(DBG_ErrorLvl, "SetupDevInfo, EN[0x%X], L[0%X], PWD[%s] P[%d]\n", pUI_CamStsInfo->tCamsSecurity.ubSec_En,
			pUI_CamStsInfo->tCamsSecurity.ubSec_PWDLen, pUI_CamStsInfo->tCamsSecurity.ubSec_PWD, pUI_CamStsInfo->tCamsSecurity.ubPaired);

	//sPRF_QueryAndSet(SET_WPA_PASSWORD, pUI_CamStsInfo->tCamsSecurity.ubSec_PWD, (unsigned short *)&uwLen);
	WiFiDt_SetWepPassword(E_DT_WPA2, E_MAG_ENABLE, pUI_CamStsInfo->tCamsSecurity.ubSec_PWD);

	Hostapd_onInit((void *)pUI_CamStsInfo->tCamsSecurity.ubHostap_Param);	
#else 
	if ((pUI_CamStsInfo->tCamsSecurity.ubSec_En == 0)||(pUI_CamStsInfo->tCamsSecurity.ubSec_En == 0xFF)) {
		pUI_CamStsInfo->tCamsSecurity.ubSec_En = 1;
		pUI_CamStsInfo->tCamsSecurity.ubSec_PWDLen = 0; 
		memset(&pUI_CamStsInfo->tCamsSecurity.ubSec_PWD, 0, 20);
	}
	printd(DBG_ErrorLvl, "AppInit(): EN[0x%X], L[0%X], PWD[%s]\n", pUI_CamStsInfo->tCamsSecurity.ubSec_En,
			pUI_CamStsInfo->tCamsSecurity.ubSec_PWDLen, pUI_CamStsInfo->tCamsSecurity.ubSec_PWD);

	WiFiDt_SetWepPassword(E_DT_OPEN, E_MAG_ENABLE, "");
#endif	//! End of #if (MANUAL_DT_ENCRY_TYPE == 1) 
#endif	//! End of #ifdef OP_STA
#ifdef OP_AP
	WiFiBdg_SetupDevInfo();
#endif	//! End of #ifdef OP_AP
}
#endif //! End of #if (defined(S2019A) && defined(RVCS_APP))
