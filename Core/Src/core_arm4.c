/*
 * core_arm4.c
 *
 *  Created on: Nov 16, 2025
 *      Author: dobao
 */

#include "core_arm4.h"


/*
 * @brief	Set Application Interrupt and Reset Control Register (AIRCR)
 * 			AIRCR is located in SCB (System Control Block)
 * 			It same as global interrupt + reset + endianness
 * @note	To modify AIRCR, VECTKEY (0x5FA) must be written
 */
void NVIC_setPriorityGrouping(NVIC_PriorityGroup_t priorityGrouping){
	uint32_t regVal = 0;
	uint32_t priorityGroupingTemp = (priorityGrouping & (uint32_t)0x7u);

	regVal = SCB_REG -> AIRCR;
	regVal &= ~((SCB_AIRCR_VECTKEY_Msk_) | (SCB_AIRCR_PRIGROUP_Msk_));
	regVal = (regVal | ((uint32_t)0x5FAu << SCB_AIRCR_VECTKEY_Pos_) | (priorityGroupingTemp << SCB_AIRCR_PRIGROUP_Pos_));
	SCB_REG -> AIRCR = regVal;
}


/*
 * @brief	Get Priority Grouping
 * 			Reads the priority grouping field from the NVIC Interrupt Controller
 *
 * @return	Priority grouping field (SCB -> AIRCR [10:8] priority field)
 */
#if 0
uint32_t NVIC_getPriorityGrouping(void){
	return ((uint32_t)(((SCB_REG -> AIRCR) & SCB_AIRCR_PRIGROUP_Msk_) >> SCB_AIRCR_PRIGROUP_Pos_));
}
#else
NVIC_PriorityGroup_t NVIC_getPriorityGrouping(void){
	return ((uint32_t)(((SCB_REG -> AIRCR) & SCB_AIRCR_PRIGROUP_Msk_) >> SCB_AIRCR_PRIGROUP_Pos_));
}
#endif


/*
 * @brief	Enable an interrupt in the NVIC
 * @note	Adding __asm volatile("" ::: "memory") to prevent compiler from reordering memory access around the NVIC register write.
 * @param	irqNumber	The IRQ number as defined in the vector table (0-81)
 * @retVal	true	OK
 * 			false	STH is wrong
 */
bool NVIC_enableIRQ(IRQn_t irqNumber){
	if(((int32_t)irqNumber < 0) || ((int32_t)irqNumber >= IRQn_COUNT)) return false;

	uint32_t idx 	= ((uint32_t)irqNumber) / 32; //Determine which ISER reg (ISER0 or ISER1...ISER7)
	uint32_t bitPos = ((uint32_t)irqNumber) % 32;	//Determine which bit inside that ISER reg
	__asm volatile("" ::: "memory"); //Same as __COMPILER_BARRIER()
	NVIC_REG -> ISER_[idx] = (1u << bitPos);	//Set the corresponding bit to enable IRQ
	__asm volatile("" ::: "memory"); //Same as __COMPILER_BARRIER()
	return true;
}


/*
 * @brief	Get the enable status of an external IRQ in the NVIC (ISER)
 *
 * @param	irqNumber	The IRQ number as defined in the vector table (0-81)
 * @param	irqStatus	Output Pointer receiving the status
 * 						0 = Interrupt disabled
 * 						1 = Interrupt enabled
 *
 * @retval	true	OK
 * 			false	STH is wrong
 */
bool NVIC_getEnableIRQ(IRQn_t irqNumber, uint32_t* irqStatus){
	if((irqStatus == NULL) || ((int32_t)irqNumber < 0) || ((int32_t)irqNumber >= IRQn_COUNT)) return false;

	uint32_t idx 		= ((uint32_t)irqNumber) / 32;	//Determine which ISER reg (ISER0 or ISER1...ISER7)
	uint32_t bitPos 	= ((uint32_t)irqNumber) % 32;	//Determine which bit inside that ISER reg

	*irqStatus = (uint32_t)(((NVIC_REG -> ISER_[idx]) >> bitPos) & 1u); //Retrieve only one/lowest bit after shifting

	return true;
}


/*
 * @brief	Disable an interrupt in the NVIC
 *
 * @param	irqNumber	The IRQ number as defined in the vector table (0-81)
 *
 * @retVal	true	OK
 * 			false	STH is wrong
 */
