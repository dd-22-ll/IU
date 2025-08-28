/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		KNL.h
	\brief		Kernel Control header file
	\author		Justin Chen
	\version	2.20
	\date		2024/03/25
	\copyright	Copyright(C) 2021 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __KNL_h
#define __KNL_h

#include <stdio.h>
#include "_510PF.h"
#include "APP_CFG.h"
#include "BB_API.h"
#include "IMG_API.h"
#include "JPEG_API.h"
#include "TWC_API.h"
#include "H264_API.h"
#include "ADO_API.h"
#include "FS_API.h"
#include "REC_API.h"
#include "PLY_API.h"
#include "sPRF_API.h"
#ifdef RTC676x
#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE
#define FREQ_SEL_DBG_EN						0

#define FREQ_MID_FOR_NUM2					2441	//MHz
#define FREQ_P1_FOR_NUM3					2427	//HHz
#define FRQE_P2_FOR_NUM3					2454	//MHz

#define FREQ_TABLE_NUM						2	//Freq-Table[0] & Freq-Table[2]
//#define FREQ_TABLE_NUM					3	//Freq-Table[0] & Freq-Table[1]	& Freq-Table[2]

#define TRY_FT_AFTER_POWER_ON				5	//Unit : Second
#define KEEP_CNT_AFTER_PAIR					3	//Unit : Second

#endif
#endif
#define BUF_INFO_OUTPUT						0

#define KNL_DEBUG_INFO_EN					0
#define KNL_DEBUG_APP_Q_FULL_EN				0

//Resolution <= 1280x720
#define BIG_BUF_UNDER_HD_DVR				(192*1024)	//Big Buffer Size(DVR) 		<= Resolustion(1280x720) @A7130 System
#define SMALL_BUF_UNDER_HD_DVR				(32*1024)	//Small Buffer Size(DVR) 	<= Resolustion(1280x720) @A7130 System
#define BIG_BUF_UNDER_HD_GENERAL			(192*1024)	//Big Buffer Size(GENERAL) 	<= Resolustion(1280x720) @A7130 System					
#define SMALL_BUF_UNDER_HD_GENERAL			(32*1024)	//Small Buffer Size(GENERAL)<= Resolustion(1280x720) @A7130 System

//Resolution > 1280x720
#define BIG_BUF_OVER_HD_VBM_REMOTE_PLY		(192*1024)	//Big Buffer Size(VBM+RemotePly)   > Resolustion(1280x720) @A7130 System				
#define SMALL_BUF_OVER_HD_VBM_REMOTE_PLY	(136*1024)	//Small Buffer Size(VBM+RemotePly) > Resolustion(1280x720) @A7130 System
#define BIG_BUF_OVER_HD_GENERAL				(368*1024)	//Big Buffer Size(GENERAL)         > Resolustion(1280x720) @A7130 System				
#define SMALL_BUF_OVER_HD_VBM_GENERAL		(64*1024)	//Small Buffer Size(GENERAL)       > Resolustion(1280x720) @A7130 System

#define RF_MODE_SW_OLD						0
#define RF_MODE_SW_NEW						1
#define RF_MODE_SW_SEL						RF_MODE_SW_NEW

#define ENC_I_WHEN_BUSY						0
#define DROP_FRM_WHEN_BUSY					1
#define VDO_ENC_METHOD						DROP_FRM_WHEN_BUSY	//Drop frame when busy
//#define VDO_ENC_METHOD					ENC_I_WHEN_BUSY		//Encode I-frame when busy

#define RELEASE_BUF_DIRECT					0
#define RELEASE_BUF_QUE						1
#define RELEASE_BUF_METHOD					RELEASE_BUF_QUE
//Richwave RTC676x
//=================================================
#define KNL_RATIO_QPSKTO16QAM				0.9
#define KNL_RATIO_16QAMTOQPSK				0.6

#define KNL_FREE_BUF_TO_SEND_I_FRAME		3
#define KNL_FREE_BUF_TO_SEND_P_FRAME		1
#define KNL_DYNAMIC_SLOT		 			0

#define KNL_ERR_CNT_TH						1		//Times
#define KNL_ERR_STATIC						2		//Seconds
#define KNL_ACK_STATIC_NUM					100		//Packets
#define KNL_LINK_ACTIVE_TIME				10		//Seconds
#define KNL_RF_MONIT_TIME					10

#define RF_MODULE_NEW								//Default is V3.0 Module

#ifdef RF_MODULE_NEW								//V3.0 Module
	#define KNL_STABLE_TIME					0		//ms
#else
	#define KNL_STABLE_TIME					900		//ms
#endif

#define CHK_TIME_OUT_FOR_STA				3000	//3.0 Sec
#define CHK_TIME_OUT_FOR_AP					3000	//3.0 Sec
#define COMM_DATA_TYPE_NUM					3
#define COMM_MONIT_TIME						3		//3 Second
#define COMM_TRY_AGAIN_1					11
#define COMM_TRY_AGAIN_2					(-11)
#define	COMM_TIMEOUT_1						110
#define	COMM_TIMEOUT_2						(-110)
#define COMM_NACK							(-161)

#define TRXBW_RPT_EN						1

#define KNL_USB_OFF2ON_TIME					1000
#define KNL_WIRELESS_CAM_NUM				1

#define KNL_XU_MAX_DATA_LEN					64
//#define KNL_XU_MAX_EXT_DATA_LEN			59	//64-2(OpCode)-2(DataLen)-1(CamIndex) = 59
#define KNL_XU_MAX_EXT_DATA_LEN				60	//64-2(OpCode)-2(DataLen) = 60

#define KNL_EXT_BUF_NUM						8
#define KNL_ALIGN_UNIT						4
#define KNL_EXT_DATA_INTERNAL_LEN			4
#define KNL_EXT_DATA_EXTERNAL_LEN			512

#define KNL_NON_EXT_DATA					0xFFFFFFFFL

typedef enum
{	
	UVC_DEVICE1			= 0,
	UVC_DEVICE2			= 1,
}UVC_DEVICE;
typedef enum
{	
	MODE_QPSK			= 0,
	MODE_QPSKTO16QAM	= 1,
	MODE_16QAM			= 2,	
}RW_MODULATION_MODE;	//Modulation mode of RF

typedef enum
{
	EVENT_TRX_GOOD		= 0,	//Transmission is good	
	EVENT_TRX_NOTGOOD	= 1,	//Transmission is not good
}RW_MODULATION_EVENT;	//Event of RF	

typedef struct
{
	RW_MODULATION_MODE	tMode;
	uint8_t ubRcUpdateFlg;
	uint8_t ubModeUpdateFlg;		
}KNL_RF_MODE_SW;

