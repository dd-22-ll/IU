/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		RC.c
	\brief		Rate Control function
	\author		Chris
	\version	1.27
	\date		2020/12/11
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#include <string.h>
#include "RC.h"
#include "APP_CFG.h"
#include "BB_API.h"
#include "KNL.h"
#include "SEN.h"
#include "CLI.h"
#include "H264_API.h"
#include "IQ_PARSER_API.h"
#include "USBD_API.h"
#include "APP_HS.h"
#include "VDO.h"
#include "TRX_IF.h"
#include "WDT.h"
#if (defined(OP_STA) && defined(S2019A) && defined(RVCS_APP))
#include "WiFi_API.h"
#include "AE_API.h"
#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
#include "UI_CMD.h"
#include "UI_UART2.h"
#endif
#endif

uint8_t ubRC_Qp[4][10] = 
{
#if (APP_BLE_SN9380_FUNC_ENABLE == 1)	
	51,51,51,51,51,51,51,51,51,51,
#else
	42,42,42,42,42,42,42,42,42,42,
#endif
	42,42,42,42,42,42,42,42,42,42,
	42,42,42,42,42,42,42,42,42,42,
	42,42,42,42,42,42,42,42,42,42
};

RC_STATUS tRC_SysStatus = RC_STATUS_RDY;

uint8_t ubRC_CurFps = 20;
uint8_t ubRC_IncFlg = 1;

RC_INFO tRC_Info[4];

uint8_t ubRC_FinalMaxQp[4] = {46,46,46,46};
uint8_t ubRC_FinalMinQp[4] = {32,32,32,32};
uint32_t ulRC_EngModeBitRate[4];
uint8_t ubRC_EngModeFps[4];

void RC_EngModeSet(uint8_t ubCodecIdx,uint32_t ulTarBitRate,uint8_t ubFps)
{
	ulRC_EngModeBitRate[ubCodecIdx] = ulTarBitRate;
	ubRC_EngModeFps[ubCodecIdx]		= ubFps;
}

uint32_t ulRC_GetEngModeBitRate(uint8_t ubCodecIdx)
{
	return ulRC_EngModeBitRate[ubCodecIdx];
}

uint8_t ubRC_GetEngModeFps(uint8_t ubCodecIdx)
{
	return ubRC_EngModeFps[ubCodecIdx];
}

uint32_t ulRC_MaxQP = 47;
uint32_t ulRC_MinQP = 32;

uint8_t ubRC_RefreshRate;
uint8_t ubRC_UpdateFlg[4] = {0,0,0,0};

uint32_t ulRC_FinalBitRate[4] = {(50*8192L),(50*8192L),(50*8192L),(50*8192L)};
osThreadId  RC_ThreadId[4] = {NULL,NULL,NULL,NULL};
osThreadId  RC_SysThreadId;

uint8_t ubRC_EnableFlg = 0;

#define RC_MAJORVER    1        //!< Major version = 1
#define RC_MINORVER    23        //!< Minor version = 23
uint16_t uwRC_GetVersion (void)
{
    return ((RC_MAJORVER << 8) + RC_MINORVER);
}

uint32_t ulRC_GetInitBitRate(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ulInitBitRate;
}

uint8_t ubRC_GetTargetQp(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubTargetQp;
}

uint8_t ubRC_GetMinQp(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubMinQp;
}

uint8_t ubRC_GetMaxQp(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubMaxQp;
}

uint8_t ubRC_GetFinalMinQp(uint8_t ubCodecIdx)
{
	return ubRC_FinalMinQp[ubCodecIdx];
}

uint8_t ubRC_GetFinalMaxQp(uint8_t ubCodecIdx)
{
	return ubRC_FinalMaxQp[ubCodecIdx];
}

uint8_t ubRC_GetOpMode(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubMode;
}

#if OP_STA
uint8_t ubRC_GetAvgQp(uint8_t ubCodecIdx)
{
	uint8_t i;
	uint16_t uwTemp;
	
	uwTemp = 0;
	for(i=0;i<10;i++)
	{
		uwTemp += ubRC_Qp[ubCodecIdx][i];
	}
	
	return (uwTemp/10);
}
#endif

uint8_t ubRC_GetFps(void)
{
	return ubRC_CurFps;
}

void RC_FuncSet(RC_INFO *pInfo)
{		
	tRC_Info[pInfo->ubCodecIdx].ubEnableFlg 		= pInfo->ubEnableFlg;	
	tRC_Info[pInfo->ubCodecIdx].ubCodecIdx			= pInfo->ubCodecIdx;	
	tRC_Info[pInfo->ubCodecIdx].ubMode 				= pInfo->ubMode;
	tRC_Info[pInfo->ubCodecIdx].ubMinQp 			= pInfo->ubMinQp;
	tRC_Info[pInfo->ubCodecIdx].ubMaxQp				= pInfo->ubMaxQp;
	tRC_Info[pInfo->ubCodecIdx].ubTargetQp			= pInfo->ubTargetQp;

	tRC_Info[pInfo->ubCodecIdx].ulStepBitRate		= pInfo->ulStepBitRate;	
	
	tRC_Info[pInfo->ubCodecIdx].ubMaxFps	 		= pInfo->ubMaxFps;
	tRC_Info[pInfo->ubCodecIdx].ubMinFps	 		= pInfo->ubMinFps;
	tRC_Info[pInfo->ubCodecIdx].ubStepFps 			= pInfo->ubStepFps;	
	
	tRC_Info[pInfo->ubCodecIdx].ubKeySecRatio 		= pInfo->ubKeySecRatio;
	tRC_Info[pInfo->ubCodecIdx].ubNonKeySecRatio 	= pInfo->ubNonKeySecRatio;
	tRC_Info[pInfo->ubCodecIdx].tAdjSequence 		= pInfo->tAdjSequence;
	tRC_Info[pInfo->ubCodecIdx].ulUpdateRatePerMs 	= pInfo->ulUpdateRatePerMs;
	tRC_Info[pInfo->ubCodecIdx].ubRefreshRate		= pInfo->ubRefreshRate;	
	
	tRC_Info[pInfo->ubCodecIdx].ubQpBuf				= pInfo->ubQpBuf;
	tRC_Info[pInfo->ubCodecIdx].ulBwBuf				= pInfo->ulBwBuf;
	tRC_Info[pInfo->ubCodecIdx].ubFpsBuf			= pInfo->ubFpsBuf;
	
	tRC_Info[pInfo->ubCodecIdx].ulHighQtyTh			= pInfo->ulHighQtyTh;
	tRC_Info[pInfo->ubCodecIdx].ulLowQtyTh			= pInfo->ulLowQtyTh;	
	tRC_Info[pInfo->ubCodecIdx].ubHighBwMaxQp		= pInfo->ubHighBwMaxQp;
	tRC_Info[pInfo->ubCodecIdx].ubHighBwMinQp		= pInfo->ubHighBwMinQp;	
	tRC_Info[pInfo->ubCodecIdx].ubMediumBwMaxQp		= pInfo->ubMediumBwMaxQp;
	tRC_Info[pInfo->ubCodecIdx].ubMediumBwMinQp		= pInfo->ubMediumBwMinQp;	
	tRC_Info[pInfo->ubCodecIdx].ubLowBwMaxQp		= pInfo->ubLowBwMaxQp;
	tRC_Info[pInfo->ubCodecIdx].ubLowBwMinQp		= pInfo->ubLowBwMinQp;
	
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[0]			= pInfo->ulBwTh[0];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[1]			= pInfo->ulBwTh[1];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[2]			= pInfo->ulBwTh[2];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[3]			= pInfo->ulBwTh[3];	
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[4]			= pInfo->ulBwTh[4];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[5]			= pInfo->ulBwTh[5];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[6]			= pInfo->ulBwTh[6];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[7]			= pInfo->ulBwTh[7];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[8]			= pInfo->ulBwTh[8];
	
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[0]		= pInfo->ubTargetFps[0];	
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[1]		= pInfo->ubTargetFps[1];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[2]		= pInfo->ubTargetFps[2];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[3]		= pInfo->ubTargetFps[3];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[4]		= pInfo->ubTargetFps[4];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[5]		= pInfo->ubTargetFps[5];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[6]		= pInfo->ubTargetFps[6];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[7]		= pInfo->ubTargetFps[7];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[8]		= pInfo->ubTargetFps[8];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[9]		= pInfo->ubTargetFps[9];
	
	tRC_Info[pInfo->ubCodecIdx].ubInitFps			= pInfo->ubInitFps;
	tRC_Info[pInfo->ubCodecIdx].ulInitBitRate		= pInfo->ulInitBitRate;
	tRC_Info[pInfo->ubCodecIdx].ubFpsSectionNum		= pInfo->ubFpsSectionNum;
	
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[0]		= pInfo->ubTarMaxQp[0];
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[1]		= pInfo->ubTarMaxQp[1];
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[2]		= pInfo->ubTarMaxQp[2];
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[3]		= pInfo->ubTarMaxQp[3];
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[4]		= pInfo->ubTarMaxQp[4];
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[5]		= pInfo->ubTarMaxQp[5];
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[6]		= pInfo->ubTarMaxQp[6];
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[7]		= pInfo->ubTarMaxQp[7];
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[8]		= pInfo->ubTarMaxQp[8];
	tRC_Info[pInfo->ubCodecIdx].ubTarMaxQp[9]		= pInfo->ubTarMaxQp[9];
	
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[0]		= pInfo->ubTarMinQp[0];
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[1]		= pInfo->ubTarMinQp[1];
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[2]		= pInfo->ubTarMinQp[2];
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[3]		= pInfo->ubTarMinQp[3];
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[4]		= pInfo->ubTarMinQp[4];
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[5]		= pInfo->ubTarMinQp[5];
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[6]		= pInfo->ubTarMinQp[6];
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[7]		= pInfo->ubTarMinQp[7];
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[8]		= pInfo->ubTarMinQp[8];
	tRC_Info[pInfo->ubCodecIdx].ubTarMinQp[9]		= pInfo->ubTarMinQp[9];
}

void ubRC_SetFlg(uint8_t ubCodecIdx, uint8_t ubRcCtrlFlg)
{
	tRC_Info[ubCodecIdx].ubEnableFlg = ubRcCtrlFlg;
}

uint8_t ubRC_GetFlg(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubEnableFlg;
}

uint8_t ubRC_GetRefreshRate(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubRefreshRate;
}

uint8_t ubRC_GetUpdateFlg(uint8_t ubCodecIdx)
{
	return ubRC_UpdateFlg[ubCodecIdx];
}	

void RC_SetUpdateFlg(uint8_t ubCodecIdx,uint8_t ubFlg)
{
	ubRC_UpdateFlg[ubCodecIdx] = ubFlg;
}	

void RC_VariableInit(void)
{
	tRC_Info[0].ubMaxQp = 47;
	tRC_Info[1].ubMaxQp = 47;
	tRC_Info[2].ubMaxQp = 47;
	tRC_Info[3].ubMaxQp = 47;
	
	tRC_Info[0].ubMinQp = 32;
	tRC_Info[1].ubMinQp = 32;
	tRC_Info[2].ubMinQp = 32;
	tRC_Info[3].ubMinQp = 32;
	
	tRC_Info[0].ubTargetQp = 40;
	tRC_Info[1].ubTargetQp = 40;
	tRC_Info[2].ubTargetQp = 40;
	tRC_Info[3].ubTargetQp = 40;
}
void RC_Init(uint8_t ubCodecIdx)
{	
	ubRC_UpdateFlg[ubCodecIdx] = 0;	
	
	ulRC_FinalBitRate[ubCodecIdx] = tRC_Info[ubCodecIdx].ulInitBitRate;	
	ubRC_CurFps = tRC_Info[ubCodecIdx].ubInitFps;
	
//	printf("Init->FBR[%d]:%d KB\r\n",ubCodecIdx,ulRC_FinalBitRate[ubCodecIdx]/8192);
//	printf("Init->FPS[%d]:%d \r\n",ubCodecIdx,ubRC_CurFps);
	
	if(tRC_Info[ubCodecIdx].ubEnableFlg)
	{
		H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMaxQp);
		H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMinQp);
	}	

	if(RC_SysThreadId == NULL)
	{
	#if OP_STA	
		osThreadDef(RcSysThread, RC_SysThread, THREAD_PRIO_RC_SYS_MONIT, 1, THREAD_STACK_RC_SYS_MONIT);
		RC_SysThreadId = osThreadCreate(osThread(RcSysThread), NULL);
	#endif
	}

	if(RC_ThreadId[ubCodecIdx] == NULL)
	{
	#if (OP_STA && !defined(RC_MONITHD_DIS))
		if(ubCodecIdx == 0)
		{
			osThreadDef(RcMonitThread0, RC_MonitThread0, THREAD_PRIO_RC_MONIT, 1, THREAD_STACK_RC_MONIT);
			RC_ThreadId[0] = osThreadCreate(osThread(RcMonitThread0), NULL);
		}
		if(ubCodecIdx == 1)
		{
			osThreadDef(RcMonitThread1, RC_MonitThread1, THREAD_PRIO_RC_MONIT, 1, THREAD_STACK_RC_MONIT);
			RC_ThreadId[1] = osThreadCreate(osThread(RcMonitThread1), NULL);
		}
		if(ubCodecIdx == 2)
		{
			osThreadDef(RcMonitThread2, RC_MonitThread2, THREAD_PRIO_RC_MONIT, 1, THREAD_STACK_RC_MONIT);
			RC_ThreadId[2] = osThreadCreate(osThread(RcMonitThread2), NULL);
		}
		if(ubCodecIdx == 3)
		{
			osThreadDef(RcMonitThread3, RC_MonitThread3, THREAD_PRIO_RC_MONIT, 1, THREAD_STACK_RC_MONIT);
			RC_ThreadId[3] = osThreadCreate(osThread(RcMonitThread3), NULL);
		}
	#endif
	}
	
	osThreadDef(RC_MonitLatencyThread, RC_MonitLatencyThread, THREAD_PRIO_RC_MONIT, 1, THREAD_STACK_RC_MONIT);
	osThreadCreate(osThread(RC_MonitLatencyThread), NULL);	
}

