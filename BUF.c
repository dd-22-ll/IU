/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		BUF.c
	\brief		Buffer access function
	\author		Justin Chen
	\version	1.12
	\date		2024/03/13
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/

#include "BUF.h"
#include "DDR_API.h"

//------------------------------------------------------------------------------
/*!	\file BUF.c	
BUF Initial Flow Chart:	
	\dot
		digraph G {
	node [shape=record,fontsize=10];
	"BUF_Init" -> "BUF_BufInit";		
	}
	\enddot
*/
//------------------------------------------------------------------------------
osSemaphoreId tBUF_Sen1YuvAcc;
osSemaphoreId tBUF_Sen2YuvAcc;
osSemaphoreId tBUF_Sen3YuvAcc;

osSemaphoreId tBUF_MainBs0BufAcc;
osSemaphoreId tBUF_MainBs1BufAcc;
osSemaphoreId tBUF_MainBs2BufAcc;
osSemaphoreId tBUF_MainBs3BufAcc;
osSemaphoreId tBUF_AdcBufAcc;
osSemaphoreId tBUF_Dac0BufAcc;
osSemaphoreId tBUF_Dac1BufAcc;
osSemaphoreId tBUF_Dac2BufAcc;
osSemaphoreId tBUF_Dac3BufAcc;
osSemaphoreId tBUF_Dac4BufAcc;
osSemaphoreId tBUF_Dac5BufAcc;

osSemaphoreId tBUF_UsbdVdoAcc;
osSemaphoreId tBUF_UsbdAdoAcc;


osSemaphoreId tBUF_VdoPacketizeBigAcc;	//Transmit(VDO)
osSemaphoreId tBUF_VdoPacketizeSmallAcc;	//Transmit(VDO)
osSemaphoreId tBUF_AdoPacketizeAcc;	//Transmit(ADO)
osSemaphoreId tBUF_Packet0Acc;		//Receive(Common)
osSemaphoreId tBUF_Packet1Acc;		//Receive(Common)
osSemaphoreId tBUF_Packet2Acc;		//Receive(Common)
osSemaphoreId tBUF_Packet3Acc;		//Receive(Common)

osSemaphoreId tBUF_VdoPkt0Acc;
osSemaphoreId tBUF_VdoPkt1Acc;
osSemaphoreId tBUF_VdoPkt2Acc;
osSemaphoreId tBUF_VdoPkt3Acc;

osSemaphoreId tBUF_AdoPkt0Acc;
osSemaphoreId tBUF_AdoPkt1Acc;
osSemaphoreId tBUF_AdoPkt2Acc;
osSemaphoreId tBUF_AdoPkt3Acc;
//ISP Output (PATH1)
uint8_t ubBUF_Sen1YuvBufTag[2];
uint32_t ulBUF_Sen1YuvBufAddr[2];	
uint8_t ubBUF_Sen1YuvBufIdx;
uint32_t ulBUF_Sen1YuvBufSz;
uint8_t ubBUF_Sen1YuvBufNum;

//ISP Output (PATH2)
uint8_t ubBUF_Sen2YuvBufTag[2];
uint32_t ulBUF_Sen2YuvBufAddr[2];	
uint8_t ubBUF_Sen2YuvBufIdx;
uint32_t ulBUF_Sen2YuvBufSz;
uint8_t ubBUF_Sen2YuvBufNum;

//ISP Output (PATH3)
uint8_t ubBUF_Sen3YuvBufTag[2];
uint32_t ulBUF_Sen3YuvBufAddr[2];	
uint8_t ubBUF_Sen3YuvBufIdx;
uint32_t ulBUF_Sen3YuvBufSz;
uint8_t ubBUF_Sen3YuvBufNum;

//BS0 Main
uint8_t ubBUF_VdoMainBs0BufTag[BUF_NUM_VDO_BS0];
uint32_t ulBUF_VdoMainBs0BufAddr[BUF_NUM_VDO_BS0];	
uint8_t ubBUF_VdoMainBs0BufIdx;
uint32_t ulBUF_VdoMainBs0BufSz;
uint8_t ubBUF_VdoMainBs0BufNum = BUF_NUM_VDO_BS0;

//BS1 Main
uint8_t ubBUF_VdoMainBs1BufTag[BUF_NUM_VDO_BS1];
uint32_t ulBUF_VdoMainBs1BufAddr[BUF_NUM_VDO_BS1];	
uint8_t ubBUF_VdoMainBs1BufIdx;
uint32_t ulBUF_VdoMainBs1BufSz;
uint8_t ubBUF_VdoMainBs1BufNum = BUF_NUM_VDO_BS1;

//BS2 Main
uint8_t ubBUF_VdoMainBs2BufTag[BUF_NUM_VDO_BS2];
uint32_t ulBUF_VdoMainBs2BufAddr[BUF_NUM_VDO_BS2];	
uint8_t ubBUF_VdoMainBs2BufIdx;
uint32_t ulBUF_VdoMainBs2BufSz;
uint8_t ubBUF_VdoMainBs2BufNum = BUF_NUM_VDO_BS2;

//BS3 Main
uint8_t ubBUF_VdoMainBs3BufTag[BUF_NUM_VDO_BS3];
uint32_t ulBUF_VdoMainBs3BufAddr[BUF_NUM_VDO_BS3];	
uint8_t ubBUF_VdoMainBs3BufIdx;
uint32_t ulBUF_VdoMainBs3BufSz;
uint8_t ubBUF_VdoMainBs3BufNum = BUF_NUM_VDO_BS3;

//BS0 Aux
uint8_t ubBUF_VdoAuxBs0BufTag[BUF_NUM_VDO_BS0];
uint32_t ulBUF_VdoAuxBs0BufAddr[BUF_NUM_VDO_BS0];	
uint8_t ubBUF_VdoAuxBs0BufIdx;
uint32_t ulBUF_VdoAuxBs0BufSz;
uint8_t ubBUF_VdoAuxBs0BufNum = BUF_NUM_VDO_BS0;

//BS1 Aux
uint8_t ubBUF_VdoAuxBs1BufTag[BUF_NUM_VDO_BS1];
uint32_t ulBUF_VdoAuxBs1BufAddr[BUF_NUM_VDO_BS1];	
uint8_t ubBUF_VdoAuxBs1BufIdx;
uint32_t ulBUF_VdoAuxBs1BufSz;
uint8_t ubBUF_VdoAuxBs1BufNum = BUF_NUM_VDO_BS1;

//BS2 Aux
uint8_t ubBUF_VdoAuxBs2BufTag[BUF_NUM_VDO_BS2];
uint32_t ulBUF_VdoAuxBs2BufAddr[BUF_NUM_VDO_BS2];	
uint8_t ubBUF_VdoAuxBs2BufIdx;
uint32_t ulBUF_VdoAuxBs2BufSz;
uint8_t ubBUF_VdoAuxBs2BufNum = BUF_NUM_VDO_BS2;

//BS3 Aux
uint8_t ubBUF_VdoAuxBs3BufTag[BUF_NUM_VDO_BS3];
uint32_t ulBUF_VdoAuxBs3BufAddr[BUF_NUM_VDO_BS3];
uint8_t ubBUF_VdoAuxBs3BufIdx;
uint32_t ulBUF_VdoAuxBs3BufSz;
uint8_t ubBUF_VdoAuxBs3BufNum = BUF_NUM_VDO_BS3;

//Index 0
//BS0 Sub
uint8_t ubBUF_VdoSubBs00BufTag[BUF_NUM_VDO_BS0];
uint32_t ulBUF_VdoSubBs00BufAddr[BUF_NUM_VDO_BS0];	
uint8_t ubBUF_VdoSubBs00BufIdx;
uint32_t ulBUF_VdoSubBs00BufSz;
uint8_t ubBUF_VdoSubBs00BufNum = BUF_NUM_VDO_BS0;

//BS1 Sub
uint8_t ubBUF_VdoSubBs10BufTag[BUF_NUM_VDO_BS1];
uint32_t ulBUF_VdoSubBs10BufAddr[BUF_NUM_VDO_BS1];	
uint8_t ubBUF_VdoSubBs10BufIdx;
uint32_t ulBUF_VdoSubBs10BufSz;
uint8_t ubBUF_VdoSubBs10BufNum = BUF_NUM_VDO_BS1;

//BS2 Sub
uint8_t ubBUF_VdoSubBs20BufTag[BUF_NUM_VDO_BS2];
uint32_t ulBUF_VdoSubBs20BufAddr[BUF_NUM_VDO_BS2];	
uint8_t ubBUF_VdoSubBs20BufIdx;
uint32_t ulBUF_VdoSubBs20BufSz;
uint8_t ubBUF_VdoSubBs20BufNum = BUF_NUM_VDO_BS2;

//BS3 Sub
uint8_t ubBUF_VdoSubBs30BufTag[BUF_NUM_VDO_BS3];
uint32_t ulBUF_VdoSubBs30BufAddr[BUF_NUM_VDO_BS3];	
uint8_t ubBUF_VdoSubBs30BufIdx;
uint32_t ulBUF_VdoSubBs30BufSz;
uint8_t ubBUF_VdoSubBs30BufNum = BUF_NUM_VDO_BS3;

//Index 1
//BS0 Sub
uint8_t ubBUF_VdoSubBs01BufTag[BUF_NUM_VDO_BS0];
uint32_t ulBUF_VdoSubBs01BufAddr[BUF_NUM_VDO_BS0];	
uint8_t ubBUF_VdoSubBs01BufIdx;
uint32_t ulBUF_VdoSubBs01BufSz;
uint8_t ubBUF_VdoSubBs01BufNum = BUF_NUM_VDO_BS0;

//BS1 Sub
uint8_t ubBUF_VdoSubBs11BufTag[BUF_NUM_VDO_BS1];
uint32_t ulBUF_VdoSubBs11BufAddr[BUF_NUM_VDO_BS1];	
uint8_t ubBUF_VdoSubBs11BufIdx;
uint32_t ulBUF_VdoSubBs11BufSz;
uint8_t ubBUF_VdoSubBs11BufNum = BUF_NUM_VDO_BS1;

//BS2 Sub
uint8_t ubBUF_VdoSubBs21BufTag[BUF_NUM_VDO_BS2];
uint32_t ulBUF_VdoSubBs21BufAddr[BUF_NUM_VDO_BS2];	
uint8_t ubBUF_VdoSubBs21BufIdx;
uint32_t ulBUF_VdoSubBs21BufSz;
uint8_t ubBUF_VdoSubBs21BufNum = BUF_NUM_VDO_BS2;

//BS3 Sub
uint8_t ubBUF_VdoSubBs31BufTag[BUF_NUM_VDO_BS3];
uint32_t ulBUF_VdoSubBs31BufAddr[BUF_NUM_VDO_BS3];	
uint8_t ubBUF_VdoSubBs31BufIdx;
uint32_t ulBUF_VdoSubBs31BufSz;
uint8_t ubBUF_VdoSubBs31BufNum = BUF_NUM_VDO_BS3;

//ADC Buf Information
uint8_t ubBUF_AdcBufTag[BUF_NUM_ADC];
uint32_t ulBUF_AdcBufAddr[BUF_NUM_ADC];	
uint8_t ubBUF_AdcBufIdx;
uint32_t ulBUF_AdcBufSz;
uint8_t ubBUF_AdcBufNum;

//DAC Buf Information
//0
uint8_t ubBUF_Dac0BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac0BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac0BufIdx;
uint32_t ulBUF_Dac0BufSz;
uint8_t ubBUF_Dac0BufNum;

//1
uint8_t ubBUF_Dac1BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac1BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac1BufIdx;
uint32_t ulBUF_Dac1BufSz;
uint8_t ubBUF_Dac1BufNum;

//2
uint8_t ubBUF_Dac2BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac2BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac2BufIdx;
uint32_t ulBUF_Dac2BufSz;
uint8_t ubBUF_Dac2BufNum;

//3
uint8_t ubBUF_Dac3BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac3BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac3BufIdx;
uint32_t ulBUF_Dac3BufSz;
uint8_t ubBUF_Dac3BufNum;

//4
uint8_t ubBUF_Dac4BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac4BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac4BufIdx;
uint32_t ulBUF_Dac4BufSz;
uint8_t ubBUF_Dac4BufNum;

//5
uint8_t ubBUF_Dac5BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac5BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac5BufIdx;
uint32_t ulBUF_Dac5BufSz;
uint8_t ubBUF_Dac5BufNum;

//IP Buffer (Used for IP/Function Internal Only)
uint32_t ulBUF_ImgEncBufAddr[4];
uint32_t ulBUF_ImgDecBufAddr[4];
uint32_t ulBUF_AdoIpBufAddr;
uint32_t ulBUF_LcdIpBufAddr;
uint32_t ulBUF_BBIpBufAddr;
uint32_t ulBUF_ImgMergeIpBufAddr;
uint32_t ulBUF_SenIpBufAddr;
uint32_t ulBUF_UsbdIpBufAddr;
uint32_t ulBUF_UsbhIpBufAddr;
uint32_t ulBUF_3DNRIpBufAddr;
uint32_t ulBUF_MDw0IpBufAddr;
uint32_t ulBUF_MDw1IpBufAddr;
uint32_t ulBUF_MDw2IpBufAddr;
uint32_t ulBUF_IQbinBufAddr;
uint32_t ulBUF_VdoTemp1BufAddr;
uint32_t ulBUF_UsbTrx_BufAddr;

//IMG DS IP
uint32_t ulBUF_IMG_DS1BufAddr;

//FOTA
uint32_t ulBUF_FotaBufAddr = 0;
//Transmit(Video)


uint8_t ubBUF_VdoPacketizeBigBufTag[BUF_NUM_VDO_PACKETIZE];
uint8_t ubBUF_VdoPacketizeSmallBufTag[BUF_NUM_VDO_PACKETIZE];


uint32_t ulBUF_VdoPacketizeBigBufAddr[BUF_NUM_VDO_PACKETIZE];
uint32_t ulBUF_VdoPacketizeSmallBufAddr[BUF_NUM_VDO_PACKETIZE];


uint8_t ubBUF_VdoPacketizeBigBufIdx;
uint8_t ubBUF_VdoPacketizeSmallBufIdx;


uint32_t ulBUF_VdoPacketizeBigBufSz;
uint32_t ulBUF_VdoPacketizeSmallBufSz;


uint8_t ubBUF_VdoPacketizeBigBufNum;
uint8_t ubBUF_VdoPacketizeSmallBufNum;

//Transmit(Audio)
uint8_t ubBUF_AdoPacketizeBufTag[BUF_NUM_ADO_PACKETIZE];
uint32_t ulBUF_AdoPacketizeBufAddr[BUF_NUM_ADO_PACKETIZE];
uint8_t ubBUF_AdoPacketizeBufIdx;
uint32_t ulBUF_AdoPacketizeBufSz;
uint8_t ubBUF_AdoPacketizeBufNum;

//Receive(Video)
uint8_t ubBUF_VdoPacketize0BufTag[BUF_NUM_VDO_PACKETIZE_RCV];
uint32_t ulBUF_VdoPacketize0BufAddr[BUF_NUM_VDO_PACKETIZE_RCV];
uint8_t ubBUF_VdoPacketize0BufIdx;
uint32_t ulBUF_VdoPacketize0BufSz;
uint8_t ubBUF_VdoPacketize0BufNum;

uint8_t ubBUF_VdoPacketize1BufTag[BUF_NUM_VDO_PACKETIZE_RCV];
uint32_t ulBUF_VdoPacketize1BufAddr[BUF_NUM_VDO_PACKETIZE_RCV];
uint8_t ubBUF_VdoPacketize1BufIdx;
uint32_t ulBUF_VdoPacketize1BufSz;
uint8_t ubBUF_VdoPacketize1BufNum;

uint8_t ubBUF_VdoPacketize2BufTag[BUF_NUM_VDO_PACKETIZE_RCV];
uint32_t ulBUF_VdoPacketize2BufAddr[BUF_NUM_VDO_PACKETIZE_RCV];
uint8_t ubBUF_VdoPacketize2BufIdx;
uint32_t ulBUF_VdoPacketize2BufSz;
uint8_t ubBUF_VdoPacketize2BufNum;

uint8_t ubBUF_VdoPacketize3BufTag[BUF_NUM_VDO_PACKETIZE_RCV];
uint32_t ulBUF_VdoPacketize3BufAddr[BUF_NUM_VDO_PACKETIZE_RCV];
uint8_t ubBUF_VdoPacketize3BufIdx;
uint32_t ulBUF_VdoPacketize3BufSz;
uint8_t ubBUF_VdoPacketize3BufNum;

//Receive (Audio)
uint8_t ubBUF_AdoPacketize0BufTag[BUF_NUM_ADO_PACKETIZE_RCV];
uint32_t ulBUF_AdoPacketize0BufAddr[BUF_NUM_ADO_PACKETIZE_RCV];
uint8_t ubBUF_AdoPacketize0BufIdx;
uint32_t ulBUF_AdoPacketize0BufSz;
uint8_t ubBUF_AdoPacketize0BufNum;

