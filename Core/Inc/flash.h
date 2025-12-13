/*
 * flash.h
 *	Created on: July 18, 2025
 *  Updated on: Dec 13, 2025
 *      Author: dobao
 *
 *  Description		Header file for STM32F405 Internal FLASH Driver
 *  				Includes:
 *  					* General definitions
 *  					* Register definitions
 *  					* Configuration Macros
 *  					* Public APIs
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32f405_addr.h"
#include "general.h"
#include "BLong.h"

/* Section attribute for linker script placement */
#define RAMFUNC		__attribute__((section(".RamFunc")))
#define RAMCONST	__attribute__((section(".RamConst")))


/*
 * @brief	Configuration: Run constants from RAM.
 * @note	Set to 1	COPIES dta to RAM. SAFE.
 * 						If constants are left in FLASH, the CPU cannot read them
 * 						while the FLASH is busy erasing -> Reading from FLASH during erasing = CRASH
 *
 * @note	Set to 0	Keeps data in FLASH. Saves RAM, but unsafe for erasing.
 * 						Suitable for normal performance of FLASH
 */
#define FLASH_CONF_USE_RAM_CONST	0
#if FLASH_CONF_USE_RAM_CONST == 1
	#define FLASH_RAMCONST_ATTRIBUTE	RAMCONST
#else
	#define FLASH_RAMCONST_ATTRIBUTE
#endif


/*
 * @brief	Run Driver Functions from RAM
 * @note	Set to 1	Runs code from RAM
 *
 * @note	You cannot execute instruction from FLASH while erasing it. If set to 0, CPU tries to read
 * 			it from FLASH -> Fail.
 *
 * 			Set to 0	Code runs from FLASH. Only use this if you do not erase FLASH
 */
#define FLASH_CONF_USE_RAM_FUNC 	0
#if FLASH_CONF_USE_RAM_FUNC == 1
	#define FLASH_RAMFUNC_ATTRIBUTE		RAMFUNC
#else
	#define FLASH_RAMFUNC_ATTRIBUTE
#endif


/*
 * =======================================================================
 * 					LOOK UP TABLE OF FLASH SECTOR ADDRESSES
 * =======================================================================
 */
/* @brief	Type definition for the array of Sector Start Addresses */
typedef volatile uint32_t* Sector_Addr_t[12];

/*
 * ===========================================================
 * 			  RESERVED STARTED BITS AND MASKS
 * ===========================================================
 */
/* FLASH_ACR Reserved bits */
#define ACR_RESERVED_Pos_3		3u
#define ACR_RESERVED_Pos_13		13u
#define ACR_RESERVED_Msk		~((0x1Fu << ACR_RESERVED_Pos_3) | 	\
								(0x7FFFFu << ACR_RESERVED_Pos_13))

/* FLASH_SR Reserved bits */
#define	SR_RESERVED_Pos_2		2u
#define	SR_RESERVED_Pos_8		8u
#define	SR_RESERVED_Pos_17		17u
#define SR_RESERVED_Msk			~((0x3u << SR_RESERVED_Pos_2) |		\
								(0xFFu << SR_RESERVED_Pos_8) | 		\
								(0x7FFFu << SR_RESERVED_Pos_17))

/* FLASH_CR Reserved bits */
#define CR_RESERVED_Pos_7		7u
#define CR_RESERVED_Pos_10		10u
#define CR_RESERVED_Pos_17		17u
#define CR_RESERVED_Pos_26		26u
#define CR_RESERVED_Msk			~((0x1u << CR_RESERVED_Pos_7) | 	\
								(0x3Fu << CR_RESERVED_Pos_10) | 	\
								(0x7Fu << CR_RESERVED_Pos_17) | 	\
								(0x1Fu << CR_RESERVED_Pos_26))

/* FLASH_OPTCR Reserved bits */
#define OPTCR_RESERVED_Pos_4	4u
#define	OPTCR_RESERVED_Pos_28	28u
#define OPTCR_RESERVED_Msk		~((0x1u << OPTCR_RESERVED_Pos_4) | 	\
								(0xFu << OPTCR_RESERVED_Pos_28))


/*
 * =============================================================
 * 					BIT POSITIONS: FLASH_ACR
 * =============================================================
 */
