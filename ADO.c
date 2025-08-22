/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		ADO.c
	\brief		Audio Process for VBM
	\author		Hanyi Chiu
	\version	0.5
	\date		2020/05/04
	\copyright	Copyright(C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#if defined(VBM_PU) || defined(VBM_BU)

#include "ADO.h"
#include "BSP.h"

const static ADO_KNLRoleInfo_t tADO_KNLRoleInfo[] = 
{
	//! OTHER_A source : Record, OTHER_B source : Push talk
	[KNL_STA1] = {KNL_SRC_1_OTHER_A, KNL_SRC_1_OTHER_B},
	[KNL_STA2] = {KNL_SRC_2_OTHER_A, KNL_SRC_2_OTHER_B},
	[KNL_STA3] = {KNL_SRC_3_OTHER_A, KNL_SRC_3_OTHER_B},
	[KNL_STA4] = {KNL_SRC_4_OTHER_A, KNL_SRC_4_OTHER_B},
	[KNL_NONE] = {KNL_SRC_NONE, 	 KNL_SRC_NONE},
};
static KNL_ROLE	tADO_TargetRole;
uint8_t ubNrSetting;
uint8_t ubAecSetting;
//------------------------------------------------------------------------------
void ADO_GetNrSettingFromUiInfo(ADO_NOISE_PROCESS_TYPE UiNrSetting)
{
	ubNrSetting = UiNrSetting;
}
//------------------------------------------------------------------------------
void ADO_GetAecSettingFromUiInfo(ADO_NOISE_PROCESS_TYPE AecSetting)
{
	ubAecSetting = AecSetting;
}
//------------------------------------------------------------------------------
void ADO_Init(void)
{
	KNL_ROLE tADO_KNLRole = KNL_STA1;

	//! Audio Parameter Setting
	tADO_TargetRole = KNL_NONE;
	ADO_KNLParamSetup(SAMPLERATE_16kHZ, MONO);

	//! Data Path Setting
	KNL_AdoPathReset();
	KNL_SetAdoRoleInfoCbFunc(ADO_GetSourceNumber);

#ifdef VBM_BU
	tADO_KNLRole = (KNL_ROLE)ubKNL_GetRole();
	tADO_KNLRole = (KNL_NONE != tADO_KNLRole)?tADO_KNLRole:KNL_STA1;
	ADO_DataPathSetup(tADO_KNLRole, ADO_MAIN_SRC);
	ADO_DataPathSetup(tADO_KNLRole, ADO_PTT_SRC);
#endif
#ifdef VBM_PU
	for(tADO_KNLRole = KNL_STA1; tADO_KNLRole < DISPLAY_MODE; tADO_KNLRole++)
		ADO_DataPathSetup(tADO_KNLRole, ADO_MAIN_SRC);
	ADO_DataPathSetup(KNL_STA1, ADO_PTT_SRC);
	ADO_MixModeOFF();
#endif
}
//------------------------------------------------------------------------------
void ADO_DataPathSetup(KNL_ROLE tADO_KNLRole, ADO_SrcType_t tADO_SrcType)
{
#ifdef VBM_PU
	KNL_NODE tADO_KNLSrcMainPath[] = {KNL_NODE_COMM_RX_ADO, KNL_NODE_DAC_BUF, KNL_NODE_DAC, KNL_NODE_END};
	KNL_NODE tADO_KNLSrcPttPath[]  = {KNL_NODE_ADC, KNL_NODE_ADC_BUF, KNL_NODE_COMM_TX_ADO, KNL_NODE_END};
#endif
#ifdef VBM_BU
	KNL_NODE tADO_KNLSrcMainPath[] = {KNL_NODE_ADC, KNL_NODE_ADC_BUF, KNL_NODE_COMM_TX_ADO, KNL_NODE_END};
	KNL_NODE tADO_KNLSrcPttPath[]  = {KNL_NODE_COMM_RX_ADO, KNL_NODE_DAC_BUF, KNL_NODE_DAC, KNL_NODE_END};
#endif
	KNL_NODE *pADO_KNLPath[ADO_SRC_MAX] = {[ADO_MAIN_SRC] = tADO_KNLSrcMainPath,
	                                       [ADO_PTT_SRC]  = tADO_KNLSrcPttPath};
	KNL_NODE_INFO tADO_KNLNodeInfo = {0};
	KNL_SRC tADO_KNLSrcNum;
	uint16_t uwADO_NodeNum, i;

	tADO_KNLSrcNum = (tADO_SrcType == ADO_MAIN_SRC)?tADO_KNLRoleInfo[tADO_KNLRole].tKNL_SrcNum:tADO_KNLRoleInfo[tADO_KNLRole].tKNL_SubSrcNum;
	uwADO_NodeNum  = ((tADO_SrcType == ADO_MAIN_SRC)?(sizeof tADO_KNLSrcMainPath):(sizeof tADO_KNLSrcPttPath)) / sizeof(KNL_NODE);
	for(i = 0; i < uwADO_NodeNum; i++)
	{
		tADO_KNLNodeInfo.ubPreNode	= (i == 0)?KNL_NODE_NONE:pADO_KNLPath[tADO_SrcType][i-1];
		tADO_KNLNodeInfo.ubCurNode 	= pADO_KNLPath[tADO_SrcType][i];
		tADO_KNLNodeInfo.ubNextNode = pADO_KNLPath[tADO_SrcType][i+1];
		ubKNL_SetAdoPathNode(tADO_KNLSrcNum, i, tADO_KNLNodeInfo);
	}
}
//------------------------------------------------------------------------------
#ifdef VBM_BU
void ADO_KNLSysInfoSetup(KNL_ROLE tADO_KNLRole)
{
	KNL_AdoPathReset();
	ADO_DataPathSetup(tADO_KNLRole, ADO_MAIN_SRC);
	ADO_DataPathSetup(tADO_KNLRole, ADO_PTT_SRC);
}
#endif
//------------------------------------------------------------------------------
#ifdef VBM_PU
void ADO_RemoveDataPath(KNL_ROLE tADO_Role)
{
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_Role].tKNL_SrcNum);
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_Role].tKNL_SubSrcNum);
	if(tADO_TargetRole == tADO_Role)
		tADO_TargetRole = KNL_NONE;
}
//------------------------------------------------------------------------------
void ADO_PTTStart(void)
{	
	if(tADO_TargetRole > KNL_STA4)
		return;
	
	for(uint32_t i=0; i<ADO_AUDIO32_MAX_NUM; i++)
	{
		ADO_Ado32EncInit(i,ADO_GetAdo32EncFmt(i));
	}
#if(ADO_ENC_TYPE==AUDIO32_ENC)
	ADO_Noise_Process_Type(NOISE_NR, AEC_NR_16kHZ);
#elif(ADO_ENC_TYPE==HW_ALAW_ENC)
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);	
#elif(ADO_ENC_TYPE==SW_ALAW_ENC)
	ADO_Noise_Process_Type(NOISE_NR, AEC_NR_16kHZ);
