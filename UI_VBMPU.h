/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI_VBMPU.h
	\brief		User Interface Header file (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.12
	\date		2018/09/05
	\copyright	Copyright (C) 2018 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _UI_VBMPU_H_
#define _UI_VBMPU_H_

#include "UI.h"
#include "APP_HS.h"
#include "LCD.h"
#include "OSD.h"
#include "RTC_API.h"
#include "FS_API.h"

#define	INVALID_ID 					0xFFFFFFFF
#define INVAILD_ACT					0xFFFFFFFE
#define UI_UPDATESTS_PERIOD			(1000 / UI_TASK_PERIOD)
#define UI_SHOWLOSTLOGO_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_UPDATEBRILVL_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_UPDATEVOLLVL_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_WARNINGNOTE_PERIOD		(3000 / UI_TASK_PERIOD)
#define UI_PHOTOGRAPHSTS_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_RECGRAPHSTS_PERIOD		(1000 / UI_TASK_PERIOD)

#define QUAD_TYPE_ITEM				6
#define SCAN_TYPE_ITEM				5
#define DUAL_TYPE_ITEM				4
#define UI_SLEEP_HISTORY_DEPTH   8 

typedef void (*pvUiFuncPtr)(void);

typedef enum
{
	UI_ICON_NORMAL = 0,
	UI_ICON_HIGHLIGHT,
	UI_ICON_READY
}UI_IconType_t;

typedef enum
{
	UP_ARROW,
	DOWN_ARROW,
	LEFT_ARROW,
	RIGHT_ARROW,
	ENTER_ARROW,
	EXIT_ARROW,
}UI_ArrowKey_t;

//typedef enum
//{
//	KEY_NOT_ACTION,
//	KEY_ACTION,
//}UI_ArrowKeyAct_t;

typedef enum
{
	UI_DISPLAY_STATE = 0x20,			//! Display Image
	UI_MAINMENU_STATE,					//! Menu
	UI_SUBMENU_STATE,					//! Sub menu
	UI_SUBSUBMENU_STATE,				//! Sub sub menu
	UI_SUBSUBSUBMENU_STATE,				//! Sub sub sub menu
	UI_CAM_SEL_STATE,					//! Camera Selection
	UI_SET_VDOMODE_STATE,				//! Setting Video mode
	UI_SET_ADOSRC_STATE,				//! Selection Audio source
	UI_SET_BUECOMODE_STATE,				//! Setting ECO mode of BU
	UI_SET_PUPSMODE_STATE,				//! Setting VOX/WOR mode of PU
	UI_CAMSETTINGMENU_STATE,
	UI_SET_CAMCOLOR_STATE,
	UI_DPTZ_CONTROL_STATE,
	UI_MD_WINDOW_STATE,
	UI_PAIRING_STATE,
	UI_DUALVIEW_CAMSEL_STATE,
	UI_MULTIVIEWSEL_STATE,
	UI_SDFWUPG_STATE,
	UI_VOXPS_STATE,
	UI_ADOONLYPS_STATE,
	UI_RECFOLDER_SEL_STATE,
	UI_RECFILES_SEL_STATE,
	UI_RECTXFOLDER_SEL_STATE,
	UI_RECTXFILES_SEL_STATE,
	UI_RECPLAYLIST_STATE,
	UI_RECPLAYADOSRC_SEL_STATE,
	UI_PHOTOPLAYNRDY_STATE,
	UI_PHOTOPLAYLIST_STATE,
	UI_SDCARDFMT_STATE,
	UI_SHOWSTSICON_STATE,
	UI_ENGMODE_STATE,
	UI_FWUPG_STATE,
	UI_SPRF_SEL_STATE,
	UI_DT_SEL_STATE,
	UI_STATE_MAX,
}UI_State_t;

typedef enum
{
	CAMERAS_ITEM,
	PAIRING_ITEM,
	RECORD_ITEM,
	PHOTO_ITEM,
	PLAYBACK_ITEM,
	PS_ITEM,
	SETTING_ITEM,
	MENUITEM_MAX,
}UI_MenuItemList_t;

typedef enum
{
	NOT_ACTION,
	DRAW_MENUPAGE,
	DRAW_HIGHLIGHT_MENUICON,
	EXECUTE_MENUFUNC,
	EXIT_MENUFUNC,
}UI_MenuAct_t;

typedef enum
{
//	CAMVIEW_ITEM,
//	PTZ_ITEM,
//	CAMBL_ITEM,
	CAMSSELCAM_ITEM,
	CAMSANR_ITEM,
	CAMS3DNR_ITEM,
	CAMSvLDS_ITEM,
	CAMSAEC_ITEM,
	CAMSDIS_ITEM,
	CAMSCBR_ITEM,
	CAMSCONDENSE_ITEM,
	CAMSFLICKER_ITEM,
	CAMSITEM_MAX
}UI_CamsSubMenuItemList_t;

typedef enum
{
	CAMSET_OFF,
	CAMSET_ON,
}UI_CamsSetMode_t;

typedef enum
{
	CAMFLICKER_50HZ,
	CAMFLICKER_60HZ,
}UI_CamFlicker_t;

typedef enum
{
	CAM1VIEW_ITEM,
	CAM2VIEW_ITEM,
	CAM3VIEW_ITEM,
	CAM4VIEW_ITEM,	
	QUALVIEW_ITEM,
	CAMVIEWITEM_MAX,
	SINGLEVIEW_ITEM = 1,
	DUALVIEW_ITEM = 3,
}UI_CamsSubSubMenuItemList_t;

typedef enum
{
	PAIRCAM_ITEM,
	DELCAM_ITEM,
	PAIRITEM_MAX
}UI_PairSubMenuItemList_t;

typedef enum
{
//	RECSELCAM_ITEM,
//	RECMODE_ITEM,
//	RECRES_ITEM,
//	SDCARD_ITEM,
	RECMODE_ITEM,
	RECTIME_ITEM,
	SDCARD_ITEM,
	RECITEM_MAX
}UI_RecordSubMenuItemList_t;

typedef enum
{
	REC_LOOPING,
	REC_MANUAL,
	REC_TRIGGER,
	REC_OFF,
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
	RECTIME_1MIN,
	RECTIME_3MIN,
	RECTIME_5MIN,
	RECTIME_MAX,
}UI_RecordTime_t;

typedef enum
{
	PHOTOSELCAM_ITEM,
	PHOTOFUNC_ITEM,
	PHOTORES_ITEM,
	PHOTOITEM_MAX
}UI_PhotoSubMenuItemList_t;

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
	DATETIME_ITEM,
	AECSET_ITEM,
	CCASET_ITEM,
	STORAGESTS_ITEM,
	LANGUAGESET_ITEM,
	DEFUSET_ITEM,
	SWUSBDMODE_ITEM,
	SETTINGITEM_MAX
}UI_SettingSubMenuItemList_t;