uint8_t ubBUF_AdoPacketize1BufTag[BUF_NUM_ADO_PACKETIZE_RCV];
uint32_t ulBUF_AdoPacketize1BufAddr[BUF_NUM_ADO_PACKETIZE_RCV];
uint8_t ubBUF_AdoPacketize1BufIdx;
uint32_t ulBUF_AdoPacketize1BufSz;
uint8_t ubBUF_AdoPacketize1BufNum;

uint8_t ubBUF_AdoPacketize2BufTag[BUF_NUM_ADO_PACKETIZE_RCV];
uint32_t ulBUF_AdoPacketize2BufAddr[BUF_NUM_ADO_PACKETIZE_RCV];
uint8_t ubBUF_AdoPacketize2BufIdx;
uint32_t ulBUF_AdoPacketize2BufSz;
uint8_t ubBUF_AdoPacketize2BufNum;

uint8_t ubBUF_AdoPacketize3BufTag[BUF_NUM_ADO_PACKETIZE_RCV];
uint32_t ulBUF_AdoPacketize3BufAddr[BUF_NUM_ADO_PACKETIZE_RCV];
uint8_t ubBUF_AdoPacketize3BufIdx;
uint32_t ulBUF_AdoPacketize3BufSz;
uint8_t ubBUF_AdoPacketize3BufNum;

//For Cam1
uint8_t ubBUF_Packet0BufTag[BUF_NUM_PACKET];
uint32_t ulBUF_Packet0BufAddr[BUF_NUM_PACKET];
uint8_t ubBUF_Packet0BufIdx;
uint32_t ulBUF_Packet0BufSz;
uint8_t ubBUF_Packet0BufNum;

//For Cam2
uint8_t ubBUF_Packet1BufTag[BUF_NUM_PACKET];
uint32_t ulBUF_Packet1BufAddr[BUF_NUM_PACKET];
uint8_t ubBUF_Packet1BufIdx;
uint32_t ulBUF_Packet1BufSz;
uint8_t ubBUF_Packet1BufNum;

//For Cam3
uint8_t ubBUF_Packet2BufTag[BUF_NUM_PACKET];
uint32_t ulBUF_Packet2BufAddr[BUF_NUM_PACKET];
uint8_t ubBUF_Packet2BufIdx;
uint32_t ulBUF_Packet2BufSz;
uint8_t ubBUF_Packet2BufNum;

//For Cam4
uint8_t ubBUF_Packet3BufTag[BUF_NUM_PACKET];
uint32_t ulBUF_Packet3BufAddr[BUF_NUM_PACKET];
uint8_t ubBUF_Packet3BufIdx;
uint32_t ulBUF_Packet3BufSz;
uint8_t ubBUF_Packet3BufNum;
//JPG
uint32_t ulBUF_JpgBsBuffAddr[4];
uint32_t ulBUF_JpgRawBuffAddr;
uint32_t ulBUF_ResvYuvBuffAddr;
uint32_t ulBUF_DecodeYuvBuffAddr;

#if defined(BSP_DVR_SDK)
uint32_t ulBUF_TempYuvBuffAddr;
//JPG2,(Local Sensor Preview)
uint32_t ulBUF_JpgBs2BuffAddr;
#endif
//SD
uint32_t ulBUF_SdBufAddr;
//FS
uint32_t ulBUF_FsBufAddr;
//REC
uint32_t ulBUF_RecBufAddr;

//MAC
uint32_t ulBUF_MacBufAddr;

//Thumbnail showing
uint32_t ulBUF_ThmShowingRdTempBufAddr;
uint32_t ulBUF_ThmShowingDecYuvBufAddr;

//USBD path
uint8_t ubBUF_VdoUsbdBufTag[BUF_NUM_USBD_VDO];
uint32_t ulBUF_VdoUsbdBufAddr[BUF_NUM_USBD_VDO];	
uint8_t ubBUF_VdoUsbdBufNum;
uint8_t ubBUF_AdoUsbdBufTag[BUF_NUM_USBD_ADO];
uint32_t ulBUF_AdoUsbdBufAddr[BUF_NUM_USBD_ADO];
uint8_t ubBUF_AdoUsbdBufNum;

// Richwave RF Driver
uint32_t ulBUF_RFDriverBufAddr;

// Richwave frame buffer
uint32_t ulBUF_RWFrameBufAddr;

// OSD Rectangle for Face Detection
uint32_t ulBUF_FDRectOsdBufAddr;

uint32_t ulBUF_FreeBufAddr;
uint32_t ulBUF_InitFreeBufAddr;

#if (defined(OP_AP) && ((defined(S2019A) && defined(RVCS_APP)) || defined(sWIFIBDG)))
// Bridge Mode
uint8_t ubBUF_VdoBdgBsBufTag[BUF_NUM_BDG_VDO_BS];
uint32_t ulBUF_VdoBdgBsBufAddr[BUF_NUM_BDG_VDO_BS];	
uint8_t ubBUF_VdoBdgBsBufIdx;
uint32_t ulBUF_VdoBdgBsBufSz;
uint8_t ubBUF_VdoBdgBsBufNum = BUF_NUM_BDG_VDO_BS;
#endif

uint16_t uwBUF_GetVersion (void)
{
    return ((BUF_MAJORVER << 8) + BUF_MINORVER);
}

void BUF_Init(uint32_t ulBUF_StartAddress)
{
	ulBUF_FreeBufAddr = ulBUF_InitFreeBufAddr = ulBUF_StartAddress;	
	
	osSemaphoreDef(tBUF_Sen1YuvAcc);
	tBUF_Sen1YuvAcc	= osSemaphoreCreate(osSemaphore(tBUF_Sen1YuvAcc), 1);
	osSemaphoreDef(tBUF_Sen2YuvAcc);
	tBUF_Sen2YuvAcc	= osSemaphoreCreate(osSemaphore(tBUF_Sen2YuvAcc), 1);
	osSemaphoreDef(tBUF_Sen3YuvAcc);
	tBUF_Sen3YuvAcc	= osSemaphoreCreate(osSemaphore(tBUF_Sen3YuvAcc), 1);
	
	osSemaphoreDef(tBUF_MainBs0BufAcc);
	tBUF_MainBs0BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_MainBs0BufAcc), 1);
	osSemaphoreDef(tBUF_MainBs1BufAcc);
	tBUF_MainBs1BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_MainBs1BufAcc), 1);
	osSemaphoreDef(tBUF_MainBs2BufAcc);
	tBUF_MainBs2BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_MainBs2BufAcc), 1);
	osSemaphoreDef(tBUF_MainBs3BufAcc);
	tBUF_MainBs3BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_MainBs3BufAcc), 1);	
	
	osSemaphoreDef(tBUF_AdcBufAcc);
	tBUF_AdcBufAcc	= osSemaphoreCreate(osSemaphore(tBUF_AdcBufAcc), 1);	
	osSemaphoreDef(tBUF_Dac0BufAcc);
	tBUF_Dac0BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac0BufAcc), 1);		
	osSemaphoreDef(tBUF_Dac1BufAcc);
	tBUF_Dac1BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac1BufAcc), 1);	
	osSemaphoreDef(tBUF_Dac2BufAcc);
	tBUF_Dac2BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac2BufAcc), 1);	
	osSemaphoreDef(tBUF_Dac3BufAcc);
	tBUF_Dac3BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac3BufAcc), 1);	
	osSemaphoreDef(tBUF_Dac4BufAcc);
	tBUF_Dac4BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac4BufAcc), 1);	
	osSemaphoreDef(tBUF_Dac5BufAcc);
	tBUF_Dac5BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac5BufAcc), 1);

	osSemaphoreDef(tBUF_UsbdVdoAcc);
	tBUF_UsbdVdoAcc	= osSemaphoreCreate(osSemaphore(tBUF_UsbdVdoAcc), 1);
	osSemaphoreDef(tBUF_UsbdAdoAcc);
	tBUF_UsbdAdoAcc	= osSemaphoreCreate(osSemaphore(tBUF_UsbdAdoAcc), 1);
	

	osSemaphoreDef(tBUF_VdoPacketizeBigAcc);
	tBUF_VdoPacketizeBigAcc	= osSemaphoreCreate(osSemaphore(tBUF_VdoPacketizeBigAcc), 1);
	osSemaphoreDef(tBUF_VdoPacketizeSmallAcc);
	tBUF_VdoPacketizeSmallAcc	= osSemaphoreCreate(osSemaphore(tBUF_VdoPacketizeSmallAcc), 1);
	
	osSemaphoreDef(tBUF_AdoPacketizeAcc);
	tBUF_AdoPacketizeAcc	= osSemaphoreCreate(osSemaphore(tBUF_AdoPacketizeAcc), 1);
	
	osSemaphoreDef(tBUF_Packet0Acc);
	tBUF_Packet0Acc	= osSemaphoreCreate(osSemaphore(tBUF_Packet0Acc), 1);
	osSemaphoreDef(tBUF_Packet1Acc);
	tBUF_Packet1Acc	= osSemaphoreCreate(osSemaphore(tBUF_Packet1Acc), 1);
	osSemaphoreDef(tBUF_Packet2Acc);
	tBUF_Packet2Acc	= osSemaphoreCreate(osSemaphore(tBUF_Packet2Acc), 1);
	osSemaphoreDef(tBUF_Packet3Acc);
	tBUF_Packet3Acc	= osSemaphoreCreate(osSemaphore(tBUF_Packet3Acc), 1);
	
	osSemaphoreDef(tBUF_VdoPkt0Acc);
	tBUF_VdoPkt0Acc	= osSemaphoreCreate(osSemaphore(tBUF_VdoPkt0Acc), 1);	
	osSemaphoreDef(tBUF_VdoPkt1Acc);
	tBUF_VdoPkt1Acc	= osSemaphoreCreate(osSemaphore(tBUF_VdoPkt1Acc), 1);
	osSemaphoreDef(tBUF_VdoPkt2Acc);
	tBUF_VdoPkt2Acc	= osSemaphoreCreate(osSemaphore(tBUF_VdoPkt2Acc), 1);
	osSemaphoreDef(tBUF_VdoPkt3Acc);
	tBUF_VdoPkt3Acc	= osSemaphoreCreate(osSemaphore(tBUF_VdoPkt3Acc), 1);
	
	osSemaphoreDef(tBUF_AdoPkt0Acc);
	tBUF_AdoPkt0Acc	= osSemaphoreCreate(osSemaphore(tBUF_AdoPkt0Acc), 1);	
	osSemaphoreDef(tBUF_AdoPkt1Acc);
	tBUF_AdoPkt1Acc	= osSemaphoreCreate(osSemaphore(tBUF_AdoPkt1Acc), 1);
	osSemaphoreDef(tBUF_AdoPkt2Acc);
	tBUF_AdoPkt2Acc	= osSemaphoreCreate(osSemaphore(tBUF_AdoPkt2Acc), 1);
	osSemaphoreDef(tBUF_AdoPkt3Acc);
	tBUF_AdoPkt3Acc	= osSemaphoreCreate(osSemaphore(tBUF_AdoPkt3Acc), 1);
}

void BUF_ResetFreeAddr(void)
{
	ulBUF_FreeBufAddr = ulBUF_InitFreeBufAddr;
}

void BUF_SetFreeAddr(uint32_t ulAddr)
{
    ulBUF_FreeBufAddr = ulAddr;
}

uint32_t ulBUF_GetFreeAddr(void)
{
	return ulBUF_FreeBufAddr;
}