bool NVIC_disableIRQ(IRQn_t irqNumber){
	if(((int32_t)irqNumber < 0) || ((int32_t)irqNumber >= IRQn_COUNT)) return false;

	uint32_t idx 		= ((uint32_t)irqNumber) / 32;	//Determine which ISER reg (ISER0 or ISER1...ISER7)
	uint32_t bitPos 	= ((uint32_t)irqNumber) % 32;	//Determine which bit inside that ISER reg
	NVIC_REG -> ICER_[idx] = (1u << bitPos); 		//Set the corresponding bit to disable IRQ

	/*
	 * Data Synchronization Barrier (DSB): Finish all previous & current memory writes before continuing a new one
	 * Instruction Synchronization Barrier (ISB): CPU must reload next instruction fresh / forget the old instructions
	 */
	__asm volatile ("dsb 0xF" ::: "memory");
	__asm volatile ("isb 0xF" ::: "memory");

	return true;
}


/*
 * @brief	Get status of Pending Interrupt register (ISPR) in the NVIC
 *
 * @param	irqNumber	The IRQ number defined in Vector Table (0-81)
 *
 * @retVal	0	Interrupt status is not pending
 * 			1	Interrupt status is pending.
 */
uint32_t NVIC_getPendingIRQ(IRQn_t irqNumber){
	if(((int32_t)irqNumber < 0) || ((int32_t)irqNumber >= IRQn_COUNT)) return 0u;

	uint32_t idx	= ((uint32_t)irqNumber) >> 5u;
	uint32_t bitPos	= ((uint32_t)irqNumber) & 0x1Fu;
	uint32_t mask	= 1u << bitPos;
	return ((uint32_t)(((NVIC_REG -> ISPR_[idx]) & mask) != 0u) ? 1u : 0u);
}


/*
 * @brief	Set Pending Interrupt register in the NVIC
 * 			Purpose: Force an Interrupt (IRQ) into the "pending" state"
 * 			It means "This interrupt needs attention - please run the ISR when possible"
 *
 * @param	irqNumber	The IRQ number as defined in the Vector Table (0-81)
 *
 * @note	This function applies proper shifting and mask instead of '/32' and '%32' like functions above.
 * 			This change makes the code run faster.
 *
 * @retVal	true	OK
 * 			false	STH is wrong
 */
bool NVIC_setPendingIRQ(IRQn_t irqNumber){
	if(((int32_t)irqNumber < 0) || ((int32_t)irqNumber >= IRQn_COUNT)) return false;
	NVIC_REG -> ISPR_[(((uint32_t)irqNumber) >> 5u)] = (uint32_t)(1u << (((uint32_t)irqNumber) & 0x1Fu)); // >> 5 same as / 32, &0x1F same as % 32
	return true;
}


/*
 * @brief	Clear Pending Interrupt (ICPR) in the NVIC
 * 			Purpose: Clear the pending state of an interrupt
 * 			Reason:
 * 				Remove a pending interrupt before it executes
 * 				Clean up after an interrupt has been handled
 * 				Cancel a software-triggered interrupt
 *
 * @param	irqNumber	The IRQ number defined in Vector Table (0-81)
 *
 * @retVal	true		OK
 * 			false		STH is wrong
 */
bool NVIC_clearPendingIRQ(IRQn_t irqNumber){
	if(((int32_t)irqNumber < 0) || ((int32_t)irqNumber >= IRQn_COUNT)) return false;
	NVIC_REG -> ICPR_[(((uint32_t)irqNumber) >> 5u)] = (uint32_t)(1u << (((uint32_t)irqNumber) & 0x1Fu)); // >> 5 same as / 32, &0x1F same as % 32
	return true;
}


/*
 * @brief	Read Interrupt Active Bit Register
 * 			"Is this interrupt currently executing on the CPU right now?"
 *
 * @retVal	0: NOT running
 * 			1: This interrupt's ISR is running
 */
uint32_t NVIC_getActiveInterrupt(IRQn_t irqNumber){
	if(((int32_t)irqNumber < 0) || ((int32_t)irqNumber >= IRQn_COUNT)) return 0u;

	uint32_t idx = ((uint32_t)irqNumber) >> 5u;
	uint32_t bitPos = ((uint32_t)irqNumber) & 0x1Fu;
	uint32_t mask = 1u << bitPos;

	return (((NVIC_REG -> IABR_[idx]) & mask) != 0) ? 1u : 0u;
}


