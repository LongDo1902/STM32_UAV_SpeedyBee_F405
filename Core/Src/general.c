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

	/* ~mask zeros valid bits and keeps invalid bits to 1
	 * value & ~mask extract the only bits that should NOT be present*/
	uint32_t mask = (1u << bitWidth) - 1u;
	if((value & ~mask) == 0u) return true;
	else return false;
}





