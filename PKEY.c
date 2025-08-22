/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PKEY.c
	\brief		Power KEY Function
	\author		Hanyi Chiu
	\version	1.1
	\date		2017/11/28
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "PKEY.h"
#include "RTC_API.h"

#if PKEY_ENABLE
//----------------------------------------------------------
KEY_Event_t tPKEY_Event;
PKEY_SCAN_t tPKEY_Scan;
//----------------------------------------------------------
void PKEY_Init(void)
{
	memset(&tPKEY_Scan, 0, sizeof(PKEY_SCAN_t));
	tPKEY_Scan.ubPKEY_KeyScanState = KEY_UNKNOW_STATE;
	tPKEY_Scan.uwPKEY_KeyScanTime = PKEY_DET_TIME;
}
//----------------------------------------------------------
void PKEY_Thread(void)
{
	static uint16_t uwPKEY_ThreadCnt = 0;

	if(uwPKEY_ThreadCnt % tPKEY_Scan.uwPKEY_KeyScanTime == 0)
	{
		uwPKEY_ThreadCnt = 0;
		switch(tPKEY_Scan.ubPKEY_KeyScanState)
		{
			case KEY_UNKNOW_STATE:
				if(ubRTC_GetKey() == 0)
					tPKEY_Scan.ubPKEY_KeyScanState = KEY_DET_STATE;
				tPKEY_Scan.uwPKEY_KeyScanTime = PKEY_DET_TIME;
				break;
			case KEY_DET_STATE:
				if(ubRTC_GetKey() == 1)
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_DEBONC_STATE;
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_DEB_TIME;
				}
				else
				{
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_DET_TIME;
				}
				break;
			case KEY_DEBONC_STATE:
				if(ubRTC_GetKey() == 1)
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_CNT_STATE;
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_CNT_TIME;
					tPKEY_Event.ubKeyAction = KEY_DOWN_ACT;
					tPKEY_Event.ubKeyID	  	= PKEY_ID0;
					tPKEY_Event.uwKeyCnt  	= 0;
					KEY_QueueSend(PKEY, &tPKEY_Event);
				}
				else
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_DET_STATE;
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_DET_TIME;
				}
				break;
			case KEY_CNT_STATE:
				if(ubRTC_GetKey() == 1)
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_CNT_STATE; 
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_CNT_TIME;
					tPKEY_Event.ubKeyAction = KEY_CNT_ACT;
					tPKEY_Event.ubKeyID	  	= PKEY_ID0;
					tPKEY_Event.uwKeyCnt  	= ++tPKEY_Scan.uwPKEY_KeyScanCnt;
					KEY_QueueSend(PKEY, &tPKEY_Event);
				}
				else
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_DET_STATE;
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_DET_TIME;
					tPKEY_Event.ubKeyAction = KEY_UP_ACT;
					tPKEY_Event.ubKeyID	  	= PKEY_ID0;
					tPKEY_Event.uwKeyCnt  	= tPKEY_Scan.uwPKEY_KeyScanCnt;
					KEY_QueueSend(PKEY, &tPKEY_Event);
					tPKEY_Scan.uwPKEY_KeyScanCnt = 0;
				}
				break;
			default:
				break;
		}
	}
	uwPKEY_ThreadCnt++;
}
//----------------------------------------------------------
uint8_t ubPKEY_GetKey(void)
{
	return (wRTC_ReadSysRam(3) & 0x1);
}
#endif
