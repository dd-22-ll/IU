/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		BUF.h
	\brief		Buffer header function
	\author		Justin Chen
	\version	1.12
	\date		2021/01/05
	\copyright	Copyright(C) 2021 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __BUF_h
#define __BUF_h

#include <stdio.h>
#include "_510PF.h"
#include "APP_CFG.h"
#include "BSP.h"

#define BUF_MAJORVER    1        //!< Major version = 1
#define BUF_MINORVER    12        //!< Minor version = 12

//Buffer Setting
//-----------------------------------------------------
// Stream Buffer
#ifdef S2019A
#define BUF_LFRM_NUM				2
#define BUF_LFRM_RATIO				2
#ifdef BSP_DVR_SDK
#define BUF_NUM_VDO_BS				(BUF_LFRM_NUM*BUF_LFRM_RATIO+17)
#elif defined(RVCS_APP)
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE )
#define BUF_NUM_VDO_BS				(3)//(BUF_LFRM_NUM*BUF_LFRM_RATIO+0)
#else
#define BUF_NUM_VDO_BS				(BUF_LFRM_NUM*BUF_LFRM_RATIO+26)
#endif
#else
#define BUF_NUM_VDO_BS				(BUF_LFRM_NUM*BUF_LFRM_RATIO+22)
#endif
#define BUF_NUM_ADC					4
#define BUF_NUM_DAC					6
#else
#if (defined(BSP_VBM_SDK) && APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE )
#define BUF_NUM_VDO_BS				6
#else
#define BUF_NUM_VDO_BS				9
#endif
#define BUF_NUM_ADC					3
#define BUF_NUM_DAC					3
#endif

#define BUF_NUM_VDO_BS_NOREC    	7
#if defined(BSP_DVR_SDK)
#define BUF_NUM_VDO_BS_STORAGE		8           //!<  VDO_MAIN_BS0
#endif

#if defined(OP_STA)
#define BUF_NUM_VDO_BS_SUBSTREAM    2           //!<  VDO_AUX_BS0
#endif

#ifdef S2019A
#define BUF_NUM_BDG_VDO_BS 			(BUF_NUM_VDO_BS+20)
#else
#define BUF_NUM_BDG_VDO_BS 			(30)
#endif

#define BUF_NUM_VDO_BS0				BUF_NUM_VDO_BS
#define BUF_NUM_VDO_BS1				BUF_NUM_VDO_BS
#define BUF_NUM_VDO_BS2				BUF_NUM_VDO_BS
#define BUF_NUM_VDO_BS3				BUF_NUM_VDO_BS

#define BUF_NUM_VDO_BS_ADJ			4


#define BUF_NUM_VDO_PACKETIZE_BIG	2
#define BUF_NUM_VDO_PACKETIZE		16	//Transmit(Video)
#define BUF_NUM_VDO_PACKETIZE_ADJ	14	//Transmit(Video)
#define BUF_NUM_ADO_PACKETIZE		8	//Transmit(Audio)

#define BUF_NUM_VDO_PACKETIZE_RCV	4	//Receive(Video)
#define BUF_NUM_ADO_PACKETIZE_RCV	4   //Receive(Audio)

//#define BUF_NUM_PACKET				32
#define BUF_NUM_PACKET				64
//#define BUF_NUM_PACKET				(64+64)

#define BUF_NUM_USBD_VDO			128
#define BUF_NUM_USBD_ADO			72

typedef enum
{
	BUF_FREE = 0,			//Buffer is free
	BUF_USED				//Buffer is used
}BUF_STATE;

typedef enum
{
	BUF_OK		= 1,		//Buffer access ok
	BUF_FAIL 	= 0xA5L,	//Buffer access fail
}BUF_RPT;

