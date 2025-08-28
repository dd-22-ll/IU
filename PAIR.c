/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PAIR.c
	\brief		Pairing Function
	\author		Bing
	\version	2020/05/04
	\date		0.20
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include "APP_CFG.h"
#include "PAIR.h"
#include "BB_API.h"
#include "TWC_API.h"
#include "SF_API.h"
#include "KNL.h"

#ifdef RTC676x
#include "rwrf.h"
#include "RTC676x_CTRL.h"
#endif
#define PAIR_MAJORVER	0
#define PAIR_MINORVER	20



#ifdef RTC676x
uint8_t ubPAIR_RdyToPairFlg = 0;
#endif

uint8_t ubPAIR_InPairIngFlg = 0;

uint32_t        ulPAIR_Timeout, ulPAIR_TimeCnt;
uint8_t         ubPAIR_State, ubPAIR_Number;
osThreadId      PAIR_ThreadId;
osMessageQId    *xAppEventQueue;
PAIR_RRP_Hdr    PAIR_PrpPket;
PAIR_RAP_Hdr    PAIR_PapPket;

#if( defined(T2R) && defined(A7130))
#if OP_AP
PAIR_ID_TABLE   PAIR_IdTable;
PAIR_Info_t 	tPAIR_Info;
#else
PAIR_ID_TABLE   PAIR_IdTable;
PAIR_Info_t 	tPAIR_Info[4];
uint8_t ubPAIR_NowGroup;
uint8_t ubPAIR_NewGroup;
#endif
#else
PAIR_ID_TABLE   PAIR_IdTable;
PAIR_Info_t 	tPAIR_Info;
#endif

#if( (defined(T2R)) && defined(A7130) && (defined(OP_AP)))
uint32_t ulPAIR_BufStaSign[4];
#endif


PAIR_TAG		tPAIR_DelStaNum;
static uint32_t ulPAIR_SFAddr;
const uint8_t ubFixChTable_20[3] =
{
	20, 83, 154
};
const uint8_t ubFixChTable_19[3] =
{
	20, 83, 146
};
const uint8_t ubTestChTable_20[3] = 		
{
	20, 83, 154
};

const uint8_t ubTestChTable_19[3] = 		
{
	20, 83, 146
};

uint8_t ubPAIR_GetInPairIngFlg(void)
{
	return ubPAIR_InPairIngFlg;
}

#ifdef RTC676x
void PAIR_SetRdyToPairFlg(uint8_t ubRdyFlg)
{
	ubPAIR_RdyToPairFlg = ubRdyFlg;
}

uint8_t ubPAIR_GetRdyToPairFlg(void)
{
	return ubPAIR_RdyToPairFlg;
}

//void event_cb(int event,void *arg)
void PAIR_EventCb(int event,void *arg)
{
	if(event == PAIRING_EVENT_READY)
	{
		printf("Ready to Pairing\r\n");
		PAIR_SetRdyToPairFlg(1);
	}
}