typedef enum
{
	AECFUNC_OFF,
	AECFUNC_ON,
	AECFUNC_MAX,
}UI_AecFunction_t;

typedef enum
{
	CCAFUNC_OFF,
	CCAFUNC_ON,
	CCAMODE_MAX,
}UI_CCAMode_t;

typedef enum
{
	YEAR_ITEM,
	MONTH_ITEM,
	DATE_ITEM,
	HOUR_ITEM,
	MIN_ITEM,
	SEC_ITEM,
	ALLCALE_ITEM,
}UI_CalendarItem_t;

typedef struct
{
	void (*pvFuncPtr)(UI_ArrowKey_t);
}UI_MenuFuncPtr_t;

typedef struct
{
	void (*pvFuncPtr)(void);
}UI_DrawSubMenuFuncPtr_t;

typedef struct
{
	uint8_t 	 ubItemIdx;
	uint8_t 	 ubItemPreIdx;
}UI_MenuItem_t;

typedef struct
{
	uint8_t 	  ubFirstItem;
	uint8_t 	  ubItemCount;
	UI_MenuItem_t tSubMenuInfo;
}UI_SubMenuItem_t;

typedef struct
{
	UI_CamNum_t tCamNum4CamSetSub;
	UI_CamNum_t tCamNum4CamView;	
	UI_CamNum_t tCamNum4RecSub;
	UI_CamNum_t tCamNum4PhotoSub;
}UI_SubMenuCamNum_t;

typedef struct
{
	UI_SubMenuItem_t tCameraViewPage;
}UI_CamsSubSubMenuItem_t;