#if OP_STA
void RC_QtyAndFpsProcess1(uint8_t ubCodecIdx)
{
#ifdef A7130
	uint32_t ulAvgCurBw = 0;
	uint32_t ulBw = 0,ulN1Bw = 0,ulN2Bw = 0;
	uint32_t ulCurBw = 0;					//Bit-Rate
	uint32_t ulCurTargetDr = 0;
	uint8_t ubIsKeySecFlg = 0;
	uint8_t ubUpdateFlg = 0;
	uint8_t ubAdjQtyFlg = 0;
	uint8_t ubAdjFpsFlg = 0;
	uint8_t ubAdjDownFlg = 0;

	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)))
	{
		#if 0
		if(ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))
		{
			ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		}
		else
		{
			ulCurBw = 0;
		}
		#else
		ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		#endif

		ulAvgCurBw = ulCurBw;

		ulBw = (ulAvgCurBw)/8192;

		if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_10)
		{
			ulCurTargetDr = ulAvgCurBw/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_20)
		{
			ulCurTargetDr = (ulAvgCurBw*2)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_30)
		{
			ulCurTargetDr = (ulAvgCurBw*3)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_40)
		{
			ulCurTargetDr = (ulAvgCurBw*4)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_50)
		{
			ulCurTargetDr = (ulAvgCurBw*5)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_60)
		{
			ulCurTargetDr = (ulAvgCurBw*6)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_70)
		{
			ulCurTargetDr = (ulAvgCurBw*7)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_80)
		{
			ulCurTargetDr = (ulAvgCurBw*8)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_90)
		{
			ulCurTargetDr = (ulAvgCurBw*9)/10;
		}
		
		ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMaxQp;	
		ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMinQp;
		
		H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMaxQp);
		H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMinQp);	
		
		ulN1Bw = ulCurTargetDr/8192;
		
		//======================================================
		if(ulKNL_GetVdoFrmIdx(ubCodecIdx) <= ubKNL_GetVdoFps())
		{
			ubIsKeySecFlg = 1;			
		}
		else if( (ulKNL_GetVdoGop() > (ubKNL_GetVdoFps()*2))  && (ulKNL_GetVdoFrmIdx(ubCodecIdx) > (ulKNL_GetVdoGop()-(ubKNL_GetVdoFps()*2))) )
		{
			ubIsKeySecFlg = 1;			
		}
		else
		{
			ubIsKeySecFlg = 0;			
		}	

		if(ubIsKeySecFlg)
		{
			//printf("==== KeySec ==== \r\n");
		}
		else
		{			
			if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_10)
			{
				ulCurTargetDr = ulAvgCurBw/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_20)
			{
				ulCurTargetDr = (ulAvgCurBw*2)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_30)
			{
				ulCurTargetDr = (ulAvgCurBw*3)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_40)
			{
				ulCurTargetDr = (ulAvgCurBw*4)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_50)
			{
				ulCurTargetDr = (ulAvgCurBw*5)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_60)
			{
				ulCurTargetDr = (ulAvgCurBw*6)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_70)
			{
				ulCurTargetDr = (ulAvgCurBw*7)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_80)
			{
				ulCurTargetDr = (ulAvgCurBw*8)/10;
			}	
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_90)
			{
				ulCurTargetDr = (ulAvgCurBw*9)/10;
			}		
		}
		//======================================================
		
		//Case Judgement
		//------------------------------------------------------
		if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) >= ulAvgCurBw)
		{
			//printf("(-)F\r\n");
			ubAdjQtyFlg = 0;
			ubAdjFpsFlg = 1;
			ubAdjDownFlg = 1;
		}
		else if((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf >= ulAvgCurBw) && (ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) < ulAvgCurBw))
		{
			//printf("(-)Q\r\n");
			ubAdjQtyFlg = 1;
			ubAdjFpsFlg = 0;		
			ubAdjDownFlg = 1;		
		}
		else if(
		((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf) < ulAvgCurBw) &&
		(ubRC_CurFps != tRC_Info[ubCodecIdx].ubMaxFps))	
		{
			//printf("(+)F\r\n");
			ubAdjQtyFlg = 0;
			ubAdjFpsFlg = 1;		
			ubAdjDownFlg = 0;
		}	
		
		//Adjust Quality
		//================================================
		if(ubAdjDownFlg && ubAdjQtyFlg)
		{
			if(ulCurTargetDr > tRC_Info[ubCodecIdx].ulStepBitRate)
			{
				ulCurTargetDr -= tRC_Info[ubCodecIdx].ulStepBitRate;
			}
			else
			{
				ulCurTargetDr = ulCurTargetDr/2;
			}
		}
		
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr;					
		ulN2Bw = ulCurTargetDr/8192;	

		//Adjust FPS
		if(ubAdjDownFlg && ubAdjFpsFlg)
		{		
			if(ubRC_CurFps >= (tRC_Info[ubCodecIdx].ubMinFps + tRC_Info[ubCodecIdx].ubStepFps))
			{
				ubRC_CurFps = ubRC_CurFps - tRC_Info[ubCodecIdx].ubStepFps;
				ubUpdateFlg = 1;
				//printf("(-)New FPS : %d\r\n",ubRC_CurFps);
				
				SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
				if(ubRC_CurFps <= 5)
				{
					IQ_SetAEPID(1);
				}
				else
				{
					IQ_SetAEPID(0);
				}				
				KNL_SetVdoFps(ubRC_CurFps);	
			}
		}
		else if((!ubAdjDownFlg) && ubAdjFpsFlg)
		{
			if((ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps) <= tRC_Info[ubCodecIdx].ubMaxFps)
			{
				ubRC_CurFps = ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps;
				ubUpdateFlg = 1;
				//printf("(+)New FPS : %d\r\n",ubRC_CurFps);
				
				SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
				if(ubRC_CurFps <= 5)
				{
					IQ_SetAEPID(1);
				}
				else
				{
					IQ_SetAEPID(0);
				}				
				KNL_SetVdoFps(ubRC_CurFps);			
			}
		}

		//======================================================
		if(ubUpdateFlg)
		{
			RC_SetUpdateFlg(ubCodecIdx,1);
		}
		//H264_SetRCParameter(tRC_Info[0].ubCodecIdx,ulCurTargetDr,ubKNL_GetVdoFps());
	}
//	printf("B[%d]:%d_%d_%d_Q->%d,F->%d\r\n",tRC_Info[0].ubCodecIdx,ulBw,ulN1Bw,ulN2Bw,ubRC_GetAvgQp(ubCodecIdx),ubRC_CurFps);		
	printd(DBG_CriticalLvl, "B[%d]:%d_%d_%d_Q->%d,F->%d,V->%d,f->%d\r\n",
		tRC_Info[0].ubCodecIdx,
		ulBw,
		ulN1Bw,
		ulN2Bw,
		ubRC_GetAvgQp(ubCodecIdx),
		ubRC_CurFps,
		ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO, ubCodecIdx)/8192,
		ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_MASTER_AP));
	if(ubRC_GetAvgQp(ubCodecIdx) == 0)
	{
		printf("Case1-3\r\n");
		printf("QP Err1\r\n");
		printf("QP Err2\r\n");
		printf("QP Err3\r\n");
		printf("Restart System !!!\r\n");
		SYS_Reboot();		
	}
#endif
}

void RC_QtyAndFpsProcess2(uint8_t ubCodecIdx)
{
#ifdef A7130
	uint32_t ulAvgCurBw = 0;
	uint32_t ulBw,ulN1Bw = 0, ulN2Bw = 0;
	uint32_t ulCurBw = 0;					//Bit-Rate
	uint32_t ulCurTargetDr = 0;
	uint8_t ubIsKeySecFlg = 0;
	uint8_t ubUpdateFlg = 0;
	uint8_t ubTargetFps = 0;
	
	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)))
	{
		#if 0
		if(ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))
		{
			ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		}
		else
		{
			ulCurBw = 0;
		}
		#else
			ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		#endif
		
		ulAvgCurBw = ulCurBw;
		
		//======================================================
		if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[8])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[9];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[7])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[8];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[6])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[7];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[5])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[6];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[4])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[5];
		}
		
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[3])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[4];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[2])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[3];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[1])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[2];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[0])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[1];
		}
		else
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[0];
		}
		//======================================================
					
		ulBw = (ulAvgCurBw)/8192;	

		if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_10)
		{
			ulCurTargetDr = ulAvgCurBw/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_20)
		{
			ulCurTargetDr = (ulAvgCurBw*2)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_30)
		{
			ulCurTargetDr = (ulAvgCurBw*3)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_40)
		{
			ulCurTargetDr = (ulAvgCurBw*4)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_50)
		{
			ulCurTargetDr = (ulAvgCurBw*5)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_60)
		{
			ulCurTargetDr = (ulAvgCurBw*6)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_70)
		{
			ulCurTargetDr = (ulAvgCurBw*7)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_80)
		{
			ulCurTargetDr = (ulAvgCurBw*8)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_90)
		{
			ulCurTargetDr = (ulAvgCurBw*9)/10;
		}
		
		ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMaxQp;	
		ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMinQp;
		
		H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMaxQp);
		H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMinQp);	
		
		ulN1Bw = ulCurTargetDr/8192;
		
		//======================================================
		if(ulKNL_GetVdoFrmIdx(ubCodecIdx) <= ubKNL_GetVdoFps())
		{
			ubIsKeySecFlg = 1;			
		}
		else if( (ulKNL_GetVdoGop() > (ubKNL_GetVdoFps()*2))  && (ulKNL_GetVdoFrmIdx(ubCodecIdx) > (ulKNL_GetVdoGop()-(ubKNL_GetVdoFps()*2))) )
		{
			ubIsKeySecFlg = 1;			
		}
		else
		{
			ubIsKeySecFlg = 0;			
		}	

		if(ubIsKeySecFlg)
		{
			//printf("==== KeySec ==== \r\n");
		}
		else
		{			
			if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_10)
			{
				ulCurTargetDr = ulAvgCurBw/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_20)
			{
				ulCurTargetDr = (ulAvgCurBw*2)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_30)
			{
				ulCurTargetDr = (ulAvgCurBw*3)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_40)
			{
				ulCurTargetDr = (ulAvgCurBw*4)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_50)
			{
				ulCurTargetDr = (ulAvgCurBw*5)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_60)
			{
				ulCurTargetDr = (ulAvgCurBw*6)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_70)
			{
				ulCurTargetDr = (ulAvgCurBw*7)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_80)
			{
				ulCurTargetDr = (ulAvgCurBw*8)/10;
			}	
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_90)
			{
				ulCurTargetDr = (ulAvgCurBw*9)/10;
			}			
		}
		//======================================================
		
		//Adjust Quality
		//------------------------------------------------------
		if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf >= ulAvgCurBw)
		{
			//printf("(-)QTY\r\n");
			if(ulCurTargetDr > tRC_Info[ubCodecIdx].ulStepBitRate)
			{
				ulCurTargetDr -= tRC_Info[ubCodecIdx].ulStepBitRate;
			}		
		}
		
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr;					
		ulN2Bw = ulCurTargetDr/8192;	

		//Adjust FPS
		//------------------------------------------------------	
		if((ubRC_GetAvgQp(ubCodecIdx) <= (tRC_Info[ubCodecIdx].ubTargetQp + tRC_Info[ubCodecIdx].ubQpBuf)) &&
		  ((ubRC_GetAvgQp(ubCodecIdx)+tRC_Info[ubCodecIdx].ubQpBuf) >= tRC_Info[ubCodecIdx].ubTargetQp ))
		{
			if(ubRC_CurFps != ubTargetFps)
			{
				if((ubRC_CurFps > ubTargetFps) && (ubRC_CurFps > tRC_Info[ubCodecIdx].ubStepFps))
				{
					ubRC_CurFps = ubRC_CurFps - tRC_Info[ubCodecIdx].ubStepFps;
					ubUpdateFlg = 1;
					//printf("(1-)New FPS : %d\r\n",ubRC_CurFps);
					
					SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
					if(ubRC_CurFps <= 5)
					{
						IQ_SetAEPID(1);
					}
					else
					{
						IQ_SetAEPID(0);
					}				
					KNL_SetVdoFps(ubRC_CurFps);			
				}
				else if((ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps) <= (ubTargetFps+tRC_Info[ubCodecIdx].ubFpsBuf))
				{
					ubRC_CurFps = ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps;
					ubUpdateFlg = 1;
					//printf("(1+)New FPS : %d\r\n",ubRC_CurFps);
					
					SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
					if(ubRC_CurFps <= 5)
					{
						IQ_SetAEPID(1);
					}
					else
					{
						IQ_SetAEPID(0);
					}
					KNL_SetVdoFps(ubRC_CurFps);			
				}
			}
		}	
		else if(ubRC_GetAvgQp(ubCodecIdx) > (tRC_Info[ubCodecIdx].ubTargetQp + tRC_Info[ubCodecIdx].ubQpBuf))
		{			
			if((ubRC_CurFps + tRC_Info[ubCodecIdx].ubFpsBuf) > ubTargetFps)
			{
				if((ubRC_CurFps > tRC_Info[ubCodecIdx].ubStepFps) && ((ubRC_CurFps + tRC_Info[ubCodecIdx].ubFpsBuf ) >= (ubTargetFps + tRC_Info[ubCodecIdx].ubStepFps ) ))	
				{
					ubRC_CurFps = ubRC_CurFps - tRC_Info[ubCodecIdx].ubStepFps;
					
					if(ubRC_CurFps <= tRC_Info[ubCodecIdx].ubMinFps)
					{
						ubRC_CurFps = tRC_Info[ubCodecIdx].ubMinFps;
					}
				
					ubUpdateFlg = 1;
					//printf("(2-)New FPS : %d\r\n",ubRC_CurFps);
					
					SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
					if(ubRC_CurFps <= 5)
					{
						IQ_SetAEPID(1);
					}
					else
					{
						IQ_SetAEPID(0);
					}
					KNL_SetVdoFps(ubRC_CurFps);	
				}
			}
		}
		else
		{		
			if((ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps) <= (ubTargetFps+tRC_Info[ubCodecIdx].ubFpsBuf))
			{
				ubRC_CurFps = ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps;
				
				if(ubRC_CurFps >= tRC_Info[ubCodecIdx].ubMaxFps)
				{
					ubRC_CurFps = tRC_Info[ubCodecIdx].ubMaxFps;
				}
				ubUpdateFlg = 1;
				//printf("(2+)New FPS : %d\r\n",ubRC_CurFps);
				
				SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);			
				if(ubRC_CurFps <= 5)
				{
					IQ_SetAEPID(1);
				}
				else
				{
					IQ_SetAEPID(0);
				}
				KNL_SetVdoFps(ubRC_CurFps);	
			}
		}

		//======================================================
		if(ubUpdateFlg)
		{
			RC_SetUpdateFlg(ubCodecIdx,1);
		}
		//H264_SetRCParameter(tRC_Info[0].ubCodecIdx,ulCurTargetDr,ubKNL_GetVdoFps());
	}
//	printf("B[%d]:%d_%d_%d_Q->%d,F->%d\r\n",tRC_Info[0].ubCodecIdx,ulBw,ulN1Bw,ulN2Bw,ubRC_GetAvgQp(ubCodecIdx),ubRC_CurFps);		
	printd(DBG_CriticalLvl, "B[%d]:%d_%d_%d_Q->%d,F->%d,V->%d,f->%d\r\n",
		tRC_Info[0].ubCodecIdx,
		ulBw,
		ulN1Bw,
		ulN2Bw,
		ubRC_GetAvgQp(ubCodecIdx),
		ubRC_CurFps,
		ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO, ubCodecIdx)/8192,
		ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_MASTER_AP));
	if(ubRC_GetAvgQp(ubCodecIdx) == 0)
	{
		printf("Case1-4\r\n");
		printf("QP Err1\r\n");
		printf("QP Err2\r\n");
		printf("QP Err3\r\n");
		printf("Restart System !!!\r\n");
		SYS_Reboot();		
	}
#endif
}

