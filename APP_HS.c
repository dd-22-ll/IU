/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_HS.c
	\brief		Application function (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.87
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "APP_HS.h"
#include "APP_CFG.h"
#include "FWU_API.h"
#include "PROFILE_API.h"
#include "RTC_API.h"
#include "SF_API.h"
#include "MMU_API.h"
#include "DMAC_API.h"
#include "BB_API.h"
#include "SD_API.h"
#include "VDO.h"
#include "ADO.h"
#include "RC.h"
#include "CLI.h"
#include "CIPHER_API.h"
#include "LCD.h"
#include "WDT.h"
#include "I2C.h"
#include "UI.h"
#include "cmsis_os.h" // FreeRTOS

#ifdef VBM_PU
#include "UI_VBMPU.h"
#endif

#ifdef CFG_UART1_ENABLE
#include "UI_UART1.h"
#endif
//------------------------------------------------------------------------------
#if (defined(VBM_BU))
const char cAPP_ModelName[] __attribute__((section(".ARM.__at_0x00005FE0"))) = "937VBMBU";
#elif (defined(VBM_PU))
const char cAPP_ModelName[] __attribute__((section(".ARM.__at_0x00005FE0"))) = "937VBMPU";
#endif
const uint8_t ubAPP_SfWpGpioPin __attribute__((section(".ARM.__at_0x00005FF0"))) = SF_WP_GPIN;
osMessageQId APP_EventQueue;
pvAPP_StateCtrl APP_StateCtrlFunc;
static APP_StatusReport_t tAPP_StsReport;
osMutexId APP_UpdateMutex;
uint32_t ulAPP_WaitTickTime;
const static APP_StaNumMap_t tAPP_STANumTable[CAM_4T] =
{
	[CAM1] = {KNL_STA1, PAIR_STA1, TWC_STA1},
	[CAM2] = {KNL_STA2, PAIR_STA2, TWC_STA2},
	[CAM3] = {KNL_STA3, PAIR_STA3, TWC_STA3},
	[CAM4] = {KNL_STA4, PAIR_STA4, TWC_STA4},
};
static APP_KNLInfo_t tAPP_KNLInfo;
#ifdef VBM_PU
const static APP_DispLocMap_t tAPP_DispLocMap[] =
{
	//! Qual View
	[DISP_UPPER_LEFT]  = {KNL_DISP_LOCATION3},
	[DISP_UPPER_RIGHT] = {KNL_DISP_LOCATION1},
	[DISP_LOWER_LEFT]  = {KNL_DISP_LOCATION4},
	[DISP_LOWER_RIGHT] = {KNL_DISP_LOCATION2},
	//! Dual View
	[DISP_LEFT]  	   = {KNL_DISP_LOCATION2},
	[DISP_RIGHT]  	   = {KNL_DISP_LOCATION1},
	//! Single View
	[DISP_1T]		   = {KNL_DISP_LOCATION1},
};
APP_PairRoleInfo_t tAPP_PairRoleInfo;
#endif
static void APP_StartThread(void const *argument);
static void APP_WatchDogThread(void const *argument);
//------------------------------------------------------------------------------
unsigned char invalid_param;
void APP_Init(void)
{
    osStatus APP_OsStatus;

	APP_OsStatus = osKernelInitialize((uint8_t*)ulMMU_GetCacheHeapStartAddr(), 
												osHeapSize, 
									  (uint8_t*)ulMMU_GetUnCacheHeapStartAddr(), 
												ulMMU_GetUnCacheHeapSize());   	
	RTC_Init((BSP_RTC_TIMER_SEL == RTC_TIMER_INTERNAL && APP_TIMESTAMP_FUNC_ENABLE)?RTC_TimerEnable:RTC_TimerDisable);
	BSP_Init();
	if(APP_OsStatus != osOK)
	{
			printd(DBG_ErrorLvl, "RTOS initial fail\n");
	}
	
	printd(DBG_ErrorLvl, "APP Init\n");
	
	if(ubAPP_SfWpGpioPin <= 13)
	{
		printd(DBG_InfoLvl, "SF_WP=GPIO%d\n", ubAPP_SfWpGpioPin);
	}
	
	SF_SetWpPin(ubAPP_SfWpGpioPin);
	SF_Init();
	PROF_Init();
	MMU_Init();
	TWC_Init();
	CLI_Init();
	CIPHER_Init();

#ifdef CFG_UART1_ENABLE
	UART1_RecvInit();
#endif

	FWU_Init();
	UI_Init(&APP_EventQueue);
	if (BSP_DDRSIZE > (ulDDR_GetCapacity() >> 20))
	{	
		printd(DBG_ErrorLvl, "Select IC part number fail!\n");
		while(1);
	}
	
	tAPP_StsReport.tAPP_State  	= APP_POWER_OFF_STATE;
	ulAPP_WaitTickTime      	= 0;	//!< osWaitForever;
	APP_StateCtrlFunc 			= APP_StateFlowCtrl;
	osMutexDef(AppUpdate);
	APP_UpdateMutex 			= osMutexCreate(osMutex(AppUpdate));
	osMessageQDef(APP_EventQueue, APP_EVENTQUEUE_SZ, APP_EventMsg_t);
	APP_EventQueue = osMessageCreate(osMessageQ(APP_EventQueue), NULL);
    osThreadDef(APP_StartThread, APP_StartThread, THREAD_PRIO_APP_HANDLER, 1, THREAD_STACK_APP_HANDLER);
    if(osThreadCreate(osThread(APP_StartThread), NULL) == NULL)
	{
		printd(DBG_ErrorLvl, "Create APP_StartThread fail!\n");
		while(1);
	}
    osThreadDef(APP_WatchDogThread, APP_WatchDogThread, THREAD_PRIO_WDT_HANDLER, 1, 256);
    if(osThreadCreate(osThread(APP_WatchDogThread), NULL) == NULL)
	{
		printd(DBG_ErrorLvl, "Create APP_WatchDogThread fail!\n");
		while(1);
	}
	//
	osThreadDef(APP_I2CSlaveTask2, APP_I2CSlaveTask2, THREAD_PRIO_I2C_SLAVE2_HANDLER, 1, THREAD_STACK_I2C_SLAVE2_HANDLER);
  if(osThreadCreate(osThread(APP_I2CSlaveTask2), NULL) == NULL)
  {
      printd(DBG_ErrorLvl, "Create APP_I2CSlaveTask2 fail!\n");
  }
	/*
	osThreadDef(APP_I2CMasterTask2, APP_I2CMasterTask2, THREAD_PRIO_I2C_MASTER2_HANDLER, 1, THREAD_STACK_I2C_MASTER2_HANDLER);
  if(osThreadCreate(osThread(APP_I2CMasterTask2), NULL) == NULL)
  {
      printd(DBG_ErrorLvl, "Create APP_I2CMasterTask2 fail!\n");
  }
	*/
	/*! Start the kernel.  From here on, only tasks and interrupts will run. */
	osKernelStart();

	/*! If all is well, the scheduler will now be running, and the following
	line will never be reached. */
	for( ;; );
}
//------------------------------------------------------------------------------
void APP_StartThread(void const *argument)
{
	osStatus osAPP_EventStauts;
	APP_EventMsg_t tAPP_EventMsg;

	while(1)
	{
		osAPP_EventStauts = osMessageGet(APP_EventQueue, &tAPP_EventMsg, ulAPP_WaitTickTime);
		if(osAPP_EventStauts == osEventTimeout)
			tAPP_EventMsg.ubAPP_Event = APP_REFRESH_EVENT;
		if(APP_StateCtrlFunc)
			APP_StateCtrlFunc(&tAPP_EventMsg);
	}
}
//------------------------------------------------------------------------------
void APP_WatchDogThread(void const *argument)
{
	while(1)
	{
		WDT_TimerClr(WDT_RST);
		osDelay(500);
	}
}
//------------------------------------------------------------------------------
//
void APP_I2CSlaveTask2(void const *argument)
{
    I2C1_Type *pI2C2_Handle; 
    uint8_t ubRxBuffer[64];  

    printf("[I2C2 Slave Task] Started.\n");

    pI2C2_Handle = pI2C_SlaveInit(I2C_2, I2C_SLAVE, I2C_SADDR8, 0x68);
	
    if (pI2C2_Handle == NULL)
    {
        printf("[I2C2 Slave Task] Error: Initialization failed!\n");
        return; 
    }
    printf("[I2C2 Slave Task] Initialized. Slave Addr = 0x%02X. Waiting for messages...\n", 0x68);
		
		//GPIO->GPIO_OE13 = 0;
    //GPIO->GPIO_OE14 = 0;
		printf("[I2C2 Master Task] Current pin function (GLB->PADIO13): %d\n", GLB->PADIO13);
    printf("[I2C2 Master Task] Current pin function (GLB->PADIO14): %d\n", GLB->PADIO14);
		
		//I2C_SlaveSetSpeed (pI2C2_Handle, I2C_SCL_100K);

    while(1)
    {
				
				if(bI2C_SlaveAddrMatch(pI2C2_Handle) == true)
				{
						printf("11111success\n");
						//clear flag
						osDelay(100);
				}
				else
				{
						printf("11111111fail\n");
						osDelay(100);
				}
				
			
			/*
        memset(ubRxBuffer, 0, sizeof(ubRxBuffer));

        if (bI2C_SlaveIntRead(pI2C2_Handle, I2C_START_INT, ubRxBuffer, sizeof(ubRxBuffer)))
        {
						printf("[I2C2 Slave Task] Received data (Buffer content):");
            for (uint32_t i = 0; i < sizeof(ubRxBuffer); ++i)
            {
                printf(" %02X", ubRxBuffer[i]);
            }
            printf("\n");
        }
				else
				{
					printf("1111111");
					osDelay(100);
				}
				*/
				
        osDelay(10);
    }
}
//
//------------------------------------------------------------------------------
/*
void APP_I2CMasterTask2(void const *argument)
{
    I2C1_Type *pI2C2_Handle;
    uint8_t ubTxBuffer[2] = {0x50, 0x55}; 
    uint8_t target_slave_address = 0x68 >> 1; 

    printf("[I2C2 Master Task] Started.\n");

    pI2C2_Handle = pI2C_MasterInit(I2C_2, I2C_SCL_100K);

    if (pI2C2_Handle == NULL)
    {
        printf("[I2C2 Master Task] Error: Master Initialization failed!\n");
        osThreadTerminate(NULL);
        return;
    }
    printf("[I2C2 Master Task] Initialized as Master.\n");
		
		printf("[I2C2 Master Task] Forcing GPIO output mode on pins 13 & 14...\n");
    GPIO->GPIO_OE13 = 1;
    GPIO->GPIO_OE14 = 1;
    
    printf("[I2C2 Master Task] Current pin function (GLB->PADIO13): %d\n", GLB->PADIO13);
    printf("[I2C2 Master Task] Current pin function (GLB->PADIO14): %d\n", GLB->PADIO14);

    while(1)
    {
        osDelay(1000); 
				
        //printf("[I2C2 Master Task] Writing data: 0x%02X 0x%02X to Slave  0x%02X\n",
               //ubTxBuffer[0], ubTxBuffer[1], target_slave_address);bI2C_MasterProcess(pI2C2_Handle, target_slave_address, ubTxBuffer, sizeof(ubTxBuffer), NULL, 0)
				
        if (bI2C_MasterInt (pI2C2_Handle, I2C_INT, target_slave_address, ubTxBuffer, sizeof(ubTxBuffer), NULL, 0))
        {
            printf("[I2C2 Master Task] Write successful.\n");
        }
        else
        {
            printf("[I2C2 Master Task] Error: Write failed (No ACK?).\n");
        }
				
				//printf(" 8~15: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO8,  GLB->PADIO9,  GLB->PADIO10, GLB->PADIO11, GLB->PADIO12, GLB->PADIO13, GLB->PADIO14, GLB->PADIO15);
				
				printf("[I2C2 Master Task] Toggling GPIO_O_SET/CLR for 13 & 14...\n");
        
        
        //GPIO->GPIO_O_CLR = (1 << 13) | (1 << 14); 
        //osDelay(4);
        
        //GPIO->GPIO_O_SET = (1 << 13) | (1 << 14); 
        //osDelay(4);
		}
}
*/
//------------------------------------------------------------------------------
void APP_PowerOnFunc(void)
{
	uint32_t ulBUF_StartAddr = 0;

	
#if (defined(OP_STA))		
		//Sensor Buffer
#if (BSP_DDRSIZE == 16)
	#if (defined(RTC676x))
		KNL_SetYuvBufNub(1);
		KNL_SetSenThenEncEnable(1);
	#else
		KNL_SetYuvBufNub(2);
		KNL_SetSenThenEncEnable(0);
	#endif
#else
        KNL_SetYuvBufNub(2);
		KNL_SetSenThenEncEnable(0);
#endif	
#endif	
	
	
	APP_LoadKNLSetupInfo();
#if USBD_ENABLE
	//! USB Device initialization
	USBD_Init(tAPP_KNLInfo.tUsbdClassMode);
	if(USBD_COMPOSITE_MODE == tAPP_KNLInfo.tUsbdClassMode)
		tUSBD_RegXuCbFunc(USBD_OPC_NVR, NULL);
#endif

	//! Firmware Upgrade Setup
	APP_FWUgradeSetup();

	//! System initialization
	DMAC_Init();
	PAIR_Init(&APP_EventQueue);

	ulBUF_StartAddr  = ulMMU_GetBufStartAddr();
	//! UI Buffer Setup
	ulBUF_StartAddr += ulUI_BufSetup(ulBUF_StartAddr);

	//! Kernel / Buffer initialization
	KNL_Init();
	BUF_Init(ulBUF_StartAddr);

	//! Kernel Parameter Setup
	APP_KNLParamSetup();

	//! Rate Control Preset Setup
#ifdef S2019A
	RC_PresetSetup(RC_QTY_AND_DYNAFPS);
#else
//#ifdef SU5390
//	if(tBB_GetDataRateMode() == BB_2M_DATA_RATE)
//	{
//		printf("2M Mode\r\n");
//	}
//	else
//	{
//		printf("4M Mode\r\n");
//	}
//#endif
	RC_PresetSetup(RC_QTY_AND_BW);	//RC_QTY_AND_BW  RC_QTY_THEN_FPS
	printf("RC_PresetSetup\r\n");
#endif

	//! Video / Audio initialization
	VDO_Init();
	ADO_Init();

	//! Kernel Buffer Calculate
	KNL_BufInit();

	//! UI Plug-in
	UI_PlugIn();

	//! Application Start
	APP_Start();
}
//------------------------------------------------------------------------------
void APP_StateFlowCtrl(APP_EventMsg_t *ptEventMsg)
{
	const static APP_StateFunc_t tAppStateFunc[] =
	{
		[APP_POWER_OFF_STATE] 	= APP_PowerCtrlFunc,
		[APP_IDLE_STATE] 		= APP_IdleStateFunc,
		[APP_LINK_STATE] 		= APP_LinkStateFunc,
		[APP_LOSTLINK_STATE] 	= APP_LostLinkStateFunc,
		[APP_PAIRING_STATE] 	= APP_PairingStateFunc,
	};
	if(tAppStateFunc[tAPP_StsReport.tAPP_State].pvFuncPtr)
		tAppStateFunc[tAPP_StsReport.tAPP_State].pvFuncPtr(ptEventMsg);
}
//------------------------------------------------------------------------------
void APP_PowerCtrlFunc(APP_EventMsg_t *ptEventMsg)
{
	ulAPP_WaitTickTime = osWaitForever;
	switch(tAPP_StsReport.tAPP_State)
	{
		case APP_POWER_OFF_STATE:
			APP_PowerOnFunc();
			break;
		default:
			tAPP_StsReport.tAPP_State = APP_POWER_OFF_STATE;
			break;
	}
}
//------------------------------------------------------------------------------
void APP_IdleStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_LINKSTATUS_REPORT_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_LINKSTS_RPT;
			tAPP_StsReport.tAPP_State = (APP_UpdateLinkStatus() == APP_LINK_EVENT)?APP_LINK_STATE:APP_LOSTLINK_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
		case APP_PAIRING_START_EVENT:
			APP_doPairingStart(ptEventMsg->ubAPP_Message);
			tAPP_StsReport.tAPP_State = APP_PAIRING_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
	#ifdef VBM_PU
		case APP_UNBIND_CAM_EVENT:
			APP_doUnbindBU(ptEventMsg);
			break;
		case APP_VIEWTYPECHG_EVENT:
			APP_SwitchViewTypeExec(ptEventMsg);
			break;
	#endif
		case APP_POWERSAVE_EVENT:
			APP_PowerSaveExec(ptEventMsg);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_LinkStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_LINKSTATUS_REPORT_EVENT:
			#ifdef VBM_BU
			/*H264_SetQp(ENCODE_0,40,40);
			H264_SetQp(ENCODE_1,40,40);
			H264_SetQp(ENCODE_2,40,40);
			H264_SetQp(ENCODE_3,40,40);		
			printf(">> H264_SetQp: 40\n"); */			
			#endif
		
			tAPP_StsReport.tAPP_ReportType = APP_LINKSTS_RPT;
			tAPP_StsReport.tAPP_State = (APP_UpdateLinkStatus() == APP_LINK_EVENT)?APP_LINK_STATE:APP_LOSTLINK_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
		case APP_PAIRING_START_EVENT:
			APP_doPairingStart(ptEventMsg->ubAPP_Message);
			tAPP_StsReport.tAPP_State = APP_PAIRING_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
	#ifdef VBM_PU
		case APP_UNBIND_CAM_EVENT:
			APP_doUnbindBU(ptEventMsg);
			break;
		case APP_VIEWTYPECHG_EVENT:
			APP_SwitchViewTypeExec(ptEventMsg);
			break;
		case APP_ADOSRCSEL_EVENT:
		{
			KNL_ROLE tKNL_Role = tAPP_STANumTable[ptEventMsg->ubAPP_Message[1]].tKNL_StaNum;
			uint8_t ubUpdFlag  = ptEventMsg->ubAPP_Message[2];

			tAPP_KNLInfo.tAdoSrcRole = tKNL_Role;
			if(tAPP_KNLInfo.tAdoSrcRole<=KNL_STA4)
			{
				ADO_RstRcvTimeLatency(tAPP_KNLInfo.tAdoSrcRole);
			}
			ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
			if(TRUE == ubUpdFlag)
				APP_UpdateKNLSetupInfo();
			break;
		}
		case APP_PTT_EVENT:
		{
			uint8_t ubAPP_PttFlag = ptEventMsg->ubAPP_Message[1];
			ADO_PttFuncPtr_t tAPP_PttFunc[] = {ADO_PTTStop, ADO_PTTStart};

			if(tAPP_PttFunc[ubAPP_PttFlag].ADO_tPttFunPtr)
				tAPP_PttFunc[ubAPP_PttFlag].ADO_tPttFunPtr();
			break;
		}
	#endif
		case APP_POWERSAVE_EVENT:
			APP_PowerSaveExec(ptEventMsg);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_LostLinkStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_LINKSTATUS_REPORT_EVENT:
			#ifdef VBM_BU
			/*H264_SetQp(ENCODE_0,50,50);
			H264_SetQp(ENCODE_1,50,50);
			H264_SetQp(ENCODE_2,50,50);
			H264_SetQp(ENCODE_3,50,50);		
			printf(">> H264_SetQp: 50\n"); */	
			#endif
		
			tAPP_StsReport.tAPP_ReportType = APP_LINKSTS_RPT;
			tAPP_StsReport.tAPP_State = (APP_UpdateLinkStatus() == APP_LINK_EVENT)?APP_LINK_STATE:APP_LOSTLINK_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
		case APP_PAIRING_START_EVENT:
			APP_doPairingStart(ptEventMsg->ubAPP_Message);
			tAPP_StsReport.tAPP_State = APP_PAIRING_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
	#ifdef VBM_PU
		case APP_UNBIND_CAM_EVENT:
			APP_doUnbindBU(ptEventMsg);
			break;
	#endif
		case APP_POWERSAVE_EVENT:
			APP_PowerSaveExec(ptEventMsg);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_PairingStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_PAIRING_STOP_EVENT:
			PAIR_Stop();
		case APP_PAIRING_FAIL_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_PAIRSTS_RPT;
			tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
			tAPP_StsReport.ubAPP_Report[0] = rFAIL;
		#ifdef VBM_PU
			tAPP_StsReport.ubAPP_Report[2] = tAPP_PairRoleInfo.ubAppUpdUiStsFlag;
		#endif
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			printd(DBG_Debug1Lvl, (APP_PAIRING_STOP_EVENT == ptEventMsg->ubAPP_Event)?"Pairing_stop\r\n":"Pairing_fail\r\n");
			break;
		case APP_PAIRING_SUCCESS_EVENT:
		{
		#ifdef VBM_PU
			UI_CamNum_t tAdoSrcCamNum;
		#endif
			tAPP_StsReport.tAPP_ReportType = APP_PAIRSTS_RPT;
			tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
			tAPP_StsReport.ubAPP_Report[0] = rSUCCESS;
		#if (defined(VBM_PU))
			KNL_ResetLcdChannel();
			APP_KNLRoleMap2CamNum(tAPP_KNLInfo.tAdoSrcRole, tAdoSrcCamNum);
			tAPP_StsReport.ubAPP_Report[1] = tAdoSrcCamNum;
			tAPP_StsReport.ubAPP_Report[2] = tAPP_PairRoleInfo.ubAppUpdUiStsFlag;
			if(TRUE == tAPP_PairRoleInfo.ubAppUpdUiStsFlag)
			{
				tAPP_StsReport.ubAPP_Report[3] = tAPP_PairRoleInfo.tPairBURole;
				tAPP_StsReport.ubAPP_Report[4] = tAPP_PairRoleInfo.tAppDispLoc;
			}
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			if((UI_CamNum_t)tAPP_StsReport.ubAPP_Report[1] != tAdoSrcCamNum)
			{
				tAdoSrcCamNum = (UI_CamNum_t)tAPP_StsReport.ubAPP_Report[1];
				tAPP_KNLInfo.tAdoSrcRole = APP_GetSTANumMappingTable(tAdoSrcCamNum)->tKNL_StaNum;
			}
			tAPP_KNLInfo.tBURoleInfo[tAPP_PairRoleInfo.tPairBURole].tKNL_DispLoc = tAPP_PairRoleInfo.tPairBUDispLoc;
			VDO_DisplayLocationSetup(tAPP_PairRoleInfo.tPairBURole, tAPP_PairRoleInfo.tPairBUDispLoc);
			VDO_UpdateDisplayParameter();
			if(APP_UNBIND_CAM_EVENT == ptEventMsg->ubAPP_Message[2])
			{
				UI_CamNum_t tDelCam;

				tAPP_StsReport.tAPP_ReportType = APP_PAIRUDBU_PRT;
				tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
				APP_PairTagMap2CamNum((PAIR_TAG)ptEventMsg->ubAPP_Message[1], tDelCam);
				tAPP_StsReport.ubAPP_Report[0] = tDelCam;
				tAPP_StsReport.ubAPP_Report[1] = FALSE;
				UI_UpdateAppStatus(&tAPP_StsReport);
				tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			}
		#elif (defined(VBM_BU))
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			tAPP_KNLInfo.tKNL_Role   = tAPP_STANumTable[PAIR_GetStaNumber()].tKNL_StaNum;
			tAPP_KNLInfo.tAdoSrcRole = tAPP_KNLInfo.tKNL_Role;
			VDO_KNLSysInfoSetup(tAPP_KNLInfo.tKNL_Role);
			ADO_KNLSysInfoSetup(tAPP_KNLInfo.tKNL_Role);
		#endif
			APP_UpdateKNLSetupInfo();
			break;
		}
		default:
			return;
	}
	VDO_Start();
	ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
}
//------------------------------------------------------------------------------
void APP_doPairingStart(void *pvPairInfo)
{
#if (defined(VBM_PU))
	uint8_t *pAPP_PairInfo 				= (uint8_t *)pvPairInfo;
	PAIR_TAG tPair_Tag 					= tAPP_STANumTable[pAPP_PairInfo[1]].tPAIR_StaNum;
	tAPP_PairRoleInfo.tPairBURole		= tAPP_STANumTable[pAPP_PairInfo[1]].tKNL_StaNum;
	tAPP_PairRoleInfo.tPairBUDispLoc	= tAPP_DispLocMap[pAPP_PairInfo[2]].tKNL_DispLocation;
	tAPP_PairRoleInfo.ubAppUpdUiStsFlag = pAPP_PairInfo[3];
	tAPP_PairRoleInfo.tAppDispLoc 		= (UI_DisplayLocation_t)pAPP_PairInfo[2];

	if(TRUE == tAPP_PairRoleInfo.ubAppUpdUiStsFlag)
	{
		tAPP_StsReport.tAPP_ReportType = APP_DISPPAIRICON_RPT;
		UI_UpdateAppStatus(&tAPP_StsReport);
		tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
	}
#elif (defined(VBM_BU))
	PAIR_TAG tPair_Tag = PAIR_AP_ASSIGN;
#if (APP_REC_FUNC_ENABLE && APP_PLAY_REMOTE_ENABLE)
    if(KNL_VIDEO_PLAY == tKNL_GetRecordFunc() && UI_ChkRemotePlayTrigger() == 0)
    {
        UI_RemotePlayTrigger(1);
        UI_RemotePlayStopTwc();
        KNL_TXRecordResume(KNL_TXREC_BUSY_PLAYSTOPVDO,1);
        KNL_VideoPlayStop();
    }
#endif
#endif
	ADO_Stop();
	VDO_Stop();
	PAIR_Start(tPair_Tag, APP_PAIRING_TIMEOUT, 0);
}
//------------------------------------------------------------------------------
#ifdef VBM_PU
void APP_doUnbindBU(APP_EventMsg_t *ptEventMsg)
{
	PAIR_TAG tPair_Tag = tAPP_STANumTable[ptEventMsg->ubAPP_Message[1]].tPAIR_StaNum;
	KNL_ROLE tKNL_Role = tAPP_STANumTable[ptEventMsg->ubAPP_Message[1]].tKNL_StaNum;

	if((tPair_Tag > PAIR_STA4) || (tKNL_Role > KNL_STA4))
		return;
	PAIR_DeleteTxId(tPair_Tag);
	VDO_RemoveDataPath(tKNL_Role);
	ADO_RemoveDataPath(tKNL_Role);
	tAPP_KNLInfo.tBURoleInfo[tKNL_Role].tKNL_DispLoc = KNL_DISP_LOCATION_ERR;
	tAPP_KNLInfo.tAdoSrcRole = (tAPP_KNLInfo.tAdoSrcRole == tKNL_Role)?KNL_NONE:tAPP_KNLInfo.tAdoSrcRole;
	APP_UpdateKNLSetupInfo();
}
#endif
//------------------------------------------------------------------------------
uint8_t APP_UpdateLinkStatus(void)
{
#if (defined(VBM_PU))
	uint8_t ubAPP_Event = APP_LOSTLINK_EVENT;
	KNL_ROLE ubKNL_RoleNum;

	for(ubKNL_RoleNum = KNL_STA1; ubKNL_RoleNum <= KNL_STA4; ubKNL_RoleNum++)
	{
		tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum] 	 = rLOSTLINK;
		tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+4] = 0;
		tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+8] = 0;
		if(ubKNL_GetCommLinkStatus(ubKNL_RoleNum) == BB_LINK)
		{
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum]   = rLINK;
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+4] = KNL_GetPerValue(ubKNL_RoleNum);
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+8] = KNL_GetRssiValue(ubKNL_RoleNum);
			ubAPP_Event = APP_LINK_EVENT;
		}
	}
	return ubAPP_Event;