//-----------------------------------------------------------------
//FS setting
//-----------------------------------------------------------------
#define KNL_FS_SD_POLLING_EN			0
#define KNL_FS_CIPHER_DMA_EN			1
typedef struct KNL_FS_MEDIA_INFORMATION
{
	FS_KNL_PARA_t InitPara;
	FS_MEDIA_SEL MediaSel;
	FS_MEDIA_TYPE MediaType[FS_MEDIA_MAX];
}KNL_FS_MEDIA_INFO_t;
//-----------------------------------------------------------------
void KNL_AutoGenPhoto(uint32_t ulCapNum);
void KNL_CopyFile(uint32_t ulCopyNum);
//-----------------------------------------------------------------
/*!
\brief Select the media of FS
\param MediaSel		media select
\return(no)
*/
void KNL_FS_MediaSel(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------
/*!
\brief Get current media of FS
\return FS_MEDIA_SEL
*/
FS_MEDIA_SEL KNL_GetFsMedia(void);
//-----------------------------------------------------------------
typedef struct KNL_MESSAGE_Q_INFORMATION
{
	osMessageQDef_t Def;
	osMessageQId Id;
}KNL_MSG_Q_INFO_t;
//-----------------------------------------------------------------------------
typedef struct KNL_THREAD_INFORMATION
{
	osThreadId Id;
}KNL_THRD_INFO_t;
//-----------------------------------------------------------------------------
typedef struct KNL_MUTEX_INFORMATION
{
	osMutexDef_t Def;
	osMutexId Id;
}KNL_MUTEX_INFO_t;
//-----------------------------------------------------------------------------
typedef struct KNL_SEMAPHORE_INFORMATION
{
	osSemaphoreDef_t Def;
	osSemaphoreId Id;
}KNL_SEMAPHORE_INFO_t;
//-----------------------------------------------------------------------------
typedef enum
{	
	KNL_OPERATION_SET	= 0,
	KNL_OPERATION_GET,
	KNL_OPERATION_MINUS,
	KNL_OPERATION_PLUS,
}KNL_OPERATION;

typedef enum
{	
	KNL_SLOT_MANUAL_SET	= 0,
	KNL_SLOT_AUTO_SET = 1,
}KNL_SLOT_SWITCH;

typedef enum
{	
	KNL_ACK_FAIL	= 0,
	KNL_ACK_OK		= 1,
}KNL_ACK_STATUS;

typedef enum
{	
	COMM_DATA_TYPE_VDO	= 0,
	COMM_DATA_TYPE_ADO	= 1,
	COMM_DATA_TYPE_CMD	= 2,
}COMM_DATA_TYPE;

typedef enum
{	
	INIT_STATE				= 0,
	LINK_STATE				= 1,
	NO_LINK_STATE			= 2,
	SW_RESET_STATE			= 3,	
	NO_INTERRUPT_STATE		= 4,
	SYS_RESTART_STATE		= 5,
	
}RW_RF_STATE;	//State of Richwave RF

typedef enum
{		
	LINK_EVENT			= 0,
	NO_LINK_EVENT		= 1,	
	NO_INTERRUPT_EVENT	= 2,
	
}RW_RF_EVENT;	//Event of Richwave RF

#define MAX_RW_RESET_TIME1	5	//For No-Link Event
#define MAX_RW_RESET_TIME2	2	//For No-Interrupt Event
//=================================================

typedef enum
{
	PKT_MAIN_VDO = 0,
	PKT_SUB_VDO,
	PKT_AUX_VDO,
	PKT_ADO,
	PKT_CMD,	
	PKT_FOTA,
}KNL_PKT_TYPE;

typedef enum
{	
	PKT_INFO_OFS_MODE		= 0,		//[0]Modulation, 5->16QAM, 9->QPSK, 10->BPSK
	PKT_INFO_OFS_RSV		= 1,		//[1]Reserve for future
	
	PKT_INFO_OFS_TYPE 		= (0+2),	//[2]
										//0->Main-Video
										//1->Sub-Video
										//2->Aux-Video
										//3->Audio
										//4->Command	
	PKT_INFO_OFS_ROLE 		= (1+2),	//[3]
	PKT_INFO_OFS_SRCNUM 	= (2+2),	//[4]
	PKT_INFO_OFS_TOTAL_IDX	= (3+2),	//[5]Rolling Code
	PKT_INFO_OFS_TOTAL_NUM	= (4+2),	//[6][7]4,5(Little-Endian)	
	PKT_INFO_OFS_FRM_IDX	= (6+2),	//[8][9]6,7(Little-Endian)
	PKT_INFO_OFS_PKT_IDX	= (8+2),	//[10][11]8,9(Little-Endian)
	
	PKT_INFO_OFS_CHKSUM		= (10+2),	//[12][13]10,11(Little-Endian)	
}PACKET_INFO;

#define MAX_NUM_PKT_PER_FRAME			256
#define MAX_PKT_ACC_ACK					64

//#define PKT_HEADER_SZ					12
//#define PKT_ACTIVE_HEADER_SZ			10
#define PKT_HEADER_SZ					14
#define PKT_ACTIVE_HEADER_SZ			12

#define PKT_VDO_PAYLOAD_SZ				(MAX_VDO_PACKET_LEN-PKT_HEADER_SZ)
#define PKT_ADO_PAYLOAD_SZ				(MAX_ADO_PACKET_LEN-PKT_HEADER_SZ)

#define MAX_AUX_SZ						48	//16*3

#define MAX_VDO_BIG_FRAM_SZ				(256*1024L)
#define MAX_VDO_SMALL_FRAM_SZ			(40*1024L)

//Setting for IQ-Tuning
#define KNL_RC_BITRATE					(0x12C000L<<4)	//150KB, Target BitRate for Main video stream

#define KNL_RC_QP_MAX                   (48)
#define KNL_RC_QP_MIN                   (28)
#define KNL_RC_FPS						(30)            //Target Frame-Rate
#define KNL_RC_GOP                      (20)

#if A7130
#define KNL_AUX_CRC_FUNC				0		//!< Auxiliary CRC Function
#endif

#if RTC676x
#define KNL_AUX_CRC_FUNC				1		//!< Auxiliary CRC Function
#endif

#define KNL_MIN_COMPRESS_RATIO_ADJ		9

#ifdef S2019A
#if (defined(BSP_RVCS_SDK)&&APP_REC_FUNC_ENABLE&&APP_PLAY_REMOTE_ENABLE)
#define KNL_MIN_COMPRESS_RATIO			8//6		//!< Minimum Compression Ratio
#else
#define KNL_MIN_COMPRESS_RATIO			6		//!< Minimum Compression Ratio
#endif
#define KNL_MIN_COMPRESS_RATIO_LR		8		//!< Minimum Compression Ratio @Local Recording
#else
#if defined(BSP_DVR_SDK)
#define KNL_MIN_COMPRESS_RATIO			8		//!< Minimum Compression Ratio
#define KNL_MIN_COMPRESS_RATIO_LR		8		//!< Minimum Compression Ratio @Local Recording
#elif (APP_ADJ_BUF_ENABLE == 1)
#define KNL_MIN_COMPRESS_RATIO			9		//!< Minimum Compression Ratio
#else
#if (defined(BSP_VBM_SDK)&&APP_REC_FUNC_ENABLE&&APP_PLAY_REMOTE_ENABLE)
#define KNL_MIN_COMPRESS_RATIO			10		//!< Minimum Compression Ratio
#else
#define KNL_MIN_COMPRESS_RATIO			7		//!< Minimum Compression Ratio
#endif
#endif
#endif

#define KNL_BB_RX_DON_Q_SIZE			10		//!< RX Queue Size

#if (defined(OP_STA) || (defined(OP_AP) && (!defined(BSP_DVR_SDK))) )
#define KNL_PROC_QUEUE_NUM				150		//!< KNL_Process Queue Size
#else
#define KNL_PROC_QUEUE_NUM				200		//!< KNL_Process Queue Size
#endif
#define KNL_AVG_PLY_QUEUE_NUM			30		//!< KNL_AvgPlyProcess Queue Size
#define KNL_JPEG_QUEUE_NUM				64		//!< JPEG_Monitor Queue Size
#define KNL_MAX_NODE_NUM				16		//!< Maximum Node Number
#if defined(BSP_DVR_SDK)
#define KNL_SRC_NUM						22		//!< Source Number	//justin 2019.07.19
#else
#define KNL_SRC_NUM						20		//!< Source Number
#endif

#define KNL_ADO_SUB_PKT_LEN				36		//!< Audio Sub-Packet Length

#define KNL_AVG_PLY_CNT_TH				4
#define KNL_ADO_DEBUG_EN				0

#define KNL_FILE_SIZE_THRESHOLD         300     // 300M, according to data rate 500KB, formula=500(KB)*60(s)*10(min)+500(header)*4(src)

#define KNL_SD_PHOTO_SIZE_THRESHOLD		300
#define KNL_SF_PHOTO_SIZE_THRESHOLD		1
#define KNL_GARBAGE_USAGE_THRESHOLD		30

#if defined(VBM_BU) || defined(BUC_CAM) || defined(BIO_BU)
#define KNL_LCD_FUNC_ENABLE				0
#else
#define KNL_LCD_FUNC_ENABLE				1
#endif

#define KNL_CU_STORAGE_DATARATE			(1536)	//1536KB For Local Storage Video
#if defined(BSP_DVR_SDK)
#define KNL_JPG_BS_SIZE                 0x110000     //!1MB
#elif (defined(BSP_RVCS_SDK)&&defined(BUC_CAM)&&APP_REC_FUNC_ENABLE&&APP_PLAY_REMOTE_ENABLE)
#define KNL_JPG_BS_SIZE                 0x090000     //!512KB
#else
#define KNL_JPG_BS_SIZE                 0x50000     //!256KB
#endif
#define KNL_JPG_BS_BIGSIZE              0x510000     //!5MB
#define KNL_JPG_HEADER_SIZE             0x800		//0x10000//0xBDE//0x400

#define KNL_JPG_SOI_SIZE                0x02
#define KNL_JPG_EXIF_SIZE               0x3FE		//0xFBFE,Total Exif size(inculde exif header)
#define KNL_JPG_FRAMEHEADER_SIZE        0x13        //!19
#define KNL_JPG_APP1_SIZE               0x1B5       //!435 For FS Info
#define KNL_JPG_DQT_SIZE                0x238       //!568 Define Quantization Table

#define KNL_JPG_EXIF_START             (KNL_JPG_SOI_SIZE)
#define KNL_JPG_FRAMEHEADER_START      (KNL_JPG_EXIF_START + KNL_JPG_EXIF_SIZE)
#define KNL_JPG_APP1_START             (KNL_JPG_FRAMEHEADER_START + KNL_JPG_FRAMEHEADER_SIZE)
#define KNL_JPG_DQT_START              (KNL_JPG_APP1_START + KNL_JPG_APP1_SIZE)

#define KNL_JPEG_HEADER_V_OFFSET        0x05
#define KNL_JPEG_HEADER_H_OFFSET        0x07
#define KNL_JPEG_HEADER_FMT_OFFSET      0x0B


#define KNL_TX_GOP						900L
#define KNL_TX_RVCS_GOP					1L 
#define KNL_TX_RECGOP					300L

#define KNL_REC_SOURCE_MODE				REC_SRCMODE_CFG

//! USB Host
#if (defined(S2019A) || (defined(OP_AP) && defined(sWIFIBDG)))
	#define KNL_USBH_SPRF
	#define KNL_USBH_FUNC_ENABLE        1
	#define KNL_USBH_UVCCLASS_ENABLE	0
#else
#if APP_DUAL_HOST_ENABLE
	#define KNL_USBH_FUNC_ENABLE        1
	#define KNL_USBH_UVCCLASS_ENABLE	1
#else
	#define KNL_USBH_FUNC_ENABLE        0
	#define KNL_USBH_UVCCLASS_ENABLE	0
#endif	//! End of #if APP_DUAL_HOST_ENABLE
#endif	//! End of #if (defined(S2019A) && (defined(OP_AP) && defined(sWIFIBDG)))

#define KNL_RFPWR_CTRL_ENABLE			0

#define ISP_DS_EN						1
#define IMG_DS_EN						0

#define KNL_MJ_BUF_ALIGN_1K				1024

#define KNL_UVCMJ_QP_VALUE				10
#define KNL_UVCMJ_BS_SIZE				0x100000
#define KNL_UVCMJ_HQBS_SIZE				0x300000
#define KNL_UVCMJ_FORMAT				JPEG_YUV422

typedef enum
{
	KNL_FPS_OUT		= 0,			//!< Output FPS
	KNL_FPS_IN,						//!< Input FPS
	KNL_BB_FRM_OK
}KNL_FPS_TYPE;

typedef enum
{	
	KNL_OPMODE_VBM_1T		= 0,	//!< VBM->1T Operation Mode
	KNL_OPMODE_VBM_2T		= 1,	//!< VBM->2T Operation Mode
	KNL_OPMODE_VBM_4T		= 2,	//!< VBM->4T Operation Mode	
	KNL_OPMODE_BUC_4T 		= 3,	//!< BUC->H Operation Mode	
	KNL_OPMODE_BUC_3T_1T2B 	= 4,	//!< BUC->3T_1T2B Operation Mode
	KNL_OPMODE_BUC_3T_2T1B 	= 5,	//!< BUC->3T_2T1B Operation Mode
	KNL_OPMODE_BUC_3T_2L1R 	= 6,	//!< BUC->3T_2L1R Operation Mode
	KNL_OPMODE_BUC_3T_1L2R 	= 7,	//!< BUC->3T_1L2R Operation Mode
	KNL_OPMODE_BUC_2T_1T1B 	= 8,	//!< BUC->2T_1T1B Operation Mode
	KNL_OPMODE_BUC_2T_1L1R 	= 9,	//!< BUC->2T_1L1R Operation Mode
	KNL_OPMODE_BUC_1T 		= 10,	//!< BUC->1T Operation Mode	
}KNL_OPMODE;

typedef enum
{	
	KNL_NORMAL_PLY = 0,
	KNL_AVG_PLY,
}KNL_PLY_MODE;

typedef enum
{	
	KNL_NODE_START = 0,					//!< Start State
	KNL_NODE_TRANS = 1,					//!< Transition State
	KNL_NODE_STOP  = 0xFF,				//!< Stop State
}KNL_NODE_STATE;

typedef enum
{	
	KNL_DATA_TYPE_VDO = 0,					
	KNL_DATA_TYPE_ADO = 1,			
}KNL_DATA_TYPE;

typedef enum
{
	KNL_DISP_ROTATE_0,					//!< Without Rotate
	KNL_DISP_ROTATE_90					//!< With Rotate (90 Degree)	
}KNL_DISP_ROTATE;

typedef enum
{	
	KNL_SEN_MAIN_PATH = 0,				//!< MAIN Stream Output
	KNL_SEN_AUX_PATH,					//!< AUX Stream Output
	KNL_SEN_SUB_PATH,					//!< SUB Stream Output
}KNL_SEN_PATH;

typedef enum
{	
	KNL_DISP_SINGLE		= 0,			//!< Single
	KNL_DISP_DUAL_C		= 1,			//!< 1 Top,1 Bottom
	KNL_DISP_DUAL_U		= 2,			//!< 1 Left,1 Right
	KNL_DISP_QUAD 		= 3,			//!< Quad
	KNL_DISP_H			= 4,			//!< H Type
	KNL_DISP_3T_1T2B	= 5,			//!< 1 Top,2 Bottom
	KNL_DISP_3T_2T1B  	= 6,			//!< 2 Top,1 Bottom
	KNL_DISP_3T_2L1R  	= 7,			//!< 2 Left,1 Right
	KNL_DISP_3T_1L2R  	= 8,   			//!< 1 Left,2 Right
#if defined(BSP_DVR_SDK)
	KNL_DISP_PIP_0		= 9,			//!< Display PIP, inset picture at upper left
	KNL_DISP_PIP_1		= 10,			//!< Display PIP, inset picture at upper right
	KNL_DISP_PIP_2		= 11,			//!< Display PIP, inset picture at lower left
	KNL_DISP_PIP_3		= 12,			//!< Display PIP, inset picture at lower right
#endif
	KNL_DISP_3T_3C		= 13,			//!< 3 Column(Show Ratio -> 1:1:1)	
    KNL_DISP_3T_3COL	= 14,
	
	KNL_DISP_NONSUP		= 0xFF,
}KNL_DISP_TYPE;

typedef enum
{	
	KNL_DISP_LOCATION1	= 0,			//!< Display @ Location1
	KNL_DISP_LOCATION2	= 1,			//!< Display @ Location2
	KNL_DISP_LOCATION3 	= 2,			//!< Display @ Location3
	KNL_DISP_LOCATION4 	= 3,			//!< Display @ Location4
	KNL_DISP_LOCATION_ERR = 0xFF,		//!< Error
}KNL_DISP_LOCATION;

typedef enum
{	
	KNL_VDO_CODEC_H264 = 0,				//!< H264 Codec
	KNL_VDO_CODEC_JPEG = 1				//!< JPEG Codec
}KNL_VDO_CODEC;

typedef enum
{	
	KNL_ADO_CODEC_PCM	= 0,			//!< PCM Codec
	KNL_ADO_CODEC_MSADPCM,				//!< MSADPCM Codec
	KNL_ADO_CODEC_ALAW,					//!< A-Law Codec
	KNL_ADO_CODEC_AAC,					//!< AAC Codec
	KNL_ADO_CODEC_ADO32,				//!< Audio32 Codec
}KNL_ADO_CODEC;

typedef enum
{	
	KNL_PATH_VDO	= 0,				//!< Video Path
	KNL_PATH_ADO 	= 1,				//!< Audio Path
}KNL_PATH_TYPE;

typedef enum
{	
	KNL_VDO_PKT		= 0,				//!< Video Packet
	KNL_ADO_PKT 	= 1,				//!< Audio Packet
	KNL_FOTA_PKT	= 2,				//!< FOTA Packet
}KNL_PACKET_TYPE;

typedef enum 
{
	KNL_FILE,       //!< Record MP4 or AVI
	KNL_PHOTO,   //!< Record Photo
}KNL_REC_TYPE;

#if defined(RTC676x)
typedef struct
{
	uint32_t ulRealFwSz;
	uint32_t ulPadFwSz;
	uint16_t uwNumOfFrm;
	uint16_t uwSzPerFrm;
	uint8_t ubFwType;
	uint16_t uwFrmIdx;
	uint8_t ubRsv;
}FOTA_INFO;
#endif

//[FOTA]
#define FOTA_TARGET_FILE_NAME		   	"SNCC7XFW"

#define FOTA_REAL_FW_SZ_OFS				0
#define FOTA_PAD_FW_SZ_OFS				4
#define FOTA_NUM_OF_FRM_OFS				8
#define FOTA_SZ_PER_FRM_OFS				10
#define FOTA_FW_TYPE_OFS				12
#define FOTA_FRM_IDX_OFS				13
#define FOTA_FW_OFS						16

#define FOTA_INFO_SZ					16
#define FOTA_MAX_SZ_PER_FRM				(15*1024)
#define FOTA_DUMMY_SZ					128

#ifdef OP_STA
#define FOTA_ALLOC_BUF_SZ				(2560*1024)	//2.5 MBytes
#else
#define FOTA_ALLOC_BUF_SZ	 			(FOTA_MAX_SZ_PER_FRM+FOTA_INFO_SZ+FOTA_MAX_SZ_PER_FRM+FOTA_DUMMY_SZ)
#endif

//Define for Check Valid
#define FOTA_MAX_REAL_FW_SZ				(FOTA_ALLOC_BUF_SZ-16)	//16 Bytes for Information
#define FOTA_MAX_PAD_FW_SZ				(FOTA_ALLOC_BUF_SZ-16)	//16 Bytes for Information
#define FOTA_MAX_NUM_OF_FRM				(FOTA_MAX_REAL_FW_SZ/FOTA_MAX_SZ_PER_FRM)
#define FOTA_PKT_SZ						(FOTA_INFO_SZ+FOTA_MAX_SZ_PER_FRM+FOTA_DUMMY_SZ)	//FOTA Packet @Air
#define FOTA_MAX_FW_TYPE				16
#define FOTA_MAX_FRM_IDX				(FOTA_MAX_NUM_OF_FRM-1)

typedef struct
{
	uint8_t		ubSrcNum;				//!< Source Number
	uint8_t 	ubCurNode;				//!< Current Node	
	uint8_t 	ubNextNode;				//!< Next Node	
	uint32_t	ulDramAddr1;			//!< RAW Data
	uint32_t	ulDramAddr2;			//!< Bit-Stream Data	
	uint32_t	ulSize;					//!< Size
	uint8_t 	ubPath;					//!< Video or Audio Path
	uint8_t 	ubCodecIdx;				//!< Codec Index
	uint32_t 	ulIdx;					//!< Index for Aux-Info
	uint8_t 	ubVdoGop;				//!< Video Group for Aux-Info
	uint32_t 	ulGop;					//!< GOP for Aux-Info
    uint32_t    ulTime;               
	uint8_t 	ubTargetRole;
	uint8_t 	ubTwcCmd;
	KNL_PKT_TYPE tPktType;				//For RTC676x
	uint8_t     ubHqCapFlag;
#ifdef S2019A
	uint32_t    ulsFrmTm;
#endif
	uint8_t     ubRstVdoGrp;

	uint8_t 	ubEvent;
	uint16_t 	uwExtLen;				//For Extra Data @Frame
	uint8_t 	ubDeviceId;				//For Extra Data @Frame
	uint32_t 	ulExtDataOfs;			//For Extra Data @Frame
	uint8_t 	ubIsBigBuf;				//1 -> Big Buffer, 0 -> Small Buffer
	
	uint16_t uwHSize;
	uint16_t uwVSize;
}KNL_PROCESS;

typedef struct
{
	uint8_t		ubSrcNum;				//!< Source Number
	uint8_t 	ubCurNode;				//!< Current Node	
	uint8_t 	ubNextNode;				//!< Next Node	
	uint32_t	ulDramAddr1;			//!< RAW Data
	uint32_t	ulDramAddr2;			//!< Bit-Stream Data	
	uint32_t	ulSize;					//!< Size
	uint8_t 	ubPath;					//!< Video or Audio Path
	uint32_t 	ulIdx;					//!< Index for Aux-Info
	uint32_t 	ulGop;					//!< GOP for Aux-Info
}KNL_AVG_PLY_PROCESS;

typedef enum
{		
	KNL_NODE_SEN 			= 0x00,		//!< Sensor
	KNL_NODE_SEN_YUV_BUF	= 0x01,		//!< YUV Buffer for Sensor
	KNL_NODE_H264_ENC		= 0x02,		//!< H264 Encode	
	KNL_NODE_VDO_BS_BUF1	= 0x03,		//!< Bit-Stream Bufffer1
	KNL_NODE_VDO_BS_BUF2	= 0x04,		//!< Bit-Stream Bufffer2	
	KNL_NODE_COMM_TX_VDO	= 0x05,		//!< Communication TX For Video
	KNL_NODE_COMM_RX_VDO	= 0x06,		//!< Communication RX For Video
	KNL_NODE_H264_DEC		= 0x07,		//!< H264 Decode	
	KNL_NODE_LCD			= 0x08,		//!< LCD	
	KNL_NODE_JPG_ENC		= 0x09,		//!< JPEG Encode	
	KNL_NODE_JPG_DEC1		= 0x0A,		//!< JPEG Decode1
	KNL_NODE_JPG_DEC2		= 0x0B,		//!< JPEG Decode2
	KNL_NODE_ADC			= 0x0C,		//!< ADC
	KNL_NODE_DAC			= 0x0D,		//!< DAC
	KNL_NODE_ADC_BUF		= 0x0E,		//!< ADC Buffer
	KNL_NODE_DAC_BUF		= 0x0F,		//!< DAC Buffer
	KNL_NODE_RETRY_ADC_BUF	= 0x10,		//!< Retry for ADC_BUF Process
	KNL_NODE_COMM_TX_ADO	= 0x11,		//!< Communication TX For Audio
	KNL_NODE_COMM_RX_ADO	= 0x12,		//!< Communication RX For Audio	
	KNL_NODE_IMG_MERGE_BUF	= 0x13,		//!< Image Merge Buffer
	KNL_NODE_IMG_MERGE_H	= 0x14,		//!< Horizontal Image Merge	
	KNL_NODE_VDO_REC		= 0x15,		//!< Video Record
	KNL_NODE_ADO_REC		= 0x16,		//!< Audio Record
	KNL_NODE_UVC_MAIN		= 0x17,		//!< UVC Main Video
	KNL_NODE_UVC_SUB		= 0x18,		//!< UVC Sub Video
	KNL_NODE_UVC_MJPG		= 0x19,		//!< UVC MJPG Video
#if defined(BSP_DVR_SDK)
	KNL_NODE_JPG_BS			= 0x1A,
	KNL_NODE_JPG_RAW		= 0x1B,
#endif
	KNL_NODE_MSC_ADO		= 0x31,
	KNL_NODE_REC_PLAY		= 0x32,
	
	KNL_NODE_BDG_PTT		= 0xDF,
	KNL_NODE_NONE 			= 0xE0,		//!< None Node
	KNL_NODE_END 			= 0xF0,		//!< End Node		
}KNL_NODE;

typedef enum
{
	LOCAL_REMOTE,		//!< ado from local to remote
	APP_LOCAL_REMOTE	//!< ado from app to local, and from local to remote
}KNL_ADO_SRC_SEL;

typedef enum
{	
	//For Video Path
	KNL_SRC_1_MAIN 	= 0,				//!< Source Number (MAIN1)	
	KNL_SRC_2_MAIN 	= 1,				//!< Source Number (MAIN2)
	KNL_SRC_3_MAIN 	= 2,				//!< Source Number (MAIN3)
	KNL_SRC_4_MAIN 	= 3,				//!< Source Number (MAIN4)	
	KNL_SRC_1_SUB	= 4,				//!< Source Number (SUB1)
	KNL_SRC_2_SUB	= 5,				//!< Source Number (SUB2)
	KNL_SRC_3_SUB	= 6,				//!< Source Number (SUB3)
	KNL_SRC_4_SUB	= 7,				//!< Source Number (SUB4)
	KNL_SRC_1_AUX	= 8,				//!< Source Number (AUX1)
	KNL_SRC_2_AUX	= 9,				//!< Source Number (AUX2)
	KNL_SRC_3_AUX	= 10,				//!< Source Number (AUX3)
	KNL_SRC_4_AUX	= 11,				//!< Source Number (AUX4)
	
	//For Audio Path
	KNL_SRC_1_OTHER_A	= 12,			//!< Source Number (OTHER_A_1)
	KNL_SRC_2_OTHER_A	= 13,			//!< Source Number (OTHER_A_1)
	KNL_SRC_3_OTHER_A	= 14,			//!< Source Number (OTHER_A_1)
	KNL_SRC_4_OTHER_A	= 15,			//!< Source Number (OTHER_A_1)	
	KNL_SRC_1_OTHER_B	= 16,			//!< Source Number (OTHER_B_1)
	KNL_SRC_2_OTHER_B	= 17,			//!< Source Number (OTHER_B_1)
	KNL_SRC_3_OTHER_B	= 18,			//!< Source Number (OTHER_B_1)
	KNL_SRC_4_OTHER_B	= 19,			//!< Source Number (OTHER_B_1)
#if defined(BSP_DVR_SDK)
	KNL_SRC_PREVIEW_LOCAL,				//YUV(1280x720) for Local Preview (YUV->LCD)	
	KNL_SRC_STORAGE_LOCAL,				//YUV(1920x1088) for Local Storage(YUV->H264->MP4->FS->SD)
#endif

	KNL_SRC_MASTER_AP   = 0xFE,

	KNL_SRC_NONE		= 0xFF,			//!< Source Number (NONE)
}KNL_SRC;

typedef enum
{
	KNL_READY	= 0,
	KNL_BUSY	= 1,
}KNL_STATUS;

typedef enum	
{
	FOTA_INIT			= 0,
	FOTA_START			= 1,
	FOTA_LOST_LINK		= 2,	
	FOTA_NO_FREE_BUF 	= 3,
	FOTA_SEND_OK		= 4,
	
	FOTA_FILE_OK		= 9,
	FOTA_QUEUE_OK		= 10,
	FOTA_READ_FILE_OK	= 11,
	
	FOTA_NO_SD			= 12,	//No SD Card exist	
	FOTA_SD_NOT_RDY		= 13,	//SD Card Not Ready
	FOTA_NO_FILE		= 14,	//No File
	FOTA_READ_FILE_FAIL = 15,
	FOTA_SEND_FAIL		= 16,
	FOTA_STOP			= 17,
	FOTA_QUEUE_FAIL		= 18,
}FOTA_STATUS;
typedef enum
{
	KNL_STA1 = 0,						//!< Role (STA1)
	KNL_STA2,							//!< Role (STA2)
	KNL_STA3,							//!< Role (STA3)
	KNL_STA4,							//!< Role (STA4)
	KNL_SLAVE_AP,						//!< Role (SLAVE_AP)
	KNL_MASTER_AP,						//!< Role (MASTER_AP)
	KNL_NONE,							//!< Role (NONE)
	KNL_MAX_ROLE,
}KNL_ROLE;

#if defined(OP_AP)
#if defined(RTC676x)
#if APP_RTC676X_FOTA_ENABLE
uint8_t ubKNL_ChkFotaStatusValid(void);
FOTA_STATUS tKNL_GetFotaStatus(void);
void KNL_StartFota(KNL_ROLE tRole);
FOTA_STATUS tKNL_StartFotaProcess(KNL_ROLE tRole);
void KNL_StopFotaProcess(void);
uint8_t ubKNL_GetFotaStatusRpt(void);
uint8_t ubKNL_FotaDataSend(KNL_ROLE tRole,uint32_t ulAddr,uint32_t ulSize);
uint32_t ulKNL_FotaDataPacketize(uint8_t ubAccessUnit,uint32_t ulStartRun,uint32_t ulInputAddr,uint32_t ulInputSize,uint32_t ulBufAddr,uint8_t ubFwType);
#endif
#endif
#endif
#if defined(BSP_DVR_SDK)
typedef enum
{
	KNL_LOCAL = 0,
	KNL_REMOTE,
	KNL_POSI_NONE,
	KNL_MAX_POSITION,
}KNL_POSITION;
#endif

typedef enum
{
	KNL_LOST_LINK = 0,
	KNL_LINK,
}KNL_LINK_STATUS;

enum
{
	KNL_IMG_MERGE_H = 0,	//!< Horizontal Image Merge
};

typedef enum
{
	KNL_SCALE_X1 	= 1,	//!< Without Scaling
	KNL_SCALE_X0P5	= 2,	//!< (1/2) Scaling
	KNL_SCALE_X0P25	= 3		//!< (1/4) Scaling
}KNL_SCALE;

typedef enum
{
	KNL_REAL_FLD,
	KNL_SIM_FLD,
}KNL_FldType_t;

typedef struct KNL_TXREC_CFG
{
	uint8_t  ubCfg;
    uint8_t  ubCh;
    uint16_t uwFrmRate;
    uint32_t ulHoSize1;
    uint32_t ulVoSize1;
    uint16_t uwBSMaxBufNum;
    uint8_t  ubReserve[2];
}KNL_TXREC_CFG;

typedef struct
{
    uint8_t 	ubTrgger;
	uint8_t		ubFSTp;
    uint8_t		ubRev[2];
	uint32_t 	ulAdr;
	uint32_t 	ulSize;
}KNL_TX_FSSORT;

typedef struct
{
    uint8_t 	ubFldNum;
    uint8_t		ubLinked[23];
}KNL_TX_FLDINFO;

typedef enum
{
	KNL_TX_FLDROOT,    // L1,Root Folder List
	KNL_TX_FLDSUB1,     // L2,Folder List 
	KNL_TX_FLDSUB2,     // L3,File List Folder
}KNL_TX_FLDLAYER;

typedef enum
{
	KNL_PLY_PREV,      // File Index--
	KNL_PLY_NEXT,      // File Index++
	KNL_PLY_KEEP,      // Not Jump Next File
}KNL_PLY_NextItem_t;

typedef struct
{
	uint8_t 	ubPreNode;			//!< Previous Node
	uint8_t 	ubCurNode;			//!< Current Node
	uint8_t 	ubNextNode;			//!< Next Node
	uint16_t 	uwVdoH;				//!< Horizontal Resolution
	uint16_t 	uwVdoV;				//!< Vertical Resolution
	uint8_t 	ubHMirror;			//!< Horizontal Mirror
	uint8_t 	ubVMirror;			//!< Vertical Mirror
	uint8_t 	ubRotate;			//!< Rotation
	uint8_t 	ubHScale;			//!< Horizontal Scaling
	uint8_t 	ubVScale;			//!< Vertical Scaling
	uint8_t 	ubMergeSrc1;		//!< Source 1 for Image Merge
	uint8_t 	ubMergeSrc2;		//!< Source 2 for Image Merge
	uint8_t 	ubMergeDest;		//!< Destination for Image Merge	
	uint16_t 	uwMergeH;			//!< Horizontal Resolution for Image Merge
	uint16_t 	uwMergeV;			//!< Vertical Resolution for Image Merge
	uint8_t 	ubCodecIdx;			//!< Codec Index
	uint8_t 	ubJpgScale;			//!< 0=Orig,1=1/2, 2=1/4
}KNL_NODE_INFO;

typedef enum
{
	KNL_I_FRAME = 0,	//!< H264 I Frame
	KNL_P_FRAME = 1,	//!< H264 P Frame
	KNL_ERR_FRAME = 0x10,
}KNL_FRAME_TYPE;

typedef enum
{	
	KNL_COMM_STATE_STOP	 = 0,	//!< Communication State -> Stop
	KNL_COMM_STATE_START = 1	//!< Communication State -> Start
}KNL_COMM_STATE;

typedef enum
{
	IMG_MERGE_LCD	= 0,		//!< Source for Image Merge
}KNL_IMG_MERGE_SOURCE;

typedef struct					//Kernel Information
{		
	//System
	uint8_t	ubRole;						//!< Role
	uint8_t ubOpMode;									//!< Operation Mode
	uint8_t	ubAuxInfoFlg;				//!< With or Without Aux Information
	uint8_t ubOnlineStaNum;				//!< Current Number of Station on-line
	uint8_t ubMaxSlotNum;				
	uint32_t ulMasterId;
	uint32_t ulSlaveId[4];
	uint8_t ubSlaveIdx;
	
	//Video
	uint8_t ubVdoCodec;					//!< Video Codec	
	uint16_t uwVdoH[KNL_SRC_NUM];		//!< Video Horizontal Resolution	
	uint16_t uwVdoV[KNL_SRC_NUM];		//!< Video Vertical Resolution	
	uint8_t ubJpegCodecQp;				//!< JPEG Codec QP
	H264_ENCODE_INDEX tEncIdx;			//!< H264 Encoder Index
	uint32_t ulGop;						//!< H264 GOP
	
	//Audio
	ADO_COMPRESS_MODE 	tAdoCodec;		//!< Audio Codec
	ADO_SAMPLERATE 	tAdoSamplingRate;	//!< Audio Sampling Rate	
	uint32_t ulAdcBufSz;				//!< Audio ADC Buffer Size
	uint32_t ulAdcRptSz;				//!< Audio ADC Report Size	
	uint32_t ulDacBufSz;				//!< Audio DAC Buffer Size
	uint32_t ulDacRptSz;				//!< Audio DAC Report Size		
	ADO_ADC_DEV_t	tAdcDevice;			//!< ADC Device(Type)
	ADO_DAC_DEV_t	tDacDevice;			//!< DAC Device(Type)

	//Display
	KNL_DISP_TYPE	tDispType;			//!< Display Type
	KNL_DISP_ROTATE tDispRotate;		//!< Display Rotate
	uint8_t ubDisp1SrcNum;				//!< Source Number @Disp1 Location
	uint8_t ubDisp2SrcNum;				//!< Source Number @Disp2 Location
	uint8_t ubDisp3SrcNum;				//!< Source Number @Disp3 Location
	uint8_t ubDisp4SrcNum;				//!< Source Number @Disp4 Location	
	uint16_t uwLcdDmyImgH;				//!< Horizontal Resolution for {H Type,Rotate 0}	
	uint16_t uwDispH;					//!< Display Horizontal Resolution
	uint16_t uwDispV;					//!< Display Vertical Resolution
	
	//Sen
	uint8_t ubSenPath1Src;				//!< Source Number for Sensor path1 output									
	uint8_t ubSenPath2Src;				//!< Source Number for Sensor path2 output
	uint8_t ubSenPath3Src;				//!< Source Number for Sensor path3 output
	
	//Multi-Output
	uint8_t ubMultiOutFlg[0x100L];		//!< Multiple-Output Flag
	uint8_t ubMultiInSrc[0x100L];		//!< Input Source for Multi-Output Node
	uint8_t ubMultiOutSrc1[0x100L];		//!< Output Source 1 for Multi-Output Node
	uint8_t ubMultiOutSrc2[0x100L];		//!< Output Source 2 for Multi-Output Node	
	
	//Play mode
	KNL_DISP_TYPE tPlayDispType;
	KNL_PLY_MODE tPlyMode;				//!< Play mode
	uint8_t ubVdoFps;					//!< Video fps
}KNL_INFO;

typedef struct
{
	uint16_t uwMergeH;
	uint16_t uwMergeV;
	uint32_t ulMergeAddr1;
	uint32_t ulMergeAddr2;
	uint32_t ulMergeAddr3;
	uint32_t ulMergeDestAddr;
	IMG_MERGE_TYPE tMergeType;
	IMG_MERGE_LOCATION tMergeLoc;
	IMG_SCALING_DOWN_RATIO tMergeScale;
}KNL_ImgMergeParam_t;

typedef struct
{
	uint8_t			  	ubSetupFlag;
	KNL_SRC			  	tSrcNum[4];
	KNL_DISP_LOCATION 	tSrcLocate[4];
	uint8_t				ubDispBufChgFlag;
	uint8_t				ubFixDispChFlag;
}KNL_SrcLocateMap_t;

typedef enum
{
	KNL_TUNINGMODE_OFF = 0,
	KNL_TUNINGMODE_ON,
}KNL_TuningMode_t;

typedef enum
{
	KNL_VDO_I_FRAME,
	KNL_VDO_P_FRAME,
	KNL_ADO_FRAME,
}KNL_UsbdFrm_t;

typedef enum
{	
	KNL_MAIN_PATH = 0,				//!< MAIN Stream Output	
	KNL_SUB_PATH,					//!< SUB Stream Output
	KNL_AUX_PATH,					//!< AUX Stream Output
}KNL_VA_DATAPATH;

typedef KNL_SRC (*pvRoleMap2Src)(KNL_VA_DATAPATH, KNL_ROLE);

typedef struct
{
	uint8_t 	ubDsEn;
	uint16_t 	uwImg_HSize;
	uint16_t 	uwImg_VSize;
	uint32_t 	ulDs_BufAddr;
	uint16_t 	uwDs_PosX;
	uint16_t 	uwDs_PosY;
	uint8_t  	ubDs[8];
}KNL_ImgCtrlDsInfo_t;

typedef enum
{
	KNL_CAPHQ_DIS = 0,
	KNL_CAPHQ_EN  = 0x2A,
	KNL_CAPHQ_TXRDY,
	KNL_CAPHQ_RXRDY,
}KNL_HQImgSte_t;

typedef struct
{
	uint8_t  ubHQ_En;
	uint16_t uwHQ_H;
	uint16_t uwHQ_V;
}KNL_HQImgCap_t;

typedef enum
{	
	KNL_CVBS = 0,
    KNL_720P = 1,
    KNL_1080P = 2,
	KNL_NULL = 3,
}KNL_AHD_FORMAT;

typedef enum
{	
    KNL_AHD = 0,
    KNL_TVI = 1,
    KNL_CVI = 3,
}KNL_AHD_TYPE;

typedef enum
{	
    KNL_PAL = 0,
    KNL_NTSC = 1,
}KNL_N_P_MODE;


typedef struct
{
	KNL_AHD_FORMAT tFormat;
	KNL_AHD_TYPE	tType;
	KNL_N_P_MODE	tMode;	
}KNL_AHD_INFO;

typedef enum
{	
    KNL_RP_1FHD = 0,
    KNL_RP_2FHD,
    KNL_RP_1SXGA,
    KNL_RP_1HD,
    KNL_RP_1XGA,
    KNL_RP_1WSVGA,
    KNL_RP_1SVGA,
    KNL_RP_1WVGA,
    KNL_RP_1VGA,
    KNL_RP_1HD_1FHD,
    KNL_RP_2HD,
    KNL_RP_4VGA,
    KNL_RP_2XGA,
    KNL_R_1FHD,
    KNL_RP_1HD60FPS,
    KNL_RP_1VGA60FPS,
    KNL_R_1HD60FPS,
}KNL_RP_SRC_MODE;

typedef enum
{	
    KNL_ISP_HD = 0,
    KNL_ISP_FHD = 1,
    KNL_ISP_1296P = 2,
}KNL_ISP_RESOLUTION;

typedef void (*pvImgDsUpdFunc)(KNL_ImgCtrlDsInfo_t);

#if defined(RTC676x)
#if APP_RTC676X_FOTA_ENABLE
typedef void (*pvFotaEventCbFunc)(void);
#ifdef OP_STA
void KNL_RegisterFotaEventAtTx(pvFotaEventCbFunc pvCB);
#endif
//#ifdef OP_AP
//void KNL_RegisterFotaEventAtRx(pvFotaEventCbFunc pvCB);
//#endif
#endif
#endif
//------------------------------------------------------------------------------
/*!
\brief 	Get KNL Function Version	
\return	Unsigned short value, high byte is the major version and low byte is the minor version
\par [Example]
\code		 
	 uint16_t uwVer;
	 
	 uwVer = uwKNL_GetVersion();
	 printf("KNL Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
\endcode
*/
uint16_t uwKNL_GetVersion (void);

void KNL_SetMasterId(uint32_t ulId);
void KNL_SetSlaveId(uint8_t ubSlaveIdx,uint32_t ulId);
uint32_t ulKNL_GetMasterId(void);
uint32_t ulKNL_GetSlaveId(uint8_t ubSlaveIdx);

uint8_t ubKNL_GetQp(uint8_t ubCodecIdx);

#ifdef RTC676x
#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE
void KNL_SetFreqTableInSf(uint8_t ubFreqSelect);
uint8_t ubKNL_GetFreqTableInSf(void);
void KNL_SetFreqTable(uint8_t ubFreqTable);
#endif
#endif
//------------------------------------------------------------------------
/*!
\brief Get BB IP buffer size
\return Buffer size
*/
uint32_t ulKNL_GetBbIpBufSz(void);

//------------------------------------------------------------------------
/*!
\brief Wireless Device buffer initialize
\return Buffer size
*/
void KNL_WirelessDevBufInit(void);

//------------------------------------------------------------------------
/*!
\brief Set Audio Information
\param tAdoInfo Audio Information
\return (no)
*/
void KNL_SetAdoInfo(ADO_KNL_PARA_t tAdoInfo);	

//------------------------------------------------------------------------
/*!
\brief Initial Kernel
\return(no)
*/
void KNL_Init(void);

//------------------------------------------------------------------------
/*!
\brief Stop Kernel
\return(no)
*/
void KNL_Stop(void);

//------------------------------------------------------------------------
/*!
\brief Restart Kernel
\return(no)
*/
void KNL_ReStart(void);

//------------------------------------------------------------------------
/*!
\brief Set Play Mode
\param tPlyMode 0->Normal Play,1->Smooth Play
\return (no)
*/
void KNL_SetPlyMode(KNL_PLY_MODE tPlyMode);

//------------------------------------------------------------------------
/*!
\brief Set Video FPS
\param ubFps Video FPS
\return (no)
*/
void KNL_SetVdoFps(uint8_t ubFps);

//------------------------------------------------------------------------
/*!
\brief Set target FPS.
\param ubFps Target FPS
\return (no)
*/
void KNL_SetTargetFps(uint8_t ubFps);

//------------------------------------------------------------------------
/*!
\brief Get Play Mode
\return Play Mode 0->Normal Play,1->Smooth Play
*/
KNL_PLY_MODE tKNL_GetPlyMode(void);

//------------------------------------------------------------------------
/*!
\brief Get Video FPS
\return Video FPS
*/
uint8_t ubKNL_GetVdoFps(void);

uint8_t ubKNL_GetChgResFlg(void);

void KNL_SetOnlineStaNum(uint8_t ubNum);
uint8_t ubKNL_GetOnlineStaNum(void);
void KNL_SendStaOnLineInfo(uint8_t ubStaMap);
//------------------------------------------------------------------------
/*!
\brief Set H264 GOP
\param ulGop H264 GOP
\return (no)
*/
void KNL_SetVdoGop(uint32_t ulGop);

//------------------------------------------------------------------------
/*!
\brief Get H264 GOP
\return GOP
*/
uint32_t ulKNL_GetVdoGop(void);

//------------------------------------------------------------------------
/*!
\brief Get H264 Frame Index
\return Frame Index
*/
uint32_t ulKNL_GetVdoFrmIdx(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief Get Scaler parameter
\return 0:Without Scaling
		1:(1/2) Scaling
		2:(1/4) Scaling
*/
uint8_t ubKNL_GetVdoScaleParam(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Get Data Bit-Rate
\param ubDataType Data type
\return Bit-Rate
*/
uint32_t ulKNL_GetDataBitRate(uint8_t ubDataType,uint8_t ubCodecIdx);

//------------------------------------------------------------------------
/*!
\brief Get FPS
\param tFpsType	FPS Type
\param ubSrcNum	Source number
\return FPS
*/
uint32_t ulKNL_GetFps(KNL_FPS_TYPE tFpsType,uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Initial kernel buffer
\return(no)
*/
void KNL_BufInit(void);

#if (defined(OP_STA)&&(defined(BSP_VBM_SDK)||defined(BSP_RVCS_SDK))&&defined(VDO_SUBPATH_ENABLE)&&(VDO_SUBPATH_ENABLE!=0))
void KNL_DpSwBufInit(void);
#endif
//------------------------------------------------------------------------
/*!
\brief Initial kernel block
\return(no)
*/
void KNL_BlockInit(void);

uint8_t ubKNL_SrcNumMap(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Get Record and Play Memory Size
\param ubMemMode	record and play is common or seperate.
\param eMode	    Source number mode
\return Record and Play Memory Size
*/
uint32_t ulKNL_GetRECnPLYMemorySize(REC_MEM_MODE_SEL ubMemMode,KNL_RP_SRC_MODE eMode);

//------------------------------------------------------------------------
/*!
\brief Registered Record and Play SDK hook function
\return(no)
*/
void KNL_RECnPLYCBFuncRegistered(void);

//------------------------------------------------------------------------
/*!
\brief Sync Time Stamp to Tx
\return(no)
*/
void KNL_SyncTimeStamp2TX(void);

//------------------------------------------------------------------------
/*!
\brief Set kernel role
\param ubRole	Role
\return(no)
*/
void KNL_SetRole(uint8_t ubRole);

//------------------------------------------------------------------------
/*!
\brief Get kernel role
\return Kernel role
*/
uint8_t ubKNL_GetRole(void);


//------------------------------------------------------------------------
/*!
\brief Set operation mode
\param ubOpMode	Operation mode
\return(no)
*/
void KNL_SetOpMode(uint8_t ubOpMode);

//------------------------------------------------------------------------
/*!
\brief Get opration mode
\return Operation mode
*/
uint8_t ubKNL_GetOpMode(void);

//------------------------------------------------------------------------
/*!
\brief Set auxiliary information function
\param ubEnable	0->Disable auxiliary information function,1->Enable auxiliary information function
\return(no)
*/
void KNL_SetAuxInfoFunc(uint8_t ubEnable);

//------------------------------------------------------------------------
/*!
\brief Get auxiliary informaton function status
\return Status : 0->Disable,1->Enable
*/
uint8_t ubKNL_GetAuxInfoFunc(void);

//------------------------------------------------------------------------
/*!
\brief Add auxiliary information
\param tPktType	Packet type
\param ubSrcNum	Source number
\param ulAddr 	Target address
\param ulSize 	Target size
\param ulFrmIdx	Frame index
\param ulGop	Video codec GOP
\param ubVdoGroupIdx Video group
\return 0->Fail\n
		1->Pass
*/
uint32_t ulKNL_AddAuxInfo(KNL_PACKET_TYPE tPktType,uint8_t ubSrcNum,uint32_t ulAddr,uint32_t ulSize,uint32_t ulFrmIdx,uint32_t ulGop,uint8_t ubVdoGroupIdx,uint32_t ulTime);

//------------------------------------------------------------------------
/*!
\brief Set video codec
\param ubVdoCodec	Video codec : 0->H264 Codec,1->JPEG Codec
\return(no)
*/
void KNL_SetVdoCodec(uint8_t ubVdoCodec);

//------------------------------------------------------------------------
/*!
\brief Get video codec
\return Video codec : 0->H264 Codec,1->JPEG Codec
*/
uint8_t ubKNL_GetVdoCodec(void);

//------------------------------------------------------------------------
/*!
\brief Set video horizontal resolution
\param ubSrcNum Source number
\param uwVdoH video horizontal resolution
\return (no)
*/
void KNL_SetVdoH(uint8_t ubSrcNum,uint16_t uwVdoH);

//------------------------------------------------------------------------
/*!
\brief Get video horizontal resolution
\param ubSrcNum Source number
\return Video horizontal resolution
*/
uint16_t uwKNL_GetVdoH(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Set video vertical resolution
\param ubSrcNum Source number
\param uwVdoV video vertical resolution
\return (no)
*/
void KNL_SetVdoV(uint8_t ubSrcNum,uint16_t uwVdoV);

//------------------------------------------------------------------------
/*!
\brief Get video vertical resolution
\param ubSrcNum Source number
\return Video vertical resolution
*/
uint16_t uwKNL_GetVdoV(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Reset video flow
\return(no)
*/
void KNL_VdoReset(void);

//------------------------------------------------------------------------
/*!
\brief Suspend video path
\param ubSrcNum Source number
\return(no)
*/
void KNL_VdoSuspend(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Resume video path
\param ubSrcNum Source number
\return(no)
*/
void KNL_VdoResume(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Start video flow
\param ubSrcNum Source number
\return(no)
*/
void KNL_VdoStart(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Stop video flow
\param ubSrcNum Source number
\return(no)
*/
void KNL_VdoStop(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief BB Datapath activity
\return(no)
*/
void KNL_SetTRXPathActivity(void);
//------------------------------------------------------------------------
/*!
\brief Disable manual control of BB Datapath
\return(no)
*/
void KNL_DisManuCtrlTRXPath(void);

//------------------------------------------------------------------------
/*!
\brief Check video flow activation
\param ubSrcNum Source number
\return 0->Without active\n
		1->Active
*/
uint8_t ubKNL_ChkVdoFlowAct(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Check IMG block ready or not
\return 0->Not ready\n
		1->Ready
*/
uint8_t ubKNL_ChkImgRdy(void);

uint8_t ubKNL_ChkBbInitEnd(void);
uint8_t ubKNL_ChkImgInitEnd(void);
uint8_t ubKNL_ChkCodecInitEnd(uint8_t ubCodecIdx);

//------------------------------------------------------------------------
/*!
\brief Get frame type
\param ulAddr Target address
\return 0->I frame\n
		1->P frame
*/
KNL_FRAME_TYPE tKNL_GetFrameType(uint32_t ulAddr);

//------------------------------------------------------------------------
/*!
\brief Set JPEG QP
\param ubQp : 0~255
\return(no)
*/
void KNL_SetJpegQp(uint8_t ubQp);

//------------------------------------------------------------------------
/*!
\brief Get JPEG QP
\return JPEG QP
*/
uint8_t ubKNL_GetJpegQp(void);

//------------------------------------------------------------------------
/*!
\brief Get communication link status
\param ubRole :
\return 0->Un-Link\n
		1->Link
*/
uint8_t ubKNL_GetCommLinkStatus(uint8_t ubRole);

uint8_t ubKNL_GetRtCommLinkStatus(uint8_t ubRole);
//------------------------------------------------------------------------
/*!
\brief Hook function for TWC
\param GetSta Target role : 0->TWC_STA1,1->TWC_STA2,2->TWC_STA3,3->TWC_STA4,4->TWC_AP_SLAVE,5->TWC_AP_MASTER
\param ubStatus Status : 0->TWC_SUCCESS,1->TWC_FAIL,2->TWC_BUSY
\return(no)
*/
void KNL_TwcResult(TWC_TAG GetSta,TWC_STATUS ubStatus);

//------------------------------------------------------------------------
/*!
\brief Send two way command
\param ubRole Destination device : 0->KNL_STA1,1->KNL_STA2,2->KNL_STA3,3->KNL_STA4,4->KNL_AP_SLAVE,5->KNL_AP_MASTER
\param Opc Operation command
\param *Data Pointer for TWC data
\param ubLen Length for TWC data
\param ubRetry Retry times for TWC
\return 0->Fail\n
		1->Success
*/
uint8_t ubKNL_TwcSend(uint8_t ubRole,TWC_OPC Opc,uint8_t *Data,uint8_t ubLen,uint8_t ubRetry);

//------------------------------------------------------------------------
/*!
\brief Reset node state
\return(no)
*/
void KNL_NodeStateReset(void);

//------------------------------------------------------------------------
/*!
\brief Set node state
\param ubSrcNum Source number
\param ubNode Node : 0~255
\param ubState State : 0->KNL_NODE_START,1->KNL_NODE_TRANS,0xFF->KNL_NODE_STOP
\return(no)
*/
void KNL_SetNodeState(uint8_t ubSrcNum,uint8_t ubNode,uint8_t ubState);

//------------------------------------------------------------------------
/*!
\brief Check node is finish or not
\param ubSrcNum Source number
\return 0->Not finish\n
		1->Finish
*/
uint8_t ubKNL_ChkNodeFinish(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Wait node to finish state
\param ubSrcNum Source number
\return 0->Fail\n
		1->Success
*/
uint8_t ubKNL_WaitNodeFinish(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Set Multiple output node
\param ubNode Node : 0~255
\param ubEnable 0->Disable,1->Enable
\param ubInSrc Input source
\param ubOutSrc1 Output source1
\param ubOutSrc2 Output Source2
\return(no)
*/
void KNL_SetMultiOutNode(uint8_t ubNode,uint8_t ubEnable,uint8_t ubInSrc,uint8_t ubOutSrc1,uint8_t ubOutSrc2);

//------------------------------------------------------------------------
/*!
\brief Check node is multiple output node or not
\param ubNode Node : 0~255
\return 0->Not multiple output node\n
		1->Multiple output node
*/
uint8_t ubKNL_ChkMultiOutNode(uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get output source
\param ubNode Node : 0~255
\param ubOutSrc Output Source : 0/1
\return Output source
*/
uint8_t ubKNL_GetMultiOutSrc(uint8_t ubNode,uint8_t ubOutSrc);

//------------------------------------------------------------------------
/*!
\brief Get input source
\param ubNode Node : 0~255
\return Input source
*/
uint8_t ubKNL_GetMultiInSrc(uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get node index
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return Index : 0~255
*/
uint8_t ubKNL_GetNodeIdx(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get node information
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return Node information
*/
KNL_NODE_INFO tKNL_GetNodeInfo(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get next node
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return Next node : 0~255
*/
uint8_t ubKNL_GetNextNode(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get previous node
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return Previous node : 0~255
*/
uint8_t ubKNL_GetPreNode(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Check node is exist or not
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return 0->Not exist\n
		1->Exist
*/
uint8_t ubKNL_ExistNode(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Check node is exist or not
\param ubNode Node : 0~255
\return 0->Not exist\n
		1->Exist
*/
uint8_t ubKNL_ChkExistNode(uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Reset video path
\return(no)
*/
void KNL_VdoPathReset(void);

//------------------------------------------------------------------------
/*!
\brief Set video path
\param ubSrcNum Source number
\param ubNodeIndex Node index : 0~255
\param tNodeInfo Node information
\return 0->Fail\n
		1->Success
*/
uint8_t ubKNL_SetVdoPathNode(uint8_t ubSrcNum,uint8_t ubNodeIndex,KNL_NODE_INFO tNodeInfo);

//------------------------------------------------------------------------
/*!
\brief Reset node information of vidio path.
\param ubSrcNum Source number
\return (no)
*/
void KNL_VdoPathNodeReset(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Show video path
\param ubSrcNum Source number
\return (no)
*/
void KNL_ShowVdoPathNode(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Get bit-stream buffer address
\param ubSrcNum Source number
\return Address
*/
uint32_t ulKNL_GetBsBufAddr(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Release bit-stream buffer
\param ubCurNode Current node
\param ubSrcNum Source number
\param ulBufAddr Buffer address
\return 0xA5(Fail)\n
			1(Success)
*/
uint8_t ubKNL_ReleaseBsBufAddr(uint8_t ubCurNode,uint8_t ubSrcNum,uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Get LCD buffer size
\return LCD buffer size
*/
uint32_t ulKNL_CalLcdBufSz(void);

//------------------------------------------------------------------------
/*!
\brief Set Cropping and Scale Parameter
\return 0(Fail)\n
			1(Success)
*/
uint8_t ubKNL_SetDispCropScaleParam(void);

//------------------------------------------------------------------------
/*!
\brief Check Lcd Display location status
\return 0(Fail)\n
		1(Ok)
*/
uint8_t KNL_ChkLcdDispLocation(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief LCD Display Setting
\return no
*/
void KNL_LcdDisplaySetting(void);

//------------------------------------------------------------------------
/*!
\brief Reset LCD Channel
\return (no)
*/
void KNL_ResetLcdChannel(void);

//------------------------------------------------------------------------
/*!
\brief Get LCD display address
\param ubSrcNum Source number
\return LCD display address
*/
uint32_t ulKNL_GetLcdDispAddr(uint8_t ubSrcNum);
//------------------------------------------------------------------------
/*!
\brief Check LCD display Ready
\param ubSrcNum Source number
\return 1 is ready or 0 is unready
*/
uint8_t ubKNL_ChkLcdDispReady(uint8_t ubSrcNum);
//------------------------------------------------------------------------
/*!
\brief Acitve LCD display buffer
\param ubSrcNum Source number
\return (no)
*/
void KNL_ActiveLcdDispBuf(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Set LCD dummy image horizontal resolution
\param uwH Horizontal resolution
\return (no)
*/
void KNL_SetLcdDmyImgH(uint16_t uwH);

//------------------------------------------------------------------------
/*!
\brief Get LCD dummy image horizontal resolution
\return Dummy image horizontal resolution
*/
uint16_t uwKNL_GetLcdDmyImgH(void);

//------------------------------------------------------------------------
/*!
\brief Set display horizontal/vertical resolution
\param uwDispH Display horizontal resolution
\param uwDispV Display vertical resolution
\return (no)
*/
void KNL_SetDispHV(uint16_t uwDispH,uint16_t uwDispV);

//------------------------------------------------------------------------
/*!
\brief Set display type
\param tDispType Display type : 
\return (no)
*/
void KNL_SetDispType(KNL_DISP_TYPE tDispType);

//------------------------------------------------------------------------
/*!
\brief Get display type
\return Display type
*/
KNL_DISP_TYPE tKNL_GetDispType(void);

//------------------------------------------------------------------------
/*!
\brief Get APP CFG Display Mode
\return Display Mode
*/
uint8_t ubKNL_GetAPPCfgDispMode(void);

//------------------------------------------------------------------------
/*!
\brief Set display source
\param ubDispLocation Display location
\param ubDispSrcNum Display source number
\return (no)
*/
void KNL_SetDispSrc(KNL_DISP_LOCATION tDispLocation,uint8_t ubDispSrcNum);

//------------------------------------------------------------------------
/*!
\brief Get display source
\param tDispLocation Display location
\return Display source
*/
uint8_t ubKNL_GetDispSrc(KNL_DISP_LOCATION tDispLocation);

//------------------------------------------------------------------------
/*!
\brief Get display location
\param ubSrcNum Display source
\return Display location : 0->KNL_DISP_LOCATION1,1->KNL_DISP_LOCATION2,2->KNL_DISP_LOCATION3,3->KNL_DISP_LOCATION4
*/
KNL_DISP_LOCATION tKNL_GetDispLocation(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Get source number by display location.
\param ubDispLoc 		Display location : 0->KNL_DISP_LOCATION1,1->KNL_DISP_LOCATION2,2->KNL_DISP_LOCATION3,3->KNL_DISP_LOCATION4
\return source number 	0->KNL_STA1, 1->KNL_STA2, 2->KNL_STA3, 3->KNL_STA4
*/
uint8_t ubKNL_SearchSrcByDispLoc(uint8_t ubDispLoc);

//------------------------------------------------------------------------
/*!
\brief Modify display type
\param tDispType 			Display type
\param ubDispSrcNum  		Display source number, use for display is single.
\param KNL_SrcLocateMap_t  	Display source number and location, use for display is single and dual(if config 4T/1R mode).
\code
	1. KNL_SwDispInfo.tSrcNum[0] = KNL_SRC_1_MAIN;
	   KNL_ModifyDispType(KNL_DISP_SINGLE, KNL_SwDispInfo);
	2. KNL_ModifyDispType(KNL_DISP_QUAD, KNL_SwDispInfo);
	3. If config 4T/1R mode.
	   KNL_SwDispInfo.ubSetupFlag   = TRUE;
	   KNL_SwDispInfo.tSrcNum[0] 	= KNL_SRC_1_MAIN;
	   KNL_SwDispInfo.tSrcNum[1] 	= KNL_SRC_2_MAIN;
	   KNL_SwDispInfo.tSrcLocate[0] = KNL_DISP_LOCATION1;
	   KNL_SwDispInfo.tSrcLocate[1] = KNL_DISP_LOCATION2;
	   KNL_ModifyDispType(KNL_DISP_DUAL_C, KNL_SwDispInfo);
	4. If config 2T/1R mode
	   KNL_SwDispInfo.ubSetupFlag = FALSE;
	   KNL_ModifyDispType(KNL_DISP_DUAL_C, KNL_SwDispInfo);
\endcode
\return (no)
*/
void KNL_ModifyDispType(KNL_DISP_TYPE tDispType, KNL_SrcLocateMap_t tSrcLocate);

//------------------------------------------------------------------------
/*!
\brief Set display rotate
\param tRotateType Rotate type
\return (no)
*/
void KNL_SetDispRotate(KNL_DISP_ROTATE tRotateType);

//------------------------------------------------------------------------
/*!
\brief Get display rotate
\return Display rotate
*/
KNL_DISP_ROTATE tKNL_GetDispRotate(void);

void KNL_ResetVdoProc(void);

//------------------------------------------------------------------------
/*!
\brief RX video input proecess.
\param ubSta 		Tx number
\param ulVdoAddr 	Buffer address of video bs.
\param ulVdoSize 	Video BS size.
\return Display rotate
*/
KNL_SRC tKNL_RxVdoInProcess(uint8_t ubSta, uint32_t ulVdoAddr, uint32_t ulVdoSize);

//------------------------------------------------------------------------
/*!
\brief Sensor yuv buffer process task
\param tProc Information for process
\return (no)
*/
void KNL_SenYuvBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief H264 Encode process task
\param tProc Information for process
\return (no)
*/
void KNL_H264EncProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief H264 Decode process task
\param tProc Information for process
\return (no)
*/
void KNL_H264DecProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief JPEG Decode1 process task
\param tProc Information for process
\return (no)
*/
void KNL_JpegDec1Process(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief JPEG Decode2 process task
\param tProc Information for process
\return (no)
*/
void KNL_JpegDec2Process(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief JPEG Encode process task
\param tProc Information for process
\return (no)
*/
void KNL_JpegEncProcess(KNL_PROCESS tProc);

#if defined(BUC_CU)
void KNL_VdoCodecProcOff(void);
void KNL_VdoCodecProcOn(void);
#endif
//------------------------------------------------------------------------
/*!
\brief Video bit-stream buffer1 process task
\param tProc Information for process
\return (no)
*/
void KNL_VdoBsBuf1Process(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Video bit-stream buffer2 process task
\param tProc Information for process
\return (no)
*/
void KNL_VdoBsBuf2Process(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Image merge buffer process task
\param tProc Information for process
\return (no)
*/
void KNL_ImgMergeBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Image horizontal merge process task
\param tProc Information for process
\return (no)
*/
void KNL_ImgMergeHProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Video transmit process task
\param tProc Information for process
\return (no)
*/
void KNL_BbTxVdoProcess(KNL_PROCESS tProc);		//For AMIC A7130
void KNL_BbTxVdoProcess2(KNL_PROCESS tProc);	//For Richwave RTC676x

//------------------------------------------------------------------------
/*!
\brief ADC buffer process task
\param tProc Information for process
\return (no)
*/
void KNL_AdcBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief ADC buffer retry process task
\param tProc Information for process
\return (no)
*/
void KNL_RetryAdcBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief DAC buffer process task
\param tProc Information for process
\return (no)
*/
void KNL_DacBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Sensor start
\param ubSrcNum Source number
\return (no)
*/
void KNL_SenStart(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Sensor stop
\param ubSrcNum Source number
\return (no)
*/
void KNL_SenStop(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Image stability
\return (no)
*/
void KNL_ImgStabNotifyFunc(void);
//------------------------------------------------------------------------
/*!
\brief Check sensor state change is done or not
\return 0->not yet\n
			1->done
*/
uint8_t ubKNL_ChkSenStateChangeDone(void);

//------------------------------------------------------------------------
/*!
\brief Image encode initial
\param CodecIdx encode codec index
\param uwVdoH Video horizontal resolution
\param uwVdoV Video vertical resolution
\return (no)
*/
void KNL_ImgEncInit(H264_ENCODE_INDEX CodecIdx,uint16_t uwVdoH,uint16_t uwVdoV);

//------------------------------------------------------------------------
/*!
\brief Image decode initial
\param CodecIdx decode codec index
\param uwVdoH Video horizontal resolution
\param uwVdoV Video vertical resolution
\return (no)
*/
void KNL_ImgDecInit(H264_DECODE_INDEX CodecIdx,uint16_t uwVdoH,uint16_t uwVdoV);

//------------------------------------------------------------------------
/*!
\brief Image decoder setup
\param ubSrcNum Source number
\return (no)
*/
void KNL_ImageDecodeSetup(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Image encoder setup
\param ubSrcNum Source number
\return (no)
*/
void KNL_ImageEncodeSetup(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Image encode
\param CodecIdx encode codec index
\param ulYuvAddr YUV buffer address
\param ulBsAddr Bit-stream buffer address
\return 0->Fail\n
			1->Success
*/
uint8_t ubKNL_ImgEnc(H264_ENCODE_INDEX CodecIdx,uint32_t ulYuvAddr,uint32_t ulBsAddr);

//------------------------------------------------------------------------
/*!
\brief Image decode
\param CodecIdx decode codec index
\param ulYuvAddr YUV buffer address
\param ulBsAddr Bit-stream buffer address
\return 0->Fail\n
			1->Success
*/
uint8_t ubKNL_ImgDec(H264_DECODE_INDEX CodecIdx,uint32_t ulYuvAddr,uint32_t ulBsAddr);

//------------------------------------------------------------------------
/*!
\brief Reset H264DecFlg value reference by Vdo Path.
\return (no)
*/
void KNL_H264DecFlgReset(void);

//------------------------------------------------------------------------
/*!
\brief Setup date stampe callback function of image controller.
\param pImgDsCb		Date stampe callback function
\return (no)
*/
void KNL_SetDsUpdCbFunc(pvImgDsUpdFunc pImgDsCb);

//------------------------------------------------------------------------
/*!
\brief Get image merge buffer address
\param ubSrcNum Source number
\return Buffer address
*/
uint32_t ulKNL_GetImgMergeBufAddr(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Get image merge buffer size
\return Buffer size
*/
uint32_t ulKNL_GetImgMergeBufSz(void);

//------------------------------------------------------------------------
/*!
\brief Image Monitor function
\param ReceiveResult H264 Codec Report
\return(no)
*/
void KNL_ImgMonitorFunc(struct IMG_RESULT ReceiveResult);

//------------------------------------------------------------------------
/*!
\brief Check audio flow activation
\param ubSrcNum Source number
\return 0->Without active\n
		1->Active
*/
uint8_t ubKNL_ChkAdoFlowAct(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido start
\param ubSrcNum Source number
\return (no)
*/
void KNL_AdoStart(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido stop
\param ubSrcNum Source number
\return (no)
*/
void KNL_AdoStop(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido resume
\param ubSrcNum Source number
\return (no)
*/
void KNL_AdoResume(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido suspend
\param ubSrcNum Source number
\return (no)
*/
void KNL_AdoSuspend(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Reset audio path
\return(no)
*/
void KNL_AdoPathReset(void);

//------------------------------------------------------------------------
/*!
\brief Alignment audio packet size
\param ulInputSz Input size
\return Output size
*/
uint32_t ulKNL_AlignAdoPktSz(uint32_t ulInputSz);

//------------------------------------------------------------------------
/*!
\brief Auido ADC start
\return (no)
*/
void KNL_AdcStart(void);

//------------------------------------------------------------------------
/*!
\brief Auido ADC stop
\return (no)
*/
void KNL_AdcStop(void);

//------------------------------------------------------------------------
/*!
\brief Auido DAC start
\param ubSrcNum Source number
\return (no)
*/
void KNL_DacStart(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido DAC stop
\param ubSrcNum Source number
\return (no)
*/
void KNL_DacStop(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Set audio path
\param ubSrcNum Source number
\param ubNodeIndex Node index : 0~255
\param tNodeInfo Node information
\return 0->Fail\n
		1->Success
*/
uint8_t ubKNL_SetAdoPathNode(uint8_t ubSrcNum,uint8_t ubNodeIndex,KNL_NODE_INFO tNodeInfo);
//------------------------------------------------------------------------
/*!
\brief Show audio path
\param ubSrcNum Source number
\return (no)
*/
void KNL_ShowAdoPathNode(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Check DAC flow activation
\param ubSrcNum Source number
\return 0->Without active\n
		1->Active
*/
uint8_t ubKNL_ChkDacFlowAct(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief DAC stop case
\return (no)
*/
void KNL_DacStopCase(void);
//------------------------------------------------------------------------
/*!
\brief JPEG Encode process
\param tEncodeFmt		Set encode format.
\param tEncodeNotify	Set notify mode when JPEG codec is finish.
\param uwH 				Set image size of horizontal.
\param uwV 				Set image size of vertical.
\param ulVdoAddr 		Start address of Image.
\param ulJpgAddr 		Start address of JPEG.
\return(no)
*/
uint8_t ubKNL_JPEGEncode(JPEG_CODEC_FMT_t tEncodeFmt, JPEG_CODEC_FN_MODE_t tEncodeNotify, uint16_t uwH, uint16_t uwV, uint32_t ulVdoAddr, uint32_t ulJpgAddr);
//------------------------------------------------------------------------
/*!
\brief JPEG Decode process
\param pKNL_NodeInfo	Image information
\param tDecodeFmt		Set decode format.
\param tDecodeNotify	Set notify mode when JPEG codec is finish.
\param uwH 				Set image size of horizontal.
\param uwV 				Set image size of vertical.
\param ulVdoAddr 		Start address of Image.
\param ulJpgAddr 		Start address of JPEG.
\return(no)
*/
uint8_t ubKNL_JPEGDecode(KNL_NODE_INFO *pKNL_NodeInfo, JPEG_CODEC_FMT_t tDecodeFmt, JPEG_CODEC_FN_MODE_t tDecodeNotify, uint16_t uwH, uint16_t uwV, uint32_t ulVdoAddr, uint32_t ulJpgAddr);

//------------------------------------------------------------------------
/*!
\brief Source number mapping to role number for video
\param VdoRoleMap_cb	Mapping callback function
\return(no)
*/
void KNL_SetVdoRoleInfoCbFunc(pvRoleMap2Src VdoRoleMap_cb);

//------------------------------------------------------------------------
/*!
\brief Source number mapping to role number for audio
\param AdoRoleMap_cb	Mapping callback function
\return(no)
*/
void KNL_SetAdoRoleInfoCbFunc(pvRoleMap2Src AdoRoleMap_cb);

//------------------------------------------------------------------------
/*!
\brief Get RSSI value
\param tKNL_Role		Kernel Role Number
\return RSSI Value
*/
uint8_t KNL_GetRssiValue(KNL_ROLE tKNL_Role);
//------------------------------------------------------------------------
/*!
\brief Get PER value
\param tKNL_Role		Kernel Role Number
\return (100 - PER) Value
*/
uint8_t KNL_GetPerValue(KNL_ROLE tKNL_Role);
//------------------------------------------------------------------------
/*!
\brief Get H264 Encoder Index
\return Encoder Index
*/
H264_ENCODE_INDEX tKNL_GetEncIdx(void);
	
//------------------------------------------------------------------------
/*!
\brief Enable WOR function
\return no
*/
void KNL_EnableWORFunc(void);

//------------------------------------------------------------------------
/*!
\brief Disable WOR function
\return no
*/
void KNL_DisableWORFunc(void);

//------------------------------------------------------------------------
/*!
\brief Wake-up device on WOR mode
\param tKNL_Role		Kernel Role Number
\param ubMode			Start/Stop wake-up function, 1:start, 0:stop
\return no
*/
uint8_t KNL_WakeupDevice(KNL_ROLE tKNL_Role, uint8_t ubMode);

#ifdef A7130
//------------------------------------------------------------------------
/*!
\brief Set WOR mode.
\param tWorMode			WOR mode
\return no
*/
void KNL_SetWORMode(SET_WOR_MODE tWorMode);

//------------------------------------------------------------------------
/*!
\brief Get WOR mode.
\return WOR mode
*/
SET_WOR_MODE tKNL_GetWORMode(void);
#endif

//------------------------------------------------------------------------
/*!
\brief Release Buffer of USBD
\return no
*/
void KNL_ReleaseUsbdBuf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Setup USBD information of Audio packet
\param ulAdoAddr		Audio data buffer address
\param ulAdoSize		Audio data buffer size
\return no
*/
uint32_t ulKNL_SetUsbdAdoPktInfo(uint32_t ulAdoAddr, uint32_t ulAdoSize);

//------------------------------------------------------------------------
/*!
\brief Update audio data throught USBD.
\param ulAdoPcmAddr		Audio data buffer address
\param ulAdoPcmSize		Audio data buffer size
\return no
*/
void KNL_UpdateUsbdAdoData(uint32_t ulAdoPcmAddr, uint32_t ulAdoPcmSize, ADO_SAMPLERATE SampleRate, ADO_CHANNEL_MODE Channel);

//------------------------------------------------------------------------
/*!
\brief Audio data of PTT
\param pAdoData			Audio data buffer address
\return no
*/
void KNL_RecvAdoDataFromApp(uint8_t *pAdoData, uint32_t ulLBA, uint32_t ulLBA_Cnt);

//------------------------------------------------------------------------
/*!
\brief Turn on tuning tool
\return no
*/
void KNL_TurnOnTuningTool(void);
//------------------------------------------------------------------------
/*!
\brief Turn off tuning tool
\return no
*/
void KNL_TurnOffTuningTool(void);
//------------------------------------------------------------------------
/*!
\brief Get mode of tuning tool
\return Tuning tool mode
*/
KNL_TuningMode_t KNL_GetTuningToolMode(void);
//------------------------------------------------------------------------
/*!
\brief Firmware upgrade function use SD card
\return no
*/
void KNL_SDUpgradeFwFunc(void);
//------------------------------------------------------------------------
/*!
\brief Resend I Frame function
\param tKNL_Role	Role number
\return no
*/
void KNL_ResendIframeFunc(KNL_ROLE tKNL_Role);
//------------------------------------------------------------------------
/*!
\brief Resend I Frame by displayMode 
\return no
*/
void KNL_ResendIframe(void);
//------------------------------------------------------------------------
/*!
\brief Modify the view type for USBD application
\param ubViewType	View type
\return no
*/
void KNL_ModifyUsbdViewType(uint8_t ubViewType);
//------------------------------------------------------------------------
/*!
\brief Audio encode activility for USBD application
\param ubAct	Activility
\return no
*/
void KNL_ActiveUsbdAdoEncFlag(uint8_t ubAct);
//------------------------------------------------------------------------
/*!
\brief Process UVC Set image infromation for USBD application
\param ubAct	Activility
\return no
*/
void KNL_RecvUsbdUvcSetImageInf(uint16_t uwHsize,uint16_t uwVsize);

typedef enum
{
	KNL_OK,
	KNL_ErrorNoCard,
	KNL_ErrorTimeout,
	KNL_ErrorFsFmt,
	KNL_ErrorCap = KNL_ErrorTimeout,
	KNL_ErrorCardNRdy = 0x7E,
	KNL_ErrorVdoPlay = 0x7F,
    KNL_ERR = 0x80,
    KNL_CARDIN,
	KNL_VDOREC_STOP,
    KNL_VDOREC_STOP_TIMEOUT,
	KNL_VDOREC_START,
	KNL_VDOPLAY_STOP,
}KNL_Status_t;

//------------------------------------------------------------------------
/*!
\brief Image merge fucntion for H View
\param ubSrcNum		Kernel source number
\return Merge result
*/
KNL_Status_t tKNL_StartUpImageMergeForDispH(uint8_t ubSrcNum);

typedef enum
{
	JPG_LCDCH_DISABLE,
	JPG_LCDCH_ENABLE,
}KNL_JpgLcdChCtrl_t;

//------------------------------------------------------------------------
/*!
\brief Enable or Disable LCD channel for JPEG
\param tJpgLcdDispAct	Enable or Disable LCD channel for JPEG
\par [Example]
\code
	//! Display jpeg on LCD panel.
	if(KNL_OK != tKNL_EnJpegLcdCh(JPG_LCDCH_ENABLE))
		return;
	OSD_LogoJpeg(JPEG_PICTURE);

	//! Display video image on LCD panel.
	tKNL_EnJpegLcdCh(JPG_LCDCH_DISABLE)
\endcode
\return Setup result
*/
KNL_Status_t tKNL_EnJpegLcdCh(KNL_JpgLcdChCtrl_t tJpgLcdDispAct);
//------------------------------------------------------------------------
/*!
\brief Get LCD channel status for JPEG
\return Status
*/
KNL_JpgLcdChCtrl_t tKNL_GetJpegLcdChCtrl(void);

typedef enum
{
	KNL_UVCO_FAIL,
	KNL_UVCO_RSTI,
	KNL_UVCO_OK,
}KNL_UsbdUvcRet_t;

KNL_UsbdUvcRet_t tKNL_UpdateMjUvcImage(uint32_t ulYuvAddr, uint8_t ubSrcNum);

void KNL_SetDmyPatColor(uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue);
void KNL_CreateBlackDmyPat(uint32_t ulBufAddr, uint32_t ulBufSize);

typedef void(*pvRecordNtyFunc)(uint8_t);

typedef enum
{
	KNL_RECORDING_STOP,
	KNL_RECORDING_START,
}KNL_RecordStatus_t;

typedef enum
{
	KNL_RECORDFUNC_DISABLE,
    KNL_RECORDFUNC_LOOP,
    KNL_RECORDFUNC_MANU,
	KNL_PHOTO_CAPTURE,
	KNL_PHOTO_PLAY,
    KNL_VIDEO_PLAY,
    KNL_VIDEO_DOWNLOAD,
    KNL_VIDEO_STOP,
}KNL_RecordFunc_t;

typedef struct KNL_OPEN_PROCESS
{
	FS_FILE_NAME_INFO_t FileName;
	FS_SRC_NUM SrcNum;
	uint32_t ulFirstClus;
	uint32_t ulFileSize;
	FS_NO_FAT_CHAIN_FLG NoFatChainFlag;
	uint8_t ubReserves[2];	//!< used for meeting a multiple of 4 
}KNL_READ_FILE_PROCESS_t;

typedef struct
{
	KNL_RecordFunc_t tRecordFunc;
	pvRecordNtyFunc pRecordStsNtyCb;
	uint8_t ubPhotoCapSrc[4];
    uint8_t ubNowPhotoCapSrc;
	uint16_t uwRecordGroupIdx;
	KNL_READ_FILE_PROCESS_t tPhotoPlayInfo;
    uint32_t ulVideoPlayIdx[4];
    uint8_t ubPlayFileNum;
    uint8_t ubBackupDispInfoflag;
	KNL_DISP_TYPE tPlayDispTye;
	KNL_RecordStatus_t tRecordSts;
    uint8_t ubRecOnceStopCBFlag;
    uint8_t tREC_TmLapse;
    KNL_FldType_t tSimFolder;
    uint16_t uwSimFldSelIdx;
    uint8_t ubPhotoLocResIdx;
	uint8_t ubReserves[2];
}KNL_RecordAct_t;
//------------------------------------------------------------------------
/*!
\brief Get Decode address if playback
\return decode address
*/
uint32_t ulKNL_GetResvDecAddr(void);
//------------------------------------------------------------------------
/*!
\brief Update Jpeg Header
\return no
*/
//void KNL_UpdateJpgHeader(void);
void KNL_UpdateJpgHeader(uint16_t uwH,uint16_t uwV);
//------------------------------------------------------------------------
/*!
\brief Search source number for capture function
\param ubSrcNum		Source Number
\return 0: Not match
        1: Match
*/
uint8_t ubKNL_SearchCapSrcNum(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Capture done
\param ubCapSrc			Source Number
\param pExif	 	    Extend information data zone
\param ulSize		    Extend information data size
\return 1:valid, 0:invalid
*/
uint8_t ubKNL_PhotoExifUpdate(uint8_t ubCapSrc, uint8_t *pExif, uint32_t ulSize);

//------------------------------------------------------------------------
/*!
\brief Capture done
\param ubCapNode	 	Capture node
\param ubCapSrc			Source Number
\param ulCapSize		Capture picture size
\return no
*/
void KNL_PhotoCaptureFinFunc(uint8_t ubCapNode, uint8_t ubCapSrc, uint32_t ulCapSize);
//------------------------------------------------------------------------
/*!
\brief Photo playback
\return no
*/
void KNL_PhotoPlayFinFunc(void);
//------------------------------------------------------------------------
/*!
\brief Set record function, capture or video recording
\param tRecordFunc Record function
\return no
*/
void KNL_SetRecordFunc(KNL_RecordFunc_t tRecordFunc);
//------------------------------------------------------------------------
/*!
\brief Get record function, capture or video recording
\return Record function, capture or video recording
*/
KNL_RecordFunc_t tKNL_GetRecordFunc(void);

//------------------------------------------------------------------------
void KNL_SetPlayDestFld(KNL_FldType_t tFldType);

//------------------------------------------------------------------------
KNL_FldType_t tKNL_GetPlayDestFld(void);

//------------------------------------------------------------------------
/*!
\brief Set local sensor isp resolution index,0=normal res.;1=scaleup res.
\return no
*/
void KNL_SetPhotoLocRes(uint8_t ubResIdx);

//------------------------------------------------------------------------
/*!
\brief Get local sensor isp resolution index,0=normal res.;1=scaleup res.
\return no
*/
uint8_t ubKNL_GetPhotoLocRes(void);

//------------------------------------------------------------------------
/*!
\brief Excute the record function, capture or video recording
\return Record function, capture or video recording
*/
KNL_Status_t tKNL_ExecRecordFunc(KNL_RecordAct_t tRecordAct);
//------------------------------------------------------------------------
uint8_t ubKNL_ChkFreeSizeValid( uint8_t ubType );
//------------------------------------------------------------------------
/*!
\brief chk garbage usage valid
\param ubUsageThreshold		percentage, range=0~100
\return 1:valid, 0:invalid
*/
uint8_t ubKNL_ChkGarbageUsageValid( uint8_t ubUsageThreshold );
//------------------------------------------------------------------------
/*!
\brief Set record capture Src
\param ubSrcNum
\return no
*/
void KNL_SetRecordCapSrc(uint8_t ubSrcNum);
//------------------------------------------------------------------------
/*!
\brief Get record  capture Src
\return ubSrcNum
*/
uint8_t ubKNL_GetRecordCapSrc(void);
//------------------------------------------------------------------------
/*!
\brief Excute the record function, capture or video recording
\return Record function, capture or video recording
*/
//------------------------------------------------------------------------
/*!
\brief Set Backup Display Info flag
\param ubFlag
\return no
*/
void KNL_SetBackupDispInfoFlag(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Get Backup Display Info flag
\return ubFlag 
*/
uint8_t ubKNL_GetBackupDispInfoFlag(void);
//------------------------------------------------------------------------
/*!
\brief Excute the record function, capture or video recording
\return Record function, capture or video recording
*/
void KNL_ExecRecordFunc(KNL_RecordAct_t tRecordAct);
//------------------------------------------------------------------------
/*!
\brief Revert Display Mode
\return no
*/
void KNL_RevertDisplayMode(void);
//------------------------------------------------------------------------
/*!
\brief Get display type for play back
\return Display type
*/
uint8_t ubKNL_GetPlayDispType(void);
//------------------------------------------------------------------------
/*!
\brief PlayBack output Video Fram 
\param ubCh	 	    Source Number
\param ulAddress	Frame Address
\param ulSize		Frame Size
\return 0: Fail
        1: Pass
*/
int32_t KNL_SdVideoPlayBack(uint8_t ubCh, uint32_t ulAddress, uint32_t ulSize);
//------------------------------------------------------------------------
/*!
\brief  REC File Formate Configuation 
\return no
*/
void KNL_RecFileFormatConfigInit(void);
//------------------------------------------------------------------------
/*!
\brief  REC File Create 
\param ubCh	 	    Source Number
\return no
*/
void KNL_RecFileCreate(uint8_t ubCh,FS_KNL_CRE_PROCESS_t *tRecProc);
//------------------------------------------------------------------------
/*!
\brief  Get Emergency file number 
\return Emergency file number 
*/
uint32_t ulKNL_GetEmergencyfileNumber(void);
void KNL_ResetH264IPCnt(void);
//------------------------------------------------------------------------
/*!
\brief  Stop video playback
\return (no)
*/
uint8_t KNL_VideoPlayStop(void);
void KNL_AdoLocalRecSendQProcess(uint32_t ulTimeStamp, uint32_t ulSize, uint32_t ulDramAddr);
KNL_Status_t tKNL_ChkSdCardSts(void);
void KNL_PhotoCaptureFunc(void);
#if (defined(BSP_RVCS_SDK)&&defined(BUC_CAM)&&APP_PHOTOGRAPH_FUNC_ENABLE)
KNL_SRC KNL_CamSnapshotSrcGet(void);
void KNL_CamPhotoCaptureFunc(void);
#endif
#if (OP_STA && APP_PLAY_REMOTE_ENABLE)
void KNL_PhotoPlayStop(void);
#endif
uint32_t ulKNL_GetPktFrmIdx(uint32_t ulAddr,uint32_t ulSize);
uint8_t ubKNL_GetPktVdoGop(uint32_t ulAddr,uint32_t ulSize);
uint8_t ubKNL_GetPaddLen(uint32_t ulAddr,uint32_t ulSize);
uint32_t ulKNL_GetTimeStamp1(uint32_t ulAddr,uint32_t ulSize);
uint32_t ulKNL_GetAdoPktSZ(uint32_t ulAddr,uint32_t ulSize);
//------------------------------------------------------------------------
//Extern
extern osMessageQId KNL_ProcessQueue;
extern osMessageQId tKNL_EncEventQue;		
extern osMessageQId tKNL_DecEventQue;

//------------------------------------------------------------------------
uint32_t KNL_TIMER_Get1ms(void);
void KNL_TIMER_Get1Sec(uint32_t *ulVal);
uint32_t ulKNL_ADO_GetMemInitValueAdr(void);
uint32_t ulKNL_ADO_GetWifiPacketSize(void);
void KNL_PlayTimeBarEvent_SDK(void);
void KNL_PlayOverEvent_SDK(PLY_EVENT tEvent);
void KNL_RecordOnceEnd_SDK(void);
void KNL_1MSCounterInit(void);
void KNL_ResendIActionExt(TWC_TAG GetSta,uint8_t *pData);
void KNL_ResendIActionInt(TWC_TAG GetSta,uint8_t *pData);
typedef void(*pvKNL_BbFrmOkCbFunc)(uint8_t ubBbFrmStatus);
void KNL_SetBbFrmMonitCbFunc(pvKNL_BbFrmOkCbFunc BbFrmOkCbFunc);

typedef void (*pvKNL_GetDataFromDevice1CbFunc)(uint32_t ulAddr,uint16_t uwLen);
typedef void (*pvKNL_GetDataFromDevice2CbFunc)(uint32_t ulAddr,uint16_t uwLen);
void KNL_SetDataFromDevice1CbFunc(pvKNL_GetDataFromDevice1CbFunc pCbFunc);
void KNL_SetDataFromDevice2CbFunc(pvKNL_GetDataFromDevice2CbFunc pCbFunc);
uint8_t ubKNL_GetHQImgCapEn(void);
void KNL_SetHQImgSize(uint16_t uwH, uint16_t uwV);
uint8_t ubKNL_HQImgCapSetup(uint8_t ubSrcNum, uint8_t ubFin, uint8_t ubCapRet);
void KNL_UvcCapture(uint16_t uwH, uint16_t uwV);
void KNL_doHighQualityImageCapture(void);
//------------------------------------------------------------------------

void KNL_SetTXRSlotNum(uint8_t ubSlotNum);
uint8_t ubKNL_GetTRXSlotNum(void);

uint8_t ubKNL_SendExtraData(uint32_t ulAddr,uint16_t uwLen);
	
// Richwave RF Driver
//#define	RF_MEMORY_POOL_SIZE			(192*1024)
#define	RF_MEMORY_POOL_SIZE			(272*1024)

// Richwave
void KNL_SetSocketFd(int fd);
int KNL_GetSocketFd(void);
#if RTC676x
//void KNL_EnterFwUpgradeMode(void);
//void KNL_QuitFwUpgradeMode(void);
//void KNL_StartChkVCTimeOut(void);	//Video/Command
//void KNL_StopChkVCTimeOut(void);	//Video/Command
//void KNL_StartChkATimeOut(void);	//Audio
//void KNL_StopChkATimeOut(void);	//Audio

//uint32_t ulRTC676x_GetRtBw(uint8_t ubMaxStaNum,uint8_t ubRole);
uint32_t ulRTC676x_GetRtBw(uint8_t ubRole);
uint32_t ulRTC676x_GetMaxBw(void);
void KNL_TrxBwRptFunc(TWC_TAG GetSta, uint8_t *pData);
#if (defined(OP_AP) && TRXBW_RPT_EN)
void KNL_SetTrxBwRpt(KNL_ROLE tRole, uint32_t ulBw);
uint32_t ulKNL_GetTrxBwRpt(KNL_ROLE tRole);
#endif
#endif
void KNL_SendResendICmd(uint8_t ubTargetRole);
uint8_t ubKNL_AccessLinkActiveTime(KNL_OPERATION tOperation,uint8_t ubValue);
uint8_t ubKNL_AccessErrCnt(KNL_OPERATION tOperation,uint8_t ubValue);

extern osMessageQId KNL_TwcMonitQueue;
extern KNL_MSG_Q_INFO_t KNL_PacketQue[4];	//Receive 0(Cam1)~Receive 3(Cam4)

//------------------------------------------------------------------------------
// Sorting file related
//------------------------------------------------------------------------------
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
#define KNL_TOTAL_SUB_FLD_NUM	(902+DISPLAY_MODE) //900 is for 100SONIX~999SONIX, 2 is for emergency and timelapse folder
#else
#define KNL_TOTAL_SUB_FLD_NUM	902		//900 is for 100SONIX~999SONIX, 2 is for emergency and timelapse folder
#endif
#define KNL_MAX_FILE_NUM		1000	//1000 is max supported file num

//-----------------------------------------------------------------------------
#pragma pack(push)
#pragma pack(1)
//-----------------------------------------------------------------------------
typedef enum 
{
	KNL_SORT_EQUAL,
	KNL_SORT_SMALL,
	KNL_SORT_BIG,
}KNL_SORT_COMPARE;
//------------------------------------------------------------------------------
typedef enum
{	
	SORT_BY_NAME_DESCENDING,            		//!< Folder Supported, File Supported
    SORT_BY_NAME_ASCENDING,                     //!< Folder Supported, File Supported
	SORT_BY_NAME_DESCENDING_BY_TIME,            //!< Folder Supported, File Supported
    SORT_BY_NAME_ASCENDING_BY_TIME,             //!< Folder Supported, File Supported
	SORT_BY_TIME_DESCENDING,                    //!< Folder Supported, File Supported
    SORT_BY_TIME_ASCENDING,                     //!< Folder Supported, File Supported
    SORT_BY_FILE_SIZE_DESCENDING,               //!< Folder No Supported, File Supported
    SORT_BY_FILE_SIZE_ASCENDING,                //!< Folder No Supported, File Supported
	SORT_INVALILD = 0xFF
}KNL_SORTMODE_t;
//------------------------------------------------------------------------------
typedef struct
{
	FS_FLD_NAME_INFO_t FldName;
	FS_FILE_PATH FilePath;
    uint8_t ubReserved[3];  //!< used for meeting a multiple of 4
}KNL_FOLDERSINFO_t;
//------------------------------------------------------------------------------
typedef struct
{
	FS_FILE_HIDDEN_INFO_t HidnFileInfo;
}KNL_FILESINFO_t;
//-----------------------------------------------------------------------------
#pragma pack(pop)
//-----------------------------------------------------------------------------
void KNL_FLD_FILE_BubbleSort(uint8_t ubType, uint8_t ubSortMode, uint16_t arr[], uint32_t len);
//-----------------------------------------------------------------------------
/*!
\brief Sorting folder infomation
\param tSimFld	      Simulate Fodler
\param tSortMode	  Soring mode
\param pFoldersInfo	  Folder information
\return Folder Number
*/
uint32_t ulKNL_GetSortingFolders(KNL_FldType_t tSimFld,KNL_SORTMODE_t tSortMode, KNL_FOLDERSINFO_t *pFoldersInfo);
//-----------------------------------------------------------------------------
/*!
\brief Sorting files infomation
\param uwFolderIndex  Folder index
\param tSortMode	  Soring mode
\param pFilesInfo	  Files information
\return File Number
*/
uint16_t uwKNL_GetSortingFiles(uint16_t uwFolderIndex, KNL_SORTMODE_t tSortMode, KNL_FILESINFO_t *pFilesInfo);
//-----------------------------------------------------------------------------
/*!
\brief Reset sorting result
\return no
*/
void KNL_ResetSortingResult(void);
//-----------------------------------------------------------------------------
/*!
\brief Get Folder infomation
\return FS_FLD_INFO_t
*/
FS_FLD_INFO_t KNL_GetFLDInfo(uint32_t ulIdx);
//-----------------------------------------------------------------------------
/*!
\brief  Get Sort Folder Resilt Table Mapping
\return Real index
*/
uint16_t uwKNL_GetSortFLDResult(uint32_t ulIdx);
//-----------------------------------------------------------------------------
/*!
\brief Get Hidden File infomation
\return FS_FILE_HIDDEN_INFO_t
*/
FS_FILE_HIDDEN_INFO_t KNL_GetFileHidInfo(KNL_FldType_t tSimFld,uint32_t ulIdx);
//-----------------------------------------------------------------------------
/*!
\brief  Get Sort Resilt Table Mapping
\return Real index
*/
uint16_t uwKNL_GetSortResult(uint32_t ulIdx);
//-----------------------------------------------------------------------------
/*!
\brief  Get CRC8 value for transmitting
\return CRC value
*/
uint8_t ubKNL_TransmitCRC8(uint32_t ulData_Addr, uint32_t ulData_Len);
//-----------------------------------------------------------------------------
typedef enum
{
	KNL_PERDBG_OFF,
	KNL_PERDBG_ON,
	KNL_LATYDBG_OFF,
	KNL_LATYDBG_ON,
}KNL_PerDbgMode_t;
typedef enum
{
	FRM_LOSS_ERR,
	FRM_CRC_ERR,
	FRM_TXBUF_ERR,
	FRM_DATA_ERR,
}KNL_PerDbgErrCode_t;
typedef enum
{
	PER_FRMSEQ_RPT,
	PER_FPS_RPT,
	PER_RFBW_RPT,
	PER_FRMSTS_RPT,
	PER_SPRFSTS_RPT,
	PER_SPRFFRML_RPT,
	PER_SPRFMINL_RPT,
	PER_SPRFAVGL_RPT,
}KNL_PerRptType_t;
typedef void (*pvPerDbgRptCbFunc)(uint8_t, uint8_t, uint16_t);
void KNL_EnPerDebugMode(uint8_t ubDbgEn, pvPerDbgRptCbFunc pSetupCbFunc);

void KNL_SetEvenSlot(int EvenSlot);
int KNL_GetEvenSlot(void);
int KNL_SetRwRemoteId(int *idv, int idvcnt);	//Set Richwave RTC676x Remote ID
int KNL_GetRwBwRpt(void);						//Used for AP
int KNL_GetCurRwBbRateMode(void);
void KNL_SetSysMaxFps(uint8_t ubFps);
void KNL_GetAhdInf(KNL_AHD_INFO *pData);
uint8_t ubKNL_GetSysMaxFps(void);
#ifdef RTC676x
uint8_t ubKNL_GetVdoPacketizeQueNum(void);
typedef enum
{
	KNL_ROLENUM,
	KNL_TWCTAG,
}KNL_StaType_t;
void KNL_UpdateStaInfo(uint8_t ubWorkSta, uint8_t ubUpdSta);
uint8_t ubKNL_GetStaInfo(KNL_StaType_t tStaType, uint8_t ubSta);
#endif

void KNL_SetSlotSwitchMode(KNL_SLOT_SWITCH tMode);
KNL_SLOT_SWITCH tKNL_GetSlotSwitchMode(void);

uint32_t ulKNL_GetFrameSz(uint32_t ulAddr,uint32_t ulSize);
uint32_t ulKNL_GetBsBufSz(uint8_t ubSrcNum);

extern uint8_t ubKNL_InitBBFlg;
extern uint8_t ubKNL_InitImgFlg;
extern uint8_t ubKNL_InitH264EncFlg[4];

#if (defined(BSP_DVR_SDK) || (defined(BSP_RVCS_SDK)&&defined(OP_STA)&& APP_PHOTOGRAPH_FUNC_ENABLE) )
void KNL_SetPlayBackIngFlg(uint8_t ubFlg);
uint8_t ubKNL_GetPlayBackIngFlg(void);
void KNL_SetLocalSnapshotYuvBuf(uint32_t ulBufAddr);
void KNL_SetLocalSnapshotBsBuf(uint32_t ulBufAddr);
uint32_t ulKNL_GetLocalSnapshotBsBuf(void);
void KNL_LocalSnapshotTrig(void);
void KNL_LocalSnapshotRelease(void);
uint8_t ubKNL_GetLocalSnapshotProc(void);
void KNL_SetLocalSnapShotSz(uint32_t ulBsSz);
uint32_t ulKNL_GetLocalSnapshotSz(void);
#endif
#if (defined(BSP_DVR_SDK))
void KNL_Recrpt(pvRecordNtyFunc pfunc);
uint8_t KNL_ChkSDPlugIn(void);
void KNL_LocalDpExec(uint8_t ubSenPath, uint32_t ulYuvAddr, uint32_t ulPathSize);
#endif
#if (APP_REC_FUNC_ENABLE && OP_STA)
#define TXREC_TIMEOUT_SCALE     10  // 10ms per count
#define TXREC_TIMEOUT_MS(x)     (x/TXREC_TIMEOUT_SCALE)
typedef enum
{
	KNL_TXREC_IDLE,
	KNL_TXREC_STOP,
	KNL_TXREC_WAITRECSTOP,
	KNL_TXREC_LOOP,
	KNL_TXREC_ONCE,
	KNL_TXREC_PHOTO,
	KNL_TXREC_WAITPHOTO,
	KNL_TXREC_WAITPHOTOFINISH,
	KNL_TXREC_SDFORMAT,
	KNL_TXREC_WAITFORMAT,
	KNL_TXREC_WAITRECSTOPFMT,
	KNL_TXREC_CHECKSD,
}KNL_TxRecMachine_t;
typedef enum
{
    KNL_TXREC_BUSY_NONE,
	KNL_TXREC_BUSY_PLAYSTOPVDO,
	KNL_TXREC_BUSY_PLAYSTOPPHOTO,
}KNL_TxRecBusyMode_t;
uint8_t ubKNL_GetTXRecMode(void);
void KNL_TXRecordFormat(void);
void KNL_TXRecordStop(void);
void KNL_TXRecordPhoto(void);
void KNL_TXRecordResume(uint8_t ubMode,uint8_t ubBusy);
void KNL_TXRecordResumeBusyClr(void);
uint8_t KNL_TXRecordResumeBusyMode(void);
void KNL_TXRecordReady(void);
#endif

typedef struct
{
    KNL_DISP_TYPE tPlayDispTye;
    uint8_t ubOrigFileNum;    
    FS_SRC_NUM ubOrigSrcNum[4];
    uint32_t ulOrigFileIdx[4];
    uint16_t uwOrigGpIdx;

    uint8_t ubCurFileNum;
    uint32_t ulCurFileIdx[4];
    uint16_t uwCurGpIdx;
    uint8_t ubCurFileInGPNum;
}KNL_PlayAct_t;
//-----------------------------------------------------------------------------
/*!
\brief Search next or previous file.
\param ubDirt =0,Search previous file; =1,Search next file.
\param ulFileIdx Sorted file index
\return Next or Previous Sorted file index
*/
uint16_t uwKNL_SortNexFile(KNL_FldType_t tSimFld,KNL_PLY_NextItem_t ubDirt, KNL_PlayAct_t *pPlayInfo, FS_FILE_HIDDEN_INFO_t *pPtr);
//-----------------------------------------------------------------------------
/*!
\brief Play next or previous file.
\param ubDirt =0,Play previous file; =1,Play next file.
\return Null
*/
uint32_t ulKNL_PlayNextFile(KNL_PLY_NextItem_t ubDirt);
//-----------------------------------------------------------------------------
/*!
\brief Select ado source and sent to remote
\param Sel		select path
\return Null
*/
void KNL_AdoSourceSelect(KNL_ADO_SRC_SEL Sel);
//-----------------------------------------------------------------------------
#ifdef RTC676x
uint8_t ubKNL_GetStopTrxFlg(void);
void KNL_ClearStopTrxFlg(void);
//int iKNL_SendPacket(COMM_DATA_TYPE tType,const void *buf, size_t len, int flags, int dest_id, uint64_t *ack);	
#endif
void KNL_SetFixDisplayLocateEnable(uint8_t ubFlg);
uint8_t ubKNL_GetFixDisplayLocateEnable(void);
void KNL_SetSenThenEncEnable(uint8_t ubFlg);
uint8_t ubKNL_GetSenThenEncEnable(void);
uint8_t ubKNL_GetIspEnable(void);
void KNL_SetAdjBufEnable(uint8_t ubFlg);
uint8_t ubKNL_GetAdjBufEnable(void);
#ifdef S2019A
void KNL_SetReEncIfrmFlag(uint8_t ubFlag);
uint8_t ubKNL_H264EncSetup(uint8_t ubCodecIdx);
void H264EncParamUpdate(KNL_PROCESS tProc);
uint32_t ulKNL_H264EncReport(uint8_t ubSrcNum, uint32_t ulEncBs, uint32_t ulFrmIdx);
void KNL_H264DecSetup(KNL_PROCESS tProc);
void KNL_sPRFDevPwrCtrl(sPRF_PwrCtrl_t tCtrl);
uint8_t ubKNL_sPRFRxProcess(sPRF_RxRpt_t tWiFi_RxRpt);
uint32_t ulKNL_GetSPRFRxBufAddr(sPRF_DevId_t tDevId, sPRF_PktType_t tPacketType, uint8_t ubBufType);
uint8_t ubKNL_GetSPRFDataBufNum(sPRF_DevId_t tDevId);
uint32_t ulKNL_GetBsOvfBufAddr(uint8_t ubSrcNum);
void KNL_StopSPRFTxProcess(uint8_t ubClrBuf);
void KNL_ResetVdoGop(sPRF_DevId_t tDevId, uint32_t ulGop);
uint32_t ulKNL_IspOutIsrProc(void);
uint32_t ulKNL_SenVsyncIsrProc(void);
void KNL_LcdVsncIsr(void);
void KNL_SetDrvMode(sPRF_DrvMode_t tDrvMd);
#ifdef RVCS_APP
void KNL_DrvMdSwSetup(void);
void KNL_SwitchRes(uint8_t ubSwitchDir);
void KNL_SetupRes(uint8_t ubReso);
float KNL_RC_ResoSwitch_Ratio(void);
#endif	//! End of #ifdef RVCS_APP
#endif	//! End of #ifdef S2019A
#if (defined(S2019A) || (defined(OP_AP) && defined(sWIFIBDG)))
void KNL_sPRFLinkReportFunc(sPRF_DevId_t tDevId, sPRF_LinkSts_t tLinkSts);
#if  (defined(RVCS_APP) || defined(sWIFIBDG))
void KNL_DrvModeSwProc(sPRF_DrvMode_t tDrvMd);
void KNL_EnStaDtDrvMode(void);
uint8_t ubKNL_ApBdgRxProc(sPRF_PktType_t tPacketType, KNL_SRC tBdgSrc, uint32_t ulDataAddr, uint32_t ulDataSize);
uint8_t ubKNL_ApBdgRxSendLostLinkImg(void);
uint8_t ubKNL_ApBdgTxProc(sPRF_PktType_t tPacketType, uint8_t *pData, uint32_t ulDataSize);
void KNL_SetApBdgPrvSrc(KNL_SRC tBdgSrc);
KNL_SRC tKNL_GetApBdgPrvSrc(void);
uint32_t tKNL_Get_StreamType(void);
#endif	//! End of #if  (defined(RVCS_APP) || defined(sWIFIBDG))
#endif	//! End of #if (defined(S2019A) || (defined(OP_AP) && defined(sWIFIBDG)))

#if (APP_DUAL_HOST_ENABLE == 1)
void KNL_GetFramFromUsbHost(uint8_t ubDeviceId,uint8_t ubFrameType,uint32_t ulAddr,uint32_t ulSize);
void KNL_UvcRdyEvent(uint8_t ubIndex);
void KNL_UvcErrEvent(uint8_t ubIndex);
void KNL_SetUvcChkFlg(uint8_t ubIdx,uint8_t ubFlg);
uint8_t uKNL_GetUvcChkFlg(uint8_t ubIdx);
#endif
#if (APP_DOORPHONE_ENABLE==1) 
uint8_t KNL_DP_GetVideoOnOff(uint8_t ubKNLSrcNum);
void KNL_DP_SetVideoOnOff(uint8_t ubKNLSrcNum, uint8_t ubValue);
#endif

#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
#if OP_AP
typedef struct
{
	uint8_t     ubFrmType;
	uint8_t		ubKnlSrcStaNum;
	uint8_t		ubStartPlayFg;
}KNL_RemotePlaySync_t;
typedef void(*pvKNL_TxFsCbFunc)(uint8_t ubType);
void KNL_TxFSInfoInstall(uint32_t *pFolder, uint32_t *pFile);
void KNL_SetTxFSCbFunc(pvKNL_TxFsCbFunc TxFsCbFunc);
uint8_t ubKNL_GetTXFldNum(void);
KNL_ROLE ubKNL_GetTXFldRole(uint8_t ubIdx);
void vKNL_SetPlayRole(KNL_ROLE eRole);
uint8_t ubKNL_GetRemotePlayFrmType(void);
uint8_t ubKNL_GetRemotePlayStaNump(void);
uint8_t ubKNL_CheckVdoTransmit(uint8_t ubKNL_ROLE);
#if defined(A7130)
void KNL_TXBBStart(uint8_t ubSrcNum);
#endif
#elif OP_STA
void KNL_TXFSSetFldLayer(KNL_TX_FLDLAYER tLayer);
void KNL_TXFSSort(uint8_t ubTrgger, uint8_t ubFSTp, uint32_t ulAdr, uint32_t ulSize);
void KNL_TXFSHidenInfo(uint8_t ubTrgger, uint8_t ubFSTp, uint16_t uwIdx);
uint8_t KNL_TXFSTriggerChk(void);
uint8_t ubKNL_GetBufUse(uint8_t ubCh);
typedef void(*pvKNL_RemotePlayCbFunc)(uint8_t tEvent,uint8_t ubData1,uint8_t ubData2,uint16_t uwIdx);
void KNL_SetRemoteFSCbFunc(pvKNL_RemotePlayCbFunc RP_CbFunc);
void KNL_SetRemotePlayCbFunc(pvKNL_RemotePlayCbFunc RP_CbFunc);
void KNL_PhotoPlayFuncSTA(KNL_RecordAct_t *pPhotoInfo);
void KNL_StartDownloadFile(uint16_t uwRecFileIdx);
#ifdef RVCS_APP
uint8_t KNL_VideoDownloadStop(void);
#endif
#endif
#endif

void KNL_PLY_ADOBufWrite(uint8_t EncType, uint32_t ulSrcAdr, uint32_t ulSize);
void KNL_TxAdoPly(uint32_t ulSrcAdr, uint32_t ulSize);

typedef struct KNL_THUMBNAIL_PROCESS_INFORMATION
{
	uint32_t ulAddr;
	uint32_t ulPixel_H;		//unit:1pixel, must be multiple of 4
	uint32_t ulPixel_V;		//unit:1pixel, must be multiple of 4
	uint32_t ulPixel_H_Ofs;	//unit:1pixel, must be multiple of 4
	uint32_t ulPixel_V_Ofs;	//unit:1pixel, must be multiple of 4
}KNL_THM_PROC_INFO_t;
void KNL_ShowOneThumbnailProcess(KNL_THM_PROC_INFO_t *SrcInfo, KNL_THM_PROC_INFO_t *DstInfo, uint32_t ulOutputThmPixel_H, uint32_t ulOutputThmPixel_V);

#if APP_FS_FILE_LIST_STYLE
typedef struct
{
	//----------------------------------------
	//for Buf
	//----------------------------------------
	uint32_t ulReadTempBufAddr;
	uint32_t ulDecYuvBufAddr;
	
	//----------------------------------------
	//for KNL
	//----------------------------------------
	uint8_t ubEnFlg;
	uint32_t ulLcdDispCurAddr;
	uint8_t ubLcdDispAddrKeepFlg;
	uint32_t ulLcdDispKeepAddr;
	osThreadId osThrdID;
	uint8_t ubShowingThmFlg;
	uint8_t ubDispRevertFlg;
	uint8_t ubRevers[3];
	//----------------------------------------
	//for UI
	//----------------------------------------
	uint8_t ubInFldListFlg;
	uint8_t ubVdoDataPathSetUpFirstFlg;
	uint16_t uwHSize_Now;
	uint16_t uwVSize_Now;
	uint16_t uwHSize_Pre;
	uint16_t uwVSize_Pre;
	uint16_t uwHSize_Display;
	uint16_t uwVSize_Display;
}KNL_THM_SHOW_INFO_t;
extern KNL_THM_SHOW_INFO_t KNL_ThmShowInfo;

void KNL_FillSameColorOnDisp(uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue, uint32_t ulDispAddr, uint32_t ulDispH, uint32_t ulDispV);
KNL_Status_t KNL_ShowingThm(uint32_t ulThmIdx, KNL_FILESINFO_t *FileInfo);
void KNL_FillWhiteOnLcd(uint16_t uwHSz, uint16_t uwVSz);
void KNL_ThumbnailSwtichView(void);
#endif

#if APP_PC_CONNECT_EN
void KNL_LeavePcConnect(void);
void KNL_EnterPcConnect(void);
typedef enum 
{
	KNL_PCCONN_WAIT,
	KNL_PCCONN_NO_UPDATE,
	KNL_PCCONN_UPDATE,
}KNL_PCCONN_FW_UPDATE_MODE;
typedef void(*KNL_PcConn_PcConnHook)(void);
typedef struct
{
	KNL_PcConn_PcConnHook EnterPcConnCb;
	KNL_PcConn_PcConnHook LeavePcConnCb;
	KNL_PcConn_PcConnHook OtherAction;
	KNL_PcConn_PcConnHook SdCardPlugoutCb;
	FS_FILE_NAME_INFO_t SdFwuFileName;
	FS_FILE_NAME_INFO_t EngModeFileName;	//don't delete fw file when thie name matched
}KNL_PCCONN_INIT_t;
void KNL_PcConnectInit(KNL_PCCONN_INIT_t *Info);
void KNL_PcConn_SetSdFwuMode(KNL_PCCONN_FW_UPDATE_MODE Mode);
uint8_t ubKNL_GetPcConnEnterStatus(void);	//0 is non-connection; 1 is connected
uint8_t ubKNL_GetPcConnSdFwuStatus(void);	//0 isn't processing fwu; 1 is processing fwu
#endif

void KNL_StoreDbgInfo(const char *cFuncName, uint32_t ulLine);
void KNL_PrintDbgInfo(void);
void KNL_AutoEnterFileListStart(void);
void KNL_AutoEnterFileListStop(void);

void KNL_ForceStopRec(void);

uint32_t ubKNL_GetRecDataRate(void);	//return unit:bytes/s
uint8_t ubKNL_CheckCvbs(void);
void KNL_SetSensorRes(uint16_t *puwH, uint16_t *puwV);
void KNL_SetYuvBufNub(uint8_t ubNb);
void KNL_AhdCamInSetResFunc(uint16_t uwH, uint16_t uwV);
void KNL_AhdCamInSetBufFunc(uint16_t uwH, uint16_t uwV);
//-----------------------------------------------------------------------
ALAW_RETURN_FLAG_t KNL_Alaw_Encode(uint32_t ulSrcAddr, uint32_t ulDstAddr, uint32_t ulInputSize, uint32_t *ulOutputSize);
ALAW_RETURN_FLAG_t KNL_Alaw_Decode(uint32_t ulSrcAddr, uint32_t ulDstAddr, uint32_t ulInputSize, uint32_t *ulOutputSize);

void KNL_Ado32EncInit(uint8_t ubUseIdx, ADO_SNX_AUD32_FORMAT Format);
void KNL_Ado32DecInit(uint8_t ubUseIdx, ADO_SNX_AUD32_FORMAT Format);

ADO_AUD32_ENC_INFO KNL_Ado32_Encode(uint8_t ubUseIdx, uint32_t ulSrcAddr, uint32_t ulDestAddr, uint32_t ulSize);
ADO_AUD32_DEC_INFO KNL_Ado32_Decode(uint8_t ubUseIdx, uint32_t ulSrcAddr, uint32_t ulDestAddr, uint32_t ulSize);

ADO_RETURN_FLAG KNL_AAC_EncInit(uint8_t ubUseIdx, uint8_t ubEncType, uint32_t ulSampleRate, uint32_t ulChannels);
ADO_RETURN_FLAG KNL_AAC_DecInit(uint8_t ubUseIdx, uint8_t ubDecType, uint32_t ulSampleRate, uint32_t ulChannels);
void KNL_AAC_EncUnInit(uint8_t ubUseIdx);
void KNL_AAC_DecUnInit(uint8_t ubUseIdx);
uint32_t ulKNL_AAC_GetEncUnitSize(uint8_t ubUseIdx);
ADO_RETURN_FLAG KNL_AAC_EncEncode(uint8_t ubUseIdx, uint32_t SrcAddr, uint32_t SrcSize, uint32_t DestAddr, uint32_t *DestSize);
ADO_RETURN_FLAG KNL_AAC_EncEncodeFlush(uint8_t ubUseIdx, uint32_t DestAddr, uint32_t *DestSize);
ADO_RETURN_FLAG KNL_AAC_DecDecode(uint8_t ubUseIdx, uint32_t SrcAddr, uint32_t SrcSize, uint32_t DestAddr, uint32_t *DestSize);
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
KNL_ISP_RESOLUTION ubKNL_GetIspResolution(void);
#endif
#if defined(OP_STA)
void KNL_MDResChBackup(uint8_t ubMd);
uint8_t ubKNL_MDResChRestore(void);
#endif
#endif
