/*
 * BLong_cortexx.h
 *
 *  Created on: Dec 8, 2025
 *      Author: dobao
 */

#ifndef INC_BLONG_CORTEX_H_
#define INC_BLONG_CORTEX_H_

#include <stdint.h>
#include <stdbool.h>

#include "core_arm4.h"

typedef enum{
	SYSTICK_CLK_SOURCE_HCLK			= 0x00000004u,
	SYSTICK_CLK_SOURCE_HCLK_DIV8_	= 0x00000000u
}SysTick_Clk_Source_t;

/*
 * =============================================================
 * 						FUNCTION DECLARATIONS
 * =============================================================
 */
void BLong_NVIC_setPriorityGrouping(NVIC_PriorityGroup_t priorityGroupSel);
NVIC_PriorityGroup_t BLong_getPriorityGrouping(void);

bool BLong_NVIC_enableIRQ(IRQn_t irqNumber);
bool BLong_NVIC_disableIRQ(IRQn_t irqNumber);

uint32_t BLong_NVIC_getPendingIRQ(IRQn_t irqNumber);
bool BLong_NVIC_setPendingIRQ(IRQn_t irqNumber);

bool BLong_NVIC_clearPendingIRQ(IRQn_t irqNumber);
uint32_t BLong_NVIC_getActiveInterrupt(IRQn_t irqNumber);

bool BLong_NVIC_setPriorityIRQ(IRQn_t irqNumber, uint32_t preemptPriority, uint32_t subPriority);
void BLong_NVIC_getPriorityIRQ(NVIC_PriorityGroup_t priorityGroup, IRQn_t irqNumber, uint32_t* preemptPriority, uint32_t* subPriority);

void BLon_NVIC_systemReset(void);
bool BLong_SysTickConfig(uint32_t ticks);

void BLong_CORTEX_clearEvent(void);
void BLong_SysTick_cloclSourceConfig(SysTick_Clk_Source_t clkSource);

void BLong_SysTick_IRQHandler(void);
__attribute__((weak)) void BLong_SysTick_callBack(void);

#endif /* INC_BLONG_CORTEX_H_ */
