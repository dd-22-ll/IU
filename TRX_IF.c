/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		TRX_IF.c
	\brief		Transmit/Receive Interface
	\author		Justin
	\version	0.4
	\date		2020/05/04
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <MATH.H>
#include <STDARG.H>
#include <ABSACC.H>
#include <STRING.H>
#include <CTYPE.H>
#include <STDLIB.H>
#include "TRX_IF.h"
#include "_510PF.h"
#include "KNL.h"

#if RTC676x
#include "rwrf.h"
#endif

uint8_t ubTRX_GetLinkStatus(uint8_t ubRole)
{
#if defined(A7130) || defined(RTC676x)
#if A7130
	return ubKNL_GetRtCommLinkStatus(ubRole);
#endif
	
#if RTC676x
	#if OP_AP
		ubRole = ubKNL_GetStaInfo(KNL_ROLENUM, ubRole);
		if(ubRole == KNL_STA1)
		{
			return rf_get_link_status(rf_get_remote_id(0));
		}
		else if(ubRole == KNL_STA2)
		{
			return rf_get_link_status(rf_get_remote_id(1));
		}
		else if(ubRole == KNL_STA3)
		{
			return rf_get_link_status(rf_get_remote_id(2));
		}
		else if(ubRole == KNL_STA4)
		{
			return rf_get_link_status(rf_get_remote_id(3));
		}
		else
		{
			return 0;
		}
	#endif
	#if OP_STA
		return rf_get_link_status(rf_get_remote_id(0));
	#endif
#endif
#else
	return 1;
#endif	
}

//Unit : Byte
//ubRole is used @OP_AP
uint32_t ulTRX_GetRtBw(uint8_t ubRole)	
{
#if (defined(A7130))
	#if OP_AP
		if(ubTRX_GetLinkStatus(ubRole))
		{		
			return (20*1024);
		}
		else
		{
			return 0;
		}
	#endif
	#if OP_STA
		if(ubTRX_GetLinkStatus(KNL_MASTER_AP))
		{
			return ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW);
		}
		else
		{
			return 0;
		}
	#endif
#elif (defined(RTC676x))
	#if OP_AP
		if(ubTRX_GetLinkStatus(ubRole))
		{			
			return ulRTC676x_GetRtBw(ubRole);
		}
		else
		{
			return 0;
		}
	#endif
	#if OP_STA
		if(ubTRX_GetLinkStatus(0))
		{
			return ulRTC676x_GetRtBw(0);
		}
		else
		{
			return 0;
		}
	#endif
#else
	return 0;
#endif
}

//Unit : Byte
uint32_t ulTRX_GetMaxBw(uint8_t ubRole)		//Used for Richwave RTC676x
{
#if (defined(A7130))
	#if OP_STA
	BB_GET_DATA_FLOW tDataFlow;
	#endif
	
	#if OP_AP	
		return (20*1024);
	#endif
	#if OP_STA		
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
		return ulBB_GetBBFlow(tDataFlow);	
	#endif
#elif (defined(RTC676x))
	#if OP_AP
		return (20*1024);
	#endif
	#if OP_STA
		return ulRTC676x_GetMaxBw();
	#endif
#else
	return 0;
#endif
}