/*
 * @brief	Set interrupt priority for both:
 * 				External interrupts (IRQn >= 0) (NVIC -> IPR[])
 * 				System exceptions	(IRQn < 0)	(SCB -> SHP[])
 * @note	Priority values in range of 0(highest priority) - 15 (lowest priority)
 * 			Priority encoding rule:
 * 				ARM cortex M4 (STM32F405...) implements 4 priority bits
 * 				These bits occupy the upper bits of a 8-bit priority byte
 * 				Example for F405:
 * 					priority << (8 - NVIC_PRIO_BITS)
 * 					NVIC_PRIO_BITS = 4 -> shift = 8 - 4 = 4 -> The actual priority is stored in [7:4]
 * 			Since Reset, HardFault and NonMaskableInt are fixed negative priority, so we excluded them in this function
 *
 * @retval	true	Success
 * 			false	Invalid IRQ number
 */
bool NVIC_setPriorityIRQ(IRQn_t irqNumber, uint32_t priority){
	/* STM32F4: 4 bits priority -> logical range 0 - 15 */
	if(priority > 15u) return false;

	if((int32_t)irqNumber >= 0){
		/* External STM32 Interrupt upper bound check */
		if((uint32_t)irqNumber < (uint32_t)IRQn_COUNT){
			NVIC_REG -> IPR_[(uint32_t)irqNumber] = (uint8_t)((priority << (8u - NVIC_PRIO_BITS)) & (uint32_t)0xFFu);
			return true;
		}
	}
	/* Set priority for system exceptions which has IRQn < 0 */
	else{
		if(((int32_t)irqNumber >= MemoryManagement_IRQn_) && ((int32_t)irqNumber <= SysTick_IRQn_)){
			/* SysTick_IRQn = -1
			 * Then, C converts signed "-1" to uint32_t which is 0xFFFF-FFFF-FFFF-FFFF...
			 * 0xFFFF-FFFF-FFFF-FFFF... & 0xF = 0b1111
			 * 0b1111 - 4u = index in SHP (System Handler Priority)
			 * SHP[] index = Exception - 4. Because SHP[0] corresponding to exception #4 */
			SCB_REG -> SHPR[(((uint32_t)irqNumber) & 0xFu) - 4u] = (uint8_t)((priority << (8u - NVIC_PRIO_BITS)) & 0xFFu);
			return true;
		}
	}
	return false;
}


/*
 * @brief	Get Exception Priority
 * @note	Remember to input the valid IRQ numbers
 * @return	uint32_t logical priority value (0 - 15 on STM32F405)
 */
uint32_t NVIC_getPriorityIRQ(IRQn_t irqNumber){
	/* External Interrupts */
	if(((int32_t)irqNumber) >= 0){
		return((uint32_t)NVIC_REG -> IPR_[(uint32_t)irqNumber] >> (8u - NVIC_PRIO_BITS));
	}
	/* System Exception */
	else{
		if((int32_t)irqNumber >= MemoryManagement_IRQn_){
			return((uint32_t)SCB_REG -> SHPR[(((uint32_t)irqNumber) & 0xFu) - 4u] >> (8u - NVIC_PRIO_BITS));
		}
	}
	return 0xFFFFFFFFu; //Means IRQ number is out of range
}


/*
 * @brief	Encode Pre-emption & Sub Priority to form a single value in NVIC priority value
 *
 * @note	priorityGrouping = NVIC_PRIORITYGROUP_0_ (0x7) to NVIC_PRIORITYGROUP_4_ (0x3)
 * 			NVIC_PRIO_BITS = total implemented priority bit in STM32F4 which is 4
 *
 * @param	priorityGrouping	Uses predefined value in enum NVIC_PriorityGroup_t
 * @param	preemptPriority 	Preemptive Priority value (starting from 0)
 * @param	subPriority			Sub Priority value (starting from 0)
 */
uint32_t NVIC_encodePriority(NVIC_PriorityGroup_t priorityGrouping, uint32_t preemptPriority, uint32_t subPriority){
	uint32_t priorityGroupingTemp = ((uint32_t)priorityGrouping) & ((uint32_t)0x7u);
	uint32_t preemptPriorityBits;
	uint32_t subPriorityBits;
	/*
	 * PriorityGroup0 = 7 (0 bit pre-emption, 4 subpriority)
	 * PriorityGroup1 = 6 (1 bit pre-emption, 3 subpriority)
	 * PriorityGroup2 = 5 (2 bit pre-emption, 2 subpriority)
	 * PriorityGroup3 = 4 (3 bit pre-emption, 1 subpriority)
	 * PriorityGroup4 = 3 (4 bit pre-emption, 0 subpriority)
	 */
	/* Determine how many bits go to Pre-emption Priority */
	preemptPriorityBits = (((7u - priorityGroupingTemp) > NVIC_PRIO_BITS) ? (uint32_t)(NVIC_PRIO_BITS) : (uint32_t)(7u - priorityGroupingTemp));

	/* Determine how many bits go to Sub Priority */
	subPriorityBits = (((priorityGroupingTemp + NVIC_PRIO_BITS) < 7u) ? (uint32_t)(0u) : (uint32_t)(priorityGroupingTemp - 7u + NVIC_PRIO_BITS));

	/* Build the final value that combines [Pre-emption Priority][Sub Priority]	 */
	return(((preemptPriority & (uint32_t)((1u << preemptPriorityBits) - 1u)) << subPriorityBits) |
			(subPriority & (uint32_t)((1u << subPriorityBits) - 1u)));
}