typedef enum
{
	BUF_SEN_1_YUV 		= 0,	//ISP Output1 YUV Data Buffer
	BUF_SEN_2_YUV 		= 1,	//ISP Output2 YUV Data Buffer
	BUF_SEN_3_YUV 		= 2,	//ISP Output3 YUV Data Buffer
	
	BUF_VDO_MAIN_BS0	= 3,	//Video Bitstream Data Buffer
	BUF_VDO_MAIN_BS1	= 4,	//Video Bitstream Data Buffer
	BUF_VDO_MAIN_BS2	= 5,	//Video Bitstream Data Buffer
	BUF_VDO_MAIN_BS3	= 6,	//Video Bitstream Data Buffer
	
	BUF_VDO_AUX_BS0		= 7,	//Video Bitstream Data Buffer
	BUF_VDO_AUX_BS1		= 8,	//Video Bitstream Data Buffer
	BUF_VDO_AUX_BS2		= 9,	//Video Bitstream Data Buffer
	BUF_VDO_AUX_BS3		= 10,	//Video Bitstream Data Buffer
	
	BUF_VDO_SUB_BS00	= 11,	//Video Bitstream Data Buffer
	BUF_VDO_SUB_BS10	= 12,	//Video Bitstream Data Buffer
	BUF_VDO_SUB_BS20	= 13,	//Video Bitstream Data Buffer
	BUF_VDO_SUB_BS30	= 14,	//Video Bitstream Data Buffer
	
	BUF_VDO_SUB_BS01	= 15,	//Video Bitstream Data Buffer
	BUF_VDO_SUB_BS11	= 16,	//Video Bitstream Data Buffer
	BUF_VDO_SUB_BS21	= 17,	//Video Bitstream Data Buffer
	BUF_VDO_SUB_BS31	= 18,	//Video Bitstream Data Buffer
	
	BUF_USBD_VDO		= 22,
	BUF_USBD_ADO		= 23,
	
	BUF_ADO_ADC			= 24,	//Audio ADC Buffer
	
	BUF_ADO_DAC0		= 25,	//Audio DAC Buffer
	BUF_ADO_DAC1		= 26,
	BUF_ADO_DAC2		= 27,
	BUF_ADO_DAC3		= 28,
	BUF_ADO_DAC4		= 29,
	BUF_ADO_DAC5		= 30,
   
    BUF_FS      		= 31,    
    BUF_REC      		= 32,
    
    BUF_USBH_IP			= 33,	//Used for USBH IP (Internal)
	
	BUF_JPG_BS			= 34,	//For Snapshot
	BUF_JPG_RAW			= 35,	//For Snapshot
#if !defined(BSP_DVR_SDK)
	BUF_RESV_YUV		= 36,

	BUF_RW_FRAME        = 37,   // Richwave frame buffer
	BUF_RF_DRIVER       = 38,   // Richwave RF Driver

	BUF_IMG_DS1			= 39,
#else
	BUF_JPG_BS2			= 36,	//For Local Sensor Preview @BUC_CU Side	
	
	BUF_RESV_YUV		= 37,
	BUF_TEMP_YUV		= 38,

	BUF_RW_FRAME        = 39,   // Richwave frame buffer
	BUF_RF_DRIVER       = 40,   // Richwave RF Driver

	BUF_IMG_DS1			= 41,
#endif
    BUF_DEC_YUV        = 50,

	BUF_sPRF,
	BUF_WIFI_DT,
	BUF_WIFI_BDG,

	BUF_PACKET0,				//Packet Buffer
	BUF_PACKET1,				//Packet Buffer
	BUF_PACKET2,				//Packet Buffer
	BUF_PACKET3,				//Packet Buffer
	

	BUF_VDO_PACKETIZE_BIG,		//Packetize Buffer for Transmit(Big Buffer)
	BUF_VDO_PACKETIZE_SMALL,	//Packetize Buffer for Transmit(Small Buffer)	
	BUF_ADO_PACKETIZE,			//Packetize Buffer for Transmit
	
	//Video
	BUF_VDO_PACKETIZE0,			//Packetize Buffer for Receive @Cam1
	BUF_VDO_PACKETIZE1,			//Packetize Buffer for Receive @Cam2
	BUF_VDO_PACKETIZE2,			//Packetize Buffer for Receive @Cam3
	BUF_VDO_PACKETIZE3,			//Packetize Buffer for Receive @Cam4	
	
	//Audio
	BUF_ADO_PACKETIZE0,			//Packetize Buffer for Receive @Cam1
	BUF_ADO_PACKETIZE1,			//Packetize Buffer for Receive @Cam2
	BUF_ADO_PACKETIZE2,			//Packetize Buffer for Receive @Cam3
	BUF_ADO_PACKETIZE3,			//Packetize Buffer for Receive @Cam4	
    
	BUF_SD,	

	BUF_MAC,

	BUF_THM_SHOWING_READ_TEMP,	// store the I frame temporarily reading from file
	BUF_THM_SHOWING_DEC_YUV,	// store the decoded frame from BUF_THM_SHOWING_READ_TEMP
	
    //Face Detection
    BUF_FD_RECT_OSD,            //Rectangle OSD for Face Detection

    BUF_VDO_TEMP1,
	
	BUF_USB_TRX,
	
	//[FOTA]
	BUF_FOTA,					//Used for FOTA Procedure

	BUF_IMG_ENC			= 0xF0,	//Used for IMG Enc IP (Internal)
	BUF_IMG_DEC			= 0xF1,	//Used for IMG Dec IP (Internal)
	BUF_ADO_IP			= 0xF2,	//Used for Audio IP (Internal)
	BUF_DS1_IP			= 0xF3,	//Used for DS1 IP (Internal)
	BUF_DS2_IP			= 0xF4,	//Used for DS2 IP (Internal)
	BUF_LCD_IP			= 0xF5,	//Used for LCD IP (Internal)
	BUF_BB_IP			= 0xF6,	//Used for BB IP (Internal)
	BUF_IMG_MERGE		= 0xF7,	//Used for IMG Merge IP (For Node)
	BUF_SEN_IP			= 0xF8,	//Used for SEN IP (Internal)
    BUF_USBD_IP			= 0xF9,	//Used for USBD IP (Internal)
    BUF_ISP_3DNR_IP 	= 0xFA,	//Used for ISP IP (3DNR)
    BUF_ISP_MD_W0_IP 	= 0xFB,	//Used for ISP IP (MD)
    BUF_ISP_MD_W1_IP 	= 0xFC,	//Used for ISP IP (MD)
    BUF_ISP_MD_W2_IP 	= 0xFD,	//Used for ISP IP (MD)
    BUF_IQ_BIN_FILE 	= 0xFE,	//Used for IQ

	BUF_USE_MAX			= 0xFF
}BUF_MODE;

