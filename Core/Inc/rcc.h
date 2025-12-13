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
 * 					RCC BIT-POSITION DEFINITIONS
 * ================================================================
 */
#define	HSION_Pos		0u	//Internal high-speed clock enable
#define	HSIRDY_Pos		1u	//Internal high-speed clock ready flag
#define	HSITRIM_Pos		3u	//Internal high-speed clock trimming
#define HSICAL_Pos		8u	//Internal high-speed clock calibration
#define	HSEON_Pos		16u	//HSE clock enable
#define	HSERDY_Pos		17u	//HSE clock ready flag
#define HSEBYP_Pos		18u	//HSE clock bypass
#define CSSON_Pos		19u	//Clock security sysem enable
#define	PLLON_Pos		24u	//Main PLL enable
#define PLLRDY_Pos		25u //Main PLL clock ready flag
#define PLLI2SON_Pos	26u	//PLLI2S enable
#define PLLI2SRDY_Pos	27u	//PLLI2S clock ready flag

#define PLLM_Pos		0u	//Division factor for the main PLL and audio PLLI2S input clock
#define	PLLN_Pos		6u	//Main PLL multiplication factor
#define PLLP_Pos		16u	//Main PLL division factor for main system clock
#define PLLSRC_Pos		22u	//Main PLL and PLLI2S entry clock source
#define PLLQ_Pos		24u	//Main PLL division factor for USB OTG FS, SDIO and random number generator clocks

#define SW_Pos			0u	//System clock switch
#define	SWS_Pos			2u	//System clock switch status
#define	HPRE_Pos		4u	//AHB prescaler
#define	PPRE1_Pos		10u	//APB low speed prescaler (APB1)
#define PPRE2_Pos		13u	//APB high-speed prescaler (APB2)
#define RTCPRE_Pos		16u	//HSE division factor for RTC clock
#define	MCO1_Pos		21u	//Microcontroller clock output 1
#define	I2SSRC_Pos		23u	//I2S clock selection
#define MCO1PRE_Pos		24u	//MCO1 prescaler
#define MCO2PRE_Pos		27u //MCO2 prescaler
#define MCO2_Pos		30u	//Microcontroller clock output 2

#define LSIRDYF_Pos		0u	//LSI ready interrupt flag
#define	LSERDYF_Pos		1u	//LSE ready interrupt flag
#define HSIRDYF_Pos		2u	//HSI ready interrupt flag
#define HSERDYF_Pos		3u	//HSE ready interrupt flag
#define PLLRDYF_Pos		4u	//Main PLL ready interrupt flag
#define PLLI2SRDYF_Pos	5u	//PLLI2S ready interrupt flag
#define CSSF_Pos		7u	//Clock security system interrupt flag
#define LSIRDYIE_Pos	8u	//LSI ready interrupt enable
#define LSERDYIE_Pos	9u	//LSE ready interrupt enable
#define HSIRDYIE_Pos	10u	//HSI ready interrupt enable
#define HSERDYIE_Pos	11u	//HSE ready interrupt enable
#define	PLLRDYIE_Pos	12u	//Main PLL ready interrupt enable
#define PLLI2SRDYIE_Pos	13u	//PLLI2S ready interrupt enable
#define LSIRDYC_Pos		16u	//LSI ready interrupt clear
#define LSERDYC_Pos		17u	//LSE ready interrupt clear
#define HSIRDYC_Pos		18u	//HSI ready interrupt clear
#define HSERDYC_Pos		19u	//HSE ready interrupt clear
#define PLLRDYC_Pos		20u	//Main PLL ready interrupt clear
#define PLLI2SRDYC_Pos	21u	//PLLI2S ready interrupt clear
#define CSSC_Pos		23u	//Clock security system interrupt clear

#define GPIOARST_Pos	0u	//Port A reset
#define GPIOBRST_Pos	1u	//Port B reset
#define GPIOCRST_Pos	2u	//Port C reset
#define GPIODRST_Pos	3u	//Port D reset
#define CRCRST_Pos		12u	//CRC reset
#define	DMA1RST_Pos		21u //DMA1 reset
#define DMA2RST_Pos		22u	//DMA2 reset
#define ETHMACRST_Pos	25u	//Ethernet MAC reset
#define OTGHSRST_Pos	29u	//USB OTG HS module reset