#elif (defined(VBM_BU))
	return (ubKNL_GetCommLinkStatus(KNL_MASTER_AP) == BB_LINK)?APP_LINK_EVENT:APP_LOSTLINK_EVENT;
#endif
}
//------------------------------------------------------------------------------
APP_StaNumMap_t *APP_GetSTANumMappingTable(UI_CamNum_t tCamNum)
{
	return (APP_StaNumMap_t*)&tAPP_STANumTable[tCamNum];
}
//------------------------------------------------------------------------------
void APP_LoadKNLSetupInfo(void)
{
	uint32_t ulAPP_KNLInfoSFAddr = pSF_Info->ulSize - (KNL_SF_START_SECTOR * pSF_Info->ulSecSize);

	tAPP_KNLInfo.tAdoSrcRole = KNL_NONE;
	osMutexWait(APP_UpdateMutex, osWaitForever);
	SF_Read(ulAPP_KNLInfoSFAddr, sizeof(APP_KNLInfo_t), (uint8_t *)&tAPP_KNLInfo);
	osMutexRelease(APP_UpdateMutex);
	printd(DBG_InfoLvl, "KNL TAG:%s\n",tAPP_KNLInfo.cbKNL_InfoTag);
	printd(DBG_InfoLvl, "KNL VER:%s\n",tAPP_KNLInfo.cbKNL_FwVersion);
#if (defined(VBM_BU))
	if ((strncmp(tAPP_KNLInfo.cbKNL_InfoTag, SF_STA_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1) != 0)
	|| (strncmp(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1) != 0)) {
		printd(DBG_ErrorLvl, "TAG no match, Reset KNL\n");
		tAPP_KNLInfo.tUsbdClassMode = USBD_DEFU_CLASS;
		tAPP_KNLInfo.tTuningMode = APP_TUNINGMODE_OFF;
		return;
	}
	if(tAPP_KNLInfo.tUsbdClassMode >= USBD_UNKNOWN_MODE)
		tAPP_KNLInfo.tUsbdClassMode = USBD_MSC_MODE;
	if(tAPP_KNLInfo.tTuningMode > APP_TUNINGMODE_ON)
		tAPP_KNLInfo.tTuningMode = APP_TUNINGMODE_OFF;
	tAPP_KNLInfo.tUsbdClassMode = (!USBD_ENABLE)?USBD_MSC_MODE:USBD_DEFU_CLASS;
	if(APP_TUNINGMODE_ON == tAPP_KNLInfo.tTuningMode)
		tAPP_KNLInfo.tUsbdClassMode = USBD_UVC_MODE;
#elif (defined(VBM_PU))
	tAPP_KNLInfo.tTuningMode    = APP_TUNINGMODE_OFF;
	tAPP_KNLInfo.tUsbdClassMode = (!USBD_ENABLE)?USBD_MSC_MODE:USBD_DEFU_CLASS;
#endif
}
//------------------------------------------------------------------------------
void APP_UpdateKNLSetupInfo(void)
{
	uint32_t ulAPP_KNLInfoSFAddr;

	osMutexWait(APP_UpdateMutex, osWaitForever);
	ulAPP_KNLInfoSFAddr 		= pSF_Info->ulSize - (KNL_SF_START_SECTOR * pSF_Info->ulSecSize);
#ifdef OP_AP
	memcpy(tAPP_KNLInfo.cbKNL_InfoTag, SF_AP_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1);
#else
	memcpy(tAPP_KNLInfo.cbKNL_InfoTag, SF_STA_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1);
#endif
	memcpy(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1);
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulAPP_KNLInfoSFAddr, pSF_Info->ulSecSize, 1);
	SF_Write(ulAPP_KNLInfoSFAddr, sizeof(APP_KNLInfo_t), (uint8_t *)&tAPP_KNLInfo);	
	SF_EnableWrProtect();
	osMutexRelease(APP_UpdateMutex);
}
//------------------------------------------------------------------------------
void APP_KNLParamSetup(void)
{
#if (defined(VBM_PU))
	KNL_ROLE tKNL_BURole;
	UI_CamNum_t tCamNum;

	KNL_SetRole((tAPP_KNLInfo.tKNL_Role = KNL_MASTER_AP));
	if ((strncmp(tAPP_KNLInfo.cbKNL_InfoTag,   SF_AP_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1) == 0) &&
		(strncmp(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1) == 0))
	{
		for(tCamNum = CAM1; tCamNum < DISPLAY_MODE; tCamNum++)
		{
			tKNL_BURole = tAPP_STANumTable[tCamNum].tKNL_StaNum;
			VDO_DisplayLocationSetup(tKNL_BURole, tAPP_KNLInfo.tBURoleInfo[tKNL_BURole].tKNL_DispLoc);
		}
	}
	if(tAPP_KNLInfo.tAdoSrcRole > KNL_STA4)
		tAPP_KNLInfo.tAdoSrcRole = KNL_STA1;
#elif (defined(VBM_BU))
	if(USBD_UVC_MODE == tAPP_KNLInfo.tUsbdClassMode)
	{
		APP_TuningFuncPtr_t tAPP_TuningFunc[] = {[APP_TUNINGMODE_OFF] = KNL_TurnOffTuningTool,
												 [APP_TUNINGMODE_ON]  = KNL_TurnOnTuningTool};
		tAPP_TuningFunc[tAPP_KNLInfo.tTuningMode].pvAPP_TuningFunc();
	}
	if ((strncmp(tAPP_KNLInfo.cbKNL_InfoTag,   SF_STA_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1) == 0) &&
		(strncmp(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1) == 0))
	{
		tAPP_KNLInfo.tKNL_Role   = (tAPP_KNLInfo.tKNL_Role <= KNL_STA4)?tAPP_KNLInfo.tKNL_Role:KNL_NONE;
	}
	else
	{
		tAPP_KNLInfo.tKNL_Role   = KNL_NONE;
	}
	tAPP_KNLInfo.tAdoSrcRole = tAPP_KNLInfo.tKNL_Role;
	KNL_SetRole(tAPP_KNLInfo.tKNL_Role);
#endif
	tAPP_KNLInfo.tKNL_OpMode = (DISPLAY_MODE ==	DISPLAY_4T1R)?KNL_OPMODE_VBM_4T:
	                           (DISPLAY_MODE ==	DISPLAY_2T1R)?KNL_OPMODE_VBM_2T:
							   (DISPLAY_MODE ==	DISPLAY_1T1R)?KNL_OPMODE_VBM_1T:KNL_OPMODE_VBM_4T;
	KNL_SetOpMode(tAPP_KNLInfo.tKNL_OpMode);
}
//------------------------------------------------------------------------------
void APP_FWUgradeStatusReport(uint8_t ubStsReport)
{
#define PROGRESS_BAR_SCALE	25
	APP_StatusReport_t tAPP_UpgStsReport;

	tAPP_UpgStsReport.ubAPP_Report[0] = ubStsReport;
	tAPP_UpgStsReport.ubAPP_Report[1] = PROGRESS_BAR_SCALE;
	UI_UpdateFwUpgStatus(&tAPP_UpgStsReport);
	switch(ubStsReport)
	{
		case FWU_UPG_INPROGRESS:
		{
		#if (defined(VBM_BU))
			KNL_WakeupDevice(KNL_MASTER_AP, FALSE);
		#elif (defined(VBM_PU))
			UI_CamNum_t tCamNum;
			KNL_ROLE tKnlRole;

			for(tCamNum = CAM1; tCamNum < DISPLAY_MODE; tCamNum++)
			{
				tKnlRole = tAPP_STANumTable[tCamNum].tKNL_StaNum;
				KNL_WakeupDevice(tKnlRole, FALSE);
			}
		#endif
			UI_StopUpdateThread();
			ADO_Stop();
			VDO_Stop();
			break;
		}
		case FWU_UPG_SUCCESS:
#if APP_PC_CONNECT_EN
			if(ubKNL_GetPcConnSdFwuStatus()==0)
#endif
			{
				printd(DBG_ErrorLvl, "APP FWU->system reboot\n");
				SYS_Reboot();
			}
			break;
		case FWU_UPG_DEVTAG_FAIL:
		case FWU_UPG_FAIL:
			VDO_Start();
			ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
			UI_StartUpdateThread();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_FWUgradeSetup(void)
{
	FWU_MODE_t tAPP_FwuMode = FWU_USBDMSC;
	char *pFW_Ver = SN937XX_FW_VERSION, *p, *q;
	FWU_MSCParam_t tAPP_FWUParam = {{0},{0},{0}, NULL, NULL};
	FWU_SDParam_t  tAPP_FWUSdParam;

	strncpy(tAPP_FWUParam.cVolumeLable, SN937XX_VOLUME_LABLE, sizeof(tAPP_FWUParam.cVolumeLable));
	for(p=pFW_Ver; (q=strchr(p, '.'))!=NULL; p=q+1)
		p = q;
	strncpy(tAPP_FWUParam.cFileName, pFW_Ver, (p - pFW_Ver - 1));
	strncpy(tAPP_FWUParam.cFileNameExt, ((strlen(pFW_Ver) - (p - pFW_Ver)) > 3)?(p + 1):p, (strlen(pFW_Ver) - (p - pFW_Ver)));
#if !USBD_ENABLE
	if(FWU_USBDMSC == tAPP_FwuMode)
		tAPP_FwuMode = FWU_DISABLE;
#endif
	//! Setup check function for Model name or device tag.
	FWU_EnChkUpgFwFunc(FALSE, FALSE);
	//! USBD UPG Parameter Setup
	tAPP_FWUParam.pStsRptCbFunc = APP_FWUgradeStatusReport;
	tAPP_FWUParam.pVendorCmdCbFunc = NULL;
	FWU_Setup(tAPP_FwuMode, &tAPP_FWUParam);
	//! SD UPG Parameter Setup
	tAPP_FWUSdParam.ubTargetFileNameLen = 9;
	strncpy(tAPP_FWUSdParam.cTargetFileName, "SN937XXFW", tAPP_FWUSdParam.ubTargetFileNameLen);
	tAPP_FWUSdParam.pStsRptCbFunc = APP_FWUgradeStatusReport;
	tAPP_FWUSdParam.ubIncrementProgressBy = PROGRESS_BAR_SCALE;
	FWU_Setup(FWU_SDCARD, &tAPP_FWUSdParam);
	FWU_Enable();
}
//------------------------------------------------------------------------------
#ifdef VBM_PU
void APP_ViewTypeSetup(UI_CamViewType_t tAPP_ViewType)
{
	APP_EventMsg_t *ptViewTypeParam;	
	KNL_DISP_TYPE tAPP_KnlDispType;
	KNL_ROLE tAPP_KnlRole[3];

	ptViewTypeParam = tUI_ViewTypeSetup(tAPP_ViewType);
	if(NULL == ptViewTypeParam)
		return;
	tAPP_KnlRole[0] = (ptViewTypeParam->ubAPP_Message[0] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[0]].tKNL_StaNum:KNL_NONE;
	tAPP_KnlRole[1] = (ptViewTypeParam->ubAPP_Message[1] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[1]].tKNL_StaNum:KNL_NONE;
	tAPP_KnlRole[2] = (ptViewTypeParam->ubAPP_Message[2] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[2]].tKNL_StaNum:KNL_NONE;
	if(tAPP_KNLInfo.tAdoSrcRole != tAPP_KnlRole[0])
		tAPP_KNLInfo.tAdoSrcRole = tAPP_KnlRole[0];
	tAPP_KnlDispType = (tAPP_ViewType == SINGLE_VIEW)?KNL_DISP_SINGLE:
					   (tAPP_ViewType == TRIPLE_2L1R_VIEW)?KNL_DISP_3T_2L1R:(tAPP_ViewType == TRIPLE_1L2R_VIEW)?KNL_DISP_3T_1L2R:
	                   (tAPP_ViewType == TRIPLE_2T1B_VIEW)?KNL_DISP_3T_2T1B:(tAPP_ViewType == TRIPLE_1T2B_VIEW)?KNL_DISP_3T_1T2B:
					   (tAPP_ViewType == TRIPLE_3COL_VIEW)?KNL_DISP_3T_3COL:(tAPP_ViewType == DUAL_VIEW)?KNL_DISP_DUAL_C:KNL_DISP_QUAD;
	VDO_SwitchDisplayType(tAPP_KnlDispType, tAPP_KnlRole);
}
//------------------------------------------------------------------------------
void APP_SwitchViewTypeExec(APP_EventMsg_t *ptEventMsg)
{
	KNL_ROLE tKNL_Role[3];
	KNL_DISP_TYPE tKNL_DispType;
	UI_CamViewType_t tAPP_CamView;
	uint8_t i, ubBuNum = 2;

    UI_SwitchViewTypeFg(1);
	tAPP_CamView  = (UI_CamViewType_t)ptEventMsg->ubAPP_Message[1];
	tKNL_DispType = ((tAPP_CamView == SINGLE_VIEW) || (tAPP_CamView == SCAN_VIEW))?KNL_DISP_SINGLE:
	                 (tAPP_CamView == TRIPLE_2L1R_VIEW)?KNL_DISP_3T_2L1R:(tAPP_CamView == TRIPLE_1L2R_VIEW)?KNL_DISP_3T_1L2R:
	                 (tAPP_CamView == TRIPLE_2T1B_VIEW)?KNL_DISP_3T_2T1B:(tAPP_CamView == TRIPLE_1T2B_VIEW)?KNL_DISP_3T_1T2B:
				     (tAPP_CamView == TRIPLE_3COL_VIEW)?KNL_DISP_3T_3COL:(tAPP_CamView == DUAL_VIEW)?KNL_DISP_DUAL_C:KNL_DISP_QUAD;
	ubBuNum = (IS_UI_DISP3T_VIEW(tAPP_CamView))?3:2;
	for(i = 0; i < ubBuNum; i++)
		tKNL_Role[i] = (ptEventMsg->ubAPP_Message[2+i] <= CAM4)?tAPP_STANumTable[ptEventMsg->ubAPP_Message[2+i]].tKNL_StaNum:KNL_NONE;
	tAPP_StsReport.tAPP_ReportType = APP_VWMODESTS_RPT;
	tAPP_StsReport.ubAPP_Report[0] = tAPP_CamView;

	if ((tAPP_CamView == SINGLE_VIEW) || (tAPP_CamView == SCAN_VIEW)){
		tAPP_KNLInfo.tAdoSrcRole = tAPP_STANumTable[tKNL_Role[0]].tKNL_StaNum;
		ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
	}
	VDO_SwitchDisplayType(tKNL_DispType, tKNL_Role);

	UI_UpdateAppStatus(&tAPP_StsReport);
	tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
	UI_SwitchViewTypeFg(0);
}
//------------------------------------------------------------------------------
void APP_LcdDisplayOff(void)
{
	switch(LCD_PM)
	{
		case LCD_PWR_OFF:
			LCD_UnInit();
			LCD->LCD_MODE = LCD_GPIO;
			GLB->LCD_FUNC_DIS  = 1;
			SSP->SSP_GPIO_MODE = 1;
			LCD_PWR_DISABLE;
			break;
		case LCD_PM_SUSPEND:
			LCD_Suspend();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_LcdDisplayOn(void)
{
	switch(LCD_PM)
	{
		case LCD_PWR_OFF:
			LCD_PWR_ENABLE;
			osDelay(200);
			LCD_RESET(10);
			GLB->LCD_FUNC_DIS = 0;
			LCD_Init(LCD_LCD_PANEL);
			VDO_UpdateDisplayParameter();
			LCD_Start();
			break;
		case LCD_PM_SUSPEND:
			LCD_Resume();
			break;
		default:
			return;
	}
	LCDBL_ENABLE(UI_ENABLE);
}
#endif
//------------------------------------------------------------------------------
void APP_PowerSaveExec(APP_EventMsg_t *ptEventMsg)
{
	UI_PowerSaveMode_t tAPP_PsMode = (UI_PowerSaveMode_t)ptEventMsg->ubAPP_Message[1];
#ifdef VBM_PU
	UI_CamNum_t tPsCamNum          = (UI_CamNum_t)ptEventMsg->ubAPP_Message[3];
	KNL_ROLE tKNL_Role;
#endif
	switch(tAPP_PsMode)
	{
		case PS_VOX_MODE:
		{
			VDO_PsFuncPtr_t tAPP_VoxFunc[] = {VDO_Start, VDO_Stop};
		#ifdef VBM_PU
			APP_ActFuncPtr_t tAPP_LcdFunc[] = {APP_LcdDisplayOn, APP_LcdDisplayOff};
			SYS_PowerState_t tAPP_PsState[]	= {SYS_PS0, SYS_PS1};
		#endif
			uint8_t ubAPP_PsFlag = ptEventMsg->ubAPP_Message[2];
#ifdef S2019A
			if(ubAPP_PsFlag)
				sPRF_SetTxBlockLvl(sPRF_BLK_AV);
		#ifdef VBM_PU
			else
				sPRF_SetTxBlockLvl(sPRF_ACCP_ALL);
		#endif
#endif
			if(tAPP_VoxFunc[ubAPP_PsFlag].VDO_tPsFunPtr)
				tAPP_VoxFunc[ubAPP_PsFlag].VDO_tPsFunPtr();
		#ifdef VBM_PU
			SYS_SetPowerStates(tAPP_PsState[ubAPP_PsFlag]);
			SIGNAL_LED_IO_ENABLE = (!ubAPP_PsFlag)?TRUE:FALSE;
			tAPP_StsReport.tAPP_ReportType = APP_VOXMODESTS_RPT;
			tAPP_StsReport.ubAPP_Report[0] = ubAPP_PsFlag;
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			if(tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr)
				tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr();
		#endif
			break;
		}
#ifdef A7130
		case PS_ECO_MODE:
		#if (defined(VBM_BU) && defined(BSP_SN93710_FHD_REC_TX_V4_1))
			VDO_ChangePlayState(tAPP_KNLInfo.tKNL_Role, VDO_STOP);
			KNL_EnableWORFunc();
			break;
		#elif (defined(VBM_PU))
		{
			VDO_PlayState_t tAPP_VdoPlySte = (TRUE == ptEventMsg->ubAPP_Message[4])?VDO_START:VDO_STOP;

			tKNL_Role = tAPP_STANumTable[tPsCamNum].tKNL_StaNum;
			VDO_ChangePlayState(tKNL_Role, tAPP_VdoPlySte);
			if((BB_ENABLE_ALL_STA_WAKEUP == tKNL_GetWORMode()) && (FALSE == ptEventMsg->ubAPP_Message[2]))
				break;
			KNL_WakeupDevice(tKNL_Role, ptEventMsg->ubAPP_Message[2]);
			break;
		}
		#endif
		case PS_WOR_MODE:
		#if (defined(VBM_BU))
		{
			VDO_PsFuncPtr_t tAPP_WorFunc[] = {VDO_Stop, VDO_Start};
			uint8_t ubAPP_PsFlag = ptEventMsg->ubAPP_Message[2];
			uint8_t ubAPP_VdoActFlag = ptEventMsg->ubAPP_Message[4];

			if((TRUE == ubAPP_VdoActFlag) &&
			   (tAPP_WorFunc[ubAPP_PsFlag].VDO_tPsFunPtr))
				tAPP_WorFunc[ubAPP_PsFlag].VDO_tPsFunPtr();
			KNL_WakeupDevice(KNL_MASTER_AP, ptEventMsg->ubAPP_Message[3]);
			break;
		}
		#elif (defined(VBM_PU))
			KNL_EnableWORFunc();
			break;
		#endif
#endif
#ifdef VBM_PU
		case PS_ADOONLY_MODE:
		{
			VDO_PsFuncPtr_t tAPP_AdoOnFunc[] = {VDO_Start, VDO_Stop};
			APP_ActFuncPtr_t tAPP_LcdFunc[]  = {LCD_Resume, LCD_Suspend};
			SYS_PowerState_t tAPP_PsState[]	 = {SYS_PS0, SYS_PS1};
			uint8_t ubAPP_PsFlag 			 = ptEventMsg->ubAPP_Message[2];
		#ifdef S2019A
			sPRF_SetTxBlockLvl((ubAPP_PsFlag)?sPRF_BLK_V:sPRF_ACCP_ALL);
		#endif
			KNL_SetTRXPathActivity();
			if(tAPP_AdoOnFunc[ubAPP_PsFlag].VDO_tPsFunPtr)
				tAPP_AdoOnFunc[ubAPP_PsFlag].VDO_tPsFunPtr();
			SYS_SetPowerStates(tAPP_PsState[ubAPP_PsFlag]);
			SIGNAL_LED_IO_ENABLE = (!ubAPP_PsFlag)?TRUE:FALSE;
			if(tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr)
				tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr();
			if(FALSE == ubAPP_PsFlag)
				LCDBL_ENABLE(UI_ENABLE);
			break;
		}
		case POWER_NORMAL_MODE:
			tKNL_Role = tAPP_STANumTable[tPsCamNum].tKNL_StaNum;
			KNL_WakeupDevice(tKNL_Role, FALSE);
			break;
#endif
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_SetTuningToolMode(APP_TuningMode_t tTuningMode)
{
	tAPP_KNLInfo.tTuningMode = tTuningMode;
	switch(tTuningMode)
	{
		case APP_TUNINGMODE_ON:
			tAPP_KNLInfo.tUsbdClassMode = USBD_UVC_MODE;
			KNL_TurnOnTuningTool();
			break;
		case APP_TUNINGMODE_OFF:
			tAPP_KNLInfo.tUsbdClassMode = USBD_DEFU_CLASS;
			KNL_TurnOffTuningTool();
			break;
		default:
			return;
	}
	APP_UpdateKNLSetupInfo();
}
//------------------------------------------------------------------------------
APP_TuningMode_t APP_GetTuningToolMode(void)
{
	return tAPP_KNLInfo.tTuningMode;
}
//------------------------------------------------------------------------------
void APP_Start(void)
{
#ifdef A7130
	//! WOR Mode Setup
	KNL_SetWORMode(BB_DISABLE_ALL_STA_WAKEUP);
#endif

	//! Kernel Setup
	KNL_BlockInit();

#ifdef VBM_PU
	//! Video View Type Setup
	APP_ViewTypeSetup((DISPLAY_MODE == DISPLAY_4T1R)?QUAD_VIEW:
					  (DISPLAY_MODE == DISPLAY_2T1R)?DUAL_VIEW:SINGLE_VIEW);
#endif

	//! Video Start
	VDO_Start();

	//! Audio Start
	ADO_Start(tAPP_KNLInfo.tAdoSrcRole);

	//! Two way command Start
	TWC_Start();

	tAPP_StsReport.tAPP_State = APP_IDLE_STATE;
	UI_UpdateAppStatus(&tAPP_StsReport);
}
