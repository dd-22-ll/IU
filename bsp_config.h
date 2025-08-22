/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_config.h
	\brief		BSP Config header file
	\author		Wales Wang
	\version	0.7
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_CONFIG_H_
#define _BSP_CONFIG_H_

//------------------------------------------------------------------------------
#include "bsp_select.h"

//! BSP RTC Time Definition
#define RTC_TIMER_INTERNAL				0
#define RTC_TIMER_EXTERNAL				1
#define RTC_TIMER_NULL					2
//! BSP Audio aec/nr process by hw/sw define
#define AEC_NR_SW						0
#define AEC_NR_HW						1
//------------------------------------------------------------------------------
//! Include CU Board Config File 
#ifdef VBM_PU
#ifdef  _510PF_SDK_
#include "bsp_SN93711_FHD_REC_RX.h"
#endif  //! End of #ifdef _510PF_SDK_
#endif  //! End of #ifdef VBM_PU
//------------------------------------------------------------------------------
//! Include CAM Board Config file 
#ifdef VBM_BU

#ifdef  _510PF_SDK_
#if (BU_BOARD_SELECT == SN93710_FHD_REC_TX_V4_1)
#include "bsp_SN93710_FHD_REC_TX.h"
#endif
#if (BU_BOARD_SELECT == D_SN93712_VBM_TX_V2)
#include "bsp_SN93712_VBM_TX.h"
#endif
#if (BU_BOARD_SELECT == D_SNCC72_TX_V1)
#include "bsp_SNCC72_TX.h"
#endif
#if (BU_BOARD_SELECT == D_SN93714_TX_V1)
#include "bsp_SN93714_TX.h"
#endif
#if (BU_BOARD_SELECT == D_SN93716_TX_V1)
#include "bsp_SN93716_TX.h"
#endif
#endif	//! End of #ifdef  _510PF_SDK_

#endif	//! End of #ifdef VBM_BU
#endif	//! End of #ifndef _BSP_CONFIG_H_