//UNIT -> "Bit" Rate
void RC_QtyAndBwProcess(uint8_t ubCodecIdx)
{
#if (defined(A7130) || defined(RTC676x))
	uint32_t ulAvgCurBw = 0;
	uint32_t ulBw = 0, ulN1Bw = 0, ulN2Bw = 0;
	uint32_t ulCurBw = 0;					//Bit-Rate
	uint32_t ulCurTargetDr = 0;
	uint8_t ubIsKeySecFlg = 0;

	ulBw = ulBw;
	ulN1Bw = ulN1Bw;
	ulN2Bw = ulN2Bw;

	#if A7130
	//uint8_t index;
	//static uint8_t HighBW=1;
	#endif

	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)))
	{
	#if A7130
		#if 0
		if(ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))
		{
			ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		}
		else
		{
			ulCurBw = 0;
		}
		#else
		ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		#endif
	#endif 

	#if RTC676x						
		ulCurBw = ulRTC676x_GetRtBw(0)*8;		
	#endif

		ulAvgCurBw = ulCurBw;	

		ulBw = (ulAvgCurBw)/8192;

		if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_10)
		{
			ulCurTargetDr = ulAvgCurBw/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_20)
		{
			ulCurTargetDr = (ulAvgCurBw*2)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_30)
		{
			ulCurTargetDr = (ulAvgCurBw*3)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_40)
		{
			ulCurTargetDr = (ulAvgCurBw*4)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_50)
		{
			ulCurTargetDr = (ulAvgCurBw*5)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_60)
		{
			ulCurTargetDr = (ulAvgCurBw*6)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_70)
		{
			ulCurTargetDr = (ulAvgCurBw*7)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_75)
		{
			ulCurTargetDr = (ulAvgCurBw*75)/100;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_80)
		{
			ulCurTargetDr = (ulAvgCurBw*8)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_85)
		{
			ulCurTargetDr = (ulAvgCurBw*85)/100;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_90)
		{
			ulCurTargetDr = (ulAvgCurBw*9)/10;
		}
		
		//Get New Setting		
		//=============================================================	
		tRC_Info[ubCodecIdx].ulHighQtyTh 	= (ulTRX_GetMaxBw(0)*8*2)/3;
		tRC_Info[ubCodecIdx].ulLowQtyTh		= (ulTRX_GetMaxBw(0)*8*1)/3;
		
		if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulHighQtyTh)
		{				
			if((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf) <=  ulAvgCurBw)
			{
				//printf("H-L\r\n");
				ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMaxQp;
				ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMinQp;
				
				//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMaxQp);
				//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMinQp);
			}
			else if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) >  ulAvgCurBw)	//Heavy Case
			{
				//printf("H-H\r\n");
				if((tRC_Info[ubCodecIdx].ubHighBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubHighBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
			else //Light Case
			{				
				//printf("H-M\r\n");
				
				if((tRC_Info[ubCodecIdx].ubHighBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubHighBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
		}		
		else if(ulAvgCurBw <= tRC_Info[ubCodecIdx].ulLowQtyTh)
		{
			if((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf) <=  ulAvgCurBw)
			{
				//printf("L-L\r\n");
				ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMaxQp;
				ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMinQp;
				
				//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMaxQp);
				//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMinQp);	
			}
			else if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) > ulAvgCurBw)	//Heavy Case
			{
				//printf("L-H\r\n");
				if((tRC_Info[ubCodecIdx].ubLowBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubLowBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
			else	//Light Case
			{
				//printf("L-M\r\n");
				if((tRC_Info[ubCodecIdx].ubLowBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubLowBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
		}
		else
		{
			if((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf) <=  ulAvgCurBw)
			{
				//printf("M-L\r\n");
				ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMaxQp;
				ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMinQp;
				
				//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMaxQp);
				//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMinQp);
			}
			else if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) > ulAvgCurBw) //Heavy Case
			{
				//printf("M-H\r\n");
				if((tRC_Info[ubCodecIdx].ubMediumBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubMediumBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
			else //Light Case
			{
				//printf("M-M\r\n");
				if((tRC_Info[ubCodecIdx].ubMediumBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					//H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubMediumBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					//H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}		
		}			
		//===========================================================
		
		ulN1Bw = ulCurTargetDr/8192;
		
		//======================================================
		//if(ulKNL_GetVdoFrmIdx(ubCodecIdx) <= ubKNL_GetVdoFps())
		if(ulKNL_GetVdoFrmIdx(ubCodecIdx) <= (ubKNL_GetVdoFps()*2))
		{
			ubIsKeySecFlg = 1;			
		}
		else if( (ulKNL_GetVdoGop() > (ubKNL_GetVdoFps()*2))  && (ulKNL_GetVdoFrmIdx(ubCodecIdx) > (ulKNL_GetVdoGop()-(ubKNL_GetVdoFps()*2))) )
		{
			ubIsKeySecFlg = 1;			
		}
		else
		{
			ubIsKeySecFlg = 0;			
		}	

		if(ubIsKeySecFlg)
		{
			//printf("==== KeySec ==== \r\n");
		}
		else
		{			
			if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_10)
			{
				ulCurTargetDr = ulAvgCurBw/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_20)
			{
				ulCurTargetDr = (ulAvgCurBw*2)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_30)
			{
				ulCurTargetDr = (ulAvgCurBw*3)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_40)
			{
				ulCurTargetDr = (ulAvgCurBw*4)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_50)
			{
				ulCurTargetDr = (ulAvgCurBw*5)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_60)
			{
				ulCurTargetDr = (ulAvgCurBw*6)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_70)
			{
				ulCurTargetDr = (ulAvgCurBw*7)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_75)
			{
				ulCurTargetDr = (ulAvgCurBw*75)/100;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_80)
			{
				ulCurTargetDr = (ulAvgCurBw*8)/10;
			}	
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_85)
			{
				ulCurTargetDr = (ulAvgCurBw*85)/100;
			}	
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_90)
			{
				ulCurTargetDr = (ulAvgCurBw*9)/10;
			}		
		}
		//======================================================
		
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr;
		ulN2Bw = ulCurTargetDr/8192;
	}
	else
	{
		ubRC_FinalMaxQp[ubCodecIdx]   = 51;
		ubRC_FinalMinQp[ubCodecIdx]   = 30;
		ulRC_FinalBitRate[ubCodecIdx] = 20*1024*8L;
	}
	//RC_SetUpdateFlg(ubCodecIdx,1);	
	//H264_SetRCParameter(tRC_Info[0].ubCodecIdx,ulCurTargetDr,ubKNL_GetVdoFps());	
//	printf("B[%d]:%d_%d_%d_Q->%d,F->%d\r\n",tRC_Info[0].ubCodecIdx,ulBw,ulN1Bw,ulN2Bw,ubRC_GetAvgQp(ubCodecIdx),ubRC_CurFps);
	#if A7130
	printd(DBG_CriticalLvl, "B[%d]:%d_%d_%d_Q->%d,F->%d,V->%d,f->%d\r\n",
		tRC_Info[0].ubCodecIdx,
		ulBw,
		ulN1Bw,
		ulN2Bw,
		ubRC_GetAvgQp(ubCodecIdx),
		ubRC_CurFps,
		ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO, ubCodecIdx)/8192,
		ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_MASTER_AP));
	#endif
	if(ubRC_GetAvgQp(ubCodecIdx) == 0)
	{
		printf("Case1-1\r\n");
		printf("QP Err1\r\n");
		printf("QP Err2\r\n");
		printf("QP Err3\r\n");
		printf("Restart System !!!\r\n");
		SYS_Reboot();	
	}
#else
	ubRC_FinalMaxQp[ubCodecIdx]   = 51;
	ubRC_FinalMinQp[ubCodecIdx]   = 30;
	ulRC_FinalBitRate[ubCodecIdx] = 20*1024*8L;
#endif
}

void RC_EnterSetupMode(void)
{
	tRC_SysStatus = RC_STATUS_SETUP;
}

void RC_ExitSetupMode(void)
{
	tRC_SysStatus = RC_STATUS_RDY;
}

RC_STATUS tRC_GetSysStatus(void)
{
	return tRC_SysStatus;
}

//VBM+BUC
void RC_DynamicFpsProcess(uint8_t ubCodecIdx)
{
#if (defined(A7130) || defined(RTC676x))
#if RTC676x	
	uint8_t ubBufNumVdoPacketize;
#endif
	
#if A7130
	BB_GET_DATA_FLOW tDataFlow;
#endif
	uint8_t i;

	uint8_t ubPrFps = 0;
	uint8_t ubNewFps = 0;
	
	uint32_t ulAvgCurBw = 0;
	uint32_t ulBw = 0, ulN1Bw = 0, ulN2Bw = 0;
	uint32_t ulCurBw = 0;					//Bit-Rate
	uint32_t ulCurTargetDr = 0;
	uint8_t ubIsKeySecFlg = 0;

	ulBw = ulBw;
	ulN1Bw = ulN1Bw;
	ulN2Bw = ulN2Bw;	

	uint8_t ubLvlIdx = 0;
	
	ubPrFps = ubRC_CurFps;	
	
	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)) && (tRC_GetSysStatus()==RC_STATUS_RDY))
	{
		ulCurBw = ulTRX_GetRtBw(0)*8;	
		
		ulAvgCurBw = ulCurBw;	

		ulBw = (ulAvgCurBw)/8192;

		if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_10)
		{
			ulCurTargetDr = ulAvgCurBw/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_20)
		{
			ulCurTargetDr = (ulAvgCurBw*2)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_30)
		{
			ulCurTargetDr = (ulAvgCurBw*3)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_40)
		{
			ulCurTargetDr = (ulAvgCurBw*4)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_50)
		{
			ulCurTargetDr = (ulAvgCurBw*5)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_60)
		{
			ulCurTargetDr = (ulAvgCurBw*6)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_70)
		{
			ulCurTargetDr = (ulAvgCurBw*7)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_75)
		{
			ulCurTargetDr = (ulAvgCurBw*75)/100;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_80)
		{
			ulCurTargetDr = (ulAvgCurBw*8)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_90)
		{
			ulCurTargetDr = (ulAvgCurBw*9)/10;
		}
		
		//QP Setting
//		ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMaxQp;
//		ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMinQp;	
		
//		H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMaxQp);
//		H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMinQp);		
		
		//Target Bit-Rate Setting
		//======================================================
		if(ulKNL_GetVdoFrmIdx(ubCodecIdx) <= ubKNL_GetVdoFps())
		{
			ubIsKeySecFlg = 1;			
		}
		else if( (ulKNL_GetVdoGop() > (ubKNL_GetVdoFps()*2))  && (ulKNL_GetVdoFrmIdx(ubCodecIdx) > (ulKNL_GetVdoGop()-(ubKNL_GetVdoFps()*2))) )
		{
			ubIsKeySecFlg = 1;			
		}
		else
		{
			ubIsKeySecFlg = 0;			
		}	

		if(ubIsKeySecFlg)
		{
			//printf("==== KeySec ==== \r\n");
		}
		else
		{			
			if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_10)
			{
				ulCurTargetDr = ulAvgCurBw/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_20)
			{
				ulCurTargetDr = (ulAvgCurBw*2)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_30)
			{
				ulCurTargetDr = (ulAvgCurBw*3)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_40)
			{
				ulCurTargetDr = (ulAvgCurBw*4)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_50)
			{
				ulCurTargetDr = (ulAvgCurBw*5)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_60)
			{
				ulCurTargetDr = (ulAvgCurBw*6)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_70)
			{
				ulCurTargetDr = (ulAvgCurBw*7)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_75)
			{
				ulCurTargetDr = (ulAvgCurBw*75)/100;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_80)
			{
				ulCurTargetDr = (ulAvgCurBw*8)/10;
			}	
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_90)
			{
				ulCurTargetDr = (ulAvgCurBw*9)/10;
			}		
		}
		//======================================================			
		
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr;
		
		ubLvlIdx = 0xFF;
		//FPS Information
		//for(i=0;i<tRC_Info[ubCodecIdx].ubFpsSectionNum;i++)
		for(i=0;i<tRC_Info[ubCodecIdx].ubFpsSectionNum-1;i++)
		{
			//if(ulCurBw >= tRC_Info[ubCodecIdx].ulBwTh[tRC_Info[ubCodecIdx].ubFpsSectionNum-1-i])
			if(ulCurBw >= tRC_Info[ubCodecIdx].ulBwTh[tRC_Info[ubCodecIdx].ubFpsSectionNum-2-i])
			{				
				ubLvlIdx = tRC_Info[ubCodecIdx].ubFpsSectionNum-1-i;
				break;
			}
		}		
		
		
		if(ubLvlIdx <= (tRC_Info[ubCodecIdx].ubFpsSectionNum-1))
		{					
		#if RTC676x
			if(ubKNL_GetAdjBufEnable())
				ubBufNumVdoPacketize = BUF_NUM_VDO_PACKETIZE_ADJ;
			else
				ubBufNumVdoPacketize = BUF_NUM_VDO_PACKETIZE;
			
			if(ubKNL_GetVdoPacketizeQueNum() <= (ubBufNumVdoPacketize/5))
			{				
				KNL_SetVdoFps(tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				//printf("F1:%d\r\n",tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				
				ubRC_CurFps = tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]; 
			}
			else if(ubKNL_GetVdoPacketizeQueNum() <= (ubBufNumVdoPacketize/3))
			{				
				KNL_SetVdoFps(tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				printf("F12:%d\r\n",tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				
				ubRC_CurFps = tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]; 
				
				ulRC_FinalBitRate[ubCodecIdx] = ulRC_FinalBitRate[ubCodecIdx]*0.75;
			}
		#endif
		#if A7130			
			if(ubBB_GetTxUsedBufNum(BB_DATA_VIDEO,BB_TX_MASTER) <= (ubBB_GetTxTotalBufNum(BB_DATA_VIDEO,BB_TX_MASTER)/5))
			{
				KNL_SetVdoFps(tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				//printf("F1:%d\r\n",tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				
				ubRC_CurFps = tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx];
			}
			else if(ubBB_GetTxUsedBufNum(BB_DATA_VIDEO,BB_TX_MASTER) <= (ubBB_GetTxTotalBufNum(BB_DATA_VIDEO,BB_TX_MASTER)/3))
			{
				KNL_SetVdoFps(tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				//printf("F1:%d\r\n",tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
				
				ubRC_CurFps = tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx];
				
				ulRC_FinalBitRate[ubCodecIdx] = ulRC_FinalBitRate[ubCodecIdx]*0.75;
			}
		#endif
			else
			{				
				if(tRC_GetSysStatus() == RC_STATUS_RDY)
				{
					if(tRC_Info[ubCodecIdx].ubFpsSectionNum != 1)
					{
						if((tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]/2) >= tRC_Info[ubCodecIdx].ubTargetFps[0])
						{
							KNL_SetVdoFps(tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]/2);
							SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]/2);
							//printf("F21:%d\r\n",tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]/2);
							
							ubRC_CurFps = tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]/2;
							
							ulRC_FinalBitRate[ubCodecIdx] = ulRC_FinalBitRate[ubCodecIdx]*0.5;
						}
						else
						{
							KNL_SetVdoFps(tRC_Info[ubCodecIdx].ubTargetFps[0]);
							SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[ubCodecIdx].ubTargetFps[0]);
							//printf("F22:%d\r\n",tRC_Info[ubCodecIdx].ubTargetFps[0]);
							
							ubRC_CurFps = tRC_Info[ubCodecIdx].ubTargetFps[0];
							
							ulRC_FinalBitRate[ubCodecIdx] = ulRC_FinalBitRate[ubCodecIdx]*0.5;
						}
					}
					else
					{
						KNL_SetVdoFps(tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]/2);
						SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]/2);
						//printf("F23:%d-%d\r\n",tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]/2,ubKNL_GetVdoPacketizeQueNum());
							
						ubRC_CurFps = tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]/2;
						
						ulRC_FinalBitRate[ubCodecIdx] = ulRC_FinalBitRate[ubCodecIdx]*0.5;
					}
				}
			}			
			
			ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubTarMaxQp[ubLvlIdx];
			ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubTarMinQp[ubLvlIdx];		
//			H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubTarMaxQp[ubLvlIdx]);
//			H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubTarMinQp[ubLvlIdx]);			
		}
		else
		{
			ubLvlIdx = 0;			
			
			KNL_SetVdoFps(tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
			SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
			//printf("F:%d\r\n",tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx]);
			
			ubRC_CurFps = tRC_Info[ubCodecIdx].ubTargetFps[ubLvlIdx];
			
			ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubTarMaxQp[ubLvlIdx];
			ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubTarMinQp[ubLvlIdx];		
//			H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubTarMaxQp[ubLvlIdx]);
//			H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubTarMinQp[ubLvlIdx]);	
		}
		
	}
	else
	{	
		ubRC_FinalMaxQp[ubCodecIdx] = 51;
		ubRC_FinalMinQp[ubCodecIdx] = 30;
		
//		H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
//		H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,30);
		
		ulRC_FinalBitRate[ubCodecIdx] = 20*1024*8L;
	}
	
	ubNewFps = ubRC_CurFps;
	if(ubPrFps != ubNewFps)
		RC_SetUpdateFlg(ubCodecIdx,1);		
	
	//printf("(L,F,M,m:%d,%d,%d,%d)\r\n",ubLvlIdx,ubRC_CurFps,tRC_Info[ubCodecIdx].ubTarMaxQp[ubLvlIdx],tRC_Info[ubCodecIdx].ubTarMinQp[ubLvlIdx]);
	
	
	//printf("Tar:%d KB\r\n",ulRC_FinalBitRate[ubCodecIdx]/8192);
	
	//printf("B:%d,%d,%d,%d\r\n",ulCurBw/8192,ubBUF_GetVdoMainBs0UsedNum(),ubBB_GetTxTotalBufNum(BB_DATA_VIDEO,BB_TX_MASTER),ubBB_GetTxUsedBufNum(BB_DATA_VIDEO,BB_TX_MASTER));

#if A7130	
	if(ubKNL_GetTRXSlotNum() == 1)
	{
		tDataFlow = BB_GET_DEF_1T1R_STA1_VDO_FLOW;
	}
	else if(ubKNL_GetTRXSlotNum() == 2)
	{
		tDataFlow = BB_GET_DEF_2T1R_STA1_VDO_FLOW;
	}
	else if(ubKNL_GetTRXSlotNum() == 3)
	{
		tDataFlow = BB_GET_DEF_3T1R_STA1_VDO_FLOW;
	}
	else if(ubKNL_GetTRXSlotNum() == 4)
	{
		tDataFlow = BB_GET_DEF_4T1R_STA1_VDO_FLOW;
	}
	
	//printf("(%d,%d)\r\n",ubRC_FinalMaxQp[ubCodecIdx],ubRC_FinalMinQp[ubCodecIdx]);
	printf("B:%d,%d,%d,%d,%d\r\n",ulBB_GetBBFlow(tDataFlow)/1028,ulCurBw/8192,ubRC_GetAvgQp(ubCodecIdx),ubBB_GetTxTotalBufNum(BB_DATA_VIDEO,BB_TX_MASTER),ubBB_GetTxUsedBufNum(BB_DATA_VIDEO,BB_TX_MASTER));		
#endif
	if(ubRC_GetAvgQp(ubCodecIdx) == 0)
	{
		printf("Case1-2\r\n");
		printf("QP Err1\r\n");
		printf("QP Err2\r\n");
		printf("QP Err3\r\n");
		printf("Restart System !!!\r\n");
		SYS_Reboot();
	}
#else
	ubRC_FinalMaxQp[ubCodecIdx]   = 51;
	ubRC_FinalMinQp[ubCodecIdx]   = 30;
	ulRC_FinalBitRate[ubCodecIdx] = 20*1024*8L;
#endif
}

//------------------------------------------------------------------------------
#if (defined(S2019A) && defined(OP_STA))
#define DYNA_FPS_RC_LVL		6
typedef struct
{
	uint8_t ubBufNum[DYNA_FPS_RC_LVL];
	uint8_t ubTargetFps[DYNA_FPS_RC_LVL];
}RC_TxFps_t;
RC_TxFps_t tRC_sPRfTxFps[] = {
								//!       Buffer				       FPS
								{{25, 20, 16, 16, 11, 11},  {25, 20, 15, 15, 10, 10}},		//!< Rate mode = sPRF_NORMALMD_RATE
								{{25, 20, 16, 16, 11, 11},  {15, 13, 11, 10, 10, 10}},		//!< Rate mode = sPRF_LOWMD_RATE
						     };