void BUF_Reset(uint8_t ubBufMode)
{
	uint8_t i;
	
	if(ubBufMode == BUF_IMG_ENC)
	{		
	}
	else if(ubBufMode == BUF_IMG_DEC)
	{		
	}
	else if(ubBufMode == BUF_SEN_1_YUV)
	{
		for(i=0;i<ubBUF_Sen1YuvBufNum;i++)		
		{
			ubBUF_Sen1YuvBufTag[i] = BUF_FREE;						
		}
		ubBUF_Sen1YuvBufIdx = ubBUF_Sen1YuvBufNum-1;		
	}
	else if(ubBufMode == BUF_SEN_2_YUV)
	{
		for(i=0;i<ubBUF_Sen2YuvBufNum;i++)		
		{
			ubBUF_Sen2YuvBufTag[i] = BUF_FREE;						
		}
		ubBUF_Sen2YuvBufIdx = ubBUF_Sen2YuvBufNum-1;		
	}
	else if(ubBufMode == BUF_SEN_3_YUV)
	{
		for(i=0;i<ubBUF_Sen3YuvBufNum;i++)		
		{
			ubBUF_Sen3YuvBufTag[i] = BUF_FREE;						
		}
		ubBUF_Sen3YuvBufIdx = ubBUF_Sen3YuvBufNum-1;		
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS0)
	{
		for(i=0;i<ubBUF_VdoMainBs0BufNum;i++)
		{
			ubBUF_VdoMainBs0BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoMainBs0BufIdx 	= ubBUF_VdoMainBs0BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS1)
	{
		for(i=0;i<ubBUF_VdoMainBs1BufNum;i++)
		{
			ubBUF_VdoMainBs1BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoMainBs1BufIdx 	= ubBUF_VdoMainBs1BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS2)
	{
		for(i=0;i<ubBUF_VdoMainBs2BufNum;i++)
		{
			ubBUF_VdoMainBs2BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoMainBs2BufIdx 	= ubBUF_VdoMainBs2BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS3)
	{
		for(i=0;i<ubBUF_VdoMainBs3BufNum;i++)
		{
			ubBUF_VdoMainBs3BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoMainBs3BufIdx 	= ubBUF_VdoMainBs3BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS0)
	{
		for(i=0;i<ubBUF_VdoAuxBs0BufNum;i++)
		{
			ubBUF_VdoAuxBs0BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoAuxBs0BufIdx 	= ubBUF_VdoAuxBs0BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS1)
	{
		for(i=0;i<ubBUF_VdoAuxBs1BufNum;i++)
		{
			ubBUF_VdoAuxBs1BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoAuxBs1BufIdx = ubBUF_VdoAuxBs1BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS2)
	{
		for(i=0;i<ubBUF_VdoAuxBs2BufNum;i++)
		{
			ubBUF_VdoAuxBs2BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoAuxBs2BufIdx = ubBUF_VdoAuxBs2BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS3)
	{
		for(i=0;i<ubBUF_VdoAuxBs3BufNum;i++)
		{
			ubBUF_VdoAuxBs3BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoAuxBs3BufIdx = ubBUF_VdoAuxBs3BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS00)
	{
		for(i=0;i<ubBUF_VdoSubBs00BufNum;i++)
		{
			ubBUF_VdoSubBs00BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs00BufIdx 	= ubBUF_VdoSubBs00BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS10)
	{
		for(i=0;i<ubBUF_VdoSubBs10BufNum;i++)
		{
			ubBUF_VdoSubBs10BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs10BufIdx 	= ubBUF_VdoSubBs10BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS20)
	{
		for(i=0;i<ubBUF_VdoSubBs20BufNum;i++)
		{
			ubBUF_VdoSubBs20BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs20BufIdx = ubBUF_VdoSubBs20BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS30)
	{
		for(i=0;i<ubBUF_VdoSubBs30BufNum;i++)
		{
			ubBUF_VdoSubBs30BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs30BufIdx = ubBUF_VdoSubBs30BufNum-1;
	}	
	else if(ubBufMode == BUF_VDO_SUB_BS01)
	{
		for(i=0;i<ubBUF_VdoSubBs01BufNum;i++)
		{
			ubBUF_VdoSubBs01BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs01BufIdx = ubBUF_VdoSubBs01BufNum-1;	
	}
	else if(ubBufMode == BUF_VDO_SUB_BS11)
	{
		for(i=0;i<ubBUF_VdoSubBs11BufNum;i++)
		{
			ubBUF_VdoSubBs11BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs11BufIdx = ubBUF_VdoSubBs11BufNum-1;	
	}
	else if(ubBufMode == BUF_VDO_SUB_BS21)
	{
		for(i=0;i<ubBUF_VdoSubBs21BufNum;i++)
		{
			ubBUF_VdoSubBs21BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs21BufIdx = ubBUF_VdoSubBs21BufNum-1;	
	}
	else if(ubBufMode == BUF_VDO_SUB_BS31)
	{
		for(i=0;i<ubBUF_VdoSubBs31BufNum;i++)
		{
			ubBUF_VdoSubBs31BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs31BufIdx = ubBUF_VdoSubBs31BufNum-1;
	}		
	else if(ubBufMode == BUF_ADO_ADC)
	{
		for(i=0;i<ubBUF_AdcBufNum;i++)
		{
			ubBUF_AdcBufTag[i] = BUF_FREE;			
		}
		ubBUF_AdcBufIdx = ubBUF_AdcBufNum-1;		
	}
	
	else if(ubBufMode == BUF_ADO_DAC0)
	{
		for(i=0;i<ubBUF_Dac0BufNum;i++)
		{
			ubBUF_Dac0BufTag[i]	= BUF_FREE;			
		}
		ubBUF_Dac0BufIdx = ubBUF_Dac0BufNum-1;		
	}
	else if(ubBufMode == BUF_ADO_DAC1)
	{
		for(i=0;i<ubBUF_Dac1BufNum;i++)
		{
			ubBUF_Dac1BufTag[i]	= BUF_FREE;			
		}
		ubBUF_Dac1BufIdx = ubBUF_Dac1BufNum-1;		
	}
	else if(ubBufMode == BUF_ADO_DAC2)
	{
		for(i=0;i<ubBUF_Dac2BufNum;i++)
		{
			ubBUF_Dac2BufTag[i]	= BUF_FREE;			
		}
		ubBUF_Dac2BufIdx = ubBUF_Dac2BufNum-1;		
	}
	else if(ubBufMode == BUF_ADO_DAC3)
	{
		for(i=0;i<ubBUF_Dac3BufNum;i++)
		{
			ubBUF_Dac3BufTag[i]	= BUF_FREE;
		}
		ubBUF_Dac3BufIdx = ubBUF_Dac3BufNum-1;
	}
	else if(ubBufMode == BUF_USBD_VDO)
	{
		for(i = 0; i < ubBUF_VdoUsbdBufNum; i++)
		{
			ubBUF_VdoUsbdBufTag[i] = BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_USBD_ADO)
	{
		for(i = 0; i < ubBUF_AdoUsbdBufNum; i++)
		{
			ubBUF_AdoUsbdBufTag[i] = BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_VDO_PACKETIZE_BIG)	//(VDO)Transmit
	{
		for(i=0;i<ubBUF_VdoPacketizeBigBufNum;i++)
		{
			ubBUF_VdoPacketizeBigBufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_VDO_PACKETIZE_SMALL)	//(VDO)Transmit
	{
		for(i=0;i<ubBUF_VdoPacketizeSmallBufNum;i++)
		{
			ubBUF_VdoPacketizeSmallBufTag[i]	= BUF_FREE;
		}
	}
	
	
	else if(ubBufMode == BUF_ADO_PACKETIZE)	//(ADO)Transmit
	{
		for(i=0;i<ubBUF_AdoPacketizeBufNum;i++)
		{
			ubBUF_AdoPacketizeBufTag[i]	= BUF_FREE;
		}
	}
	
	else if(ubBufMode == BUF_VDO_PACKETIZE0)//Receive(CAM1)_VDO
	{
		for(i=0;i<ubBUF_VdoPacketize0BufNum;i++)
		{
			ubBUF_VdoPacketize0BufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_VDO_PACKETIZE1)//Receive(CAM2)_VDO
	{
		for(i=0;i<ubBUF_VdoPacketize1BufNum;i++)
		{
			ubBUF_VdoPacketize1BufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_VDO_PACKETIZE2)//Receive(CAM3)_VDO
	{
		for(i=0;i<ubBUF_VdoPacketize2BufNum;i++)
		{
			ubBUF_VdoPacketize2BufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_VDO_PACKETIZE3)//Receive(CAM4)_VDO
	{
		for(i=0;i<ubBUF_VdoPacketize3BufNum;i++)
		{
			ubBUF_VdoPacketize3BufTag[i]	= BUF_FREE;
		}
	}
	
	
	else if(ubBufMode == BUF_ADO_PACKETIZE0)//Receive(CAM1)_ADO
	{
		for(i=0;i<ubBUF_AdoPacketize0BufNum;i++)
		{
			ubBUF_AdoPacketize0BufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_ADO_PACKETIZE1)//Receive(CAM2)_ADO
	{
		for(i=0;i<ubBUF_AdoPacketize1BufNum;i++)
		{
			ubBUF_AdoPacketize1BufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_ADO_PACKETIZE2)//Receive(CAM3)_ADO
	{
		for(i=0;i<ubBUF_AdoPacketize2BufNum;i++)
		{
			ubBUF_AdoPacketize2BufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_ADO_PACKETIZE3)//Receive(CAM4)_ADO
	{
		for(i=0;i<ubBUF_AdoPacketize3BufNum;i++)
		{
			ubBUF_AdoPacketize3BufTag[i]	= BUF_FREE;
		}
	}
	
	else if(ubBufMode == BUF_PACKET0)
	{
		for(i=0;i<ubBUF_Packet0BufNum;i++)
		{
			ubBUF_Packet0BufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_PACKET1)
	{
		for(i=0;i<ubBUF_Packet1BufNum;i++)
		{
			ubBUF_Packet1BufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_PACKET2)
	{
		for(i=0;i<ubBUF_Packet2BufNum;i++)
		{
			ubBUF_Packet2BufTag[i]	= BUF_FREE;
		}
	}
	else if(ubBufMode == BUF_PACKET3)
	{
		for(i=0;i<ubBUF_Packet3BufNum;i++)
		{
			ubBUF_Packet3BufTag[i]	= BUF_FREE;
		}
	}
	
	else if(ubBufMode == BUF_RESV_YUV)
	{
		ulBUF_ResvYuvBuffAddr = BUF_FREE;
	}
    else if(ubBufMode == BUF_DEC_YUV)
	{
		ulBUF_DecodeYuvBuffAddr = BUF_FREE;
	}
#if defined(BSP_DVR_SDK)
	else if(ubBufMode == BUF_TEMP_YUV)
	{
		ulBUF_TempYuvBuffAddr = BUF_FREE;
	}
#endif
#if (defined(OP_AP) && ((defined(S2019A) && defined(RVCS_APP)) || defined(sWIFIBDG)))
	else if(ubBufMode == BUF_WIFI_BDG)
	{
		for(i=0;i<ubBUF_VdoBdgBsBufNum;i++)
		{
			ubBUF_VdoBdgBsBufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoBdgBsBufIdx = ubBUF_VdoBdgBsBufNum-1;
	}
#endif
}

uint32_t ulBuf_GetBsBufSize(uint8_t ubBufMode)
{
	if(ubBufMode == BUF_VDO_MAIN_BS0)
		return ulBUF_VdoMainBs0BufSz;
	else if(ubBufMode == BUF_VDO_MAIN_BS1)
		return ulBUF_VdoMainBs1BufSz;
	else if(ubBufMode == BUF_VDO_MAIN_BS2)
		return ulBUF_VdoMainBs2BufSz;
	else if(ubBufMode == BUF_VDO_MAIN_BS3)
		return ulBUF_VdoMainBs3BufSz;
	
	else if(ubBufMode == BUF_VDO_AUX_BS0)
		return ulBUF_VdoAuxBs0BufSz;
	else if(ubBufMode == BUF_VDO_AUX_BS1)
		return ulBUF_VdoAuxBs1BufSz;
	else if(ubBufMode == BUF_VDO_AUX_BS2)
		return ulBUF_VdoAuxBs2BufSz;
	else if(ubBufMode == BUF_VDO_AUX_BS3)
		return ulBUF_VdoAuxBs3BufSz;	
	
	else if(ubBufMode == BUF_VDO_SUB_BS00)
		return ulBUF_VdoSubBs00BufSz;
	else if(ubBufMode == BUF_VDO_SUB_BS10)
		return ulBUF_VdoSubBs10BufSz;
	else if(ubBufMode == BUF_VDO_SUB_BS20)
		return ulBUF_VdoSubBs20BufSz;
	else if(ubBufMode == BUF_VDO_SUB_BS30)
		return ulBUF_VdoSubBs30BufSz;
	
	else if(ubBufMode == BUF_VDO_SUB_BS01)	
		return ulBUF_VdoSubBs01BufSz;		
	else if(ubBufMode == BUF_VDO_SUB_BS11)	
		return ulBUF_VdoSubBs11BufSz;		
	else if(ubBufMode == BUF_VDO_SUB_BS21)	
		return ulBUF_VdoSubBs21BufSz;		
	else if(ubBufMode == BUF_VDO_SUB_BS31)	
		return ulBUF_VdoSubBs31BufSz;	

	else
	{
		printf("Err @%s\r\n",__func__);
		return 0;
	}
}
//ubIndex is useful for BUF_IMG_ENC/BUF_IMG_DEC Mode
//-----------------------------------------------------------------------
void BUF_BufInit(uint8_t ubBufMode,uint8_t ubBufNum,uint32_t ulUnitSz,uint8_t ubIndex)
{
	uint8_t i;

	//printd(DBG_CriticalLvl, "BufInit Mode=%d, Adr=0x%08X, Num=%d, Index=%d, Sz=0x%X\n",ubBufMode,ulBUF_FreeBufAddr,ubBufNum,ubIndex,ulUnitSz);
	printd(DBG_CriticalLvl,"Buf M=%d,A=0x%08X,N=%d,I=%d,S=0x%X\n",ubBufMode,ulBUF_FreeBufAddr,ubBufNum,ubIndex,ulUnitSz);

	//IP or Function Buffer
	if(ubBufMode == BUF_IMG_ENC)
	{
		ulBUF_ImgEncBufAddr[ubIndex] = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_IMG_DEC)
	{
		ulBUF_ImgDecBufAddr[ubIndex] = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_ADO_IP)
	{
		ulBUF_AdoIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_LCD_IP)
	{
		ulBUF_LcdIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_BB_IP)
	{
		ulBUF_BBIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_IMG_MERGE)
	{
		ulBUF_ImgMergeIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_SEN_IP)
	{
		ulBUF_SenIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
    else if(ubBufMode == BUF_USBD_IP)
	{
		ulBUF_UsbdIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
    else if(ubBufMode == BUF_USBH_IP)
	{
		ulBUF_UsbhIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "USBH--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\r\n",ulBUF_FreeBufAddr); 
	}
    else if(ubBufMode == BUF_ISP_3DNR_IP)
	{
		ulBUF_3DNRIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 	
    else if(ubBufMode == BUF_ISP_MD_W0_IP)
	{
		ulBUF_MDw0IpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 	
    else if(ubBufMode == BUF_ISP_MD_W1_IP)
	{
		ulBUF_MDw1IpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 	
    else if(ubBufMode == BUF_ISP_MD_W2_IP)
	{
		ulBUF_MDw2IpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 	
    else if(ubBufMode == BUF_IQ_BIN_FILE)
	{
		ulBUF_IQbinBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
    else if(ubBufMode == BUF_VDO_TEMP1)
	{
		ulBUF_VdoTemp1BufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 
	//FOTA
	else if(ubBufMode == BUF_FOTA)
	{
		ulBUF_FotaBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 	
	//Data Buffer
	else if(ubBufMode == BUF_SEN_1_YUV)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Sen1YuvBufTag[i]	= BUF_FREE;				
			ulBUF_Sen1YuvBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}			
		ubBUF_Sen1YuvBufIdx	= ubBufNum-1;		
		ulBUF_Sen1YuvBufSz	= ulUnitSz;
		ubBUF_Sen1YuvBufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_SEN_2_YUV)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Sen2YuvBufTag[i]	= BUF_FREE;				
			ulBUF_Sen2YuvBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}			
		ubBUF_Sen2YuvBufIdx 	= ubBufNum-1;		
		ulBUF_Sen2YuvBufSz	= ulUnitSz;
		ubBUF_Sen2YuvBufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_SEN_3_YUV)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Sen3YuvBufTag[i]	= BUF_FREE;				
			ulBUF_Sen3YuvBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}			
		ubBUF_Sen3YuvBufIdx = ubBufNum-1;		
		ulBUF_Sen3YuvBufSz  = ulUnitSz;
		ubBUF_Sen3YuvBufNum = ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS0)
	{
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoMainBs0BufTag[i]	= BUF_FREE;			
			ulBUF_VdoMainBs0BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoMainBs0BufIdx 	= ubBufNum-1;
		ulBUF_VdoMainBs0BufSz 	= ulUnitSz;
		ubBUF_VdoMainBs0BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_VDO_MAIN_BS1)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoMainBs1BufTag[i]	= BUF_FREE;			
			ulBUF_VdoMainBs1BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoMainBs1BufIdx 	= ubBufNum-1;
		ulBUF_VdoMainBs1BufSz	= ulUnitSz;
		ubBUF_VdoMainBs1BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS2)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoMainBs2BufTag[i]	= BUF_FREE;			
			ulBUF_VdoMainBs2BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoMainBs2BufIdx 	= ubBufNum-1;
		ulBUF_VdoMainBs2BufSz	= ulUnitSz;
		ubBUF_VdoMainBs2BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS3)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoMainBs3BufTag[i]	= BUF_FREE;			
			ulBUF_VdoMainBs3BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoMainBs3BufIdx 	= ubBufNum-1;
		ulBUF_VdoMainBs3BufSz	= ulUnitSz;
		ubBUF_VdoMainBs3BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS0)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoAuxBs0BufTag[i]	= BUF_FREE;			
			ulBUF_VdoAuxBs0BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoAuxBs0BufIdx 	= ubBufNum-1;
		ulBUF_VdoAuxBs0BufSz	= ulUnitSz;
		ubBUF_VdoAuxBs0BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_VDO_AUX_BS1)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoAuxBs1BufTag[i]	= BUF_FREE;			
			ulBUF_VdoAuxBs1BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoAuxBs1BufIdx 	= ubBufNum-1;
		ulBUF_VdoAuxBs1BufSz	= ulUnitSz;
		ubBUF_VdoAuxBs1BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS2)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoAuxBs2BufTag[i]	= BUF_FREE;			
			ulBUF_VdoAuxBs2BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoAuxBs2BufIdx = ubBufNum-1;
		ulBUF_VdoAuxBs2BufSz  = ulUnitSz;
		ubBUF_VdoAuxBs2BufNum = ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS3)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoAuxBs3BufTag[i]	= BUF_FREE;			
			ulBUF_VdoAuxBs3BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoAuxBs3BufIdx = ubBufNum-1;
		ulBUF_VdoAuxBs3BufSz  = ulUnitSz;
		ubBUF_VdoAuxBs3BufNum = ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS00)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs00BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs00BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs00BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs00BufSz	= ulUnitSz;
		ubBUF_VdoSubBs00BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_VDO_SUB_BS10)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs10BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs10BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs10BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs10BufSz	= ulUnitSz;
		ubBUF_VdoSubBs10BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS20)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs20BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs20BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs20BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs20BufSz	= ulUnitSz;
		ubBUF_VdoSubBs20BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS30)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs30BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs30BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs30BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs30BufSz	= ulUnitSz;
		ubBUF_VdoSubBs30BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS01)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs01BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs01BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr 	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs01BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs01BufSz	= ulUnitSz;
		ubBUF_VdoSubBs01BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_VDO_SUB_BS11)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs11BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs11BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs11BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs11BufSz	= ulUnitSz;
		ubBUF_VdoSubBs11BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS21)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs21BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs21BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr 	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs21BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs21BufSz	= ulUnitSz;
		ubBUF_VdoSubBs21BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS31)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs31BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs31BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr 	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs31BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs31BufSz	= ulUnitSz;
		ubBUF_VdoSubBs31BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_ADO_ADC)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_AdcBufTag[i]	= BUF_FREE;			
			ulBUF_AdcBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_AdcBufIdx 	= ubBufNum-1;
		ulBUF_AdcBufSz		= ulUnitSz;
		ubBUF_AdcBufNum		= ubBufNum;
	}
	
	else if(ubBufMode == BUF_ADO_DAC0)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Dac0BufTag[i]		= BUF_FREE;			
			ulBUF_Dac0BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_Dac0BufIdx 	= ubBufNum-1;
		ulBUF_Dac0BufSz		= ulUnitSz;
		ubBUF_Dac0BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_ADO_DAC1)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Dac1BufTag[i]		= BUF_FREE;			
			ulBUF_Dac1BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_Dac1BufIdx 	= ubBufNum-1;
		ulBUF_Dac1BufSz		= ulUnitSz;
		ubBUF_Dac1BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_ADO_DAC2)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Dac2BufTag[i]		= BUF_FREE;			
			ulBUF_Dac2BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_Dac2BufIdx 	= ubBufNum-1;
		ulBUF_Dac2BufSz		= ulUnitSz;
		ubBUF_Dac2BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_ADO_DAC3)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Dac3BufTag[i]		= BUF_FREE;			
			ulBUF_Dac3BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}
		ubBUF_Dac3BufIdx 	= ubBufNum-1;
		ulBUF_Dac3BufSz		= ulUnitSz;
		ubBUF_Dac3BufNum	= ubBufNum;
	}
    else if(ubBufMode == BUF_SD)
    {
		ulBUF_SdBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "SD--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\n",ulBUF_FreeBufAddr); 
    }
    else if(ubBufMode == BUF_FS)
    {
		ulBUF_FsBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "FS--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\n",ulBUF_FreeBufAddr); 
    }
    else if(ubBufMode == BUF_REC)
    {
		ulBUF_RecBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "REC--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\n",ulBUF_FreeBufAddr); 
    }
    else if(ubBufMode == BUF_MAC)
    {
		ulBUF_MacBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "MAC--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\n",ulBUF_FreeBufAddr); 
    }
    else if(ubBufMode == BUF_THM_SHOWING_READ_TEMP)
    {
		ulBUF_ThmShowingRdTempBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "Thm showing rd temp--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\n",ulBUF_FreeBufAddr); 
    }
    else if(ubBufMode == BUF_THM_SHOWING_DEC_YUV)
    {
		ulBUF_ThmShowingDecYuvBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "Thm showing dec yuv--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\n",ulBUF_FreeBufAddr); 
    }
	else if(ubBufMode == BUF_USBD_VDO)
	{
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoUsbdBufTag[i]	= BUF_FREE;
			ulBUF_VdoUsbdBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_VdoUsbdBufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_USBD_ADO)
	{
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_AdoUsbdBufTag[i]	= BUF_FREE;
			ulBUF_AdoUsbdBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_AdoUsbdBufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_JPG_BS)
	{
		for(i = 0; i < ubBufNum; i++)
		{
			ulBUF_JpgBsBuffAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
	}
#if defined(BSP_DVR_SDK)
	else if(ubBufMode == BUF_JPG_BS2)
	{		
		ulBUF_JpgBs2BuffAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);	
	}
#endif
	else if(ubBufMode == BUF_JPG_RAW)
	{
		ulBUF_JpgRawBuffAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_RESV_YUV)
	{
		ulBUF_ResvYuvBuffAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
    else if(ubBufMode == BUF_DEC_YUV)
	{
		ulBUF_DecodeYuvBuffAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
#if defined(BSP_DVR_SDK)
	else if(ubBufMode == BUF_TEMP_YUV)
	{
		ulBUF_TempYuvBuffAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
#endif
	else if(ubBufMode == BUF_VDO_PACKETIZE_BIG)	//(VDO)Transmit
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoPacketizeBigBufTag[i]	= BUF_FREE;
			ulBUF_VdoPacketizeBigBufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_VdoPacketizeBigBufIdx 	= ubBufNum-1;
		ulBUF_VdoPacketizeBigBufSz 	= ulUnitSz;
		ubBUF_VdoPacketizeBigBufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_VDO_PACKETIZE_SMALL)	//(VDO)Transmit
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoPacketizeSmallBufTag[i]	= BUF_FREE;
			ulBUF_VdoPacketizeSmallBufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_VdoPacketizeSmallBufIdx 	= ubBufNum-1;
		ulBUF_VdoPacketizeSmallBufSz 	= ulUnitSz;
		ubBUF_VdoPacketizeSmallBufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_ADO_PACKETIZE)	//(ADO)Transmit
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_AdoPacketizeBufTag[i]	= BUF_FREE;
			ulBUF_AdoPacketizeBufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_AdoPacketizeBufIdx 	= ubBufNum-1;
		ulBUF_AdoPacketizeBufSz 	= ulUnitSz;
		ubBUF_AdoPacketizeBufNum	= ubBufNum;		
	}
	
	else if(ubBufMode == BUF_VDO_PACKETIZE0)	//Receive(Cam1)
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoPacketize0BufTag[i]	= BUF_FREE;
			ulBUF_VdoPacketize0BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_VdoPacketize0BufIdx 	= ubBufNum-1;
		ulBUF_VdoPacketize0BufSz 	= ulUnitSz;
		ubBUF_VdoPacketize0BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_VDO_PACKETIZE1)	//Receive(Cam2)
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoPacketize1BufTag[i]	= BUF_FREE;
			ulBUF_VdoPacketize1BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_VdoPacketize1BufIdx 	= ubBufNum-1;
		ulBUF_VdoPacketize1BufSz 	= ulUnitSz;
		ubBUF_VdoPacketize1BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_VDO_PACKETIZE2)	//Receive(Cam3)
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoPacketize2BufTag[i]	= BUF_FREE;
			ulBUF_VdoPacketize2BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_VdoPacketize2BufIdx 	= ubBufNum-1;
		ulBUF_VdoPacketize2BufSz 	= ulUnitSz;
		ubBUF_VdoPacketize2BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_VDO_PACKETIZE3)	//Receive(Cam4)
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoPacketize3BufTag[i]	= BUF_FREE;
			ulBUF_VdoPacketize3BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_VdoPacketize3BufIdx 	= ubBufNum-1;
		ulBUF_VdoPacketize3BufSz 	= ulUnitSz;
		ubBUF_VdoPacketize3BufNum	= ubBufNum;		
	}
	
	
	else if(ubBufMode == BUF_ADO_PACKETIZE0)	//Receive(Cam1)_ADO
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_AdoPacketize0BufTag[i]	= BUF_FREE;
			ulBUF_AdoPacketize0BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_AdoPacketize0BufIdx 	= ubBufNum-1;
		ulBUF_AdoPacketize0BufSz 	= ulUnitSz;
		ubBUF_AdoPacketize0BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_ADO_PACKETIZE1)	//Receive(Cam2)_ADO
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_AdoPacketize1BufTag[i]	= BUF_FREE;
			ulBUF_AdoPacketize1BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_AdoPacketize1BufIdx 	= ubBufNum-1;
		ulBUF_AdoPacketize1BufSz 	= ulUnitSz;
		ubBUF_AdoPacketize1BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_ADO_PACKETIZE2)	//Receive(Cam3)_ADO
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_AdoPacketize2BufTag[i]	= BUF_FREE;
			ulBUF_AdoPacketize2BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_AdoPacketize2BufIdx 	= ubBufNum-1;
		ulBUF_AdoPacketize2BufSz 	= ulUnitSz;
		ubBUF_AdoPacketize2BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_ADO_PACKETIZE3)	//Receive(Cam4)_ADO
	{			
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_AdoPacketize3BufTag[i]	= BUF_FREE;
			ulBUF_AdoPacketize3BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_AdoPacketize3BufIdx 	= ubBufNum-1;
		ulBUF_AdoPacketize3BufSz 	= ulUnitSz;
		ubBUF_AdoPacketize3BufNum	= ubBufNum;		
	}
	
	else if(ubBufMode == BUF_PACKET0)
	{	
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Packet0BufTag[i]	= BUF_FREE;
			ulBUF_Packet0BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			//ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo512B(ulBUF_FreeBufAddr);
		}
		ubBUF_Packet0BufIdx = ubBufNum-1;
		ulBUF_Packet0BufSz 	= ulUnitSz;
		ubBUF_Packet0BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_PACKET1)
	{	
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Packet1BufTag[i]	= BUF_FREE;
			ulBUF_Packet1BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			//ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo512B(ulBUF_FreeBufAddr);
		}
		ubBUF_Packet1BufIdx = ubBufNum-1;
		ulBUF_Packet1BufSz 	= ulUnitSz;
		ubBUF_Packet1BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_PACKET2)
	{	
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Packet2BufTag[i]	= BUF_FREE;
			ulBUF_Packet2BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			//ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo512B(ulBUF_FreeBufAddr);
		}
		ubBUF_Packet2BufIdx = ubBufNum-1;
		ulBUF_Packet2BufSz 	= ulUnitSz;
		ubBUF_Packet2BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_PACKET3)
	{	
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Packet3BufTag[i]	= BUF_FREE;
			ulBUF_Packet3BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			//ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo512B(ulBUF_FreeBufAddr);
		}
		ubBUF_Packet3BufIdx = ubBufNum-1;
		ulBUF_Packet3BufSz 	= ulUnitSz;
		ubBUF_Packet3BufNum	= ubBufNum;		
	}
	else if(ubBufMode == BUF_RF_DRIVER)
	{
		// Richwave
		ulBUF_RFDriverBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);

		//printf("RF Driver--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\r\n",ulBUF_FreeBufAddr);
	}
    else if(ubBufMode == BUF_RW_FRAME)
    {
		// Richwave
		ulBUF_RWFrameBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);

        //printf("Richwave Frame--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\r\n", ulBUF_FreeBufAddr);
    }
    else if(ubBufMode == BUF_FD_RECT_OSD)
    {
        ulBUF_FDRectOsdBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
    }
	else if(ubBufMode == BUF_IMG_DS1)
	{
		ulBUF_IMG_DS1BufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_sPRF)
	{
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_WIFI_DT)
	{
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
#if (defined(OP_AP) && ((defined(S2019A) && defined(RVCS_APP)) || defined(sWIFIBDG)))
	else if(ubBufMode == BUF_WIFI_BDG)
	{
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoBdgBsBufTag[i]	 = BUF_FREE;
			ulBUF_VdoBdgBsBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
		ubBUF_VdoBdgBsBufIdx 	= ubBufNum-1;
		ulBUF_VdoBdgBsBufSz		= ulUnitSz;
		ubBUF_VdoBdgBsBufNum	= ubBufNum;
	}
#endif
	else if(ubBufMode == BUF_USB_TRX)
	{
		ulBUF_UsbTrx_BufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	
#if defined(BSP_DVR_SDK)
	if (ulBUF_FreeBufAddr >= (70*1024*1024L)) {	//justin 2019.08.08
#else
	if (ulBUF_FreeBufAddr >= ulDDR_GetCapacity()) {
#endif	
        printd(DBG_ErrorLvl, "Buffer full:%d\n",ulBUF_FreeBufAddr); 
		while(1);
    }
}

/* Richwave */
uint32_t ulBUF_GetRFDriverStartAddr(void)
{
    return ulBUF_RFDriverBufAddr;
}

/* Richwave */
uint32_t ulBUF_GetRWFrameBufStartAddr(void)
{
	return ulBUF_RWFrameBufAddr;
}

uint32_t ulBUF_GetSen1YuvFreeBuf(void)
{
	uint32_t ulTemp;
	
#if RTOS
	osSemaphoreWait(tBUF_Sen1YuvAcc, osWaitForever);
#endif
	
	if(ubBUF_Sen1YuvBufNum == 1)
	{
		ubBUF_Sen1YuvBufTag[0] = BUF_USED;
	#if RTOS
		osSemaphoreRelease(tBUF_Sen1YuvAcc);
	#endif
		return ulBUF_Sen1YuvBufAddr[0];
	}
	
	if(++ubBUF_Sen1YuvBufIdx >= ubBUF_Sen1YuvBufNum)
		ubBUF_Sen1YuvBufIdx = 0;
	
	if(ubBUF_Sen1YuvBufTag[ubBUF_Sen1YuvBufIdx] == BUF_USED)
	{
		//Restore		
		if(ubBUF_Sen1YuvBufIdx == 0)
			ubBUF_Sen1YuvBufIdx = ubBUF_Sen1YuvBufNum-1;
		else		
			ubBUF_Sen1YuvBufIdx--;		
		
		printd(DBG_ErrorLvl, "S1 Busy\n");		
	#if RTOS
		osSemaphoreRelease(tBUF_Sen1YuvAcc);
	#endif	
		return BUF_FAIL;
	}
	ubBUF_Sen1YuvBufTag[ubBUF_Sen1YuvBufIdx] = BUF_USED;
	//return ulBUF_Sen1YuvBufAddr[ubBUF_Sen1YuvBufIdx];
	ulTemp = ulBUF_Sen1YuvBufAddr[ubBUF_Sen1YuvBufIdx];
	
#if RTOS
	osSemaphoreRelease(tBUF_Sen1YuvAcc);
#endif	
	return ulTemp;	
}

uint32_t ulBUF_GetSen2YuvFreeBuf(void)
{
	uint32_t ulTemp;
	
#if RTOS
	osSemaphoreWait(tBUF_Sen2YuvAcc, osWaitForever);
#endif
	
	if(++ubBUF_Sen2YuvBufIdx >= ubBUF_Sen2YuvBufNum)
		ubBUF_Sen2YuvBufIdx = 0;
	if(ubBUF_Sen2YuvBufTag[ubBUF_Sen2YuvBufIdx] == BUF_USED)
	{
		//Restore		
		if(ubBUF_Sen2YuvBufIdx == 0)
			ubBUF_Sen2YuvBufIdx = ubBUF_Sen2YuvBufNum-1;
		else		
			ubBUF_Sen2YuvBufIdx--;		
		
		printd(DBG_ErrorLvl, "S2 Busy\n");		
	#if RTOS
		osSemaphoreRelease(tBUF_Sen2YuvAcc);
	#endif		
		return BUF_FAIL;
	}
	ubBUF_Sen2YuvBufTag[ubBUF_Sen2YuvBufIdx] = BUF_USED;
	//return ulBUF_Sen2YuvBufAddr[ubBUF_Sen2YuvBufIdx];
	ulTemp = ulBUF_Sen2YuvBufAddr[ubBUF_Sen2YuvBufIdx];
	
#if RTOS
	osSemaphoreRelease(tBUF_Sen2YuvAcc);
#endif	
	return ulTemp;
}

uint32_t ulBUF_GetSen3YuvFreeBuf(void)
{
	uint32_t ulTemp;
	
#if RTOS
	osSemaphoreWait(tBUF_Sen3YuvAcc, osWaitForever);
#endif
	
	if(++ubBUF_Sen3YuvBufIdx >= ubBUF_Sen3YuvBufNum)
		ubBUF_Sen3YuvBufIdx = 0;
	if(ubBUF_Sen3YuvBufTag[ubBUF_Sen3YuvBufIdx] == BUF_USED)
	{
		//Restore		
		if(ubBUF_Sen3YuvBufIdx == 0)
			ubBUF_Sen3YuvBufIdx = ubBUF_Sen3YuvBufNum-1;
		else		
			ubBUF_Sen3YuvBufIdx--;		
		
		printd(DBG_ErrorLvl, "S3 Busy\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Sen3YuvAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Sen3YuvBufTag[ubBUF_Sen3YuvBufIdx] = BUF_USED;
	//return ulBUF_Sen3YuvBufAddr[ubBUF_Sen3YuvBufIdx];
	ulTemp = ulBUF_Sen3YuvBufAddr[ubBUF_Sen3YuvBufIdx];
	
#if RTOS
	osSemaphoreRelease(tBUF_Sen3YuvAcc);
#endif	
	return ulTemp;
}


uint16_t uwBUF_GetVdoPacketizeBigTotalNum(void)
{
	uint16_t uwTemp;
#ifdef RTOS
	osSemaphoreWait(tBUF_VdoPacketizeBigAcc, osWaitForever);
#endif
	uwTemp = ubBUF_VdoPacketizeBigBufNum;
#ifdef RTOS
	osSemaphoreRelease(tBUF_VdoPacketizeBigAcc);
#endif
	return uwTemp;	
}
uint16_t uwBUF_GetVdoPacketizeSmallTotalNum(void)
{
	uint16_t uwTemp;
#ifdef RTOS
	osSemaphoreWait(tBUF_VdoPacketizeSmallAcc, osWaitForever);
#endif
	uwTemp = ubBUF_VdoPacketizeSmallBufNum;
#ifdef RTOS
	osSemaphoreRelease(tBUF_VdoPacketizeSmallAcc);
#endif
	return uwTemp;	
}
	
uint16_t uwBUF_GetVdoPacketizeBigFreeNum(void)
{
	uint16_t uwTemp,i;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_VdoPacketizeBigAcc, osWaitForever);
#endif
	
	uwTemp = 0;
	
	for(i=0;i<ubBUF_VdoPacketizeBigBufNum;i++)
	{
		if(ubBUF_VdoPacketizeBigBufTag[i] == BUF_FREE)
			uwTemp++;
	}	
#ifdef RTOS
	osSemaphoreRelease(tBUF_VdoPacketizeBigAcc);
#endif
	return uwTemp;
}
uint16_t uwBUF_GetVdoPacketizeSmallFreeNum(void)
{
	uint16_t uwTemp,i;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_VdoPacketizeSmallAcc, osWaitForever);
#endif
	
	uwTemp = 0;
	
	for(i=0;i<ubBUF_VdoPacketizeSmallBufNum;i++)
	{
		if(ubBUF_VdoPacketizeSmallBufTag[i] == BUF_FREE)
			uwTemp++;
	}	
#ifdef RTOS
	osSemaphoreRelease(tBUF_VdoPacketizeSmallAcc);
#endif
	return uwTemp;
}

uint32_t ulBUF_GetVdoPacketizeBigFreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPacketizeBigAcc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_VdoPacketizeBigBufNum;i++)
	{
		if(ubBUF_VdoPacketizeBigBufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_VdoPacketizeBigBufIdx >= ubBUF_VdoPacketizeBigBufNum)
			ubBUF_VdoPacketizeBigBufIdx = 0;

		if(ubBUF_VdoPacketizeBigBufTag[ubBUF_VdoPacketizeBigBufIdx] != BUF_FREE)
		{			
			printd(DBG_ErrorLvl, "Vdo-P Busy(Big)\n");
			
		#if RTOS
			osSemaphoreRelease(tBUF_VdoPacketizeBigAcc);
		#endif			
			return BUF_FAIL;
		}
		ubBUF_VdoPacketizeBigBufTag[ubBUF_VdoPacketizeBigBufIdx] = BUF_USED;
		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPacketizeBigAcc);
	#endif	
		return ulBUF_VdoPacketizeBigBufAddr[ubBUF_VdoPacketizeBigBufIdx];
	}
	else
	{
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPacketizeBigAcc);
	#endif		
		return BUF_FAIL;
	}
}
uint32_t ulBUF_GetVdoPacketizeSmallFreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPacketizeSmallAcc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_VdoPacketizeSmallBufNum;i++)
	{
		if(ubBUF_VdoPacketizeSmallBufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_VdoPacketizeSmallBufIdx >= ubBUF_VdoPacketizeSmallBufNum)
			ubBUF_VdoPacketizeSmallBufIdx = 0;

		if(ubBUF_VdoPacketizeSmallBufTag[ubBUF_VdoPacketizeSmallBufIdx] != BUF_FREE)
		{			
			printd(DBG_ErrorLvl, "Vdo-P Busy(Small)\n");
			
		#if RTOS
			osSemaphoreRelease(tBUF_VdoPacketizeSmallAcc);
		#endif			
			return BUF_FAIL;
		}
		ubBUF_VdoPacketizeSmallBufTag[ubBUF_VdoPacketizeSmallBufIdx] = BUF_USED;
		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPacketizeSmallAcc);
	#endif	
		return ulBUF_VdoPacketizeSmallBufAddr[ubBUF_VdoPacketizeSmallBufIdx];
	}
	else
	{
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPacketizeSmallAcc);
	#endif		
		return BUF_FAIL;
	}
}

