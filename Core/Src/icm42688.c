/*
 * icm42688p.c
 *
 *  Created on: Dec 22, 2025
 *      Author: dobao
 */

#include "icm42688.h"

/*
 * =============================================================================
 * 								PRIVATE CONSTANTS
 * =============================================================================
 */
/* Register-field code -> physical full-scale value mapping */
static const float gyro_FSR_value[]		= {2000.0f, 1000.0f, 500.0f, 250.0f, 125.0f, 65.5f, 31.25f, 15.625f};
static const float accel_FSR_value[]	= {16.0f, 8.0f, 4.0f, 2.0f};


/*
 * =============================================================================
 * 					  			PRIVATE HELPERS
 * =============================================================================
 */
static inline void ICM42688_CS_Low(ICM42688_Handle_t *handle){
	HAL_GPIO_WritePin(handle -> spi_io.cs_port, handle -> spi_io.cs_pin, GPIO_PIN_RESET);
}


static inline void ICM42688_CS_High(ICM42688_Handle_t *handle){
	HAL_GPIO_WritePin(handle -> spi_io.cs_port, handle -> spi_io.cs_pin, GPIO_PIN_SET);
}


/*
 * @brief Ensure a register bank is selected (and cache is in sync)
 *
 * @param handle   Device handle
 * @param bank     Target bank
 *
 * @return		HAL_OK on success
 * 				HAL_ERROR on invalid args or SPI failure
 */
static HAL_StatusTypeDef ICM42688_EnsureRegBank(ICM42688_Handle_t* handle, ICM42688_RegBank_t bank){
	if(!handle) return HAL_ERROR;
	if((bank > REG_BANK_4) || (bank == REG_BANK_3)) return HAL_ERROR;

	/* Fast path: if cache already says correct bank and you trust cache,
	 * you could skip the sensor-read. But your IsRegBank() resyncs, so
	 * we keep it deterministic */
	if(ICM42688_IsRegBank(handle, bank) == HAL_OK) return HAL_OK;

	return ICM42688_Set_RegBank(handle, bank);
}


/*
 * =============================================================================
 * 					 		 LOW-LEVEL REGISTER ACCESS
 * =============================================================================
 */
/*
 * @brief	Write a single byte to an ICM42688 register via SPI
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
 * =============================================================================
 * 						  		 BANK MANAGEMENTS
 * =============================================================================
 */
/*
 * @brief	Select the active register bank of the ICM42688
 *
 * @note	Write the desired register bank to USB0.REG_BANK_SEL
 * 			and updates the cache register bank in the handle on success
 * 			If the requested register bank is already selected,
 * 			no SPI transmit is performed.
 *
 * @param	handle		Pointer to a valid ICM42688 handle structure
 * @param	regBank		Target register bank to select
 * 						(REG_BANK_0, REG_BANK_1, REG_BANK_2, REG_BANK_4)
 *
 * @retVal	HAL_OK		Register bank successfully selected
 * 			HAL_ERROR	Invalid handle or invalid register bank selection
 */
HAL_StatusTypeDef ICM42688_Set_RegBank(ICM42688_Handle_t* handle, ICM42688_RegBank_t regBank){
	/* Sanity checks */
	if(!handle || (regBank > REG_BANK_4) || (regBank == REG_BANK_3)) return HAL_ERROR;

	/* Force write for the first time */
	if(handle -> isInitialized == true){
		/* Already selected case unless unless the first initialization is performed */
		if(regBank == (handle -> regBank)) return HAL_OK;
	}

	/* Write the selected register bank to the register */
	HAL_StatusTypeDef status = ICM42688_WriteReg(handle, ICM42688_UB0_REG_BANK_SEL, (uint8_t)regBank);
	/* Save the selected register bank to update cache only if success*/
	if(status == HAL_OK) handle -> regBank = regBank;
	return status;
}


/*
 * @brief	Get the currently existing cached register bank
 *
 *
 * @note	Returns the register bank value stored in the driver cache.
 * 			This function does NOT access the sensor hardware and
 * 			therefore does not guarantee that the cached value matches
 * 			the actual bank currently selected on the sensor.
 *
 * 			Intended for fast checks or debugging purposes.
 *
 * @param	handle	Pointer to ICM42688 handle
 *
 * @retVal	Cached register bank value
 * 			REG_BANK_0 is returned if handle is NULL.
 */
