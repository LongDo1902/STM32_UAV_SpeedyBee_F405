/*
 * icm42688_rw.c
 *
 *  Created on: Mar 5, 2026
 *      Author: dobao
 */
#include "imu/icm42688_rw.h"

/*
 * =============================================================================
 *	PRIVATE HELPERS
 * =============================================================================
 */
static inline void ICM42688_CS_Low(ICM42688_Handle_t *handle)
{
	//Pull CS low to start an SPI transaction
	HAL_GPIO_WritePin(handle -> spi_config.cs_port, handle -> spi_config.cs_pin, GPIO_PIN_RESET);
}


static inline void ICM42688_CS_High(ICM42688_Handle_t *handle)
{
	//Pull CS high to end an SPI transaction
	HAL_GPIO_WritePin(handle -> spi_config.cs_port, handle -> spi_config.cs_pin, GPIO_PIN_SET);
}




/*
 * =============================================================================
 *	LOW-LEVEL REGISTER ACCESS
 * =============================================================================
 */
/* @brief	Automatically write bank number to the corresponding input encoded register
 * @param	handle			Pointer to ICm42688 Handle struct
 * @param	encodedReg		Encoded register containing both bank and register address */
HAL_StatusTypeDef ICM42688_WriteBankAuto(ICM42688_Handle_t* handle, ICM42688_Reg_t encodedReg)
{
	ICM42688_RegBank_t bank = ICM42688_REG_BANK(encodedReg);
	if((bank > REG_BANK_4) || (bank == REG_BANK_3)) return HAL_ERROR;

	uint8_t bankSelAddr = ICM42688_REG_ADDR(ICM42688_UB0_REG_BANK_SEL);
	uint8_t bank_tx[2] = {
			/* 1st sent is register address + write command bit
			 * 2nd sent is data byte */
			(uint8_t)(bankSelAddr & ICM42688_SPI_ADDR_MASK),
			(uint8_t)(bank)
	};

	ICM42688_CS_Low(handle);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(handle -> spi_config.hspi, bank_tx, 2, ICM42688_SPI_TIMEOUT_MS);
	ICM42688_CS_High(handle);
	return status;
}


/* @brief	Write one byte to the target ICM42688 register over SPI
 * @param	handle		Pointer to the ICM42688 Handle struct
 * @param	val			Data byte to be written into target register */
HAL_StatusTypeDef ICM42688_WriteReg(ICM42688_Handle_t* handle, ICM42688_Reg_t encodedReg, uint8_t val)
{
	if((!handle) || (!handle -> spi_config.hspi) ||
	(!handle -> spi_config.cs_port)) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_WriteBankAuto(handle, encodedReg);
	if(status != HAL_OK) return status;

	/* Extract register address information and start writing to desired register */
	/* 1st sent is write command + register address
	 * 2nd sent is data byte */
	uint8_t regAddr = ICM42688_REG_ADDR(encodedReg);
	uint8_t tx[2] = {
			(uint8_t)(regAddr & ICM42688_SPI_ADDR_MASK),
			val
	};

	ICM42688_CS_Low(handle);
	status = HAL_SPI_Transmit(handle -> spi_config.hspi, tx, 2, ICM42688_SPI_TIMEOUT_MS);
	ICM42688_CS_High(handle);

	return status;
}


/* @brief	Read one byte from the target ICM42688 over SPI
 * @param	handle		Pointer to ICM42688 Handle struct
 * @param	encodedReg	Encoded register containing both bank number and register address
 * @param	outVal		Pointer to a variable that stores the read register value */
HAL_StatusTypeDef ICM42688_ReadReg(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t* outVal)
{
	if((!handle) || (!outVal) ||
	(!handle -> spi_config.hspi) ||
	(!handle -> spi_config.cs_port)) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_WriteBankAuto(handle, encodedReg);
	if(status != HAL_OK) return status;

	/* Extract register address information and start writing to desired register */
	/* 1st sent is write command + register address
	 * 2nd sent is data byte */
	uint8_t regAddr	= ICM42688_REG_ADDR(encodedReg);
	uint8_t tx[2] = {
			(uint8_t)((regAddr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT),
			0x00U //Dummy to clock out data
	};
	uint8_t rx[2] = {0};

	ICM42688_CS_Low(handle);
	status = HAL_SPI_TransmitReceive(handle -> spi_config.hspi, tx, rx, 2, ICM42688_SPI_TIMEOUT_MS);
	ICM42688_CS_High(handle);

	if(status == HAL_OK) *outVal = rx[1]; //rx[0] corresponding to address phase (dummy)/undefined value

	return status;
}


/* @brief	Read multiple consecutive registers from the ICM42688 over SPI
 * @param	handle				Pointer to ICM42688 Handle struct
 * @param	startEncodedReg 	Encoded start register
 * @param	buf					Pointer to the buffer that stores the received bytes
 * @param	bufLength			Number of consecutive bytes to read */
HAL_StatusTypeDef ICM42688_ReadRegs(ICM42688_Handle_t* handle, ICM42688_Reg_t startEncodedReg,
									uint8_t* buf, uint16_t bufLength)
{
	if((!handle) || (!buf) || (bufLength == 0) ||
	(!handle -> spi_config.hspi) ||
	(!handle -> spi_config.cs_port)) return HAL_ERROR;

	HAL_StatusTypeDef status = ICM42688_WriteBankAuto(handle, startEncodedReg);
	if(status != HAL_OK) return status;

	/* Extract register address information and start writing to desired register */
	/* 1st sent is write command + register address
	 * 2nd sent is data byte */
	uint8_t regAddr = ICM42688_REG_ADDR(startEncodedReg);
	if(((uint16_t)regAddr + bufLength) > 0x80U) return HAL_ERROR;

	/* Send read command + start addr, then burst receive */
	uint8_t addr = (uint8_t)((regAddr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT);

	ICM42688_CS_Low(handle);

	status = HAL_SPI_Transmit(handle -> spi_config.hspi, &addr, 1, ICM42688_SPI_TIMEOUT_MS);
	if(status == HAL_OK){
		status = HAL_SPI_Receive(handle -> spi_config.hspi, buf, bufLength, ICM42688_SPI_TIMEOUT_MS);
	}

	ICM42688_CS_High(handle);
	return status;
}


/* @brief	Updates selected bit fields of a target register using read-modify-write operation
 * @param	handle			Pointer to ICM42688 Handle struct
 * @param	encodedReg		Encoded register value
 * @param	mask			Bit mask indicating which register bits will be updated
 * @param	valueMasked		New field value already shifted and masked to match the mask */
HAL_StatusTypeDef ICM42688_Update_Reg_Bits(ICM42688_Handle_t* handle, ICM42688_Reg_t encodedReg,
										uint8_t mask, uint8_t valueMasked)
{
	if(!handle) return HAL_ERROR;
	if((valueMasked & (uint8_t)~mask) != 0U) return HAL_ERROR;

	uint8_t cur_reg = 0U;
	HAL_StatusTypeDef status = ICM42688_ReadReg(handle, encodedReg, &cur_reg);
	if(status != HAL_OK) return status;

	cur_reg = (uint8_t)((cur_reg & (uint8_t)~mask) | valueMasked);
	return ICM42688_WriteReg(handle, encodedReg, cur_reg);
}
