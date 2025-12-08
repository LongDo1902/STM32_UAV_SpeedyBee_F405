	/*
 * flash.h
 *
 *  Created on: Nov 13, 2025
 *      Author: dobao
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32f405_addr.h"
#include "general.h"

/*
 * ===========================================================
 * 			  RESERVED BITS AND ITS STARED POSITION
 * ===========================================================
 */
#define ACR_RESERVED_Pos_4		4
#define ACR_RESERVED_Pos_13		13

#define	SR_RESERVED_Pos_2		2
#define	SR_RESERVED_Pos_9		9
#define	SR_RESERVED_Pos_17		17

#define CR_RESERVED_Pos_7		7
#define CR_RESERVED_Pos_10		10
#define CR_RESERVED_Pos_17		17
#define CR_RESERVED_Pos_26		26

#define OPTCR_RESERVED_Pos_4	4
#define	OPTCR_RESERVED_Pos_24	24

#define LATENCY_Pos		0
#define	PRFTEN_Pos		8
#define	ICEN_Pos		9
#define DCEN_Pos		10
#define	ICRST_Pos		11
#define	DCRST_Pos		12

typedef enum{
	ZERO_WS,
	ONE_WS,
	TWO_WS,
	THREE_WS,
	FOUR_WS,
	FIVE_WS,
	SIX_WS,
	SEVEN_WS
}FLASH_Latency_t;

typedef enum{
	FLASH_ERROR		= 0x00u,
	FLASH_OK		= 0x01u,
	FLASH_BUSY		= 0x02u,
	FLASH_TIMEOUT	= 0x03u
}FLASH_Status_t;

typedef enum{
	FLASH_ACR,
	FLASH_KEYR,
	FLASH_OPT_KEYR,
	FLASH_SR,
	FLASH_CR,
	FLASH_OPT_CR,
	FLASH_REG_COUNT
}FLASH_Reg_t;

/*
 * ===========================================================================
 * 								PUBLIC APIs
 * ===========================================================================
 */
FLASH_Status_t FLASH_init();
FLASH_Status_t FLASH_resetCache();

bool FLASH_setLatency(FLASH_Latency_t waitState);



bool FLASH_enablePrefetch();
bool FLASH_disablePrefetch();

bool FLASH_enableInstructionCache();
bool FLASH_disableInstructionCache();

bool FLASH_enableDataCache();
bool FLASH_disableDataCache();

bool FLASH_resetInstructionCache();
bool FLASH_resetDataCache();



bool writeFLASH(uint8_t bitPosition, FLASH_Reg_t regID, uint32_t value);
bool readFLASH(uint8_t bitPosition, FLASH_Reg_t regID, uint32_t* outVal);

#endif /* INC_FLASH_H_ */
