/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_CFG.h
	\brief		APP Configuration header file
	\author		Wales
	\version	0.11
	\date		2020/05/27
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef _APP_CFG_H_
#define _APP_CFG_H_

#include <stdint.h>
#include "bsp_config.h"

//------------------------------------------------------------------------------
//!Thread Priority Setting
#define THREAD_PRIO_UI_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_UIEVENT_HANDLER		osPriorityAboveNormal
#define THREAD_PRIO_KEY_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_BB_HANDLER			osPriorityRealtime
#define THREAD_PRIO_TWC_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_KNL_PROC			osPriorityHigh
#define THREAD_PRIO_KNL_AVG_PLY			osPriorityAboveNormal
#define THREAD_PRIO_KNL_TWC_MONIT		osPriorityNormal
#define THREAD_PRIO_KNL_VDOIN_PROC		osPriorityAboveNormal
#define THREAD_PRIO_KNL_RESSW_PROC		osPriorityAboveNormal
#define THREAD_PRIO_ADO_ENC_PROC		osPriorityAboveNormal
#define THREAD_PRIO_ADO_DEC_PROC		osPriorityNormal		//! osPriorityHigh
#define THREAD_PRIO_COMM_TX_VDO			osPriorityHigh
#define THREAD_PRIO_COMM_RX_VDO			osPriorityNormal
#define THREAD_PRIO_COMM_RX_ADO			osPriorityNormal
#define THREAD_PRIO_IMG_MONIT			osPriorityNormal
#define THREAD_PRIO_APP_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_PAIRING_HANDLER		osPriorityNormal
#define THREAD_PRIO_EN_HANDLER			osPriorityBelowNormal
#define THREAD_PRIO_KNLRECORD_HANDLER	osPriorityBelowNormal
#define THREAD_PRIO_JPEG_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_LINK_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_RC_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_RC_SYS_MONIT		osPriorityBelowNormal
#define THREAD_PRIO_LINK_UPDATE			osPriorityBelowNormal
#define THREAD_PRIO_SEC_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_TIME_STAMP          osPriorityNormal
#define THREAD_PRIO_TX_REC              osPriorityNormal
#define THREAD_PRIO_GOPSYNC          	osPriorityNormal
#define THREAD_PRIO_SYS_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_KNL_SD_HANDLER      osPriorityNormal
#define THREAD_PRIO_KNL_EMERGENCY_HANDLER      osPriorityNormal
#define THREAD_PRIO_KNL_RTC_MONIT_HANDLER    osPriorityNormal
#define THREAD_PRIO_WDT_HANDLER			osPriorityLow				// other thread can not set osPriorityLow!!!

//------------------------------------------------------------------------------
//! Thread Stack Size Setting
#define THREAD_STACK_UI_HANDLER			3072
#define THREAD_STACK_UIEVENT_HANDLER	22528
#define THREAD_STACK_KEY_HANDLER		1024
#define THREAD_STACK_BB_HANDLER			8192
#define THREAD_STACK_TWC_HANDLER		2048
#define THREAD_STACK_EN_HANDLER			8192
#define THREAD_STACK_KNL_PROC			15360
#define THREAD_STACK_KNL_AVG_PLY		2048
#define THREAD_STACK_KNL_TWC_MONIT		2048
#define THREAD_STACK_KNL_VDOIN_PROC		2048
#define THREAD_STACK_KNL_RESSW_PROC		2048

#define THREAD_STACK_ADO_PROC			8192
#define THREAD_STACK_COMM_TX_VDO	    2048
#define THREAD_STACK_COMM_RX_VDO	    1536
#define THREAD_STACK_COMM_RX_ADO	    1536
#define THREAD_STACK_IMG_MONIT		    1024
#define THREAD_STACK_APP_HANDLER		25600
#define THREAD_STACK_PAIRING_HANDLER	512
#define THREAD_STACK_KNLRECORD_HANDLER	32768
#define THREAD_STACK_JPEG_MONIT			2048
#define THREAD_STACK_LINK_MONIT			1024
#define THREAD_STACK_RC_MONIT			2048
#define THREAD_STACK_RC_SYS_MONIT		2048
#define THREAD_STACK_LINK_UPDATE		1024
#define THREAD_STACK_SEC_MONIT			1024
#define THREAD_STACK_TIME_STAMP         1024
#define THREAD_STACK_TX_REC             3072
#define THREAD_STACK_SYNCGOP            256
#define THREAD_STACK_SYS_MONIT			1024
#define THREAD_STACK_KNL_SD_HANDLER     2048
#define THREAD_STACK_KNL_EMERGENCY_HANDLER     2048
#define THREAD_STACK_RTC_MONIT			1024

//! APP Event Definition
#define APP_REFRESH_EVENT				0
#define APP_PWRCTRL_EVENT				1
#define APP_LINK_EVENT					2
#define APP_LOSTLINK_EVENT				3
#define APP_PAIRING_START_EVENT			4
#define APP_PAIRING_STOP_EVENT			5
#define APP_PAIRING_SUCCESS_EVENT		6
#define APP_PAIRING_FAIL_EVENT			7
#define APP_LINKSTATUS_REPORT_EVENT		8
#define APP_UNBIND_CAM_EVENT			9
#define APP_VIEWTYPECHG_EVENT			10
#define APP_ADOSRCSEL_EVENT				11
#define APP_PTT_EVENT					12
#define APP_POWERSAVE_EVENT				13

//! APP Queue message type
typedef struct
{
	uint8_t ubAPP_Event;
	uint8_t ubAPP_Message[7];
}APP_EventMsg_t;

//------------------------------------------------------------------------------
//! APP Serial Flash Start Sector
#define PAIR_SF_START_SECTOR			2
#define UI_SF_START_SECTOR				6		//! Greater than to 2
#define KNL_SF_START_SECTOR				7		//! Greater than to 2
#define DRV_MD_SF_START_SECTOR			8		//! Greater than to 2

#define	SF_AP_UI_SECTOR_TAG 			"SN93701_UI"
#define	SF_STA_UI_SECTOR_TAG 			"SN93700_UI"

#define	SF_AP_KNL_SECTOR_TAG 			"SN93701_KNL"
#define	SF_STA_KNL_SECTOR_TAG 			"SN93700_KNL"

//------------------------------------------------------------------------------
//! APP FWU Volume label
#ifdef VBM_BU
#define SN937XX_VOLUME_LABLE			"SN93700BU"
#endif
#ifdef VBM_PU
#define SN937XX_VOLUME_LABLE			"SN93701PU"
#endif
//------------------------------------------------------------------------------
#include "APP_CFG_SEL.h"
//------------------------------------------------------------------------------

#if (!APP_SD_FUNC_ENABLE && APP_PHOTOGRAPH_FUNC_ENABLE && APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SD)
#error No SD Card: Photo cannot be stored in SD Card!	
#endif
#if (APP_REC_FUNC_ENABLE && defined(VBM_PU) && BSP_DDRSIZE < 64)
#error The IC no supports the recording then must change IC Part Number!
#endif
#if ((APP_REC_FUNC_ENABLE || APP_PHOTOGRAPH_FUNC_ENABLE) && defined(VBM_BU) && BSP_DDRSIZE < 32)
#error The IC no supports the recording or capture photo then must change IC Part Number!
#endif
#if ( APP_PLAY_REMOTE_ENABLE && APP_FS_FILE_LIST_STYLE)
#error TX Remote Play function not support Thumbnail.
#endif
#endif

