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
	HAL_GPIO_WritePin(handle -> spi_io.cs_port, handle -> spi_io.cs_pin, GPIO_PIN_RESET);
}


static inline void ICM42688_CS_High(ICM42688_Handle_t *handle){
	HAL_GPIO_WritePin(handle -> spi_io.cs_port, handle -> spi_io.cs_pin, GPIO_PIN_SET);
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
	if((!handle) || ((handle -> spi_io.hspi) == NULL) || ((handle -> spi_io.cs_port) == NULL)) return HAL_ERROR;

	/* 1st sent is write command + register address
	 * 2nd sent is data byte */
	uint8_t tx[2];
	tx[0] = (uint8_t)(regAddr & ICM42688_SPI_ADDR_MASK); //R/W = 0 + regAddr[6:0]
	tx[1] = (uint8_t)val;

	/* Start to send tx[1 and tx[0] via SPI */
	ICM42688_CS_Low(handle);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(handle -> spi_io.hspi, tx, 2, ICM42688_SPI_TIMEOUT_MS);
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
	/* Sanity checks */
	if((!handle) || ((handle -> spi_io.hspi) == NULL) || ((handle -> spi_io.cs_port) == NULL)) return HAL_ERROR;

	uint8_t addr = (uint8_t)(regAddr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT;

	/* Start to send the address byte in read command */
	ICM42688_CS_Low(handle);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(handle -> spi_io.hspi, &addr, 1, ICM42688_SPI_TIMEOUT_MS);

	/* Read a single byte */
	if(status == HAL_OK){
		status = HAL_SPI_Receive(handle -> spi_io.hspi, outVal, 1, ICM42688_SPI_TIMEOUT_MS);
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
	if((!handle) || (!handle -> spi_io.hspi) || (!handle -> spi_io.cs_port) || (!buf) || (bufLength == 0U)) return HAL_ERROR;

	uint8_t addr = (uint8_t)((startRegAddr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT);

	/* Start to send the address byte and read command */
	ICM42688_CS_Low(handle);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(handle -> spi_io.hspi, &addr, 1, ICM42688_SPI_TIMEOUT_MS);

	/* Burst read */
	if(status == HAL_OK){
		status = HAL_SPI_Receive(handle -> spi_io.hspi, buf, bufLength, ICM42688_SPI_TIMEOUT_MS);
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
 * @brief	Select the active register bank of the ICM42688
 *
 * @note	Write the desired register bank to USB0.REG_BANK_SEL
 * 			and updates the cache register bank in the handle on success
 * 			If the requested register bank is already selected,
 * 			no SPI transmit is performed.
 *
 * @param	handle		Pointer to ICM42688 handle
 * @param	regBank		Target register bank to select
 *
 * @retVal	HAL_OK		Write Successfully
 * 			HAL_ERROR	Invalid handle struct or register bank selection
 */
HAL_StatusTypeDef ICM42688_RegBank_Sel(ICM42688_Handle_t* handle, ICM42688_RegBank_t regBank){
	/* Sanity checks */
	if(!handle || (regBank > REG_BANK_4)) return HAL_ERROR;
	/* Already selected case */
	if(handle -> regBank == regBank) return HAL_OK;
	/* Write the selected register bank to the register */
	HAL_StatusTypeDef status = ICM42688_WriteReg(handle, ICM42688_UB0_REG_BANK_SEL, (uint8_t)regBank);
	/* Save the selected register bank to update cache only if success*/
	if(status == HAL_OK) handle -> regBank = regBank;
	return status;
}


/*
 * @brief	Get the existing register bank in cache
 * @note	const	Read-only mode
 * @param	handle	Pointer to ICM42688 handle
 */
inline ICM42688_RegBank_t ICM42688_Get_RegBankCached(const ICM42688_Handle_t* handle){
	return handle ? handle -> regBank : REG_BANK_0;
}


/*
 * @brief	Read and Extract the register bank setting from the sensor
 *
 * @param	handle	Pointer to ICM42688 handle
 * @param	regBank	Pointer to the variable that receives the current register bank
 *
 * @retVal	HAL_OK		On success
 * 			HAL_ERROR	If argument is invalid
 */
HAL_StatusTypeDef ICM42688_Get_RegBankSensor(ICM42688_Handle_t* handle, ICM42688_RegBank_t* regBank){
	/* Sanity Check */
	if((!handle) || (!regBank)) return HAL_ERROR;

	uint8_t val = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_REG_BANK_SEL, &val);
	if(status != HAL_OK) return status;

	*regBank = (ICM42688_RegBank_t)(val & 0x07U);

	return status;
}


/*
 * @brief	Get the return WHO_AM_I value from the sensor
 *
 * @param	handle		Pointer to the ICM42688 handle
 * @param	who_val		Pointer to a variable to receive the returned WHO_AM_I value
 *
 * @retVal	HAL_OK		On success
 * 			HAL_ERROR	Invalid handle or who_val pointer
 */
inline HAL_StatusTypeDef ICM42688_Get_WhoAmI(ICM42688_Handle_t* handle, uint8_t* who_val){
	if(!handle || !who_val) return HAL_ERROR;
	return ICM42688_ReadReg(handle, ICM42688_UB0_WHO_AM_I, who_val);
}


/*
 * @brief	Verify if the sensor is active by checking WHO_AM_I value
 *
 * @param	handle	Pointer to ICM42688 handle
 *
 * @retVal	HAL_OK		The sensor is alive
 * 			HAL_ERROR	Invalid handle or sensor is not alive
 */
HAL_StatusTypeDef ICM42688_IsAlive(ICM42688_Handle_t* handle){
	/* Sanity check */
	if(!handle) return HAL_ERROR;

	/* Read and extract the returned value from WHO_AM_I register */
	uint8_t who = 0U;
	HAL_StatusTypeDef status = ICM42688_Get_WhoAmI(handle, &who);
	if(status != HAL_OK){
		handle -> isAlive = false;
		return status;
	}

	/* Check if returned value is correct by comparing it to 0x47 */
	if(who == ICM42688_WHO_AM_I_DEFAULT){
		handle -> isAlive = true;
		return HAL_OK;
	} else{
		handle -> isAlive = false;
		return HAL_ERROR;
	}
}


/*
 * @brief	Select the desired SPI mode in DEVICE_CONFIG
 * 			- SPI_MODE_0_3 => SPI_MODE bit = 0
 * 			- SPI_MODE_1_2 => SPI_MODE bit = 1
 * 			THe function performs a read-modify-write so other bits are preserved
 */
HAL_StatusTypeDef ICM42688_SPI_Mode_Sel(ICM42688_Handle_t* handle, ICM42688_SPI_Mode_t spiMode){
	if(!handle) return HAL_ERROR;

	/* Skip writing if SPI mode is already set in cache */
	if(spiMode == (handle -> config.spiMode)){
		return HAL_OK;
	}

	/* The target SPI mode differs from cached SPI mode */
	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_DEVICE_CONF, &reg);
	if(status != HAL_OK) return status;

	/* Clear bit and start to write the target SPI mode to the sensor */
	reg &= (uint8_t)~ICM42688_DEVICE_CONFIG_SPI_MODE_Msk;
	reg |= ICM42688_DEVICE_CONFIG_SPI_MODE_Val(spiMode);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_DEVICE_CONF, reg);

	/* Update Cache */
	if(status == HAL_OK) handle -> config.spiMode = spiMode;

	return status;
}


/*
 * @brief	Software reset ICM42688
 * @note	After triggering soft reset, wait 1ms for it to be effective
 * @param	handle	Pointer to ICM42688 handle
 * @retVal	HAL_ERROR	Invalid handle, fail to write to sensor
 * 			HAL_OK		Successfully enable software reset
 */
HAL_StatusTypeDef ICM42688_SoftReset(ICM42688_Handle_t* handle){
	if(!handle) return HAL_ERROR;

	/* Extract the current soft-reset bit setting */
	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_DEVICE_CONF, &reg);
	if(status != HAL_OK) return status;

	/* Start to enable soft-reset */
	reg |= (uint8_t)ICM42688_DEVICE_CONFIG_SOFT_RESET_Msk;
	status = ICM42688_WriteReg(handle, ICM42688_UB0_DEVICE_CONF, reg);
	if(status != HAL_OK) return status;

	/* Wait >= 1ms after reset */
	HAL_Delay(5);

	/* After reset, set every flag to a default/known state */
	handle -> isInitialized = false;
	handle -> isAlive = false;
	handle -> regBank = REG_BANK_0;

	return status;
}


/*
 * @brief	Get the current slew rate setting from the sensor
 *
 * @param	handle		Pointer to handle struct
 * @param	slewRate	Pointer to a variable to receive the returned slew rate from the sensor
 *
 * @retVal	HAL_ERROR	Invalid handle OR read failure
 * 			HAL_OK		Operate successfully
 */
HAL_StatusTypeDef ICM42688_SPI_Get_SlewRate(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t* slewRate){
	if(!handle || !slewRate) return HAL_ERROR;

	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_DRIVE_CONF, &reg);
	if(status != HAL_OK) return status;

	reg &= ICM42688_DRIVE_CONFIG_SPI_SR_Msk;
	*slewRate = (ICM42688_SPI_SLEWRATE_t)(reg >> ICM42688_DRIVE_CONFIG_SPI_SR_Pos);

	return status;
}


