/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		RC.h
	\brief		Rate Control header file
	\author		Justin Chen
	\version	1.23
	\date		2020/12/11
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __RC_h
#define __RC_h

#include <stdio.h>
#include "_510PF.h"

#define RC_DEBUG_EN				1
#define RC_SECTION_NUM			5

#ifdef VBM_BU		//For VBM
#define RC_MIN_FPS				5
#else				//For BUC
#define RC_MIN_FPS				10
#endif

#define RC_MAX_BW_A1_SERIES		(295*1024*8L)	//Bit-Rate
#define RC_MAX_BW_A2_SERIES		(145*1024*8L)	//Bit-Rate
#define RC_MAX_BW_A4_SERIES		(75*1024*8L)	//Bit-Rate

//Setting for SONIX SU5390(2M Mode)
#define RC_MAX_BW_S1_SERIES		(150*1024*8L)	//Bit-Rate
#define RC_MAX_BW_S2_SERIES		(75*1024*8L)	//Bit-Rate
#define RC_MAX_BW_S4_SERIES		(37*1024*8L)	//Bit-Rate
#define RC_MAX_BW_R1_SERIES		(438*1024*8L)	//Bit-Rate
#define RC_MAX_BW_R2_SERIES		(219*1024*8L)	//Bit-Rate
#define RC_MAX_BW_R4_SERIES		(109*1024*8L)	//Bit-Rate

typedef enum
{	
	RC_COMM_CH1 = 0,
	RC_COMM_CH2,
}RC_COMM_CH;

typedef enum
{	
	RC_COMM_UTILITY_10 = 0,	//!<  10%
	RC_COMM_UTILITY_20,		//!<  20%
	RC_COMM_UTILITY_30,		//!<  30%
	RC_COMM_UTILITY_40,		//!<  40%
	RC_COMM_UTILITY_50,		//!<  50%
	RC_COMM_UTILITY_60,		//!<  60%
	RC_COMM_UTILITY_70,		//!<  70%
	RC_COMM_UTILITY_80,		//!<  80%
	RC_COMM_UTILITY_90,		//!<  90%
	RC_COMM_UTILITY_100,	//!< 100%	
}RC_COMM_UTILITY;

typedef enum
{	
	RC_MODE_FIXED_DATA_RATE = 0,
	RC_MODE_DYNAMIC_QTY_AND_BW, 	
	RC_MODE_DYNAMIC_QTY_AND_FPS,
	RC_MODE_DYNAMIC_FPS2,			//For BUC Application
}RC_MODE;

typedef enum
{	
	RC_MODE_NONE = 0,
	RC_MODE_QTY_AND_BW, 	
	RC_MODE_QTY_AND_FPS,
	RC_MODE_FPS_AND_QTY,
}RC_MODE2;

typedef enum
{	
	RC_ADJ_QTY_THEN_FPS = 0,
	RC_ADJ_FPS_THEN_QTY,
}RC_ADJ;

typedef enum
{	
	RC_RATIO_10 = 0,	//!< 10%
	RC_RATIO_20,		//!< 20%
	RC_RATIO_30,		//!< 30%
	RC_RATIO_40,		//!< 40%
	RC_RATIO_45,		//!< 45%
	RC_RATIO_50,		//!< 50%
	RC_RATIO_60,		//!< 60%
	RC_RATIO_70,		//!< 70%	
	RC_RATIO_75,		//!< 75%	
	RC_RATIO_80,		//!< 80%
	RC_RATIO_85,		//!< 85%	
	RC_RATIO_90,		//!< 90%
	RC_RATIO_100,		//!< 100%
}RC_RATIO;

typedef enum
{	
	RC_DISABLE = 0, 
	RC_ENABLE = 1,
}RC_SWITCH;

typedef enum
{	
	RC_STATUS_RDY = 0, 
	RC_STATUS_SETUP
}RC_STATUS;

typedef struct
{
	uint8_t 	ubEnableFlg;
	uint8_t 	ubCodecIdx;
	uint8_t 	ubMode;
	RC_ADJ		tAdjSequence;
	uint32_t	ulUpdateRatePerMs;
	uint8_t 	ubRefreshRate;
	uint8_t 	ubKeySecRatio;
	uint8_t 	ubNonKeySecRatio;
	
	uint8_t 	ubMinQp;
	uint8_t 	ubMaxQp;
	uint8_t 	ubTargetQp;
	uint8_t 	ubQpBuf;

	uint32_t 	ulInitBitRate;
	uint32_t 	ulStepBitRate;
	uint32_t 	ulBwBuf;
	uint32_t  	ulBwTh[9];			//10 Level BW
	
	uint8_t 	ubInitFps;
	uint8_t		ubMaxFps;
	uint8_t 	ubMinFps;
	uint8_t 	ubStepFps;	
	uint8_t 	ubFpsBuf;
	uint8_t 	ubTargetFps[10];	//10 Level FPS
	
	uint8_t 	ubTarMaxQp[10];		//10 Level MAX.QP
	uint8_t 	ubTarMinQp[10];		//10 Level MIN.QP
	
	uint32_t 	ulHighQtyTh;
	uint32_t 	ulLowQtyTh;	
	uint8_t 	ubHighBwMaxQp;
	uint8_t 	ubHighBwMinQp;	
	uint8_t 	ubMediumBwMaxQp;
	uint8_t 	ubMediumBwMinQp;	
	uint8_t 	ubLowBwMaxQp;
	uint8_t 	ubLowBwMinQp;
	
	uint8_t 	ubFpsSectionNum;
}RC_INFO;

