/*
 * icm42688p.c
 *
 *  Created on: Dec 22, 2025
 *      Author: dobao
 */

#include "icm42688.h"

/*
 * ===========================================================
 * 						GLOBAL PARAMETERS
 * ===========================================================
 */
uint32_t SPI_clockFreq = ICM42688_SPI_MAX_CLKFREQ;


/*
 * ===========================================================
 * 						PRIVATE PARAMETERS
 * ===========================================================
 */
static const float gyro_FSR_value[] = {2000.0f, 1000.0f, 500.0f, 250.0f, 125.0f, 65.5f, 31.25f, 15.625f};
static const float accel_FSR_value[] = {16.0f, 8.0f, 4.0f, 2.0f};

/*
 * ===========================================================
 * 					  PRIVATE PROTOTYPES
 * ===========================================================
 */
static inline void ICM42688_CS_Low(ICM42688_Handle_t *handle){
	HAL_GPIO_WritePin(handle -> cs_port, handle -> cs_pin, GPIO_PIN_RESET);
}


static inline void ICM42688_CS_High(ICM42688_Handle_t *handle){
	HAL_GPIO_WritePin(handle -> cs_port, handle -> cs_pin, GPIO_PIN_SET);
}


/*
 * @brief	Write a single byte to an ICM42688 register via SPI
 *
 * 			The chip select (CS) is pull low before transmission and release high after completion
 *
 * @param	handle		Pointer to the ICM42688 device handle struct
 * @param	regAddr		7-bit register address to be written
 * @param	val			Data byte to be written into the specific register address @p regAddr
 *
 * @retVal	HAL_OK		SPI transmission completed successfully
 * 			HAL_ERROR	Invalid handle or SPI transmission failure
 */
HAL_StatusTypeDef ICM42688_WriteReg(ICM42688_Handle_t *handle, uint8_t regAddr, uint8_t val){
	/* Sanity Checks */
	if((handle == NULL) || ((handle -> hspi) == NULL) || ((handle -> cs_port) == NULL)) return HAL_ERROR;

	/* 1st sent is write command + register address
	 * 2nd sent is data byte */
	uint8_t tx[2];
	tx[0] = (uint8_t)(regAddr & ICM42688_SPI_ADDR_MASK); //R/W = 0 + regAddr[6:0]
	tx[1] = (uint8_t)val;

	/* Start to send tx[1 and tx[0] via SPI */
	ICM42688_CS_Low(handle);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(handle -> hspi, tx, 2, ICM42688_SPI_TIMEOUT_MS);
	ICM42688_CS_High(handle);

	return status;
}


/*
 * @brief	Read single byte from ICM42688 register via SPI
 *
 * @param	handle		Pointer to the ICM42688 device handle struct
 * @param	regAddr		7-bit register address to be read
 * @param	outVal		Pointer to store the output byte that is read from the register
 *
 * @retVal	HAL_OK		SPI TX and RX completed successfullt
 * 			HAL_ERROR	Invalid handle and SPI TX, RX failure
 */