inline ICM42688_RegBank_t ICM42688_Get_RegBankCached(const ICM42688_Handle_t* handle){
	return handle ? handle -> regBank : REG_BANK_0;
}


/*
 * @brief	Read the active register bank directly from the sensor
 *
 * @param	handle		Pointer to a valid ICM42688 handle
 * @param	regBank		Pointer to a variable that receives the active register bank
 * 						read from the sensor
 *
 * @retVal	HAL_OK		Register bank successfully read
 * 			HAL_ERROR	Invalid arguments or SPI read failure
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
 * @brief	Verify and synchronize the active register bank
 *
 * @param	handle					Pointer to a valid ICM42688 handle
 * @param	targetRegBankToCheck	Register bank expected to be active
 *
 * @retVal	HAL_OK		Sensor is currently operating in the target bank
 * 			HAL_ERROR	Invalid parameters
 * 						SPI failure
 * 						Bank mismatch
 */
HAL_StatusTypeDef ICM42688_IsRegBank(ICM42688_Handle_t* handle, ICM42688_RegBank_t targetRegBankToCheck){
	/* Sanity check */
	if(!handle) return HAL_ERROR;
	if((targetRegBankToCheck > REG_BANK_4) || (targetRegBankToCheck == REG_BANK_3)) return HAL_ERROR;

	ICM42688_RegBank_t regBankSensor = REG_BANK_0;
	if(ICM42688_Get_RegBankSensor(handle, &regBankSensor) != HAL_OK) return HAL_ERROR;

	/* Resync cache */
	handle -> regBank = regBankSensor;

	return(regBankSensor == targetRegBankToCheck) ? HAL_OK : HAL_ERROR;
}


/*
 * =============================================================================
 * 							IDENTITY / RESET / SPI CONFIG
 * =============================================================================
 */
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
HAL_StatusTypeDef ICM42688_Set_SPI_Mode(ICM42688_Handle_t* handle, ICM42688_SPI_Mode_t spiMode){
	/* Sanity checks */
	if(!handle) return HAL_ERROR;

	/* Force write for the first time */
	if(handle -> isInitialized == true){
		/* Skip writing if SPI mode is already set in cache */
		if(spiMode == (handle -> config.spiMode)){
			return HAL_OK;
		}
	}

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
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

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	}

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
	handle -> isReset		= true;
	handle -> isInitialized = false;
	handle -> isAlive 		= false;
	handle -> regBank 		= REG_BANK_0;

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
HAL_StatusTypeDef ICM42688_Get_SPI_SlewRate(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t* slewRate){
	if(!handle || !slewRate) return HAL_ERROR;

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	}

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
HAL_StatusTypeDef ICM42688_Set_SPI_SlewRate(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t slewRate){
	/* Sanity Checks */
	if(!handle) return HAL_ERROR;
	if((uint8_t)slewRate > 5U) return HAL_ERROR;

	/* Force write for the first time */
	if(handle -> isInitialized == true){
		/* Skip writing if SPI Slew Rate is already set in cache */
		if(slewRate == (handle -> config.spiSlewRate)) return HAL_OK;
	}

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	}

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
 * =============================================================================
 * 									GYRO CONFIG
 * =============================================================================
 */
/*
 * @brief	Configure Gyro Mode
 *
 * @param	handle	Pointer to handle struct
 * @param	mode	Target/desired gyro mode to be set
 *
 * @retVal	HAL_ERROR	Invalid handle or mode or write/read failure
 * 			HAL_OK		Successfully built
 */
HAL_StatusTypeDef ICM42688_Set_GyroMode(ICM42688_Handle_t* handle, ICM42688_GyroMode_t mode){
	if(!handle) return HAL_ERROR;
	if(((uint8_t)mode > 3U) || ((uint8_t)mode == 2U)) return HAL_ERROR;

	/* Force write for the first time */
	if(handle -> isInitialized == true){
		/* Skip writing if Gyro Mode is already set in cache */
		if(mode == (handle -> config.gyro_mode)) return HAL_OK;
	}

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	}

	/* Extract the bit field of the whole register */
	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &reg);
	if(status != HAL_OK) return status;

	/* Start to write */
	reg &= (uint8_t)~ICM42688_GYRO_MODE_Msk;
	reg |= (uint8_t)ICM42688_GYRO_MODE_Val(mode);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_PWR_MGMT0, reg);
	if(status == HAL_OK) handle -> config.gyro_mode = (ICM42688_GyroMode_t)mode;

	return status;
}