typedef enum
{
	RC_NONE = 0,
	RC_FIXED_DATA_RATE,
	RC_QTY_AND_BW,
	RC_QTY_THEN_FPS,
	RC_FPS_THEN_QTY,
	RC_DYNAMIC_FPS,
	RC_QTY_AND_DYNAFPS,
	RC_DT_DYNAQTYFPS,
	RC_PRESET_MAX
}RC_Rreset_t;

typedef struct
{
	void (*pvPresetFunc)(void);
}RC_PresetSetup_t;

//------------------------------------------------------------------------------
/*!
\brief 	Get RC Function Version	
\return	Unsigned short value, high byte is the major version and low byte is the minor version
\par [Example]
\code		 
	 uint16_t uwVer;
	 
	 uwVer = uwRC_GetVersion();
	 printf("RC Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
\endcode
*/
uint16_t uwRC_GetVersion(void);

uint8_t ubRC_GetTargetQp(uint8_t ubCodecIdx);
uint8_t ubRC_GetMinQp(uint8_t ubCodecIdx);
uint8_t ubRC_GetMaxQp(uint8_t ubCodecIdx);
uint32_t ulRC_GetInitBitRate(uint8_t ubCodecIdx);

uint8_t ubRC_GetFps(void);	

//------------------------------------------------------------------------
/*!
\brief Set RC Function
\param ubEnableFlg 0->Disable, 1->Enable
\param tInfo RC Information
\return(no)
*/
void RC_FuncSet(RC_INFO *pInfo);

//------------------------------------------------------------------------------
/*!
\brief 	Set RC Function Enable Flag	
\return(no)
*/
void ubRC_SetFlg(uint8_t ubCodecIdx, uint8_t ubRcCtrlFlg);

//------------------------------------------------------------------------------
/*!
\brief 	Get RC Function Enable Flag	
\return	Flag, 0->Diable RC, 1->Enable RC
*/
uint8_t ubRC_GetFlg(uint8_t ubCodecIdx);

//------------------------------------------------------------------------------
/*!
\brief 	Get RC RC Refresh Rate (Sec)	
\return	Second
*/
uint8_t ubRC_GetRefreshRate(uint8_t ubCodecIdx);

//------------------------------------------------------------------------------
/*!
\brief 	Get RC Final Bit-Rate	
\return	Final Bit-Rate
*/
uint32_t ulRC_GetFinalBitRate(uint8_t ubCodecIdx);

//------------------------------------------------------------------------
/*!
\brief Initial Rate Control
\return(no)
*/
void RC_Init(uint8_t ubCodecIdx);

void RC_VariableInit(void);

uint8_t ubRC_GetUpdateFlg(uint8_t ubCodecIdx);
void RC_SetUpdateFlg(uint8_t ubCodecIdx,uint8_t ubFlg);

uint8_t ubRC_GetOpMode(uint8_t ubCodecIdx);

#if OP_STA
//------------------------------------------------------------------------
/*!
\brief RC monitor thread
\param argument Argument for monitor thread
\return(no)
*/
void RC_MonitThread0(void const *argument);
void RC_MonitThread1(void const *argument);
void RC_MonitThread2(void const *argument);
void RC_MonitThread3(void const *argument);

//------------------------------------------------------------------------
/*!
\brief RC system thread
\param argument Argument for monitor thread
\return(no)
*/
void RC_SysThread(void const *argument);


void RC_EnHighQualityPresetMode(uint8_t ubEn, uint8_t ubCodecIdx);

#endif
void RC_EnUvcPresetMode(uint8_t ubEn, uint8_t ubCodecIdx);

//------------------------------------------------------------------------
/*!
\brief Setup RC Preset
\param tRC_Preset 	Preset parameter
\return(no)
*/
void RC_PresetSetup(RC_Rreset_t tRC_Preset);

//RC Setting for IQ-Tuning
void RC_EngModeSet(uint8_t ubCodecIdx,uint32_t ulTarBitRate,uint8_t ubFps);
uint32_t ulRC_GetEngModeBitRate(uint8_t ubCodecIdx);
uint8_t ubRC_GetEngModeFps(uint8_t ubCodecIdx);


//------------------------------------------------------------------------
/*!
\brief Audio Latency Control  thread
\param argument Argument for monitor thread
\return(no)
*/
void RC_MonitLatencyThread(void const *argument);
//------------------------------------------------------------------------

void RC_QtyAndBwModePresetFor1SlotMode(void);
void RC_QtyAndBwModePresetFor2SlotMode(void);
void RC_QtyAndBwModePresetFor4SlotMode(void);

//void RC_DynamicFpsMode(uint8_t ubMaxFps,uint8_t ubMinFps,uint8_t ubSectionNum);
void RC_DynamicFpsMode(uint8_t ubMaxFps,uint8_t ubMinFps,uint8_t ubSectionNum,float fRatio);
void RC_EnterSetupMode(void);
void RC_ExitSetupMode(void);
RC_STATUS tRC_GetSysStatus(void);

uint8_t ubRC_GetFinalMinQp(uint8_t ubCodecIdx);
uint8_t ubRC_GetFinalMaxQp(uint8_t ubCodecIdx);

#if OP_STA
uint8_t ubRC_GetAvgQp(uint8_t ubCodecIdx);
#endif

uint8_t ubRC_QtyAndDynaFpsProcess(uint8_t ubCodecIdx);
#ifdef S2019A
//#define RC_MONITHD_DIS
void RC_UpdateDynaFpsParam(uint8_t ubMaxFps, uint8_t ubMinFps);
#ifdef OP_STA
uint8_t ubRC_ChkAdjQpRange(uint8_t ubCodecIdx);
void RC_DtModeReset(uint8_t ubCodecIdx, uint8_t ubRstFlag);
#endif
#endif

#endif