typedef struct
{
	UI_SubMenuItem_t tPairS[PAIRITEM_MAX];
}UI_PairSubSubMenuItem_t;

typedef struct
{
	UI_SubMenuItem_t tRecordS[RECITEM_MAX];
}UI_RecSubSubMenuItem_t;

typedef struct
{
	UI_SubMenuItem_t tPhotoS[CAM_4T][PHOTOITEM_MAX];
}UI_PhotoSubSubMenuItem_t;

typedef struct
{
	UI_SubMenuItem_t tSettingS[SETTINGITEM_MAX];
}UI_SettingSubSubMenuItem_t;

typedef struct
{
	uint8_t 	 ubItemIdx;
	uint8_t 	 ubItemPreIdx;
}UI_SettingSubSubSubItem_t;

typedef enum
{
	UI_OsdUpdate,
	UI_OsdErase,
}UI_OsdImgFnType_t;

typedef struct
{
	UI_CamViewType_t tCamViewType;
	UI_CamNum_t 	 tCamViewPool[CAM_4T];
}UI_CamViewSelect_t;

typedef struct
{
	uint32_t ulCardSize;
	uint32_t ulRemainSize;
}UI_SdInfo_t;

typedef enum
{
	UI_SD_CFM,
	UI_SD_NRDY,
	UI_SD_RDY,
}UI_SdSts_t;

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
	CAM_OFFLINE,
	CAM_ONLINE,
	CAM_STSMAX,
}UI_CamConnectStatus_t;

typedef enum
{
	ANT_SIGNALLVL5,
	ANT_SIGNALLVL4,
	ANT_SIGNALLVL3,
	ANT_SIGNALLVL2,
	ANT_SIGNALLVL1,
	ANT_NOSIGNAL,
}UI_AntLvl_t;

typedef enum
{
	BAT_LVL4,
	BAT_LVL3,
	BAT_LVL2,
	BAT_LVL1,
	BAT_LVL0,
	BAT_CHARGE,
}UI_BatLvl_t;

typedef enum
{
	BL_LVL0,
	BL_LVL1,
	BL_LVL2,
	BL_LVL3,
	BL_LVL4,
	BL_LVL5,
}UI_BrightnessLvl_t;

typedef struct
{
	UI_CamNum_t 			tPairSelCam;
	UI_DisplayLocation_t 	tDispLocation;
	uint8_t					ubDrawFlag;
}UI_PairingInfo_t;

typedef struct
{
	UI_AntLvl_t tAntLvl;
	uint8_t 	ubRssiValue;
}UI_RssiMap2AntLvl_t;

typedef struct
{
	UI_AntLvl_t tAntLvl;
	uint8_t 	ubPerValue;
}UI_PerMap2AntLvl_t;

typedef enum
{
	UI_SCALEUP_2X = 2,
	UI_SCALEUP_4X,
}UI_ScaleUp_t;

typedef enum
{
	DUAL_CAM1_CAM2,
	DUAL_CAM1_CAM3,
	DUAL_CAM1_CAM4,
	DUAL_CAM2_CAM1,
	DUAL_CAM2_CAM3,
	DUAL_CAM2_CAM4,
	DUAL_CAM3_CAM1,
	DUAL_CAM3_CAM2,
	DUAL_CAM3_CAM4,
	DUAL_CAM4_CAM1,
	DUAL_CAM4_CAM2,
	DUAL_CAM4_CAM3,
	DUAL_VIEWCAMSEL_MAX,
}UI_DualViewCamSel_t;
	
typedef enum
{
	UI_CAMISP_SETUP,
	UI_CAMFUNC_SETUP,
}UI_CameraSettingMenu_t;

typedef enum
{
	UI_COLOR_ITEM,
	UI_DPTZ_ITEM,
	UI_MD_ITEM,
}UI_CameraSettingItem_t;

typedef struct
{
	LCD_DYN_INFOR_TYP tUI_LcdCropParam;
	UI_ScaleUp_t	  tScaleParam;
}UI_DPTZParam_t;

typedef struct
{
	uint8_t ubColorBL;
	uint8_t ubColorContrast;
	uint8_t ubColorSaturation;
	uint8_t ubColorHue;
}UI_ColorParam_t;

