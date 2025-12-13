/*
 * flash.c
 *	Created on: July 18, 2025
 *  Updated on: Dec 13, 2025
 *      Author: dobao
 *
 *  Desciption:	Low-level driver for STM32F405 Internal FLASH Memory.
 *  Supports register manipulation, Sector Erase, and Firmware Programming.
 */

#include "flash.h"

/* Default programming parallellism (x8) */
uint8_t programSizeDefault = 0b00;


/*
 * @brief	Table of start address for all 12 FLASH Sector (STM32F405).
 * 			Used to determine where to begin writting or erasing.
 */
FLASH_RAMCONST_ATTRIBUTE const Sector_Addr_t FLASH_startSectorAddr = {
		(volatile uint32_t*) 0x08000000u, //Sector 0
		(volatile uint32_t*) 0x08004000u, //Sector 1

		(volatile uint32_t*) 0x08008000u, //Sector 2
		(volatile uint32_t*) 0x0800C000u, //Sector 3

		(volatile uint32_t*) 0x08010000u, //Sector 4
		(volatile uint32_t*) 0x08020000u, //Sector 5

		(volatile uint32_t*) 0x08040000u, //Sector 6
		(volatile uint32_t*) 0x08060000u, //Sector 7

		(volatile uint32_t*) 0x08080000u, //Sector 8
		(volatile uint32_t*) 0x080A0000u, //Sector 9

		(volatile uint32_t*) 0x080C0000u, //Sector 10
		(volatile uint32_t*) 0x080E0000u, //Sector 11
};


/*
 * @brief	Table of FLASH Sector sizes (in Bytes).
 * 			Used to calculate how many sectors are needed for a given firmware size.
 */
FLASH_RAMCONST_ATTRIBUTE static uint32_t const FLASH_offsetSectorAddr[12] = {
		0x00004000, // Sector 0 (16KB)
		0x00008000, // Sector 1
		0x0000C000, // Sector 2
		0x00010000, // Sector 3
		0x00020000, // Sector 4 (Start of 128KB sectors)
		0x00040000, // Sector 5
		0x00060000, // Sector 6
		0x00080000, // Sector 7
		0x000A0000, // Sector 8
		0x000C0000, // Sector 9
		0x000E0000, // Sector 10
		0x00100000  // Sector 11 (End of 1MB Flash)
};


/*
 * @brief	Lookup table for all FLASH registers.
 * 			Map FLASH_Reg_t enum to physical hardware addresses.
 * 			Each entry is a pointer to a real hardware register.
 *
 *
 * @note	This table lets us access FLASH registers by enum index
 * 			instead of writing their addresses manually.
 */
FLASH_RAMCONST_ATTRIBUTE static volatile uint32_t* const flashRegLookupTable[FLASH_REG_COUNT] = {
		[FLASH_ACR] = FLASH_GET_REG(FLASH_ACR),
		[FLASH_KEYR] = FLASH_GET_REG(FLASH_KEYR),
		[FLASH_OPT_KEYR] = FLASH_GET_REG(FLASH_OPT_KEYR),
		[FLASH_SR] = FLASH_GET_REG(FLASH_SR),
		[FLASH_CR] = FLASH_GET_REG(FLASH_CR),
		[FLASH_OPT_CR] = FLASH_GET_REG(FLASH_OPT_CR)
};


/*
 * @brief	Valid bit masks for FLASH registers.
 * 			'1' is allowed to be written/read
 * 			'0' is a reserved
 *
 * @note	This helps prevent writing to reserved bits.
 */
FLASH_RAMCONST_ATTRIBUTE static const uint32_t FLASH_VALID_BIT[FLASH_REG_COUNT] = {
		[FLASH_ACR] = ACR_RESERVED_Msk,
		[FLASH_KEYR] = 0xFFFFFFFFu,
		[FLASH_OPT_KEYR] = 0xFFFFFFFFu,
		[FLASH_SR] = SR_RESERVED_Msk,
		[FLASH_CR] = CR_RESERVED_Msk,
		[FLASH_OPT_CR] = OPTCR_RESERVED_Msk,
};


/*
 * @brief	Verifies if a specific bit position in FLASH reg is valid (not reserved)
 *
 * @param	bitPosition		Bit index (0-31) to check within the given FLASH reg
 * @param	flashReg		Target FLASH register (FLASH_Reg_t)
 *
 * @retval	true	Safe to write and read
 * 			false	Bit is reserved or out of range
 */