uint32_t ulBUF_GetAdoPacketizeFreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_AdoPacketizeAcc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_AdoPacketizeBufNum;i++)
	{
		if(ubBUF_AdoPacketizeBufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_AdoPacketizeBufIdx >= ubBUF_AdoPacketizeBufNum)
			ubBUF_AdoPacketizeBufIdx = 0;

		if(ubBUF_AdoPacketizeBufTag[ubBUF_AdoPacketizeBufIdx] != BUF_FREE)
		{			
			printd(DBG_ErrorLvl, "Ado-P Busy\n");
			
		#if RTOS
			osSemaphoreRelease(tBUF_AdoPacketizeAcc);
		#endif			
			return BUF_FAIL;
		}
		ubBUF_AdoPacketizeBufTag[ubBUF_AdoPacketizeBufIdx] = BUF_USED;
		
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPacketizeAcc);
	#endif	
		return ulBUF_AdoPacketizeBufAddr[ubBUF_AdoPacketizeBufIdx];
	}
	else
	{
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPacketizeAcc);
	#endif
		
		return BUF_FAIL;
	}
}

//Video
uint32_t ulBUF_GetVdoPacketize0BufSz(void)
{
	return ulBUF_VdoPacketize0BufSz;
}
uint32_t ulBUF_GetVdoPacketize1BufSz(void)
{
	return ulBUF_VdoPacketize1BufSz;
}
uint32_t ulBUF_GetVdoPacketize2BufSz(void)
{
	return ulBUF_VdoPacketize2BufSz;
}
uint32_t ulBUF_GetVdoPacketize3BufSz(void)
{
	return ulBUF_VdoPacketize3BufSz;
}
uint32_t ulBUF_GetVdoPacketize0FreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPkt0Acc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_VdoPacketize0BufNum;i++)
	{
		if(ubBUF_VdoPacketize0BufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_VdoPacketize0BufIdx >= ubBUF_VdoPacketize0BufNum)
			ubBUF_VdoPacketize0BufIdx = 0;

		if(ubBUF_VdoPacketize0BufTag[ubBUF_VdoPacketize0BufIdx] != BUF_FREE)
		{			
			printd(DBG_ErrorLvl, "VdoP0 Busy\n");			
		#if RTOS
			osSemaphoreRelease(tBUF_VdoPkt0Acc);
		#endif
			return BUF_FAIL;
		}
		ubBUF_VdoPacketize0BufTag[ubBUF_VdoPacketize0BufIdx] = BUF_USED;		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt0Acc);
	#endif
		return ulBUF_VdoPacketize0BufAddr[ubBUF_VdoPacketize0BufIdx];
	}
	else
	{		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt0Acc);
	#endif		
		return BUF_FAIL;
	}
}