//void readable_cb(void *arg)
void PAIR_ReadableCb(void *arg)
{
	while (1) {
		uint8_t buf[64] = {0};
		if (rf_recv_pairing_data(buf, sizeof(buf)) < 0)
			break;

		int *opc = (int *)buf;
		uint8_t *data = buf + 4;

	#if (OP_STA||OP_AP_SLAVE)
		if (*opc == TWC_PAP) {
			PAIR_Pap((TWC_TAG)0, data);
		}
	#endif

	#if OP_AP
		if (*opc == TWC_PRP) {
			PAIR_Prp((TWC_TAG)0, data);
		}

		if (*opc == TWC_PAAP) {
			PAIR_Paap((TWC_TAG)0, data);
		}
	#endif
	}
}
#endif
//------------------------------------------------------------------------------
uint8_t ubPAIR_EndFlag = FALSE;
static void PAIR_Thread(void const *argument)
{
#if (defined(A7130) || defined(S2019A))
	TWC_STATUS tTwcSts;
#endif
    PAIR_EventMsg_t tPair_EvtMsg = {0};
    uint16_t uwDelayMs = 100;
	
	while(1)
	{
		if(ubPAIR_State == PAIR_NULL)
		{
			ubPAIR_EndFlag = FALSE;
			osThreadSuspend(PAIR_ThreadId);
		}
#if (OP_STA || OP_AP_SLAVE)
		if(ubPAIR_State == PAIR_START)
		{
			PAIR_PreparePrp();
			ulPAIR_TimeCnt = 0;	
			ubPAIR_State = PAIR_PRP;
			uwDelayMs = PAIR_PRP_DELAY;
		}
		else if(ubPAIR_State == PAIR_PRP)
		{
		#if (defined(A7130) || defined(S2019A))
			tTwcSts = tTWC_Send(TWC_AP_MASTER, TWC_PRP, (uint8_t *)&PAIR_PrpPket, sizeof(PAIR_PrpPket), 8);
			printf("Sent TWC_PRP_%s \n", (TWC_SUCCESS == tTwcSts)?"ok":"fail");
		#endif
		#ifdef RTC676x
			ubKNL_AccessLinkActiveTime(KNL_OPERATION_SET,KNL_LINK_ACTIVE_TIME);
			if(ubPAIR_GetRdyToPairFlg())
			{
				uint8_t buf[32];
				int *opc = (int *)buf;
				*opc = TWC_PRP;
				memcpy(buf + 4, (uint8_t *)&PAIR_PrpPket, sizeof(PAIR_PrpPket));
				int result = rf_send_pairing_data(buf, sizeof(buf));

				printf("Sent PRP %s\n", (result < 0) ? "Fail" : "OK");
			}
		#endif
			uwDelayMs = PAIR_PRP_DELAY;
		}
		else if(ubPAIR_State == PAIR_PAAP)
		{
		#if (defined(A7130) || defined(S2019A))
			tTwcSts = tTWC_Send(TWC_AP_MASTER, TWC_PAAP, (uint8_t *)&PAIR_PapPket, sizeof(PAIR_PapPket), 8);
			printf("Sent TWC_PAAP_%s\n", (TWC_SUCCESS == tTwcSts)?"ok":"fail");
		#endif
		#ifdef RTC676x
			ubKNL_AccessLinkActiveTime(KNL_OPERATION_SET,KNL_LINK_ACTIVE_TIME);
			if(ubPAIR_GetRdyToPairFlg())
			{
				uint8_t buf[32];
				int *opc = (int *)buf;
				*opc = TWC_PAAP;
				memcpy(buf + 4, (uint8_t *)&PAIR_PapPket, sizeof(PAIR_PapPket));
				int result = rf_send_pairing_data(buf, sizeof(buf));

				printf("Sent PAAP %s\n", (result < 0) ? "Fail" : "OK");
			}
		#endif
			uwDelayMs = PAIR_PAAP_DELAY;
			if(FALSE == ubPAIR_EndFlag)
			{
				ulPAIR_TimeCnt = 0;
				ulPAIR_Timeout = 3000;
				ubPAIR_EndFlag = TRUE;
			}
		#ifdef RTC676x
			// Because there will be no ack or response when sending pairing packet
			// We set state to PAIR_END after a while
			if (ulPAIR_TimeCnt > 200)
				ubPAIR_State = PAIR_END;
		#endif
		}
		else if(ubPAIR_State == PAIR_END)
		{
			tTWC_StopTwcSend(TWC_AP_MASTER, TWC_PRP);
			tTWC_StopTwcSend(TWC_AP_MASTER, TWC_PAAP);
		#ifdef S2019A
			PAIR_SaveId();
			sPRF_SetDevId((sPRF_DevId_t)tPAIR_Info.tPAIR_Table.ubTxNumber);
			sPRF_UpdateIdInfo(sPRF_AP, &tPAIR_Info.tPAIR_Table.ubAp_ID[0]);
			sPRF_StopPairing();
			tPair_EvtMsg.ubPAIR_Event 		= APP_PAIRING_SUCCESS_EVENT;
			tPair_EvtMsg.ubPAIR_Message[0] 	= PAIR_GetStaNumber();
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 0);
		#endif

		#ifdef A7130
			BB_HoppingPairingEnd();
			PAIR_SaveId();
			tPair_EvtMsg.ubPAIR_Event 		= APP_PAIRING_SUCCESS_EVENT;
			tPair_EvtMsg.ubPAIR_Message[0] 	= PAIR_GetStaNumber();
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 0);
		#endif	
			
		#ifdef RTC676x			

			int result;
			//RTC676x_SetPower(1);	//High Power
			
			result = rf_stop_pairing();
			printf("rf_stop_pairing result = %d\n", result);
			PAIR_SetRdyToPairFlg(0);
			
			update_id();
			
			PAIR_SaveId();		
			
		#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE				
			PAIR_LoadId();
		#if FREQ_SEL_DBG_EN
			printf("After-Pairing Select-Ch @Sf:0x%x\r\n",PAIR_IdTable.ubSelectCh);
		#endif
			KNL_SetFreqTable(PAIR_IdTable.ubSelectCh);			
			KNL_SetFreqTableInSf(PAIR_IdTable.ubSelectCh);			
		#endif	
			
			tPair_EvtMsg.ubPAIR_Event 		= APP_PAIRING_SUCCESS_EVENT;
			tPair_EvtMsg.ubPAIR_Message[0] 	= PAIR_GetStaNumber();
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg,0);			
		#endif
			printf("PAIR_END\n");
			ulPAIR_TimeCnt = 0;
			ubPAIR_EndFlag = FALSE;
			ubPAIR_InPairIngFlg = 0;
			osThreadSuspend(PAIR_ThreadId);
		}
		ulPAIR_TimeCnt += uwDelayMs;
		if(ulPAIR_TimeCnt > ulPAIR_Timeout)
		{
			tTWC_StopTwcSend(TWC_AP_MASTER, TWC_PRP);
			tTWC_StopTwcSend(TWC_AP_MASTER, TWC_PAAP);
		#ifdef S2019A
//			PAIR_LoadId();
			sPRF_StopPairing();
			ubPAIR_State = PAIR_TIMEOUT;
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_FAIL_EVENT;
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 0);
		#endif

		#ifdef A7130
			BB_HoppingPairingEnd();
			PAIR_LoadId();
			ubPAIR_State = PAIR_TIMEOUT;
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_FAIL_EVENT;
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 0);
		#endif

		#ifdef RTC676x			
			int result;
			
			//RTC676x_SetPower(1);	//High Power
			
			result = rf_stop_pairing();
			printf("rf_stop_pairing result = %d\n", result);
			PAIR_SetRdyToPairFlg(0);
			PAIR_LoadId();
			
		#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE	
		#if FREQ_SEL_DBG_EN
			printf("After-Pairing Select-Ch @Sf:0x%x\r\n",ubKNL_GetFreqTableInSf());
		#endif
			KNL_SetFreqTable(ubKNL_GetFreqTableInSf());			
		#endif	
			
			ubPAIR_State = PAIR_TIMEOUT;
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_FAIL_EVENT;
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 5000);
		#endif
			
			ubPAIR_InPairIngFlg = 0;
			ubPAIR_EndFlag = FALSE;
			printf("PAIR_TIMEOUT\n");
			osThreadSuspend(PAIR_ThreadId);
		}
		osDelay(uwDelayMs);
