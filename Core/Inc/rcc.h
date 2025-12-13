/*
 * rcc.h
 *
 *  Created on: Nov 13, 2025
 *      Author: dobao
 */

#ifndef INC_RCC_H_
#define INC_RCC_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "stm32f405_addr.h"

/*
 * ================================================================
 * 				RESERVED BIT POSITIONS AND MASKS
 * ================================================================
 */
#define	RCC_CR_RESERVED_Pos_2			2u
#define RCC_CR_RESERVED_Pos_20			20u
#define	RCC_CR_RESERVED_Pos_28			28u
#define RCC_CR_RESERVED_Msk				~((1u << RCC_CR_RESERVED_Pos_2) |	\
										(0xFu << RCC_CR_RESERVED_Pos_20) | 	\
										(0xFu << RCC_CR_RESERVED_Pos_28))

#define RCC_PLL_CFGR_RESERVED_Pos_15		15u
#define RCC_PLL_CFGR_RESERVED_Pos_18		18u
#define RCC_PLL_CFGR_RESERVED_Pos_23		23u
#define RCC_PLL_CFGR_RESERVED_Pos_28		28u
#define RCC_PLL_CFGR_RESERVED_Msk		~((1u << RCC_PLL_CFGR_RESERVED_Pos_15) |	\
										(0xFu << RCC_PLL_CFGR_RESERVED_Pos_18) |	\
										(1u << RCC_PLL_CFGR_RESERVED_Pos_23) |		\
										(0xFu << RCC_PLL_CFGR_RESERVED_Pos_28))

#define RCC_CFGR_RESERVED_Pos_8			8u
#define RCC_CFGR_RESERVED_Msk			~(0x3u << RCC_CFGR_RESERVED_Pos_8)

#define RCC_CIR_RESERVED_Pos_6			6u
#define RCC_CIR_RESERVED_Pos_14			14u
#define RCC_CIR_RESERVED_Pos_22			22u
#define RCC_CIR_RESERVED_Pos_24			24u
#define RCC_CIR_RESERVED_Msk			~((1u << RCC_CIR_RESERVED_Pos_6) |	\
										(0x3u << RCC_CIR_RESERVED_Pos_14) |	\
										(1u << RCC_CIR_RESERVED_Pos_22) | 	\
										(0xFFu << RCC_CIR_RESERVED_Pos_24))

#define RCC_AHB1_RSTR_RESERVED_Pos_9	9u
#define RCC_AHB1_RSTR_RESERVED_Pos_13	13u
#define RCC_AHB1_RSTR_RESERVED_Pos_23	23u
#define RCC_AHB1_RSTR_RESERVED_Pos_26	26u
#define RCC_AHB1_RSTR_RESERVED_Pos_30	30u
#define RCC_AHB1_RSTR_RESERVED_Msk		~((0x7u << RCC_AHB1_RSTR_RESERVED_Pos_9) |	\
										(0xFFu << RCC_AHB1_RSTR_RESERVED_Pos_13) |	\
										(0x3u << RCC_AHB1_RSTR_RESERVED_Pos_23) | 	\
										(0x7u << RCC_AHB1_RSTR_RESERVED_Pos_26) | 	\
										(0x3u << RCC_AHB1_RSTR_RESERVED_Pos_30))

#define RCC_AHB2_RSTR_RESERVED_Pos_1	1u
#define RCC_AHB2_RSTR_RESERVED_Pos_8	8u
#define RCC_AHB2_RSTR_RESERVED_Msk		~((0x7u << RCC_AHB2_RSTR_RESERVED_Pos_1) |	\
										(0xFFFFFFu << RCC_AHB2_RSTR_RESERVED_Pos_8))

#define RCC_AHB3_RSTR_RESERVED_Pos_1	1u
#define	RCC_AHB3_RSTR_RESERVED_Msk		~(0x7FFFFFFFu << RCC_AHB3_RSTR_RESERVED_Pos_1)

#define	RCC_APB1_RSTR_RESERVED_Pos_9	9u
#define RCC_APB1_RSTR_RESERVED_Pos_12	12u
#define	RCC_APB1_RSTR_RESERVED_Pos_16	16u
#define RCC_APB1_RSTR_RESERVED_Pos_24	24u
#define	RCC_APB1_RSTR_RESERVED_Pos_27	27u
#define RCC_APB1_RSTR_RESERVED_Pos_30	30u
#define RCC_APB1_RSTR_RESERVED_Msk		~((0x3u << RCC_APB1_RSTR_RESERVED_Pos_9) |	\
										(0x3u << RCC_APB1_RSTR_RESERVED_Pos_12) |	\
										(1u << RCC_APB1_RSTR_RESERVED_Pos_16) | 	\
										(1u << RCC_APB1_RSTR_RESERVED_Pos_24) | 	\
										(1u << RCC_APB1_RSTR_RESERVED_Pos_27) | 	\
										(0x3u << RCC_APB1_RSTR_RESERVED_Pos_30))