typedef struct
{
	uint8_t  ubPer;
	uint8_t  ubMaxQp;
	uint8_t  ubMinQp[5];				//! 0:1T, 1:2T, 2:4T
	uint32_t ulTargetBitRate[3];		//! 0:1T, 1:2T, 2:4T
	RC_RATIO tRcRatio[5];				//! 0:1T, 1:2T, 2:4T
	uint8_t  ubBufNum[3];				//! 0:1T, 1:2T, 2:4T
}RC_TxBps_t;
RC_TxBps_t tRC_sPRfTxBps[] = {//!					                    1T             2T             4T
								{20,  40,  {23, 25, 25, 35, 25},  {(520*1024*8L), (260*1024*8L), (220*1024*8L)},  {RC_RATIO_70, RC_RATIO_60, RC_RATIO_60, RC_RATIO_40, RC_RATIO_30},  {25, 25, 25}},
								{30,  42,  {30, 32, 32, 38, 30},  {(440*1024*8L), (220*1024*8L), (180*1024*8L)},  {RC_RATIO_60, RC_RATIO_60, RC_RATIO_60, RC_RATIO_40, RC_RATIO_30},  {22, 22, 22}},
								{45,  42,  {35, 35, 35, 41, 35},  {(200*1024*8L), (160*1024*8L), (150*1024*8L)},  {RC_RATIO_60, RC_RATIO_50, RC_RATIO_60, RC_RATIO_40, RC_RATIO_30},  {19, 19, 19}},
								{60,  42,  {35, 35, 35, 43, 40},  {(200*1024*8L), (120*1024*8L), (120*1024*8L)},  {RC_RATIO_60, RC_RATIO_50, RC_RATIO_50, RC_RATIO_40, RC_RATIO_30},  {16, 16, 16}},
								{75,  45,  {35, 38, 38, 45, 43},  {(150*1024*8L), (100*1024*8L), (100*1024*8L)},  {RC_RATIO_60, RC_RATIO_50, RC_RATIO_50, RC_RATIO_40, RC_RATIO_30},  {13, 13, 13}},
								{100, 45,  {35, 40, 45, 45, 45},  {(130*1024*8L), ( 80*1024*8L), ( 80*1024*8L)},  {RC_RATIO_60, RC_RATIO_50, RC_RATIO_50, RC_RATIO_40, RC_RATIO_30},  {10, 10, 10}},
							 };
static uint8_t ubRC_TxTarget = 2;
void RC_UpdateDynaFpsParam(uint8_t ubMaxFps, uint8_t ubMinFps)
{
	uint8_t ubLvl = 0;

	ubRC_CurFps = ubMaxFps - 3;
	tRC_sPRfTxFps[0].ubTargetFps[0] = ubMaxFps;
	tRC_sPRfTxFps[0].ubTargetFps[1] = ubMaxFps - 5;
	tRC_sPRfTxFps[0].ubTargetFps[2] = (tRC_sPRfTxFps[0].ubTargetFps[1] < 15)?tRC_sPRfTxFps[0].ubTargetFps[1]:(tRC_sPRfTxFps[0].ubTargetFps[1] - 5);
	tRC_sPRfTxFps[0].ubTargetFps[DYNA_FPS_RC_LVL-1] = ubMinFps;
	for(ubLvl = 3; ubLvl < (DYNA_FPS_RC_LVL - 1); ubLvl++)
		tRC_sPRfTxFps[0].ubTargetFps[ubLvl] = ubMinFps;
	tRC_sPRfTxFps[1].ubTargetFps[0] = (ubMaxFps < 15)?ubMaxFps:15;
	tRC_sPRfTxFps[1].ubTargetFps[1] = ubMaxFps - 5;
	for(ubLvl = 2; ubLvl < (DYNA_FPS_RC_LVL - 2); ubLvl++)
		tRC_sPRfTxFps[1].ubTargetFps[ubLvl] = (tRC_sPRfTxFps[1].ubTargetFps[1] < 10)?tRC_sPRfTxFps[1].ubTargetFps[1]:10;
	tRC_sPRfTxFps[1].ubTargetFps[DYNA_FPS_RC_LVL-1] = 5;
}
void RC_UpdateTrxBufParam(uint8_t ubToltalBufNum)
{
	uint8_t ubBpsLvlCnt = 6, ubLvl, ubTxNum;
	uint8_t ubMaxBufNum = 30, ubMinBufNum = 10, ubBufNumIntv;

	ubBpsLvlCnt  = sizeof tRC_sPRfTxBps / sizeof(RC_TxBps_t);
	ubMaxBufNum  = ubToltalBufNum - 6;
	ubMinBufNum  = ubToltalBufNum * 0.4;
	ubBufNumIntv = (uint8_t)(((float)((ubMaxBufNum - ubMinBufNum)))/(ubBpsLvlCnt - 1) + (float)0.5);
	for(ubTxNum = 0; ubTxNum < 3; ubTxNum++)
	{
		for(ubLvl = 0; ubLvl < 2; ubLvl++)
			tRC_sPRfTxBps[ubLvl].ubBufNum[ubTxNum] = ubMaxBufNum - ubLvl;
		for(;ubLvl < ubBpsLvlCnt; ubLvl++)
			tRC_sPRfTxBps[ubLvl].ubBufNum[ubTxNum] = tRC_sPRfTxBps[ubLvl-1].ubBufNum[ubTxNum] - ubBufNumIntv;
	}
	ubMaxBufNum  = ubToltalBufNum - 8;
	ubMinBufNum  = ubToltalBufNum * 0.4;
	ubBufNumIntv = (uint8_t)(((float)((ubMaxBufNum - ubMinBufNum)))/(DYNA_FPS_RC_LVL - 1) + (float)0.5);
	for(ubLvl = 0; ubLvl < 2; ubLvl++)
	{
		tRC_sPRfTxFps[0].ubBufNum[ubLvl] = ubMaxBufNum - ubLvl;
		tRC_sPRfTxFps[1].ubBufNum[ubLvl] = ubMaxBufNum - ubLvl;
	}
	for(;ubLvl < DYNA_FPS_RC_LVL; ubLvl++)
	{
		tRC_sPRfTxFps[0].ubBufNum[ubLvl] = tRC_sPRfTxFps[0].ubBufNum[ubLvl-1] - ubBufNumIntv;
		tRC_sPRfTxFps[1].ubBufNum[ubLvl] = tRC_sPRfTxFps[1].ubBufNum[ubLvl-1] - ubBufNumIntv;
	}
}
uint8_t ubRC_ChkAdjQpRange(uint8_t ubCodecIdx)
{
	sPRF_DevId_t tTxId;
	uint8_t ubBpsLvlCnt = sizeof tRC_sPRfTxBps / sizeof(RC_TxBps_t), ubRC_TxNum = 1;
	uint8_t ubRC_FreeBufNum = 0, ubBufTarget = 3, ubRC_TxMdSel = 0, ubLvl = 0;
	uint8_t ubUpdFlag = FALSE;
	sPRF_RateMd_t tRC_TxRateMd = sPRF_NORMALMD_RATE;
	uint8_t ubTargetFps = 15, ubTxRateMd = 0;

	tTxId = tsPRF_GetDevId();
	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)) &&
	   (sPRF_DEVDRV_EN == tsPRF_GetDrvSts(tTxId)) && (sPRF_TRX_MODE == tsPRF_GetDrvMode()))
	{
		ubRC_TxNum   = ubsPRF_GetAttachedDevNums();
		ubRC_TxMdSel = (ubRC_TxNum < 1)?1:(ubRC_TxNum >= 3)?3:ubRC_TxNum;
		ubRC_FreeBufNum = ubKNL_GetSPRFDataBufNum(tTxId);
		if(ubRC_FreeBufNum >= tRC_sPRfTxBps[0].ubBufNum[ubRC_TxMdSel-1])
			return FALSE;
		tRC_TxRateMd = tsPRF_GetRateLvl();
		ubTxRateMd  = (sPRF_NORMALMD_RATE == (tRC_TxRateMd&0x1))?0:1;
		ubTargetFps = tRC_sPRfTxFps[ubTxRateMd].ubTargetFps[(DYNA_FPS_RC_LVL-1)];
		ubBufTarget = ubBpsLvlCnt - 1;
		for(ubLvl = 0; ubLvl < ubBpsLvlCnt; ubLvl++)
		{
			if(ubRC_FreeBufNum > tRC_sPRfTxBps[ubLvl].ubBufNum[ubRC_TxMdSel-1])
			{
				ubBufTarget = ubLvl;
				break;
			}
		}
		ubTargetFps = tRC_sPRfTxFps[ubTxRateMd].ubTargetFps[ubBufTarget];
		ubUpdFlag   = (ubBufTarget > ubRC_TxTarget)?TRUE:FALSE;
		if(ubRC_CurFps != ubTargetFps)
		{
			ubRC_TxTarget = ubBufTarget;
			ubRC_CurFps = ubTargetFps;
			KNL_SetVdoFps(ubRC_CurFps);
			SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
			RC_SetUpdateFlg(ubCodecIdx, (ubRC_UpdateFlg[ubCodecIdx]|0x1));
		}
	}
	return ubUpdFlag;
}
#endif	//! End of #if (defined(S2019A) && defined(OP_STA))
uint8_t ubRC_QtyAndDynaFpsProcess(uint8_t ubCodecIdx)
{
#if (defined(S2019A) && defined(OP_STA))
#define SPRF_MAX_QP_VAL	45
	sPRF_DevId_t tTxId;
	sPRF_RateMd_t tRC_TxRateMd = sPRF_NORMALMD_RATE;
	uint8_t ubRC_TxNum = 1, ubCurTarget = 3;
	uint8_t ubPerTarget = 0, ubBufTarget = 0;
	uint8_t ubRC_FreeBufNum = 0, ubRC_TxMdSel = 0;
	uint8_t ubRC_TxPer = 0, ubLvl, ubUpdFlag = 0;
	uint8_t ubBpsLvlCnt = sizeof tRC_sPRfTxBps / sizeof(RC_TxBps_t);
	uint8_t ubTargetFps = 15, ubTxRateMd = 0;
	uint8_t ubLowLaty = FALSE, ubQpLvl = 0;
	uint8_t ubSpecfyMd = FALSE;
	uint32_t ulCurTargetDr = 0, ulTotalBitPerSec = 0;
	float fCurTargetDr = 0;
	float fPercent[] = {
							[RC_RATIO_10] = 1, [RC_RATIO_20]  = 2,   [RC_RATIO_30] = 3,
							[RC_RATIO_40] = 4, [RC_RATIO_50]  = 5,	 [RC_RATIO_60] = 6,
							[RC_RATIO_70] = 7, [RC_RATIO_75]  = 7.5, [RC_RATIO_80] = 8,
							[RC_RATIO_90] = 9, [RC_RATIO_100] = 10,
					   };

	tTxId = tsPRF_GetDevId();
	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)) &&
	   (sPRF_DEVDRV_EN == tsPRF_GetDrvSts(tTxId)))
	{
		ulTotalBitPerSec = ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO, ubCodecIdx)/8192;
		tRC_TxRateMd = tsPRF_GetRateLvl();
		ubRC_TxNum   = ubsPRF_GetAttachedDevNums();
		ubRC_TxMdSel = (ubRC_TxNum < 1)?1:(ubRC_TxNum >= 3)?3:ubRC_TxNum;
		ubRC_TxPer   = tsPRF_GetPER(tTxId);
		if(ubRC_TxPer > 100)
			ubRC_TxPer = 100;
		ubPerTarget = ubBpsLvlCnt - 1;
		for(ubLvl = 0; ubLvl < ubBpsLvlCnt; ubLvl++)
		{
			if(ubRC_TxPer <= tRC_sPRfTxBps[ubLvl].ubPer)
			{
				ubPerTarget = ubLvl;
				break;
			}
		}
		ubRC_FreeBufNum = ubKNL_GetSPRFDataBufNum(tTxId);
		ubBufTarget = ubBpsLvlCnt - 1;
		for(ubLvl = 0; ubLvl < ubBpsLvlCnt; ubLvl++)
		{
			if(ubRC_FreeBufNum >= tRC_sPRfTxBps[ubLvl].ubBufNum[ubRC_TxMdSel-1])
			{
				ubBufTarget = ubLvl;
				break;
			}
		}
		ubCurTarget = (ubPerTarget >= ubBufTarget)?ubPerTarget:ubBufTarget;
		if((ubRC_TxTarget) && (ubCurTarget < ubRC_TxTarget))
			ubCurTarget = ubRC_TxTarget - 1;
		ubTxRateMd  = (sPRF_NORMALMD_RATE == (tRC_TxRateMd&0x1))?0:1;
		ubTargetFps = tRC_sPRfTxFps[ubTxRateMd].ubTargetFps[ubCurTarget];	
		ubLowLaty  = (tRC_TxRateMd & 0x8)?TRUE:FALSE;
		ubSpecfyMd = (ubRC_TxNum <= 2)?((tRC_TxRateMd >> 2) & 0x1):FALSE;
		ubQpLvl    = (TRUE == ubLowLaty)?3:(ubRC_TxMdSel-1);
		ubQpLvl    = (TRUE == ubSpecfyMd)?4:ubQpLvl;
		ubRC_FinalMaxQp[ubCodecIdx] = (TRUE == ubLowLaty)?SPRF_MAX_QP_VAL:(tRC_sPRfTxBps[ubCurTarget].ubMaxQp + (((TRUE == ubSpecfyMd) && (ubCurTarget <= 2))?(2 - ubCurTarget):0));
		ubRC_FinalMinQp[ubCodecIdx] = tRC_sPRfTxBps[ubCurTarget].ubMinQp[ubQpLvl] + (((TRUE == ubSpecfyMd) && (ubCurTarget <= 2))?(4 - ubCurTarget):0);
		fCurTargetDr = tRC_sPRfTxBps[ubCurTarget].ulTargetBitRate[(ubRC_TxMdSel-1)] * (fPercent[tRC_sPRfTxBps[ubCurTarget].tRcRatio[ubQpLvl]]/10);
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr = fCurTargetDr;
		if(ubRC_CurFps != ubTargetFps)
		{
			ubRC_CurFps = ubTargetFps;
			KNL_SetVdoFps(ubRC_CurFps);
			SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
			ubUpdFlag |= 1;
		}
		if(ubRC_TxTarget != ubCurTarget)
		{
			ubRC_TxTarget = ubCurTarget;
			ubUpdFlag |= 2;
		}
		if(ubUpdFlag)
			RC_SetUpdateFlg(ubCodecIdx, ubUpdFlag);
	}
	else
	{
		ubRC_TxNum = 0;
		ubRC_FinalMaxQp[ubCodecIdx]   = SPRF_MAX_QP_VAL;
		ubRC_FinalMinQp[ubCodecIdx]   = 35;
		ulCurTargetDr			  	  = 80*1024*8L;
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr;
		ulTotalBitPerSec = 0;
	}
	printd(DBG_CriticalLvl, "\nW[%d]D=%d,m=%d,t=%d_%d,p=%02d%%,Q=%02d,bf=%02d,F=%02d,NB=%03d,V=%d\r\n",
		tTxId,
		ubRC_TxNum,
		tRC_TxRateMd,
		ubRC_TxTarget,
		ubRC_TxMdSel,
		ubRC_TxPer,
		ubRC_GetAvgQp(ubCodecIdx),
		ubRC_FreeBufNum,
		ubRC_CurFps,
		ulCurTargetDr/8192,
		ulTotalBitPerSec);
	return ubUpdFlag;
#else
	ubRC_FinalMaxQp[ubCodecIdx]   = 45;
	ubRC_FinalMinQp[ubCodecIdx]   = 35;
	ulRC_FinalBitRate[ubCodecIdx] = 20*1024*8L;
	return TRUE;
#endif	//! End of #if (defined(S2019A) && defined(OP_STA))
}
#if (defined(S2019A) && defined(OP_STA) && defined(RVCS_APP))
#define DT_FPS_RC_LVL		6
typedef struct
{
	uint8_t ubBufNum[DT_FPS_RC_LVL];
	uint8_t ubTargetFps[DT_FPS_RC_LVL];
}RC_TxFpsDT_t;
#if (VDO_FRC_FPS == 25)
RC_TxFpsDT_t tRC_sPRfTxFpsDT[] = {
								//!       Buffer			   FPS
								{{18, 15, 11,  9, 7, 5},   {25, 22, 15, 12, 10, 10}},		//!< Rate mode = sPRF_NORMALMD_RATE								
								{{20, 16, 16, 11, 11, 5},  {15, 13, 11, 10, 10, 10}},		//!< Rate mode = sPRF_LOWMD_RATE
						     };

#else
RC_TxFpsDT_t tRC_sPRfTxFpsDT[] = {
								//!       Buffer			   FPS
								{{19, 17, 15, 11, 8, 5},   {15, 12, 10, 10, 10, 10}},		//!< Rate mode = sPRF_NORMALMD_RATE
								{{20, 16, 16, 11, 11, 5},  {15, 13, 11, 10, 10, 10}},		//!< Rate mode = sPRF_LOWMD_RATE
						     };
#endif

#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
RC_TxFpsDT_t tRC_sBLETxFpsDT[] = {
								//!       Buffer			   FPS
								{{18, 15, 11,  9, 7, 5},   {7/*5*/, 5/*4*/, 4, 3, 2, 1}},		//!< Rate mode = sPRF_NORMALMD_RATE								
								{{20, 16, 16, 11, 11, 5},  {5, 4, 3, 2, 1, 1}},		//!< Rate mode = sPRF_LOWMD_RATE
						     };
#endif

typedef struct
{
	uint8_t  ubPer;
	uint8_t  ubMaxQp;
	uint8_t  ubMinQp;			
	uint32_t ulTargetBitRate;	
	RC_RATIO tRcRatio;			
	uint8_t  ubBufNum;			
}RC_TxBpsDT_t;

