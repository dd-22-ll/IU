/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		GKEY.c
	\brief		Gpio KEY Scan Function
	\author		Penghui Xue
	\version	1.0
	\date		2019/07/08
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "GKEY.h" 

#if GKEY_ENABLE
GKEY_ID_t tGKEY_Map[GKEY_MAX_NUM] =
{
	GKEY_ID0,
	GKEY_ID1,
	GKEY_ID2,
	GKEY_ID3,
	GKEY_ID4,
	GKEY_ID5,
	GKEY_ID6,
	GKEY_ID7,
	GKEY_ID8,
	GKEY_ID9,
	GKEY_ID10,
	GKEY_ID11,
	GKEY_ID12,
	GKEY_ID13,
	GKEY_ID14,
	GKEY_ID15,
	GKEY_ID16,
	GKEY_ID17,
	GKEY_ID18,
	GKEY_ID19,
	GKEY_ID20
};
KEY_Event_t tGKEY_Event;
GKEY_SCAN_t tGKEY_Scan;
uint32_t ulGKEY_DetEnFlag = 0;
//------------------------------------------------------------------------------
void GKEY_Init(void)
{
	uint8_t i;
	memset(&tGKEY_Event, 0, sizeof(KEY_Event_t));
	memset(&tGKEY_Scan, 0, sizeof(GKEY_SCAN_t));
	for(i=0;i<GKEY_MAX_NUM-1;i++)
	{	
		tGKEY_Scan.ubGKEY_KeyScanState[i]  = KEY_DET_STATE;
		tGKEY_Scan.uwGKEY_KeyScanTime[i]   = GKEY_DET_TIME;
	}
}
//------------------------------------------------------------------------------
void GKEY_Thread(void)
{
	uint8_t i;
	uint32_t ulGKEY_DetID;
	static uint16_t uwGKEY_ThreadCnt[GKEY_MAX_NUM] = {0};

	if(!ulGKEY_DetEnFlag)
		return;

	ulGKEY_DetID = (GPIO->GPIO_I & ulGKEY_DetEnFlag);
	
	for(i=0;i<GKEY_MAX_NUM-1;i++)
	{
		if(ulGKEY_DetEnFlag & (1 << i))
		{
			if((uwGKEY_ThreadCnt[i] % tGKEY_Scan.uwGKEY_KeyScanTime[i]) == 0)
			{
				uwGKEY_ThreadCnt[i] = 0;
				switch(tGKEY_Scan.ubGKEY_KeyScanState[i])
				{
					case KEY_DET_STATE:
						if((ulGKEY_DetID & (1 << i)) != 0)
						{
							tGKEY_Scan.ubGKEY_KeyScanState[i] 	= KEY_DEBONC_STATE;
							tGKEY_Scan.uwGKEY_KeyScanTime[i]  	= GKEY_DEB_TIME;
							tGKEY_Scan.uwGKEY_KeyScanCnt[i] 	= 0;
						}
						else
						{
							tGKEY_Scan.uwGKEY_KeyScanTime[i]  = GKEY_DET_TIME;
						}
						break;
					case KEY_DEBONC_STATE:
						if((ulGKEY_DetID & (1 << i)) != 0)
						{
							tGKEY_Scan.ubGKEY_KeyScanState[i] = KEY_CNT_STATE;
							tGKEY_Scan.uwGKEY_KeyScanTime[i]  = GKEY_CNT_TIME;
							tGKEY_Event.ubKeyAction = KEY_DOWN_ACT;
							tGKEY_Event.ubKeyID	  	= tGKEY_Map[i];
							tGKEY_Event.uwKeyCnt  	= 0;
							KEY_QueueSend(GKEY, &tGKEY_Event);
						}
						else
						{
							tGKEY_Scan.ubGKEY_KeyScanState[i] = KEY_DET_STATE;
							tGKEY_Scan.uwGKEY_KeyScanTime[i]  = GKEY_DET_TIME;
						}
						break;
					case KEY_CNT_STATE:
						if((ulGKEY_DetID & (1 << i)) != 0)
						{
							tGKEY_Scan.uwGKEY_KeyScanCnt[i] = (tGKEY_Scan.uwGKEY_KeyScanCnt[i] < 255)?
														   (++tGKEY_Scan.uwGKEY_KeyScanCnt[i]):tGKEY_Scan.uwGKEY_KeyScanCnt[i];
							tGKEY_Event.ubKeyAction = KEY_CNT_ACT;
							tGKEY_Event.ubKeyID	  	= tGKEY_Map[i];
							tGKEY_Event.uwKeyCnt  	= tGKEY_Scan.uwGKEY_KeyScanCnt[i];
							KEY_QueueSend(GKEY, &tGKEY_Event);
						}
						else
						{
							tGKEY_Scan.uwGKEY_KeyScanCnt[i]   = 0;
							tGKEY_Scan.ubGKEY_KeyScanState[i] = KEY_DET_STATE;
							tGKEY_Scan.uwGKEY_KeyScanTime[i]  = GKEY_DET_TIME;
							tGKEY_Event.ubKeyAction = KEY_UP_ACT;
							tGKEY_Event.ubKeyID	  	= tGKEY_Map[i];
							tGKEY_Event.uwKeyCnt  	= 0;
							KEY_QueueSend(GKEY, &tGKEY_Event);
						}
						break;
				}
			}
			uwGKEY_ThreadCnt[i]++;
		}		
	}
}	
//------------------------------------------------------------------------------
void GKEY_SetDetPin(uint8_t ubGpioNum)
{
	if(ubGpioNum < GKEY_MAX_NUM)
		ulGKEY_DetEnFlag |= 1<<ubGpioNum;
}
#endif
