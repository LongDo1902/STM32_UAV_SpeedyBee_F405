/*
 * rcc.h
 *
 *  Created on: Nov 13, 2025
 *      Author: dobao
 */

#ifndef INC_RCC_H_
#define INC_RCC_H_

#include <stdint.h>

typedef enum{
	RCC_CR,
	RCC_PLL_CFGR,
	RCC_CFGR,
	RCC_CIR,

	RCC_AHB1_RSTR,
	RCC_AHB2_RSTR,
	RCC_AHB3_RSTR,

	RCC_APB1_RSTR,
	RCC_APB2_RSTR,

	RCC_AHB1_ENR,
	RCC_AHB2_ENR,
	RCC_AHB3_ENR,

	RCC_APB1_ENR,
	RCC_APB2_ENR,

	RCC_AHB1_LP_ENR,
	RCC_AHB2_LP_ENR,
	RCC_AHB3_LP_ENR,

	RCC_APB1_LP_ENR,
	RCC_APB2_LP_ENR,

	RCC_BDCR,
	RCC_CSR,
	RCC_SSCGR,
	RCC_PLL_I2S_CFGR,
}RCC_Reg_t;

#endif /* INC_RCC_H_ */
