/*!
    The information contained herein is the exclusive property of SONiX and
    shall not be distributed, or disclosed in whole or in part without prior
    permission of SONiX.
    SONiX reserves the right to make changes without further notice to the
    product to improve reliability, function or design. SONiX does not assume
    any liability arising out of the application or use of any product or
    circuits described herein. All application information is advisor and does
    not from part of the specification.

    \file       INTC.c
    \brief      Interrupt Controller
    \author     Nick Huang
    \version    3
    \date       2022/02/24
    \copyright  Copyright (C) 2022 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "INTC.h"

__attribute__((section("D_SRAM"))) INTC_IrqHandler INTC_IrqFP[64];              //!< IRQ function pointer
#ifdef FIQ_ENABLE
INTC_FiqHandler INTC_FiqFP[32];                                                 //!< FIQ function pointer
#endif

const static INTC_TrigModeMap_t tINTC_TrigModeMap[] =
{
    [INTC_TIMER1_IRQ]       = {INTC_LEVEL_TRIG},
    [INTC_TIMER2_IRQ]       = {INTC_LEVEL_TRIG},
    [INTC_TIMER3_IRQ]       = {INTC_LEVEL_TRIG},
    [INTC_USB2DEV_IRQ]      = {INTC_LEVEL_TRIG},
    [INTC_USB2HOST2_IRQ]    = {INTC_LEVEL_TRIG},
    [INTC_USB2HOST1_IRQ]    = {INTC_LEVEL_TRIG},
    [INTC_LCD_IRQ]          = {INTC_LEVEL_TRIG},
    [INTC_MIPI_IRQ]         = {INTC_LEVEL_TRIG},
    [INTC_SEN_HSYNC_IRQ]    = {INTC_LEVEL_TRIG},
    [INTC_ISP_WIN_END_IRQ]  = {INTC_LEVEL_TRIG},
    [INTC_SEN_VSYNC_IRQ]    = {INTC_LEVEL_TRIG},
    [INTC_ISP_DIS_IRQ]      = {INTC_LEVEL_TRIG},
    [INTC_IMG_TX_IRQ]       = {INTC_LEVEL_TRIG},
    [INTC_H264_ENCODE_IRQ]  = {INTC_LEVEL_TRIG},
    [INTC_JPEG_IRQ]         = {INTC_LEVEL_TRIG},
    [INTC_RF1_IRQ]          = {INTC_EDGE_TRIG},
    [INTC_RF2_IRQ]          = {INTC_EDGE_TRIG},
    [INTC_MAC_IRQ]          = {INTC_LEVEL_TRIG},
    [INTC_UART1_IRQ]        = {INTC_LEVEL_TRIG},
    [INTC_UART2_IRQ]        = {INTC_LEVEL_TRIG},
    [INTC_SD1_DET_IRQ]      = {INTC_LEVEL_TRIG},
    [INTC_SD1_IRQ]          = {INTC_LEVEL_TRIG},
    [INTC_SD2_DET_IRQ]      = {INTC_LEVEL_TRIG},
    [INTC_SD2_IRQ]          = {INTC_LEVEL_TRIG},
    [INTC_SD3_IRQ]          = {INTC_LEVEL_TRIG},
    [INTC_SF_IRQ]           = {INTC_LEVEL_TRIG},
    [INTC_CRC_OK_IRQ]       = {INTC_LEVEL_TRIG},
    [INTC_CIPHER_OK_IRQ]    = {INTC_LEVEL_TRIG},
    [INTC_GPIO_IRQ]         = {INTC_LEVEL_TRIG},
    [INTC_DMA_IRQ]          = {INTC_LEVEL_TRIG},
    [INTC_ADO_R_IRQ]        = {INTC_LEVEL_TRIG},
    [INTC_ADO_W_IRQ]        = {INTC_LEVEL_TRIG},
    [INTC_AHB0_AHBC_IRQ]    = {INTC_LEVEL_TRIG},
    [INTC_AHBC1_IRQ]        = {INTC_LEVEL_TRIG},
    [INTC_AHBC2_IRQ]        = {INTC_LEVEL_TRIG},
    [INTC_AHBC3_IRQ]        = {INTC_LEVEL_TRIG},
    [INTC_I2C1_IRQ]         = {INTC_LEVEL_TRIG},
    [INTC_I2C2_IRQ]         = {INTC_LEVEL_TRIG},
    [INTC_I2C3_IRQ]         = {INTC_LEVEL_TRIG},
    [INTC_I2C4_IRQ]         = {INTC_LEVEL_TRIG},
    [INTC_I2C5_IRQ]         = {INTC_LEVEL_TRIG},
    [INTC_SSP_IRQ]          = {INTC_LEVEL_TRIG},
    [INTC_APB_IRQ]          = {INTC_LEVEL_TRIG},
    [INTC_DDR_CTRL_IRQ]     = {INTC_LEVEL_TRIG},
    [INTC_WDOG_IRQ]         = {INTC_EDGE_TRIG},
    [INTC_RTC_ALARM_IRQ]    = {INTC_EDGE_TRIG},
    [INTC_RTC_WAKEUP_IRQ]   = {INTC_EDGE_TRIG},
    [INTC_CMDQUE_IRQ]       = {INTC_LEVEL_TRIG},
    [INTC_KEY_IRQ]          = {INTC_EDGE_TRIG},
    [INTC_RTC_GPIO0_IRQ]    = {INTC_LEVEL_TRIG},
    [INTC_RTC_GPIO1_IRQ]    = {INTC_LEVEL_TRIG},
    [INTC_REG_RW_ERR_IRQ]   = {INTC_EDGE_TRIG},
    [INTC_ISP_MD_IRQ]       = {INTC_LEVEL_TRIG}
};
//------------------------------------------------------------------------------
void INTC_IrqSetup(INTC_IrqSrc_t irqSrc, INTC_IrqHandler pfISR)
{
    INTC_TrigMode_t trigMode;
    if(pfISR == NULL) return;                                                   //!< Check whether ISR function valid or not
    INTC_IrqFP[irqSrc] = pfISR;

    trigMode =  tINTC_TrigModeMap[irqSrc].tINTC_TrigMode;

    if(irqSrc < 32)
    {
        if(trigMode)
            SET_BIT(INTC->IRQ0_TRG_MODE, irqSrc);
        else
            CLR_BIT(INTC->IRQ0_TRG_MODE, irqSrc);
    }
    else
    {
        irqSrc -= 32;
        if(trigMode)
            SET_BIT(INTC->IRQ1_TRG_MODE, irqSrc);
        else
            CLR_BIT(INTC->IRQ1_TRG_MODE, irqSrc);
    }
}
//------------------------------------------------------------------------------
void INTC_IrqEnable(INTC_IrqSrc_t irqSrc)
{
    if(irqSrc < 32)
    {
        SET_BIT(INTC->IRQ0_EN, irqSrc);
    }
    else
    {
        irqSrc -= 32;
        SET_BIT(INTC->IRQ1_EN, irqSrc);
    }
}
//------------------------------------------------------------------------------
void INTC_IrqDisable(INTC_IrqSrc_t irqSrc)
{
    if(irqSrc < 32)
    {
        CLR_BIT(INTC->IRQ0_EN, irqSrc);
    }
    else
    {
        irqSrc -= 32;
        CLR_BIT(INTC->IRQ1_EN, irqSrc);
    }
}
//------------------------------------------------------------------------------
void INTC_IrqClear(INTC_IrqSrc_t irqSrc)
{
    uint32_t ulIrq;

    if(irqSrc < 32)
    {
        ulIrq = ((uint32_t) 1) << irqSrc;
        INTC->CLR_IRQ0 = ulIrq;
    }
    else
    {
        irqSrc -= 32;
        ulIrq = ((uint32_t) 1) << irqSrc;
        INTC->CLR_IRQ1 = ulIrq;
    }
}
//------------------------------------------------------------------------------
__attribute__((section("PG_SRAM"))) void INTC_IRQ_Handler(void)
{
    uint8_t ubi = 0;
    uint32_t ulFlag0, ulFlag1;

    ulFlag0 = INTC->IRQ0_FLAG;
    ulFlag1 = INTC->IRQ1_FLAG;

    do
    {
        if(ulFlag0 & INTC_RF1_FLAG)
        {
            INTC_IrqFP[INTC_RF1_IRQ]();
            ulFlag0 &= ~INTC_RF1_FLAG;
        }
        
        if(ulFlag0 & INTC_RF2_FLAG)
        {
            INTC_IrqFP[INTC_RF2_IRQ]();
            ulFlag0 &= ~INTC_RF2_FLAG;
        }
        
        if(ubi < 32)
        {
            if(ulFlag0 & (((uint32_t) 1) << ubi))
                INTC_IrqFP[ubi]();
        }
        else
        {
            if(ulFlag1 & (((uint32_t) 1) << (ubi - 32)))
                INTC_IrqFP[ubi]();
        }

        if(ubi++ == INTC_MAX_IRQ) ubi = 0;

        ulFlag0 = INTC->IRQ0_FLAG;
        ulFlag1 = INTC->IRQ1_FLAG;

    } while(ulFlag0 | ulFlag1);
}
//------------------------------------------------------------------------------
__attribute__((section("PG_SRAM"))) void INTC_UNDEFINED_INSTRUCITONS_Handler(void)
{
    printf("undefined instructions\n");
    while(1);
}
//------------------------------------------------------------------------------
__attribute__((section("PG_SRAM"))) void INTC_PREFETCH_ABORT_Handler(void)
{
    printf("prefetch abort\n");
    while(1);
}
//------------------------------------------------------------------------------
__attribute__((section("PG_SRAM"))) void INTC_DATA_ABORT_Handler(void)
{
    printf("data abort\n");
    while(1);
}
//------------------------------------------------------------------------------

#ifdef FIQ_ENABLE
//------------------------------------------------------------------------------
void INTC_FiqSetup(INTC_FiqSrc_t fiqSrc, INTC_TrigMode_t trigMode, INTC_FiqHandler pfISR)
{
    if(pfISR == NULL) return;                                                   //!< Check whether ISR function valid or not
    INTC_FiqFP[fiqSrc] = pfISR;

    if(trigMode)
        SET_BIT(INTC->FIQ_TRG_MODE, fiqSrc);
    else
        CLR_BIT(INTC->FIQ_TRG_MODE, fiqSrc);
}
//------------------------------------------------------------------------------
void INTC_FiqEnable(INTC_FiqSrc_t fiqSrc)
{
    SET_BIT(INTC->FIQ_EN, fiqSrc);
}
//------------------------------------------------------------------------------
void INTC_FiqDisable(INTC_FiqSrc_t fiqSrc)
{
    CLR_BIT(INTC->FIQ_EN, fiqSrc);
}
//------------------------------------------------------------------------------
void INTC_FiqClear(INTC_FiqSrc_t fiqSrc)
{
    uint32_t ulFiq;

    ulFiq = ((uint32_t) 1) << fiqSrc;
    INTC->CLR_FIQ = ulFiq;
}
//------------------------------------------------------------------------------
void INTC_FiqChk(INTC_FiqSrc_t fiqSrc)
{
    if(INTC->FIQ_FLAG & (((uint32_t) 1) << fiqSrc))
        if(INTC_FiqFP[fiqSrc] != NULL)
            INTC_FiqFP[fiqSrc]();
}
//------------------------------------------------------------------------------
void INTC_FIQ_Handler(void)
{
    //! FIQ check order can be changed
    INTC_FiqChk(INTC_CT16B0_FIQ);
    INTC_FiqChk(INTC_CT16B1_FIQ);
    INTC_FiqChk(INTC_CT16B2_FIQ);
    INTC_FiqChk(INTC_CT16B3_FIQ);
    INTC_FiqChk(INTC_CT32B0_FIQ);
    INTC_FiqChk(INTC_CT32B1_FIQ);
    INTC_FiqChk(INTC_CT32B2_FIQ);
    INTC_FiqChk(INTC_CT32B3_FIQ);
    INTC_FiqChk(INTC_CT32B4_FIQ);
    INTC_FiqChk(INTC_RF1_FIQ);
    INTC_FiqChk(INTC_RF2_FIQ);
    INTC_FiqChk(INTC_GPIO_FIQ);
}
//------------------------------------------------------------------------------
#endif