#if (E_RVCS_ALL_I_FRM == 1)
#if (DT_DISPLAY_MODE == DT_DISPLAY_FHD)	
RC_TxBpsDT_t tRC_sPRfTxBpsDT[] = {//! FHD					                       
								{20,  45,  40,  ( 320*1024*8L), RC_RATIO_100, 19},
								{20,  47,  43,  ( 260*1024*8L), RC_RATIO_90,  17},
								{30,  49,  46,  ( 160*1024*8L), RC_RATIO_80,  15},
								{60,  50,  49,  ( 80*1024*8L), RC_RATIO_60,  11},
								{75,  51,  50,  (  48*1024*8L), RC_RATIO_40,  8},
								{100, 51,  51,  (  20*1024*8L), RC_RATIO_20,  5},
							 };

#elif (DT_DISPLAY_MODE == DT_DISPLAY_HD)	
RC_TxBpsDT_t tRC_sPRfTxBpsDT[] = {//! HD					                       
								{20,  45,  41,  ( 320*1024*8L), RC_RATIO_100, 19},
								{20,  47,  44,  ( 260*1024*8L), RC_RATIO_90,  17},
								{30,  49,  46,  ( 160*1024*8L), RC_RATIO_80,  15},
								{60,  50,  49,  ( 80*1024*8L), RC_RATIO_60,  11},
								{75,  51,  50,  (  48*1024*8L), RC_RATIO_40,  8},
								{100, 51,  51,  (  20*1024*8L), RC_RATIO_20,  5},
							 };
#else // (DT_DISPLAY_MODE == DT_DISPLAY_VGA)	
RC_TxBpsDT_t tRC_sPRfTxBpsDT[] = {//! VGA					                       
								{20,  39,  35,  ( 420*1024*8L), RC_RATIO_100, 19},
								{20,  43,  37,  ( 320*1024*8L), RC_RATIO_90,  17},
								{30,  46,  40,  ( 240*1024*8L), RC_RATIO_80,  15},
								{60,  49,  43,  ( 125*1024*8L), RC_RATIO_60,  11},
								{75,  50,  46,  ( 100*1024*8L), RC_RATIO_40,  8},
								{100, 51,  48,  (  40*1024*8L), RC_RATIO_20,  5},
							 };
#endif	// DT_DISPLAY_MODE
#else
#if (DT_DISPLAY_MODE == DT_DISPLAY_FHD)	
RC_TxBpsDT_t tRC_sPRfTxBpsDT[] = {//! FHD					                      
								{20,  45,  40,	( 360*1024*8L), RC_RATIO_100, 19},
								{20,  47,  43,	( 300*1024*8L), RC_RATIO_90,  17},
								{30,  49,  46,	( 180*1024*8L), RC_RATIO_80,  15},
								{60,  50,  49,	( 80*1024*8L), RC_RATIO_60,  11},
								{75,  51,  50,	(  48*1024*8L), RC_RATIO_40,  8},
								{100, 51,  51,	(  20*1024*8L), RC_RATIO_20,  5},
							 };
#elif (DT_DISPLAY_MODE == DT_DISPLAY_HD)
RC_TxBpsDT_t tRC_sPRfTxBpsDT[] = {//! HD					                      
								{20,  40,  37,	( 320*1024*8L), RC_RATIO_100, 19},
								{20,  43,  40,	( 260*1024*8L), RC_RATIO_90,  17},
								{30,  47,  42,	( 160*1024*8L), RC_RATIO_80,  15},
								{60,  50,  49,	( 80*1024*8L), RC_RATIO_60,  11},
								{75,  51,  50,	(  48*1024*8L), RC_RATIO_40,  8},
								{100, 51,  51,	(  20*1024*8L), RC_RATIO_20,  5},
							 };
#else // (DT_DISPLAY_MODE == DT_DISPLAY_VGA)
RC_TxBpsDT_t tRC_sPRfTxBpsDT[] = {//! VGA				                       
								{20,  39,  35,  ( 420*1024*8L), RC_RATIO_100, 19},
								{20,  43,  37,  ( 320*1024*8L), RC_RATIO_90,  17},
								{30,  46,  40,  ( 240*1024*8L), RC_RATIO_80,  15},
								{60,  49,  43,  ( 125*1024*8L), RC_RATIO_60,  11},
								{75,  50,  46,  ( 100*1024*8L), RC_RATIO_40,  8},
								{100, 51,  48,  (  40*1024*8L), RC_RATIO_20,  5},
							 };
#endif 	// DT_DISPLAY_MODE
#endif	// (E_RVCS_ALL_I_FRM == 1)

#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
RC_TxBpsDT_t tRC_sBLETxBpsDT[] = {//! for BLE video					                      
								{20,  39,  34,	( 200/*20*/*1024*8L), RC_RATIO_100, 19},
								{20,  43,  37,	( 150/*20*/*1024*8L), RC_RATIO_90,  17},
								{30,  47,  40,	( 110/*20*/*1024*8L), RC_RATIO_80,  15},
								{60,  49,  43,	( 90*1024*8L), RC_RATIO_60,  11},
								{75,  50,  46,	( 45*1024*8L), RC_RATIO_40,  8},
								{100, 51,  48,	( 25*1024*8L), RC_RATIO_20,  5},};
#endif

#define RC_DT_MAX_BIT_RATE			(768*1024)   //1200000  		// HD 2200000
#define E_VDO_MAX_QP				50
#define E_VDO_MIN_QP				25
static float ubRC_TxTargetDr = 0;
#if (E_RVCS_ALL_I_FRM == 1)
static uint8_t ubRC_1stActive = 0;
#endif
#if (VDO_RESO_AUTO_SWITCH == 1)
uint8_t ubMonitorFpsRec[30];
uint8_t ubRC_MonitorFps(uint8_t ubCurFps, uint8_t ubReset)
{
	static uint8_t ubCurRecIdx = 0;
	static uint8_t ubAvgFps = 0xFF;
	uint8_t i = 0;
	uint16_t ubAccFps;

	if (VDO_RESO_AUTO_SWCH_OBSV_SEC > sizeof(ubMonitorFpsRec)/sizeof(uint8_t))
	{
		printd(DBG_ErrorLvl, "Err: observing sec exceeds %d\n", sizeof(ubMonitorFpsRec)/sizeof(uint8_t));
		return 0;
	}

	if (ubReset == 1) {
		memset((uint8_t*)ubMonitorFpsRec, 0, sizeof(ubMonitorFpsRec)/sizeof(uint8_t));
		ubCurRecIdx = 0;
		ubAvgFps = 0xFF;
	}

	ubMonitorFpsRec[ubCurRecIdx] = ubCurFps;

	if (ubCurRecIdx == VDO_RESO_AUTO_SWCH_OBSV_SEC-1) {
		ubAccFps = 0;
		for (i=0; i<VDO_RESO_AUTO_SWCH_OBSV_SEC; i++) {
			ubAccFps += ubMonitorFpsRec[i];
		}
		ubAvgFps = (uint8_t)(ubAccFps / VDO_RESO_AUTO_SWCH_OBSV_SEC);
	}
	else
		ubAvgFps = 0xFF;

	ubCurRecIdx = (ubCurRecIdx+1) % VDO_RESO_AUTO_SWCH_OBSV_SEC;

	return ubAvgFps;
}
#endif
#endif
void RC_DtDynaQtyFpsProcess(uint8_t ubCodecIdx)
{
#if (defined(S2019A) && defined(OP_STA) && defined(RVCS_APP))
	uint8_t ubCurTarget = 0, ubBufTarget = 0;
	uint8_t ubRC_FreeBufNum = 0; 
	uint8_t ubLvl = 0, ubUpdFlag = 0, ubCKFlag = 0;
	uint8_t ubBpsLvlCnt = sizeof tRC_sPRfTxBpsDT / sizeof(RC_TxBpsDT_t);
	uint8_t ubTargetFps = 15, ubCurFps;
	#if (VDO_RESO_AUTO_SWITCH == 1)
	uint8_t ubAvgFps=0xFF;
	#endif
	uint8_t ubTxRate = 0;
	uint32_t ulCurTargetDr = 0, ulTotalBitPerSec = 0;
	float fCurTargetDr = 0;
	char AvgFpsInfo[16] = {0};
	float fPercent[] = {
							[RC_RATIO_10] = 1, [RC_RATIO_20]  = 2,   [RC_RATIO_30] = 3,
							[RC_RATIO_40] = 4, [RC_RATIO_50]  = 5,	 [RC_RATIO_60] = 6,
							[RC_RATIO_70] = 7, [RC_RATIO_75]  = 7.5, [RC_RATIO_80] = 8,
							[RC_RATIO_90] = 9, [RC_RATIO_100] = 10,
					   };
  #if (APP_BLE_SN9380_FUNC_ENABLE == 1)
	RC_TxBpsDT_t * tRC_BpsTbl; 
  #endif

	ubTxRate = ubWiFiDt_GetRateLvl(1);	
  #if (APP_BLE_SN9380_FUNC_ENABLE == 1)
  	if (UIcmd_GetConnSts() == 1 && UIcmd_GetVideoStartStatus() == 1)
		ubCurFps = UART2_BLEVideo_GetFps(1);
	else
		ubCurFps = ubWiFiDt_GetTransFrm();
  #else
	ubCurFps = ubWiFiDt_GetTransFrm();
  #endif
	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)))
	{
		ulTotalBitPerSec = ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO, ubCodecIdx)/8192;
	#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
		if (UIcmd_GetConnSts() == 1 && UIcmd_GetVideoStartStatus() == 1) {
			ubRC_FreeBufNum = uwWiFiDt_BleUse_GetTxVdoQues(1);
			tRC_BpsTbl = tRC_sBLETxBpsDT;
		}
		else {
			ubRC_FreeBufNum = uwWiFiDt_GetTxVdoQues(1);
			tRC_BpsTbl = tRC_sPRfTxBpsDT;
		}

		ubCKFlag = 0;
		for(ubLvl = 0; ubLvl < ubBpsLvlCnt; ubLvl++)
		{
			if(ubRC_FreeBufNum >= tRC_BpsTbl[ubLvl].ubBufNum)
			{
				ubCKFlag = 1;
				break;
			}
		}
	  #else
		ubRC_FreeBufNum = uwWiFiDt_GetTxVdoQues(1);
		ubCKFlag = 0;
		for(ubLvl = 0; ubLvl < ubBpsLvlCnt; ubLvl++)
		{
			if(ubRC_FreeBufNum >= tRC_sPRfTxBpsDT[ubLvl].ubBufNum)
			{
				ubCKFlag = 1;
				break;
			}
		}
	  #endif
		
		if (ubCKFlag == 1)
			ubBufTarget = ubLvl;
		else 
			ubBufTarget = ubBpsLvlCnt - 1;
	#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
		if (UIcmd_GetConnSts() == 1 && UIcmd_GetVideoStartStatus() == 1) {
			ubTargetFps = tRC_sBLETxFpsDT[0].ubTargetFps[ubBufTarget];
			fCurTargetDr = tRC_sBLETxBpsDT[ubTxRate].ulTargetBitRate * (fPercent[tRC_sBLETxBpsDT[ubBufTarget].tRcRatio]/10);
		}
		else {
			ubTargetFps = tRC_sPRfTxFpsDT[0].ubTargetFps[ubBufTarget];
			fCurTargetDr = tRC_sPRfTxBpsDT[ubTxRate].ulTargetBitRate * (fPercent[tRC_sPRfTxBpsDT[ubBufTarget].tRcRatio]/10);
		}
	#else
		ubTargetFps = tRC_sPRfTxFpsDT[0].ubTargetFps[ubBufTarget];

		fCurTargetDr = tRC_sPRfTxBpsDT[ubTxRate].ulTargetBitRate * (fPercent[tRC_sPRfTxBpsDT[ubBufTarget].tRcRatio]/10);
	#endif
	#if (VDO_RESO_AUTO_SWITCH == 1)	
		fCurTargetDr = fCurTargetDr * KNL_RC_ResoSwitch_Ratio();
	#endif
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr = fCurTargetDr;
		ubCKFlag = 0;
		for(ubLvl = 0; ubLvl < ubBpsLvlCnt; ubLvl++)
		{
		#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
			if (UIcmd_GetConnSts() == 1 && UIcmd_GetVideoStartStatus() == 1) {
				if(fCurTargetDr >= tRC_sBLETxBpsDT[ubLvl].ulTargetBitRate)
				{
					ubCKFlag = 1;
					break;
				}
			}
			else {
				if(fCurTargetDr >= tRC_sPRfTxBpsDT[ubLvl].ulTargetBitRate)
				{
					ubCKFlag = 1;
					break;
				}
			}
		#else
			if(fCurTargetDr >= tRC_sPRfTxBpsDT[ubLvl].ulTargetBitRate)
			{
				ubCKFlag = 1;
				break;
			}
		#endif
		}
		if (ubCKFlag == 1)
			ubCurTarget = ubLvl;
		else 
			ubCurTarget = ubBpsLvlCnt - 1;

		#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
		if (UIcmd_GetConnSts() == 1 && UIcmd_GetVideoStartStatus() == 1) {
			ubRC_FinalMaxQp[ubCodecIdx] = tRC_sBLETxBpsDT[ubCurTarget].ubMaxQp;
			ubRC_FinalMinQp[ubCodecIdx] = tRC_sBLETxBpsDT[ubCurTarget].ubMinQp;
		}
		else {
			ubRC_FinalMaxQp[ubCodecIdx] = tRC_sPRfTxBpsDT[ubCurTarget].ubMaxQp;
			ubRC_FinalMinQp[ubCodecIdx] = tRC_sPRfTxBpsDT[ubCurTarget].ubMinQp;
		}
		#else
		ubRC_FinalMaxQp[ubCodecIdx] = tRC_sPRfTxBpsDT[ubCurTarget].ubMaxQp;
		ubRC_FinalMinQp[ubCodecIdx] = tRC_sPRfTxBpsDT[ubCurTarget].ubMinQp;
		#endif

		if(ubRC_CurFps != ubTargetFps)
		{
			ubRC_CurFps = ubTargetFps;
			KNL_SetVdoFps(ubRC_CurFps);
			SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
			ubUpdFlag |= 1;
		}
		if(ubRC_TxTargetDr != fCurTargetDr)
		{
			ubRC_TxTargetDr = fCurTargetDr;
			ubUpdFlag |= 2;
		}
#if (E_RVCS_ALL_I_FRM == 1)
		if((ubUpdFlag)||(ubRC_1stActive == 0)) {
			ubRC_1stActive = 1;
			RC_SetUpdateFlg(ubCodecIdx, ubUpdFlag);
		}
#else
		if(ubUpdFlag) {
			RC_SetUpdateFlg(ubCodecIdx, ubUpdFlag);
		}
#endif

#if (VDO_RESO_AUTO_SWITCH == 1) && (DT_DISPLAY_MODE == DT_DISPLAY_HD || DT_DISPLAY_MODE == DT_DISPLAY_FHD) && (APP_BLE_SN9380_FUNC_ENABLE == 0)
		ubAvgFps = ubRC_MonitorFps(ubCurFps, 0);
		if (ubAvgFps != 0xFF)
			sprintf(AvgFpsInfo, "afps:%d,", ubAvgFps);
		else
			sprintf(AvgFpsInfo, "afps:x,");
		if (ubAvgFps != 0xFF) {
			if (ubAvgFps < VDO_RESO_SWCH_DWN_FPS)
				KNL_SwitchRes(0);
			else if (ubAvgFps >= VDO_RESO_SWCH_UP_FPS)
				KNL_SwitchRes(1);
		}
#endif		
	}
	else
	{
		ubRC_FinalMaxQp[ubCodecIdx] = 51;
		ubRC_FinalMinQp[ubCodecIdx] = 35;
		ulCurTargetDr			  	= 80*1024*8L;
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr;
		ulTotalBitPerSec = 0;
#if (E_RVCS_ALL_I_FRM == 1)
		ubRC_1stActive = 0;
#endif
#if (VDO_RESO_AUTO_SWITCH == 1)
		ubAvgFps = ubRC_MonitorFps(ubCurFps, 1);
		if (ubAvgFps != 0xFF)
			sprintf(AvgFpsInfo, "afps:%d,", ubAvgFps);
		else
			sprintf(AvgFpsInfo, "afps:x,");
#endif
	}

	printd(DBG_CriticalLvl, "\nfps:%2d,%sQ:%2d,CQ:%2d,B:%2d,QP:%d,RSI[%3d],F=%02d Dg[%d %d %d/%d],BR[%03d/%d],Ps:%d/%d,Twc:%d/%d/%d,Hb:%d/%d,WT:%d,BLE[%d/%d],[%s]\n",
			ubCurFps, AvgFpsInfo,
			#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
			(UIcmd_GetConnSts() == 1 && UIcmd_GetVideoStartStatus() == 1) ? uwWiFiDt_BleUse_GetTxVdoQues(0):uwWiFiDt_GetTxVdoQues(0), 
			#else
			uwWiFiDt_GetTxVdoQues(0),
			#endif
			uwWiFiDt_GetRxCmdQues(), uwWiFiDt_GetBufUsedLvl(),	// Q, CQ, B
			ubRC_GetAvgQp(ubCodecIdx)/*H264_GetCurrentQP()*/, (int8_t)ubWiFiDt_GetRssi(0),
			ubTargetFps, 
			ubUpdFlag, ubTxRate, ubBufTarget, ubCurTarget,	// Dg
			ulCurTargetDr/8192, ulTotalBitPerSec,	// BR
			ubWiFiDt_getPwrSvStatus(), ubWiFiDt_getPwrSvTime(),	// Ps
			uwWiFiDt_getRcvTwcCnt(), uwWiFiDt_getRcvFrmAckCnt(1), uwWiFiDt_getRcvFrmAckCnt(0),
			uwWiFiDt_getRcvPwrSvCtrlPktCnt(1), uwWiFiDt_getRcvPwrSvCtrlPktCnt(0), uwWiFiDt_getFrmAckWaitTime(),
			#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
			UIcmd_video_flow_ctrl_val(E_BLE_UART_TX_PKT_CNT), UIcmd_video_flow_ctrl_val(E_BLE_SN9380_SEND_OUT),
			#else
			0, 0,
			#endif
			uwKNL_GetVdoH(ubKNL_GetRole()) == FHD_WIDTH ? "FHD" : uwKNL_GetVdoH(ubKNL_GetRole()) == HD_WIDTH ? "HD" : "VGA");
	
	return;