/*
 * @brief	Configure Output Data Rate of Gyro
 */
HAL_StatusTypeDef ICM42688_Set_GyroODR(ICM42688_Handle_t* handle, ICM42688_GyroODR_t odr){
	/* Sanity check */
	if(!handle) return HAL_ERROR;
	if(((uint8_t)odr > (uint8_t)GYRO_ODR_500Hz) ||
	  ((uint8_t)odr == 0x0CU) ||
	  ((uint8_t)odr == 0x0DU) ||
	  ((uint8_t)odr == 0x0EU)) return HAL_ERROR;

	/* Force write for the first time */
	if(handle -> isInitialized == true){
		/* Skip writing if Gyro ODR is already set in cache */
		if(odr == (handle -> config.gyro_odr)) return HAL_OK;
	}

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	}

	/* Extract the bit field of the whole register */
	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_GYRO_CONF0, &reg);
	if(status != HAL_OK) return status;

	/* Start writting */
	reg &= (uint8_t)~ICM42688_GYRO_ODR_Msk;
	reg |= (uint8_t)ICM42688_GYRO_ODR_Val(odr);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_GYRO_CONF0, reg);
	if(status == HAL_OK) handle -> config.gyro_odr = (ICM42688_GyroODR_t)odr;

	return status;
}


/*
 * @brief	Configure Full Scale of Gyro
 */
HAL_StatusTypeDef ICM42688_Set_GyroFS(ICM42688_Handle_t* handle, ICM42688_GyroFSR_t fullScale){
	/* Sanity check */
	if(!handle) return HAL_ERROR;
	if((uint8_t)fullScale > (uint8_t)GYRO_FSR_15dps625) return HAL_ERROR;

	/* Force write for the first time */
	if(handle -> isInitialized == true){
		/* Skip writing if Gyro full scale is already set in cache */
		if(fullScale == (handle -> config.gyro_fsr)) return HAL_OK;
	}

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	}

	/* Extract the bit field of the whole register */
	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_GYRO_CONF0, &reg);
	if(status != HAL_OK) return status;

	/* Start writting */
	reg &= (uint8_t)~ICM42688_GYRO_FS_SEL_Msk;
	reg |= (uint8_t)ICM42688_GYRO_FS_SEL_Val(fullScale);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_GYRO_CONF0, reg);
	if(status == HAL_OK) handle -> config.gyro_fsr = (ICM42688_GyroFSR_t)fullScale;

	return status;
}


/*
 * =============================================================================
 * 									ACCEL CONFIG
 * =============================================================================
 */
/*
 * @brief	Configure Accel Mode
 *
 * @param	handle	Pointer to handle struct
 * @param	mode	Target/desired accel mode to be set
 *
 * @retVal	HAL_ERROR	Invalid handle or mode or write/read failure
 * 			HAL_OK		Successfully built
 */
HAL_StatusTypeDef ICM42688_Set_AccelMode(ICM42688_Handle_t* handle, ICM42688_AccelMode_t mode){
	if(!handle) return HAL_ERROR;
	if((uint8_t)mode > 3U) return HAL_ERROR;

	/* Force write for the first time */
	if(handle -> isInitialized == true){
		/* Skip writing if Accel Mode is already set in cache */
		if(mode == (handle -> config.accel_mode)) return HAL_OK;
	}

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	}

	/* Extract the bit field of the whole register */
	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &reg);
	if(status != HAL_OK) return status;

	/* Start to write */
	reg &= (uint8_t)~ICM42688_ACCEL_MODE_Msk;
	reg |= (uint8_t)ICM42688_ACCEL_MODE_Val(mode);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_PWR_MGMT0, reg);
	if(status == HAL_OK) handle -> config.accel_mode = (ICM42688_AccelMode_t)mode;

	return status;
}