#define DCMIRST_Pos		0u	//Camera interface reset
#define CRYPRST_Pos		4u	//Cryptographic module reset
#define HASHRST_Pos		5u	//Hash module reset
#define RNGRST_Pos		6u	//Random number generator module reset
#define OTGFSRST_Pos	7u	//USB OTG FS module reset

#define FSMCRST_Pos		0u	//Flexible static memory controller module reset

#define TIM2RST_Pos		0u	//TIM2 reset
#define TIM3RST_Pos		1u	//TIM3 reset
#define TIM4RST_Pos		2u	//TIM4 reset
#define TIM5RST_Pos		3u	//TIM5 reset
#define TIM6RST_Pos		4u	//TIM6 reset
#define TIM7RST_Pos		5u	//TIM7 reset
#define TIM12RST_Pos	6u	//TIM12 reset
#define TIM13RST_Pos	7u	//TIM13 reset
#define TIM14RST_Pos	8u	//TIM14 reset
#define WWDGRST_Pos		11u	//Window watchdog reset
#define	SPI2RST_Pos		14u	//SPI2 reset
#define SPI3RST_Pos		15u	//SPI3 reset
#define USART2RST_Pos	17u //USART2 reset
#define USART3RST_Pos	18u	//USART3 reset
#define	UART4RST_Pos	19u	//UART4 reset
#define UART5RST_Pos	20u	//UART5 reset
#define I2C1RST_Pos		21u	//I2C1 reset
#define I2C2RST_Pos		22u	//I2C2 reset
#define I2C3RST_Pos		23u	//I2C3 reset
#define	CAN1RST_Pos		25u	//CAN1 reset
#define CAN2RST_Pos		26u	//CAN2 reset
#define PWRRST_Pos		28u	//Power interface reset
#define DACRST_Pos		29u	//DAC reset


#define TIM1RST_Pos		0u	//TIM1 reset
#define TIM8RST_Pos		1u	//TIM8 reset
#define USART1RST_Pos	4u	//USART1 reset
#define USART6RST_Pos	5u	//USART6 reset
#define	ADCRST_Pos		8u	//ADC interface reset
#define SDIORST_Pos		11u //SDIO reset
#define SPI1RST_Pos		12u	//SPI1 reset
#define	SYSCFGRST_Pos	14u	//System config controller reset
#define TIM9RST_Pos		16u	//TIM9 reset
#define TIM10RST_Pos	17u	//TIM10 reset
#define	TIM11RST_Pos	18u	//TIM11 reset

#define GPIOAEN_Pos			0u	//GPIOA clock enable
#define GPIOBEN_Pos			1u	//GPIOB clock enable
#define GPIOCEN_Pos			2u	//GPIOC clock enable
#define GPIODEN_Pos			3u	//GPIOD clock enable
#define CRCEN_Pos			12u	//CRC clock enable
#define BKPSRAMEN_Pos		18u	//Backup SRAM interface clock enable
#define CCMDATARAMEN_Pos	20u	//CCM data RAM clock enabe
#define DMA1EN_Pos			21u	//DMA1 clock enable
#define DMA2EN_Pos			22u	//DMA2 clock enable
#define ETHMACEN_Pos		25u	//Ethernet MAC clock enable
#define ETHMACTXEN_Pos		26u	//Ethernet Transmission clock enable
#define ETHMACRXEN_Pos		27u	//Ethernet RX clock enable
#define ETHMACPTPEN_Pos		28u	//Ethernet PTP clock enable
#define OTGHSEN_Pos			29u	//USB OTG HS clock enable
#define OTGHSULPIEN_Pos		30u	//USB OTG HSULPI clock enable

#define DCMIEN_Pos		0u	//Camera interface enable
#define CRYPEN_Pos		4u	//Cryptographic modules clock enable
#define HASHEN_Pos		5u	//Hash modules clock enable
#define	RNGEN_Pos		6u	//Random number generator clock enable
#define OTGFSEN_Pos		7u	//USB OTG FS clock enable

#define FSMCEN_Pos		0u	//Flexible static memory controller module clock enable