//------------------------------------------------------------------------------
/*!
\brief 	Get BUF Function Version	
\return	Unsigned short value, high byte is the major version and low byte is the minor version
\par [Example]
\code		 
	 uint16_t uwVer;
	 
	 uwVer = uwBUF_GetVersion();
	 printf("BUF Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
\endcode
*/
uint16_t uwBUF_GetVersion (void);

//------------------------------------------------------------------------
/*!
\brief Initial BUF block
\param ulBUF_StartAddress Available memory address of DDR
\return(no)
*/
void BUF_Init(uint32_t ulBUF_StartAddress);

//------------------------------------------------------------------------
/*!
\brief Reset free buffer address
\return(no)
*/
void BUF_ResetFreeAddr(void);


//------------------------------------------------------------------------
/*!
\brief Reset buffer
\param ulAddr	Set Free Address position
\return(no)
*/
void BUF_SetFreeAddr(uint32_t ulAddr);
    
//------------------------------------------------------------------------
/*!
\brief Get free buffer address
\return Free buffer address
*/
uint32_t ulBUF_GetFreeAddr(void);

//------------------------------------------------------------------------
/*!
\brief Reset buffer
\param ubBufMode	Buffer mode
\return(no)
*/
void BUF_Reset(uint8_t ubBufMode);

//------------------------------------------------------------------------
/*!
\brief Buffer initial
\param ubBufMode	Buffer mode
\param ubBufNum	Buffer number
\param ulUnitSz	Unit size
\param ubIndex	Buffer index
\return(no)
*/
void BUF_BufInit(uint8_t ubBufMode,uint8_t ubBufNum,uint32_t ulUnitSz,uint8_t ubIndex);

//------------------------------------------------------------------------
/*!
\brief Alignment address to 1K
\param ulAddr Input address
\return Output addrss
*/
uint32_t ulBUF_AlignAddrTo1K(uint32_t ulAddr);

uint32_t ulBUF_AlignAddrTo512B(uint32_t ulAddr);

//------------------------------------------------------------------------
/*!
\brief Get SEN 1 yuv free buffer
\return Buffer address
*/
uint32_t ulBUF_GetSen1YuvFreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get SEN 2 yuv free buffer
\return Buffer address
*/
uint32_t ulBUF_GetSen2YuvFreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get SEN 3 yuv free buffer
\return Buffer address
*/
uint32_t ulBUF_GetSen3YuvFreeBuf(void);


uint32_t ulBUF_GetVdoPacketizeBigFreeBuf(void);
uint32_t ulBUF_GetVdoPacketizeSmallFreeBuf(void);