#else
	ubRC_FinalMaxQp[ubCodecIdx]   = 51;
	ubRC_FinalMinQp[ubCodecIdx]   = 35;
	ulRC_FinalBitRate[ubCodecIdx] = 20*1024*8L;
#endif	//! End of #if (defined(S2019A) && defined(OP_STA) && defined(RVCS_APP))
}
#if (defined(S2019A) && defined(OP_STA) && defined(RVCS_APP))
void RC_DtModeReset(uint8_t ubCodecIdx, uint8_t ubRstFlag)
{
	RC_DtDynaQtyFpsProcess(ubCodecIdx);
}
#endif	//! End of #if (defined(S2019A) && defined(OP_STA) && defined(RVCS_APP))
//------------------------------------------------------------------------------

#endif	//! End of #if OP_STA

#if OP_STA
void RC_SysThread(void const *argument)
{	
	uint8_t ubIdx = 10-1;	
	
	while(1)
	{
		ubIdx++;		
		if(ubIdx >= 10)
		{
			ubIdx = 0;
		}
		
		ubRC_Qp[0][ubIdx] = ubKNL_GetQp(0);
		ubRC_Qp[1][ubIdx] = ubKNL_GetQp(1);
		ubRC_Qp[2][ubIdx] = ubKNL_GetQp(2);
		ubRC_Qp[3][ubIdx] = ubKNL_GetQp(3);	
		
		printf(" >>- Qp [%d] \n", ubKNL_GetQp(0));	//xxx
		
		osDelay(100);
	}	
}
#endif

void RC_MonitLatencyThread(void const *argument)
{
#if (defined(A7130))
	static uint8_t HighBW1=1;
#if OP_AP	
	static uint8_t HighBW2=1,HighBW3=1,HighBW4=1;
#endif
	
	osDelay(1000);
	while(1)
	{
		#ifdef OP_STA
		
		if(ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)>100*1024)
		{	
			if(HighBW1==0)
			{
				//! Latency Setting
				ADO_SetRcvTimeLatency(0, 250);
				printd(DBG_Debug3Lvl,"BB_GET_RXSTA1_VDO_FLOW Latency = 250 \n");
				HighBW1 = 1;
			}
		}
		else
		{
			if(HighBW1==1)
			{
				//! Latency Setting
				ADO_SetRcvTimeLatency(0, 1000);
				printd(DBG_Debug3Lvl,"BB_GET_RXSTA1_VDO_FLOW Latency = 1000\n");
				HighBW1 = 0;
			}
		}
		#endif
		
		#ifdef OP_AP
		if(ulBB_GetBBFlow(BB_GET_RXSTA1_VDO_FLOW)>100*1024)
		{	
			if(HighBW1==0)
			{
				//! Latency Setting
				ADO_SetRcvTimeLatency(BB_GET_RXSTA1_VDO_FLOW, 250);
				printd(DBG_Debug3Lvl,"BB_GET_RXSTA1_VDO_FLOW Latency = 250 \n");
				HighBW1 = 1;
			}
		}
		else
		{
			if(HighBW1==1)
			{
				//! Latency Setting

					ADO_SetRcvTimeLatency(BB_GET_RXSTA1_VDO_FLOW, 1000);
				printd(DBG_Debug3Lvl,"BB_GET_RXSTA1_VDO_FLOW Latency = 1000\n");
				HighBW1 = 0;
			}
		}
		
		if(ulBB_GetBBFlow(BB_GET_RXSTA2_VDO_FLOW)>100*1024)
		{	
			if(HighBW2==0)
			{
				//! Latency Setting

					ADO_SetRcvTimeLatency(BB_GET_RXSTA2_VDO_FLOW, 250);
					printd(DBG_Debug3Lvl,"BB_GET_RXSTA2_VDO_FLOW Latency = 250 \n");
				HighBW2 = 1;
			}
		}
		else
		{
			if(HighBW2==1)
			{
				//! Latency Setting

					ADO_SetRcvTimeLatency(BB_GET_RXSTA2_VDO_FLOW, 1000);
					printd(DBG_Debug3Lvl,"BB_GET_RXSTA2_VDO_FLOW Latency = 1000 \n");
				HighBW2 = 0;
			}
		}
		
		if(ulBB_GetBBFlow(BB_GET_RXSTA3_VDO_FLOW)>100*1024)
		{	
			if(HighBW3==0)
			{
				//! Latency Setting

					ADO_SetRcvTimeLatency(BB_GET_RXSTA3_VDO_FLOW, 250);
					printd(DBG_Debug3Lvl,"BB_GET_RXSTA3_VDO_FLOW Latency = 250 \n");
				HighBW3 = 1;
			}
		}
		else
		{
			if(HighBW3==1)
			{
				//! Latency Setting

					ADO_SetRcvTimeLatency(BB_GET_RXSTA3_VDO_FLOW, 1000);
					printd(DBG_Debug3Lvl,"BB_GET_RXSTA3_VDO_FLOW Latency = 1000 \n");
				HighBW3 = 0;
			}
		}
		
		if(ulBB_GetBBFlow(BB_GET_RXSTA4_VDO_FLOW)>100*1024)
		{	
			if(HighBW4==0)
			{
				//! Latency Setting

					ADO_SetRcvTimeLatency(BB_GET_RXSTA4_VDO_FLOW, 250);
					printd(DBG_Debug3Lvl,"BB_GET_RXSTA4_VDO_FLOW Latency = 250 \n");
				HighBW4 = 1;
			}
		}
		else
		{
			if(HighBW4==1)
			{
				//! Latency Setting

					ADO_SetRcvTimeLatency(BB_GET_RXSTA4_VDO_FLOW, 1000);
					printd(DBG_Debug3Lvl,"BB_GET_RXSTA4_VDO_FLOW Latency = 1000 \n");
				HighBW4 = 0;
			}
		}
		#endif

		osDelay(500);
	}
#else
	while(1)
	{
		osDelay(60000);
	}
#endif
}

#if OP_STA
void RC_MonitThread0(void const *argument)
{
	uint8_t ubRC_FixPreset0Flg = 0;
	uint8_t ubForceGopFlg = 1;
    uint8_t ubSrc;
	uint16_t uwHSize = 0, uwVSize = 0;

	ubForceGopFlg = ubForceGopFlg;
#if APP_UVC_CAM_ENABLE
	ubForceGopFlg = 1;
#endif

	osDelay(1000);
	while(1)
	{
		if(!ubRC_GetFlg(0))
		{
			//if((!ubRC_FixPreset0Flg) && (KNL_TUNINGMODE_ON == KNL_GetTuningToolMode()))
			if((!ubRC_FixPreset0Flg) && ((KNL_TUNINGMODE_ON == KNL_GetTuningToolMode()) || ubForceGopFlg))
			{
				if(KNL_TUNINGMODE_ON == KNL_GetTuningToolMode()) //for UVC出图
				{
				SEN_SetFrameRate(SENSOR_PATH1, KNL_RC_FPS);
				H264_SetGOP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, KNL_RC_GOP);
				H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, KNL_RC_QP_MAX);
				H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, KNL_RC_QP_MIN);
				H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, KNL_RC_BITRATE, KNL_RC_FPS);
                ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
                uwHSize = uwKNL_GetVdoH(ubSrc);
                uwVSize = uwKNL_GetVdoV(ubSrc);
				SetH264Rate(uwHSize, uwVSize, KNL_RC_FPS);
                ubRC_FixPreset0Flg = 1;
				}
				else if((ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_MASTER_AP)<=3)) //for掉线后回连
					{
						SEN_SetFrameRate(SENSOR_PATH1, 12);
						H264_SetGOP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 900);
						H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 50);
						H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 47);
						H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, KNL_RC_BITRATE, 15);
						ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
						uwHSize = uwKNL_GetVdoH(ubSrc);
						uwVSize = uwKNL_GetVdoV(ubSrc);
				   SetH264Rate(uwHSize, uwVSize, KNL_RC_FPS);
           ubRC_FixPreset0Flg = 1;
					}
			}
            printf("B[%d]:Q->%d,F->%d\r\n",tRC_Info[0].ubCodecIdx,ubRC_GetAvgQp(0),ubRC_CurFps);
		}
		else if(tRC_Info[0].ubMode == RC_MODE_FIXED_DATA_RATE)
		{
		}
		else if(tRC_Info[0].ubMode == RC_MODE_DYNAMIC_QTY_AND_BW)
		{
			RC_QtyAndBwProcess(0);
			ubRC_FixPreset0Flg = 0;
		}
		else if((tRC_Info[0].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[0].tAdjSequence == RC_ADJ_QTY_THEN_FPS))
		{			
			RC_QtyAndFpsProcess1(0);
			ubRC_FixPreset0Flg = 0;
		}
		else if((tRC_Info[0].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[0].tAdjSequence == RC_ADJ_FPS_THEN_QTY))
		{			
			RC_QtyAndFpsProcess2(0);
			ubRC_FixPreset0Flg = 0;
		}
		else if(tRC_Info[0].ubMode == RC_MODE_DYNAMIC_FPS2)
		{
			RC_DynamicFpsProcess(0);
            ubRC_FixPreset0Flg = 0;
		}
		else if(tRC_Info[0].ubMode == RC_QTY_AND_DYNAFPS)
		{
			ubRC_QtyAndDynaFpsProcess(0);
			ubRC_FixPreset0Flg = 0;
		}
#if (defined(OP_STA) && defined(S2019A) && defined(RVCS_APP))
		else if(tRC_Info[0].ubMode == RC_DT_DYNAQTYFPS)
		{
			RC_DtDynaQtyFpsProcess(0);
			ubRC_FixPreset0Flg 	= 0;
		}
#endif
		osDelay(tRC_Info[0].ulUpdateRatePerMs);
	}
}


void RC_MonitThread1(void const *argument)
{
	uint8_t ubRC_FixPreset1Flg = 0;
	uint8_t ubForceGopFlg = 0;
	
	ubForceGopFlg = ubForceGopFlg;
#if APP_UVC_CAM_ENABLE
	ubForceGopFlg = 1;
#endif

	osDelay(1000);
	while(1)
	{
		if(!ubRC_GetFlg(1))
		{
			//if((!ubRC_FixPreset1Flg) && (KNL_TUNINGMODE_ON == KNL_GetTuningToolMode()))
			if((!ubRC_FixPreset1Flg) && ((KNL_TUNINGMODE_ON == KNL_GetTuningToolMode()) || ubForceGopFlg))
			{
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[0].ubInitFps);
				H264_SetGOP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 15);
				H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMinQp);
				//IQ_SetAEPID(((tRC_Info[0].ubInitFps <= 5)?1:0));
				IQ_SetAEPID(((ubRC_GetEngModeFps(0) <= 5)?1:0));
				H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,ulRC_GetEngModeBitRate(0),ubRC_GetEngModeFps(0));
				ubRC_FixPreset1Flg = 1;
			}
		}
		if(tRC_Info[1].ubMode == RC_MODE_FIXED_DATA_RATE)
		{
		}
		else if(tRC_Info[1].ubMode == RC_MODE_DYNAMIC_QTY_AND_BW)
		{
			RC_QtyAndBwProcess(1);
			ubRC_FixPreset1Flg = 0;
		}
		else if((tRC_Info[1].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[1].tAdjSequence == RC_ADJ_QTY_THEN_FPS))
		{			
			RC_QtyAndFpsProcess1(1);
			ubRC_FixPreset1Flg = 0;
		}
		else if((tRC_Info[1].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[1].tAdjSequence == RC_ADJ_FPS_THEN_QTY))
		{			
			RC_QtyAndFpsProcess2(1);
			ubRC_FixPreset1Flg = 0;
		}
		else if(tRC_Info[1].ubMode == RC_QTY_AND_DYNAFPS)
		{
			ubRC_QtyAndDynaFpsProcess(1);
			ubRC_FixPreset1Flg = 0;
		}
		else if(tRC_Info[1].ubMode == RC_DT_DYNAQTYFPS)
		{
			RC_DtDynaQtyFpsProcess(1);
			ubRC_FixPreset1Flg = 0;
		}
		osDelay(tRC_Info[1].ulUpdateRatePerMs);
	}	
}

void RC_MonitThread2(void const *argument)
{
	uint8_t ubRC_FixPreset2Flg = 0;
	uint8_t ubForceGopFlg = 0;
	
	ubForceGopFlg = ubForceGopFlg;
#if APP_UVC_CAM_ENABLE
	ubForceGopFlg = 1;
#endif

	osDelay(1000);
	while(1)
	{
		if(!ubRC_GetFlg(2))
		{
			//if((!ubRC_FixPreset2Flg) && (KNL_TUNINGMODE_ON == KNL_GetTuningToolMode()))
			if((!ubRC_FixPreset2Flg) && ((KNL_TUNINGMODE_ON == KNL_GetTuningToolMode()) || ubForceGopFlg))
			{
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[0].ubInitFps);
				H264_SetGOP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 15);
				H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMinQp);
				//IQ_SetAEPID(((tRC_Info[0].ubInitFps <= 5)?1:0));
				IQ_SetAEPID(((ubRC_GetEngModeFps(0) <= 5)?1:0));
				H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,ulRC_GetEngModeBitRate(0),ubRC_GetEngModeFps(0));
				ubRC_FixPreset2Flg = 1;
			}
		}
		if(tRC_Info[2].ubMode == RC_MODE_FIXED_DATA_RATE)
		{
		}
		else if(tRC_Info[2].ubMode == RC_MODE_DYNAMIC_QTY_AND_BW)
		{
			RC_QtyAndBwProcess(2);
			ubRC_FixPreset2Flg = 0;
		}
		else if((tRC_Info[2].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[2].tAdjSequence == RC_ADJ_QTY_THEN_FPS))
		{			
			RC_QtyAndFpsProcess1(2);
			ubRC_FixPreset2Flg = 0;
		}
		else if((tRC_Info[2].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[2].tAdjSequence == RC_ADJ_FPS_THEN_QTY))
		{			
			RC_QtyAndFpsProcess2(2);
			ubRC_FixPreset2Flg = 0;
		}
		else if(tRC_Info[2].ubMode == RC_QTY_AND_DYNAFPS)
		{
			ubRC_QtyAndDynaFpsProcess(2);
			ubRC_FixPreset2Flg = 0;
		}
		else if(tRC_Info[2].ubMode == RC_DT_DYNAQTYFPS)
		{
			RC_DtDynaQtyFpsProcess(2);
			ubRC_FixPreset2Flg = 0;
		}
		osDelay(tRC_Info[2].ulUpdateRatePerMs);
	}	
}

void RC_MonitThread3(void const *argument)
{
	uint8_t ubRC_FixPreset3Flg = 0;
	uint8_t ubForceGopFlg = 0;
	
	ubForceGopFlg = ubForceGopFlg;
#if APP_UVC_CAM_ENABLE
	ubForceGopFlg = 1;
#endif

	osDelay(1000);
	while(1)
	{
		if(!ubRC_GetFlg(3))
		{
			//if((!ubRC_FixPreset3Flg) && (KNL_TUNINGMODE_ON == KNL_GetTuningToolMode()))
			if((!ubRC_FixPreset3Flg) && ((KNL_TUNINGMODE_ON == KNL_GetTuningToolMode()) || ubForceGopFlg))
			{
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[0].ubInitFps);
				H264_SetGOP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 15);
				H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMinQp);
				//IQ_SetAEPID(((tRC_Info[0].ubInitFps <= 5)?1:0));
				IQ_SetAEPID(((ubRC_GetEngModeFps(0) <= 5)?1:0));
				H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,ulRC_GetEngModeBitRate(0),ubRC_GetEngModeFps(0));
				ubRC_FixPreset3Flg = 1;
			}
		}
		if(tRC_Info[3].ubMode == RC_MODE_FIXED_DATA_RATE)
		{
		}
		else if(tRC_Info[3].ubMode == RC_MODE_DYNAMIC_QTY_AND_BW)
		{
			RC_QtyAndBwProcess(3);
			ubRC_FixPreset3Flg = 0;
		}
		else if((tRC_Info[3].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[3].tAdjSequence == RC_ADJ_QTY_THEN_FPS))
		{			
			RC_QtyAndFpsProcess1(3);
			ubRC_FixPreset3Flg = 0;
		}
		else if((tRC_Info[3].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[3].tAdjSequence == RC_ADJ_FPS_THEN_QTY))
		{			
			RC_QtyAndFpsProcess2(3);
			ubRC_FixPreset3Flg = 0;
		}
		else if(tRC_Info[3].ubMode == RC_QTY_AND_DYNAFPS)
		{
			ubRC_QtyAndDynaFpsProcess(3);
			ubRC_FixPreset3Flg = 0;
		}
		else if(tRC_Info[3].ubMode == RC_DT_DYNAQTYFPS)
		{
			RC_DtDynaQtyFpsProcess(3);
			ubRC_FixPreset3Flg = 0;
		}
		osDelay(tRC_Info[3].ulUpdateRatePerMs);
	}	
}

