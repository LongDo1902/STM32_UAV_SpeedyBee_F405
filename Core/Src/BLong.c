/*
 * BLong_general.c
 *
 *  Created on: Nov 30, 2025
 *      Author: dobao
 */

#include "BLong.h"


/*
 * ==============================================================================
 * 						GLOBAL PARAMETER DECLARATION
 * ==============================================================================
 */
volatile uint32_t tickGlobal;
uint32_t tickPriorityGlobal = (1u << NVIC_PRIO_BITS);	//Invalid Priority
Tick_Freq_t tickFreqGlobal = TICK_FREQ_DEFAULT;			//1kHZ Tick Frequency


/*
 * ==============================================================================
 * 							FUNCTIONS START HERE
 * ==============================================================================
 */

/*
 * @brief	This function rewritten from HAL_Init() for private use and study only!
 * 			This function must be the first instruction to be executed in the main program
 *
 * 			It performs:
 * 				Configures FLASH Prefetch, Instruction Caches, and Data Caches.
 * 				Configures the SysTick to generate an interrupt each 1 milisecond, which is
 * 				clocked by which is clocked by the HSI (internally 16MHz)
 * 				Set NVIC Group Priority to Group 4
 * 				Calls the BLong_MsInit() call back function defined in user file "stm32f4xx_hal_msp.c"
 * 				to do the global low level hardware init
 */
BLong_Status_t BLong_init(){
	/* Enable FLASH Prefetch */
	if(!FLASH_enablePrefetch()) return BLong_ERROR;

	/* Enable FLASH Instruction Cache */
	if(!FLASH_enableInstructionCache()) return BLong_ERROR;

	/* Enable FLASH Data Cache */
	if(!FLASH_enableDataCache()) return BLong_ERROR;

	/*
	 * Priority Group 4:
	 * 4 bits Pre-emption Priority
	 * 0 bits Sub Priority
	 */
	BLong_NVIC_setPriorityGrouping(NVIC_PRIORITY_GROUP_4_);

	/* Use SysTick as time base source for Cortex to run
	 * Configure 1ms tick
	 * Default: clock after Reset is HSI (16MHz) unless user change it */
	BLong_initTick(TICK_INTERRUPT_PRIORITY_, TICK_FREQ_1KHZ);

	/* Init low level register (if have any) */
	BLong_MspInit();

	return BLong_OK;
}


/*
 * @brief	Initialize the MSP
 * @retVal	None
 */
__attribute__((weak)) void BLong_MspInit(void){
	/*
	 * Add some special initial hardware/register setups if you have/special requirement of some boards
	 */
	//	  __HAL_RCC_SYSCFG_CLK_ENABLE();
	//	  __HAL_RCC_PWR_CLK_ENABLE();
}


/*
 * @brief	DeInitialize the MSP
 * @retVal 	None
 */
__attribute__((weak)) void BLong_MspDeInit(void){

}


/*
 * @brief	This function configures the source of the time base.
 * 			The time source is configured to have 1ms time base with a dedicated Tick Interrupt Priority.
 *
 * @param	tickPriority	Tick Interrupt Priority
 * @param	tickFreqSel		Tick frequency value defined in enum Tick_Freq_t
 *
 * @note	The function is declared as __attribute__((weak))
 * @retVal	BLong status
 */
__attribute__((weak)) BLong_Status_t BLong_initTick(uint32_t tickPriority, Tick_Freq_t tickFreqSel){
	uint32_t _1msInterrupt = (uint32_t)SYSTEM_CORE_CLOCK / (uint32_t)tickFreqSel;

	if(!SysTick_config(_1msInterrupt)) return BLong_ERROR;

	/* Configure SysTick priority */
	if(tickPriority <= ((1u << NVIC_PRIO_BITS) - 1u)){ //The valid priority range is 0-15 (& 0xF)
		BLong_NVIC_setPriorityIRQ(SysTick_IRQn_, tickPriority, 0u);
		tickPriorityGlobal = tickPriority;
	}
	else return BLong_ERROR;

	return BLong_OK;
}


/*
 * ====================================================================
 * 						BLong CONTROL FUNCTIONS
 * ====================================================================
 */

/*
 * @brief	This function is called to increment a global tick value
 * 			Used as application time base
 *
 * @note	Default: increment each 1ms in SysTick ISR
 * 			Declared as __weak so user can overwritten in case of other implementation in user file
 *
 * @retVal	None
 */
__attribute__((weak)) void BLong_incTick(void){
	tickGlobal += (volatile uint32_t)(tickFreqGlobal/1000u);
}