#define LATENCY_Pos		0u		//Latency
#define	PRFTEN_Pos		8u		//Prefetch
#define	ICEN_Pos		9u		//Instruction cache
#define DCEN_Pos		10u		//Data cache
#define	ICRST_Pos		11u		//Instruction cache reset
#define	DCRST_Pos		12u		//Data cache reset


/*
 * =============================================================
 * 					BIT POSITIONS: FLASH_SR
 * =============================================================
 */
#define EOP_Pos			0u	//End of operation
#define	OPERR_Pos		1u	//Operation error
#define	WRPERR_Pos		4u	//Write Protection Error
#define	PGAERR_Pos		5u	//Programming alignment error
#define	PGPERR_Pos		6u	//Programming parallelism error
#define PGSERR_Pos		7u	//Programming sequence error
#define	BSY_Pos			16u	//Busy


/*
 * =============================================================
 * 					BIT POSITIONS: FLASH_CR
 * =============================================================
 */
#define PG_Pos		0u	//Flash programming activated
#define	SER_Pos		1u	//Sector Erase
#define MER_Pos		2u	//Mass Erase
#define SNB_Pos		3u	//Sector number
#define	PSIZE_Pos	8u	//Program size
#define STRT_Pos	16u	//Start
#define	EOPIE_Pos	24u	//End of operation interrupt enable
#define	ERRIE_Pos	25u	//Error interrupt enable
#define LOCK_Pos	31u	//Lock


/*
 * =============================================================
 * 					BIT POSITIONS OF FLASH_OPTCR
 * =============================================================
 */
#define OPTLOCK_Pos		0u	//Option lock
#define	OPTSTRT_Pos		1u	//Option start
#define	BOR_LEV_Pos		2u	//BOR reset level
#define USER_Pos		5u	//User option bytes
#define RDP_Pos			8u	//Read protect
#define	nWRP_Pos		16u	//Not write protect


typedef enum{
	_0_WS,	//0 Wait States
	_1_WS,	//...
	_2_WS,
	_3_WS,
	_4_WS,
	_5_WS,
	_6_WS,
	_7_WS	//7 Wait States
}FLASH_Latency_t;


/* @brief	FLASH sectors (STM32F405) */
typedef enum{
	SECTOR_0,	//Base @ 0x08000000 (16KB)
	SECTOR_1,	//Base @ 0x08004000 (16KB)

	SECTOR_2,	//Base @ 0x08008000 (16KB)
	SECTOR_3,	//Base @ 0x0800C000 (16KB)

	SECTOR_4,	//Base @ 0x08010000 (64KB)
	SECTOR_5,	//Base @ 0x08020000 (128KB)

	SECTOR_6,	//Base @ 0x08040000 (128KB)
	SECTOR_7,	//Base @ 0x08060000 (128KB)

	SECTOR_8,	//Base @ 0x08080000 (128KB)
	SECTOR_9,	//Base @ 0x080A0000 (128KB)

	SECTOR_10,	//Base @ 0x080C0000 (128KB)
	SECTOR_11	//Base @ 0x080E0000 (128KB)
}Sector_t;


/* @brief	FLASH Driver Status */
typedef enum{
	FLASH_ERROR		= 0x00u,
	FLASH_OK		= 0x01u,
	FLASH_BUSY		= 0x02u,
	FLASH_TIMEOUT	= 0x03u
}FLASH_Status_t;


/* @brief	FLASH register enum (for lookup table) */
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
 * 					PUBLIC APIs FOR SPECIAL FLASH OPERATION
 * ===========================================================================
 */
FLASH_RAMFUNC_ATTRIBUTE FLASH_Status_t FLASH_sectorErase(Sector_t sector);
FLASH_RAMFUNC_ATTRIBUTE FLASH_Status_t FLASH_programming(volatile uint8_t* flashDestination, uint8_t* programBuf, int bufSize);
FLASH_RAMFUNC_ATTRIBUTE void FLASH_firmwareUpdate(uint8_t* programBuf, uint32_t bufSize);


/*
 * ===========================================================================
 * 					   PUBLIC APIs FOR NORMAL FLASH OPERATION
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

#endif /* INC_FLASH_H_ */
