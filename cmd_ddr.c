/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_ddr.c
	\brief		DDR Memory Configuration command line
	\author		Hanyi Chiu
	\version	0.1
	\date		2017/12/27
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "cmd.h"
#include "CLI.h"
#include "BUF.h"
#include "DDR_API.h"

#ifdef CONFIG_CLI_CMD_DDR
//------------------------------------------------------------------------------
int32_t cmd_ddr_getMemoryConf(int argc, char* argv[])
{
	uint32_t ulDdrSz = 0, ulFreeAddr = 0;

	printf("===================================\n");
	printf("        DDR Configuration  		   \n");
	printf("-----------------------------------\n");

	ulDdrSz	   = ulDDR_GetCapacity();
	ulFreeAddr = ulBUF_GetFreeAddr();
	printf("DDR Size: %d MB\n", (ulDdrSz/1024/1024));
	printf("DDR Memory: 0x%X[%d]\n", ulFreeAddr, ulFreeAddr);

	printf("===================================\n");
	return cliPASS;
}
#endif
