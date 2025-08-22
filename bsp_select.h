/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_select.h
	\brief		BSP Select header file
	\author		Wales
	\version	0.6
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_SELECT_H_
#define _BSP_SELECT_H_

//------------------------------------------------------------------------------
// Board Supported List
//------------------------------------------------------------------------------

// VBM PU
#define SN93711_FHD_REC_RX_V4       1                       //! PU Board #1, SN93711_FHD_REC_RX_V3,SN93711_FHD_REC_RX_V4
#define D_SN93701_SSD2828_RX_V5     2                       //! PU Board #2
#define D_SN93701_TC358778_RX_V6    3                       //! PU Board #3
#define D_SN93703_TC358778_RX_V6    4                       //! PU Board #4

// VBM_BU
#define SN93710_FHD_REC_TX_V4_1     1                       //! BU  Board #1
#define D_SN93712_VBM_TX_V2         2                       //! BU  Board #2
#define D_SNCC72_TX_V1              3				        //! BU  Board #3
#define D_SN93714_TX_V1		        4                       //! BU  Board #4
#define D_SN93716_TX_V1		        5                       //! BU  Board #5

//! <<< Use Configuration Wizard in Context Menu >>>
//! <h> Board Part Number
//------------------------------------------------------------------------------
//! Select Board
//------------------------------------------------------------------------------
//! Select PU Board
//! <ol> PU Board Select			<1=> SN93711_FHD_REC_RX_V4     (HD LCD Panel)
//!    								<2=> D_SN93701_SSD2828_RX_V5   (HD LCD Panel)
//!    								<3=> D_SN93701_TC358778_RX_V6  (HD LCD Panel)
//!    								<4=> D_SN93703_TC358778_RX_V6  (HD LCD Panel)
#define PU_BOARD_SELECT        4

//! Select BU Board
//! <ol> BU Board Select			<1=> SN93710_FHD_REC_TX_V4_1     
//!    								<2=> D_SN93712_VBM_TX_V2 
//!    								<3=> D_SNCC72_TX_V1
//!    								<4=> D_SN93714_TX_V1
//!    								<5=> D_SN93716_TX_V1
#define BU_BOARD_SELECT       2
//!	</h>

//------------------------------------------------------------------------------
//! IC Supported List
//------------------------------------------------------------------------------
#define SN93701                0xC0            			//!< PU IC 	(32M)
#define SNCC71				   0xC1            			//!< PU IC  (64M)
#define SN93703                0xC2            			//!< PU IC 	(16M)
#define SN93712                0x10            			//!< BU IC  (16M)
#define SNCC72                 0x11            			//!< BU IC  (16M)
#define SN93700                0x20            			//!< BU IC  (32M)
#define SN93714                0x21            			//!< BU IC  (32M)
#define SN93716                0x22            			//!< BU IC  (32M)

//! <h> IC Part Number
//------------------------------------------------------------------------------
//! Select IC
//------------------------------------------------------------------------------
//! <ol> PU IC Select				<0xC0=> SN93701
//!    								<0xC1=> SNCC71
//!    								<0xC2=> SN93703
#define PU_IC_SELECT		   0xC2

//! <ol> BU IC Select				<0x10=> SN93712
//!    								<0x11=> SNCC72
//!    								<0x20=> SN93700
//!    								<0x21=> SN93714
//!    								<0x22=> SN93716
#define BU_IC_SELECT		   0x10
//! </h
//! <<< end of configuration section >>>
#define BSP_VBM_SDK
//------------------------------------------------------------------------------
//! PU Board Config

#ifdef VBM_PU

#ifdef  _510PF_EVB_
#define BSP_BOARD_VBMPU_EV
#endif

#ifdef  _510PF_SDK_
#if (PU_BOARD_SELECT == SN93711_FHD_REC_RX_V4)
#define BSP_SN93711_FHD_REC_RX_V4
#endif

#if (PU_BOARD_SELECT == D_SN93701_SSD2828_RX_V5)
#define BSP_D_SN93701_SSD2828_RX_V5
#endif

#if (PU_BOARD_SELECT == D_SN93701_TC358778_RX_V6)
#define BSP_D_SN93701_TC358778_RX_V6
#endif

#if (PU_BOARD_SELECT == D_SN93703_TC358778_RX_V6)
#define BSP_D_SN93703_TC358778_RX_V6
#endif

#if (PU_IC_SELECT == SN93701)
#define BSP_DDRSIZE	(32)
#endif

#if (PU_IC_SELECT == SN93703)
#define BSP_DDRSIZE	(16)
#endif

#if (PU_IC_SELECT == SNCC71)
#define BSP_DDRSIZE	(64)
#endif

#ifndef BSP_DDRSIZE
#define BSP_DDRSIZE	(32)
#endif
#endif  //! End of #ifdef _510PF_SDK_

#endif  //! End of #ifdef VBM_PU

//------------------------------------------------------------------------------
//! BU Board Config 
#ifdef VBM_BU

#ifdef  _510PF_EVB_
#define BSP_BOARD_VBMBU_EV
#endif

#ifdef  _510PF_SDK_
#if (BU_BOARD_SELECT == SN93710_FHD_REC_TX_V4_1)
#define BSP_SN93710_FHD_REC_TX_V4_1         	//!< SN937X0 Demo board
#define BSP_SD_CARD 1
#endif
#if (BU_BOARD_SELECT == D_SN93712_VBM_TX_V2)
#define BSP_D_SN93712_VBM_TX_V2               	//!< SN93712 Demo board
#define BSP_SD_CARD 0
#endif
#if (BU_BOARD_SELECT == D_SNCC72_TX_V1)
#define BSP_D_SNCC72_TX_V1
#define BSP_SD_CARD 0
#endif
#if (BU_BOARD_SELECT == D_SN93714_TX_V1)
#define BSP_D_SN93714_TX_V1         			//!< SN93714 Demo board
#define BSP_SD_CARD 1
#endif
#if (BU_BOARD_SELECT == D_SN93716_TX_V1)
#define BSP_D_SN93716_TX_V1         			//!< SN93716 Demo board
#define BSP_SD_CARD 1
#endif

#if (BU_IC_SELECT == SN93712)
#define BSP_DDRSIZE	(16)
#endif
#if (BU_IC_SELECT == SNCC72)
#define BSP_DDRSIZE	(16)
#endif
#if (BU_IC_SELECT == SN93700)
#define BSP_DDRSIZE	(32)
#endif

#if (BU_IC_SELECT == SN93714)
#define BSP_DDRSIZE	(32)
#endif

#if (BU_IC_SELECT == SN93716)
#define BSP_DDRSIZE	(32)
#endif

#ifndef BSP_DDRSIZE
#define BSP_DDRSIZE	(32)
#endif
#endif	//! End of #ifdef  _510PF_SDK_

#endif	//! End of #ifdef VBM_BU

#endif	//!< End of #ifndef _BSP_SELECT_H_