uint16_t uwBUF_GetVdoPacketizeBigFreeNum(void);
uint16_t uwBUF_GetVdoPacketizeSmallFreeNum(void);
uint16_t uwBUF_GetVdoPacketizeBigTotalNum(void);
uint16_t uwBUF_GetVdoPacketizeSmallTotalNum(void);
uint32_t ulBUF_GetAdoPacketizeFreeBuf(void);

//Video
uint32_t ulBUF_GetVdoPacketize0FreeBuf(void);
uint32_t ulBUF_GetVdoPacketize1FreeBuf(void);
uint32_t ulBUF_GetVdoPacketize2FreeBuf(void);
uint32_t ulBUF_GetVdoPacketize3FreeBuf(void);

uint32_t ulBUF_GetVdoPacketize0BufSz(void);
uint32_t ulBUF_GetVdoPacketize1BufSz(void);
uint32_t ulBUF_GetVdoPacketize2BufSz(void);
uint32_t ulBUF_GetVdoPacketize3BufSz(void);
//Audio
uint32_t ulBUF_GetAdoPacketize0FreeBuf(void);
uint32_t ulBUF_GetAdoPacketize1FreeBuf(void);
uint32_t ulBUF_GetAdoPacketize2FreeBuf(void);
uint32_t ulBUF_GetAdoPacketize3FreeBuf(void);

//Commmon
uint32_t ulBUF_GetPacket0FreeBuf(void);
uint32_t ulBUF_GetPacket1FreeBuf(void);
uint32_t ulBUF_GetPacket2FreeBuf(void);
uint32_t ulBUF_GetPacket3FreeBuf(void);

#if (defined(OP_AP) && ((defined(S2019A) && defined(RVCS_APP)) || defined(sWIFIBDG)))
uint32_t ulBUF_GetVdoBdgBsFreeBuf(void);
uint8_t ubBUF_ChkVdoBdgBsBufIdx(void);
uint8_t ubBUF_ReleaseVdoBdgBsBuf(uint32_t ulBufAddr);
uint8_t ubBUF_GetVdoBdgBsFreeBufNum(void);
#endif
#ifdef S2019A
uint8_t ubBUF_GetVdoMainBsFreeBufNum(BUF_MODE tBufMode);
uint32_t ulBUF_GetVdoMainBsFreeBuf(BUF_MODE tBufMode, uint8_t ubisIfrm);
uint8_t ubBUF_ChkVdoMainBsBufIdx(BUF_MODE tBufMode, uint8_t ubisIfrm);
#else	//! #ifdef S2019A
//------------------------------------------------------------------------
/*!
\brief Get VDO MAIN bit-stream[0] free buffer
\return Buffer address
*/

uint16_t uwBUF_GetVdoMainBs0FreeNum(void);
uint16_t uwBUF_GetVdoMainBs1FreeNum(void);
uint16_t uwBUF_GetVdoMainBs2FreeNum(void);
uint16_t uwBUF_GetVdoMainBs3FreeNum(void);

uint32_t ulBUF_GetVdoMainBs0FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO MAIN bit-stream[1] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoMainBs1FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO MAIN bit-stream[2] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoMainBs2FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO MAIN bit-stream[3] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoMainBs3FreeBuf(void);

#endif	//! Endo of #ifdef S2019A

//------------------------------------------------------------------------
/*!
\brief Get VDO AUX bit-stream[0] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoAuxBs0FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO AUX bit-stream[1] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoAuxBs1FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO AUX bit-stream[2] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoAuxBs2FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO AUX bit-stream[3] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoAuxBs3FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO SUB bit-stream[00] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoSubBs00FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO SUB bit-stream[10] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoSubBs10FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO SUB bit-stream[20] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoSubBs20FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO SUB bit-stream[30] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoSubBs30FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO SUB bit-stream[01] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoSubBs01FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO SUB bit-stream[11] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoSubBs11FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO SUB bit-stream[21] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoSubBs21FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get VDO SUB bit-stream[31] free buffer
\return Buffer address
*/
uint32_t ulBUF_GetVdoSubBs31FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get ADC free buffer
\return Buffer address
*/
uint32_t ulBUF_GetAdcFreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get DAC0 free buffer
\return Buffer address
*/
uint32_t ulBUF_GetDac0FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get DAC1 free buffer
\return Buffer address
*/
uint32_t ulBUF_GetDac1FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get DAC2 free buffer
\return Buffer address
*/
uint32_t ulBUF_GetDac2FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get DAC3 free buffer
\return Buffer address
*/
uint32_t ulBUF_GetDac3FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get DAC4 free buffer
\return Buffer address
*/
uint32_t ulBUF_GetDac4FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get DAC5 free buffer
\return Buffer address
*/
uint32_t ulBUF_GetDac5FreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Release SEN yuv buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseSenYuvBuf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release SEN 1 yuv buffer
\param ubBufIdx Buffer Number
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseSen1YuvBuf(uint8_t ubBufIdx);

