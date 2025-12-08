/*
 * stm32f405_baseAddr.h
 *
 *  Created on: Nov 13, 2025
 *      Author: dobao
 */

#ifndef INC_STM32F405_ADDR_H_
#define INC_STM32F405_ADDR_H_


/*
 * ========================================================
 * 		  STM32F405RGT6 PERIPHERAL'S BASE ADDRESSES
 * ========================================================
 */
/* GPIOs */
#define GPIO_A_BASE_ADDR	0x40020000u
#define GPIO_B_BASE_ADDR	0x40020400u
#define GPIO_C_BASE_ADDR	0x40020800u

/* SPI */
#define SPI_1_BASE_ADDR		0x40013000u
#define	SPI_2_BASE_ADDR		0x40003800u
#define SPI_3_BASE_ADDR		0x40003C00u

/* I2C */
#define I2C_1_BASE_ADDR		0x40005400u
#define I2C_2_BASE_ADDR		0x40005800u
#define I2C_3_BASE_ADDR		0x40005C00u

/* UART */
#define UART_1_BASE_ADDR	0x40011000u
#define UART_2_BASE_ADDR	0x40004400u
#define UART_3_BASE_ADDR	0x40004800u
#define UART_4_BASE_ADDR	0x40004C00u
#define UART_5_BASE_ADDR	0x40005000u
#define UART_6_BASE_ADDR	0x40011400u

/* TIMER */
#define TIMER_1_BASE_ADDR	0x40010000u
#define TIMER_2_BASE_ADDR	0x40000000u
#define TIMER_3_BASE_ADDR	0x40000400u
#define TIMER_4_BASE_ADDR	0x40000800u
#define TIMER_5_BASE_ADDR	0x40000C00u
#define TIMER_6_BASE_ADDR	0x40001000u
#define TIMER_7_BASE_ADDR	0x40001400u
#define TIMER_8_BASE_ADDR	0x40010400u
#define TIMER_9_BASE_ADDR	0x40014000u
#define TIMER_10_BASE_ADDR	0x40014400u
#define TIMER_11_BASE_ADDR	0x40014800u
#define TIMER_12_BASE_ADDR	0x40001800u
#define TIMER_13_BASE_ADDR	0x40001C00u
#define TIMER_14_BASE_ADDR	0x40002000u

/* ADC */
#define ADC_BASE_ADDR	0x40012000u

/* OTHERS */
#define RCC_BASE_ADDR	0x40023800u
#define DMA_1_BASE_ADDR	0x40026000u
#define DMA_2_BASE_ADDR	0x40026400u
#define PWR_BASE_ADDR	0x40007000u
#define EXTI_BASE_ADDR	0x40013C00u
#define FLASH_BASE_ADDR	0x40023C00u

/*
 * ========================================================
 * 		PERIPHERAL REGISTER ADDRESS OFFSET STRUCTURES
 * ========================================================
 */
/* General Purpose I/O Register Map */
typedef struct{
	volatile uint32_t GPIO_MODER; 	//Mode
	volatile uint32_t GPIO_OTYPER;	//Output Type
	volatile uint32_t GPIO_OSPEEDR;	//Output Speed
	volatile uint32_t GPIO_PUPDR; 	//Pull up/down
	volatile uint32_t GPIO_IDR;		//Input Data
	volatile uint32_t GPIO_ODR;		//Output Data
	volatile uint32_t GPIO_BSRR;	//Bit Set/Reset
	volatile uint32_t GPIO_LCKR;	//Lock
	volatile uint32_t GPIO_AFRL;	//Alternate Function Low
	volatile uint32_t GPIO_AFRH;	//Alternate Function High
}gpioRegOffset_t;

/* Flash Interface Register Map */
typedef struct{
	volatile uint32_t FLASH_ACR;
	volatile uint32_t FLASH_KEYR;
	volatile uint32_t FLASH_OPT_KEYR;
	volatile uint32_t FLASH_SR;
	volatile uint32_t FLASH_CR;
	volatile uint32_t FLASH_OPT_CR;
}flashRegOffset_t;

