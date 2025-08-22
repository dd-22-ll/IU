/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_CFG_SEL.h
	\brief		APP Configuration header file
	\author		Pierce
	\version	0.6
	\date		2020/06/24
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _APP_CFG_SEL_H_
#define _APP_CFG_SEL_H_

//------------------------------------------------------------------------------
//! <<< Use Configuration Wizard in Context Menu >>>
#define APP_FS_MEDIA0			0
#define APP_FS_MEDIA1			1
#define APP_FS_MEDIA_TYPE_SD		0
#define APP_FS_MEDIA_TYPE_SF		1

//------------------------------------------------------------------------------
#if defined(VBM_PU)

//! <h> VBM PU

//! <e> SD Card
#define APP_SD_FUNC_ENABLE				0
#if APP_SD_FUNC_ENABLE
//! <q> Recording Video		
#define APP_REC_FUNC_ENABLE				1
#if APP_REC_FUNC_ENABLE
#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SD
#endif
#else
#define APP_REC_FUNC_ENABLE				0
#endif
//! </e>

//! <e> Capturing Photo
#define APP_PHOTOGRAPH_FUNC_ENABLE		0
//! <ol> Stored in				<0=> SD Card
//!    							<1=> Serial Flash
#define APP_PHOTO_STORE_SEL				0
//! </e>

#if APP_REC_FUNC_ENABLE 
	#if (APP_PHOTOGRAPH_FUNC_ENABLE && APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SF)	
		#define APP_DUAL_FS_ENABLE				1
		#define APP_FS_MEDIA1_TYPE				APP_FS_MEDIA_TYPE_SF
	#else
		#define APP_DUAL_FS_ENABLE				0		
	#endif
#elif APP_SD_FUNC_ENABLE
	#if (APP_PHOTOGRAPH_FUNC_ENABLE && APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SF)
		#define APP_DUAL_FS_ENABLE				1		
		#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SF
		#define APP_FS_MEDIA1_TYPE				APP_FS_MEDIA_TYPE_SD	
	#else
		#define APP_DUAL_FS_ENABLE				0		
		#define APP_FS_MEDIA0_TYPE				APP_PHOTO_STORE_SEL
	#endif
#else
	#define APP_DUAL_FS_ENABLE				0
	#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SF
#endif	
//! </h>

#else
#include "bsp_select.h"
//! <h> VBM BU
#if BSP_SD_CARD
//! <e> SD Card	
#define APP_SD_FUNC_ENABLE				0
#else
#define APP_SD_FUNC_ENABLE				0
#endif
#if APP_SD_FUNC_ENABLE
//! <e> Recording Video		
#define APP_REC_FUNC_ENABLE				1
#if APP_REC_FUNC_ENABLE
#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SD
#endif
#else
#define APP_REC_FUNC_ENABLE				0
#endif

#if APP_REC_FUNC_ENABLE
//! <ol> Tx Record Stream Select		<1=> Sub Stream
//! <i>  Becuase display mode change will initial memory, (Main) or (Main+Sub) Stream no Supported. 
#define APP_TXREC_STREAM_SEL		    1
#else
#define APP_TXREC_STREAM_SEL		    0
#endif
//! </e>
//! </e>

//! <e> Capturing Photo
#define APP_PHOTOGRAPH_FUNC_ENABLE		0
//! <ol> Stored in				<0=> SD Card
//!    							<1=> Serial Flash
#define APP_PHOTO_STORE_SEL				1
//! </e>

#if APP_REC_FUNC_ENABLE 
	#if (APP_PHOTOGRAPH_FUNC_ENABLE && APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SF)	
		#define APP_DUAL_FS_ENABLE				1
		#define APP_FS_MEDIA1_TYPE				APP_FS_MEDIA_TYPE_SF
	#else
		#define APP_DUAL_FS_ENABLE				0		
	#endif
#elif APP_SD_FUNC_ENABLE
	#if (APP_PHOTOGRAPH_FUNC_ENABLE && APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SF)
		#define APP_DUAL_FS_ENABLE				1		
		#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SF
		#define APP_FS_MEDIA1_TYPE				APP_FS_MEDIA_TYPE_SD	
	#else
		#define APP_DUAL_FS_ENABLE				0		
		#define APP_FS_MEDIA0_TYPE				APP_PHOTO_STORE_SEL
	#endif
#else
	#define APP_DUAL_FS_ENABLE				0
	#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SF
#endif	
//! </h>

#define APP_AUTO_PAIR					0
#define APP_AUTO_PAIR_MODE				0
#define APP_UVC_CAM_ENABLE              0
#endif
#define APP_DUAL_HOST_ENABLE            0

//! APP Display mode
#define	DISPLAY_1T1R  					1
#define	DISPLAY_2T1R  					2
#define	DISPLAY_4T1R  					4
//! Display Mode
//! <ol> Display Mode			<1=> Display 1T1R
//!    							<2=> Display 2T1R
//!    							<4=> Display 4T1R
#define DISPLAY_MODE					1






//! APP Audio Function Setting
//! <e> Audio
#define APP_ADO_FUNC_ENABLE				1
//! <q> Recoding Mixer Audio
#define APP_REC_FUNC_ADO_MIX_ENABLE		0
//! </e>

//! Baseband timing setting