/*
 * @brief	Control Slew Rate of all output pins
 *
 * @param	handle		Pointer to handle struct
 * @param	slewRate	Target SPI Slew Rate for output data pin
 *
 * @retVal	HAL_ERROR	Invalid handle struct OR read failure OR write failure
 * 			HAL_OK		Operate successfully
 */
HAL_StatusTypeDef ICM42688_SPI_SlewRate_Sel(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t slewRate){
	/* Sanity Checks */
	if(!handle) return HAL_ERROR;
	if((uint8_t)slewRate > 5U) return HAL_ERROR;

	/* Extract the current setting of slew rate from the sensor */
	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_DRIVE_CONF, &reg);
	if(status != HAL_OK) return status;

	/* Clear, assign bit and write bit to the sensor */
	reg &= (uint8_t)~(ICM42688_DRIVE_CONFIG_SPI_SR_Msk);
	reg |= (uint8_t)ICM42688_DRIVE_CONFIG_SPI_SR_Val(slewRate);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_DRIVE_CONF, reg);

	/* If succeed, update cached */
	if(status == HAL_OK) handle -> config.spiSlewRate = (ICM42688_SPI_SLEWRATE_t)slewRate;
	return status;
}


/*
 * @brief	Initialize and reset ICM42688 Sensor to the default settings
 */
