/*
 * BLong.c
 *
 *  Created on: Nov 27, 2025
 *      Author: dobao
 */
#include <BLong_cortex.h>

/*
 * @brief	Set the Priority Grouping Field which decides which p-emption and sub priority to use
 *
 * @param	priorityGroupSel	Can be set with the following enum values:
 * 									NVIC_PRIORITY_GROUP_4_
 * 									...
 * 									NVIC_PRIORITY_GROUP_0_
 * @note	When NVIC_PRIORITY_GROUP_4_ is selected, a full 4-bit pre-emption priority and 0-bit for sub-priority will conducted, and vice versa
 */
void BLong_NVIC_setPriorityGrouping(NVIC_PriorityGroup_t priorityGroupSel){
	/* Set the PRIGROUP[10:8] bits according to the Priority Group parameter value */
	NVIC_setPriorityGrouping(priorityGroupSel);
}


/*
 * @brief	Gets the Priority Grouping field from the NVIC AIRCR
 * @retVal	A priority grouping value in NVIC_PriorityGroup_t enum
 */
NVIC_PriorityGroup_t BLong_getPriorityGrouping(void){
	/* Get the PRIGROUP[10:8] field value */
	return NVIC_getPriorityGrouping();
}


/*
 * @brief	Enable a device specific interrupt in Interrupt Set Enable Reg
 *
 * @param	irqNumber	External Interrupt Number that you want to enable
 */
bool BLong_NVIC_enableIRQ(IRQn_t irqNumber){
	if(!NVIC_enableIRQ(irqNumber)) return false;
	return true;
}


/*
 * @brief	Disables a device specific interrupt in Interrupt Set Enable Reg
 *
 * @param	irqNumber	External Interrupt Number that you want to disable
 */
bool BLong_NVIC_disableIRQ(IRQn_t irqNumber){
	if(!NVIC_disableIRQ(irqNumber)) return false;
	return true;
}


/*
 * @brief	Gets Pending Interrupt
 * 			It reads the pending register in NVIC and returns the pending bit for specific interrupt
 *
 * @param	irqNumber	External STM32 Interrupt number
 *
 * @retVal	0	Interrupt status is not pending
 * 			1	Interrupt status is pending
 */
uint32_t BLong_NVIC_getPendingIRQ(IRQn_t irqNumber){
	return NVIC_getPendingIRQ(irqNumber);
}


/*
 * @brief	Set Pending bit of an External STM32 Interrupt
 * @param	irqNumber	External STM32 Interrupt number
 * @retVal	none
 */
bool BLong_NVIC_setPendingIRQ(IRQn_t irqNumber){
	return NVIC_setPendingIRQ(irqNumber);
}


/*
 * @brief	Clears the Pending bit of a specific External STM32 Interrupt
 * @param	irqNumber	an External STM32 Interrupt number
 * @retVal	none
 */
bool BLong_NVIC_clearPendingIRQ(IRQn_t irqNumber){
	return NVIC_clearPendingIRQ(irqNumber);
}


/*
 * @brief	Read Interrupt Active Bit Register
 * 			"Is this interrupt currently executing on the CPU right now?"
 *
 * @retVal	0: NOT running
 * 			1: This interrupt's ISR is running
 */
uint32_t BLong_NVIC_getActiveInterrupt(IRQn_t irqNumber){
	return NVIC_getActiveInterrupt(irqNumber);
}


/*
 * @brief	Sets the priority of an interrupt
 *
 * @param	irqNumber			External STM32 Interrupt number
 *
 * @param	preemptPriority		The Preemption Priority for the IRQn channel
 * 								A lower priority value, indicates a higher priority
 * 								Range from 0-15
 *
 * @param	subPriority			The subpriority level for IRQ Channel
 * 								A lower priority value, indicates a higher priority
 * 								Range from 0-15
 */
bool BLong_NVIC_setPriorityIRQ(IRQn_t irqNumber, uint32_t preemptPriority, uint32_t subPriority){
	NVIC_PriorityGroup_t priorityGroup = 0x00u; //Just initialize
	priorityGroup = NVIC_getPriorityGrouping();
	return NVIC_setPriorityIRQ(irqNumber, NVIC_encodePriority(priorityGroup, preemptPriority, subPriority));
}


/*
 * @brief	get priority of an Interrupt
 *
 * @param	irqNumber		External STM32 Interrupt number
 * @param	priorityGroup	"Priority Group" value defined in NVIC_PriorityGroup_t enum
 * @param	preemptPriority	Pointer on the Preemptive Priority value (starting from 0).
 * @param	subPriority		Pointer on the SubPriority value (starting from 0)
 */
void BLong_NVIC_getPriorityIRQ(NVIC_PriorityGroup_t priorityGroup, IRQn_t irqNumber, uint32_t* preemptPriority, uint32_t* subPriority){
	/* Get priority for Cortex-M system or device specific interrupt */
	NVIC_decodePriority(priorityGroup, NVIC_getPriorityIRQ(irqNumber), preemptPriority, subPriority);
}


/*
 * @brief	Initializes a system reset request to request MCU
 */
void BLong_NVIC_systemReset(void){
	NVIC_SystemReset(); //System Reset
}


/*
 * @brief	Initialize the System Timer/Clock and its Interrupt
 * 			Starts System Tick Timer
 * 			Counter is in free running mode to
 * @param	ticks	Reload value for the SysTick Timer
 * 					It tells SysTick "Count this many CPU Clock cycles, then generate an interrupt"
 */
bool BLong_SysTickConfig(uint32_t ticks){
	return SysTick_config(ticks);
}


/*
 * @brief	Clear Pending Events
 */
void BLong_CORTEX_clearEvent(void){
	__asm volatile ("sev");
	__asm volatile ("wfe":::"memory");
}


/*
 * @brief	Configure/choose Clock Source for SysTick
 *
 * @param	clkSource	SYSTICK_CLK_SOURCE_HCLK or SYSTICK_CLK_SOURCE_HCLK_DIV8 value in SysTick_Clk_Source_t enum
 *
 * @retVal	None
 */
void BLong_SysTick_cloclSourceConfig(SysTick_Clk_Source_t clkSource){
	if(clkSource == SYSTICK_CLK_SOURCE_HCLK) SysTick_REG -> CTRL = SYSTICK_CLK_SOURCE_HCLK;
	else SysTick_REG -> CTRL &= ~SYSTICK_CLK_SOURCE_HCLK;
}

/*
 * @brief	This function handles SysTick Interrupt request
 * @retVal	None
 */
void BLong_SysTick_IRQHandler(void){
	BLong_SysTick_callBack();
}


/*
 * @brief	SysTick callback
 * @retVal	None
 */
__attribute__((weak)) void BLong_SysTick_callBack(void){
}