#endif

uint32_t ulRC_GetFinalBitRate(uint8_t ubCodecIdx)
{
	return ulRC_FinalBitRate[ubCodecIdx];
}

//------------------------------------------------------------------------------
void RC_NoneModePreset(void)
{
	RC_INFO tRcInfo;
	
	tRcInfo.ubEnableFlg 		= 0;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubTargetQp			= 43;
	
	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------
void RC_FixedDataRateModePreset(void)
{
	RC_INFO tRcInfo;
	
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_FIXED_DATA_RATE;
	tRcInfo.ulUpdateRatePerMs	= 1000;		
	tRcInfo.ubMinQp				= 28;
	tRcInfo.ubMaxQp				= 46;	
	tRcInfo.ulInitBitRate		= 0x168000L;	//180KByte
	tRcInfo.ubInitFps			= 15;	

	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------

void RC_QtyAndBwModePresetFor1SlotMode(void)
{
	RC_INFO tRcInfo;
	
	//Max.Bw -> 440KB		
	tRcInfo.ulInitBitRate 	= 220*1024*8L;	//50% MAX.BW @1T1R Mode
	tRcInfo.ulBwBuf			= 30*1024*8L;
	tRcInfo.ubInitFps		= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(KNL_DISP_SINGLE), VDO_MAIN_V_SIZE(KNL_DISP_SINGLE));
	tRcInfo.ulHighQtyTh		= 300*1024*8L;
	tRcInfo.ulLowQtyTh		= 160*1024*8L;
	
	tRcInfo.ubHighBwMaxQp	= 51;
	tRcInfo.ubHighBwMinQp	= 30;	
	tRcInfo.ubMediumBwMaxQp	= 51;
	tRcInfo.ubMediumBwMinQp	= 33;
	tRcInfo.ubLowBwMaxQp	= 51;
	tRcInfo.ubLowBwMinQp	= 36;
	
	RC_FuncSet(&tRcInfo);
}

void RC_QtyAndBwModePresetFor2SlotMode(void)
{
	RC_INFO tRcInfo;
	
	//Max.Bw -> 220KB		
	tRcInfo.ulInitBitRate 	= 110*1024*8L;	//50% MAX.BW @2T1R Mode
	tRcInfo.ulBwBuf			= 20*1024*8L;
	tRcInfo.ubInitFps		= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(KNL_DISP_DUAL_U), VDO_MAIN_V_SIZE(KNL_DISP_DUAL_U));
	tRcInfo.ulHighQtyTh		= 150*1024*8L;
	tRcInfo.ulLowQtyTh		= 80*1024*8L;
	
	tRcInfo.ubHighBwMaxQp	= 51;
	tRcInfo.ubHighBwMinQp	= 30;	
	tRcInfo.ubMediumBwMaxQp	= 51;
	tRcInfo.ubMediumBwMinQp	= 33;
	tRcInfo.ubLowBwMaxQp	= 51;
	tRcInfo.ubLowBwMinQp	= 36;
	
	RC_FuncSet(&tRcInfo);
}

void RC_QtyAndBwModePresetFor4SlotMode(void)
{
	RC_INFO tRcInfo;
	
	//Max.Bw -> 110KB		
	tRcInfo.ulInitBitRate 	= 55*1024*8L;	//50% MAX.BW @4T1R Mode
	tRcInfo.ulBwBuf			= 15*1024*8L;
	tRcInfo.ubInitFps		= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(VDO_DISP_TYPE), VDO_MAIN_V_SIZE(VDO_DISP_TYPE));
	tRcInfo.ulHighQtyTh		= 75*1024*8L;
	tRcInfo.ulLowQtyTh		= 40*1024*8L;
	
	tRcInfo.ubHighBwMaxQp	= 51;
	tRcInfo.ubHighBwMinQp	= 32;	
	tRcInfo.ubMediumBwMaxQp	= 51;
	tRcInfo.ubMediumBwMinQp	= 34;
	tRcInfo.ubLowBwMaxQp	= 51;
	tRcInfo.ubLowBwMinQp	= 37;
	
	RC_FuncSet(&tRcInfo);
}

//void RC_DynamicFpsMode(uint8_t ubMaxFps,uint8_t ubMinFps,uint8_t ubSectionNum)
void RC_DynamicFpsMode(uint8_t ubMaxFps,uint8_t ubMinFps,uint8_t ubSectionNum,float fRatio)
{
	RC_INFO tRcInfo;
	uint8_t i;
	uint32_t ulMaxBw;	//UNIT : Bit
	uint32_t ulMinBw;	//UNIT : Bit
	
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_DYNAMIC_FPS2;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;	
	
	tRcInfo.ubKeySecRatio		= RC_RATIO_75;	
	tRcInfo.ubNonKeySecRatio	= RC_RATIO_75;
	
	tRcInfo.ubMinQp				= 30;
	tRcInfo.ubMaxQp				= 51;	
		
#if RTC676x	//RTC676x
	tRcInfo.ulInitBitRate 		= ulTRX_GetMaxBw(0)*8*(0.5);		//50% MAX.BW
#else		//A7130	
	if(DISPLAY_MODE == DISPLAY_1T1R)
		tRcInfo.ulInitBitRate 		= RC_MAX_BW_A1_SERIES*(0.5);	//50% MAX.BW
	else if(DISPLAY_MODE == DISPLAY_2T1R)
		tRcInfo.ulInitBitRate 		= RC_MAX_BW_A2_SERIES*(0.5);
	else if(DISPLAY_MODE == DISPLAY_4T1R)
		tRcInfo.ulInitBitRate 		= RC_MAX_BW_A4_SERIES*(0.5);
#endif
	tRcInfo.ubInitFps			= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(KNL_DISP_SINGLE), VDO_MAIN_V_SIZE(KNL_DISP_SINGLE));
	
	tRcInfo.ubFpsSectionNum = ubSectionNum;
	
#if RTC676x	//RTC676x	
	//ulMaxBw = ulTRX_GetMaxBw(0)*8;		//Transfer to "Bit"	
	ulMaxBw = ulTRX_GetMaxBw(0)*8*fRatio;	//Transfer to "Bit"		
#else	
	if(DISPLAY_MODE == DISPLAY_1T1R)
		ulMaxBw = RC_MAX_BW_A1_SERIES;
	else if(DISPLAY_MODE == DISPLAY_2T1R)
		ulMaxBw = RC_MAX_BW_A2_SERIES;	
	else if(DISPLAY_MODE == DISPLAY_4T1R)
		ulMaxBw = RC_MAX_BW_A4_SERIES;
#endif	
	ulMinBw = ((ulMaxBw*ubMinFps)/ubMaxFps);

	for(i=0;i<ubSectionNum;i++)
	{
		tRcInfo.ubTargetFps[i]	=  (uint8_t)(((((float)(ubMaxFps-ubMinFps))/(ubSectionNum-1))*i)+ubMinFps);			
		//tRcInfo.ulBwTh[i]		= 	((((ulMaxBw-ulMinBw)/(ubSectionNum-0))*i)+ulMinBw);
		tRcInfo.ulBwTh[i]		= 	((((ulMaxBw-ulMinBw)/(ubSectionNum-1))*i)+ulMinBw);
		if(i == (ubSectionNum-1))
		{
			tRcInfo.ubTargetFps[i] 	= ubMaxFps;			
		}
		//printf("[%d]BW:%d KB,FPS:%d\r\n",i,(tRcInfo.ulBwTh[i]/8092),tRcInfo.ubTargetFps[i]);		
		printf("[%d]B:%d,F:%d\r\n",i,(tRcInfo.ulBwTh[i]/8192),tRcInfo.ubTargetFps[i]);
	}
	printf("[M]B:%d\r\n",ulMaxBw/8192);
	
	tRcInfo.ubHighBwMaxQp		= 51;
	tRcInfo.ubHighBwMinQp		= 30;	
	tRcInfo.ubMediumBwMaxQp		= 51;
	tRcInfo.ubMediumBwMinQp		= 33;
	tRcInfo.ubLowBwMaxQp		= 51;
	tRcInfo.ubLowBwMinQp		= 36;	
	
	if(ubMaxFps != ubMinFps)
	{
		tRcInfo.ubTarMaxQp[0]		= 50;
		tRcInfo.ubTarMaxQp[1]		= 49;
		tRcInfo.ubTarMaxQp[2]		= 48;
		tRcInfo.ubTarMaxQp[3]		= 47;
		tRcInfo.ubTarMaxQp[4]		= 46;
		
		tRcInfo.ubTarMinQp[0]		= 34;
		tRcInfo.ubTarMinQp[1]		= 33;
		tRcInfo.ubTarMinQp[2]		= 32;
		tRcInfo.ubTarMinQp[3]		= 31;
		tRcInfo.ubTarMinQp[4]		= 30;
	}
	else					
	{
		tRcInfo.ubTarMaxQp[0]		= 46;
		tRcInfo.ubTarMaxQp[1]		= 46;
		tRcInfo.ubTarMaxQp[2]		= 46;
		tRcInfo.ubTarMaxQp[3]		= 46;
		tRcInfo.ubTarMaxQp[4]		= 46;
		
		tRcInfo.ubTarMinQp[0]		= 30;
		tRcInfo.ubTarMinQp[1]		= 30;
		tRcInfo.ubTarMinQp[2]		= 30;
		tRcInfo.ubTarMinQp[3]		= 30;
		tRcInfo.ubTarMinQp[4]		= 30;
	}
	
	RC_FuncSet(&tRcInfo);
}
	
void RC_QtyAndBwModePreset(void)
{
	RC_INFO tRcInfo;

	printf("RC_QtyAndBwModePreset\r\n");
	
	
#if A7130
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_DYNAMIC_QTY_AND_BW;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;
	
#if SU5390	
	if(tBB_GetDataRateMode() == BB_2M_DATA_RATE)
	{		
		//printf("  @_@  BB_2M_DATA_RATE\r\n"); //todo
		tRcInfo.ubKeySecRatio		= RC_RATIO_70;		//RC_RATIO_80
		tRcInfo.ubNonKeySecRatio	= RC_RATIO_50;	//RC_RATIO_70
	}
	else
	{
		tRcInfo.ubKeySecRatio		= RC_RATIO_80;
		tRcInfo.ubNonKeySecRatio	= RC_RATIO_70;
	}
#else
	tRcInfo.ubKeySecRatio		= RC_RATIO_80;
	tRcInfo.ubNonKeySecRatio	= RC_RATIO_70;
#endif
	
	tRcInfo.ubMinQp				= 29;
	tRcInfo.ubMaxQp				= 49;		
	
	
	
#if SU5390	
	if(tBB_GetDataRateMode() == BB_2M_DATA_RATE)		
		tRcInfo.ubQpBuf		= 3;	//6  //huang
	else	
		tRcInfo.ubQpBuf		= 3;	//6  //huang
#else
	tRcInfo.ubQpBuf			= 6;
#endif

	if(DISPLAY_MODE == DISPLAY_1T1R)
	{
		if(tBB_GetDataRateMode() == BB_2M_DATA_RATE)
			tRcInfo.ulInitBitRate	= RC_MAX_BW_S1_SERIES/2;	//SU5390 2M Mode
		else
			tRcInfo.ulInitBitRate	= RC_MAX_BW_A1_SERIES/2;	//SU5390 4M or A7130
	}
	if(DISPLAY_MODE == DISPLAY_2T1R)
	{
		if(tBB_GetDataRateMode() == BB_2M_DATA_RATE)
			tRcInfo.ulInitBitRate	= RC_MAX_BW_S2_SERIES/2;	//SU5390 2M Mode
		else
			tRcInfo.ulInitBitRate	= RC_MAX_BW_A2_SERIES/2;	//SU5390 4M or A7130
	}
	if(DISPLAY_MODE == DISPLAY_4T1R)
	{
		if(tBB_GetDataRateMode() == BB_2M_DATA_RATE)
			tRcInfo.ulInitBitRate	= RC_MAX_BW_S4_SERIES/2;	//SU5390 2M Mode
		else
			tRcInfo.ulInitBitRate	= RC_MAX_BW_A4_SERIES/2;	//SU5390 4M or A7130
	}

#if SU5390	
	if(tBB_GetDataRateMode() == BB_2M_DATA_RATE)
		tRcInfo.ulBwBuf	= 20*1024*8L;
	else
		tRcInfo.ulBwBuf	= 30*1024*8L;
#else
	tRcInfo.ulBwBuf		= 30*1024*8L;
#endif
	
#if SU5390	
	if(tBB_GetDataRateMode() == BB_2M_DATA_RATE)
		tRcInfo.ubInitFps	= 12;
	else
		tRcInfo.ubInitFps	= 15;	
#else
	tRcInfo.ubInitFps		= 15;
#endif	
	
	if(DISPLAY_MODE == DISPLAY_1T1R)
	{
		tRcInfo.ulHighQtyTh			= (RC_MAX_BW_A1_SERIES*2)/3;
		tRcInfo.ulLowQtyTh			= (RC_MAX_BW_A1_SERIES*1)/3;	
	}
	else if(DISPLAY_MODE == DISPLAY_2T1R)
	{
		tRcInfo.ulHighQtyTh			= (RC_MAX_BW_A2_SERIES*2)/3;
		tRcInfo.ulLowQtyTh			= (RC_MAX_BW_A2_SERIES*1)/3;	
	}
	else if(DISPLAY_MODE == DISPLAY_4T1R)
	{
		tRcInfo.ulHighQtyTh			= (RC_MAX_BW_A4_SERIES*2)/3;
		tRcInfo.ulLowQtyTh			= (RC_MAX_BW_A4_SERIES*1)/3;	
	}
#if (defined(BSP_VBM_SDK)&&defined(OP_STA)&&defined(VDO_SUBPATH_ENABLE)&&(VDO_SUBPATH_ENABLE!=0))
// Avoid memory size overflow, decrease BB bandwidth and QP. 
	tRcInfo.ubHighBwMaxQp		= 46;		
	tRcInfo.ubHighBwMinQp		= 40;	
	tRcInfo.ubMediumBwMaxQp		= 49;		
	tRcInfo.ubMediumBwMinQp		= 42;
	tRcInfo.ubLowBwMaxQp		= 51;		
	tRcInfo.ubLowBwMinQp		= 45;
#else

#if SU5390
	if(tBB_GetDataRateMode() == BB_2M_DATA_RATE)	
	{
		tRcInfo.ubHighBwMaxQp		= 43+2+0;		//huang
		tRcInfo.ubHighBwMinQp		= 29-1;	
		tRcInfo.ubMediumBwMaxQp		= 46+1;		
		tRcInfo.ubMediumBwMinQp		= 32-1;
		tRcInfo.ubLowBwMaxQp		= 49+1;		
		tRcInfo.ubLowBwMinQp		= 35-1;
	}
	else
	{
		tRcInfo.ubHighBwMaxQp		= 43;
		tRcInfo.ubHighBwMinQp		= 29;	
		tRcInfo.ubMediumBwMaxQp		= 46;
		tRcInfo.ubMediumBwMinQp		= 32;
		tRcInfo.ubLowBwMaxQp		= 49;
		tRcInfo.ubLowBwMinQp		= 35;
	}
#else
	tRcInfo.ubHighBwMaxQp		= 43;		
	tRcInfo.ubHighBwMinQp		= 29;	
	tRcInfo.ubMediumBwMaxQp		= 46;		
	tRcInfo.ubMediumBwMinQp		= 32;
	tRcInfo.ubLowBwMaxQp		= 49;		
	tRcInfo.ubLowBwMinQp		= 35;
#endif
#endif
#endif

	
	
#if RTC676x
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_DYNAMIC_QTY_AND_BW;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;
	tRcInfo.ubKeySecRatio		= RC_RATIO_70;	
	tRcInfo.ubNonKeySecRatio	= RC_RATIO_60;
	tRcInfo.ubMinQp				= 29;
	tRcInfo.ubMaxQp				= 50;
	tRcInfo.ubQpBuf				= 3;

	//printf("  @_@  RTC676x\r\n"); //todo
	
	if(ubKNL_GetTRXSlotNum() == 1)
	{
		//Max.Bw -> 440KB		
		tRcInfo.ulInitBitRate 	= RC_MAX_BW_R1_SERIES/2;
		tRcInfo.ulBwBuf			= 30*1024*8L;
		tRcInfo.ubInitFps		= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(KNL_DISP_SINGLE), VDO_MAIN_V_SIZE(KNL_DISP_SINGLE));
		tRcInfo.ulHighQtyTh		= (RC_MAX_BW_R1_SERIES*2)/3;
		tRcInfo.ulLowQtyTh		= (RC_MAX_BW_R1_SERIES*1)/3;
		
		tRcInfo.ubHighBwMaxQp	= 51;
		tRcInfo.ubHighBwMinQp	= 30;	
		tRcInfo.ubMediumBwMaxQp	= 51;
		tRcInfo.ubMediumBwMinQp	= 33;
		tRcInfo.ubLowBwMaxQp	= 51;
		tRcInfo.ubLowBwMinQp	= 36;
	}
	else if(ubKNL_GetTRXSlotNum() == 2)
	{
		//Max.Bw -> 220KB		
		tRcInfo.ulInitBitRate 	= RC_MAX_BW_R2_SERIES/2;
		tRcInfo.ulBwBuf			= 20*1024*8L;
		tRcInfo.ubInitFps		= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(KNL_DISP_DUAL_U), VDO_MAIN_V_SIZE(KNL_DISP_DUAL_U));
		tRcInfo.ulHighQtyTh		= (RC_MAX_BW_R2_SERIES*2)/3;
		tRcInfo.ulLowQtyTh		= (RC_MAX_BW_R2_SERIES*1)/3;
		
		tRcInfo.ubHighBwMaxQp	= 51;
		tRcInfo.ubHighBwMinQp	= 30;	
		tRcInfo.ubMediumBwMaxQp	= 51;
		tRcInfo.ubMediumBwMinQp	= 33;
		tRcInfo.ubLowBwMaxQp	= 51;
		tRcInfo.ubLowBwMinQp	= 36;
	}	
	else if(ubKNL_GetTRXSlotNum() == 4)
	{
		//Max.Bw -> 110KB		
		tRcInfo.ulInitBitRate 	= RC_MAX_BW_R4_SERIES/2;
		tRcInfo.ulBwBuf			= 15*1024*8L;
		tRcInfo.ubInitFps		= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(VDO_DISP_TYPE), VDO_MAIN_V_SIZE(VDO_DISP_TYPE));
		tRcInfo.ulHighQtyTh		= (RC_MAX_BW_R4_SERIES*2)/3;
		tRcInfo.ulLowQtyTh		= (RC_MAX_BW_R4_SERIES*1)/3;
		
		tRcInfo.ubHighBwMaxQp	= 51;
		tRcInfo.ubHighBwMinQp	= 32;	
		tRcInfo.ubMediumBwMaxQp	= 51;
		tRcInfo.ubMediumBwMinQp	= 34;
		tRcInfo.ubLowBwMaxQp	= 51;
		tRcInfo.ubLowBwMinQp	= 37;
	}	
	else
	{
		//Max.Bw -> 110KB		
		tRcInfo.ulInitBitRate 	= RC_MAX_BW_R4_SERIES/2;
		tRcInfo.ulBwBuf			= 15*1024*8L;
		tRcInfo.ubInitFps		= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(VDO_DISP_TYPE), VDO_MAIN_V_SIZE(VDO_DISP_TYPE));
		tRcInfo.ulHighQtyTh		= (RC_MAX_BW_R4_SERIES*2)/3;
		tRcInfo.ulLowQtyTh		= (RC_MAX_BW_R4_SERIES*1)/3;
		
		tRcInfo.ubHighBwMaxQp	= 51;
		tRcInfo.ubHighBwMinQp	= 32;	
		tRcInfo.ubMediumBwMaxQp	= 51;
		tRcInfo.ubMediumBwMinQp	= 34;
		tRcInfo.ubLowBwMaxQp	= 51;
		tRcInfo.ubLowBwMinQp	= 37;
	}