#endif
#if OP_AP
		if(ubPAIR_State == PAIR_START)
		{
			printf("Wait PRP\n");
			uwDelayMs = PAIR_START_DELAY;
		}
		else if(ubPAIR_State == PAIR_PAP)
		{
		#if (defined(A7130) || defined(S2019A))
			tTwcSts = tTWC_Send(TWC_STA1, TWC_PAP, (uint8_t *)&PAIR_PapPket, sizeof(PAIR_PapPket), 8);
			printf("Sent PAIR_PAP_%s\n", (TWC_SUCCESS == tTwcSts)?"ok":"fail");
		#endif
		#ifdef RTC676x
			if(ubPAIR_GetRdyToPairFlg())
			{
				uint8_t buf[32];
				int *opc = (int *)buf;
				*opc = TWC_PAP;
				memcpy(buf + 4, (uint8_t *)&PAIR_PapPket, sizeof(PAIR_PapPket));
				int result = rf_send_pairing_data(buf, sizeof(buf));

				printf("Sent PAP %s\n", (result < 0) ? "Fail" : "OK");
			}
		#endif
			uwDelayMs = PAIR_PAP_DELAY;
			if(FALSE == ubPAIR_EndFlag)
			{
				ulPAIR_TimeCnt = 0;
				ulPAIR_Timeout = 3000;
				ubPAIR_EndFlag = TRUE;
			}			
		}
		else if(ubPAIR_State == PAIR_END)
		{
			tTWC_StopTwcSend(TWC_STA1, TWC_PRP);
		#ifdef S2019A
			PAIR_SaveId();
			sPRF_UpdateIdInfo((sPRF_DevId_t)ubPAIR_Number, &tPAIR_Info.tPAIR_Table.ubSTA_ID[ubPAIR_Number][0]);
			sPRF_StopPairing();
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_SUCCESS_EVENT;
			if(tPAIR_DelStaNum <= PAIR_STA4)
			{
				tPair_EvtMsg.ubPAIR_Message[0] = 1;		//! Message Length
				tPair_EvtMsg.ubPAIR_Message[1] = tPAIR_DelStaNum;
				tPair_EvtMsg.ubPAIR_Message[2] = APP_UNBIND_CAM_EVENT;
			}
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 0);
			printf("PAIR_END\n");
			ulPAIR_TimeCnt = 0;
			ubPAIR_EndFlag = FALSE;
			tPair_EvtMsg.ubPAIR_Message[2] = APP_REFRESH_EVENT;
			ubPAIR_State = PAIR_SUCCESS;
		#endif

		#ifdef A7130
			BB_HoppingPairingEnd();
			PAIR_SaveId();
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_SUCCESS_EVENT;
			if(tPAIR_DelStaNum <= PAIR_STA4)
			{
				tPair_EvtMsg.ubPAIR_Message[0] = 1;		//! Message Length
				tPair_EvtMsg.ubPAIR_Message[1] = tPAIR_DelStaNum;
				tPair_EvtMsg.ubPAIR_Message[2] = APP_UNBIND_CAM_EVENT;
			}
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 0);
			printf("PAIR_END\n");
			ulPAIR_TimeCnt = 0;
			ubPAIR_EndFlag = FALSE;
			tPair_EvtMsg.ubPAIR_Message[2] = APP_REFRESH_EVENT;
			ubPAIR_State = PAIR_SUCCESS;
		#endif
		
		#ifdef RTC676x			
			int result;
			
			//RTC676x_SetPower(1);	//High Power
			
			result = rf_stop_pairing();
			if(result != 0)
				printf("rf_stop_pairing result = %d\n", result);
			
			update_id();
			
			PAIR_SaveId();			
		
		#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE	
			PAIR_LoadId();
		#if FREQ_SEL_DBG_EN	
			printf("After-Pairing SelectCh @Sf:0x%x\r\n",PAIR_IdTable.ubSelectCh);
		#endif
			KNL_SetFreqTable(PAIR_IdTable.ubSelectCh);			
			KNL_SetFreqTableInSf(PAIR_IdTable.ubSelectCh);			
		#endif	
			
			PAIR_SetRdyToPairFlg(0);	
			
			
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_SUCCESS_EVENT;
			if(tPAIR_DelStaNum <= PAIR_STA4)
			{
				tPair_EvtMsg.ubPAIR_Message[0] = 1;		//! Message Length
				tPair_EvtMsg.ubPAIR_Message[1] = tPAIR_DelStaNum;
				tPair_EvtMsg.ubPAIR_Message[2] = APP_UNBIND_CAM_EVENT;
			}
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg,0);			
			
			printf("PAIR_END\n");
			ulPAIR_TimeCnt = 0;
			ubPAIR_EndFlag = FALSE;
			tPair_EvtMsg.ubPAIR_Message[2] = APP_REFRESH_EVENT;
			ubPAIR_State = PAIR_SUCCESS;//Justin 2019.01.30
		#endif
			osThreadSuspend(PAIR_ThreadId);
		}
		ulPAIR_TimeCnt += uwDelayMs;
		if(ulPAIR_TimeCnt > ulPAIR_Timeout)
		{
			tTWC_StopTwcSend(TWC_STA1, TWC_PRP);
		#ifdef S2019A
//			PAIR_LoadId();
			sPRF_StopPairing();
			ubPAIR_State = PAIR_TIMEOUT;
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_FAIL_EVENT;
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 0);
		#endif

		#ifdef A7130
			BB_HoppingPairingEnd();			
			PAIR_LoadId();
			ubPAIR_State = PAIR_TIMEOUT;
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_FAIL_EVENT;
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 0);
		#endif
		
		#ifdef RTC676x
			int result;
			
			//RTC676x_SetPower(1);	//High Power
			
			result = rf_stop_pairing();
			printf("rf_stop_pairing result = %d\n", result);
			
			PAIR_LoadId();
			
		#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE
		#if FREQ_SEL_DBG_EN		
			printf("After-Pairing SelectCh @Sf:0x%x\r\n",PAIR_IdTable.ubSelectCh);
		#endif
			KNL_SetFreqTable(PAIR_IdTable.ubSelectCh);			
			KNL_SetFreqTableInSf(PAIR_IdTable.ubSelectCh);			
		#endif	
		
			PAIR_SetRdyToPairFlg(0);

			ubPAIR_State = PAIR_TIMEOUT;
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_FAIL_EVENT;
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg,0);
		#endif
			ubPAIR_EndFlag = FALSE;
			osThreadSuspend(PAIR_ThreadId);
		}
		osDelay(uwDelayMs);
