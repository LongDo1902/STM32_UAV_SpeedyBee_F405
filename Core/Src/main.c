#include <main.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * =====================================================
 * 					 PRIVATE INCLUDES
 * =====================================================
 */
#include "stm32f4xx_hal.h"
#include "BLong_cortex.h"
#include "BLong.h"
#include "flash.h"


int main(void){
	FLASH_setLatency(_6_WS);
	FLASH_enablePrefetch();
	FLASH_enableInstructionCache();
	FLASH_enableDataCache();

	FLASH_disablePrefetch();
	FLASH_disableInstructionCache();
	FLASH_disableDataCache();

	FLASH_resetInstructionCache();
	FLASH_resetDataCache();

	FLASH_sectorErase(SECTOR_9);

	while(1);
}