#define	RCC_APB2_RSTR_RESERVED_Pos_2	2u
#define RCC_APB2_RSTR_RESERVED_Pos_6	6u
#define	RCC_APB2_RSTR_RESERVED_Pos_9	9u
#define RCC_APB2_RSTR_RESERVED_Pos_13	13u
#define RCC_APB2_RSTR_RESERVED_Pos_15	15u
#define	RCC_APB2_RSTR_RESERVED_Pos_19	19u
#define RCC_APB2_RSTR_RESERVED_Msk		~((0x3u << RCC_APB2_RSTR_RESERVED_Pos_2) |	\
										(0x3u << RCC_APB2_RSTR_RESERVED_Pos_6) | 	\
										(0x3u << RCC_APB2_RSTR_RESERVED_Pos_9) | 	\
										(1u << RCC_APB2_RSTR_RESERVED_Pos_13) | 	\
										(1u << RCC_APB2_RSTR_RESERVED_Pos_15) | 	\
										(0x1FFF << RCC_APB2_RSTR_RESERVED_Pos_19))

#define	RCC_AHB1_ENR_RESERVED_Pos_9		9u
#define RCC_AHB1_ENR_RESERVED_Pos_13	13u
#define	RCC_AHB1_ENR_RESERVED_Pos_19	19u
#define RCC_AHB1_ENR_RESERVED_Pos_23	23u
#define RCC_AHB1_ENR_RESERVED_Pos_31	31u
#define RCC_AHB1_ENR_RESERVED_Msk		~((0x7u << RCC_AHB1_ENR_RESERVED_Pos_9) | 	\
										(0x1F << RCC_AHB1_ENR_RESERVED_Pos_13) |	\
										(1u << RCC_AHB1_ENR_RESERVED_Pos_19) |		\
										(0x3u << RCC_AHB1_ENR_RESERVED_Pos_23) |	\
										(1u << RCC_AHB1_ENR_RESERVED_Pos_31))

#define RCC_AHB2_ENR_RESERVED_Pos_1		1u
#define RCC_AHB2_ENR_RESERVED_Pos_8		8u
#define RCC_AHB2_ENR_RESERVED_Msk		~((0x7u << RCC_AHB2_ENR_RESERVED_Pos_1) | (0xFFFFFFu << RCC_AHB2_ENR_RESERVED_Pos_8))

#define RCC_AHB3_ENR_RESERVED_Pos_1		1u
#define RCC_AHB3_ENR_RESERVED_Msk		~(0x7FFFFFFFu << RCC_AHB3_ENR_RESERVED_Pos_1)

#define RCC_APB1_ENR_RESERVED_Pos_9		9u
#define RCC_APB1_ENR_RESERVED_Pos_12	12u
#define RCC_APB1_ENR_RESERVED_Pos_16	16u
#define RCC_APB1_ENR_RESERVED_Pos_24	24u
#define RCC_APB1_ENR_RESERVED_Pos_27	27u
#define RCC_APB1_ENR_RESERVED_Pos_30	30u
#define RCC_APB1_ENR_RESERVED_Msk		~((0x3u << RCC_APB1_ENR_RESERVED_Pos_9) | 	\
										(0x3u << RCC_APB1_ENR_RESERVED_Pos_12) | 	\
										(1u << RCC_APB1_ENR_RESERVED_Pos_16) | 		\
										(1u << RCC_APB1_ENR_RESERVED_Pos_24) | 		\
										(1u << RCC_APB1_ENR_RESERVED_Pos_27) | 		\
										(0x3u << RCC_APB1_ENR_RESERVED_Pos_30))

#define RCC_APB2_ENR_RESERVED_Pos_2		2u
#define RCC_APB2_ENR_RESERVED_Pos_6		6u
#define RCC_APB2_ENR_RESERVED_Pos_13	13u
#define RCC_APB2_ENR_RESERVED_Pos_15	15u
#define RCC_APB2_ENR_RESERVED_Pos_19	19u
#define RCC_APB2_ENR_RESERVED_Msk		~((0x3u << RCC_APB2_ENR_RESERVED_Pos_2) |	\
										(0x3u << RCC_APB2_ENR_RESERVED_Pos_6) | 	\
										(1u << RCC_APB2_ENR_RESERVED_Pos_13) | 		\
										(1u << RCC_APB2_ENR_RESERVED_Pos_15) | 		\
										(0x1FFFu << RCC_APB2_ENR_RESERVED_Pos_19))

#define RCC_AHB1_LP_ENR_RESERVED_Pos_9		9u
#define RCC_AHB1_LP_ENR_RESERVED_Pos_13		13u
#define RCC_AHB1_LP_ENR_RESERVED_Pos_19		19u
#define RCC_AHB1_LP_ENR_RESERVED_Pos_23		23u
#define RCC_AHB1_LP_ENR_RESERVED_Pos_31		31u
#define	RCC_AHB1_LP_ENR_RESERVED_Msk		~((0x7u << RCC_AHB1_LP_ENR_RESERVED_Pos_9) |	\
											(0x3u << RCC_AHB1_LP_ENR_RESERVED_Pos_13) | 	\
											(0x3u << RCC_AHB1_LP_ENR_RESERVED_Pos_19) | 	\
											(0x3u << RCC_AHB1_LP_ENR_RESERVED_Pos_23) | 	\
											(1u << RCC_AHB1_LP_ENR_RESERVED_Pos_31))