#define TIM2EN_Pos		0u	//TIM2 clock enable
#define	TIM3EN_Pos		1u	//TIM3 clock enable
#define TIM4EN_Pos		2u	//TIM4 clock enable
#define TIM5EN_Pos		3u	//TIM5 clock enable
#define TIM6EN_Pos		4u	//TIM6 clock enable
#define TIM7EN_Pos		5u	//TIM7 clock enable
#define TIM12EN_Pos		6u	//TIM12 clock enable
#define TIM13EN_Pos		7u	//TIM13 clock enable
#define TIM14EN_Pos		8u	//TIM14 clock enable
#define WWDGEN_Pos		11u	//Window watchdog clock enable
#define SPI2EN_Pos		14u	//SPI2 clock enable
#define SPI3EN_Pos		15u	//SPI3 clock enable
#define USART2EN_Pos	17u	//USART2 clock enable
#define USART3EN_Pos	18u	//USART3 clock enable
#define UART4EN_Pos		19u	//UART4 clock enable
#define UART5EN_Pos		20u	//UART5 clock enable
#define I2C1EN_Pos		21u	//I2C1 clock enable
#define I2C2EN_Pos		22u //I2C2 clock enable
#define I2C3EN_Pos		23u	//I2C3 clock enable
#define CAN1EN_Pos		25u	//CAN1 clock enable
#define CAN2EN_Pos		26u	//CAN2 clock enable
#define PWREN_Pos		28u	//Power interface clock enable
#define DACEN_Pos		29u	//DAC interface clock enable

#define TIM1EN_Pos		0u	//TIM1 clock enable
#define TIM8EN_Pos		1u	//TIM8 clock enable
#define USART1EN_Pos	4u	//USART1 clock enable
#define USART6EN_Pos	5u	//USART6 clock enable
#define ADC1EN_Pos		8u	//ADC1 clock enable
#define ADC2EN_Pos		9u	//ADC2 clock enable
#define ADC3EN_Pos		10u	//ADC3 clock enable
#define SDIOEN_Pos		11u	//SDIO clock enable
#define	SPI1EN_Pos		12u	//SPI1 clock enable
#define SYSCFGEN_Pos	14u	//System config controller clock enable
#define TIM9EN_Pos		16u	//TIM9 clock enable
#define TIM10EN_Pos		17u //TIM10 clock enable
#define TIM11EN_Pos		18u	//TIM11 clock enable

#define GPIOALPEN_Pos		0u	//Port A clock enable during sleep mode
#define GPIOBLPEN_Pos		1u	//Port B clock enable during sleep mode
#define GPIOCLPEN_Pos		2u	//Port C clock enable during sleep mode
#define GPIODLPEN_Pos		3u	//Port D clock enable during sleep mode
#define CRCLPEN_Pos			12u	//CRC clock enable during sleep mode
#define FLITFLPEN_Pos		15u	//FLASH interface clock enable during sleep mode
#define SRAM1LPEN_Pos		16u //SRAM1 interface clock enable during sleep mode
#define SRAM2LPEN_Pos		17u	//SRAM2 interface clock enable during sleep mode
#define BKPSRAMLPEN_Pos		18u	//Backup SRAM interface clock enable during sleep mode
#define DMA1LPEN_Pos		21u	//DMA1 clock enable during sleep mode
#define	DMA2LPEN_Pos		22u	//DMA2 clock enable during sleep mode
#define ETHMACLPEN_Pos		25u	//Ethernet MAC clock enable during sleep mode
#define ETHMACTXLPEN_Pos	26u	//Ethernet TX clock enable during sleep mode
#define ETHMACRXLPEN_Pos	27u	//Ethernet RX clock enable during sleep mode
#define ETHMACPTPLPEN_Pos	28u	//Ethernet PTP clock enable during sleep mode
#define OTGHSLPEN_Pos		29u	//USB OTG HS clock enable during sleep mode
#define OTGHSULPILPEN_Pos	30u	//USB OTG HS ULPI clock enable during sleep mode

#define DCMILPEN_Pos		0u	//Camera interface enable during sleep mode
#define CRYPLPEN_Pos		4u	//Cryptography modules clock enable during sleep mode
#define HASHLPEN_Pos		5u	//Hash modules clock enable during sleep mode
#define RNGLPEN_Pos			6u	//Random number generator clock enable during sleep mode
#define OTGFSLPEN_Pos		7u	//USB OTG FS clock enable during sleep mode

#define FSMCLPEN_Pos		0u	//Flexible static memory controller module clock enable during sleep mode