/*
 * @brief	Configure Output Data Rate of Accel
 */
HAL_StatusTypeDef ICM42688_Set_AccelODR(ICM42688_Handle_t* handle, ICM42688_AccelODR_t odr){
	/* Sanity check */
	if(!handle) return HAL_ERROR;
	if(((uint8_t)odr > (uint8_t)ACCEL_ODR_500Hz) || ((uint8_t)odr == 0x00U)) return HAL_ERROR;

	/* Force write for the first time */
	if(handle -> isInitialized == true){
		/* Skip writing if Accel ODR is already set in cache */
		if(odr == (handle -> config.accel_odr)) return HAL_OK;
	}

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	}

	/* Extract the bit field of the whole register */
	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_ACCEL_CONF0, &reg);
	if(status != HAL_OK) return status;

	/* Start writting */
	reg &= (uint8_t)~ICM42688_ACCEL_ODR_Msk;
	reg |= (uint8_t)ICM42688_ACCEL_ODR_Val(odr);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_ACCEL_CONF0, reg);
	if(status == HAL_OK) handle -> config.accel_odr = (ICM42688_AccelODR_t)odr;

	return status;
}


/*
 * @brief	Configure Full Scale of Accel
 */
HAL_StatusTypeDef ICM42688_Set_AccelFS(ICM42688_Handle_t* handle, ICM42688_AccelFSR_t fullScale){
	/* Sanity check */
	if(!handle) return HAL_ERROR;
	if((uint8_t)fullScale > (uint8_t)ACCEL_FSR_2g) return HAL_ERROR;

	/* Force write for the first time */
	if(handle -> isInitialized == true){
		/* Skip writing if Accel full scale is already set in cache */
		if(fullScale == (handle -> config.accel_fsr)) return HAL_OK;
	}

	/* Check if it is Register Bank 0 */
	if(ICM42688_IsRegBank(handle, REG_BANK_0) != HAL_OK){
		if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	}

	/* Extract the bit field of the whole register */
	uint8_t reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_ACCEL_CONF0, &reg);
	if(status != HAL_OK) return status;

	/* Start writting */
	reg &= (uint8_t)~ICM42688_ACCEL_FS_SEL_Msk;
	reg |= (uint8_t)ICM42688_ACCEL_FS_SEL_Val(fullScale);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_ACCEL_CONF0, reg);
	if(status == HAL_OK) handle -> config.accel_fsr = (ICM42688_AccelFSR_t)fullScale;

	return status;
}


/*
 * =============================================================================
 * 								  INITIALIZE
 * =============================================================================
 */
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

	/* Force the sensors OFF */
	if(ICM42688_Set_GyroMode(handle, GYRO_OFF) != HAL_OK) return HAL_ERROR;
	if(ICM42688_Set_AccelMode(handle, ACCEL_OFF) != HAL_OK) return HAL_ERROR;

	/* Configure Default */
	if(ICM42688_Set_RegBank(handle, REG_BANK_0) != HAL_OK) return HAL_ERROR;
	if(ICM42688_Set_SPI_Mode(handle, SPI_MODE_0_3) != HAL_OK) return HAL_ERROR;
	if(ICM42688_Set_SPI_SlewRate(handle, SPI_SR_2NS) != HAL_OK) return HAL_ERROR;

	/* Configure Gyro */
	ICM42688_Set_GyroODR(handle, GYRO_ODR_1KHz);
	ICM42688_Set_GyroFS(handle, GYRO_FSR_1000dps);

	/* Configure Accel */
	ICM42688_Set_AccelODR(handle, ACCEL_ODR_1KHz);
	ICM42688_Set_AccelFS(handle, ACCEL_FSR_16g);

	/* Precalculate sensitivity multiplier of Gyro and Accel
	 * Formula:	MAX FSR / 32768.0 */
	handle -> gyro_lsb_to_dps	= (float)(gyro_FSR_value[handle -> config.gyro_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);
	handle -> accel_lsb_to_g	= (float)(accel_FSR_value[handle -> config.accel_fsr] / ICM42688_SENSITIVITY_SCALE_FACTOR);

	handle -> isInitialized = true;
	return HAL_OK;
}
