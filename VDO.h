/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		VDO.h
	\brief		Video Process Header file for VBM
	\author		Hanyi Chiu
	\version	0.6
	\date		2017/11/27
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _VDO_H_
#define _VDO_H_

#include "APP_HS.h"
#include "BUF.h"
#include "SEN.h"
#include "LCD_TYPE.h"

#define IS_KNL_DISP3T_VIEW(disptype)	((((KNL_DISP_3T_2L1R == disptype) || (KNL_DISP_3T_1L2R == disptype)) ||		\
										  ((KNL_DISP_3T_2T1B == disptype) || (KNL_DISP_3T_1T2B == disptype)) ||		\
										   (KNL_DISP_3T_3COL == disptype))?1:0)

// Default video mode setting
#define VDO_DISP_TYPE					((DISPLAY_MODE == DISPLAY_4T1R)?KNL_DISP_QUAD:(DISPLAY_MODE == DISPLAY_2T1R)?KNL_DISP_DUAL_C:KNL_DISP_SINGLE)
#define VDO_DISP_SCAN					((VDO_DISP_TYPE == KNL_DISP_SINGLE)?FALSE:FALSE)
#if ( APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE )
#define VDO_MAIN_H_SIZE(DispType)		((DispType == KNL_DISP_QUAD)?VGA_WIDTH:(DispType == KNL_DISP_DUAL_C)? HD_WIDTH:FHD_WIDTH)
#define VDO_MAIN_V_SIZE(DispType)		((DispType == KNL_DISP_QUAD)?VGA_HEIGHT:(DispType == KNL_DISP_DUAL_C)? HD_HEIGHT:FHD_HEIGHT)
#else
	#if ((LCD_PANEL == LCD_H5024A) || (LCD_PANEL == LCD_FTD50B7) || (LCD_PANEL == LCD_TC358778_Y50019N00N_NEW))	// || (LCD_PANEL == LCD_MG350BN001)
#define VDO_MAIN_H_SIZE(DispType)       (((KNL_DISP_QUAD == DispType) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_WIDTH:(DispType == KNL_DISP_SINGLE)?SD1_WIDTH:WQVGA_WIDTH)
#define VDO_MAIN_V_SIZE(DispType)       (((KNL_DISP_QUAD == DispType) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_HEIGHT:(DispType == KNL_DISP_SINGLE)?SD1_HEIGHT:WQVGA_HEIGHT)	
	#else
//#define VDO_MAIN_H_SIZE(DispType)		(((KNL_DISP_QUAD == DispType) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_WIDTH:SD1_WIDTH)
//#define VDO_MAIN_V_SIZE(DispType)		(((KNL_DISP_QUAD == DispType) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_HEIGHT:SD1_HEIGHT)
//huang 2024-12-11
#define VDO_MAIN_H_SIZE(DispType)		(((KNL_DISP_QUAD == DispType) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_WIDTH:VGA_WIDTH)
#define VDO_MAIN_V_SIZE(DispType)		(((KNL_DISP_QUAD == DispType) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_HEIGHT:VGA_HEIGHT)
	#endif
#endif
#if defined(VBM_BU)
#define VDO_SUBPATH_ENABLE				(APP_TXREC_STREAM_SEL)
#else
#define VDO_SUBPATH_ENABLE				0
#endif	//! End #ifdef VBM_BU

#if (defined(OP_STA)&&defined(VDO_SUBPATH_ENABLE)&&(VDO_SUBPATH_ENABLE!=0))
#define VDO_SUB_H_SIZE					HD_WIDTH
#define VDO_SUB_V_SIZE					HD_HEIGHT
#else
#define VDO_SUB_H_SIZE					VGA_WIDTH
#define VDO_SUB_V_SIZE					VGA_HEIGHT
#endif

#ifdef S2019A
  #define VGA_RES_FPS					15
  #define HD_RES1T_FPS					15
  #define HD_RES2T_FPS					15
  #define VDO_FRAME_RATE(H, V)			((H == VGA_WIDTH) && (V == VGA_HEIGHT))?VGA_RES_FPS:((DISPLAY_MODE == DISPLAY_1T1R)?HD_RES1T_FPS:HD_RES2T_FPS)
  
 #if (DT_DISPLAY_MODE == DT_DISPLAY_FHD)	
  #define VDO_DT_H_SIZE					FHD_WIDTH	
  #define VDO_DT_V_SIZE					FHD_HEIGHT	
#elif (DT_DISPLAY_MODE == DT_DISPLAY_HD)
  #define VDO_DT_H_SIZE					HD_WIDTH	
  #define VDO_DT_V_SIZE					HD_HEIGHT		
#else // (DT_DISPLAY_MODE == DT_DISPLAY_VGA)
  #define VDO_DT_H_SIZE					VGA_WIDTH	
  #define VDO_DT_V_SIZE					VGA_HEIGHT		
#endif  
#else
  #define VGA_RES_FPS           15     //15	//huang
  #define VDO_FRAME_RATE(H, V)			((H == VGA_WIDTH) && (V == VGA_HEIGHT))?VGA_RES_FPS:HD_SUB_RES_FPS
#endif

//#define HD_SUB_RES_FPS                15	//SU5390 4M/A7130/Other Mode
#define HD_SUB_RES_FPS              12	//SU5390 2M Mode