/*
 * @brief	Decode an encoded NVIC priority value into two parts [pre-emption priority][sub priority] according to the selected Priority Grouping
 * @note	This function reverses the encoding performed by NVIC_encodePriority.
 * @param	priorityGrouping	The grouping value predefined in enum NVIC_PriorityGroup_t
 * @param	priority			Encoded priority value written in NVIC -> IPR
 * 								For STM32F4 (4 priority bits), ranging from 0 - 15
 * @param	preemptPriority		Output Pointer: retrieves extracted pre-emption priority value based on the number of pre-emption bits
 * @param	subPriority			Output Pointer: retrieves extracted sub priority value based on the number of sub priority bits
 */
void NVIC_decodePriority(NVIC_PriorityGroup_t priorityGroup, uint32_t priority, uint32_t* const preemptPriority, uint32_t* const subPriority){
	uint32_t priorityGroupTemp = ((uint32_t)priorityGroup) & ((uint32_t)0x07u);
	uint32_t preemptPriorityBits;
	uint32_t subPriorityBits;

	/* Determine the number of pre-emption priority and sub priority bits */
	preemptPriorityBits = (((7u - priorityGroupTemp) > (uint32_t)(NVIC_PRIO_BITS)) ? ((uint32_t)NVIC_PRIO_BITS) : (7u - priorityGroupTemp));
	subPriorityBits = (((priorityGroupTemp + (uint32_t)NVIC_PRIO_BITS) < 7u) ? (uint32_t)(0u) : ((uint32_t)(priorityGroupTemp - 7u) + (uint32_t)NVIC_PRIO_BITS));

	/* Extract preemption priority value */
	*preemptPriority = (priority >> subPriorityBits) & (uint32_t)((1u << preemptPriorityBits) - 1u);

	/* Extract sub priority value */
	*subPriority = (priority) & (uint32_t)((1u << subPriorityBits) - 1u);
}


/*
 * @brief	The function updates the new user's handler function address to the correct vector[index]
 * 			This function DOES NOT ACCEPT Cortex-M Exceptions but only External STM32 Interrupts
 *
 * @MUST	REALLOCATE VECTOR TABLE FROM FLASH TO RAM BEFORE USING THIS FUNCTION
 *
 * @param	irqNumber			External STM32 Interrupt only, Cortex-M exceptions (negative IRQn) are rejected for safety
 * @param	newIRQHandlerAddr	Function pointer to the new interrupt handler function
 *
 * @note	You can find a similar function in CMSIS with the name NVIC_setVector()
 * 			VectorTableIndex = IRQn + 16
 */
bool NVIC_changeExternalIRQHandlerAddr(IRQn_t irqNumber, void (*newIRQHandlerAddr)(void)){
	/* Avoid modifying exception handlers (HardFault, MemManage, BusFault) so irqNumber must be >= 0 and less than IRQn_COUNT */
	if(((int32_t)irqNumber >= IRQn_COUNT) || ((int32_t)irqNumber < 0) || (newIRQHandlerAddr == NULL)) return false;

	/* Pointer to current vector table base address (MUST POINT TO RAM) */
	uint32_t* vectors = (uint32_t*)SCB_REG -> VTOR; //Stores the address of Vector Offset Register

	/* Store the new handler address to the correct vector index */
	vectors[(uint32_t)irqNumber + NVIC_USER_IRQn_OFFSET] = (uint32_t)newIRQHandlerAddr;
	return true;
}


/*
 * @brief	The function returns the new-changed handler's address of External STM32 Interrupts only
 * @param	irqNumber	External STM32 Interrupts
 */