#define TIM2LPEN_Pos	0u	//TIM2 clock enable during sleep mode
#define TIM3LPEN_Pos	1u	//TIM3 clock enable during sleep mode
#define TIM4LPEN_Pos	2u	//TIM4 clock enable during sleep mode
#define TIM5LPEN_Pos	3u	//TIM5 clock enable during sleep mode
#define TIM6LPEN_Pos	4u	//TIM6 clock enable during sleep mode
#define TIM7LPEN_Pos	5u	//TIM7 clock enable during sleep mode
#define TIM12LPEN_Pos	6u 	//TIM12 clock enable during sleep mode
#define TIM13LPEN_Pos	7u	//TIM13 clock enable during sleep mode
#define TIM14LPEN_Pos	8u	//TIM14 clock enable during sleep mode
#define WWDGLPEN_Pos	11u	//WIndow watchdog clock enable during sleep mode
#define SPI2LPEN_Pos	14u	//SPI2 clock enable during sleep mode
#define SPI3LPEN_Pos	15u	//SPI3 clock enable during sleep mode
#define USART2LPEN_Pos 	17u	//USART2 clock enable during sleep mode
#define USART3LPEN_Pos	18u	//USART3 clock enable during sleep mode
#define UART4LPEN_Pos	19u	//UART4 clock enable during sleep mode
#define UART5LPEN_Pos	20u	//UART5 clock enable during sleep mode
#define I2C1LPEN_Pos	21u	//I2C1 clock enable during sleep mode
#define I2C2LPEN_Pos	22u	//I2C2 clock enable during sleep mode
#define I2C3LPEN_Pos	23u	//I2C3 clock enable during sleep mode
#define CAN1LPEN_Pos	25u //CAN1 clock enable during sleep mode
#define CAN2LPEN_Pos	26u	//CAN2 clock enable during sleep mode
#define PWRLPEN_Pos		28u	//Power interface clock enable during sleep mode
#define DACLPEN_Pos		29u	//DAC interface clock enable during sleep mode

#define TIM1LPEN_Pos	0u	//TIM1 clock enable during sleep mode
#define TIM8LPEN_Pos	1u	//TIM8 clock enable during sleep mode
#define USART1LPEN_Pos	4u	//USART1 clock enable during sleep mode
#define USART6LPEN_Pos	5u	//USART6 clock enable during sleep mode
#define ADC1LPEN_Pos	8u	//ADC1 clock enable during sleep mode
#define ADC2LPEN_Pos	9u	//ADC2 clock enable during sleep mode
#define ADC3LPEN_Pos	10u	//ADC3 clock enable during sleep mode
#define SDIOLPEN_Pos	11u	//SDIO clock enable during sleep mode
#define SPI1LPEN_Pos	12u //SPI1 clock enable during sleep mode
#define SYSCFGLPEN_Pos	14u	//System config controller clock enable during sleep mode
#define TIM9LPEN_Pos	16u	//TIM9 clock enable during sleep mode
#define TIM10LPEN_Pos	17u	//TIM10 clock enable during sleep mode
#define	TIM11LPEN_Pos	18u	//TIM11 clock enable during sleep mode

#define LSEON_Pos		0u	//External low-speed oscillator enable
#define LSERDY_Pos		1u	//External low-speed oscillator ready
#define LSEBYP_Pos		2u	//External low-speed oscillator bypass
#define	RTCSEL_Pos		8u	//RTC clock source selection
#define	RTCEN_Pos		15u	//RTC clock enable
#define BDRST_Pos		16u	//Backup domain software reset

#define LSION_Pos		0u	//Internal low-speed oscillator enable
#define LSIRDY_Pos		1u	//Internal low-speed oscillator ready
#define RMVF_Pos		24u	//Remove reset flag
#define BORRSTF_Pos		25u	//BOR reset flag
#define	PORRSTF_Pos		27u	//POR/PDR reset flag
#define SFTRSTF_Pos		28u	//Software reset flag
#define	IWDGRSTF_Pos	29u //Independent watchdog reset flag
#define	WWDGRSTF_Pos	30u	//Window watchdog reset flag
#define	LPWRRSTF_Pos	31u	//Low-power reset flag

#define	MODPER_Pos		0u	//Modulation period
#define	INCSTEP_Pos		13u	//Incrementation step
#define SPREADSEL_Pos	30u	//Spread select
#define	SSCGEN_Pos		31u	//Spread spectrum modulation enable

#define PLLI2SN_Pos		6u	//PLLI2S multiplication factor for VCO
#define PLLI2SR_Pos		28u	//PLLI2S division factor for I2S clocks


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
