/*
 * general.c
 *
 *  Created on: Nov 14, 2025
 *      Author: dobao
 */

#include "general.h"

/*
 * @brief	Check whether a bit field fits inside a 32-bit register.
 *
 * @param	bitPosition		LSB index of the field (0-31)
 * @param	bitWidth		Width of the field in bits (> 0)
 *
 * @retval	true	When @p bitPosition and @p bitWidth is valid
 * 			false	The field is zero-width or extends beyond bit 31
 */
bool sanityCheckField(uint8_t bitPosition, uint8_t bitWidth){
	return (bitPosition < 32u) &&
		   ((bitWidth > 0u) && (bitWidth <= 32)) &&
		   ((bitPosition + bitWidth) <= 32u);
}

/*
 * @brief	Extented version of @f sanityCheckField
 * 			It checkes whether a bit field fits inside a 32-bit register
 * 			Also check if @p value is valid in the bit field.
 *
 * @param	bitPosition		LSB index of the field (0-31)
 * @param	bitWidth		Width of the field in bits (>0)
 * @param	value			a 32-bit value fit in the bit field
 */
bool sanityCheckFieldValue(uint8_t bitPosition, uint8_t bitWidth, uint32_t value){
	if(!sanityCheckField(bitPosition, bitWidth)) return false;

	if(bitWidth == 32u) return true;

	uint32_t mask = (1u << bitWidth) - 1u;
	return (value & ~mask) == 0; //~mask zeros valid bits and keeps invalid bits to 1
	//value & ~mask extract the only bits that should NOT be present
}

/*
 * @brief	Get the logical width of a field in a peripheral register.
 *
 * @param	whichPeripheral		Peripheral group (e.g. PERIPH_FLASH, PERIPH_RCC, etc.)
 * @param	regID				Register index within that peripheral (castable to the corresponding *_Reg_t enum)
 * @param	bitPosition			LSB index of the field (0-31)
 *
 * @retval	>0	Width of the field in bits
 * 			0	Undefined, reserved, or unsupported
 */
uint8_t getBitWidth(Peripheral_t whichPeripheral, uint8_t regID, uint8_t bitPosition){
	switch(whichPeripheral){
		/* FLASH peripheral */
		case PERIPH_FLASH:{
			FLASH_Reg_t reg = (FLASH_Reg_t) regID;
			if(reg >= FLASH_REG_COUNT) return 0;
			switch(reg){
				case FLASH_ACR:
					if(bitPosition == 0) return 3;
					if((bitPosition >= 8) && (bitPosition <= 12)) return 1;
					return 0;

				case FLASH_KEYR:
				case FLASH_OPT_KEYR:
					return (bitPosition == 0) ? 32 : 0;

				case FLASH_SR: return 1;

				case FLASH_CR:
					if((bitPosition <= 2) ||
					   (bitPosition == 16) ||
					   (bitPosition == 25) ||
					   (bitPosition == 24) ||
					   (bitPosition == 31)) return 1;
					if(bitPosition == 3) return 4;
					if(bitPosition == 8) return 2;
					return 0;

				case FLASH_OPT_CR:
					if(bitPosition == 2) return 2;
					if(bitPosition == 8) return 8;
					if(bitPosition == 16) return 12;
					if((bitPosition == 0) ||
					   (bitPosition == 1) ||
					   (bitPosition == 5) ||
					   (bitPosition == 6) ||
					   (bitPosition == 7)) return 1;
					return 0;

				default: return 0;
				break;
			}
		}

		case PERIPH_RCC:{
			RCC_Reg_t reg = (RCC_Reg_t) regID;
			switch(reg){
				case RCC_CR:

					return 0;

				case RCC_PLL_CFGR:

					return 0;

				case RCC_CFGR:

					return 0;

				case RCC_CIR:

					return 0;

				case RCC_AHB1_RSTR:

					return 0;

				case RCC_AHB2_RSTR:

					return 0;

				case RCC_AHB3_RSTR:

					return 0;

				case RCC_APB1_RSTR:

					return 0;

				case RCC_APB2_RSTR:

					return 0;

				case RCC_AHB1_ENR:

					return 0;

				case RCC_AHB2_ENR:

					return 0;

				case RCC_AHB3_ENR:

					return 0;

				case RCC_APB1_ENR:

					return 0;

				case RCC_APB2_ENR:

					return 0;

				case RCC_AHB1_LP_ENR:

					return 0;

				case RCC_AHB2_LP_ENR:

					return 0;

				case RCC_AHB3_LP_ENR:

					return 0;

				case RCC_APB1_LP_ENR:

					return 0;

				case RCC_APB2_LP_ENR:

					return 0;

				case RCC_BDCR:

					return 0;

				case RCC_CSR:

					return 0;

				case RCC_SSCGR:

					return 0;

				case RCC_PLL_I2S_CFGR:

					return 0;

				default: return 0;
				break;
			}
		}

		case PERIPH_SPI:{

		}

		case PERIPH_I2C:{

		}

		case PERIPH_GPIO:{

		}

		case PERIPH_UART:{

		}

		default: return 0;
	}
}