uint32_t ulBUF_GetVdoPacketize1FreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPkt1Acc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_VdoPacketize1BufNum;i++)
	{
		if(ubBUF_VdoPacketize1BufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_VdoPacketize1BufIdx >= ubBUF_VdoPacketize1BufNum)
			ubBUF_VdoPacketize1BufIdx = 0;

		if(ubBUF_VdoPacketize1BufTag[ubBUF_VdoPacketize1BufIdx] != BUF_FREE)
		{
			printd(DBG_ErrorLvl, "VdoP1 Busy\n");			
		#if RTOS
			osSemaphoreRelease(tBUF_VdoPkt1Acc);
		#endif
			return BUF_FAIL;
		}
		ubBUF_VdoPacketize1BufTag[ubBUF_VdoPacketize1BufIdx] = BUF_USED;		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt1Acc);
	#endif
		return ulBUF_VdoPacketize1BufAddr[ubBUF_VdoPacketize1BufIdx];
	}
	else
	{		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt1Acc);
	#endif
		return BUF_FAIL;
	}
}

uint32_t ulBUF_GetVdoPacketize2FreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPkt2Acc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_VdoPacketize2BufNum;i++)
	{
		if(ubBUF_VdoPacketize2BufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_VdoPacketize2BufIdx >= ubBUF_VdoPacketize2BufNum)
			ubBUF_VdoPacketize2BufIdx = 0;

		if(ubBUF_VdoPacketize2BufTag[ubBUF_VdoPacketize2BufIdx] != BUF_FREE)
		{
			printd(DBG_ErrorLvl, "VdoP2 Busy\n");		
		#if RTOS
			osSemaphoreRelease(tBUF_VdoPkt2Acc);
		#endif	
			return BUF_FAIL;
		}
		ubBUF_VdoPacketize2BufTag[ubBUF_VdoPacketize2BufIdx] = BUF_USED;		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt2Acc);
	#endif
		return ulBUF_VdoPacketize2BufAddr[ubBUF_VdoPacketize2BufIdx];
	}
	else
	{		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt2Acc);
	#endif	
		return BUF_FAIL;
	}
}

uint32_t ulBUF_GetVdoPacketize3FreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPkt3Acc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_VdoPacketize3BufNum;i++)
	{
		if(ubBUF_VdoPacketize3BufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_VdoPacketize3BufIdx >= ubBUF_VdoPacketize3BufNum)
			ubBUF_VdoPacketize3BufIdx = 0;

		if(ubBUF_VdoPacketize3BufTag[ubBUF_VdoPacketize3BufIdx] != BUF_FREE)
		{
			printd(DBG_ErrorLvl, "VdoP3 Busy\n");			
		#if RTOS
			osSemaphoreRelease(tBUF_VdoPkt3Acc);
		#endif	
			return BUF_FAIL;
		}
		ubBUF_VdoPacketize3BufTag[ubBUF_VdoPacketize3BufIdx] = BUF_USED;		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt3Acc);
	#endif	
		return ulBUF_VdoPacketize3BufAddr[ubBUF_VdoPacketize3BufIdx];
	}
	else
	{		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt3Acc);
	#endif		
		return BUF_FAIL;
	}
}

//Audio
uint32_t ulBUF_GetAdoPacketize0FreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_AdoPkt0Acc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_AdoPacketize0BufNum;i++)
	{
		if(ubBUF_AdoPacketize0BufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_AdoPacketize0BufIdx >= ubBUF_AdoPacketize0BufNum)
			ubBUF_AdoPacketize0BufIdx = 0;

		if(ubBUF_AdoPacketize0BufTag[ubBUF_AdoPacketize0BufIdx] != BUF_FREE)
		{
			printd(DBG_ErrorLvl, "AdoP0 Busy\n");		
		#if RTOS
			osSemaphoreRelease(tBUF_AdoPkt0Acc);
		#endif			
			return BUF_FAIL;
		}
		ubBUF_AdoPacketize0BufTag[ubBUF_AdoPacketize0BufIdx] = BUF_USED;		
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt0Acc);
	#endif
		return ulBUF_AdoPacketize0BufAddr[ubBUF_AdoPacketize0BufIdx];
	}
	else
	{		
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt0Acc);
	#endif
		return BUF_FAIL;
	}
}

uint32_t ulBUF_GetAdoPacketize1FreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_AdoPkt1Acc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_AdoPacketize1BufNum;i++)
	{
		if(ubBUF_AdoPacketize1BufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_AdoPacketize1BufIdx >= ubBUF_AdoPacketize1BufNum)
			ubBUF_AdoPacketize1BufIdx = 0;

		if(ubBUF_AdoPacketize1BufTag[ubBUF_AdoPacketize1BufIdx] != BUF_FREE)
		{
			printd(DBG_ErrorLvl, "AdoP1 Busy\n");			
		#if RTOS
			osSemaphoreRelease(tBUF_AdoPkt1Acc);
		#endif	
			return BUF_FAIL;
		}
		ubBUF_AdoPacketize1BufTag[ubBUF_AdoPacketize1BufIdx] = BUF_USED;		
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt1Acc);
	#endif	
		return ulBUF_AdoPacketize1BufAddr[ubBUF_AdoPacketize1BufIdx];
	}
	else
	{		
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt1Acc);
	#endif		
		return BUF_FAIL;
	}
}

uint32_t ulBUF_GetAdoPacketize2FreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_AdoPkt2Acc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_AdoPacketize2BufNum;i++)
	{
		if(ubBUF_AdoPacketize2BufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_AdoPacketize2BufIdx >= ubBUF_AdoPacketize2BufNum)
			ubBUF_AdoPacketize2BufIdx = 0;

		if(ubBUF_AdoPacketize2BufTag[ubBUF_AdoPacketize2BufIdx] != BUF_FREE)
		{
			printd(DBG_ErrorLvl, "AdoP2 Busy\n");		
		#if RTOS
			osSemaphoreRelease(tBUF_AdoPkt2Acc);
		#endif	
			return BUF_FAIL;
		}
		ubBUF_AdoPacketize2BufTag[ubBUF_AdoPacketize2BufIdx] = BUF_USED;		

	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt2Acc);
	#endif	
		return ulBUF_AdoPacketize2BufAddr[ubBUF_AdoPacketize2BufIdx];
	}
	else
	{		
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt2Acc);
	#endif			
		return BUF_FAIL;
	}
}

uint32_t ulBUF_GetAdoPacketize3FreeBuf(void)
{
	uint8_t i;
	uint8_t ubWithFreeBufFlg;
	
#if RTOS
	osSemaphoreWait(tBUF_AdoPkt3Acc, osWaitForever);
#endif
	
	ubWithFreeBufFlg = 0;
	
	for(i=0;i<ubBUF_AdoPacketize3BufNum;i++)
	{
		if(ubBUF_AdoPacketize3BufTag[i] == BUF_FREE)
		{
			ubWithFreeBufFlg = 1;
			break;
		}
	}
	
	if(ubWithFreeBufFlg)
	{
		if(++ubBUF_AdoPacketize3BufIdx >= ubBUF_AdoPacketize3BufNum)
			ubBUF_AdoPacketize3BufIdx = 0;

		if(ubBUF_AdoPacketize3BufTag[ubBUF_AdoPacketize3BufIdx] != BUF_FREE)
		{
			printd(DBG_ErrorLvl, "AdoP3 Busy\n");			
		#if RTOS
			osSemaphoreRelease(tBUF_AdoPkt3Acc);
		#endif
			return BUF_FAIL;
		}
		ubBUF_AdoPacketize3BufTag[ubBUF_AdoPacketize3BufIdx] = BUF_USED;		

	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt3Acc);
	#endif
		return ulBUF_AdoPacketize3BufAddr[ubBUF_AdoPacketize3BufIdx];
	}
	else
	{		
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt3Acc);
	#endif	
		return BUF_FAIL;
	}
}

uint32_t ulBUF_GetPacket0FreeBuf(void)
{
	uint8_t ubTemp;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_Packet0Acc, osWaitForever);
#endif
	
	if(++ubBUF_Packet0BufIdx >= ubBUF_Packet0BufNum)
		ubBUF_Packet0BufIdx = 0;

	if(ubBUF_Packet0BufTag[ubBUF_Packet0BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "P0 Busy\n");

		//Restore
		//==============================================
		if(ubBUF_Packet0BufIdx != 0)
		{
			ubBUF_Packet0BufIdx--;
		}
		else
		{
			ubBUF_Packet0BufIdx = ubBUF_Packet0BufNum-1;
		}		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet0Acc);
	#endif		
		return BUF_FAIL;
	}
	ubBUF_Packet0BufTag[ubBUF_Packet0BufIdx] = BUF_USED;
	ubTemp = ubBUF_Packet0BufIdx;
#ifdef RTOS
	osSemaphoreRelease(tBUF_Packet0Acc);
#endif	
	return ulBUF_Packet0BufAddr[ubTemp];
}

uint32_t ulBUF_GetPacket1FreeBuf(void)
{
	uint8_t ubTemp;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_Packet1Acc, osWaitForever);
#endif
	
	if(++ubBUF_Packet1BufIdx >= ubBUF_Packet1BufNum)
		ubBUF_Packet1BufIdx = 0;

	if(ubBUF_Packet1BufTag[ubBUF_Packet1BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "P1 Busy\n");
		
		//Restore
		//==============================================
		if(ubBUF_Packet1BufIdx != 0)
		{
			ubBUF_Packet1BufIdx--;
		}
		else
		{
			ubBUF_Packet1BufIdx = ubBUF_Packet1BufNum-1;
		}
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet1Acc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Packet1BufTag[ubBUF_Packet1BufIdx] = BUF_USED;	
	ubTemp = ubBUF_Packet1BufIdx;
#ifdef RTOS
	osSemaphoreRelease(tBUF_Packet1Acc);
#endif	
	return ulBUF_Packet1BufAddr[ubTemp];
}

uint32_t ulBUF_GetPacket2FreeBuf(void)
{
	uint8_t ubTemp;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_Packet2Acc, osWaitForever);
#endif
	if(++ubBUF_Packet2BufIdx >= ubBUF_Packet2BufNum)
		ubBUF_Packet2BufIdx = 0;

	if(ubBUF_Packet2BufTag[ubBUF_Packet2BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "P2 Busy\n");
		
		//Restore
		//==============================================
		if(ubBUF_Packet2BufIdx != 0)
		{
			ubBUF_Packet2BufIdx--;
		}
		else
		{
			ubBUF_Packet2BufIdx = ubBUF_Packet2BufNum-1;
		}
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet2Acc);
	#endif		
		return BUF_FAIL;
	}
	ubBUF_Packet2BufTag[ubBUF_Packet2BufIdx] = BUF_USED;
	ubTemp = ubBUF_Packet2BufIdx;
#ifdef RTOS
	osSemaphoreRelease(tBUF_Packet2Acc);
#endif	
	return ulBUF_Packet2BufAddr[ubTemp];
}

uint32_t ulBUF_GetPacket3FreeBuf(void)
{
	uint8_t ubTemp;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_Packet3Acc, osWaitForever);
#endif
	
	if(++ubBUF_Packet3BufIdx >= ubBUF_Packet3BufNum)
		ubBUF_Packet3BufIdx = 0;

	if(ubBUF_Packet3BufTag[ubBUF_Packet3BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "P3 Busy\n");
		
		//Restore
		//==============================================
		if(ubBUF_Packet3BufIdx != 0)
		{
			ubBUF_Packet3BufIdx--;
		}
		else
		{
			ubBUF_Packet3BufIdx = ubBUF_Packet3BufNum-1;
		}
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet3Acc);
	#endif	
		return BUF_FAIL;
	}
	ubBUF_Packet3BufTag[ubBUF_Packet3BufIdx] = BUF_USED;
	ubTemp = ubBUF_Packet3BufIdx;
#ifdef RTOS
	osSemaphoreRelease(tBUF_Packet3Acc);
#endif	
	return ulBUF_Packet3BufAddr[ubTemp];
}

#if (defined(OP_AP) && ((defined(S2019A) && defined(RVCS_APP)) || defined(sWIFIBDG)))
uint32_t ulBUF_GetVdoBdgBsFreeBuf(void)
{
	uint32_t ulBufAddr = 0;
	if(++ubBUF_VdoBdgBsBufIdx >= (ubBUF_VdoBdgBsBufNum - 1))
		ubBUF_VdoBdgBsBufIdx = 0;
	if(ubBUF_VdoBdgBsBufTag[ubBUF_VdoBdgBsBufIdx] != BUF_FREE)
	{
		printd(DBG_InfoLvl, "BDG BS Busy[%d]\n", ubBUF_VdoBdgBsBufNum);
		ubBUF_VdoBdgBsBufIdx = (!ubBUF_VdoBdgBsBufIdx)?(ubBUF_VdoBdgBsBufNum - 1):(ubBUF_VdoBdgBsBufIdx - 1);
		return BUF_FAIL;
	}
	ubBUF_VdoBdgBsBufTag[ubBUF_VdoBdgBsBufIdx] = BUF_USED;	
	ulBufAddr = ulBUF_VdoBdgBsBufAddr[ubBUF_VdoBdgBsBufIdx];
	return ulBufAddr;
}

uint8_t ubBUF_ChkVdoBdgBsBufIdx(void)
{
	if((ubBUF_VdoBdgBsBufIdx + 1) < (ubBUF_VdoBdgBsBufNum - 1))
		++ubBUF_VdoBdgBsBufIdx;
	return TRUE;
}

uint8_t ubBUF_ReleaseVdoBdgBsBuf(uint32_t ulBufAddr)
{	
	uint8_t ubBufIdx;
	for(ubBufIdx = 0; ubBufIdx < ubBUF_VdoBdgBsBufNum; ubBufIdx++)
	{
		if(ulBufAddr == ulBUF_VdoBdgBsBufAddr[ubBufIdx])
			break;
	}
	if(ubBufIdx == ubBUF_VdoBdgBsBufNum)
	{
		printd(DBG_ErrorLvl, "Fail @Bdg BS Release !\r\n");
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoBdgBsBufTag[ubBufIdx] = BUF_FREE;
		return BUF_OK;
	}
}

