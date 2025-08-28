/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		TRX_IF.h
	\brief		Transmit/Receive Interface API header
	\author		Justin
	\version	0.2
	\date		2019/05/02
	\copyright	Copyright(C) 2019 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------
#ifndef __TRX_IF_H
#define __TRX_IF_H

#include <stdint.h>

uint8_t ubTRX_GetLinkStatus(uint8_t ubRole);
uint32_t ulTRX_GetRtBw(uint8_t ubRole);
uint32_t ulTRX_GetMaxBw(uint8_t ubRole);
	
#endif


