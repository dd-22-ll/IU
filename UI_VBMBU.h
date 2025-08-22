/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI_VBMBU.h
	\brief		User Interface Header file (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.1
	\date		2017/11/30
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _UI_VBMBU_H_
#define _UI_VBMBU_H_

#include "UI.h"
#include "APP_HS.h"
#include "MD_API.h"
#include "RTC_API.h"
#include "VDO.h"

#define UI_UPDATESTS_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_PAIRINGLED_PERIOD	(500 / UI_TASK_PERIOD)
#define UI_CHKWORSTS_PERIOD		(500 / UI_TASK_PERIOD)

typedef enum
{
	CAMFLICKER_50HZ,
	CAMFLICKER_60HZ,
}UI_CamFlicker_t;

typedef enum
{
	CAMSET_OFF,
	CAMSET_ON,
}UI_CamsSetMode_t;

typedef enum
{
	RECLOOP_MODE,
	RECMANU_MODE,
	PHOTO_MODE,
	RECPHOTO_MODE,
	VDOMODE_MAX
}UI_CamVdoMode_t;

typedef enum
{
	REC_LOOPING,
	REC_MANUAL,	
	REC_RECMODE_MAX,
}UI_RecordMode_t;

typedef enum
{
	RECRES_FHD,
	RECRES_HD,
	RECRES_WVGA,
	RECRES_MAX,
}UI_RecordResolution_t;

typedef enum
{
	PHOTOFUNC_OFF,
	PHOTOFUNC_ON,
	PHOTOFUNC_MAX,
}UI_PhotoFunction_t;

typedef enum
{
	PHOTORES_3M,
	PHOTORES_5M,
	PHOTORES_12M,
	PHOTORES_MAX,
}UI_PhotoResolution_t;

typedef enum
{
	BAT_LVL4,
	BAT_LVL3,
	BAT_LVL2,
	BAT_LVL1,
	BAT_LVL0,
	BAT_CHARGE,
}UI_BatLvl_t;

typedef struct
{
	uint32_t ulCardSize;
	uint32_t ulRemainSize;
}UI_SdInfo_t;

typedef struct
{
	uint8_t ubColorBL;
	uint8_t ubColorContrast;
	uint8_t ubColorSaturation;
	uint8_t ubColorHue;
}UI_ColorParam_t;

typedef enum
{
	UI_FS_ENTER,
    UI_FS_EXIT,    
    UI_FS_FORMAT,
    UI_FS_DEL,
    UI_FS_SD_IN,
    UI_FS_SD_OUT,
}UI_FS_EventItem_t;

typedef enum
{
	UI_FS_DCIM_FLD,
    UI_FS_REC_FLD,
    UI_FS_FILE,
}UI_FSSettingItem_t;

typedef enum
{
    UI_TXPLY_PLAYCU,
    UI_TXPLY_PLAYCAM,
	UI_TXPLY_STOPCU,
	UI_TXPLY_STOPCAM,
    UI_TXPLY_JMPFW,
    UI_TXPLY_JMPBW,
    UI_TXPLY_JMPPAUSE,
    UI_TXPLY_UPDATE,    
}UI_TXPLY_EventItem_t;

typedef enum
{
	UI_PLY_PREV,      // File Index--
	UI_PLY_NEXT,      // File Index++
	UI_PLY_KEEP,      // Not Jump Next File
	UI_PLY_ONCE,      // Play once
}UI_PLY_NextItem_t;

typedef struct
{
	uint16_t uwTotalRecFolderNum;
	uint16_t uwRecFolderSelIdx;
	KNL_FOLDERSINFO_t tRecFolderInfo[KNL_TOTAL_SUB_FLD_NUM];
}UI_RecFoldersInfo_t;

typedef struct
{
	uint16_t uwTotalRecFileNum;
	uint16_t uwRecFileSelIdx;
	KNL_FILESINFO_t tRecFilesInfo[KNL_MAX_FILE_NUM];
}UI_RecFilesInfo_t;

typedef enum
{
	UI_REC_STOP,
	UI_REC_START,
}UI_RecordingAct_t;

typedef enum
{
	UI_RECFILE_PAUSE,
	UI_RECFILE_PLAY,
	UI_RECFILE_STOP,
}UI_RecPlayStatus_t;

typedef enum
{
	UI_RECSKIPBKFWD_ITEM,
	UI_RECPLAYPAUSE_ITEM,
	UI_RECSKIPFRFWD_ITEM,
	UI_RECADOSRCSEL_ITEM,
	UI_RECPLAYLISTITEM_MAX
}UI_RecPlayListItem_t;

typedef struct
{
	UI_RecordingAct_t tRecAct;
	uint8_t ubPlayMode;
	UI_RecPlayStatus_t tPlaySts;
	UI_CamNum_t tVdoPlayCam[4];
	UI_CamNum_t tAdoPlayCam;
    KNL_FldType_t tSimFld;
    UI_PLY_NextItem_t tPlayDir;
    uint8_t ubReserve[2];
}UI_RecPlayAct_t;

typedef enum
{
	UI_RECPLAY_MULTIVIEW,
	UI_RECPLAY_SINGLEVIEW,
}UI_RecPlayDispType_t;

typedef enum
{
	CAMIMGFLIP_DISABLE,
	CAMIMGFLIP_ENABLE,
}UI_CamImgFlip_t;

typedef enum
{
	CAMIMGMIRROR_DISABLE,
	CAMIMGMIRROR_ENABLE,
}UI_CamImgMirror_t;

typedef struct
{
	uint8_t		ubSec_En;
	uint8_t		ubSec_PWDLen;
	uint8_t		ubSec_PWD[20];
	uint8_t		ubPaired;
	uint8_t		ubHostap_Param[32];
}UI_CamsSecurity_t;

typedef struct
{
	char 		   		  cbUI_DevStsTag[11];
	char 		  		  cbUI_FwVersion[11];
	uint32_t 			  ulCAM_ID;
	UI_DisplayLocation_t  tCamDispLocation;	
	UI_CamVdoMode_t		  tCamVdoMode;	
	UI_BatLvl_t 		  tCamBatLvl;
	UI_SdInfo_t 		  tSdInfo;
	UI_RecordMode_t		  tREC_Mode;
	UI_RecordResolution_t tREC_Resolution;
	UI_PhotoFunction_t	  tPHOTO_Func;
	UI_PhotoResolution_t  tPHOTO_Resolution;
	UI_CamsSetMode_t	  tCamAnrMode;
	UI_CamsSetMode_t	  tCam3DNRMode;
	UI_CamsSetMode_t	  tCamvLDCMode;
	UI_CamsSetMode_t	  tCamAecMode;
	UI_CamsSetMode_t	  tCamDisMode;
	UI_CamFlicker_t		  tCamFlicker;
	UI_CamsSetMode_t	  tCamCbrMode;
	UI_CamsSetMode_t	  tCamCondenseMode;
	UI_ColorParam_t		  tCamColorParam;
	struct
	{
		uint8_t			  ubMD_Mode;
		uint8_t 		  ubMD_Param[4];
	}MdParam;
	UI_PowerSaveMode_t	  tCamPsMode;
	UI_CamsSetMode_t	  tCamScanMode;
	UI_CamImgFlip_t		  tCamImgFlip;
	UI_CamImgMirror_t	  tCamImgMirror;
	UI_CamsSecurity_t	  tCamsSecurity;
	UI_VolumeLvl_t		  tCamVolLvl;
	uint8_t				  ubReserved[133];
}UI_BUStatus_t;

typedef enum
{
	UI_UPDATE_BUSTS = 0x20,
	UI_VOX_TRIG,
	UI_MD_TRIG,
	UI_VOICE_TRIG,
	UI_FS_TRIG,
	UI_PLY_TRIG,
}UI_BUReqCmdID_t;

typedef enum
{
	UI_PTZ_SETTING = 1,	
	UI_RECMODE_SETTING,
	UI_RECRES_SETTING,
	UI_SDCARD_SETTING,
	UI_PHOTOMODE_SETTING,
	UI_PHOTORES_SETTING,
	UI_SYSINFO_SETTING,
	UI_VOXMODE_SETTING,
	UI_ECOMODE_SETTING,
	UI_WORMODE_SETTING,
	UI_ADOANR_SETTING,
	UI_ADOAEC_SETTING,
	UI_IMGPROC_SETTING,
	UI_MD_SETTING,
	UI_VOICETRIG_SETTING,
	UI_PERDBGMODE_SETTING,
	UI_FS_EVENT_SETTING,
	UI_PLAY_EVENT_SETTING,
}UI_PUReqCmdID_t;

typedef enum
{
	UI_IMG3DNR_SETTING,
	UI_IMGvLDC_SETTING,
	UI_IMGWDR_SETTING,
	UI_IMGDIS_SETTING,
	UI_IMGCBR_SETTING,
	UI_IMGCONDENSE_SETTING,
	UI_FLICKER_SETTING,
	UI_IMGBL_SETTING,
	UI_IMGCONTRAST_SETTING,
	UI_IMGSATURATION_SETTING,
	UI_IMGHUE_SETTING,
	UI_IMGSETTING_MAX = 20,
}UI_ImgProcSettingItem_t;

#define	UI_CLEAR_THREADCNT(Flag, Count)			do { if(Flag == TRUE) { (Count) = 0; Flag = FALSE; } } while(0)
#define UI_CLEAR_CAMSETTINGTODEFU(xFUNC, mDEFU)	do { xFUNC = mDEFU; } while(0)
#define UI_CHK_CAMSFUNCTS(Mode, Status)			do { if(Mode > CAMSET_ON) { Mode = Status; } } while(0)
#define UI_CHK_CAMFLICER(HZ)					do { if(HZ > CAMFLICKER_60HZ) { HZ = CAMFLICKER_60HZ; } } while(0)
#define UI_CHK_CAMPARAM(Param, Value)			do { if(Param >= 128) { Param = Value; } } while(0)
#define UI_CHK_MDMODE(Mode, Status)				do { if(Mode > MD_ON) { Mode = Status; } } while(0)
#define UI_CHK_PSMODE(Mode, State)				do { if(Mode > State) { Mode = State; } } while(0)

//! Two way command timeout
#define UI_TWC_TIMEOUT		(3 * 1000)				//! Unit: ms
//! Two way command format of UI
#define UI_TWC_TYPE			0
#define UI_REPORT_ITEM		1
#define UI_REPORT_DATA		2
#define UI_SETTING_ITEM		1
#define UI_SETTING_DATA		2

typedef enum
{
	UI_REPORT,
	UI_SETTING,
}UI_TwcDataType_t;

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
typedef struct
{
	uint8_t 		ubCmd_Len;
	uint8_t			ubCmd[8];
}UI_BUReqCmd_t;

typedef struct
{
	osThreadId	thread_id;
	int32_t		iSignals;
	UI_Result_t	tReportSts;
}UI_ThreadNotify_t;
#pragma pack(pop)

typedef struct
{
	void (*pvAction)(void *);
}UI_ReportFuncPtr_t;

typedef struct
{
	void (*pvAction)(void *);
}UI_SettingFuncPtr_t;

typedef struct
{
	void (*pvImgFunc)(uint8_t);
	uint8_t *pImgParam;
}UI_IspSettingFuncPtr_t;

void UI_PowerKey(void);
void UI_PairingKey(void);
void UI_UpdateBUStatusToPU(void);
UI_Result_t UI_SendRequestToPU(osThreadId thread_id, UI_BUReqCmd_t *ptReqCmd);
void UI_RecvPUResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus);
void UI_RecvPURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data);
void UI_SystemSetup(void);
void UI_PowerSaveSetting(void *pvPS_Mode);
void UI_ChangePsModeToWorMode(void);
void UI_ChangePsModeToNormalMode(void);
void UI_DisableVox(void);
void UI_VoxTrigger(void);
void UI_VoiceTrigSetting(void *pvTrigMode);
void UI_VoiceTrigger(void);
void UI_ANRSetting(void *pvAnrMode);
void UI_AECSetting(void *pvAecMode);
void UI_IspSetup(void);
void UI_ImageProcSetting(void *pvImgProc);
void UI_SetMotionEvent (void);
void UI_MDTrigger(void);
void UI_MDSetting(void *pvMdParam);
void UI_ResetDevSetting(void);
void UI_LoadDevStatusInfo(void);
void UI_UpdateDevStatusInfo(void);
void UI_PerDebugModeSetting(void *pvDbgMode);
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
void UI_FS_EventSetting(void *pvEvent);
void UI_TXPLY_EventSetting(void *pvEventPtr);
void UI_TXFS_UpdateToCU(UI_FS_EventItem_t tEvent,uint8_t ubData1,uint8_t ubData2,uint16_t uwIdx);
void UI_TXPLY_UpdateToCU(UI_TXPLY_EventItem_t tEvent,uint8_t ubData1,uint8_t ubData2,uint16_t uwIdx);
#endif
UI_BUStatus_t *pUI_GetDevSetting(void);
#endif