#elif(ADO_ENC_TYPE==SW_AAC_ENC)
	ADO_Noise_Process_Type(NOISE_NR, AEC_NR_16kHZ);
#endif

	KNL_AdoStart(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
	printd(DBG_InfoLvl, "		=>PTT Play\n");
}
//------------------------------------------------------------------------------
void ADO_PTTStop(void)
{
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
	printd(DBG_InfoLvl, "		=>PTT Stop\n");
}
#endif
//------------------------------------------------------------------------------
KNL_SRC ADO_GetSourceNumber(KNL_VA_DATAPATH tADO_Path, KNL_ROLE tADO_KNLRole)
{
	return (KNL_MAIN_PATH == tADO_Path)?tADO_KNLRoleInfo[tADO_KNLRole].tKNL_SrcNum:(KNL_SUB_PATH == tADO_Path)?tADO_KNLRoleInfo[tADO_KNLRole].tKNL_SubSrcNum:KNL_SRC_NONE;
}
//------------------------------------------------------------------------------
void ADO_Start(KNL_ROLE tADO_Role)
{
	uint32_t i;

#ifdef VBM_PU
	KNL_ROLE tADO_KNLRole = KNL_STA1;
#endif

	if((tADO_Role > KNL_STA4) || (tADO_TargetRole == tADO_Role))
		return;

	if(KNL_NONE != tADO_TargetRole)
		ADO_Stop();
	tADO_TargetRole = tADO_Role;

	for(i=0; i<ADO_AUDIO32_MAX_NUM; i++)
	{
		ADO_Ado32EncInit(i,ADO_GetAdo32EncFmt(i));
	}

#ifdef VBM_PU
	KNL_AdoPathReset();
	for(tADO_KNLRole = KNL_STA1; tADO_KNLRole < DISPLAY_MODE; tADO_KNLRole++)
		ADO_DataPathSetup(tADO_KNLRole, ADO_MAIN_SRC);
	ADO_DataPathSetup(tADO_TargetRole, ADO_PTT_SRC);
#endif
	KNL_AdoStart(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SrcNum);
#ifdef VBM_BU
	KNL_AdoStart(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
#endif
}
//------------------------------------------------------------------------------
KNL_ROLE ADO_Stop(void)
{
	KNL_ROLE AdoRole;
	
	AdoRole = tADO_TargetRole;
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SrcNum);
#ifdef VBM_BU
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
#endif
	tADO_TargetRole = KNL_NONE;
	
	return AdoRole;
}
//------------------------------------------------------------------------------
void ADO_KNLParamSetup(ADO_SAMPLERATE SampleRate, ADO_CHANNEL_MODE Channel)
{
	ADO_KNL_PARA_t tADO_KNLParm;
	uint32_t i;
	
	//assign enc/dec cb function
	ADO_SetAlawEncDecCb(KNL_Alaw_Encode, KNL_Alaw_Decode);
	ADO_SetAdo32EncDecCb(KNL_Ado32EncInit, KNL_Ado32DecInit, KNL_Ado32_Encode, KNL_Ado32_Decode);
	ADO_SetAacEncDecCb(KNL_AAC_EncInit, KNL_AAC_DecInit, KNL_AAC_EncUnInit, KNL_AAC_DecUnInit, ulKNL_AAC_GetEncUnitSize, KNL_AAC_EncEncode, KNL_AAC_EncEncodeFlush, KNL_AAC_DecDecode);
	
#ifdef OP_STA
	tADO_KNLParm.ubSupSrcNum         = 1;
#else	
	tADO_KNLParm.ubSupSrcNum         = DISPLAY_MODE;	
#endif
	
	tADO_KNLParm.SysSpeed			 = HIGH_SPEED;
	
#if APP_ADO_AEC_NR_TYPE == AEC_NR_SW
#if (BSP_ADO_I2S_MODE_ENABLE==0)
	tADO_KNLParm.AdcDev				= SIG_DEL_ADC;
	tADO_KNLParm.DacDev				= R2R_DAC;
#elif (BSP_ADO_I2S_MODE_ENABLE==1)
	tADO_KNLParm.AdcDev			 	= I2S_ADC;
	tADO_KNLParm.DacDev			 	= I2S_DAC;
#endif
#elif APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	tADO_KNLParm.AdcDev				 = I2S_ADC;
	tADO_KNLParm.DacDev				 = I2S_DAC;
#endif

    tADO_KNLParm.SigDelAdcMode       = ADO_SIG_DIFFERENTIAL;
#if (BSP_ADO_I2S_MODE_ENABLE==1)
	tADO_KNLParm.I2S_Mode            = I2S_MODE_MASTER;
#endif
	tADO_KNLParm.AdcFmt.sign_flag   = SIGNED;
	tADO_KNLParm.AdcFmt.channel     = Channel;
	tADO_KNLParm.AdcFmt.sample_size = SAMPLESIZE_16_BIT;
	tADO_KNLParm.AdcFmt.sample_rate = SampleRate;
	tADO_KNLParm.DacFmt.sign_flag   = SIGNED;
	tADO_KNLParm.DacFmt.channel     = Channel;
	tADO_KNLParm.DacFmt.sample_size = SAMPLESIZE_16_BIT;
	tADO_KNLParm.DacFmt.sample_rate = SampleRate;
	
#if(ADO_ENC_TYPE==AUDIO32_ENC)
	tADO_KNLParm.HwCompressMode = COMPRESS_NONE;
#elif(ADO_ENC_TYPE==HW_ALAW_ENC)
	tADO_KNLParm.HwCompressMode = COMPRESS_ALAW;
#elif(ADO_ENC_TYPE==SW_ALAW_ENC)
	tADO_KNLParm.HwCompressMode = COMPRESS_NONE;
#elif(ADO_ENC_TYPE==SW_AAC_ENC)
	tADO_KNLParm.HwCompressMode = COMPRESS_NONE;
#endif
	
	//buffer size
	switch(SampleRate)
	{
		case SAMPLERATE_8kHZ:
			tADO_KNLParm.BufSz.Adc      = (Channel==MONO)?BUF_SIZE_4KB:BUF_SIZE_8KB;
			tADO_KNLParm.BufSz.Dac      = (Channel==MONO)?BUF_SIZE_2KB:BUF_SIZE_4KB;
			tADO_KNLParm.BufSz.PreHwPly = (Channel==MONO)?BUF_SIZE_32KB:BUF_SIZE_64KB;
			break;
		case SAMPLERATE_16kHZ:
			tADO_KNLParm.BufSz.Adc      = (Channel==MONO)?BUF_SIZE_8KB:BUF_SIZE_16KB;
			tADO_KNLParm.BufSz.Dac      = (Channel==MONO)?BUF_SIZE_4KB:BUF_SIZE_8KB;
			tADO_KNLParm.BufSz.PreHwPly = (Channel==MONO)?BUF_SIZE_64KB:BUF_SIZE_96KB;
			break;
		case SAMPLERATE_24kHZ:
			tADO_KNLParm.BufSz.Adc      = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			tADO_KNLParm.BufSz.Dac      = (Channel==MONO)?BUF_SIZE_8KB:BUF_SIZE_16KB;
			tADO_KNLParm.BufSz.PreHwPly = (Channel==MONO)?BUF_SIZE_96KB:BUF_SIZE_160KB;
			break;
		case SAMPLERATE_32kHZ:
			tADO_KNLParm.BufSz.Adc      = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			tADO_KNLParm.BufSz.Dac      = (Channel==MONO)?BUF_SIZE_8KB:BUF_SIZE_16KB;
			tADO_KNLParm.BufSz.PreHwPly = (Channel==MONO)?BUF_SIZE_96KB:BUF_SIZE_192KB;
			break;
		case SAMPLERATE_48kHZ:
			tADO_KNLParm.BufSz.Adc      = (Channel==MONO)?BUF_SIZE_32KB:BUF_SIZE_64KB;
			tADO_KNLParm.BufSz.Dac      = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			tADO_KNLParm.BufSz.PreHwPly = (Channel==MONO)?BUF_SIZE_160KB:BUF_SIZE_288KB;
			break;
		case SAMPLERATE_11_025kHZ:
			tADO_KNLParm.BufSz.Adc      = (Channel==MONO)?BUF_SIZE_8KB:BUF_SIZE_16KB;
			tADO_KNLParm.BufSz.Dac      = (Channel==MONO)?BUF_SIZE_4KB:BUF_SIZE_8KB;
			tADO_KNLParm.BufSz.PreHwPly = (Channel==MONO)?BUF_SIZE_64KB:BUF_SIZE_96KB;
			break;
		case SAMPLERATE_22_05kHZ:
			tADO_KNLParm.BufSz.Adc      = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			tADO_KNLParm.BufSz.Dac      = (Channel==MONO)?BUF_SIZE_8KB:BUF_SIZE_16KB;
			tADO_KNLParm.BufSz.PreHwPly = (Channel==MONO)?BUF_SIZE_96KB:BUF_SIZE_160KB;
			break;
		case SAMPLERATE_44_1kHZ:
			tADO_KNLParm.BufSz.Adc      = (Channel==MONO)?BUF_SIZE_32KB:BUF_SIZE_64KB;
			tADO_KNLParm.BufSz.Dac      = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			tADO_KNLParm.BufSz.PreHwPly = (Channel==MONO)?BUF_SIZE_160KB:BUF_SIZE_288KB;
			break;
	}
	tADO_KNLParm.BufSz.TempProc = tADO_KNLParm.BufSz.Adc;
	tADO_KNLParm.BufSz.DeHowling  = BUF_SIZE_16KB;
	tADO_KNLParm.BufSz.WavPlyTemp = BUF_SIZE_1KB;
	tADO_KNLParm.BufSz.WavPly     = BUF_SIZE_4KB;
	tADO_KNLParm.BufSz.Ado32SwEn  = BUF_SIZE_4KB;
	tADO_KNLParm.BufSz.Ado32SwDe  = BUF_SIZE_4KB;
#if(ADO_ENC_TYPE==SW_AAC_ENC)
	switch(SampleRate)
	{
		case SAMPLERATE_8kHZ:
			tADO_KNLParm.BufSz.AacSwEn = (Channel==MONO)?BUF_SIZE_4KB:BUF_SIZE_8KB;
			break;
		case SAMPLERATE_16kHZ:
			tADO_KNLParm.BufSz.AacSwEn = (Channel==MONO)?BUF_SIZE_8KB:BUF_SIZE_16KB;
			break;
		case SAMPLERATE_24kHZ:
			tADO_KNLParm.BufSz.AacSwEn = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			break;
		case SAMPLERATE_32kHZ:
			tADO_KNLParm.BufSz.AacSwEn = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			break;
		case SAMPLERATE_48kHZ:
			tADO_KNLParm.BufSz.AacSwEn = (Channel==MONO)?BUF_SIZE_32KB:BUF_SIZE_64KB;
			break;
		case SAMPLERATE_11_025kHZ:
			tADO_KNLParm.BufSz.AacSwEn = (Channel==MONO)?BUF_SIZE_8KB:BUF_SIZE_16KB;
			break;
		case SAMPLERATE_22_05kHZ:
			tADO_KNLParm.BufSz.AacSwEn = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			break;
		case SAMPLERATE_44_1kHZ:
			tADO_KNLParm.BufSz.AacSwEn = (Channel==MONO)?BUF_SIZE_32KB:BUF_SIZE_64KB;
			break;
	}
	tADO_KNLParm.BufSz.AacSwDe = tADO_KNLParm.BufSz.AacSwEn;
#else
	tADO_KNLParm.BufSz.AacSwEn    = BUF_SIZE_0KB;
	tADO_KNLParm.BufSz.AacSwDe    = BUF_SIZE_0KB;
#endif
#if APP_REC_FUNC_ENABLE
	tADO_KNLParm.BufSz.AlawSwEn   = BUF_SIZE_16KB;
	tADO_KNLParm.BufSz.AlawSwDe   = BUF_SIZE_16KB;
	tADO_KNLParm.BufSz.RecPkt     = BUF_SIZE_32KB;
	tADO_KNLParm.BufSz.RecEn      = BUF_SIZE_64KB;
#else
#if(ADO_ENC_TYPE==SW_ALAW_ENC)
	switch(SampleRate)
	{
		case SAMPLERATE_8kHZ:
			tADO_KNLParm.BufSz.AlawSwEn = (Channel==MONO)?BUF_SIZE_8KB:BUF_SIZE_16KB;
			break;
		case SAMPLERATE_16kHZ:
			tADO_KNLParm.BufSz.AlawSwEn = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			break;
		case SAMPLERATE_24kHZ:
			tADO_KNLParm.BufSz.AlawSwEn = (Channel==MONO)?BUF_SIZE_32KB:BUF_SIZE_64KB;
			break;
		case SAMPLERATE_32kHZ:
			tADO_KNLParm.BufSz.AlawSwEn = (Channel==MONO)?BUF_SIZE_32KB:BUF_SIZE_64KB;
			break;
		case SAMPLERATE_48kHZ:
			tADO_KNLParm.BufSz.AlawSwEn = (Channel==MONO)?BUF_SIZE_64KB:BUF_SIZE_128KB;
			break;
		case SAMPLERATE_11_025kHZ:
			tADO_KNLParm.BufSz.AlawSwEn = (Channel==MONO)?BUF_SIZE_16KB:BUF_SIZE_32KB;
			break;
		case SAMPLERATE_22_05kHZ:
			tADO_KNLParm.BufSz.AlawSwEn = (Channel==MONO)?BUF_SIZE_32KB:BUF_SIZE_64KB;
			break;
		case SAMPLERATE_44_1kHZ:
			tADO_KNLParm.BufSz.AlawSwEn = (Channel==MONO)?BUF_SIZE_64KB:BUF_SIZE_128KB;
			break;
	}
	tADO_KNLParm.BufSz.AlawSwDe   = tADO_KNLParm.BufSz.AlawSwEn;
#else
	tADO_KNLParm.BufSz.AlawSwEn   = BUF_SIZE_0KB;
	tADO_KNLParm.BufSz.AlawSwDe   = BUF_SIZE_0KB;
#endif
	tADO_KNLParm.BufSz.RecPkt     = BUF_SIZE_0KB;
	tADO_KNLParm.BufSz.RecEn      = BUF_SIZE_0KB;
#endif
#if defined(APP_TXREC_STREAM_SEL)
	tADO_KNLParm.BufSz.RecMix     = BUF_SIZE_0KB;
	tADO_KNLParm.BufSz.Mix        = BUF_SIZE_0KB;
#else
#if APP_REC_FUNC_ENABLE
	tADO_KNLParm.BufSz.RecMix     = BUF_SIZE_64KB;
#else
	tADO_KNLParm.BufSz.RecMix     = BUF_SIZE_0KB;
#endif
	tADO_KNLParm.BufSz.Mix        = BUF_SIZE_16KB;
#endif

	//buffer threshold
	switch(SampleRate)
	{
		case SAMPLERATE_8kHZ:
			tADO_KNLParm.BufTh.Adc = (Channel==MONO)?BUF_TH_2KB:BUF_TH_4KB;
			break;
		case SAMPLERATE_16kHZ:
			tADO_KNLParm.BufTh.Adc = (Channel==MONO)?BUF_TH_4KB:BUF_TH_8KB;
			break;
		case SAMPLERATE_24kHZ:
			tADO_KNLParm.BufTh.Adc = (Channel==MONO)?BUF_TH_8KB:BUF_TH_16KB;
			break;
		case SAMPLERATE_32kHZ:
			tADO_KNLParm.BufTh.Adc = (Channel==MONO)?BUF_TH_8KB:BUF_TH_16KB;
			break;
		case SAMPLERATE_48kHZ:
			tADO_KNLParm.BufTh.Adc = (Channel==MONO)?BUF_TH_16KB:BUF_TH_32KB;
			break;
		case SAMPLERATE_11_025kHZ:
			tADO_KNLParm.BufTh.Adc = (Channel==MONO)?BUF_TH_4KB:BUF_TH_8KB;
			break;
		case SAMPLERATE_22_05kHZ:
			tADO_KNLParm.BufTh.Adc = (Channel==MONO)?BUF_TH_8KB:BUF_TH_16KB;
			break;
		case SAMPLERATE_44_1kHZ:
			tADO_KNLParm.BufTh.Adc = (Channel==MONO)?BUF_TH_16KB:BUF_TH_32KB;
			break;
	}
	tADO_KNLParm.BufTh.Dac = tADO_KNLParm.BufTh.Adc;

	tADO_KNLParm.ulSelfTestTime = 3;
	
	tADO_KNLParm.Get1ms = KNL_TIMER_Get1ms;

	tADO_KNLParm.ulBufStartAddr  = 0;
	KNL_SetAdoInfo(tADO_KNLParm);

	//! Audio32 Setting
	for(i=0; i<ADO_AUDIO32_MAX_NUM; i++)
	{
		ADO_Ado32EncInit(i,SNX_AUD32_FMT16_16K_16KBPS);
		ADO_Ado32DecInit(i,SNX_AUD32_FMT16_16K_16KBPS);
	}
#if(ADO_ENC_TYPE==AUDIO32_ENC)
	ADO_Set_Audio32_Enable(ADO_ON);
	ADO_Set_SwAlaw_Enable(ADO_OFF);
	ADO_Set_AAC_Enable(ADO_OFF);
#elif(ADO_ENC_TYPE==HW_ALAW_ENC)
	ADO_Set_Audio32_Enable(ADO_OFF);
	ADO_Set_SwAlaw_Enable(ADO_OFF);
	ADO_Set_AAC_Enable(ADO_OFF);
#elif(ADO_ENC_TYPE==SW_ALAW_ENC)
	ADO_Set_Audio32_Enable(ADO_OFF);
	ADO_Set_SwAlaw_Enable(ADO_ON);
	ADO_Set_AAC_Enable(ADO_OFF);
#elif(ADO_ENC_TYPE==SW_AAC_ENC)
	ADO_Set_Audio32_Enable(ADO_OFF);
	ADO_Set_SwAlaw_Enable(ADO_OFF);
	ADO_Set_AAC_Enable(ADO_ON);
	switch(SampleRate)
	{
		case SAMPLERATE_8kHZ:
			ADO_AAC_EncInit(0, 1, 8000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_EncInit(1, 1, 8000, (Channel==MONO)?1:2);	//for Tx recording
			ADO_AAC_DecInit(0, 1, 8000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(1, 1, 8000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(2, 1, 8000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(3, 1, 8000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(4, 0, 8000, (Channel==MONO)?1:2);	//for movie play
			break;
		case SAMPLERATE_16kHZ:
			ADO_AAC_EncInit(0, 1, 16000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(0, 1, 16000, (Channel==MONO)?1:2);	//for Tx recording
			ADO_AAC_DecInit(0, 1, 16000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(1, 1, 16000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(2, 1, 16000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(3, 1, 16000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(4, 0, 16000, (Channel==MONO)?1:2);	//for movie play
			break;
		case SAMPLERATE_24kHZ:
			ADO_AAC_EncInit(0, 1, 24000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(0, 1, 24000, (Channel==MONO)?1:2);	//for Tx recording
			ADO_AAC_DecInit(0, 1, 24000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(1, 1, 24000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(2, 1, 24000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(3, 1, 24000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(4, 0, 24000, (Channel==MONO)?1:2);	//for movie play
			break;
		case SAMPLERATE_32kHZ:
			ADO_AAC_EncInit(0, 1, 32000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(0, 1, 32000, (Channel==MONO)?1:2);	//for Tx recording
			ADO_AAC_DecInit(0, 1, 32000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(1, 1, 32000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(2, 1, 32000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(3, 1, 32000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(4, 0, 32000, (Channel==MONO)?1:2);	//for movie play
			break;
		case SAMPLERATE_48kHZ:
			ADO_AAC_EncInit(0, 1, 48000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(0, 1, 48000, (Channel==MONO)?1:2);	//for Tx recording
			ADO_AAC_DecInit(0, 1, 48000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(1, 1, 48000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(2, 1, 48000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(3, 1, 48000, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(4, 0, 48000, (Channel==MONO)?1:2);	//for movie play
			break;
		case SAMPLERATE_11_025kHZ:
			ADO_AAC_EncInit(0, 1, 11025, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(0, 1, 11025, (Channel==MONO)?1:2);	//for Tx recording
			ADO_AAC_DecInit(0, 1, 11025, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(1, 1, 11025, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(2, 1, 11025, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(3, 1, 11025, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(4, 0, 11025, (Channel==MONO)?1:2);	//for movie play
			break;
		case SAMPLERATE_22_05kHZ:
			ADO_AAC_EncInit(0, 1, 22050, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(0, 1, 22050, (Channel==MONO)?1:2);	//for Tx recording
			ADO_AAC_DecInit(0, 1, 22050, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(1, 1, 22050, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(2, 1, 22050, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(3, 1, 22050, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(4, 0, 22050, (Channel==MONO)?1:2);	//for movie play
			break;
		case SAMPLERATE_44_1kHZ:
			ADO_AAC_EncInit(0, 1, 44100, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(0, 1, 44100, (Channel==MONO)?1:2);	//for Tx recording
			ADO_AAC_DecInit(0, 1, 44100, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(1, 1, 44100, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(2, 1, 44100, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(3, 1, 44100, (Channel==MONO)?1:2);	//for preview
			ADO_AAC_DecInit(4, 0, 44100, (Channel==MONO)?1:2);	//for movie play
			break;
	}
#endif

	//! AEC and NR Setting
#if(ADO_ENC_TYPE==AUDIO32_ENC)
	ADO_Noise_Process_Type((ubNrSetting==1)?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
#elif(ADO_ENC_TYPE==HW_ALAW_ENC)
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);	
#elif(ADO_ENC_TYPE==SW_ALAW_ENC)
	ADO_Noise_Process_Type((ubNrSetting==1)?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
#elif(ADO_ENC_TYPE==SW_AAC_ENC)
	ADO_Noise_Process_Type((ubNrSetting==1)?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
#endif

	ADO_Set_DeHowling_Enable(ADO_OFF);

	ADO_SetDeHowlingLV(DeHowlingLV0);

	//! ADC software gain compensation
	ADO_AdcSwGainCompensation(1);
	
	//! DAC software gain compensation
	ADO_DacSwGainCompensation(1);
}
//------------------------------------------------------------------------------
#endif