#define RCC_AHB2_LP_ENR_RESERVED_Pos_1		1u
#define RCC_AHB2_LP_ENR_RESERVED_Pos_8		8u
#define RCC_AHB2_LP_ENR_RESERVED_Msk		~((0x7u << RCC_AHB2_LP_ENR_RESERVED_Pos_1) | 	\
											(0xFFFFFFu << RCC_AHB2_LP_ENR_RESERVED_Pos_8))

#define RCC_AHB3_LP_ENR_RESERVED_Pos_1		1u
#define RCC_AHB3_LP_ENR_RESERVED_Msk		~(0x7FFFFFFF << RCC_AHB3_LP_ENR_RESERVED_Pos_1)

#define	RCC_APB1_LP_ENR_RESERVED_Pos_9		9u
#define RCC_APB1_LP_ENR_RESERVED_Pos_12		12u
#define RCC_APB1_LP_ENR_RESERVED_Pos_16		16u
#define RCC_APB1_LP_ENR_RESERVED_Pos_24		24u
#define RCC_APB1_LP_ENR_RESERVED_Pos_27		27u
#define RCC_APB1_LP_ENR_RESERVED_Pos_30		30u
#define RCC_APB1_LP_ENR_RESERVED_Msk		~((0x3u << RCC_APB1_LP_ENR_RESERVED_Pos_9) |	\
											(0x3u << RCC_APB1_LP_ENR_RESERVED_Pos_12) |		\
											(1u << RCC_APB1_LP_ENR_RESERVED_Pos_16) |		\
											(1u << RCC_APB1_LP_ENR_RESERVED_Pos_24) |		\
											(1u << RCC_APB1_LP_ENR_RESERVED_Pos_27) |		\
											(0x3u << RCC_APB1_LP_ENR_RESERVED_Pos_30))

#define RCC_APB2_LP_ENR_RESERVED_Pos_2		2u
#define RCC_APB2_LP_ENR_RESERVED_Pos_6		6u
#define RCC_APB2_LP_ENR_RESERVED_Pos_13		13u
#define RCC_APB2_LP_ENR_RESERVED_Pos_15		15u
#define RCC_APB2_LP_ENR_RESERVED_Pos_19		19u
#define	RCC_APB2_LP_ENR_RESERVED_Msk		~((0x3u << RCC_APB2_LP_ENR_RESERVED_Pos_2) | 	\
											(0x3u << RCC_APB2_LP_ENR_RESERVED_Pos_6) | 		\
											(1u << RCC_APB2_LP_ENR_RESERVED_Pos_13) | 		\
											(1u << RCC_APB2_LP_ENR_RESERVED_Pos_15) |		\
											(0x1FFFu << RCC_APB2_LP_ENR_RESERVED_Pos_19))

#define RCC_BDCR_RESERVED_Pos_3				3u
#define RCC_BDCR_RESERVED_Pos_10			10u
#define	RCC_BDCR_RESERVED_Pos_17			17u
#define	RCC_BDCR_RESERVED_Msk				~((0x1Fu << RCC_BDCR_RESERVED_Pos_3) | 	\
											(0x1Fu << RCC_BDCR_RESERVED_Pos_10) | 	\
											(0x7FFFu << RCC_BDCR_RESERVED_Pos_17))

#define RCC_CSR_RESERVED_Pos_2				2u
#define	RCC_CSR_RESERVED_Msk				~(0x3FFFFFu << RCC_CSR_RESERVED_Pos_2)

#define	RCC_SSCGR_RESERVED_Pos_28			28u
#define RCC_SSCGR_RESERVED_Msk				~(0x3u << RCC_SSCGR_RESERVED_Pos_28)

#define RCC_PLL_I2S_CFGR_RESERVED_Pos_0		0u
#define RCC_PLL_I2S_CFGR_RESERVED_Pos_15	15u
#define RCC_PLL_I2S_CFGR_RESERVED_Pos_31	31u
#define RCC_PLL_I2S_CFGR_RESERVED_Msk		~((0x3F << RCC_PLL_I2S_CFGR_RESERVED_Pos_0) |	\
											(0x1FFFu << RCC_PLL_I2S_CFGR_RESERVED_Pos_15) | \
											(1u << RCC_PLL_I2S_CFGR_RESERVED_Pos_31))


/*
 * @enum	RCC_Reg_t
 * @brief	Human-readable indices for RCC registers
 *
 */
typedef enum{
	/* Core	 */
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

	RCC_REG_COUNT
}RCC_Reg_t;

#endif /* INC_RCC_H_ */
