/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		WiFiDtApp.h
	\brief		WiFiDtApp Header file
	\author		Chris 
	\version	0.2
	\date		2020/11/26
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _UI_APPTWC_H_
#define _UI_APPTWC_H_

#include "_510PF.h"
#include "WiFi_API.h"
#include "TWC_API.h"
#include "KNL.h"

enum {
	E_EMPTY_FLASH = 0,					// 0
	E_MIRROR_FLIP, 						// 1
	E_BRIGHTNESS_CH,					// 2
	E_CHROMA_CH,						// 3
	E_CONTRAST_CH,						// 4
	E_SATURATION_CH,					// 5
	E_PREVIEW_TIMEOUT,					// 6

	E_PF_PWD_LEN = 30,
	E_PF_PWD_CONTENT,

	
	E_CRC_VALUE_1 = 254,				// Don't change
	E_CRC_VALUE_2 = 255,  				// Don't change
	E_PARAMDATA_FINAL = 256,			// Don't change	
};

enum {
	E_MAG_ENABLE = 0,
	E_MAG_DISABLE,
	E_MAG_ENABLE_TIMELY,
};


typedef enum {
	AP_NO_LINK,
	AP_PRO_AUTH,
	AP_AUTH_ASSOCIATE,
	AP_LINKED
}DT_LINK_STATE;

enum {
	PF_B_11M = 0x3,
	PF_G_24M = 0x14,
    PF_G_54M = 0x17,
};

enum {
	E_DT_PWD_NULL = 0,
	E_DT_PWD_SET_OLD_FW = 1,
	E_DT_PWD_SET_NEW_FW = 0xA1,
	E_DT_PWD_EMPTY = 0xFF,
};
//------------------------------------------------------------------------------
#if (defined(S2019A) && defined(RVCS_APP))
void WiFiDt_AppInit(osMessageQId *pLinkRptQue);
void WiFiDt_AppUninit(void);
void WiFiDt_ResetRateCtrl(uint8_t ubRstFlag);
void WiFiDt_UpSynCnnSts(uint8_t ubStatus);
uint8_t ubWiFiDt_VdoStartProc(DP* pPtr, uint8_t *SrcIP, uint8_t *SrcMac, uint8_t ubLinked);
void WiFiDt_AppTwcProc(DP* pPtr, uint8_t *SrcIP, uint8_t *SrcMac);
uint8_t ubWiFiDt_BLEProc(uint8_t *ubDataAddr, uint32_t ubDataLen, uint8_t ubFlag);
uint8_t ubWiFiDt_FwStatusProc(uint8_t ubStatus, uint8_t *ubData);
#if (APP_ADO_FUNC_ENABLE == 1)
void WiFiDt_AudioProc(ADOP *pPtr);
#endif
void WiFiDt_SetupDevInfo(void);
#else
#define WiFiDt_AppInit(...)			((void)0)
#define WiFiDt_AppUninit()			((void)0)
#endif	//! End of #if (defined(OP_STA) && defined(S2019A) && defined(RVCS_APP))

//------------------------------------------------------------------------------
#endif	// _UI_APPTWC_H_
