/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		OSD1Image_Table.h
	\brief		OSD1 Image Table
	\author		Hanyi Chiu
	\version	1
	\date		2016/09/20
	\copyright	Copyright (C) 2016 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

//!           Index 	  	  		  SF Address     Pos X     Pos Y
OSD1IMGPOOL(OSD1IMAGE,        			0,        	   0,        0)
OSD1IMGPOOL(MENU,        				0x1000,        0,        0)
OSD1IMGPOOL(SUBMENU,     				0x2000,        0,        0)
OSD1IMGPOOL(HDSTATUSMASK, 				0x2000,        0,        0)
OSD1IMGPOOL(QUALSTATUSMASK, 			0x2000,        0,        0)
OSD1IMGPOOL(MDROWLINE, 					0x2000,        0,        0)
OSD1IMGPOOL(MDCOLUMNLINE, 				0x2000,        0,        0)
OSD1IMGPOOL(RECPLAYLISTBG, 				0x2000,        0,      620)
OSD1IMGPOOL(FWUSTARTBG, 				0x2000,        0,      620)
OSD1IMGPOOL(APPPAIRBG_ICON,				0x2000,        0,        0)
OSD1IMGPOOL(TRIP3COL1STATUSMASK_ICON,	0x2000,        0,        0)
OSD1IMGPOOL(TRIP3COL2STATUSMASK_ICON,	0x2000,        0,        0)
//! DT
OSD1IMGPOOL(WIFIDTBG_ICON,				0x2000,        0,        0)
OSD1IMGPOOL(WIFIDTENY_BG,				0x2000,        0,        0)
#undef OSD1IMGPOOL
