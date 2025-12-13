/*
 * BLong.h
 *
 *  Created on: Dec 8, 2025
 *      Author: dobao
 */

#ifndef INC_BLONG_H_
#define INC_BLONG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "core_arm4.h"
#include "flash.h"
#include "BLong_cortex.h"


/*
 * =====================================================================
 * 							SYSTEM CONFIGURATION
 * =====================================================================
 */
#define VDD_VALUE_					3300u	//Value in mili-Vol
#define TICK_INTERRUPT_PRIORITY_	15u		//Tick Interrupt Priority
#define PREFETCH_ENABLE_			1u		//Write 0 to skip FLASH Prefetch
#define	INSTRUCTION_CACHE_ENABLE_	1u		//Write 0 to skip FLASH Instruction Cache
#define DATA_CACHE_ENABLE_			1u		//Write 0 to skip FLASH Data Cache


/*
 * =====================================================================
 * 						RCC RELATED CONFIGURATIONS
 * =====================================================================
 */
#define SYSTEM_CORE_CLOCK	16000000u	//16MHz Internal Clock Frequency
#define MAX_SYSTEM_CLOCK	168000000u	//168MHz Maximum Clock Frequency
#define MAX_DELAY			0xFFFFFFFFu

typedef enum{
	TICK_FREQ_1KHZ		= 1000u,	//1ms tick
	TICK_FREQ_100HZ		= 100u,		//10ms tick
	TICK_FREQ_10HZ		= 10u,		//100ms tick
	TICK_FREQ_DEFAULT	= TICK_FREQ_1KHZ	//1ms interrupt tick is the tick frequency default
}Tick_Freq_t;


/*
 * ====================================================================
 * 					BLong STATUS FOR DEBUGGING
 * ====================================================================
 */
typedef enum{
	BLong_ERROR,
	BLong_OK,
	BLong_BUSY,
	BLong_TIMEOUT
}BLong_Status_t;


/*
 * ====================================================================
 * 						EXTERN PARAMETERS
 * ====================================================================
 */
extern volatile uint32_t tickGlobal;
extern uint32_t tickPriorityGlobal;
extern Tick_Freq_t tickFreqGlobal;


/*
 * ====================================================================
 * 						FUNCTION DECLARATIONS
 * ====================================================================
 */
__attribute__((weak)) BLong_Status_t BLong_initTick(uint32_t tickPriority);
__attribute__((weak)) void BLong_MspInit(void);
BLong_Status_t BLong_init();


#endif /* INC_BLONG_H_ */
