/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PAIR.h
	\brief		Pairing header
	\author		Bing
	\version	0.19
	\date		2020/02/27
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------
#ifndef _PAIR_H_
#define _PAIR_H_

#include "_510PF.h"
#include "TWC_API.h"
#include "sPRF_API.h"


/*_____ I N C L U D E S ____________________________________________________*/
#if (APP_DOORPHONE_ENABLE==1)
#define T2R		0x89
#endif


#define PAIR_SIGN				0x5105046
#define PAIR_INVALID_ID			0xFBFAFCFE
#define PAIR_PRODUCT_ID			0xA5


#if OP_AP
#define PAIR_NO_PAIR_AP_ID	   		0x759C5BDA
#define PAIR_NO_PAIR_STA1_ID	   	0x3CABF295
#define PAIR_NO_PAIR_STA2_ID	   	0x85ACD257
#define PAIR_NO_PAIR_STA3_ID	   	0xCDA98752
#define PAIR_NO_PAIR_STA4_ID	   	0xDAC56824
#else
#define PAIR_NO_PAIR_AP_ID	   		0xC5EC12B3
#define PAIR_NO_PAIR_STA1_ID	   	0xACDA2526
#define PAIR_NO_PAIR_STA2_ID	   	0x75EC1125
#define PAIR_NO_PAIR_STA3_ID	   	0x2ACC554A
#define PAIR_NO_PAIR_STA4_ID	   	0x68ACD582
#endif


typedef enum
{
	PAIR_STA1		= 0,
	PAIR_STA2		= 1,
	PAIR_STA3		= 2,
	PAIR_STA4		= 3,
	PAIR_AP_SLAVE	= 4,
	PAIR_AP_ASSIGN	= 0xF,
#if( defined(T2R) && defined(A7130))
	PAIR_T2R_W_STA1	= 0x10,
	PAIR_T2R_W_STA2	= 0x11,
	PAIR_T2R_W_STA3	= 0x12,
	PAIR_T2R_W_STA4	= 0x13,
	PAIR_T2R_STA	= 0x1F,
#endif	
}PAIR_TAG;	

typedef enum 
{
	PAIR_NULL = 0,
	PAIR_SUCCESS,
	PAIR_STANDBY,
    PAIR_END,
    PAIR_TIMEOUT,
	PAIR_START,
	PAIR_PRP,
	PAIR_PAP,					
	PAIR_PAAP	
}PAIR_STATE;

#ifdef S2019A

typedef struct
{
	uint8_t						ubTxNumber; 
	uint8_t						ubIdCheckKey;
//	uint8_t						ubWorkCh[3];
	uint8_t						ubSTA_ID[6];
}PAIR_RRP_Hdr;

typedef struct
{
	uint8_t						ubTxNumber;
//	uint8_t						ubWorkCh[3];
	uint8_t						ubAp_ID[6];
    uint8_t						ubSTA_ID[4][6];
}PAIR_RAP_Hdr, PAIR_ID_TABLE;

#else

typedef struct
{
	uint8_t						ubTxNumber;
	uint8_t						ubIdCheckKey;
	uint32_t					ulSTA_ID;
#if( defined(T2R) && defined(A7130))
	uint32_t					ulPAIR_Sign;
#endif	
}PAIR_RRP_Hdr;

typedef struct
{
	uint8_t						ubTxNumber;
	uint32_t					ulAp_ID;
    uint32_t					ulSTA_ID[4];	
#ifdef RTC676x
#if APP_RTC676X_BRIDGE_COEXISTENCE_ENABLE
	uint8_t 					ubSelectCh;	// 0 for Freq-Table[0]
											// 1 for Freq-Table[1] 	
											// 2 for Freq-Table[2]
#endif
#endif	
}PAIR_RAP_Hdr, PAIR_ID_TABLE;

#endif	//! End of #ifdef S2019A

typedef struct
{
	uint32_t 					ulPAIR_Sign;
	PAIR_ID_TABLE				tPAIR_Table;
#if( defined(T2R) && defined(A7130) && defined(OP_AP))	
	uint32_t					ulT2RStaTag[4];
#endif
}PAIR_Info_t;

typedef struct
{
	uint8_t ubPAIR_Event;
	uint8_t ubPAIR_Message[7];
}PAIR_EventMsg_t;

#define PAIR_TIMEOUT_DELAY			0xFF00
#define PAIR_START_DELAY	    	200  	//!< 50 msec
#define PAIR_PRP_DELAY				300  	//!< 50 msec
#define PAIR_PAP_DELAY				500  	//!< 20 msec
#define PAIR_PAAP_DELAY				100  	//!< 20 msec
#define PAIR_END_DELAY				100  	//!< 50 msec
#define PAIR_PAAP_SENT_CNT      	10



extern PAIR_ID_TABLE   PAIR_IdTable;

//------------------------------------------------------------------------------
/*!
\brief Pairing function initialize
\param pvMsgQId			Message Queue handle
\return(no)
*/
void PAIR_Init(osMessageQId *pvMsgQId);
//------------------------------------------------------------------------------
/*!
\brief Pairing start function
\param tPair_StaNum		Station Number
\param ulPair_Timeout	Pairing timeout, Unit: seconds
\param ubPairTxPower	0:Pairing TX power 0 dBm, 1:Pairing TX power by defual
\return(no)
*/
void PAIR_Start(PAIR_TAG tPair_StaNum, uint32_t ulPair_Timeout,uint8_t ubPairTxPower);
//------------------------------------------------------------------------------
/*!
\brief Pairing stop function
\return(no)
*/
void PAIR_Stop(void);
//------------------------------------------------------------------------------
/*!
\brief Pairing stop function
\return Station number
*/
PAIR_TAG PAIR_GetStaNumber(void);
//------------------------------------------------------------------------------
/*!
\brief Delete TX ID
\param tStaNum			Station Number
\return(no)
*/
void PAIR_DeleteTxId(PAIR_TAG tStaNum);
void PAIR_SetDevInvaildId(PAIR_TAG tTag);

void PAIR_LoadId(void);
void PAIR_SaveId(void);
uint8_t *PAIR_GetId(PAIR_TAG tPair_SrcNum);
void PAIR_Task(void* pdata);
PAIR_STATE tPAIR_GetPairState(void);
void PAIR_SetPairState(PAIR_STATE tState);
#if (OP_STA||OP_AP_SLAVE)
void PAIR_PreparePrp(void);
void PAIR_Pap(TWC_TAG GetSta,uint8_t *pData);
void PAIR_PaapResp(TWC_TAG GetSta, TWC_STATUS tStatus);
void PAIR_LoadPairingResult(uint8_t *pRole);
#endif
#if OP_AP
void PAIR_PreparePap(void);
void PAIR_Prp(TWC_TAG GetSta,uint8_t *pData);
void PAIR_Paap(TWC_TAG GetSta,uint8_t *pData);
#endif
void PAIR_CheckIdTable(void);
void PAIR_ShowDeviceID(void);
uint8_t ubPAIR_GetInPairIngFlg(void);
uint8_t ubPAIR_GetPairingStatus(void);
uint8_t ubPAIR_GetNowGroup(void);
void ubPAIR_ChangeStaIDGroup(uint8_t ubGroup);
void PAIR_HopTimeStart(void);
//------------------------------------------------------------------------
/*!
\brief 	Get Pairing Version	
\return	Version
*/
uint16_t uwPAIR_GetVersion(void);
#endif