#ifdef VBM_BU
#if (defined(VDO_SUBPATH_ENABLE)&&(VDO_SUBPATH_ENABLE!=0)) 
#define KNL_SenorSetup(KNL_MainSrcNum, KNL_SubSrcNum)													\
                                        {                                                                                                       \
                                            SEN_SetPathSrc(KNL_MainSrcNum, KNL_SubSrcNum, KNL_SRC_NONE);                                        \
                                            SEN_SetOutResolution(SENSOR_PATH1, VDO_MAIN_H_SIZE(VDO_DISP_TYPE), VDO_MAIN_V_SIZE(VDO_DISP_TYPE)); \
                                            SEN_SetOutResolution(SENSOR_PATH2, VDO_SUB_H_SIZE,  VDO_SUB_V_SIZE);                                \
                                        }
#else
#define KNL_SenorSetup(KNL_MainSrcNum, KNL_SubSrcNum)																							\
										{																										\
											SEN_SetPathSrc(KNL_MainSrcNum, KNL_SRC_NONE, KNL_SubSrcNum);										\
											SEN_SetOutResolution(SENSOR_PATH1, VDO_MAIN_H_SIZE(VDO_DISP_TYPE), VDO_MAIN_V_SIZE(VDO_DISP_TYPE));	\
											SEN_SetOutResolution(SENSOR_PATH3, VDO_SUB_H_SIZE,  VDO_SUB_V_SIZE);								\
										}
#endif
#define KNL_SensorStartProcess()		SEN_InitProcess();

#endif	//! End #ifdef VBM_BU

#ifdef VBM_PU
	#if (LCD_PANEL == LCD_MG350BN001)
#define LCD_H_SIZE						VGA_WIDTH
#define LCD_V_SIZE						VGA_HEIGHT										
	#elif ((LCD_PANEL != LCD_H5024A) && (LCD_PANEL != LCD_FTD50B7))
#define LCD_H_SIZE						HD_WIDTH
#define LCD_V_SIZE						HD_HEIGHT
	#else
#define LCD_H_SIZE						SD1_WIDTH
#define LCD_V_SIZE						SD1_HEIGHT
	#endif
#define	KNL_DISP_ROTATE_SETTING			(KNL_DISP_ROTATE_0) //(KNL_DISP_ROTATE_90)
#define KNL_VdoDisplaySetting()																				\
										{																	\
											KNL_SetDispType(VDO_DISP_TYPE);									\
											KNL_SetDispHV(LCD_H_SIZE, LCD_V_SIZE);							\
											KNL_SetDispRotate(KNL_DISP_ROTATE_SETTING);						\
										}
#define KNL_VdoDisplayParamUpdate()		ubKNL_SetDispCropScaleParam();

#endif	//! End #ifdef VBM_PU

#define KNL_SetVdoResolution(KNL_SrcNum, H_SIZE, V_SIZE)													\
										{																	\
											KNL_SetVdoH(KNL_SrcNum, H_SIZE);								\
											KNL_SetVdoV(KNL_SrcNum, V_SIZE);								\
										}
#define KNL_BufSetup()																						\
										{																	\
											BUF_ResetFreeAddr();											\
											KNL_BufInit();													\
										}

typedef enum
{
	VDO_STOP,
	VDO_START,
}VDO_PlayState_t;

typedef enum
{
	VDO_MAIN_SRC,
	VDO_SUB_SRC,
	VDO_AUX_SRC,
	VDO_SRC_MAX,
}VDO_SrcType_t;

typedef struct
{
	KNL_SRC		tKNL_SrcNum;
	uint8_t		ubVDO_CodecIdx;
}VDO_KNLRoleParam_t;

typedef struct
{
	VDO_KNLRoleParam_t tVDO_KNLParam[VDO_SRC_MAX];
}VDO_KNLRoleInfo_t;

typedef struct
{
	VDO_PlayState_t   tVdoPlaySte[6];
	KNL_DISP_TYPE	  tVdoDispType;
}VDO_Status_t;

typedef struct
{
	void (*VDO_tPsFunPtr)(void);
}VDO_PsFuncPtr_t;

void VDO_Init(void);
void VDO_Setup(void);
void VDO_Start(void);
void VDO_Stop(void);
#ifdef VBM_BU
void VDO_KNLSysInfoSetup(KNL_ROLE tVDO_KNLRole);
#endif
#ifdef VBM_PU
void VDO_UpdateDisplayParameter(void);
void VDO_DisplayLocationSetup(KNL_ROLE tVDO_BURole, KNL_DISP_LOCATION tVDO_DispLocation);
void VDO_SwitchDisplayType(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_BURole);
void VDO_RestartPreview(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole);
void VDO_RemoveDataPath(KNL_ROLE tVDO_BURole);
void VDO_SetFrameColor(uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue);
void VDO_FillFrameBuf(uint8_t ubCamNum);
#if APP_FS_FILE_LIST_STYLE
void VDO_RestartThumbnail(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole);
#endif
#endif
void VDO_DataPathSetup(KNL_ROLE tVDO_KNLRole, VDO_SrcType_t tVDO_SrcType);
void VDO_ChangePlayState(KNL_ROLE tVDO_KNLRole, VDO_PlayState_t tVdoPlySte);
KNL_SRC VDO_GetSourceNumber(KNL_VA_DATAPATH tVDO_Path, KNL_ROLE tVDO_KNLRole);
KNL_ROLE VDO_KNLSrcNumMap2KNLRoleNum(KNL_SRC tVDO_SrcNum);
KNL_DISP_TYPE VDO_GetDispType(void);
#if (OP_STA && APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
void VDO_TXRestartPreview(uint16_t uwH,uint16_t uwV);
#endif
#endif
