/*
 * flash.c
 *
 *  Created on: Nov 13, 2025
 *      Author: dobao
 */

#include "flash.h"

/*
 * @brief	Create a lookup table for all FLASH registers.
 * 			Each entry is a pointer to a real hardware register.
 *
 * @note	This table lets us access FLASH registers by enum index
 * 			instead of writing their addresses manually.
 */
static volatile uint32_t* const flashRegLookupTable[FLASH_REG_COUNT] = {
		[FLASH_ACR] = FLASH_GET_REG(FLASH_ACR),
		[FLASH_KEYR] = FLASH_GET_REG(FLASH_KEYR),
		[FLASH_OPT_KEYR] = FLASH_GET_REG(FLASH_OPT_KEYR),
		[FLASH_SR] = FLASH_GET_REG(FLASH_SR),
		[FLASH_CR] = FLASH_GET_REG(FLASH_CR),
		[FLASH_OPT_CR] = FLASH_GET_REG(FLASH_OPT_CR)
};

/*
 * @brief	A valid bit mask for each FLASH register.
 * 			'1' is allowed to be used
 * 			'0' is a reserved
 *
 * @note	This helps prevent writing to reserved bits.
 */
static const uint32_t FLASH_VALID_BIT[FLASH_REG_COUNT] = {
		[FLASH_ACR] = ~((0x7FFFFu << ACR_RESERVED_Pos_13) | (0xFu << ACR_RESERVED_Pos_4)),
		[FLASH_KEYR] = 0xFFFFFFFFu,
		[FLASH_OPT_KEYR] = 0xFFFFFFFFu,
		[FLASH_SR] = ~((0x7FFFu << 17) | (0xFFu << 8) | (0x3u << 2)),
		[FLASH_CR] = ~((0x1Fu << 26) | (0x7Fu << 17) | (0x3Fu << 10) | (1u << 7)),
		[FLASH_OPT_CR] = ~((0xFu << 28) | (1u << 4)),
};

/*
 * @brief	Check if a specific bit position in FLASH reg is valid (not reserved)
 *
 * @param	bitPosition		Bit index (0-31) to check within the given FLASH reg
 * @param	flashReg		Enum value representing FLASH register being accessed
 *
 * @retval	true	Safe to write and read
 * 			false	Bit is reserved or out of range
 */
static inline bool isValidFlashBit(uint8_t bitPosition, FLASH_Reg_t flashReg){
	return ((flashReg < FLASH_REG_COUNT) &&
			(bitPosition < 32u) &&
			(((FLASH_VALID_BIT[flashReg] >> bitPosition)) & 1u) != 0);
}


/*
 * ===========================================================================
 * 								PUBLIC APIs
 * ===========================================================================
 */
FLASH_Status_t FLASH_init(){
	/* Re-enable caches and prefetch */
	if(!FLASH_enableInstructionCache()) return FLASH_ERROR;
	if(!FLASH_enableDataCache()) return FLASH_ERROR;
	if(!FLASH_enablePrefetch()) return FLASH_ERROR;
	return FLASH_OK;
}

FLASH_Status_t FLASH_resetCache(){
	/* Before resetting caches, disable them */
	if(!FLASH_disableInstructionCache()) return FLASH_ERROR;
	if(!FLASH_disableDataCache()) return FLASH_ERROR;

	/* Then, reset them */
	if(!FLASH_resetInstructionCache()) return FLASH_ERROR;
	if(!FLASH_resetDataCache()) return FLASH_ERROR;

	/* Re-enable caches */
	if(!FLASH_enableInstructionCache()) return FLASH_ERROR;
	if(!FLASH_enableDataCache()) return FLASH_ERROR;

	return FLASH_OK;
}

bool FLASH_setLatency(FLASH_Latency_t waitState){
	return writeFLASH(LATENCY_Pos, FLASH_ACR, (uint32_t)waitState);
}

bool FLASH_enablePrefetch(){
	return writeFLASH(PRFTEN_Pos, FLASH_ACR, 1u);
}

bool FLASH_disablePrefetch(){
	return writeFLASH(PRFTEN_Pos, FLASH_ACR, 0u);
}

bool FLASH_enableInstructionCache(){
	return writeFLASH(ICEN_Pos, FLASH_ACR, 1u);
}

bool FLASH_disableInstructionCache(){
	return writeFLASH(ICEN_Pos, FLASH_ACR, 0u);
}

bool FLASH_enableDataCache(){
	return writeFLASH(DCEN_Pos, FLASH_ACR, 1u);
}

bool FLASH_disableDataCache(){
	return writeFLASH(DCEN_Pos, FLASH_ACR, 0u);
}

bool FLASH_resetInstructionCache(){
	return writeFLASH(ICRST_Pos, FLASH_ACR, 1u);
}

bool FLASH_resetDataCache(){
	return writeFLASH(DCRST_Pos, FLASH_ACR, 1u);
}

/*
 * @brief	Write a value to a bit field inside a specific FLASH register
 *
 * @param	bitPosition		LSB bit index (0-31)
 * @param	regID			FLASH register enum (Flash_Reg_t)
 * @param	value			32-bit value
 *
 * @note	Auto check bitWidth and written value to avoid writing wrong size
 * 			Blocks writes to reserved bits
 * 			Handles both 'full register write' and 'partial field write'
 */
bool writeFLASH(uint8_t bitPosition, FLASH_Reg_t regID, uint32_t value){
	/* Check that reg + bit are allowed (not reserved, in range) */
	if(!isValidFlashBit(bitPosition, regID)) return false;

	/* Auto get width of the requested bit field */
	uint8_t bitWidth = getBitWidth(PERIPH_FLASH, regID, bitPosition);

	/* Check if position/width/value fit inside 32-bits register */
	if(!sanityCheckFieldValue(bitPosition, bitWidth, value)) return false;

	/* Convert register index to a real pointer hardware register */
	volatile uint32_t* reg = flashRegLookupTable[regID];

	/* Write whole register */
	if(bitWidth == 32){
		*reg = value;
		return true;
	}

	/* Other cases */
	uint32_t mask = ((1u << bitWidth) - 1u) << bitPosition;
	uint32_t shiftedVal = (value << bitPosition) & mask;
	*reg = (*reg & ~mask) | shiftedVal;
	return true;
}

/*
 * @brief	Read a bit field from a FLASH register
 *
 * @param	bitPosition		LSB index of the field (0-31)
 * @param	regID			Register index in a Flash_Reg_t enum
 * @param	*outVal			A output value pointer to store the read value from register
 *
 * @note	Checks field width and reserved bits
 * 			Returns either full register or only requested field
 */
bool readFLASH(uint8_t bitPosition, FLASH_Reg_t regID, uint32_t* outVal){
	/* Check if reg + bit are allowed*/
	if(outVal == NULL) return false;
	if(!isValidFlashBit(bitPosition, regID)) return false;

	/* Auto get width of the requested bit field */
	uint8_t bitWidth = getBitWidth(PERIPH_FLASH, regID, bitPosition); //Get the width of bit field

	/* Sanity check */
	if(!sanityCheckField(bitPosition, bitWidth)) return false;

	/* Convert register index to a real pointer hardware register */
	volatile uint32_t* reg = flashRegLookupTable[regID];

	/* Read whole register */
	if(bitWidth == 32){
		*outVal = *reg;
		return true;
	}

	/* Otherwise, retrieve a fraction of bit field in a FLASH register */
	uint32_t mask = ((1u << bitWidth) - 1u);
	*outVal = (*reg >> bitPosition) & mask;
	return true;
}