HAL_StatusTypeDef ICM42688_ReadReg(ICM42688_Handle_t* handle, uint8_t regAddr, uint8_t* outVal){
	if((!handle) || (!handle -> hspi) || (!handle -> cs_port)) return HAL_ERROR;

	uint8_t addr = (uint8_t)(regAddr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT;

	/* Start to send the address byte in read command */
	ICM42688_CS_Low(handle);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(handle -> hspi, &addr, 1, ICM42688_SPI_TIMEOUT_MS);

	/* Read a single byte */
	if(status == HAL_OK){
		status = HAL_SPI_Receive(handle -> hspi, outVal, 1, ICM42688_SPI_TIMEOUT_MS);
	}
	ICM42688_CS_High(handle);
	return status;
}


/*
 * @brief	Read multi-byte from ICM42688 register via SPI
 *
 * @param	handle			Pointer to ICM42688 handle struct
 * @param	startRegAddr	Initial 7-bit register address to be read
 * @param	buf				Pointer to the buffer
 * @param	bufLength		Size/length/No. of Elements of/in the buffer
 *
 */
HAL_StatusTypeDef ICM42688_ReadRegs(ICM42688_Handle_t* handle, uint8_t startRegAddr, uint8_t* buf, uint16_t bufLength){
	if((!handle) || (!handle -> hspi) || (!handle -> cs_port) || (!buf) || (bufLength == 0U)) return HAL_ERROR;

	uint8_t addr = (uint8_t)((startRegAddr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT);

	/* Start to send the address byte and read command */
	ICM42688_CS_Low(handle);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(handle -> hspi, &addr, 1, ICM42688_SPI_TIMEOUT_MS);

	/* Burst read */
	if(status == HAL_OK){
		status = HAL_SPI_Receive(handle -> hspi, buf, bufLength, ICM42688_SPI_TIMEOUT_MS);
	}

	ICM42688_CS_High(handle);
	return status;
}


/*
 * ============================================================
 * 						  PUBLIC APIs
 * ============================================================
 */
/*
 * @brief	A Verification Function that check if ICM42688 sensor is active
 */
HAL_StatusTypeDef ICM42688_IsAlive(ICM42688_Handle_t* handle){
	if(!handle) return HAL_ERROR;

	uint8_t who = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_WHO_AM_I, &who);
	if(status == HAL_ERROR) return status;

	return (who == ICM42688_WHO_AM_I_DEFAULT) ? HAL_OK : HAL_ERROR;
}


/*
 * @brief	Extract the current setting of register bank
 */
ICM42688_RegBank_t ICM42688_GetRegBank(ICM42688_Handle_t* handle){
	if(!handle) return REG_BANK_0;

	uint8_t curRegBank = 0U;
	if(ICM42688_ReadReg(handle, ICM42688_UB0_REG_BANK_SEL, &curRegBank) != HAL_OK){
		/* If reading is fail, fall back to cached value */
		return(ICM42688_RegBank_t)handle -> regBank;
	}
	curRegBank = curRegBank & 0x07U;
	return (ICM42688_RegBank_t)curRegBank;
}


/*
 * @brief	select/Modify Register Bank
 */
HAL_StatusTypeDef ICM42688_RegBankSel(ICM42688_Handle_t* handle, ICM42688_RegBank_t userRegBank){
	if(!handle) return HAL_ERROR;

	/* Fast path: if the user enter the same as cached regBank value, skip reading the chip */
	if((uint8_t)(handle -> regBank) == (uint8_t)userRegBank) return HAL_OK;

	HAL_StatusTypeDef status = ICM42688_WriteReg(handle, ICM42688_UB0_REG_BANK_SEL, (uint8_t)userRegBank);
	if(status == HAL_OK){
		/* If write is succeed, update cached register bank */
		handle -> regBank = (ICM42688_RegBank_t)userRegBank;
	}
	return status;
}


/*
 * @brief	Software reset ICM42688
 * @note	After triggering soft reset, wait 1ms for it to be effective
 */
HAL_StatusTypeDef ICM42688_SoftReset(ICM42688_Handle_t* handle){
	if(!handle) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_WriteReg(handle, ICM42688_UB0_DEVICE_CONF, ICM42688_DEV_CONF_SOFT_RESET_Msk);
	if(status != HAL_OK) return HAL_ERROR;

	/* Wait >= 1ms after reset */
	HAL_Delay(2);

	/* After reset, register bank is 0 */
	handle -> regBank = REG_BANK_0;

	return HAL_OK;
}


/*
 * @brief
 */
void ICM42688_Init(ICM42688_Handle_t* handle, ICM42688_Config_t config){
	/* Sanity Checks */
	if(!handle) return;
	if((config.gyro_fsr > GYRO_FSR_15dps625) || (config.accel_fsr > ACCEL_FSR_2g)) return;

	/* Precalculate sensitivity multiplier of Gyro and Accel
	 * Formula:	MAX FSR / 32768.0 */
	handle -> gyro_lsb_to_dps 	= (float)(gyro_FSR_value[config.gyro_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);
	handle -> accel_lsb_to_g 	= (float)(accel_FSR_value[config.accel_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);

	/* Register Bank Selection */
	(void)ICM42688_RegBankSel(handle, handle -> regBank);
}