#endif

	
	//printf(">> HBwMaxQp=%d, HBwMinQp=%d \n", tRcInfo.ubHighBwMaxQp, tRcInfo.ubHighBwMinQp);	//todo
	//printf(">> MBwMaxQp=%d, MBwMinQp=%d \n", tRcInfo.ubMediumBwMaxQp, tRcInfo.ubMediumBwMinQp);	//todo
	//printf(">> LBwMaxQp=%d, LBwMinQp=%d \n", tRcInfo.ubLowBwMaxQp, tRcInfo.ubLowBwMinQp);	//todo
	
	
	printf("ubKeySecRatio[%d], ubNonKeySecRatio[%d]\r\n", tRcInfo.ubKeySecRatio, tRcInfo.ubNonKeySecRatio);
	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------
void RC_QtyThenFpsModePreset(void)
{
	RC_INFO tRcInfo;
	
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_DYNAMIC_QTY_AND_FPS;
	tRcInfo.tAdjSequence		= RC_ADJ_QTY_THEN_FPS;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;
	tRcInfo.ubKeySecRatio		= RC_RATIO_70;
	tRcInfo.ubNonKeySecRatio	= RC_RATIO_50;
	tRcInfo.ubMinQp				= 29;
	tRcInfo.ubMaxQp				= 46+2;		//46	
	tRcInfo.ulInitBitRate		= 0x168000L;	//180KByte
	tRcInfo.ulStepBitRate		= 50*1024*8L;
	tRcInfo.ulBwBuf				= 50*1024*8L;	
	tRcInfo.ubInitFps			= 15;
	tRcInfo.ubMaxFps			= 15;
	tRcInfo.ubMinFps			= 2;	//4
	tRcInfo.ubStepFps			= 2;	//1

	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------
void RC_FpsThenQtyModePreset(void)
{
	RC_INFO tRcInfo;
	
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_DYNAMIC_QTY_AND_FPS;
	tRcInfo.tAdjSequence		= RC_ADJ_FPS_THEN_QTY;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;
	tRcInfo.ubKeySecRatio		= RC_RATIO_70;
	tRcInfo.ubNonKeySecRatio	= RC_RATIO_50;
	tRcInfo.ubMinQp				= 29;
	tRcInfo.ubMaxQp				= 46;
	tRcInfo.ubTargetQp			= 41;
	tRcInfo.ubQpBuf				= 2;
	tRcInfo.ulInitBitRate		= 0x168000L; //180KByte
	tRcInfo.ulStepBitRate		= 50*1024*8L;
	tRcInfo.ulBwBuf				= 50*1024*8L;
	tRcInfo.ulBwTh[0]			=  30*1024*8L;
	tRcInfo.ulBwTh[1]			=  60*1024*8L;
	tRcInfo.ulBwTh[2]			=  90*1024*8L;
	tRcInfo.ulBwTh[3]			= 120*1024*8L;
	tRcInfo.ulBwTh[4]			= 150*1024*8L;
	tRcInfo.ulBwTh[5]			= 180*1024*8L;
	tRcInfo.ulBwTh[6]			= 210*1024*8L;
	tRcInfo.ulBwTh[7]			= 240*1024*8L;
	tRcInfo.ulBwTh[8]			= 270*1024*8L;
	tRcInfo.ubInitFps			= 15;
	tRcInfo.ubMaxFps			= 15;
	tRcInfo.ubMinFps			= 5;
	tRcInfo.ubStepFps			= 2;
	tRcInfo.ubFpsBuf			= 7;
	tRcInfo.ubTargetFps[0]		= 4;
	tRcInfo.ubTargetFps[1]		= 5;
	tRcInfo.ubTargetFps[2]		= 6;
	tRcInfo.ubTargetFps[3]		= 7;
	tRcInfo.ubTargetFps[4]		= 9;
	tRcInfo.ubTargetFps[5]		= 11;
	tRcInfo.ubTargetFps[6]		= 12;
	tRcInfo.ubTargetFps[7]		= 13;
	tRcInfo.ubTargetFps[8]		= 14;
	tRcInfo.ubTargetFps[9]		= 15;

	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------
void RC_DynamicFpsModePreset(void)
{
#ifdef A7130
	KNL_DISP_TYPE tDispType;
	tDispType = tDispType;
	
	tDispType = tKNL_GetDispType();		
	RC_DynamicFpsMode(VDO_FRAME_RATE(VDO_MAIN_H_SIZE(tDispType), VDO_MAIN_V_SIZE(tDispType)), RC_MIN_FPS, RC_SECTION_NUM,1);
#endif	
	
#ifdef RTC676x
	KNL_DISP_TYPE tDispType;

	tDispType = (ubKNL_GetTRXSlotNum() == 1)?KNL_DISP_SINGLE:
				(ubKNL_GetTRXSlotNum() == 2)?KNL_DISP_DUAL_U:VDO_DISP_TYPE;
	RC_DynamicFpsMode(VDO_FRAME_RATE(VDO_MAIN_H_SIZE(tDispType), VDO_MAIN_V_SIZE(tDispType)), RC_MIN_FPS, RC_SECTION_NUM,1);
#endif
}
//------------------------------------------------------------------------------
void RC_QtyAndDynaFpsModePreset(void)
{
#if (defined(OP_STA) && defined(S2019A))
	RC_INFO tRcInfo;
	uint8_t ubRcIdx = 1;

	ubRC_TxTarget = 2;
	ubRC_CurFps	  = 5;
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_QTY_AND_DYNAFPS;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;
	tRcInfo.ubKeySecRatio		= RC_RATIO_70;
	tRcInfo.ubNonKeySecRatio	= RC_RATIO_60;
	tRcInfo.ubMinQp				= 29;
	tRcInfo.ubMaxQp				= 45;
	tRcInfo.ubQpBuf				= 6;
#define RC_MAX_BW_W1_SERIES		(520*1024*8L)	//! Bit-Rate
	tRcInfo.ulInitBitRate 		= RC_MAX_BW_W1_SERIES/2;
	tRcInfo.ulBwBuf				= 30*1024*8L;
	tRcInfo.ubInitFps			= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(VDO_DISP_TYPE), VDO_MAIN_V_SIZE(VDO_DISP_TYPE));
	tRcInfo.ulHighQtyTh			= (RC_MAX_BW_W1_SERIES*2)/3;
	tRcInfo.ulLowQtyTh			= (RC_MAX_BW_W1_SERIES*1)/3;
	tRcInfo.ubHighBwMaxQp		= 45;
	tRcInfo.ubHighBwMinQp		= 25;
	tRcInfo.ubMediumBwMaxQp		= 45;
	tRcInfo.ubMediumBwMinQp		= 32;
	tRcInfo.ubLowBwMaxQp		= 45;
	tRcInfo.ubLowBwMinQp		= 35;
	RC_UpdateTrxBufParam(BUF_NUM_VDO_BS);
	RC_UpdateDynaFpsParam(tRcInfo.ubInitFps, RC_MIN_FPS);
	ubRC_FinalMaxQp[0] 			= 45;
	ubRC_FinalMinQp[0] 			= 35;
	ulRC_FinalBitRate[0]		= tRcInfo.ulInitBitRate;
	RC_FuncSet(&tRcInfo);
	for(ubRcIdx = 1; ubRcIdx < 4; ubRcIdx++)
	{
		tRcInfo.ubEnableFlg 	   = 0;
		tRcInfo.ubCodecIdx  	   = ubRcIdx;
		ubRC_FinalMaxQp[ubRcIdx]   = 45;
		ubRC_FinalMinQp[ubRcIdx]   = 35;
		ulRC_FinalBitRate[ubRcIdx] = tRcInfo.ulInitBitRate;
		RC_FuncSet(&tRcInfo);
	}
#endif
}
//------------------------------------------------------------------------------
void RC_DtDynaQtyModePreset(void)
{
#if (defined(OP_STA) && defined(S2019A) && defined(RVCS_APP))
	RC_INFO tRcInfo;
	uint8_t ubRcIdx = 1;

	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_DT_DYNAQTYFPS;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;
	tRcInfo.ulInitBitRate 		= RC_DT_MAX_BIT_RATE;
	tRcInfo.ulBwBuf				= 30*1024*8L;
	tRcInfo.ubInitFps			= VDO_FRAME_RATE(VDO_MAIN_H_SIZE(VDO_DISP_TYPE), VDO_MAIN_V_SIZE(VDO_DISP_TYPE));
#if (APP_BLE_SN9380_FUNC_ENABLE == 1)
	tRcInfo.ubMinQp				= E_VDO_MAX_QP-3;
	tRcInfo.ubMaxQp				= E_VDO_MAX_QP;			
	ubRC_FinalMaxQp[0] 			= E_VDO_MAX_QP;
	ubRC_FinalMinQp[0] 			= E_VDO_MAX_QP-3;
	tRcInfo.ulInitBitRate 		= 80*1024;
#else
	tRcInfo.ubMinQp				= E_VDO_MIN_QP;
	tRcInfo.ubMaxQp				= E_VDO_MAX_QP;			
	ubRC_FinalMaxQp[0] 			= E_VDO_MAX_QP;
	ubRC_FinalMinQp[0] 			= E_VDO_MIN_QP;
#endif	
	ulRC_FinalBitRate[0]		= tRcInfo.ulInitBitRate;
	RC_FuncSet(&tRcInfo);
	for(ubRcIdx = 1; ubRcIdx < 4; ubRcIdx++)
	{
		tRcInfo.ubEnableFlg 	   = 0;
		tRcInfo.ubCodecIdx  	   = ubRcIdx;
		ubRC_FinalMaxQp[ubRcIdx]   = E_VDO_MAX_QP;
		ubRC_FinalMinQp[ubRcIdx]   = E_VDO_MIN_QP;
		ulRC_FinalBitRate[ubRcIdx] = tRcInfo.ulInitBitRate;
		RC_FuncSet(&tRcInfo);
	}
#endif
}
//------------------------------------------------------------------------------
void RC_PresetSetup(RC_Rreset_t tRC_Preset)
{
#ifdef OP_STA
	RC_PresetSetup_t tRC_PresetFunc[RC_PRESET_MAX] =
	{		
		[RC_NONE]				= RC_NoneModePreset,
		[RC_FIXED_DATA_RATE] 	= RC_FixedDataRateModePreset,
		[RC_QTY_AND_BW] 		= RC_QtyAndBwModePreset,
		[RC_QTY_THEN_FPS] 		= RC_QtyThenFpsModePreset,
		[RC_FPS_THEN_QTY] 		= RC_FpsThenQtyModePreset,
		[RC_DYNAMIC_FPS]		= RC_DynamicFpsModePreset,
		[RC_QTY_AND_DYNAFPS]	= RC_QtyAndDynaFpsModePreset,
		[RC_DT_DYNAQTYFPS]		= RC_DtDynaQtyModePreset,
	};

	if(tRC_PresetFunc[tRC_Preset].pvPresetFunc)
		tRC_PresetFunc[tRC_Preset].pvPresetFunc();
#endif
}
//------------------------------------------------------------------------------
#if (defined(OP_STA) || defined(BSP_DVR_SDK))
void RC_EnUvcPresetMode(uint8_t ubEn, uint8_t ubCodecIdx)
{
#define FPS_UVC_MODE	30
#define GOP_UVC_MODE	3 * FPS_UVC_MODE
	if(!ubEn)
	{
		SEN_SetFrameRate(SENSOR_PATH1, ubKNL_GetVdoFps());
		H264_SetGOP((H264_ENCODE_INDEX)ubCodecIdx, ulKNL_GetVdoGop());
		RC_SetUpdateFlg(ubCodecIdx, 1);
		ubRC_SetFlg(ubCodecIdx, TRUE);
	}
	else
	{
		ubRC_SetFlg(ubCodecIdx, FALSE);
		SetH264Rate(ISP_WIDTH, ISP_HEIGHT, FPS_UVC_MODE);
		SEN_SetFrameRate(SENSOR_PATH1, FPS_UVC_MODE);
		H264_SetGOP((H264_ENCODE_INDEX)ubCodecIdx, GOP_UVC_MODE);
		H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx, 35);
		H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx, 28);
		IQ_SetAEPID(((FPS_UVC_MODE <= 5)?1:0));
		H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 0x3E8000, FPS_UVC_MODE);
	}
}
//------------------------------------------------------------------------------
void RC_EnHighQualityPresetMode(uint8_t ubEn, uint8_t ubCodecIdx)
{
#define RC_HQ_MAX_QP	25
#define RC_HQ_MIN_QP	20
	if(!ubEn)
	{
		if(ubRC_GetFlg(ubCodecIdx))
			return;
		H264_ResetRateControl(tRC_Info[ubCodecIdx].ubMinQp);
		H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,ulRC_GetFinalBitRate(tRC_Info[0].ubCodecIdx),ubKNL_GetVdoFps());
		H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMaxQp);
		H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMinQp);
		RC_SetUpdateFlg(ubCodecIdx, 1);
		ubRC_SetFlg(ubCodecIdx, TRUE);
	}
	else
	{
		if(!ubRC_GetFlg(ubCodecIdx))
			return;
		ubRC_SetFlg(ubCodecIdx, FALSE);
		H264_ResetRateControl(RC_HQ_MIN_QP);
		H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx, 0x320000, ubKNL_GetVdoFps());
		H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx, RC_HQ_MAX_QP);
		H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx, RC_HQ_MIN_QP);
	}
}
#endif