typedef enum
{
	UI_OSDLDDISP_OFF,
	UI_OSDLDDISP_ON,
}UI_OsdLdDispSts_t;

typedef enum
{
	UI_SEARCH_DCIMFOLDER,
	UI_SEARCH_RECFILES,
	UI_SDCARD_FORMAT,
	UI_LD_DEFU,
}UI_LoadingState_t;

typedef struct
{
	uint16_t uwFunc;
	uint8_t  ubType;
	uint8_t  ubCamNum;
	uint16_t uwFuncMsg;
}UI_FuncExecMsg_t;

typedef enum
{
	UI_SDCARDFMT_ACT,
	UI_PERDBGRPT_ACT,
}UI_FuncWorkAct_t;

typedef enum
{
	UI_RECORDING_MODE,
	UI_PHOTOCAP_MODE,
	UI_VDOMODE_MAX
}UI_VdoMode_t;

typedef enum
{
	UI_VDORECLOOP_MODE,
	UI_VDORECMANU_MODE,
	UI_VDORECTRIG_MODE,
	UI_VDOPHOTO_MODE,
	UI_VDOMODELIST_MAX
}UI_VdoModeList_t;

typedef enum
{
	UI_RECIMG_COLOR1,
	UI_RECIMG_COLOR2,
	UI_RECIMG_COLOR3,
	UI_RECIMG_COLOR4,
	UI_RECIMG_COLOR5,
	UI_RECIMG_COLOR6,
	UI_RECIMG_DEFU = 0xFF,
}UI_RecImgColor_t;

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

typedef enum
{
	UI_PLY_PREV,      // File Index--
	UI_PLY_NEXT,      // File Index++
	UI_PLY_KEEP,      // Keep Last Frame
	UI_PLY_ONCE,      // Play once
}UI_PLY_NextItem_t;

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

#define REC_FOLDER_LIST_MAXNUM	10
#if !APP_FS_FILE_LIST_STYLE
#define REC_FILE_LIST_MAXNUM	10
#else
#define REC_FILE_LIST_MAXNUM	9
#endif
#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */

typedef struct
{
	uint16_t uwUI_RecNumArray[10];
	uint16_t uwUI_RecUpperLetterArray[26];
	uint16_t uwUI_RecLowerLetterArray[26];
	uint16_t uwUI_RecSymbolArray[4];
}UI_RecOsdImgDb_t;

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
	UI_WARN_RECMODE,
}UI_WarnNote_t;

typedef struct
{
	uint32_t 			  ulCAM_ID;
	UI_DisplayLocation_t  tCamDispLocation;
	UI_CamConnectStatus_t tCamConnSts;
	UI_CamVdoMode_t		  tCamVdoMode;
	UI_AntLvl_t			  tCamAntLvl;
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
	
	uint8_t             ubMicroSensitivity;		//zhu
	char                name[20];
  uint32_t            sleep_time_ms;
  uint8_t             priority;
	uint8_t							active; 
	uint8_t             sleep_hist_count;		
  uint8_t             sleep_hist_head;
  uint32_t            sleep_hist_ms[UI_SLEEP_HISTORY_DEPTH];		//4*8
	uint8_t				  ubReserved[155];		//original 216
}UI_BUStatus_t;

typedef struct
{
	uint8_t 				ubTotalBuNum;
	uint8_t 				ubPairedBuNum;
	RTC_Calendar_t 			tSysCalendar;
	uint8_t 				ubAEC_Mode;
	uint8_t 				ubCCA_Mode;
	uint8_t 				ubSTORAGE_Mode;
	struct
	{
		uint8_t				ubDrawStsIconFlag;
		uint8_t				ubRdPairIconFlag;
		uint8_t				ubClearThdCntFlag;
		uint8_t 			ubShowLostLogoFlag;
		uint8_t				ubDrawNoCardIconFlag;
		uint8_t 			ubDrawMdTrigFlag;
	}IconSts;
	struct
	{
		UI_BrightnessLvl_t	tBL_UpdateLvL;
		uint8_t				ubBL_UpdateCnt;
	}BriLvL;
	struct
	{
		UI_VolumeLvl_t		tVOL_UpdateLvL;
		uint8_t				ubVOL_UpdateCnt;
	}VolLvL;
	UI_CamNum_t				tAdoSrcCamNum;
	UI_PowerSaveMode_t		tPsMode;
	struct
	{
		UI_RecordMode_t		tREC_Mode;
		UI_RecordTime_t		tREC_Time;
	}RecInfo;
	UI_VdoMode_t			tVdoMode;
	uint8_t					ubVdoRecStsCnt;
	struct
	{
		UI_WarnNote_t 		tWarnNote;
		uint8_t				ubWarnUpdateCnt;
	}WarnIcon;
	
	uint8_t					ubReserved[211];		//reduce appropriately after change, original: 256, unchanged:  221
	uint8_t					ubVibration;		//zhu
	uint8_t					ubZoomLevel;
	uint8_t         ubLcdBrightness;
  uint8_t         ubFlipImage;
	uint8_t         ubTextTransparency;
	uint8_t					ubLanguage;
	uint8_t         ubTempAlarmOn;
  uint8_t         ubTempMax;
  uint8_t         ubTempMin;
	uint8_t         ubKeyLockAutoActivate;
}UI_PUSetting_t;