uint8_t ubBUF_GetVdoBdgBsFreeBufNum(void)
{
	uint8_t ubFreeBufNum = 0, ubBufIdx;
	for(ubBufIdx = 0; ubBufIdx < ubBUF_VdoBdgBsBufNum; ubBufIdx++)
	{
		if(BUF_FREE == ubBUF_VdoBdgBsBufTag[ubBufIdx])
			ubFreeBufNum++;
	}
	return ubFreeBufNum;
}

#endif
#ifdef S2019A
uint8_t ubBUF_GetVdoMainBsFreeBufNum(BUF_MODE tBufMode)
{
	uint8_t ubMaxBsBufNum = 0;
	uint8_t ubFreeBufNum = 0, ubBufIdx, ubBusyIdx = 0;
	uint8_t *pBsBufPool[4] = {
								[0] = &ubBUF_VdoMainBs0BufTag[0],
								[1] = &ubBUF_VdoMainBs1BufTag[0],
								[2] = &ubBUF_VdoMainBs2BufTag[0],
								[3] = &ubBUF_VdoMainBs3BufTag[0],
							};

	if((tBufMode < BUF_VDO_MAIN_BS0) || (tBufMode > BUF_VDO_MAIN_BS3))
		return 0;
	ubMaxBsBufNum = (BUF_VDO_MAIN_BS0 == tBufMode)?(ubBUF_VdoMainBs0BufNum - 1):
					(BUF_VDO_MAIN_BS1 == tBufMode)?(ubBUF_VdoMainBs1BufNum - 1):
					(BUF_VDO_MAIN_BS2 == tBufMode)?(ubBUF_VdoMainBs2BufNum - 1):
					(BUF_VDO_MAIN_BS3 == tBufMode)?(ubBUF_VdoMainBs3BufNum - 1):(BUF_NUM_VDO_BS - 1);
	ubBusyIdx = ubMaxBsBufNum;
	for(ubBufIdx = 0; ubBufIdx < ubMaxBsBufNum; ubBufIdx++)
	{
		if(BUF_FREE == pBsBufPool[(tBufMode-BUF_VDO_MAIN_BS0)][ubBufIdx])
		{
			ubFreeBufNum++;
		}
		else
		{
			if((ubBusyIdx < ubMaxBsBufNum) && ((ubBusyIdx + 1) != ubBufIdx) && (ubFreeBufNum))
				ubFreeBufNum--;
			ubBusyIdx = ubBufIdx;
		}
	}
	return ubFreeBufNum;
}

uint32_t ulBUF_GetVdoMainBsFreeBuf(BUF_MODE tBufMode, uint8_t ubisIfrm)
{
	uint8_t ubMaxBsBufNum = 0, ubBufErrFlag = FALSE;
	uint8_t ubPoolIdx = 0, ubBufIdx = 0;
	uint8_t *pBsBufPool[4]  = {
								[0] = &ubBUF_VdoMainBs0BufTag[0],
								[1] = &ubBUF_VdoMainBs1BufTag[0],
								[2] = &ubBUF_VdoMainBs2BufTag[0],
								[3] = &ubBUF_VdoMainBs3BufTag[0],
							  };
	uint8_t *pBsBufIdx[4]   = {
								[0] = &ubBUF_VdoMainBs0BufIdx,
								[1] = &ubBUF_VdoMainBs1BufIdx,
								[2] = &ubBUF_VdoMainBs2BufIdx,
								[3] = &ubBUF_VdoMainBs3BufIdx,	
						      };
	uint32_t *pBsBufAddr[4] = {
								[0] = &ulBUF_VdoMainBs0BufAddr[0],
								[1] = &ulBUF_VdoMainBs1BufAddr[0],
								[2] = &ulBUF_VdoMainBs2BufAddr[0],
								[3] = &ulBUF_VdoMainBs3BufAddr[0],
							  };

	if((tBufMode < BUF_VDO_MAIN_BS0) || (tBufMode > BUF_VDO_MAIN_BS3))
		return BUF_FAIL;
	ubPoolIdx = tBufMode-BUF_VDO_MAIN_BS0;
	ubMaxBsBufNum = (BUF_VDO_MAIN_BS0 == tBufMode)?(ubBUF_VdoMainBs0BufNum - 1):
					(BUF_VDO_MAIN_BS1 == tBufMode)?(ubBUF_VdoMainBs1BufNum - 1):
					(BUF_VDO_MAIN_BS2 == tBufMode)?(ubBUF_VdoMainBs2BufNum - 1):
					(BUF_VDO_MAIN_BS3 == tBufMode)?(ubBUF_VdoMainBs3BufNum - 1):(BUF_NUM_VDO_BS - 1);
	if(++(*pBsBufIdx[ubPoolIdx]) >= ubMaxBsBufNum)
		*pBsBufIdx[ubPoolIdx] = 0;
	ubBufIdx     = *pBsBufIdx[ubPoolIdx];
	ubBufErrFlag = (pBsBufPool[ubPoolIdx][ubBufIdx] != BUF_FREE)?TRUE:FALSE;
	if(TRUE == ubBufErrFlag)
	{
		printd(DBG_ErrorLvl, "[%d]BS Busy!\n", tBufMode);
		*pBsBufIdx[ubPoolIdx] = (!(*pBsBufIdx[ubPoolIdx]))?ubMaxBsBufNum:(*pBsBufIdx[ubPoolIdx] - 1);
		return BUF_FAIL;
	}
	pBsBufPool[ubPoolIdx][ubBufIdx] = BUF_USED;
	return pBsBufAddr[ubPoolIdx][ubBufIdx];
}

uint8_t ubBUF_ChkVdoMainBsBufIdx(BUF_MODE tBufMode, uint8_t ubisIfrm)
{
	uint8_t ubPoolIdx, ubMaxBsBufNum = 0, ubSkip = TRUE;
	uint8_t *pBsBufIdx[4] = {
								[0] = &ubBUF_VdoMainBs0BufIdx,
								[1] = &ubBUF_VdoMainBs1BufIdx,
								[2] = &ubBUF_VdoMainBs2BufIdx,
								[3] = &ubBUF_VdoMainBs3BufIdx,	
							};
	if((tBufMode < BUF_VDO_MAIN_BS0) || (tBufMode > BUF_VDO_MAIN_BS3))
		return ubSkip;
	ubMaxBsBufNum = (BUF_VDO_MAIN_BS0 == tBufMode)?(ubBUF_VdoMainBs0BufNum - 1):
					(BUF_VDO_MAIN_BS1 == tBufMode)?(ubBUF_VdoMainBs1BufNum - 1):
					(BUF_VDO_MAIN_BS2 == tBufMode)?(ubBUF_VdoMainBs2BufNum - 1):
					(BUF_VDO_MAIN_BS3 == tBufMode)?(ubBUF_VdoMainBs3BufNum - 1):(BUF_NUM_VDO_BS - 1);
	ubPoolIdx = tBufMode-BUF_VDO_MAIN_BS0;
	if((*pBsBufIdx[ubPoolIdx] + 1) < ubMaxBsBufNum)
		++(*pBsBufIdx[ubPoolIdx]);
	return ubSkip;
}

#else	//! #ifdef S2019A

uint16_t uwBUF_GetVdoMainBs0FreeNum(void)
{
	uint16_t uwTemp,i;
	

	uwTemp = 0;
	
	for(i=0;i<ubBUF_VdoMainBs0BufNum;i++)
	{
		if(ubBUF_VdoMainBs0BufTag[i] == BUF_FREE)
			uwTemp++;
	}
	

	return uwTemp;
}

uint16_t uwBUF_GetVdoMainBs1FreeNum(void)
{
	uint16_t uwTemp,i;
	

	
	uwTemp = 0;
	
	for(i=0;i<ubBUF_VdoMainBs1BufNum;i++)
	{
		if(ubBUF_VdoMainBs1BufTag[i] == BUF_FREE)
			uwTemp++;
	}

	return uwTemp;
}

uint16_t uwBUF_GetVdoMainBs2FreeNum(void)
{
	uint16_t uwTemp,i;
	uint8_t ubTemp;	
	

	
	uwTemp = 0;

#if defined(BSP_DVR_SDK)
	ubTemp = ubBUF_VdoMainBs2BufNum;
#else
	ubTemp = ubBUF_VdoMainBs2BufNum;
#endif
	
	for(i=0;i<ubTemp;i++)
	{
		if(ubBUF_VdoMainBs2BufTag[i] == BUF_FREE)
			uwTemp++;
	}
	

	return uwTemp;
}


uint16_t uwBUF_GetVdoMainBs3FreeNum(void)
{
	uint16_t uwTemp,i;
	

	
	uwTemp = 0;
	
	for(i=0;i<ubBUF_VdoMainBs3BufNum;i++)
	{
		if(ubBUF_VdoMainBs3BufTag[i] == BUF_FREE)
			uwTemp++;
	}
	

	return uwTemp;
}

uint32_t ulBUF_GetVdoMainBs0FreeBuf(void)
{
	uint32_t ulTemp;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_MainBs0BufAcc, osWaitForever);
#endif
	
	if(++ubBUF_VdoMainBs0BufIdx >= ubBUF_VdoMainBs0BufNum)
		ubBUF_VdoMainBs0BufIdx = 0;

	if(ubBUF_VdoMainBs0BufTag[ubBUF_VdoMainBs0BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "MBS0 Busy\n");
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs0BufAcc);
	#endif		
		return BUF_FAIL;
	}
	ubBUF_VdoMainBs0BufTag[ubBUF_VdoMainBs0BufIdx] = BUF_USED;	
	ulTemp = ulBUF_VdoMainBs0BufAddr[ubBUF_VdoMainBs0BufIdx];
	
#ifdef RTOS
	osSemaphoreRelease(tBUF_MainBs0BufAcc);
#endif
	return ulTemp;
}

uint32_t ulBUF_GetVdoMainBs1FreeBuf(void)
{
	uint32_t ulTemp;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_MainBs1BufAcc, osWaitForever);
#endif
	
	if(++ubBUF_VdoMainBs1BufIdx >= ubBUF_VdoMainBs1BufNum)
		ubBUF_VdoMainBs1BufIdx = 0;

	if(ubBUF_VdoMainBs1BufTag[ubBUF_VdoMainBs1BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "MBS1 Busy\n");
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs1BufAcc);
	#endif		
		return BUF_FAIL;
	}
	ubBUF_VdoMainBs1BufTag[ubBUF_VdoMainBs1BufIdx] = BUF_USED;	
	ulTemp = ulBUF_VdoMainBs1BufAddr[ubBUF_VdoMainBs1BufIdx];
	
#ifdef RTOS
	osSemaphoreRelease(tBUF_MainBs1BufAcc);
#endif
	return ulTemp;
}

uint32_t ulBUF_GetVdoMainBs2FreeBuf(void)
{
	uint32_t ulTemp;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_MainBs2BufAcc, osWaitForever);
#endif
	
#if defined(BSP_DVR_SDK)
	if(++ubBUF_VdoMainBs2BufIdx >= ubBUF_VdoMainBs2BufNum)
#else
	if(++ubBUF_VdoMainBs2BufIdx >= ubBUF_VdoMainBs2BufNum)
#endif
		ubBUF_VdoMainBs2BufIdx = 0;

	if(ubBUF_VdoMainBs2BufTag[ubBUF_VdoMainBs2BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "MBS2 Busy\n");
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs2BufAcc);
	#endif		
		return BUF_FAIL;
	}
	ubBUF_VdoMainBs2BufTag[ubBUF_VdoMainBs2BufIdx] = BUF_USED;	
	ulTemp = ulBUF_VdoMainBs2BufAddr[ubBUF_VdoMainBs2BufIdx];
	
#ifdef RTOS
	osSemaphoreRelease(tBUF_MainBs2BufAcc);
#endif
	return ulTemp;
}

uint32_t ulBUF_GetVdoMainBs3FreeBuf(void)
{
	uint32_t ulTemp;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_MainBs3BufAcc, osWaitForever);
#endif
	
	if(++ubBUF_VdoMainBs3BufIdx >= ubBUF_VdoMainBs3BufNum)
		ubBUF_VdoMainBs3BufIdx = 0;

	if(ubBUF_VdoMainBs3BufTag[ubBUF_VdoMainBs3BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "MBS3 Busy\n");
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs3BufAcc);
	#endif		
		return BUF_FAIL;
	}
	ubBUF_VdoMainBs3BufTag[ubBUF_VdoMainBs3BufIdx] = BUF_USED;	
	ulTemp = ulBUF_VdoMainBs3BufAddr[ubBUF_VdoMainBs3BufIdx];
	
#ifdef RTOS
	osSemaphoreRelease(tBUF_MainBs3BufAcc);
#endif
	return ulTemp;
}
#endif	//! End of #ifdef S2019A