#endif
	}
}
//------------------------------------------------------------------------------
void PAIR_Init(osMessageQId *pvMsgQId)
{
    xAppEventQueue = pvMsgQId;
	ulPAIR_SFAddr = pSF_Info->ulSize - (PAIR_SF_START_SECTOR * pSF_Info->ulSecSize);
#if (OP_STA||OP_AP_SLAVE)
	if(tTWC_RegTransCbFunc(TWC_PAP, NULL, PAIR_Pap) == TWC_FAIL)
		printf("Register Pair PAP TWC Fail !\n");
	if(tTWC_RegTransCbFunc(TWC_PAAP, PAIR_PaapResp, NULL) == TWC_FAIL)
		printf("Register Pair PAAP TWC Fail !\n");
#endif
#if OP_AP
	if(tTWC_RegTransCbFunc(TWC_PRP, NULL, PAIR_Prp) == TWC_FAIL)
		printf("Register Pair PRP TWC Fail !\n");
    if(tTWC_RegTransCbFunc(TWC_PAAP, NULL, PAIR_Paap) == TWC_FAIL)
		printf("Register Pair PAAP TWC Fail !\n");
	tPAIR_DelStaNum = PAIR_AP_ASSIGN;
#endif
	
#if( defined(T2R) && defined(A7130) && defined(OP_STA))
	ubPAIR_NowGroup = 0;
	ubPAIR_NewGroup = 0;
#endif	
    ubPAIR_State  = PAIR_NULL;
	PAIR_LoadId();
	osThreadDef(PAIR_Thread, PAIR_Thread, THREAD_PRIO_PAIRING_HANDLER, 1, THREAD_STACK_PAIRING_HANDLER);
    PAIR_ThreadId = osThreadCreate(osThread(PAIR_Thread), NULL);
}    
//------------------------------------------------------------------------------
void PAIR_LoadId(void)
{
#ifdef S2019A
	sPRF_DevId_t tDevId;
#endif
	
#if( defined(T2R) && defined(A7130))
#if OP_AP	
	SF_Read(ulPAIR_SFAddr, sizeof(PAIR_Info_t), (uint8_t *)&tPAIR_Info);
	if((tPAIR_Info.tPAIR_Table.ulAp_ID == 0) || (tPAIR_Info.tPAIR_Table.ulAp_ID == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulAp_ID = PAIR_NO_PAIR_AP_ID;
	}
	if((tPAIR_Info.tPAIR_Table.ulSTA_ID[0] == 0) || (tPAIR_Info.tPAIR_Table.ulSTA_ID[0] == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulSTA_ID[0] = PAIR_NO_PAIR_STA1_ID;
	}
	if((tPAIR_Info.tPAIR_Table.ulSTA_ID[1] == 0) || (tPAIR_Info.tPAIR_Table.ulSTA_ID[1] == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulSTA_ID[1] = PAIR_NO_PAIR_STA2_ID;
	}
	if((tPAIR_Info.tPAIR_Table.ulSTA_ID[2] == 0) || (tPAIR_Info.tPAIR_Table.ulSTA_ID[2] == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulSTA_ID[2] = PAIR_NO_PAIR_STA3_ID;
	}
	if((tPAIR_Info.tPAIR_Table.ulSTA_ID[3] == 0) || (tPAIR_Info.tPAIR_Table.ulSTA_ID[3] == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulSTA_ID[3] = PAIR_NO_PAIR_STA4_ID;
	}
	memcpy(&PAIR_IdTable, &tPAIR_Info.tPAIR_Table, sizeof(PAIR_ID_TABLE));
#else
	SF_Read(ulPAIR_SFAddr, (sizeof(PAIR_Info_t)*4), (uint8_t *)&tPAIR_Info);
	if((tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulAp_ID == 0) || (tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulAp_ID == 0xFFFFFFFF))
	{
		tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulAp_ID = PAIR_NO_PAIR_AP_ID;
	}
	if((tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[0] == 0) || (tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[0] == 0xFFFFFFFF))
	{
		tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[0] = PAIR_NO_PAIR_STA1_ID;
	}
	if((tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[1] == 0) || (tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[1] == 0xFFFFFFFF))
	{
		tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[1] = PAIR_NO_PAIR_STA2_ID;
	}
	if((tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[2] == 0) || (tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[2] == 0xFFFFFFFF))
	{
		tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[2] = PAIR_NO_PAIR_STA3_ID;
	}
	if((tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[3] == 0) || (tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[3] == 0xFFFFFFFF))
	{
		tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table.ulSTA_ID[3] = PAIR_NO_PAIR_STA4_ID;
	}
	memcpy(&PAIR_IdTable, &tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table, sizeof(PAIR_ID_TABLE));
#endif	
#else
	SF_Read(ulPAIR_SFAddr, sizeof(PAIR_Info_t), (uint8_t *)&tPAIR_Info);
		
	printf(">>  ulAp_ID = 0x%x \n", tPAIR_Info.tPAIR_Table.ulAp_ID);	
	
	if((tPAIR_Info.tPAIR_Table.ulAp_ID == 0) || (tPAIR_Info.tPAIR_Table.ulAp_ID == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulAp_ID = PAIR_NO_PAIR_AP_ID;
	}
	if((tPAIR_Info.tPAIR_Table.ulSTA_ID[0] == 0) || (tPAIR_Info.tPAIR_Table.ulSTA_ID[0] == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulSTA_ID[0] = PAIR_NO_PAIR_STA1_ID;
	}
	if((tPAIR_Info.tPAIR_Table.ulSTA_ID[1] == 0) || (tPAIR_Info.tPAIR_Table.ulSTA_ID[1] == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulSTA_ID[1] = PAIR_NO_PAIR_STA2_ID;
	}
	if((tPAIR_Info.tPAIR_Table.ulSTA_ID[2] == 0) || (tPAIR_Info.tPAIR_Table.ulSTA_ID[2] == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulSTA_ID[2] = PAIR_NO_PAIR_STA3_ID;
	}
	if((tPAIR_Info.tPAIR_Table.ulSTA_ID[3] == 0) || (tPAIR_Info.tPAIR_Table.ulSTA_ID[3] == 0xFFFFFFFF))
	{
		tPAIR_Info.tPAIR_Table.ulSTA_ID[3] = PAIR_NO_PAIR_STA4_ID;
	}
	
	printf(">>  ulAp_ID = 0x%x \n", tPAIR_Info.tPAIR_Table.ulAp_ID);
	memcpy(&PAIR_IdTable, &tPAIR_Info.tPAIR_Table, sizeof(PAIR_ID_TABLE));	
#endif	
	
	
#ifdef S2019A
	#ifdef OP_AP
	sPRF_SetDevId(sPRF_AP);
	for(tDevId = sPRF_STA1; tDevId <= sPRF_STA4; tDevId++)	
		sPRF_UpdateIdInfo(tDevId, &PAIR_IdTable.ubSTA_ID[tDevId][0]);
	#else
	tDevId = (sPRF_DevId_t)PAIR_IdTable.ubTxNumber;
	sPRF_SetDevId(tDevId);
	sPRF_UpdateIdInfo(sPRF_AP, &PAIR_IdTable.ubAp_ID[0]);
	#endif
#else
	PAIR_CheckIdTable();
	#ifdef RTC676x
//	printf("ID Information\r\n");
//	printf("==========================\r\n");	
//	printf("DeviceNum:0x%x\r\n",PAIR_IdTable.ubTxNumber);
//	printf("Master-ID:0x%x\r\n",PAIR_IdTable.ulAp_ID);
//	printf("Slave1-ID:0x%x\r\n",PAIR_IdTable.ulSTA_ID[0]);
//	printf("Slave2-ID:0x%x\r\n",PAIR_IdTable.ulSTA_ID[1]);
//	printf("Slave3-ID:0x%x\r\n",PAIR_IdTable.ulSTA_ID[2]);
//	printf("Slave4-ID:0x%x\r\n",PAIR_IdTable.ulSTA_ID[3]);
	
	KNL_SetMasterId(PAIR_IdTable.ulAp_ID);
	KNL_SetSlaveId(0,PAIR_IdTable.ulSTA_ID[0]);
	KNL_SetSlaveId(1,PAIR_IdTable.ulSTA_ID[1]);
	KNL_SetSlaveId(2,PAIR_IdTable.ulSTA_ID[2]);
	KNL_SetSlaveId(3,PAIR_IdTable.ulSTA_ID[3]);	
	
		#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE	
			#if FREQ_SEL_DBG_EN
				printf("Select-Ch @Sf:0x%x @%s\r\n",PAIR_IdTable.ubSelectCh,__func__);
			#endif
			KNL_SetFreqTableInSf(PAIR_IdTable.ubSelectCh);
		#endif
	
	#endif
#endif
}
//------------------------------------------------------------------------------
void PAIR_SaveId(void)
{
    SF_DisableWrProtect();
#if( defined(T2R) && defined(A7130))
	SF_Erase(SF_SE, ulPAIR_SFAddr, (sizeof(PAIR_Info_t)*4), 1);	
#else
	SF_Erase(SF_SE, ulPAIR_SFAddr, sizeof(PAIR_Info_t), 1);	
#endif	
    #if OP_AP
    PAIR_IdTable.ubTxNumber = 0x0F;
    #endif
	
#if( defined(T2R) && defined(A7130) && defined(OP_STA))
	memcpy(&tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table, &PAIR_IdTable, sizeof(PAIR_ID_TABLE));
    SF_Write(ulPAIR_SFAddr, (sizeof(PAIR_Info_t)*4), (uint8_t *)&tPAIR_Info);
#else
	memcpy(&tPAIR_Info.tPAIR_Table, &PAIR_IdTable, sizeof(PAIR_ID_TABLE));
    SF_Write(ulPAIR_SFAddr, sizeof(PAIR_Info_t), (uint8_t *)&tPAIR_Info);
#endif
	
	SF_EnableWrProtect();
}
//------------------------------------------------------------------------------
void PAIR_DeleteTxId(PAIR_TAG tStaNum)
{
#ifdef S2019A
	sPRF_DevId_t tDevId[] = {
								[PAIR_STA1] = sPRF_STA1,
								[PAIR_STA2] = sPRF_STA2,
								[PAIR_STA3] = sPRF_STA3,
								[PAIR_STA4] = sPRF_STA4,
							};

	if(tStaNum > PAIR_STA4)
		return;
	PAIR_SetDevInvaildId(tStaNum);
	PAIR_SaveId();
	sPRF_DelTxDevId(tDevId[tStaNum]);
#else
	PAIR_SetDevInvaildId(tStaNum);
	PAIR_SaveId();
	#ifdef RTC676x
	RTC676x_DelId(tStaNum);	//! update_id();
	#endif
#endif
}
//------------------------------------------------------------------------------
void PAIR_SetDevInvaildId(PAIR_TAG tTag)
{
	switch(tTag)
	{
#ifdef S2019A
		case PAIR_AP_ASSIGN:
			PAIR_IdTable.ubAp_ID[0] = 0xFE;
			PAIR_IdTable.ubAp_ID[1] = 0xFD;
			PAIR_IdTable.ubAp_ID[2] = 0xFC;
			PAIR_IdTable.ubAp_ID[3] = 0xFB;
			PAIR_IdTable.ubAp_ID[4] = 0xFA;
			PAIR_IdTable.ubAp_ID[5] = 0xF9;
			break;
		default:
			if(tTag <= PAIR_STA4)
			{
				PAIR_IdTable.ubSTA_ID[tTag][0] = 0xFE;
				PAIR_IdTable.ubSTA_ID[tTag][1] = 0xFD;
				PAIR_IdTable.ubSTA_ID[tTag][2] = 0xFC;
				PAIR_IdTable.ubSTA_ID[tTag][3] = 0xFB;
				PAIR_IdTable.ubSTA_ID[tTag][4] = 0xFA;
				PAIR_IdTable.ubSTA_ID[tTag][5] = 0xF9;
			}
			break;
#else
		case PAIR_AP_ASSIGN:
			PAIR_IdTable.ulAp_ID = PAIR_INVALID_ID;	
			break;
		default:
			if(tTag <= PAIR_STA4)
				PAIR_IdTable.ulSTA_ID[tTag] = PAIR_INVALID_ID;
			break;
#endif
	}
}
//------------------------------------------------------------------------------
uint8_t *PAIR_GetId(PAIR_TAG tPair_StaNum)
{
#ifdef S2019A
	const static uint8_t ubPAIR_InvaildID[6] = {0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9};

    if((tPair_StaNum == PAIR_AP_ASSIGN) || (tPair_StaNum == PAIR_AP_SLAVE))
        return (uint8_t *)&PAIR_IdTable.ubAp_ID[0];
    else
        return (tPair_StaNum <= PAIR_STA4)?(uint8_t *)&(PAIR_IdTable.ubSTA_ID[tPair_StaNum][0]):(uint8_t *)&ubPAIR_InvaildID[0];
#else
	const static uint8_t ubPAIR_InvaildID[4] = {0xFB, 0xFA, 0xFC, 0xFE};

    if((tPair_StaNum == PAIR_AP_ASSIGN) || (tPair_StaNum == PAIR_AP_SLAVE))
        return (uint8_t *)&(PAIR_IdTable.ulAp_ID);
#if( defined(T2R) && defined(A7130) && defined(OP_AP))
	else if((tPair_StaNum >= PAIR_T2R_W_STA1) && (tPair_StaNum <= PAIR_T2R_W_STA4))
	{
		return (uint8_t *)&(tPAIR_Info.ulT2RStaTag[(tPair_StaNum&0x0F)]);
	}		
#endif	
#if( defined(T2R) && defined(A7130) && defined(OP_STA))	
	else if(tPair_StaNum == PAIR_T2R_STA)
	{
		return (uint8_t *)&(tPAIR_Info[0].ulPAIR_Sign);
	}		
#endif	
    else
        return (tPair_StaNum <= PAIR_STA4)?(uint8_t *)&(PAIR_IdTable.ulSTA_ID[tPair_StaNum]):(uint8_t *)&ubPAIR_InvaildID;
#endif
}
//------------------------------------------------------------------------------
void PAIR_Start(PAIR_TAG tPair_StaNum, uint32_t ulPair_Timeout, uint8_t ubPairTxPower)
{
	ubPAIR_InPairIngFlg = 1;
#if( defined(T2R) && defined(A7130))	
	PAIR_HopTimeStart();
#endif	
	
#ifdef RTC676x
	ubKNL_AccessLinkActiveTime(KNL_OPERATION_SET,KNL_LINK_ACTIVE_TIME);
#endif
	
	if(ubPAIR_State > PAIR_TIMEOUT)
    {
        printf("Not in pairing mode PAP\n");
        ubPAIR_InPairIngFlg = 0;
		
		return;
    }
#ifdef A7130
	BB_SetPairingTxPower(ubPairTxPower);
	BB_HoppingPairingStart();
#endif
//#ifdef RTC676x
//	RTC676x_SetPower(ubPairTxPower);
//#endif

    ulPAIR_Timeout = (ulPair_Timeout * 1000);
    printf("PAIR_Start\n");
    ulPAIR_TimeCnt = 0;
    ubPAIR_Number  = tPair_StaNum;
    ubPAIR_State   = PAIR_START;

#ifdef RTC676x	
	PAIR_SetRdyToPairFlg(0);
	if (rf_start_pairing(PAIR_ReadableCb, PAIR_EventCb, NULL,PAIR_PRODUCT_ID) < 0) 
	{
		printf("rf_start_pairing fail\n");
	} 
	else 
	{
		printf("rf_start_pairing success\n");
	}
#endif
	
#ifdef S2019A
	sPRF_StartPairing();
#endif

	osThreadResume(PAIR_ThreadId);
}
//------------------------------------------------------------------------------
void PAIR_Stop(void)
{
	ubPAIR_State 	= PAIR_TIMEOUT;	//! PAIR_END;
	ulPAIR_TimeCnt 	= ulPAIR_Timeout + 1;
}
//------------------------------------------------------------------------------
void PAIR_SetPairState(PAIR_STATE tState)
{
	ubPAIR_State = tState;
}
//------------------------------------------------------------------------------
PAIR_STATE tPAIR_GetPairState(void)
{
	return (PAIR_STATE)ubPAIR_State;
} 
//------------------------------------------------------------------------------
#if (OP_STA||OP_AP_SLAVE)
void PAIR_PreparePrp(void)
{
#if( defined(T2R) && defined(A7130) && defined(OP_STA))	
	uint32_t ulTemp;
#endif	
	
    PAIR_PrpPket.ubTxNumber   = ubPAIR_Number;
    PAIR_PrpPket.ubIdCheckKey = Timer3->TM3_COUNTER;
#ifdef S2019A
	if((PAIR_PrpPket.ubTxNumber <= PAIR_STA4) || (PAIR_AP_ASSIGN == PAIR_PrpPket.ubTxNumber))
	{
		uint8_t ubS2019Id[6];

		sPRF_GetIdInfo(ubS2019Id);
		memcpy(&PAIR_PrpPket.ubSTA_ID[0], &ubS2019Id[0], 6);
	}
#else
	if(PAIR_GetStaNumber() <= PAIR_STA4)
		PAIR_PrpPket.ulSTA_ID = PAIR_IdTable.ulSTA_ID[PAIR_GetStaNumber()];
#if( defined(T2R) && defined(A7130) && defined(OP_STA))
	
	if(!(((uint8_t)(tPAIR_Info[0].ulPAIR_Sign>>24)) == T2R))
	{
		ulTemp = Timer3->TM3_COUNTER;
		srand(ulTemp);
		ulTemp = rand();
		tPAIR_Info[0].ulPAIR_Sign = (ulTemp&0x00FFFFFF)|(((uint32_t)T2R)<<24);
	}		
	PAIR_PrpPket.ulPAIR_Sign = tPAIR_Info[0].ulPAIR_Sign;
#endif	
#endif
}
//------------------------------------------------------------------------------
void PAIR_Pap(TWC_TAG GetSta,uint8_t *pData)
{
    uint8_t ubTemp = 0;

    printf("Get PAP\n");
    if((ubPAIR_State < PAIR_PRP)&&(ubPAIR_State != PAIR_END))
    {
        printf("Not in pairing mode PAP %x\n",ubPAIR_State);
        return;
    }
	ubTemp = ubTemp;
    ubTemp = pData[0];
    if((pData[0] == PAIR_PrpPket.ubTxNumber) &&(PAIR_PrpPket.ubTxNumber == PAIR_AP_SLAVE)&& (ubPAIR_State == PAIR_PRP))
    {
        memcpy(&PAIR_PapPket, pData, sizeof(PAIR_PapPket));
		memcpy(&PAIR_IdTable, pData, sizeof(PAIR_ID_TABLE));
	
	#ifdef RTC676x
	#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE
	#if FREQ_SEL_DBG_EN
		printf("[P1]Select-Ch:0x%x @%s\r\n",PAIR_PapPket.ubSelectCh,__func__);
	#endif
	#endif
	#endif
		
        ubPAIR_State = PAIR_PAAP;
    }
#ifdef S2019A
	else if(((pData[0] == PAIR_PrpPket.ubTxNumber)||(PAIR_PrpPket.ubTxNumber == PAIR_AP_ASSIGN))&&(ubPAIR_State == PAIR_PRP))
#else
    else if(((pData[0] == PAIR_PrpPket.ubTxNumber)||(PAIR_PrpPket.ubTxNumber == PAIR_AP_ASSIGN))&&(PAIR_PrpPket.ubIdCheckKey == pData[ubTemp*4+8]) && (ubPAIR_State == PAIR_PRP))
#endif
    {
        memcpy(&PAIR_PapPket, pData, sizeof(PAIR_PapPket));
		memcpy(&PAIR_IdTable, pData, sizeof(PAIR_ID_TABLE));
	
	#ifdef RTC676x
	#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE
		printf("[P2]Select-Ch:0x%x @%s\r\n",PAIR_PapPket.ubSelectCh,__func__);
	#endif
	#endif
        ubPAIR_State = PAIR_PAAP;
    }
}
//------------------------------------------------------------------------------
void PAIR_PaapResp(TWC_TAG GetSta, TWC_STATUS tStatus)
{
#ifdef S2019A
	if(FALSE == ubPAIR_EndFlag)
		return;
#endif
	ubPAIR_State = PAIR_END;
}
//------------------------------------------------------------------------------
void PAIR_LoadPairingResult(uint8_t *pRole)
{
    *pRole = PAIR_GetStaNumber();
}
#endif
//------------------------------------------------------------------------------
PAIR_TAG PAIR_GetStaNumber(void)
{
    return (PAIR_TAG)PAIR_IdTable.ubTxNumber;
}
//------------------------------------------------------------------------------
#if OP_AP
void PAIR_PreparePap(void)
{
#ifdef S2019A
    uint8_t ubS2019Id[6];
#else
	uint32_t ulTemp;
#endif
    memcpy(&PAIR_PapPket , &PAIR_IdTable, sizeof(PAIR_RAP_Hdr));
    if(PAIR_PrpPket.ubTxNumber == PAIR_AP_ASSIGN)
    {
        ubPAIR_Number = ubPAIR_Number;
    }
    else
    {
        ubPAIR_Number = PAIR_PrpPket.ubTxNumber;
    }
#if( defined(T2R) && defined(A7130))	
	ulPAIR_BufStaSign[ubPAIR_Number] = PAIR_PrpPket.ulPAIR_Sign;
#endif
	
#if (DISPLAY_MODE == DISPLAY_1T1R)	
	tPAIR_Info.ulPAIR_Sign = 0;
#endif
#ifdef S2019A
	PAIR_PapPket.ubTxNumber = ubPAIR_Number;
	sPRF_GetIdInfo(ubS2019Id);
	memcpy(&PAIR_PapPket.ubAp_ID[0], &ubS2019Id[0], 6);
	tPAIR_Info.ulPAIR_Sign = PAIR_SIGN;
	memcpy(&PAIR_PapPket.ubSTA_ID[ubPAIR_Number][0], &PAIR_PrpPket.ubSTA_ID[0], 6);
#else
	
	if(tPAIR_Info.ulPAIR_Sign != PAIR_SIGN)	//!< if(PAIR_PapPket.ubTxNumber != 0x0F)	
    {
//        ulTemp  = Timer3->TM3_COUNTER << 8;
//        ulTemp &= 0x00FFFF00;
//		  PAIR_PapPket.ulAp_ID 	 = ulTemp + 0x0F000000;
		ulTemp = Timer3->TM3_COUNTER;
		srand(ulTemp);
		ulTemp = rand();
        PAIR_PapPket.ulAp_ID 	 = ulTemp;
	#ifdef A7130
        PAIR_PapPket.ulSTA_ID[0] = PAIR_INVALID_ID;
        PAIR_PapPket.ulSTA_ID[1] = PAIR_INVALID_ID;
        PAIR_PapPket.ulSTA_ID[2] = PAIR_INVALID_ID;
        PAIR_PapPket.ulSTA_ID[3] = PAIR_INVALID_ID;
	#endif
	#ifdef RTC676x
		PAIR_PapPket.ulSTA_ID[0] = ulTemp + 0x01000000;
        PAIR_PapPket.ulSTA_ID[1] = ulTemp + 0x02000000;
        PAIR_PapPket.ulSTA_ID[2] = ulTemp + 0x03000000;
        PAIR_PapPket.ulSTA_ID[3] = ulTemp + 0x04000000;
		
	#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE	
	#if (FREQ_TABLE_NUM == 2)		
		if(uwsPRF_GetWorkChFreq() >= FREQ_MID_FOR_NUM2)		
		{
			PAIR_PapPket.ubSelectCh = 0xF0;//Using Table[0]	
		#if FREQ_SEL_DBG_EN		
			printf("[P1]CurWiFiCh:%d -> Select Table[0]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}	
		else
		{
			PAIR_PapPket.ubSelectCh = 0xF2;//Using Table[2]			
		#if FREQ_SEL_DBG_EN
			printf("[P1]CurWiFiCh:%d -> Select Table[2]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}
	#endif
	#if (FREQ_TABLE_NUM == 3)		
		if(uwsPRF_GetWorkChFreq() <= FREQ_P1_FOR_NUM3)
		{
			PAIR_PapPket.ubSelectCh = 0xF0;//Using Table[0]			
		#if FREQ_SEL_DBG_EN	
			printf("[P1]CurWiFiCh:%d -> Select Table[0]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}	
		else if(uwsPRF_GetWorkChFreq() <= FREQ_P2_FOR_NUM3)
		{
			PAIR_PapPket.ubSelectCh = 0xF1;//Using Table[1]			
		#if FREQ_SEL_DBG_EN
			printf("[P1]CurWiFiCh:%d -> Select Table[1]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}
		else
		{
			PAIR_PapPket.ubSelectCh = 0xF2;//Using Table[2]			
		#if FREQ_SEL_DBG_EN
			printf("[P1]CurWiFiCh:%d -> Select Table[2]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}
	#endif		
	#endif		
	#endif
		tPAIR_Info.ulPAIR_Sign   = PAIR_SIGN;			
    }
    PAIR_PapPket.ubTxNumber = ubPAIR_Number;
    if(PAIR_PrpPket.ubTxNumber != PAIR_AP_SLAVE)
    {
		ulTemp = Timer3->TM3_COUNTER;
		srand(ulTemp);
		ulTemp = rand();
		PAIR_PapPket.ulSTA_ID[ubPAIR_Number]  = ulTemp;
        PAIR_PapPket.ulSTA_ID[ubPAIR_Number] &= 0xFFFFFF00;
//        PAIR_PapPket.ulSTA_ID[ubPAIR_Number] += 0x100;
        PAIR_PapPket.ulSTA_ID[ubPAIR_Number] |= PAIR_PrpPket.ubIdCheckKey;
	#ifdef RTC676x	
	#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE			
	#if (FREQ_TABLE_NUM == 2)
		if(uwsPRF_GetWorkChFreq() >= FREQ_MID_FOR_NUM2)		
		{
			PAIR_PapPket.ubSelectCh = 0xF0;//Using Table[0]			
		#if FREQ_SEL_DBG_EN	
			printf("[P2]CurWiFiCh:%d -> Select Table[0]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}	
		else
		{
			PAIR_PapPket.ubSelectCh = 0xF2;//Using Table[2]		
		#if FREQ_SEL_DBG_EN
			printf("[P2]CurWiFiCh:%d -> Select Table[2]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}	
	#endif
	#if (FREQ_TABLE_NUM == 3)		
		if(uwsPRF_GetWorkChFreq() <= FREQ_P1_FOR_NUM3)
		{
			PAIR_PapPket.ubSelectCh = 0xF0;//Using Table[0]			
		#if FREQ_SEL_DBG_EN
			printf("[P1]CurWiFiCh:%d -> Select Table[0]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}	
		else if(uwsPRF_GetWorkChFreq() <= FREQ_P2_FOR_NUM3)
		{
			PAIR_PapPket.ubSelectCh = 0xF1;//Using Table[1]			
		#if FREQ_SEL_DBG_EN
			printf("[P1]CurWiFiCh:%d -> Select Table[1]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}
		else
		{
			PAIR_PapPket.ubSelectCh = 0xF2;//Using Table[2]			
		#if FREQ_SEL_DBG_EN
			printf("[P1]CurWiFiCh:%d -> Select Table[2]\r\n",uwsPRF_GetWorkChFreq());
		#endif
		}
	#endif			
	#endif
	#endif
    }
#endif
}
//------------------------------------------------------------------------------
void PAIR_Prp(TWC_TAG GetSta,uint8_t *pData)
{
    printf("Get PRP\n");
    if(ubPAIR_State < PAIR_START)
    {
        printf("Not in pairing mode PRP\n");
        return;
    }

    if(ubPAIR_State == PAIR_START)
    {
		PAIR_TAG tPAIR_Tag;
#ifdef S2019A
		uint8_t ubInvaildID[6] = {0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9};

		memcpy(&PAIR_PrpPket, pData, sizeof(PAIR_RRP_Hdr));
		tPAIR_DelStaNum = PAIR_AP_ASSIGN;
		for(tPAIR_Tag = PAIR_STA1; tPAIR_Tag <= PAIR_STA4; tPAIR_Tag++)
		{
			if((memcmp(&PAIR_PrpPket.ubSTA_ID[0], &ubInvaildID[0], 6)) &&
			   (!memcmp(&PAIR_PrpPket.ubSTA_ID[0], &PAIR_IdTable.ubSTA_ID[tPAIR_Tag][0], 6)))
			{
				if(tPAIR_Tag != ubPAIR_Number)
				{
					tPAIR_DelStaNum = tPAIR_Tag;
					memcpy(&PAIR_IdTable.ubSTA_ID[tPAIR_Tag][0], &ubInvaildID[0], 6);
				}
			}
		}
#else
		memcpy(&PAIR_PrpPket, pData, sizeof(PAIR_RRP_Hdr));
		tPAIR_DelStaNum = PAIR_AP_ASSIGN;
		for(tPAIR_Tag = PAIR_STA1; tPAIR_Tag <= PAIR_STA4; tPAIR_Tag++)
		{
			if((PAIR_PrpPket.ulSTA_ID != PAIR_INVALID_ID) &&
			   (PAIR_PrpPket.ulSTA_ID == PAIR_IdTable.ulSTA_ID[tPAIR_Tag]))
			{
				if(tPAIR_Tag != ubPAIR_Number)
				{
					tPAIR_DelStaNum = tPAIR_Tag;
					PAIR_IdTable.ulSTA_ID[tPAIR_Tag] = PAIR_INVALID_ID;
				}
			}
		}
#endif
        PAIR_PreparePap();
        ubPAIR_State = PAIR_PAP;
    }
}
//------------------------------------------------------------------------------
void PAIR_Paap(TWC_TAG GetSta,uint8_t *pData)
{
    printf("Get PAAP\n");
    if((ubPAIR_State < PAIR_PAP)&&(ubPAIR_State != PAIR_END))
    {
        printf("Not in pairing mode PAAP %d\n",ubPAIR_State);
        return;
    }
    if(ubPAIR_State == PAIR_PAP)
    {
        if(memcmp(&PAIR_PapPket , pData, sizeof(PAIR_RAP_Hdr))==0)
        {
			memcpy(&PAIR_IdTable, pData, sizeof(PAIR_ID_TABLE));
			
			#if( defined(T2R) && defined(A7130) && defined(OP_AP))	
			tPAIR_Info.ulT2RStaTag[ubPAIR_Number] = ulPAIR_BufStaSign[ubPAIR_Number];
			#endif
//          PAIR_SaveId();
            ubPAIR_State = PAIR_END;
        }
        else
            printf("memcmp err\n");
    }
}
#endif
//------------------------------------------------------------------------------
void PAIR_CheckIdTable(void)
{
#ifdef A7130	
	#if ((defined(OP_AP)) && ((DISPLAY_MODE == DISPLAY_1T1R) || (DISPLAY_MODE == DISPLAY_2T1R)))
	uint8_t i;
	for(i = DISPLAY_MODE; i < 4; i++)
		PAIR_IdTable.ulSTA_ID[i] = PAIR_INVALID_ID;
	#endif
    
#if OP_AP	
    if(0xFFFFFFFF == PAIR_IdTable.ulAp_ID)
        PAIR_IdTable.ulAp_ID = 0xFAFAFAFA;  
#else
    if(0xFFFFFFFF == PAIR_IdTable.ulAp_ID)
        PAIR_IdTable.ulAp_ID = 0x3B69C0F2;
#endif	
	
	#if( defined(OP_STA))
		if(PAIR_IdTable.ubTxNumber > PAIR_STA4)
		{
			PAIR_IdTable.ubTxNumber = PAIR_STA1;
		}		
	#else
		PAIR_IdTable.ubTxNumber = PAIR_AP_ASSIGN;
	#endif	

#endif
#if RTC676x	
	if((PAIR_IdTable.ulAp_ID == 0xFFFFFFFFL) || (PAIR_IdTable.ulAp_ID == PAIR_INVALID_ID))
	{
		PAIR_IdTable.ulAp_ID = 0x0;
	}
	if((PAIR_IdTable.ulSTA_ID[0] == 0xFFFFFFFFL) || (PAIR_IdTable.ulSTA_ID[0] == PAIR_INVALID_ID))
	{
		PAIR_IdTable.ulSTA_ID[0] = 0x0;
	}
	if((PAIR_IdTable.ulSTA_ID[1] == 0xFFFFFFFFL) || (PAIR_IdTable.ulSTA_ID[1] == PAIR_INVALID_ID))
	{
		PAIR_IdTable.ulSTA_ID[1] = 0x0;
	}
	if((PAIR_IdTable.ulSTA_ID[2] == 0xFFFFFFFFL) || (PAIR_IdTable.ulSTA_ID[2] == PAIR_INVALID_ID))
	{
		PAIR_IdTable.ulSTA_ID[2] = 0x0;
	}
	if((PAIR_IdTable.ulSTA_ID[3] == 0xFFFFFFFFL) || (PAIR_IdTable.ulSTA_ID[3] == PAIR_INVALID_ID))
	{
		PAIR_IdTable.ulSTA_ID[3] = 0x0;
	}
#endif
}
//------------------------------------------------------------------------------
void PAIR_ShowDeviceID(void)
{
#ifdef S2019A
	PAIR_TAG tPAIR_Tag;

    printf("ID Table: 0x%X\n", PAIR_IdTable.ubTxNumber);
	printf("    [%X],  %02X:%02X:%02X:%02X:%02X:%02X\n", 0xF, *((uint8_t *)&PAIR_IdTable.ubAp_ID[0]), *((uint8_t *)&PAIR_IdTable.ubAp_ID[1]), *((uint8_t *)&PAIR_IdTable.ubAp_ID[2]),
															  *((uint8_t *)&PAIR_IdTable.ubAp_ID[3]), *((uint8_t *)&PAIR_IdTable.ubAp_ID[4]), *((uint8_t *)&PAIR_IdTable.ubAp_ID[5]));
	for(tPAIR_Tag = PAIR_STA1; tPAIR_Tag <= PAIR_STA4; tPAIR_Tag++)
	{
	printf("    [%X],  %02X:%02X:%02X:%02X:%02X:%02X\n", tPAIR_Tag, *((uint8_t *)&PAIR_IdTable.ubSTA_ID[tPAIR_Tag][0]), *((uint8_t *)&PAIR_IdTable.ubSTA_ID[tPAIR_Tag][1]), *((uint8_t *)&PAIR_IdTable.ubSTA_ID[tPAIR_Tag][2]),
																    *((uint8_t *)&PAIR_IdTable.ubSTA_ID[tPAIR_Tag][3]), *((uint8_t *)&PAIR_IdTable.ubSTA_ID[tPAIR_Tag][4]), *((uint8_t *)&PAIR_IdTable.ubSTA_ID[tPAIR_Tag][5]));
	}
	printf("\n");
#else
	PAIR_TAG tPAIR_Tag;

    printf("ID Table: 0x%X\n", PAIR_IdTable.ubTxNumber);
	printf("    [%X] : %02X.%02X.%02X.%02X\n", 0xF, *((uint8_t *)&PAIR_IdTable.ulAp_ID), *((uint8_t *)&PAIR_IdTable.ulAp_ID + 1),
	                                                *((uint8_t *)&PAIR_IdTable.ulAp_ID + 2), *((uint8_t *)&PAIR_IdTable.ulAp_ID + 3));
	for(tPAIR_Tag = PAIR_STA1; tPAIR_Tag <= PAIR_STA4; tPAIR_Tag++)
	{
		printf("    [%X] : %02X.%02X.%02X.%02X\n", tPAIR_Tag, *((uint8_t *)&PAIR_IdTable.ulSTA_ID[tPAIR_Tag]), *((uint8_t *)&PAIR_IdTable.ulSTA_ID[tPAIR_Tag] + 1),
															  *((uint8_t *)&PAIR_IdTable.ulSTA_ID[tPAIR_Tag] + 2), *((uint8_t *)&PAIR_IdTable.ulSTA_ID[tPAIR_Tag] + 3));
	}
	printf("\n");
#endif
}
//------------------------------------------------------------------------------
void ubPAIR_ChangeStaIDGroup(uint8_t ubGroup)
{
#if( defined(T2R) && defined(A7130) && defined(OP_STA))
	ubPAIR_NewGroup = ubGroup;
#endif		
}	
//------------------------------------------------------------------------------
void PAIR_HopTimeStart(void)
{
#if( defined(T2R) && defined(A7130) && defined(OP_STA))
	if(ubPAIR_NowGroup != ubPAIR_NewGroup)
	{	
		ubPAIR_NowGroup = ubPAIR_NewGroup;
		memcpy(&PAIR_IdTable, &tPAIR_Info[ubPAIR_NowGroup].tPAIR_Table, sizeof(PAIR_ID_TABLE));	
		PAIR_CheckIdTable();
		BB_PairingStartOrEndReSetDefaulf(0);
	}
#endif	
}
//-----------------------------------------------------------------------------
uint8_t ubPAIR_GetNowGroup(void)
{
#if( defined(T2R) && defined(A7130) && defined(OP_STA))
	return ubPAIR_NowGroup;
#else
	return 0;
#endif
}		
//------------------------------------------------------------------------------
uint8_t ubPAIR_GetPairingStatus(void)
{
	return ubPAIR_State;
}
//------------------------------------------------------------------------------
uint16_t uwPAIR_GetVersion(void)
{
    return ((PAIR_MAJORVER << 8) + PAIR_MINORVER);
}