typedef struct
{
	char 		   cbUI_DevStsTag[11];
	char 		   cbUI_FwVersion[11];
	UI_PUSetting_t tPU_SettingInfo;
	UI_BUStatus_t  tBU_StatusInfo[CAM_4T];
}UI_DeviceStatusInfo_t;

#ifdef S2019A
typedef struct
{
	sPRF_DrvMode_t  tUI_DrvMd;
	uint8_t 	    ubWorkCh;
	uint8_t		    ubDtRdy[4];
	uint8_t		    ubDtLink[4];
}UI_DevDrvStsInfo_t;
#endif

#pragma pack(pop)

#define UI_CLEAR_THREADCNT(Flag, Count)			do { if(Flag == TRUE) { (Count) = 0; Flag = FALSE; } } while(0)
#define UI_CLEAR_CAMSETTINGTODEFU(xFUNC, mDEFU)	do { xFUNC = mDEFU; } while(0)
#define UI_CLEAR_CALENDAR_TODEFU(xFUNC, mDEFU)	do { xFUNC = mDEFU; } while(0)

#define UI_CHK_CAMSFUNCS(Mode, Status)			do { if(Mode > CAMSET_ON) { Mode = Status; } } while(0)
#define UI_CHK_CAMFLICKER(HZ)					do { if(HZ > CAMFLICKER_60HZ) { HZ = CAMFLICKER_50HZ; } } while(0)
#define UI_CHK_CAMPARAM(Param, Value)			do { if(Param >= 128) { Param = Value; } } while(0)
#define UI_CHK_BUSYS(SysParam, Limit, Target)	do { if(SysParam >= Limit) { SysParam = Target; } } while(0)
#define UI_CHK_PUSYS(SysParam, Limit, Target)	do { if(SysParam >= Limit) { SysParam = Target; } } while(0)

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

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
typedef struct
{
	UI_CamNum_t 	tDS_CamNum;
	uint8_t 		ubCmd_Len;
	uint8_t			ubCmd[8];
}UI_PUReqCmd_t;

typedef struct
{
	osThreadId	thread_id;
	int32_t		iSignals;
	UI_Result_t	tReportSts;
}UI_ThreadNotify_t;
#pragma pack(pop)

typedef struct
{
	void (*pvAction)(UI_CamNum_t, void *);
}UI_ReportFuncPtr_t;

typedef struct
{
	void (*pvAction)(UI_CamNum_t, void *);
}UI_SettingFuncPtr_t;