uint32_t ulBUF_GetVdoAuxBs0FreeBuf(void)
{
	if(++ubBUF_VdoAuxBs0BufIdx >= ubBUF_VdoAuxBs0BufNum)
		ubBUF_VdoAuxBs0BufIdx = 0;

	if(ubBUF_VdoAuxBs0BufTag[ubBUF_VdoAuxBs0BufIdx] != BUF_FREE)
	{		
		printd(DBG_ErrorLvl, "ABS0 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoAuxBs0BufTag[ubBUF_VdoAuxBs0BufIdx] = BUF_USED;
	return ulBUF_VdoAuxBs0BufAddr[ubBUF_VdoAuxBs0BufIdx];
}

uint32_t ulBUF_GetVdoAuxBs1FreeBuf(void)
{	
	if(++ubBUF_VdoAuxBs1BufIdx >= ubBUF_VdoAuxBs1BufNum)
		ubBUF_VdoAuxBs1BufIdx = 0;

	if(ubBUF_VdoAuxBs1BufTag[ubBUF_VdoAuxBs1BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "ABS1 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoAuxBs1BufTag[ubBUF_VdoAuxBs1BufIdx] = BUF_USED;
	return ulBUF_VdoAuxBs1BufAddr[ubBUF_VdoAuxBs1BufIdx];
}

uint32_t ulBUF_GetVdoAuxBs2FreeBuf(void)
{
	if(++ubBUF_VdoAuxBs2BufIdx >= ubBUF_VdoAuxBs2BufNum)
		ubBUF_VdoAuxBs2BufIdx = 0;

	if(ubBUF_VdoAuxBs2BufTag[ubBUF_VdoAuxBs2BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "ABS2 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoAuxBs2BufTag[ubBUF_VdoAuxBs2BufIdx] = BUF_USED;
	return ulBUF_VdoAuxBs2BufAddr[ubBUF_VdoAuxBs2BufIdx];
}

uint32_t ulBUF_GetVdoAuxBs3FreeBuf(void)
{
	if(++ubBUF_VdoAuxBs3BufIdx >= ubBUF_VdoAuxBs3BufNum)
		ubBUF_VdoAuxBs3BufIdx = 0;

	if(ubBUF_VdoAuxBs3BufTag[ubBUF_VdoAuxBs3BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "ABS3 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoAuxBs3BufTag[ubBUF_VdoAuxBs3BufIdx] = BUF_USED;
	return ulBUF_VdoAuxBs3BufAddr[ubBUF_VdoAuxBs3BufIdx];
}

uint32_t ulBUF_GetVdoSubBs00FreeBuf(void)
{
	if(++ubBUF_VdoSubBs00BufIdx >= ubBUF_VdoSubBs00BufNum)
		ubBUF_VdoSubBs00BufIdx = 0;

	if(ubBUF_VdoSubBs00BufTag[ubBUF_VdoSubBs00BufIdx] != BUF_FREE)
	{		
		printd(DBG_ErrorLvl, "SBS00 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs00BufTag[ubBUF_VdoSubBs00BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs00BufAddr[ubBUF_VdoSubBs00BufIdx];
}

uint32_t ulBUF_GetVdoSubBs10FreeBuf(void)
{
	if(++ubBUF_VdoSubBs10BufIdx >= ubBUF_VdoSubBs10BufNum)
		ubBUF_VdoSubBs10BufIdx = 0;

	if(ubBUF_VdoSubBs10BufTag[ubBUF_VdoSubBs10BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "SBS10 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs10BufTag[ubBUF_VdoSubBs10BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs10BufAddr[ubBUF_VdoSubBs10BufIdx];	
}

uint32_t ulBUF_GetVdoSubBs20FreeBuf(void)
{
	if(++ubBUF_VdoSubBs20BufIdx >= ubBUF_VdoSubBs20BufNum)
		ubBUF_VdoSubBs20BufIdx = 0;

	if(ubBUF_VdoSubBs20BufTag[ubBUF_VdoSubBs20BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "SBS20 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs20BufTag[ubBUF_VdoSubBs20BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs20BufAddr[ubBUF_VdoSubBs20BufIdx];
}

uint32_t ulBUF_GetVdoSubBs30FreeBuf(void)
{
	if(++ubBUF_VdoSubBs30BufIdx >= ubBUF_VdoSubBs30BufNum)
		ubBUF_VdoSubBs30BufIdx = 0;

	if(ubBUF_VdoSubBs30BufTag[ubBUF_VdoSubBs30BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "SBS30 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs30BufTag[ubBUF_VdoSubBs30BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs30BufAddr[ubBUF_VdoSubBs30BufIdx];
}

//-----------------------------------------------------------------------------------
uint32_t ulBUF_GetVdoSubBs01FreeBuf(void)
{
	if(++ubBUF_VdoSubBs01BufIdx >= ubBUF_VdoSubBs01BufNum)
		ubBUF_VdoSubBs01BufIdx = 0;

	if(ubBUF_VdoSubBs01BufTag[ubBUF_VdoSubBs01BufIdx] != BUF_FREE)
	{		
		printd(DBG_ErrorLvl, "SBS01 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs01BufTag[ubBUF_VdoSubBs01BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs01BufAddr[ubBUF_VdoSubBs01BufIdx];	
}

uint32_t ulBUF_GetVdoSubBs11FreeBuf(void)
{
	if(++ubBUF_VdoSubBs11BufIdx >= ubBUF_VdoSubBs11BufNum)
		ubBUF_VdoSubBs11BufIdx = 0;

	if(ubBUF_VdoSubBs11BufTag[ubBUF_VdoSubBs11BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "SBS11 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs11BufTag[ubBUF_VdoSubBs11BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs11BufAddr[ubBUF_VdoSubBs11BufIdx];	
}

uint32_t ulBUF_GetVdoSubBs21FreeBuf(void)
{
	if(++ubBUF_VdoSubBs21BufIdx >= ubBUF_VdoSubBs21BufNum)
		ubBUF_VdoSubBs21BufIdx = 0;

	if(ubBUF_VdoSubBs21BufTag[ubBUF_VdoSubBs21BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "SBS21 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs21BufTag[ubBUF_VdoSubBs21BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs21BufAddr[ubBUF_VdoSubBs21BufIdx];
}

uint32_t ulBUF_GetVdoSubBs31FreeBuf(void)
{
	if(++ubBUF_VdoSubBs31BufIdx >= ubBUF_VdoSubBs31BufNum)
		ubBUF_VdoSubBs31BufIdx = 0;

	if(ubBUF_VdoSubBs31BufTag[ubBUF_VdoSubBs31BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "SBS31 Busy\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs31BufTag[ubBUF_VdoSubBs31BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs31BufAddr[ubBUF_VdoSubBs31BufIdx];
}

uint32_t ulBUF_GetAdcFreeBuf(void)
{
#if RTOS	
	osSemaphoreWait(tBUF_AdcBufAcc, osWaitForever);
#endif
	if(++ubBUF_AdcBufIdx >= BUF_NUM_ADC)
		ubBUF_AdcBufIdx = 0;

	if(ubBUF_AdcBufTag[ubBUF_AdcBufIdx] == BUF_USED)
	{		
		printd(DBG_ErrorLvl, "AdcB Busy\n");
	#if RTOS
		osSemaphoreRelease(tBUF_AdcBufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_AdcBufTag[ubBUF_AdcBufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_AdcBufAcc);
#endif
	return ulBUF_AdcBufAddr[ubBUF_AdcBufIdx];
}

uint32_t ulBUF_GetDac0FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac0BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac0BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac0BufIdx = 0;

	if(ubBUF_Dac0BufTag[ubBUF_Dac0BufIdx] == BUF_USED)
	{		
		printd(DBG_ErrorLvl, "Dac0B Busy\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac0BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac0BufTag[ubBUF_Dac0BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac0BufAcc);
#endif
	return ulBUF_Dac0BufAddr[ubBUF_Dac0BufIdx];
}

uint32_t ulBUF_GetDac1FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac1BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac1BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac1BufIdx = 0;

	if(ubBUF_Dac1BufTag[ubBUF_Dac1BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Dac1B Busy\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac1BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac1BufTag[ubBUF_Dac1BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac1BufAcc);
#endif
	return ulBUF_Dac1BufAddr[ubBUF_Dac1BufIdx];
}

uint32_t ulBUF_GetDac2FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac2BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac2BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac2BufIdx = 0;

	if(ubBUF_Dac2BufTag[ubBUF_Dac2BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Dac2B Busy\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac2BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac2BufTag[ubBUF_Dac2BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac2BufAcc);
#endif
	return ulBUF_Dac2BufAddr[ubBUF_Dac2BufIdx];
}

uint32_t ulBUF_GetDac3FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac3BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac3BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac3BufIdx = 0;

	if(ubBUF_Dac3BufTag[ubBUF_Dac3BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Dac3B Busy\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac3BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac3BufTag[ubBUF_Dac3BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac3BufAcc);
#endif
	return ulBUF_Dac3BufAddr[ubBUF_Dac3BufIdx];
}

uint32_t ulBUF_GetDac4FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac4BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac4BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac4BufIdx = 0;

	if(ubBUF_Dac4BufTag[ubBUF_Dac4BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Dac4B Busy\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac4BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac4BufTag[ubBUF_Dac4BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac4BufAcc);
#endif
	return ulBUF_Dac4BufAddr[ubBUF_Dac4BufIdx];
}

uint32_t ulBUF_GetDac5FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac5BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac5BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac5BufIdx = 0;

	if(ubBUF_Dac4BufTag[ubBUF_Dac5BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Dac5B Busy\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac5BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac5BufTag[ubBUF_Dac5BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac5BufAcc);
#endif
	return ulBUF_Dac5BufAddr[ubBUF_Dac5BufIdx];
}

uint8_t ubBUF_ReleaseSenYuvBuf(uint32_t ulBufAddr)
{
	uint8_t ubPath1Flg = 0;
	uint8_t ubPath2Flg = 0;
	uint8_t ubPath3Flg = 0;
	uint8_t ubBufIdx   = 0;

	//Path1
	for(ubBufIdx=0;ubBufIdx<ubBUF_Sen1YuvBufNum;ubBufIdx++)
	{
		if(ulBUF_Sen1YuvBufAddr[ubBufIdx] == ulBufAddr)
		{
			ubPath1Flg = 1;
			break;
		}
	}
	if(!ubPath1Flg)
	{
		//Path2
		for(ubBufIdx=0;ubBufIdx<ubBUF_Sen2YuvBufNum;ubBufIdx++)
		{
			if(ulBUF_Sen2YuvBufAddr[ubBufIdx] == ulBufAddr)
			{
				ubPath2Flg = 1;
				break;
			}
		}
	}
	
	if((!ubPath1Flg)  && (!ubPath2Flg))
	{
		//Path3
		for(ubBufIdx=0;ubBufIdx<ubBUF_Sen3YuvBufNum;ubBufIdx++)
		{
			if(ulBUF_Sen3YuvBufAddr[ubBufIdx] == ulBufAddr)
			{
				ubPath3Flg = 1;
				break;
			}
		}
	}	

	if(ubPath1Flg)
	{
		return ubBUF_ReleaseSen1YuvBuf(ubBufIdx);
	}
	else if(ubPath2Flg)
	{
		return ubBUF_ReleaseSen2YuvBuf(ubBufIdx);
	}
	else if(ubPath3Flg)
	{
		return ubBUF_ReleaseSen3YuvBuf(ubBufIdx);
	}
	else
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_SenReleaseBuf !!!!\r\n");
		return BUF_FAIL;
	}
}

uint8_t ubBUF_ReleaseSen1YuvBuf(uint8_t ubBufIdx)
{
#if RTOS
	osSemaphoreWait(tBUF_Sen1YuvAcc, osWaitForever);
#endif	
	ubBUF_Sen1YuvBufTag[ubBufIdx] = BUF_FREE;		
#if RTOS
	osSemaphoreRelease(tBUF_Sen1YuvAcc);
#endif		
	return BUF_OK;	
}

uint8_t ubBUF_ReleaseSen2YuvBuf(uint8_t ubBufIdx)
{
#if RTOS
	osSemaphoreWait(tBUF_Sen2YuvAcc, osWaitForever);
#endif
	ubBUF_Sen2YuvBufTag[ubBufIdx] = BUF_FREE;		
#if RTOS
	osSemaphoreRelease(tBUF_Sen2YuvAcc);
#endif
	return BUF_OK;
}

uint8_t ubBUF_ReleaseSen3YuvBuf(uint8_t ubBufIdx)
{
#if RTOS
	osSemaphoreWait(tBUF_Sen3YuvAcc, osWaitForever);
#endif
	ubBUF_Sen3YuvBufTag[ubBufIdx] = BUF_FREE;		
#if RTOS
	osSemaphoreRelease(tBUF_Sen3YuvAcc);
#endif
	return BUF_OK;	
}



uint8_t ubBUF_ReleaseVdoPacketizeBigBuf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPacketizeBigAcc, osWaitForever);
#endif
	
	ubBufIdx = ubBUF_VdoPacketizeBigBufNum;
	
	for(i=0;i<ubBUF_VdoPacketizeBigBufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoPacketizeBigBufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == ubBUF_VdoPacketizeBigBufNum)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);
		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPacketizeBigAcc);
	#endif		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoPacketizeBigBufTag[ubBufIdx] = BUF_FREE;
		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPacketizeBigAcc);
	#endif		
		return BUF_OK;
	}
}
uint8_t ubBUF_ReleaseVdoPacketizeSmallBuf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPacketizeSmallAcc, osWaitForever);
#endif
	
	ubBufIdx = ubBUF_VdoPacketizeSmallBufNum;
	
	for(i=0;i<ubBUF_VdoPacketizeSmallBufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoPacketizeSmallBufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == ubBUF_VdoPacketizeSmallBufNum)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);
		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPacketizeSmallAcc);
	#endif		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoPacketizeSmallBufTag[ubBufIdx] = BUF_FREE;
		
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPacketizeSmallAcc);
	#endif		
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseAdoPacketizeBuf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_ADO_PACKETIZE;
	
#if RTOS
	osSemaphoreWait(tBUF_AdoPacketizeAcc, osWaitForever);
#endif

	for(i=0;i<BUF_NUM_ADO_PACKETIZE;i++)
	{
		if(ulBufAddr == ulBUF_AdoPacketizeBufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_ADO_PACKETIZE)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);
		
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPacketizeAcc);
	#endif		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_AdoPacketizeBufTag[ubBufIdx] = BUF_FREE;
		
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPacketizeAcc);
	#endif		
		return BUF_OK;
	}
}

//Video
uint8_t ubBUF_ReleaseVdoPacketize0Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPkt0Acc, osWaitForever);
#endif
	ubBufIdx = BUF_NUM_VDO_PACKETIZE_RCV;
	
	for(i=0;i<BUF_NUM_VDO_PACKETIZE_RCV;i++)
	{
		if(ulBufAddr == ulBUF_VdoPacketize0BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_VDO_PACKETIZE_RCV)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);			
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt0Acc);
	#endif	
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoPacketize0BufTag[ubBufIdx] = BUF_FREE;	
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt0Acc);
	#endif	
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseVdoPacketize1Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	

#if RTOS
	osSemaphoreWait(tBUF_VdoPkt1Acc, osWaitForever);
#endif
	
	ubBufIdx = BUF_NUM_VDO_PACKETIZE_RCV;
	
	for(i=0;i<BUF_NUM_VDO_PACKETIZE_RCV;i++)
	{
		if(ulBufAddr == ulBUF_VdoPacketize1BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_VDO_PACKETIZE_RCV)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);			
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt1Acc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoPacketize1BufTag[ubBufIdx] = BUF_FREE;	
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt1Acc);
	#endif
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseVdoPacketize2Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPkt2Acc, osWaitForever);
#endif
	
	ubBufIdx = BUF_NUM_VDO_PACKETIZE_RCV;	

	for(i=0;i<BUF_NUM_VDO_PACKETIZE_RCV;i++)
	{
		if(ulBufAddr == ulBUF_VdoPacketize2BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_VDO_PACKETIZE_RCV)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);			
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt2Acc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoPacketize2BufTag[ubBufIdx] = BUF_FREE;	
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt2Acc);
	#endif
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseVdoPacketize3Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	
	
#if RTOS
	osSemaphoreWait(tBUF_VdoPkt3Acc, osWaitForever);
#endif
	
	ubBufIdx = BUF_NUM_VDO_PACKETIZE_RCV;

	for(i=0;i<BUF_NUM_VDO_PACKETIZE_RCV;i++)
	{
		if(ulBufAddr == ulBUF_VdoPacketize3BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_VDO_PACKETIZE_RCV)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);			
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt3Acc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoPacketize3BufTag[ubBufIdx] = BUF_FREE;	
	#if RTOS
		osSemaphoreRelease(tBUF_VdoPkt3Acc);
	#endif
		return BUF_OK;
	}
}


//Audio
uint8_t ubBUF_ReleaseAdoPacketize0Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	

#if RTOS
	osSemaphoreWait(tBUF_AdoPkt0Acc, osWaitForever);
#endif
	
	ubBufIdx = BUF_NUM_ADO_PACKETIZE_RCV;
	
	for(i=0;i<BUF_NUM_ADO_PACKETIZE_RCV;i++)
	{
		if(ulBufAddr == ulBUF_AdoPacketize0BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_ADO_PACKETIZE_RCV)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);			
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt0Acc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_AdoPacketize0BufTag[ubBufIdx] = BUF_FREE;	
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt0Acc);
	#endif
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseAdoPacketize1Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	

#if RTOS
	osSemaphoreWait(tBUF_AdoPkt1Acc, osWaitForever);
#endif
	ubBufIdx = BUF_NUM_ADO_PACKETIZE_RCV;	
	
	for(i=0;i<BUF_NUM_ADO_PACKETIZE_RCV;i++)
	{
		if(ulBufAddr == ulBUF_AdoPacketize1BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_ADO_PACKETIZE_RCV)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);			
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt1Acc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_AdoPacketize1BufTag[ubBufIdx] = BUF_FREE;	
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt1Acc);
	#endif
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseAdoPacketize2Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	

#if RTOS
	osSemaphoreWait(tBUF_AdoPkt2Acc, osWaitForever);
#endif
	ubBufIdx = BUF_NUM_ADO_PACKETIZE_RCV;	
	
	for(i=0;i<BUF_NUM_ADO_PACKETIZE_RCV;i++)
	{
		if(ulBufAddr == ulBUF_AdoPacketize2BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_ADO_PACKETIZE_RCV)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);			
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt2Acc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_AdoPacketize2BufTag[ubBufIdx] = BUF_FREE;	
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt2Acc);
	#endif
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseAdoPacketize3Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx;	

#if RTOS
	osSemaphoreWait(tBUF_AdoPkt3Acc, osWaitForever);
#endif
	ubBufIdx = BUF_NUM_ADO_PACKETIZE_RCV;
	
	for(i=0;i<BUF_NUM_ADO_PACKETIZE_RCV;i++)
	{
		if(ulBufAddr == ulBUF_AdoPacketize3BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_ADO_PACKETIZE_RCV)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);			
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt3Acc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_AdoPacketize3BufTag[ubBufIdx] = BUF_FREE;	
	#if RTOS
		osSemaphoreRelease(tBUF_AdoPkt3Acc);
	#endif
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleasePacket0Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_PACKET;

#ifdef RTOS
	osSemaphoreWait(tBUF_Packet0Acc, osWaitForever);
#endif
	
	ubBufIdx = BUF_NUM_PACKET;
	
	for(i=0;i<BUF_NUM_PACKET;i++)
	{
		if(ulBufAddr == ulBUF_Packet0BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_PACKET)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet0Acc);
	#endif		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_Packet0BufTag[ubBufIdx] = BUF_FREE;
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet0Acc);
	#endif	
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleasePacket1Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_PACKET;

#ifdef RTOS
	osSemaphoreWait(tBUF_Packet1Acc, osWaitForever);
#endif
	
	ubBufIdx = BUF_NUM_PACKET;
	
	for(i=0;i<BUF_NUM_PACKET;i++)
	{
		if(ulBufAddr == ulBUF_Packet1BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_PACKET)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet1Acc);
	#endif		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_Packet1BufTag[ubBufIdx] = BUF_FREE;
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet1Acc);
	#endif
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleasePacket2Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_PACKET;

#ifdef RTOS
	osSemaphoreWait(tBUF_Packet2Acc, osWaitForever);
#endif
	
	ubBufIdx = BUF_NUM_PACKET;
	
	for(i=0;i<BUF_NUM_PACKET;i++)
	{
		if(ulBufAddr == ulBUF_Packet2BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_PACKET)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet2Acc);
	#endif		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_Packet2BufTag[ubBufIdx] = BUF_FREE;
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet2Acc);
	#endif		
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleasePacket3Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_PACKET;

#ifdef RTOS
	osSemaphoreWait(tBUF_Packet3Acc, osWaitForever);
#endif
	
	ubBufIdx = BUF_NUM_PACKET;
	
	for(i=0;i<BUF_NUM_PACKET;i++)
	{
		if(ulBufAddr == ulBUF_Packet3BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_PACKET)
	{
		printd(DBG_ErrorLvl, "Fail @%s !!!!\r\n",__func__);		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet3Acc);
	#endif		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_Packet3BufTag[ubBufIdx] = BUF_FREE;
	#ifdef RTOS
		osSemaphoreRelease(tBUF_Packet3Acc);
	#endif		
		return BUF_OK;
	}
}
uint8_t ubBUF_ReleaseVdoMainBs0Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx;

#ifdef RTOS
	osSemaphoreWait(tBUF_MainBs0BufAcc, osWaitForever);
#endif
	
	ubBufIdx = ubBUF_VdoMainBs0BufNum;
	
	for(i=0;i<ubBUF_VdoMainBs0BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoMainBs0BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == ubBUF_VdoMainBs0BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoMainBs0ReleaseBuf !!!!\r\n");
		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs0BufAcc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoMainBs0BufTag[ubBufIdx] = BUF_FREE;
		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs0BufAcc);
	#endif		
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseVdoMainBs1Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx;

#ifdef RTOS
	osSemaphoreWait(tBUF_MainBs1BufAcc, osWaitForever);
#endif
	
	ubBufIdx = ubBUF_VdoMainBs1BufNum;
	
	for(i=0;i<ubBUF_VdoMainBs1BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoMainBs1BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == ubBUF_VdoMainBs1BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoMainBs1ReleaseBuf !!!!\r\n");
		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs1BufAcc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoMainBs1BufTag[ubBufIdx] = BUF_FREE;
		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs1BufAcc);
	#endif		
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseVdoMainBs2Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx;
	
#ifdef RTOS
	osSemaphoreWait(tBUF_MainBs2BufAcc, osWaitForever);
#endif
	
#if defined(BSP_DVR_SDK)
	ubBufIdx = ubBUF_VdoMainBs2BufNum;
#else
	ubBufIdx = ubBUF_VdoMainBs2BufNum;
#endif

#if defined(BSP_DVR_SDK)
	for(i=0;i<ubBUF_VdoMainBs2BufNum;i++)
#else
	for(i=0;i<ubBUF_VdoMainBs2BufNum;i++)
#endif	
	{
		if(ulBufAddr == ulBUF_VdoMainBs2BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
#if defined(BSP_DVR_SDK)
	if(ubBufIdx == ubBUF_VdoMainBs2BufNum)
#else
	if(ubBufIdx == ubBUF_VdoMainBs2BufNum)
#endif
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoMainBs2ReleaseBuf !!!!\r\n");		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs2BufAcc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoMainBs2BufTag[ubBufIdx] = BUF_FREE;		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs2BufAcc);
	#endif
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoMainBs3Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx;

#ifdef RTOS
	osSemaphoreWait(tBUF_MainBs3BufAcc, osWaitForever);
#endif
	
	ubBufIdx = ubBUF_VdoMainBs3BufNum;
	
	for(i=0;i<ubBUF_VdoMainBs3BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoMainBs3BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoMainBs3BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoMainBs3ReleaseBuf !!!!\r\n");
		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs3BufAcc);
	#endif
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoMainBs3BufTag[ubBufIdx] = BUF_FREE;
		
	#ifdef RTOS
		osSemaphoreRelease(tBUF_MainBs3BufAcc);
	#endif		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoAuxBs0Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoAuxBs0BufNum;

	for(i=0;i<ubBUF_VdoAuxBs0BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoAuxBs0BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoAuxBs0BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoAuxBs0ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoAuxBs0BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoAuxBs1Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoAuxBs1BufNum;

	for(i=0;i<ubBUF_VdoAuxBs1BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoAuxBs1BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoAuxBs1BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoAuxBs1ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoAuxBs1BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoAuxBs2Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoAuxBs2BufNum;

	for(i=0;i<ubBUF_VdoAuxBs2BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoAuxBs2BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoAuxBs2BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoAuxBs2ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoAuxBs2BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoAuxBs3Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoAuxBs3BufNum;

	for(i=0;i<ubBUF_VdoAuxBs3BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoAuxBs3BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoAuxBs3BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoAuxBs3ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoAuxBs3BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs00Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoSubBs00BufNum;

	for(i=0;i<ubBUF_VdoSubBs00BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs00BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoSubBs00BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs00ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs00BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs10Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoSubBs10BufNum;

	for(i=0;i<ubBUF_VdoSubBs10BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs10BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoSubBs10BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs10ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs10BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs20Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoSubBs20BufNum;

	for(i=0;i<ubBUF_VdoSubBs20BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs20BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoSubBs20BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs20ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs20BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs30Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoSubBs30BufNum;

	for(i=0;i<ubBUF_VdoSubBs30BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs30BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoSubBs30BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs30ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs30BufTag[ubBufIdx] = BUF_FREE;
		return BUF_OK;
	}	
}


//-------------------------------------------------------------------------------
uint8_t ubBUF_ReleaseVdoSubBs01Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoSubBs01BufNum;

	for(i=0;i<ubBUF_VdoSubBs01BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs01BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoSubBs01BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs01ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs01BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs11Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoSubBs11BufNum;

	for(i=0;i<ubBUF_VdoSubBs11BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs11BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoSubBs11BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs11ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs11BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs21Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoSubBs21BufNum;

	for(i=0;i<ubBUF_VdoSubBs21BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs21BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoSubBs21BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs21ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs21BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs31Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = ubBUF_VdoSubBs31BufNum;
	
	for(i=0;i<ubBUF_VdoSubBs31BufNum;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs31BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == ubBUF_VdoSubBs31BufNum)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs31ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs31BufTag[ubBufIdx] = BUF_FREE;
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseAdcBuf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_ADC, ubAdcResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_AdcBufAcc, osWaitForever);
#endif
	for(i=0;i<BUF_NUM_ADC;i++)
	{
		if(ulBufAddr == ulBUF_AdcBufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_ADC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_AdcReleaseBuf !!!!\r\n");
		ubAdcResult = BUF_FAIL;
	}
	else
	{
		ubBUF_AdcBufTag[ubBufIdx] = BUF_FREE;
	}	
#if RTOS
	osSemaphoreRelease(tBUF_AdcBufAcc);
#endif
	return ubAdcResult;
}

uint8_t ubBUF_ReleaseDac0Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_Dac0BufAcc, osWaitForever);
#endif
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac0BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac0ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac0BufTag[ubBufIdx] = BUF_FREE;
	}
#if RTOS
	osSemaphoreRelease(tBUF_Dac0BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac1Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_Dac1BufAcc, osWaitForever);
#endif
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac1BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac1ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac1BufTag[ubBufIdx] = BUF_FREE;
	}
#if RTOS
	osSemaphoreRelease(tBUF_Dac1BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac2Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;
	
#if RTOS
	osSemaphoreWait(tBUF_Dac2BufAcc, osWaitForever);
#endif	
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac2BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac2ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac2BufTag[ubBufIdx] = BUF_FREE;
	}	
#if RTOS
	osSemaphoreRelease(tBUF_Dac2BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac3Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_Dac3BufAcc, osWaitForever);
#endif	
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac3BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac3ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac3BufTag[ubBufIdx] = BUF_FREE;
	}
#if RTOS
	osSemaphoreRelease(tBUF_Dac3BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac4Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS	
	osSemaphoreWait(tBUF_Dac4BufAcc, osWaitForever);
#endif	
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac4BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac4ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac4BufTag[ubBufIdx] = BUF_FREE;
	}	
#if RTOS
	osSemaphoreRelease(tBUF_Dac4BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac5Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_Dac5BufAcc, osWaitForever);
#endif
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac5BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac5ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac5BufTag[ubBufIdx] = BUF_FREE;
	}
#if RTOS
	osSemaphoreRelease(tBUF_Dac5BufAcc);
#endif
	return ubDacResult;
}

uint32_t ulBUF_GetVdoUsbdFreeBuf(void)
{
	uint8_t ubBufIdx;
	
#if RTOS
	osSemaphoreWait(tBUF_UsbdVdoAcc, osWaitForever);
#endif
	for(ubBufIdx = 0; ubBufIdx < ubBUF_VdoUsbdBufNum; ubBufIdx++)
	{
		if(ubBUF_VdoUsbdBufTag[ubBufIdx] == BUF_FREE)
			break;
	}
	if(ubBUF_VdoUsbdBufNum == ubBufIdx)
	{
		printd(DBG_ErrorLvl, "Get USBD VDO Buf Err !\n");
	#if RTOS
		osSemaphoreRelease(tBUF_UsbdVdoAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_VdoUsbdBufTag[ubBufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_UsbdVdoAcc);
#endif
	return ulBUF_VdoUsbdBufAddr[ubBufIdx];
}

uint32_t ulBUF_GetAdoUsbdFreeBuf(void)
{
	uint8_t ubBufIdx;

#if RTOS
	osSemaphoreWait(tBUF_UsbdAdoAcc, osWaitForever);
#endif
	for(ubBufIdx = 0; ubBufIdx < ubBUF_AdoUsbdBufNum; ubBufIdx++)
	{
		if(ubBUF_AdoUsbdBufTag[ubBufIdx] == BUF_FREE)
			break;
	}
	if(ubBUF_AdoUsbdBufNum == ubBufIdx)
	{
		printd(DBG_ErrorLvl, "Get USBD ADO Buf Err !\n");
	#if RTOS
		osSemaphoreRelease(tBUF_UsbdAdoAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_AdoUsbdBufTag[ubBufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_UsbdAdoAcc);
#endif
	return ulBUF_AdoUsbdBufAddr[ubBufIdx];
}

uint8_t ubBUF_ReleaseVdoUsbdBuf(uint32_t ulBufAddr)
{
	uint8_t ubBufIdx;

#if RTOS
	osSemaphoreWait(tBUF_UsbdVdoAcc, osWaitForever);
#endif
	for(ubBufIdx = 0; ubBufIdx < ubBUF_VdoUsbdBufNum; ubBufIdx++)
	{
		if(ulBufAddr == ulBUF_VdoUsbdBufAddr[ubBufIdx])
			break;
	}
	if(ubBUF_VdoUsbdBufNum == ubBufIdx)
	{
//		printd(DBG_ErrorLvl, "Fail @ubBUF_ReleaseVdoUsbdBuf !!!\r\n");
	#if RTOS
		osSemaphoreRelease(tBUF_UsbdVdoAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_VdoUsbdBufTag[ubBufIdx] = BUF_FREE;
#if RTOS
	osSemaphoreRelease(tBUF_UsbdVdoAcc);
#endif
	return BUF_OK;
}

uint8_t ubBUF_ReleaseAdoUsbdBuf(uint32_t ulBufAddr)
{
	uint8_t ubBufIdx;

#if RTOS
	osSemaphoreWait(tBUF_UsbdAdoAcc, osWaitForever);
#endif
	for(ubBufIdx = 0; ubBufIdx < ubBUF_AdoUsbdBufNum; ubBufIdx++)
	{
		if(ulBufAddr == ulBUF_AdoUsbdBufAddr[ubBufIdx])
			break;
	}
	if(ubBUF_AdoUsbdBufNum == ubBufIdx)
	{
//		printd(DBG_ErrorLvl, "Fail @ubBUF_ReleaseAdoUsbdBuf !!!\r\n");
	#if RTOS
		osSemaphoreRelease(tBUF_UsbdAdoAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_AdoUsbdBufTag[ubBufIdx] = BUF_FREE;
#if RTOS
	osSemaphoreRelease(tBUF_UsbdAdoAcc);
#endif
	return BUF_OK;
}

void BUF_ResetUsbdBuf(void)
{
	uint8_t ubBufIdx = 0;

#if RTOS
	osSemaphoreWait(tBUF_UsbdVdoAcc, osWaitForever);
#endif
	for(ubBufIdx = 0; ubBufIdx < ubBUF_VdoUsbdBufNum; ubBufIdx++)
		ubBUF_VdoUsbdBufTag[ubBufIdx] = BUF_FREE;
#if RTOS
	osSemaphoreRelease(tBUF_UsbdVdoAcc);
#endif

#if RTOS
	osSemaphoreWait(tBUF_UsbdAdoAcc, osWaitForever);
#endif
	for(ubBufIdx = 0; ubBufIdx < BUF_NUM_USBD_ADO; ubBufIdx++)
		ubBUF_AdoUsbdBufTag[ubBufIdx] = BUF_FREE;
#if RTOS
	osSemaphoreRelease(tBUF_UsbdAdoAcc);
#endif
}

//Get IP or Function Buffer Address
uint32_t ulBUF_GetBlkBufAddr(uint8_t ubIndex,uint8_t ubBufMode)
{
	if(ubBufMode == BUF_IMG_ENC)
	{
		return ulBUF_ImgEncBufAddr[ubIndex];
	}
	else if(ubBufMode == BUF_IMG_DEC)
	{
		return ulBUF_ImgDecBufAddr[ubIndex];
	}
	else if(ubBufMode == BUF_ADO_IP)
	{
		return ulBUF_AdoIpBufAddr;
	}
	else if(ubBufMode == BUF_LCD_IP)
	{
		return ulBUF_LcdIpBufAddr;
	}
	else if(ubBufMode == BUF_BB_IP)
	{
		return ulBUF_BBIpBufAddr;
	}
	else if(ubBufMode == BUF_IMG_MERGE)
	{
		return ulBUF_ImgMergeIpBufAddr;
	}
	else if(ubBufMode == BUF_SEN_IP)
	{
		return ulBUF_SenIpBufAddr;
	}
    else if(ubBufMode == BUF_USBD_IP)
	{
		return ulBUF_UsbdIpBufAddr;
	}
    else if(ubBufMode == BUF_USBH_IP)
	{
		return ulBUF_UsbhIpBufAddr;
	}
    else if(ubBufMode == BUF_ISP_3DNR_IP)
	{
		return ulBUF_3DNRIpBufAddr;
	}	
    else if(ubBufMode == BUF_ISP_MD_W0_IP)
	{
		return ulBUF_MDw0IpBufAddr;
	}
    else if(ubBufMode == BUF_ISP_MD_W1_IP)
	{
		return ulBUF_MDw1IpBufAddr;
	}
    else if(ubBufMode == BUF_ISP_MD_W2_IP)
	{
		return ulBUF_MDw2IpBufAddr;
	}
    else if(ubBufMode == BUF_IQ_BIN_FILE)
	{
		return ulBUF_IQbinBufAddr;
	}
    else if(ubBufMode == BUF_VDO_TEMP1)
	{
		return ulBUF_VdoTemp1BufAddr;
	} 
	//FOTA
	else if(ubBufMode == BUF_FOTA)
	{
		return ulBUF_FotaBufAddr;
	} 

    else if(ubBufMode == BUF_SD)
	{
		return ulBUF_SdBufAddr;
	}  
    else if(ubBufMode == BUF_FS)
	{
		return ulBUF_FsBufAddr;
	}  
    else if(ubBufMode == BUF_REC)
	{
		return ulBUF_RecBufAddr;
	}
    else if(ubBufMode == BUF_MAC)
	{
		return ulBUF_MacBufAddr;
	}
    else if(ubBufMode == BUF_THM_SHOWING_READ_TEMP)
    {
		return ulBUF_ThmShowingRdTempBufAddr;
    }
    else if(ubBufMode == BUF_THM_SHOWING_DEC_YUV)
    {
		return ulBUF_ThmShowingDecYuvBufAddr;
    }
	else if(ubBufMode == BUF_JPG_BS)
	{
		return ulBUF_JpgBsBuffAddr[ubIndex];
	}
#if defined(BSP_DVR_SDK)
	else if(ubBufMode == BUF_JPG_BS2)
	{
		return ulBUF_JpgBs2BuffAddr;
	}
#endif
	else if(ubBufMode == BUF_JPG_RAW)
	{
		return ulBUF_JpgRawBuffAddr;
	}
	else if(ubBufMode == BUF_RESV_YUV)
	{
		return ulBUF_ResvYuvBuffAddr;
	}
	else if(ubBufMode == BUF_DEC_YUV)
	{
		return ulBUF_DecodeYuvBuffAddr;
	}	
#if defined(BSP_DVR_SDK)
	else if(ubBufMode == BUF_TEMP_YUV)
	{
		return ulBUF_TempYuvBuffAddr;
	}	
#endif
	else if(ubBufMode == BUF_RF_DRIVER)
	{
		return ulBUF_RFDriverBufAddr;
	}
	else if(ubBufMode == BUF_RW_FRAME)
	{
		return ulBUF_RWFrameBufAddr;
	}
    else if(ubBufMode == BUF_FD_RECT_OSD)
    {
        return ulBUF_FDRectOsdBufAddr;
    }
	else if(ubBufMode == BUF_IMG_DS1)
	{
		return ulBUF_IMG_DS1BufAddr;
	}
	else if(ubBufMode == BUF_USB_TRX)
	{
		return ulBUF_UsbTrx_BufAddr;
	}
    else if(ubBufMode == BUF_SEN_1_YUV)
    {
        return ulBUF_Sen1YuvBufAddr[ubIndex];
    }
	printd(DBG_ErrorLvl, "Err @ulBUF_GetBufAddr\r\n");
	return 0;	//Error Case
}

uint32_t ulBUF_AlignAddrTo1K(uint32_t ulAddr)
{
	if((ulAddr%1024) == 0)
	{
		return ulAddr;
	}
	else
	{
		return ((ulAddr/1024)*1024)+1024;
	}
}

uint32_t ulBUF_AlignAddrTo512B(uint32_t ulAddr)
{
	if((ulAddr%512) == 0)
	{
		return ulAddr;
	}
	else
	{
		return ((ulAddr/512)*512)+512;
	}
}