//! <h> RF
//! <ol> A7130 RF Timing 		<0=> Video First
//!    							<1=> Audio First
//!    							<2=> Video Only
//! <i> When audio disable the A7130 RF timing fixes Video Only
	#define APP_BB_TIMING_SET			2	//0 //huang
	
	
//! Hopping channel number
//! RF Channel Number
//! <ol> Channel Number			<1=> 19 channels
//!    							<0=> 20 channels

#define APP_RF_HOPPING_CHANNEL_NUMBER    1
	
	
	
	
//! </h>


//! <h> File System
//! <ol> File System Path		<0=> \DCIM\100SONIX ~ \DCIM\999SONIX
//!    							<1=> \DCIM\VIDEO & \DCIM\PHOTO
#define APP_FS_CUSTOMER1_PATH_ENABLE	0	//customer recording folder at E:\DCIM\VIDEO and E:\DCIM\PHOTO

//! <ol> File Name				<0=> VDO_20200413_183030.MP4 & JPG_20200413_183030.JPG
//!    							<1=> SNX_0001 ~ SNX_9999
#define APP_FS_ROLLING_FILE_NAME		1
//! </h>

#ifdef OP_AP
//! <h> File list style
//! <ol> Type					<0=> original(words only)
//!								<1=> thumbnail
#if APP_REC_FUNC_ENABLE
#define APP_FS_FILE_LIST_STYLE        0
#else
#define APP_FS_FILE_LIST_STYLE        0
#endif
//! </h>
#endif
#ifdef OP_STA
#define APP_FS_FILE_LIST_STYLE        0
#endif

#if (BSP_RTC_TIMER_SEL == RTC_TIMER_NULL)
#define APP_TIMESTAMP_FUNC_ENABLE		0
#else
//! <q> Time Stamp				
#define APP_TIMESTAMP_FUNC_ENABLE		1
//! <i> When RTC timer is NULL the Time Stamp is disable
#define APP_MAC_FUNC_ENABLE				0
#endif

//! <q> Play Remote
#define APP_PLAY_REMOTE_ENABLE          0
//! <i> Support Remote Cam play function

//! <h> Command Line
//! <q> CLI Login function
#define APP_CFG_ENABLE_LOGIN			0
//! <i> Enable command line login function or not
//! </h>

//! <q> PC Connect
#define APP_PC_CONNECT_EN				0

//! APP Preview Mode
#define	DT_DISPLAY_VGA  				1
#define	DT_DISPLAY_HD  					2
#define	DT_DISPLAY_FHD  				3
//! Preview resolution
//! <ol> APP Preview Mode			<1=> Display VGA
//!    							<2=> Display HD 
//!    							<3=> Display FHD
#define DT_DISPLAY_MODE				2 
//! <<< end of configuration section >>>

#if (!APP_PHOTOGRAPH_FUNC_ENABLE && !APP_REC_FUNC_ENABLE)
#define APP_RAISE_OSHEAP_SIZE			(0)//(982*1024) // 000
#elif (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
#define APP_RAISE_OSHEAP_SIZE			(1456*1024)
#elif (!APP_DUAL_FS_ENABLE && APP_REC_FUNC_ENABLE)
#define APP_RAISE_OSHEAP_SIZE			(855*1024) // 010,110
#elif (!APP_DUAL_FS_ENABLE)
#define APP_RAISE_OSHEAP_SIZE			(475*1024) // 100
#elif (!APP_REC_FUNC_ENABLE)
#define APP_RAISE_OSHEAP_SIZE			(818*1024) // 101
#else
#define APP_RAISE_OSHEAP_SIZE			(982*1024) // 111
#endif
//------------------------------------------------------------------------------
//! 510PF_VBM_PU_A7130
#if (defined(VBM_PU) && defined(A7130))
#define osHeapSize               (2208*1024) + APP_RAISE_OSHEAP_SIZE
#define CODE_MAX_SIZE			 (0x00160000)				//!< I-Cache Max Size	
#define DATA_MAX_SIZE			 (0x00127000)				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap

//------------------------------------------------------------------------------
//! 510PF_VBM_BU_A7130
#elif (defined(VBM_BU) && defined(A7130))
#define osHeapSize				 (2048*1024) + APP_RAISE_OSHEAP_SIZE
#define CODE_MAX_SIZE			 (0x0014C000)				//!< I-Cache Max Size	
#define DATA_MAX_SIZE			 (0x00118000)				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap
//------------------------------------------------------------------------------
//! 510PF_VBM_PU_S2019A
#elif (defined(VBM_PU) && defined(S2019A))
#define osHeapSize               (2208*1024) + APP_RAISE_OSHEAP_SIZE
#define CODE_MAX_SIZE			 (0x00180000)				//!< I-Cache Max Size	
#define DATA_MAX_SIZE			 (0x00140000)				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap
//------------------------------------------------------------------------------
//! 510PF_VBM_BU_S2019A
#elif (defined(VBM_BU) && defined(S2019A))
#define osHeapSize				 (2048*1024) + APP_RAISE_OSHEAP_SIZE
#define CODE_MAX_SIZE			 (0x00180000)				//!< I-Cache Max Size	
#define DATA_MAX_SIZE			 (0x00130000)				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap
#endif
#endif