HAL_StatusTypeDef ICM42688_Init(ICM42688_Handle_t* handle){
	/* Sanity Checks */
	if(!handle) return HAL_ERROR;

	/* Check if the sensor is alive */
	if(ICM42688_IsAlive(handle) != HAL_OK) return HAL_ERROR;

	/* Soft reset the whole sensor */
	if(ICM42688_SoftReset(handle) != HAL_OK) return HAL_ERROR;

	/* Recheck if the sensor is alive after reset */
	if(ICM42688_IsAlive(handle) != HAL_OK) return HAL_ERROR;

	/* Configure Default */
	if(ICM42688_RegBank_Sel(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	if(ICM42688_SPI_Mode_Sel(handle, SPI_MODE_0_3) != HAL_OK) return HAL_ERROR;
	if(ICM42688_SPI_SlewRate_Sel(handle, SPI_SR_2NS) != HAL_OK) return HAL_ERROR;

	/* Precalculate sensitivity multiplier of Gyro and Accel
	 * Formula:	MAX FSR / 32768.0 */
	handle -> gyro_lsb_to_dps	= (float)(gyro_FSR_value[handle -> config.gyro_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);
	handle -> accel_lsb_to_g	= (float)(accel_FSR_value[handle -> config.accel_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);

	handle -> isInitialized = true;
	return HAL_OK;
}



///*
// * @brief	Read gyroscope data
// */
//int16_t ICM42688_readGyroX(ICM42688_Handle_t* handle){
//	uint8_t gyroX[2]; //stores upper and lower data
//	ICM42688_ReadRegs(handle, ICM42688_UB0_GYRO_DATA_X0, gyroX, sizeof(gyroX));
//	return (int16_t)((gyroX[1] << 8U) | (gyroX[0] << 0U));
//}