//------------------------------------------------------------------------
/*!
\brief Release SEN 2 yuv buffer
\param ubBufIdx Buffer Number
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseSen2YuvBuf(uint8_t ubBufIdx);

//------------------------------------------------------------------------
/*!
\brief Release SEN 3 yuv buffer
\param ubBufIdx Buffer Number
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseSen3YuvBuf(uint8_t ubBufIdx);


uint8_t ubBUF_ReleaseVdoPacketizeBigBuf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleaseVdoPacketizeSmallBuf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleaseAdoPacketizeBuf(uint32_t ulBufAddr);

//Video
uint8_t ubBUF_ReleaseVdoPacketize0Buf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleaseVdoPacketize1Buf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleaseVdoPacketize2Buf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleaseVdoPacketize3Buf(uint32_t ulBufAddr);

//Audio
uint8_t ubBUF_ReleaseAdoPacketize0Buf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleaseAdoPacketize1Buf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleaseAdoPacketize2Buf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleaseAdoPacketize3Buf(uint32_t ulBufAddr);

uint8_t ubBUF_ReleasePacket0Buf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleasePacket1Buf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleasePacket2Buf(uint32_t ulBufAddr);
uint8_t ubBUF_ReleasePacket3Buf(uint32_t ulBufAddr);
//------------------------------------------------------------------------
/*!
\brief Release VDO MAIN bit-stream[0] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoMainBs0Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO MAIN bit-stream[1] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoMainBs1Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO MAIN bit-stream[2] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoMainBs2Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO MAIN bit-stream[3] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoMainBs3Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO AUX bit-stream[0] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoAuxBs0Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO AUX bit-stream[1] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoAuxBs1Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO AUX bit-stream[2] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoAuxBs2Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO AUX bit-stream[3] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoAuxBs3Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO SUB bit-stream[00] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoSubBs00Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO SUB bit-stream[10] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoSubBs10Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO SUB bit-stream[20] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoSubBs20Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO SUB bit-stream[30] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoSubBs30Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO SUB bit-stream[01] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoSubBs01Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO SUB bit-stream[11] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoSubBs11Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO SUB bit-stream[21] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoSubBs21Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release VDO SUB bit-stream[31] buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoSubBs31Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release ADC buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseAdcBuf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release DAC0 buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseDac0Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release DAC1 buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseDac1Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release DAC2 buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseDac2Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release DAC3 buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseDac3Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release DAC4 buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseDac4Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release DAC5 buffer
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseDac5Buf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Get block buffer address
\param ubIndex Buffer index
\param ubBufMode Buffer mode
\return Buffer address
*/
uint32_t ulBUF_GetBlkBufAddr(uint8_t ubIndex,uint8_t ubBufMode);

//------------------------------------------------------------------------
/*!
\brief Get VDO buffer of UVC address
\return Buffer address
*/
uint32_t ulBUF_GetVdoUsbdFreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Get ADO buffer of USBD address
\return Buffer address
*/
uint32_t ulBUF_GetAdoUsbdFreeBuf(void);

//------------------------------------------------------------------------
/*!
\brief Release VDO buffer of UVC
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseVdoUsbdBuf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release ADO buffer of USBD
\param ulBufAddr Buffer address
\return 0xA5->Fail\n
			1->Success
*/
uint8_t ubBUF_ReleaseAdoUsbdBuf(uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Release Video/Audio buffer of USBD
\return 0xA5->Fail\n
			1->Success
*/

uint32_t ulBuf_GetBsBufSize(uint8_t ubBufMode);

void BUF_ResetUsbdBuf(void);
// Richwave RF Driver
uint32_t ulBUF_GetRFDriverStartAddr(void);

// Richwave frame buffer
uint32_t ulBUF_GetRWFrameBufStartAddr(void);
#endif