uint32_t NVIC_getExternalIRQHandlerAddr(IRQn_t irqNumber){
	if(((int32_t)irqNumber >= 0) && ((int32_t)irqNumber < IRQn_COUNT)){
		uint32_t* vectors = (uint32_t*)SCB_REG -> VTOR;
		return (uint32_t)(vectors[(uint32_t)irqNumber + NVIC_USER_IRQn_OFFSET]);
	}
	return 0u; //User has entered Cortex-M exceptions -> failed/Error
}


/*
 * @brief	The function updates the new user's handler function address to the correct vector[index]
 * 			This function ACCEPTS BOTH Cortex-M Exceptions and External STM32 Interrupts
 *
 * @MUST	REALLOCATE VECTOR TABLE FROM FLASH TO RAM BEFORE USING THIS FUNCTION
 *
 * @param	irqNumber			External STM32 Interrupt and Cortex-M exceptions (negative IRQn)
 * @param	newIRQHandlerAddr	Function pointer to store the address of a function
 *
 * @note	You can find a similar function in CMSIS with the name NVIC_setVector()
 * 			VectorTableIndex = IRQn + 16
 */
bool NVIC_changeExceptionIRQHandlerAddr(IRQn_t irqNumber, void (*newIRQHandlerAddr)(void)){
	if(((int32_t)irqNumber >= IRQn_COUNT) || (newIRQHandlerAddr == NULL)) return false;

	/* Pointer to current vector table address (MUST BE IN RAM) */
	uint32_t* vectors = (uint32_t*)SCB_REG -> VTOR; //Stores the address of Vector Offset Register

	/* Store the new handler address to the correct vector index */
	vectors[(int32_t)irqNumber + NVIC_USER_IRQn_OFFSET] = (uint32_t)newIRQHandlerAddr;
	return true;
}


/*
 * @brief	The function returns the new-changed handler's address of BOTH Cortex-M Exceptions and External STM32 Interrupts
 * @param	irqNumber	External STM32 Interrupts and Cortex-M exceptions Interrupts
 */
uint32_t NVIC_getExceptionIRQHandlerAddr(IRQn_t irqNumber){
	if(((int32_t)irqNumber < IRQn_COUNT)){
		uint32_t* vectors = (uint32_t*)SCB_REG -> VTOR;
		return (uint32_t)(vectors[(int32_t)irqNumber + NVIC_USER_IRQn_OFFSET]);
	}
	return 0; //irqNumber larger than IRQn_COUNT and smaller than NonMaskableInt_IRQn_ -> failed/error
}


/*
 * @brief	Whole System Reset
 * @note	No return after running this caller
 * 			After resetting, priority grouping is preserved
 */
__attribute__((__noreturn__)) void NVIC_SystemReset(void){
	__asm volatile ("dsb 0xF" ::: "memory"); //Complete all outstanding memory operations before requesting reset
	SCB_REG -> AIRCR = (uint32_t)((0x5FAu << SCB_AIRCR_VECTKEY_Pos_) |
								 ((SCB_REG -> AIRCR) & SCB_AIRCR_PRIGROUP_Msk_) |
								 SCB_AIRCR_SYSRESETREQ_Msk_);
	__asm volatile ("dsb 0xF" ::: "memory"); //Ensure write completes

	for(;;){
		__asm volatile ("nop");
	}
}


/*
 * @brief	Configuring SysTick timer to generate an interrupt every <ticks> CPU cycles
 *
 * 			Example:
 * 				if CPU clock = 168MHz
 * 				ticks = 168M/1000 = 168,000 -> SysTick Interrupt every 1ms
 * @param	ticks	Number of CPU cycles per SysTick period.
 *
 * @retVal 	true	Ok
 * 			false	ticks value is larger than 24-bit value
 */
bool SysTick_config(uint32_t ticks){
	if((ticks - 1u) > SysTick_LOAD_RELOAD_Msk_){ //24 bits counter so the mask is 0xFFFFFF
		return false;
	}

	SysTick_REG -> RELOAD = (uint32_t)(ticks - 1u);
	if(!NVIC_setPriorityIRQ(SysTick_IRQn_, (1u << NVIC_PRIO_BITS) - 1u)) return false; //Set priority "15" for SysTick Interrupt

	SysTick_REG -> CURVAL = 0u;								//Load the SysTick Counter value
	SysTick_REG -> CTRL = (SysTick_CTRL_CLKSOURCE_Msk_ |	//Use CPU Clock
						   SysTick_CTRL_TICKINT_Msk_ |	 	//Enable SysTick Interrupt
						   SysTick_CTRL_ENABLE_Msk_);    	//Enable SysTick Timer

	return true;
}