/* SPI Register Map */
typedef struct{
	volatile uint32_t SPI_CR1;
	volatile uint32_t SPI_CR2;
	volatile uint32_t SPI_SR;
	volatile uint32_t SPI_DR;
	volatile uint32_t SPI_CRC_PR;
	volatile uint32_t SPI_RX_CRCR;
	volatile uint32_t SPI_TX_CRCR;
	volatile uint32_t SPI_I2S_CFGR;
	volatile uint32_t SPI_I2SPR;
}spiRegOffset_t;

/* I2C Register Map */
typedef struct{
	volatile uint32_t I2C_CR1;
	volatile uint32_t I2C_CR2;
	volatile uint32_t I2C_OAR1;
	volatile uint32_t I2C_OAR2;
	volatile uint32_t I2C_DR;
	volatile uint32_t I2C_SR1;
	volatile uint32_t I2C_SR2;
	volatile uint32_t I2C_CCR;
	volatile uint32_t I2C_TRISE;
	volatile uint32_t I2C_FLTR;
}i2cRegOffset_t;

/* Reset and Clock Control Register Map */
typedef struct{
	volatile uint32_t RCC_CR;
	volatile uint32_t RCC_PLL_CFGR;
	volatile uint32_t RCC_CFGR;
	volatile uint32_t RCC_CIR;

	volatile uint32_t RCC_AHB1_RSTR;
	volatile uint32_t RCC_AHB2_RSTR;
	volatile uint32_t RCC_AHB3_RSTR;

	uint32_t RESERVED0;

	volatile uint32_t RCC_APB1_RSTR;
	volatile uint32_t RCC_APB2_RSTR;

	uint32_t RESERVED1[2];

	volatile uint32_t RCC_AHB1_ENR;
	volatile uint32_t RCC_AHB2_ENR;
	volatile uint32_t RCC_AHB3_ENR;

	volatile uint32_t RESERVED2;

	volatile uint32_t RCC_APB1_ENR;
	volatile uint32_t RCC_APB2_ENR;

	uint32_t RESERVED3[2];

	volatile uint32_t RCC_AHB1_LP_ENR;
	volatile uint32_t RCC_AHB2_LP_ENR;
	volatile uint32_t RCC_AHB3_LP_ENR;

	uint32_t RESERVED4;

	volatile uint32_t RCC_APB1_LP_ENR;
	volatile uint32_t RCC_APB2_LP_ENR;

	uint32_t RESERVED5[2];

	volatile uint32_t RCC_BDCR;
	volatile uint32_t RCC_CSR;

	uint32_t RESERVED6[2];

	volatile uint32_t RCC_SSCGR;
	volatile uint32_t RCC_PLL_I2S_CFGR;
}rccRegOffset_t;

/*
 * ========================================================
 * 				 SINGLE-FIELD ADDRESS HELPERS
 * ========================================================
 */
#define GPIO_A_GET_REG(regOffset)	(&(((gpioRegOffset_t*) GPIO_A_BASE_ADDR) -> regOffset))
#define GPIO_B_GET_REG(regOffset)	(&(((gpioRegOffset_t*) GPIO_B_BASE_ADDR) -> regOffset))
#define GPIO_C_GET_REG(regOffset)	(&(((gpioRegOffset_t*) GPIO_C_BASE_ADDR) -> regOffset))

#define SPI_1_GET_REG(regOffset)	(&(((spiRegOffset_t*) SPI_1_BASE_ADDR) -> regOffset))
#define SPI_2_GET_REG(regOffset)	(&(((spiRegOffset_t*) SPI_2_BASE_ADDR) -> regOffset))
#define SPI_3_GET_REG(regOffset)	(&(((spiRegOffset_t*) SPI_3_BASE_ADDR) -> regOffset))

#define I2C_1_GET_REG(regOffset)	(&(((i2cRegOffset_t*) I2C_1_BASE_ADDR) -> regOffset))
#define I2C_2_GET_REG(regOffset)	(&(((i2cRegOffset_t*) I2C_2_BASE_ADDR) -> regOffset))
#define I2C_3_GET_REG(regOffset)	(&(((i2cRegOffset_t*) I2C_3_BASE_ADDR) -> regOffset))

#define FLASH_GET_REG(regOffset)	(&(((flashRegOffset_t*) FLASH_BASE_ADDR) -> regOffset))
#define RCC_GET_REG(regOffset)		(&(((rccRegOffset_t*) RCC_BASE_ADDR) -> regOffset))

#endif /* INC_STM32F405_ADDR_H_ */