FLASH_RAMFUNC_ATTRIBUTE static inline bool isValidFlashBit(uint8_t bitPosition, FLASH_Reg_t flashReg){
	return ((flashReg < FLASH_REG_COUNT) &&
			(bitPosition < 32u) &&
			(((FLASH_VALID_BIT[flashReg] >> bitPosition)) & 1u) != 0);
}


/*
 * @brief	Retrieves the width (in bits) of a specific field in a FLASH register
 *
 * @param	regID				Register index (FLASH_Reg_t)
 * @param	bitPosition			LSB index of the field (0-31)
 *
 * @note	This help is used to distinguish single-bit flags from multi-bit configuration fields
 * 			Before using this function, user must not enter reserved bit position
 * 			Or before calling this function, user must have a helper to eliminate reserved bit position from entering this function
 *
 * @retval	>0	Width of the field in bits
 * 			0	Undefined, reserved, or unsupported
 */
FLASH_RAMFUNC_ATTRIBUTE static uint8_t FLASH_getBitWidth(FLASH_Reg_t regID, uint8_t bitPosition){
	uint8_t defaultBitWidth = 1;
	if(regID >= FLASH_REG_COUNT) return 0;

	switch(regID){
		case FLASH_ACR:
			if(bitPosition == 0) return 3;
			break;

		case FLASH_KEYR:
		case FLASH_OPT_KEYR:
			return (bitPosition == 0) ? 32 : 0;

		case FLASH_SR: break;

		case FLASH_CR:
			if(bitPosition == 3) return 4;
			if(bitPosition == 8) return 2;
			break;

		case FLASH_OPT_CR:
			if(bitPosition == 2) return 2;
			if(bitPosition == 8) return 8;
			if(bitPosition == 16) return 12;
			break;

		default: return 0;
		break;
	}
	return defaultBitWidth;
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
 *
 * @retVal	FLASH_OK		Write successful
 * 			FLASH_ERROR		Invalid bit position, register, or value overflow
 */
FLASH_RAMFUNC_ATTRIBUTE static FLASH_Status_t writeFLASH(uint8_t bitPosition, FLASH_Reg_t regID, uint32_t value){
	/* Check that reg + bit are allowed (not reserved, in range) */
	if(!isValidFlashBit(bitPosition, regID)) return FLASH_ERROR;

	/* Auto get width of the requested bit field */
	uint8_t bitWidth = FLASH_getBitWidth(regID, bitPosition);

	/* Check if position/width/value fit inside 32-bits register */
	if(!sanityCheckFieldValue(bitPosition, bitWidth, value)) return FLASH_ERROR;

	/* Convert register index to a real pointer hardware register */
	volatile uint32_t* reg = flashRegLookupTable[regID];

	/* Special case FLASH_SR write 1 to clear and 0 means do nothing */
	if(regID == FLASH_SR){
		/* Just write '1' to the specific bit we want to clear */
		/* Do NOT read the register first. Do NOT use OR/AND operators */
		if(value == 1){
			*reg = (1u << bitPosition);
		}
		return FLASH_OK;
	}

	/* Write whole register */
	if(bitWidth == 32){
		*reg = value;
		return FLASH_OK;
	}

	/* Read-Modify-Write for standard fields */
	uint32_t mask = ((1u << bitWidth) - 1u) << bitPosition;
	uint32_t shiftedVal = (value << bitPosition) & mask;
	*reg = (*reg & ~mask) | shiftedVal;
	return FLASH_OK;
}


/*
 * @brief	Read a specific bit field from a FLASH register
 *
 * @param	bitPosition		LSB index of the field (0-31)
 * @param	regID			Register index in a Flash_Reg_t enum
 *
 * @retVal	The value of requested field (shifted to LSB 0)
 * 			0xFFFFFFFF	Read error (invalid bit or register)
 */
FLASH_RAMFUNC_ATTRIBUTE static uint32_t readFLASH(uint8_t bitPosition, FLASH_Reg_t regID){
	uint32_t READ_ERROR = 0xFFFFFFFFu;
	if(!isValidFlashBit(bitPosition, regID)) return READ_ERROR;

	/* Auto get width of the requested bit field */
	uint8_t bitWidth = FLASH_getBitWidth(regID, bitPosition); //Get the width of bit field

	/* Sanity check */
	if(!sanityCheckField(bitPosition, bitWidth)) return READ_ERROR;

	/* Convert register index to a real pointer hardware register */
	volatile uint32_t* reg = flashRegLookupTable[regID];

	/* Read whole register */
	if(bitWidth == 32){
		return *reg;
	}

	/* Otherwise, retrieve a fraction of bit field in a FLASH register */
	uint32_t mask = ((1u << bitWidth) - 1u);
	return (*reg >> bitPosition) & mask;
}


/*
 * ===================================================================================
 * 						PUBLIC APIs FOR SPECIAL FLASH PROGRAMMING
 * ===================================================================================
 */

/*
 * @brief	Clears all error flags in the FLASH Status Register
 *
 * @note	Must be called before starting any new Sector Erase or Program operation
 */
FLASH_RAMFUNC_ATTRIBUTE static void FLASH_clearErrorFlags(void){
	FLASH_Reg_t reg = FLASH_SR;
	(void)writeFLASH(EOP_Pos, reg, 1u); 		//Clear "End of operation"
	(void)writeFLASH(OPERR_Pos, reg, 1u);		//Clear "Operation error"
	(void)writeFLASH(WRPERR_Pos, reg, 1u);	//Clear "Write protection error"
	(void)writeFLASH(PGAERR_Pos, reg, 1u);	//Clear "Programming alignment error"
	(void)writeFLASH(PGPERR_Pos, reg, 1u);	//Clear "Programming parallelism error"
	(void)writeFLASH(PGSERR_Pos, reg, 1u);	//Clear "Programming sequence error"
}


/*
 * @brief	Erase a specific FLASH Sector
 *
 * @param	sector		The sector index (SECTOR_0 to SECTOR_11). The value of Sector_t enum
 *
 * @note	This function is blocking. It waits for the BSY bit to be free
 * 			Interrupts should be disabled if this funciton is executed from FLASH
 *
 * @retVal	FLASH_OK		Erase successful
 * 			FLASH_ERROR		Invalid sector index or hardware failure
 */
FLASH_RAMFUNC_ATTRIBUTE FLASH_Status_t FLASH_sectorErase(Sector_t sector){
	/* Check if @param sector is in the valid range */
	if(!((sector >= 0u) && (sector <= 11u))) return FLASH_ERROR;

	/* Clear previous errors */
	FLASH_clearErrorFlags();

	/* Wait until there is no FLASH memory operation in progress */
	while((readFLASH(BSY_Pos, FLASH_SR) & 1u) == 1u);

	/* Check if the FLASH is blocked */
	if((readFLASH(LOCK_Pos, FLASH_CR) & 1u) == 1u){
		writeFLASH(0, FLASH_KEYR, 0x45670123u);
		writeFLASH(0, FLASH_KEYR, 0xCDEF89ABu);
	}

	writeFLASH(SER_Pos, FLASH_CR, 1u);				 	//Sector Erase Activated
	writeFLASH(SNB_Pos, FLASH_CR, (uint32_t)sector); 	//Select sector that want to be deleted
	writeFLASH(STRT_Pos, FLASH_CR, 1u); 			 	//Start
	while((readFLASH(BSY_Pos, FLASH_SR) & 1u) == 1u);	//Wait for BUSY bit to be cleared
	writeFLASH(SER_Pos, FLASH_CR, 0u);					//Clear SER bit manually as HW does not do it

	return FLASH_OK;
}


/*
 * @brief	Programs a buffer of data into FLASH memory.
 *
 * @param	flashDestination	Pointer to the FLASH address where writing begins.
 * @param	programBuf			Pointer to the source data buffer.
 * @param	bufSize				Size of buffer to write.
 *
 * @note	This function uses Byte-to-Byte programming (x8 parallelism).
 * 			Ensure the supply voltage supports the selected parallelism.
 *
 * @retVal	FLASH_OK		Programming successful.
 * @retVal	FLASH_ERROR		Null Pointers, zero size, write failure.
 */
FLASH_RAMFUNC_ATTRIBUTE FLASH_Status_t FLASH_programming(volatile uint8_t* flashDestination, uint8_t* programBuf, int bufSize){
	/* Sanity Check */
	if((flashDestination == NULL) || (programBuf == NULL) || bufSize == 0) return FLASH_ERROR;

	/* Clear previous errors */
	FLASH_clearErrorFlags();

	/* Wait until there is no FLASH memory operation in progress */
	while((readFLASH(BSY_Pos, FLASH_SR) & 1u) == 1u);

	/* Check if the FLASH is locked */
	if((readFLASH(LOCK_Pos, FLASH_CR) & 1u) == 1u){
		/* Unlock the FLASH by writing these sequences */
		writeFLASH(0, FLASH_KEYR, 0x45670123u); //Sequence 1
		writeFLASH(0, FLASH_KEYR, 0xCDEF89ABu); //Sequence 2
	}

	writeFLASH(PG_Pos, FLASH_CR, 1u); //Activate FLASH programming
	writeFLASH(PSIZE_Pos, FLASH_CR, programSizeDefault); //Set x8-bit program parallelism

	for(int i = 0; i < bufSize; i++){
		flashDestination[i] = programBuf[i];
		while((readFLASH(BSY_Pos, FLASH_SR) & 1u) == 1u); //Wait until BUSY bit to be cleared
	}

	writeFLASH(PG_Pos, FLASH_CR, 0u);	//Deactivate programming
	writeFLASH(LOCK_Pos, FLASH_CR, 1u);	//Lock the FLASH again for protection

	return FLASH_OK;
}


/*
 * @brief	Performs a complete Firmware Update.
 *
 * @param	programBuf	Pointer to the new firmware data.
 * @param	bufSize		Size of the new firmware in bytes.
 *
 * @note	Calculate the necessary sectors to erase based on @p bufSize.
 * 			Erases sectors from 0 up to the calculated target sector
 * 			Programs the new data starting at Sector 0
 * 			Reset the system (NVIC_SystemReset) upon completion
 *
 * @warning		If the bufSize > total FLASH memory, by default, it erases all 11 sectors
 */
FLASH_RAMFUNC_ATTRIBUTE void FLASH_firmwareUpdate(uint8_t* programBuf, uint32_t bufSize){
	/* The final reached sector which enough to store the bufsize */
	uint8_t targetSector = 11; //Erase everything if nothing match for safety

	/* Find the target sector */
	for(uint8_t i = 0; i < 12; i++){
		if(bufSize <= FLASH_offsetSectorAddr[i]){
			targetSector = i;
			break; //Stop check higher sectors!
		}
	}

	/* Start erasing from sector 0 to @p targetSector */
	for(uint8_t s = 0; s <= targetSector; s++){
		FLASH_sectorErase(s);
	}

	/* Start programming */
	FLASH_programming((volatile uint8_t*)FLASH_startSectorAddr[0], programBuf, bufSize);

	/* NVIC System Reset */
	BLong_NVIC_systemReset();
}


/*
 * ===========================================================================
 * 					PUBLIC APIs FOR NORMAL FLASH OPERATION
 * ===========================================================================
 */
/*
  @brief	Initializes FLASH caches, and prefetch buffer
  @retVal	FLASH_OK 		Successful
  	  	  	FLASH_ERROR		Fail
 */
FLASH_Status_t FLASH_init(){
	/* Re-enable caches and prefetch */
	if(!FLASH_enableInstructionCache()) return FLASH_ERROR;
	if(!FLASH_enableDataCache()) return FLASH_ERROR;
	if(!FLASH_enablePrefetch()) return FLASH_ERROR;
	return FLASH_OK;
}


/*
 * @brief	Resets Instruction and Data Caches.
 *
 * @note	Caches must be disabled before resetting.
 *
 * @retVal	FLASH_OK	Successful
 * 			FLASH_ERROR	Fail
 */
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


/*
 * @brief	Sets the FLASH Access Latency (Wait States)
 *
 * @param	waitState	Number of wait states (FLASH_Latency_t)
 *
 * @retVal	true it write success
 *
 */
bool FLASH_setLatency(FLASH_Latency_t waitState){
	return writeFLASH(LATENCY_Pos, FLASH_ACR, (uint32_t)waitState);
}

/* @brief	Enable Prefetch buffer */
bool FLASH_enablePrefetch(){
	return writeFLASH(PRFTEN_Pos, FLASH_ACR, 1u);
}

/* @brief	Disable Prefetch buffer */
bool FLASH_disablePrefetch(){
	return writeFLASH(PRFTEN_Pos, FLASH_ACR, 0u);
}

/* @brief	Enable Instruction Cache */
bool FLASH_enableInstructionCache(){
	return writeFLASH(ICEN_Pos, FLASH_ACR, 1u);
}

/* @brief	Disable Instruction Cache */
bool FLASH_disableInstructionCache(){
	return writeFLASH(ICEN_Pos, FLASH_ACR, 0u);
}

/* @brief	Enable Data Cache */
bool FLASH_enableDataCache(){
	return writeFLASH(DCEN_Pos, FLASH_ACR, 1u);
}

/* @brief	Disable Data Cache */
bool FLASH_disableDataCache(){
	return writeFLASH(DCEN_Pos, FLASH_ACR, 0u);
}

/* @brief	Reset Instruction Cache */
bool FLASH_resetInstructionCache(){
	return writeFLASH(ICRST_Pos, FLASH_ACR, 1u);
}

/* @brief	Rset Data Cache */
bool FLASH_resetDataCache(){
	return writeFLASH(DCRST_Pos, FLASH_ACR, 1u);
}






