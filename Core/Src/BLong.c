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
	BLong_initTick(TICK_INTERRUPT_PRIORITY_);

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
 * 			The time source is configured tickGlobal in milliseconds, using a configurable SysTick interrupt frequency.
 *
 * @param	tickPriority	Tick Interrupt Priority
 *
 * @note	The function is declared as __attribute__((weak))
 * @retVal	BLong status
 */
__attribute__((weak)) BLong_Status_t BLong_initTick(uint32_t tickPriority){
	/*
	 * Default setting:
	 * 	SYSTEM_CORE_CLOCK = 16MHz
	 * 	tickFreqGlobal = TICK_FREQ_DEFAULT = 1KHz
	 * 	_1msInterrupt = 16000
	 * 	It means when the MCU speed is 16MHz, SysTick generates each 1ms interrupt after each 16000-MCU-cycle
	 */
	uint32_t reloadVal = (uint32_t)SYSTEM_CORE_CLOCK / (uint32_t)tickFreqGlobal;

	/* Update the "reload" to SysTick Reload Register */
	if(!SysTick_config(reloadVal)) return BLong_ERROR;

	/* Configure SysTick Priority */
	if(tickPriority <= ((1u << NVIC_PRIO_BITS) - 1u)){ //The valid priority range is 0-15 (& 0xF)
		BLong_NVIC_setPriorityIRQ(SysTick_IRQn_, tickPriority, 0u);
		/* Update "tickPriorityGlobal" to a new value of "tickPriority" */
		tickPriorityGlobal = tickPriority;
	}
	else return BLong_ERROR;

	return BLong_OK;
}


/*
 * ==============================================================================
 * 						BLong CONTROLLING/CONFIGURING FUNCTIONS
 * ==============================================================================
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
	tickGlobal += (uint32_t)(1000u/tickFreqGlobal);
}


/*
 * @brief	Extract/Get a tick value in millisecond
 *
 * @note	Declared as __weak so user can overwritten in case of other implementation in user file
 *
 * @retVal	Current tick value in milliseconds
 */
__attribute__((weak)) uint32_t BLong_getTick(void){
	return tickGlobal;
}


/*
 * @brief	Update a new Tick Frequency
 *
 * @param	freqSel		Desired Tick Frequency value which is defined in Tick_Freq_t enum
 *
 * @note	BLong_init() must be called before calling BLong_setTickFreq().
 * 			Otherwise, this function will return ERROR due to unintialize/invalid Tick Priority Value
 *
 * @retVal	BLong status
 */
BLong_Status_t BLong_setTickFreq(Tick_Freq_t freqSel){
	BLong_Status_t status = BLong_OK;
	Tick_Freq_t prevTickFreq;

	if(tickFreqGlobal != freqSel){
		/* Just back up the current tickFreqGlobal value before setting a new tick frequency */
		prevTickFreq = tickFreqGlobal;
		/* Update Tick Frequency Global to a new value which used by BLong_initTick() */
		tickFreqGlobal = freqSel;
		/* Apply the new Tick Freq Global variable */
		status = BLong_initTick(tickPriorityGlobal);
		if(status != BLong_OK){
			/* Restore the previous tick frequency */
			tickFreqGlobal = prevTickFreq;
		}
	}
	return status;
}


/*
 * @brief	Return Tick Frequency
 *
 * @retVal	Tick frequency value which is defined in Tick_Freq_t enum
 */
Tick_Freq_t BLong_getTickFreq(void){
	return (Tick_Freq_t)tickFreqGlobal; //Re-cast tickFreqGlobal with Tick_Freq_t for extra safety. Re-cast is optional!
}


/*
 * @brief	This function waits for a number of milliseconds.
 * 			It uses SysTick Interrupt as its time base.
 *
 * @param	delay	Desired amount of delay in milliseconds
 *
 * @retVal	none
 */
__attribute__((weak)) void BLong_delay(uint32_t delay){
	uint32_t wait = delay; //Just store the "delay" in wait variable
	uint32_t tickStart = BLong_getTick(); //Get the current global "tick"

	if(wait < MAX_DELAY) wait += (uint32_t)(1000u/tickFreqGlobal); //Just add extra 1 step for safety

	while((BLong_getTick() - tickStart) < wait){
	}
}


/*
 * @brief	Suspend Tick Increment
 */
__attribute__((weak)) void BLong_suspendTick(void){
	/* Disable SysTick Interrupt */
	SysTick_REG -> CTRL &= ~SysTick_CTRL_TICKINT_Msk_;
}


/*
 * @brief	Resume Tick Increment
 */
__attribute__((weak)) void BLong_resumeTick(void){
	/* Enable SysTick Interrupt */
	SysTick_REG -> CTRL |= SysTick_CTRL_TICKINT_Msk_;
}