extern UI_PUSetting_t tUI_PuSetting;
void UI_ApplyFlipImageSetting(void);  //zhu
void UI_PowerKey(void);
void UI_MenuKey(void);
void UI_UpArrowKey(void);
void UI_DownArrowKey(void);
void UI_LeftArrowKey(void);
void UI_RightArrowKey(void);
void UI_EnterKey(void);
void UI_PuPowerSaveKey(void);
void UI_BuPowerSaveKey(void);
void UI_PuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey);
void UI_BuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey);
void UI_PushTalkKey(void);
void UI_DrawCameraSettingMenu(UI_CameraSettingMenu_t tCamSetMenu);
void UI_CameraSettingMenu1Key(void);
void UI_CameraSettingMenu2Key(void);
void UI_CameraSettingMenu(UI_ArrowKey_t tArrowKey);
void UI_CameraColorSetting(UI_ArrowKey_t tArrowKey);
UI_Result_t UI_DPTZ_KeyPress(uint8_t ubKeyID, uint8_t ubKeyMapIdx);
void UI_DPTZ_KeyRelease(uint8_t ubKeyID);
void UI_DPTZ_Control(UI_ArrowKey_t tArrowKey);
void UI_MD_Window(UI_ArrowKey_t tArrowKey);
void UI_CameraSelectionKey(void);
void UI_CameraMultiViewSelection(UI_ArrowKey_t tArrowKey);
void UI_CameraSelection(UI_ArrowKey_t tArrowKey);
void UI_CameraSelection4DualView(UI_ArrowKey_t tArrowKey);
void UI_ChangeVideoMode(UI_ArrowKey_t tArrowKey);
void UI_ChangeAudioSourceKey(void);
void UI_ChangeAudioSource(UI_ArrowKey_t tArrowKey);
void UI_DisplayArrowKeyFunc(UI_ArrowKey_t tArrowKey);
void UI_DrawMenuPage(void);
void UI_DrawSubMenuPage(UI_MenuItemList_t MenuItem);
void UI_Menu(UI_ArrowKey_t tArrowKey);
void UI_SubMenu(UI_ArrowKey_t tArrowKey);
void UI_SubSubMenu(UI_ArrowKey_t tArrowKey);
void UI_SubSubSubMenu(UI_ArrowKey_t tArrowKey);
void UI_CameraSettingSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_CameraSettingSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PairingSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PairingSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PairingSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_DrawPairingStatusIcon(void);
void UI_ReportPairingResult(UI_Result_t tResult);
void UI_ReportAppPairingResult(UI_Result_t tResult);
void UI_DrawDCIMFolderMenu(void);
void UI_DrawRecordFileMenu(void);
void UI_DCIMFolderSelection(UI_ArrowKey_t tArrowKey);
void UI_RecordFileSelection(UI_ArrowKey_t tArrowKey);
void UI_RecordPlayListSelection(UI_ArrowKey_t tArrowKey);
void UI_RecordPlayAdoSrcSelection(UI_ArrowKey_t tArrowKey);
void UI_PhotoPlayListSelection(UI_ArrowKey_t tArrowKey);
void UI_RecordSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_RecordSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PhotoSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PhotoSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PlaybackSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PowerSaveSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSysDateTimeSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_ResetSubMenuInfo(void);
void UI_ResetSubSubMenuInfo(void);
UI_CamNum_t UI_ChangeSelectCamNum4UiMenu(UI_CamNum_t *tCurrentCamNum, UI_ArrowKey_t *ptArrowKey);
void UI_DrawHLandNormalIcon(uint16_t uwNormalOsdImgIdx, uint16_t uwHighLigthOsdImgIdx);
UI_MenuAct_t UI_KeyEventMap2SubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubMenu);
UI_MenuAct_t UI_KeyEventMap2SubSubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubSubMenuItem);
void UI_UpdateBriLvlIcon(void);
void UI_UpdateVolLvlIcon(void);
void UI_DrawPUStatusIcon(void);
void UI_RemoveLostLinkLogo(void);
void UI_ShowLostLinkLogo(uint16_t *pThreadCnt);
void UI_UpdateBuStatusOsdImg(OSD_IMG_INFO *pOsdImgInfo, OSD_UPDATE_TYP tUpdateMode, 
	                         UI_OsdImgFnType_t tOsdImgFnType, UI_DisplayLocation_t tDispLoc);
