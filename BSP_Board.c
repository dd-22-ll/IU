/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		BSP_Board.c
	\brief		VBM PU/BU Demo/EV Board
	\author		Hanyi Chiu
	\version	0.6
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#if defined VBM_PU || defined VBM_BU

#include "BSP.h"
#include "_510PF.h"
#include "APP_CFG.h"

//------------------------------------------------------------------------------
#ifdef BSP_SN93711_FHD_REC_RX_V4
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! LCD
	//! GPIO SPI
	GLB->PADIO47 = 1;
	GLB->PADIO48 = 1;
	GLB->PADIO49 = 1;
	GLB->PADIO50 = 1;
	//! LCD Pin
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;

	//! RF SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO15 = 0;
	GLB->PADIO21 = 0;

	//! SD
	GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	GLB->PADIO6  = 4;
	GLB->PADIO7  = 4;

	//! Speaker
	GLB->PADIO54 = 0;

	//! LED
	GLB->PADIO16 = 0;
	GLB->PADIO17 = 0;
	GLB->PADIO55 = 7;
	GLB->PADIO56 = 7;
	GLB->PADIO57 = 7;
	
	//! BL
	GLB->PADIO51 = 7;

	// LCD POWER
	GLB->PADIO10 = 7;
	GLB->PADIO24 = 7;
	GLB->PADIO38 = 7;
	GLB->PADIO52 = 0;
}
#endif
//------------------------------------------------------------------------------
#if (defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6) || defined(BSP_D_SN93703_TC358778_RX_V6))
void BSP_BoardInit(void)
{
	
	//! UART
	GLB->PADIO22 = 2;		//uart2 tx
	GLB->PADIO23 = 2;		//uart2 rx

	//! LCD
	//! GPIO SPI
//	GLB->PADIO47 = 1;//1 0;0 for LCD_FTD50B7 & LCD_H5024A
//	GLB->PADIO48 = 1;//1 0;0 for LCD_FTD50B7 & LCD_H5024A
//	GLB->PADIO49 = 1;//1 0;0 for LCD_FTD50B7 & LCD_H5024A
	GLB->PADIO50 = 1;//1 0;0 for LCD_FTD50B7 & LCD_H5024A

	//! LCD Pin
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;

	//! RF SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO15 = 0;		//gpio1
	GLB->PADIO21 = 0;		//gpio7	//0 1;1 for LCD_FTD50B7 & LCD_H5024A

	//! SD
	//GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	//GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	//GLB->PADIO6  = 4;
	GLB->PADIO7  = 4;

	//! Speaker
	GLB->PADIO54 = 0;

	//! LED
	//GLB->PADIO16 = 0;
	GLB->PADIO17 = 0;
	GLB->PADIO55 = 7;
	GLB->PADIO56 = 7;
	//GLB->PADIO57 = 7;
	
	//! BL
	GLB->PADIO51 = 7;

	// LCD POWER
	GLB->PADIO10 = 7;
	GLB->PADIO24 = 7;
	GLB->PADIO38 = 7;
	//GLB->PADIO52 = 0;
	//! LCD Reset
	GLB->PADIO11 = 7;	
	
	
	
	GLB->PADIO52 = 0;		//gpio10	//charge det
	GLB->PADIO57 = 7;		//pwm9		//flash_en
	
	GLB->PADIO50 = 7;		//pwm2		//lcd reset
	GLB->PADIO0  = 0;		//gpio0		//lcd background en
	GLB->PADIO2  = 0;		//gpio2		//lcd power
	
	//lcd 
	GLB->PADIO47 = 0;		//gpio5		//lcd clk
	GLB->PADIO48 = 0;		//gpio6		//lcd cs
	GLB->PADIO49 = 7;		//pwm1		//lcd sdo
	
	
	/*//! UART
	GLB->PADIO22 = 2;		//uart2 tx
	GLB->PADIO23 = 2;		//uart2 rx

	//! LCD
	//! GPIO SPI
	GLB->PADIO47 = 1;//1 0;0 for LCD_FTD50B7 & LCD_H5024A
	GLB->PADIO48 = 1;//1 0;0 for LCD_FTD50B7 & LCD_H5024A
	GLB->PADIO49 = 1;//1 0;0 for LCD_FTD50B7 & LCD_H5024A
	GLB->PADIO50 = 1;//1 0;0 for LCD_FTD50B7 & LCD_H5024A

	//! LCD Pin
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;

	//! RF SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO15 = 0;
	GLB->PADIO21 = 0;//0 1;1 for LCD_FTD50B7 & LCD_H5024A

	//! SD
	GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	//GLB->PADIO6  = 4;
	GLB->PADIO7  = 4;

	//! Speaker
	GLB->PADIO54 = 0;

	//! LED
	GLB->PADIO16 = 0;
	GLB->PADIO17 = 0;
	GLB->PADIO55 = 7;
	GLB->PADIO56 = 7;
	GLB->PADIO57 = 7;
	
	//! BL
	GLB->PADIO51 = 7;

	// LCD POWER
	GLB->PADIO10 = 7;
	GLB->PADIO24 = 7;
	GLB->PADIO38 = 7;
	GLB->PADIO52 = 0;
	//! LCD Reset
	GLB->PADIO11 = 7; */    
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMPU_EV
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! SSP
	GLB->PADIO58 = 4;
	GLB->PADIO59 = 4;
	GLB->PADIO60 = 4;
	GLB->PADIO61 = 4;
	//! LCD Pin
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;

	//! SDIO Wi-Fi
	GLB->PADIO52 = 4;
	GLB->PADIO53 = 4;
	GLB->PADIO54 = 4;
	GLB->PADIO55 = 4;
	GLB->PADIO56 = 4;
	GLB->PADIO57 = 4;
	
	//! RF SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO15 = 0;
	GLB->PADIO21 = 0;

	//! SD
	GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	GLB->PADIO6  = 4;
	GLB->PADIO7  = 4;
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_SN93710_FHD_REC_TX_V4_1
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF_SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO28 = 0;
	GLB->PADIO27 = 0;

	//! SD
	GLB->PADIO0 = 4;
	GLB->PADIO1 = 4;
	GLB->PADIO2 = 4;
	GLB->PADIO3 = 4;
	GLB->PADIO4 = 4;
	GLB->PADIO5 = 4;
	GLB->PADIO6 = 4;
	GLB->PADIO7 = 4;

	//! I2C-2
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;

	//! Speaker
	GLB->PADIO32 = 0;

	//! LED
	GLB->PADIO29 = 0;
	GLB->PADIO30 = 0;
	GLB->PADIO31 = 0;
	
#if ( (BSP_ADO_I2S_MODE_ENABLE == 1) | (APP_ADO_AEC_NR_TYPE == AEC_NR_HW) )
	//! Tx I2S
	GLB->PADIO37 = 6;
	GLB->PADIO38 = 6;
	GLB->PADIO39 = 6;
	GLB->PADIO40 = 6;
	GLB->PADIO41 = 6;
#endif

#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	//! I2C-1
	GLB->PADIO6  = 5;	//i2c_0 clk
	GLB->PADIO46 = 1;	//i2c_0 sda
#endif
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93714_TX_V1
void BSP_BoardInit(void)
{
	//! AKEY PIN 
	GLB->PADIO8  = 7;	//!< ADC0
	
	//! RF Reset PIN 
//	GLB->PADIO9  = 0;
	
	//! Sensor
	GLB->PADIO13 = 4;	//!< I2C-2 SCL
	GLB->PADIO14 = 4;	//!< I2C-2 SDA
//	GLB->PADIO15 = 0;	//!< Sensor Reset Pin	(GPIO1)
	
	//! RF_SPI
	GLB->PADIO18 = 1;	//!< RF CLK
	GLB->PADIO19 = 1;	//!< RF CS
	GLB->PADIO20 = 1;	//!< RF MO
//	GLB->PADIO21 = 0;	//!< RF CKO				(GPIO7)
//	GLB->PADIO55 = 0;	//!< RF TXSW			(GPIO13)
	GLB->PADIO33 = 7;	//!< RF 5V Power		(PWM1)
	GLB->PADIO7  = 7;
	GLB->PADIO41 = 7;
	//! SD
	GLB->PADIO26 = 2;	//!< SD3 CLK
	GLB->PADIO27 = 2;	//!< SD3 CMD
	GLB->PADIO28 = 2;	//!< SD3 D0
	GLB->PADIO29 = 2;	//!< SD3 D1
	GLB->PADIO30 = 2;	//!< SD3 D2
	GLB->PADIO31 = 2;	//!< SD3 D3
	GLB->PADIO36 = 0;	//!< SD CD				(GPIO8)
	GLB->PADIO22 = 7;
	
	//! SF_WP
//	GLB->PADIO32 = 0;	//!< NULL				(GPIO4)

	//! LED
//	GLB->PADIO34 = 0;	//!< LEDR				(GPIO6)
	GLB->PADIO35 = 7;	//!< LEDG				(PWM3)
//	GLB->PADIO36 = 0;	//!< LEDB				(GPIO8)

	//! I2C-1
	GLB->PADIO52 = 6;	//!< I2C-1 SCL
	GLB->PADIO53 = 6;	//!< I2C-1 SDA
	
	//! SPK_EN
//	GLB->PADIO54 = 0;	//!< 					(GPIO12)
	
	//! UART
	GLB->PADIO56 = 2;	//!< UART0 TX
	GLB->PADIO57 = 2;	//!< UART0 RX
//	GLB->PADIO23 = 0;
	
#if ( (BSP_ADO_I2S_MODE_ENABLE == 1) | (APP_ADO_AEC_NR_TYPE == AEC_NR_HW) )
	//! I2S
	GLB->PADIO32 = 2;	//!< MLCK
	GLB->PADIO33 = 2;	//!< BCLK
	GLB->PADIO34 = 2;	//!< LRCK
	GLB->PADIO35 = 2;	//!< DO
	GLB->PADIO36 = 2;	//!< D1
#endif
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93716_TX_V1
void BSP_BoardInit(void)
{
	//! AKEY 
	GLB->PADIO8  = 7;	//!< ADC0
	
	//! LED
//	GLB->PADIO9  = 0;	//!< LED B				(GPO9)
//	GLB->PADIO33 = 0;	//!< LED R				(GPO5)
	//! Sensor
	GLB->PADIO13 = 4;	//!< I2C-2 SCL
	GLB->PADIO14 = 4;	//!< I2C-2 SDA
//	GLB->PADIO15 = 0;	//!< Sensor Reset Pin	(GPIO1)
	
	//! RF_SPI
	GLB->PADIO18 = 1;	//!< RF CLK
	GLB->PADIO19 = 1;	//!< RF CS
	GLB->PADIO20 = 1;	//!< RF MO
//	GLB->PADIO21 = 0;	//!< RF CKO				(GPI7)
//	GLB->PADIO36 = 0;	//!< RF TXSW			(GPI8)
	GLB->PADIO35 = 7;	//!< RF Reset			(PWM3)
	GLB->PADIO43 = 7;	//!< RF 5V Power		(PWM11)
	GLB->PADIO7  = 7;
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;	
	//! SF
//	GLB->PADIO32 = 0;	//!< SF WP PIN			(GPO4)
	//! SPK_EN
//	GLB->PADIO34 = 0;	//!< 					(GPO6)
	//! SD
	GLB->PADIO37 = 1;	//!< SD2 CLK
	GLB->PADIO38 = 1;	//!< SD2 CMD
	GLB->PADIO39 = 1;	//!< SD2 D0
	GLB->PADIO40 = 1;	//!< SD2 D1
	GLB->PADIO41 = 1;	//!< SD2 D2
	GLB->PADIO42 = 1;	//!< SD2 D3
	GLB->PADIO44 = 1;	//!< SD2 CD
	//! I2C-1
	GLB->PADIO45 = 1;	//!< I2C-1 SCL
	GLB->PADIO46 = 1;	//!< I2C-1 SDA
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC72_TX_V1
void BSP_BoardInit(void)
{
#ifdef RTC676x
	//! RF_SPI(Richwave RTC676x RF)	
	GLB->PADIO4 = 6;	//SPI_MO
	GLB->PADIO5 = 6;	//SPI_MI
	GLB->PADIO6 = 6;	//SPI_CLK
   
	GLB->PADIO42 = 0;	//RSTN, GPIO 0	
	GLB->PADIO41 = 0;	//IRQN, GPIO 13 (Input)	
	//==============================
	//Set other mode for "GPI 13"	
	GLB->PADIO27 = 7;	
	//==============================	
#endif	

#ifdef A7130
	//! A7130	
	GLB->PADIO4 = 3;
	GLB->PADIO6 = 3;
	GLB->PADIO7 = 3;
	GLB->PADIO5 = 0;
	GLB->PADIO41 = 0;
	GLB->PADIO27 = 7;
#endif	

	//! I2C-2
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;

    //RN6752 REST
    GLB->PADIO15 = 0;
	
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF PWR CTRL
	//! TMS GPIO18
	GLB->JTAG_EN = 0;
	
	//! KEY TDI GPIO19
	//! LCD TD0 GPIO20
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93712_VBM_TX_V2
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF Power
	GLB->PADIO4  = 0;
	//! RF SPI IO
	GLB->PADIO39 = 2;
	GLB->PADIO42 = 2;
	GLB->PADIO41 = 2;
	//! RF GKO IO
	GLB->PADIO12 = 4;
	GLB->PADIO26 = 2;
	GLB->PADIO40 = 0;
	//! RF TXSW IO
	GLB->PADIO5  = 0;

	//! Reset Sensor
	GLB->PADIO1  = 4;		//这里是为了避免与gpio1冲突
	GLB->PADIO15 = 0;		//gpio1

	//! I2C-2
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;

	//! Pairing Key
	GLB->PADIO6  = 0;		//gpio6

	//! LED
	GLB->JTAG_EN = 0;
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMBU_EV
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF_SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO28 = 0;
	GLB->PADIO27 = 0;

	//! SD
	GLB->PADIO0 = 4;
	GLB->PADIO1 = 4;
	GLB->PADIO2 = 4;
	GLB->PADIO3 = 4;
	GLB->PADIO4 = 4;
	GLB->PADIO5 = 4;
	GLB->PADIO6 = 4;
	GLB->PADIO7 = 4;

	//! SDIO Wi-Fi
	GLB->PADIO52 = 4;
	GLB->PADIO53 = 4;
	GLB->PADIO54 = 4;
	GLB->PADIO55 = 4;
	GLB->PADIO56 = 4;
	GLB->PADIO57 = 4;
}
#endif
//------------------------------------------------------------------------------

#endif //! End #if defined VBM_PU || defined VBM_BU