void UI_ClearBuConnectStatusFlag(void);
UI_DisplayLocation_t tUI_GetTripViewDispLoc(UI_CamNum_t tCamNum);
void UI_RedrawBuConnectStatusIcon(UI_CamNum_t tCamNum);
void UI_RedrawNoSignalOsdIcon(UI_CamNum_t tCamNum, UI_OsdImgFnType_t tOsdImgFnType);
void UI_ClearStatusBarOsdIcon(void);
void UI_RedrawStatusBar(uint16_t *pThreadCnt);
void UI_ReportBuConnectionStatus(void *pvConnectionSts);
void UI_UpdateBuStatus(UI_CamNum_t tCamNum, void *pvStatus);
void UI_LeftArrowLongKey(void);
void UI_RightArrowLongKey(void);
void UI_ShowSysTime(void);
void UI_UnBindBu(UI_CamNum_t tUI_DelCam);
void UI_VoxTrigger(UI_CamNum_t tCamNum, void *pvTrig);
void UI_EnableVox(void);
void UI_DisableVox(void);
void UI_DisablePuAdoOnlyMode(void);
void UI_MDTrigger(UI_CamNum_t tCamNum, void *pvTrig);
void UI_VoiceTrigger(UI_CamNum_t tCamNum, void *pvTrig);
UI_Result_t UI_SendRequestToBU(osThreadId thread_id, UI_PUReqCmd_t *ptReqCmd);
void UI_RecvBUResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus);
void UI_RecvBURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data);
void UI_ResetDevSetting(UI_CamNum_t tCamNum);
void UI_LoadDevStatusInfo(void);
void UI_UpdateDevStatusInfo(void);
void UI_EnableScanMode(void);
void UI_DisableScanMode(void);
void UI_ScanModeExec(void);
UI_Result_t UI_CheckCameraSource4SV(void);
void UI_SwitchCameraSource(void);
void UI_SwitchAudioSource(UI_CamNum_t tCamNum);
void UI_SwitchCameraSource4TripleView(void);
void UI_EngModeKey(void);
void UI_EngModeCtrl(UI_ArrowKey_t tArrowKey);
void UI_DisplayAppPairingScreen(void);
void UI_PairingControl(UI_ArrowKey_t tArrowKey);
void UI_FwUpgViaSdCard(void);
void UI_FwUpgExecSel(UI_ArrowKey_t tArrowKey);
void UI_UpdateRecStsIcon(void);
void UI_UpdateWarningNoteIcon(void);
void UI_SdCardFormatFunc(UI_ArrowKey_t tArrowKey);
void UI_PhotoCaptureFinish(uint8_t ubPhotoCapRet);
void UI_PhotoPlayFinish(uint8_t ubPhtoCapRet);
void UI_VideoRecordingExec(UI_RecordingAct_t tRecAct);
void UI_OsdDisplayFrmErrItem(UI_DisplayLocation_t tDispLoc);
void UI_DisplayTrxInfo(UI_FuncExecMsg_t tPerRpt);
void UI_EnPerDebugMode(void);
void UI_ClearOsdImage(void);
void UI_UpdateDrvStatusInfo(void);
void UI_LoadDrvStatusInfo(void);
#ifdef S2019A
void UI_SetSPRfWorkCh(void);
#endif
void UI_sPRfChSelection(UI_ArrowKey_t tArrowKey);
void UI_SwitchDrvMd(void);
void UI_DriveMdSelection(UI_ArrowKey_t tArrowKey);
void UI_PairingDirectlyCAM1(void);
void UI_PairingDirectlyCAM2(void);
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
void UI_DrawTXFolderMenu(void);
void UI_DrawTXRecordFileMenu(void);
void UI_ReDrawTXRecordFileMenu(void);
void UI_DCIMTXFolderSelection(UI_ArrowKey_t tArrowKey);
void UI_RecordTXFileSelection(UI_ArrowKey_t tArrowKey);
void UI_StopRemotePlay(void);
void UI_FSTrigger(UI_CamNum_t tCamNum, void *pvTrig);
void UI_TXPLYTrigger(UI_CamNum_t tCamNum, void *pvTrig);
void UI_GetTXFsInfo(UI_CamNum_t tCamNum,UI_FS_EventItem_t tEvent,UI_FSSettingItem_t tType,KNL_SORTMODE_t tSortMode,uint16_t uwIdx);
uint8_t UI_TXFsDrawEvent(UI_CamNum_t tCamNum,UI_FS_EventItem_t tEvent,UI_FSSettingItem_t tType,KNL_SORTMODE_t tSortMode,uint16_t uwIdx);
void UI_TXPlayEvent(UI_CamNum_t tCamNum,UI_TXPLY_EventItem_t tEvent,uint8_t ubData1,uint8_t ubData2,uint16_t uwIdx);
void UI_FSInfoInstall(void);
void UI_TxFsCbFunc(uint8_t ubType);
#endif
#endif
