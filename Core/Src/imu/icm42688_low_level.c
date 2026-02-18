/*
 * icm42688p.c
 *
 *  Created on: Dec 22, 2025
 *      Author: dobao
 */

#include "imu/icm42688_low_level.h"

/*
 * =============================================================================
 * 								PRIVATE CONST DECLARATIONS
 * =============================================================================
 */
static const float lsb_per_dps[] = {
		16.4f,		//2000
		32.8f,		//1000
		65.5f,		//500
		131.0f,		//250
		262.0f,		//125
		524.3f,		//62.5
		1048.6f,	//31.25
		2097.2f		//15.625
};


static const float lsb_per_g[] = {
		2048.0f,	//16
		4096.0f,	//8
		8192.0f,	//4
		16384.0f	//2
};




/*
 * =============================================================================
 * 					  			PRIVATE HELPERS
 * =============================================================================
 */
static inline void ICM42688_CS_Low(ICM42688_Handle_t *handle)
{
	HAL_GPIO_WritePin(handle -> spi_config.cs_port, handle -> spi_config.cs_pin, GPIO_PIN_RESET);
}


static inline void ICM42688_CS_High(ICM42688_Handle_t *handle)
{
	HAL_GPIO_WritePin(handle -> spi_config.cs_port, handle -> spi_config.cs_pin, GPIO_PIN_SET);
}


static inline void ICM42688_Update_ScaleFactor(ICM42688_Handle_t* handle, ICM42688_SensorSel_t gyro_or_accel)
{
	/* Assumes handle valid
	 * 1 lsb	? FSR (?dps)
	 * 32768	FSR (e.g., 2000dps) */
	switch(gyro_or_accel){
		case GYRO: {
			uint8_t idx = (uint8_t)handle -> gyro_config.gyro_fsr;
			handle -> gyro_lsb_per_dps_dtsheet = (float)lsb_per_dps[idx];
			handle -> gyro_dps_per_lsb = (float)(1.0f / (handle -> gyro_lsb_per_dps_dtsheet));
			break;
		}

		case ACCEL: {
			uint8_t idx = (uint8_t)handle -> accel_config.accel_fsr;
			handle -> accel_lsb_per_g_dtsheet = (float)lsb_per_g[idx];
			handle -> accel_g_per_lsb = (float)(1.0f / (handle -> accel_lsb_per_g_dtsheet));
			break;
		}
		default: return;
	}
}




/*
 * =============================================================================
 * 					 		 LOW-LEVEL REGISTER ACCESS
 * =============================================================================
 */
#if ICM42688_WRITE_READ_WITH_BANKED
	static inline HAL_StatusTypeDef ICM42688_WriteBank(ICM42688_Handle_t* handle, ICM42688_Reg_t reg)
	{
		ICM42688_RegBank_t bank = ICM42688_REG_BANK(reg);
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


	HAL_StatusTypeDef ICM42688_WriteReg(ICM42688_Handle_t* handle, ICM42688_Reg_t reg, uint8_t val)
	{
		if((!handle) || (!handle -> spi_config.hspi) ||
		(!handle -> spi_config.cs_port)) return HAL_ERROR;

		HAL_StatusTypeDef status = ICM42688_WriteBank(handle, reg);
		if(status != HAL_OK) return status;

		/* Extract register address information and start writing to desired register */
		/* 1st sent is write command + register address
		 * 2nd sent is data byte */
		uint8_t regAddr = ICM42688_REG_ADDR(reg);
		uint8_t tx[2] = {
				(uint8_t)(regAddr & ICM42688_SPI_ADDR_MASK),
				val
		};

		ICM42688_CS_Low(handle);
		status = HAL_SPI_Transmit(handle -> spi_config.hspi, tx, 2, ICM42688_SPI_TIMEOUT_MS);
		ICM42688_CS_High(handle);

		return status;
	}


	HAL_StatusTypeDef ICM42688_ReadReg(ICM42688_Handle_t *handle, ICM42688_Reg_t reg, uint8_t* outVal)
	{
		if((!handle) || (!outVal) ||
		(!handle -> spi_config.hspi) ||
		(!handle -> spi_config.cs_port)) return HAL_ERROR;

		HAL_StatusTypeDef status = ICM42688_WriteBank(handle, reg);
		if(status != HAL_OK) return status;

		/* Extract register address information and start writing to desired register */
		/* 1st sent is write command + register address
		 * 2nd sent is data byte */
		uint8_t regAddr	= ICM42688_REG_ADDR(reg);
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


	HAL_StatusTypeDef ICM42688_ReadRegs(ICM42688_Handle_t* handle, ICM42688_Reg_t startRegAddr,
										uint8_t* buf, uint16_t bufLength)
	{
		if((!handle) || (!buf) || (bufLength == 0) ||
		(!handle -> spi_config.hspi) ||
		(!handle -> spi_config.cs_port)) return HAL_ERROR;

		HAL_StatusTypeDef status = ICM42688_WriteBank(handle, startRegAddr);
		if(status != HAL_OK) return status;

		/* Extract register address information and start writing to desired register */
		/* 1st sent is write command + register address
		 * 2nd sent is data byte */
		uint8_t regAddr = ICM42688_REG_ADDR(startRegAddr);
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


#else
	HAL_StatusTypeDef ICM42688_WriteReg(ICM42688_Handle_t *handle, uint8_t regAddr, uint8_t val)
	{
		/* Sanity Checks */
		if((!handle) || ((handle -> spi_config.hspi) == NULL) || ((handle -> spi_config.cs_port) == NULL)) return HAL_ERROR;

		/* 1st sent is write command + register address
		 * 2nd sent is data byte */
		uint8_t tx[2];
		tx[0] = (uint8_t)(regAddr & ICM42688_SPI_ADDR_MASK); //R/W = 0 + regAddr[6:0]
		tx[1] = (uint8_t)val;

		/* Start to send tx[1] and tx[0] via SPI */
		ICM42688_CS_Low(handle);
		HAL_StatusTypeDef status = HAL_SPI_Transmit(handle -> spi_config.hspi, tx, 2, ICM42688_SPI_TIMEOUT_MS);
		ICM42688_CS_High(handle);

		return status;
	}


	HAL_StatusTypeDef ICM42688_ReadReg(ICM42688_Handle_t* handle, uint8_t regAddr, uint8_t* outVal)
	{
		/* Sanity checks */
		if((!handle) ||
		  ((handle -> spi_config.hspi) == NULL) ||
		  ((handle -> spi_config.cs_port) == NULL) ||
		  (!outVal)) return HAL_ERROR;

		uint8_t tx[2];
		uint8_t rx[2];

		tx[0] = (uint8_t)((regAddr & ICM42688_SPI_ADDR_MASK) | (ICM42688_SPI_READ_BIT));
		tx[1] = 0x00U;	//Dummy to clock out data

		/* Start to send the address byte in read command and receive returned byte*/
		ICM42688_CS_Low(handle);
		HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(handle -> spi_config.hspi, tx, rx, 2, ICM42688_SPI_TIMEOUT_MS);
		ICM42688_CS_High(handle);

		if(status == HAL_OK) *outVal = rx[1]; //rx[0] corresponding to address phase (dummy)/undefined value

		return status;
	}


	HAL_StatusTypeDef ICM42688_ReadRegs(ICM42688_Handle_t* handle, uint8_t startRegAddr,
										uint8_t* buf, uint16_t bufLength)
	{
		if((!handle) ||
		(!handle -> spi_config.hspi) 	||
		(!handle -> spi_config.cs_port) ||
		(!buf) ||
		(bufLength == 0U)) return HAL_ERROR;

		uint8_t addr = (uint8_t)((startRegAddr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT);

		/* Start to send the address byte and read command */
		ICM42688_CS_Low(handle);
		HAL_StatusTypeDef status = HAL_SPI_Transmit(handle -> spi_config.hspi, &addr, 1, ICM42688_SPI_TIMEOUT_MS);

		/* Burst read */
		if(status == HAL_OK){
			status = HAL_SPI_Receive(handle -> spi_config.hspi, buf, bufLength, ICM42688_SPI_TIMEOUT_MS);
		}

		ICM42688_CS_High(handle);
		return status;
	}
#endif




/*
 * =============================================================================
 * 						  		 BANK MANAGEMENTS
 * =============================================================================
 */
#if !ICM42688_WRITE_READ_WITH_BANKED
	HAL_StatusTypeDef ICM42688_Set_RegBank(ICM42688_Handle_t* handle, ICM42688_RegBank_t regBank)
	{
		/* Sanity checks */
		if(!handle || (regBank > REG_BANK_4) || (regBank == REG_BANK_3)) return HAL_ERROR;

		/* Write the selected register bank to the register */
		HAL_StatusTypeDef status = ICM42688_WriteReg(handle, ICM42688_UB0_REG_BANK_SEL, (uint8_t)regBank);

		return status;
	}


	HAL_StatusTypeDef ICM42688_Get_RegBankSensor(ICM42688_Handle_t* handle, ICM42688_RegBank_t* regBank)
	{
		/* Sanity Check */
		if((!handle) || (!regBank)) return HAL_ERROR;

		uint8_t reg = 0U;
		HAL_StatusTypeDef status = ICM42688_ReadReg(handle, ICM42688_UB0_REG_BANK_SEL, &reg);
		if(status != HAL_OK) return status;

		*regBank = (ICM42688_RegBank_t)(reg & 0x07U);

		return status;
	}
#endif




/*
 * =============================================================================
 * 								IDENTITY / RESET /
 * =============================================================================
 */
static inline HAL_StatusTypeDef ICM42688_Get_WhoAmI(ICM42688_Handle_t* handle, uint8_t* who_val)
{
	if(!handle || !who_val) return HAL_ERROR;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		HAL_StatusTypeDef status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

	return ICM42688_ReadReg(handle, ICM42688_UB0_WHO_AM_I, who_val);
}


HAL_StatusTypeDef ICM42688_IsAlive(ICM42688_Handle_t* handle)
{
	/* Sanity check */
	if(!handle) return HAL_ERROR;

	/* Read and extract the returned value from WHO_AM_I register */
	uint8_t who = 0U;
	HAL_StatusTypeDef status = ICM42688_Get_WhoAmI(handle, &who);
	if(status != HAL_OK){
		handle -> is_alive = false;
		return status;
	}

	/* Check if returned value is correct by comparing it to 0x47 */
	if(who == ICM42688_WHO_AM_I_DEFAULT){
		handle -> is_alive = true;
		return HAL_OK;
	} else{
		handle -> is_alive = false;
		return HAL_ERROR;
	}
}


HAL_StatusTypeDef ICM42688_SoftReset(ICM42688_Handle_t* handle)
{
	if(!handle) return HAL_ERROR;

	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

	/* Extract the current soft-reset bit setting */
	uint8_t reg = 0U;
	status = ICM42688_ReadReg(handle, ICM42688_UB0_DEVICE_CONF, &reg);
	if(status != HAL_OK) return status;

	/* Start to enable soft-reset */
	reg |= (uint8_t)ICM42688_DEVICE_CONFIG_SOFT_RESET_Msk;
	status = ICM42688_WriteReg(handle, ICM42688_UB0_DEVICE_CONF, reg);
	if(status != HAL_OK) return status;

	/* Wait >= 1ms after reset */
	HAL_Delay(5);

	/* After reset, set every flag to a default/known state */
	handle -> gyro_config.gyro_odr			= GYRO_ODR_1KHz;
	handle -> gyro_config.gyro_fsr			= GYRO_FSR_2000dps;
	handle -> gyro_config.gyro_notch		= GYRO_NOTCHBW_680Hz;
	handle -> gyro_config.gyro_filt_order 	= GYRO_SECOND_ORDER;
	handle -> gyro_config.gyro_mode 		= GYRO_OFF;

	handle -> accel_config.accel_odr		= ACCEL_ODR_1KHz;
	handle -> accel_config.accel_fsr		= ACCEL_FSR_16g;
	handle -> accel_config.accel_filt_order	= ACCEL_SECOND_ORDER;
	handle -> accel_config.accel_mode		= ACCEL_OFF;

	handle -> int1_config.int1_polarity		= INT_ACTIVE_LOW;
	handle -> int1_config.int1_drive		= INT_OPEN_DRAIN;
	handle -> int1_config.int1_mode			= INT_PUSHED;

	handle -> int2_config.int2_polarity		= INT_ACTIVE_LOW;
	handle -> int2_config.int2_drive		= INT_OPEN_DRAIN;
	handle -> int2_config.int2_mode			= INT_PUSHED;

	handle -> gyro_dps_per_lsb			= 0.0f;
	handle -> gyro_lsb_per_dps_dtsheet	= 0.0f;

	handle -> accel_g_per_lsb 			= 0.0f;
	handle -> accel_lsb_per_g_dtsheet	= 0.0f;

	handle -> temp_config.temp_state	= TEMP_ENABLE;

	handle -> is_reset			= true;
	handle -> is_initialized 	= false;
	handle -> is_alive 			= false;

	return status;
}




/*
 * =============================================================================
 * 									SPI CONFIG
 * =============================================================================
 */
HAL_StatusTypeDef ICM42688_Set_SPI_Mode(ICM42688_Handle_t* handle, ICM42688_SPI_Mode_t spiMode)
{
    if ((!handle) || ((spiMode != SPI_MODE_0_3) && (spiMode != SPI_MODE_1_2))) return HAL_ERROR;

	HAL_StatusTypeDef status = HAL_OK;
	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

    // If already initialized and value matches cache, skip
    if (handle->is_initialized && (handle->spi_config.spi_mode == spiMode)) {
        return HAL_OK;
    }

    uint8_t reg = 0U;
    status = ICM42688_ReadReg(handle, ICM42688_UB0_DEVICE_CONF, &reg);
    if (status != HAL_OK) return status;

    reg &= (uint8_t)~ICM42688_DEVICE_CONFIG_SPI_MODE_Msk;
    reg |= (uint8_t)ICM42688_DEVICE_CONFIG_SPI_MODE_Val(spiMode);

    status = ICM42688_WriteReg(handle, ICM42688_UB0_DEVICE_CONF, reg);
    if (status == HAL_OK) {
        handle->spi_config.spi_mode = spiMode;
    }

    return status;
}


HAL_StatusTypeDef ICM42688_Get_SPI_SlewRate(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t* slewRate)
{
	if(!handle || !slewRate) return HAL_ERROR;

	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

	uint8_t reg = 0U;
	status = ICM42688_ReadReg(handle, ICM42688_UB0_DRIVE_CONF, &reg);
	if(status != HAL_OK) return status;

	reg &= ICM42688_DRIVE_CONFIG_SPI_SR_Msk;
	*slewRate = (ICM42688_SPI_SLEWRATE_t)(reg >> ICM42688_DRIVE_CONFIG_SPI_SR_Pos);

	return status;
}


HAL_StatusTypeDef ICM42688_Set_SPI_SlewRate(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t slewRate)
{
	/* Sanity Checks */
	if(!handle) return HAL_ERROR;

	uint8_t spi_slewRateMax = 5U;
	if((uint8_t)slewRate > spi_slewRateMax) return HAL_ERROR;

	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

	if((handle -> is_initialized) && ((handle -> spi_config.spi_slew_rate) == slewRate)) return HAL_OK;

	/* Force write for the first time */
	/* Extract the current setting of slew rate from the sensor */
	uint8_t reg = 0U;
	status = ICM42688_ReadReg(handle, ICM42688_UB0_DRIVE_CONF, &reg);
	if(status != HAL_OK) return status;

	/* Clear, assign bit and write bit to the sensor */
	reg &= (uint8_t)~(ICM42688_DRIVE_CONFIG_SPI_SR_Msk);
	reg |= (uint8_t)ICM42688_DRIVE_CONFIG_SPI_SR_Val(slewRate);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_DRIVE_CONF, reg);

	/* If succeed, update cached */
	if(status == HAL_OK) handle -> spi_config.spi_slew_rate = (ICM42688_SPI_SLEWRATE_t)slewRate;
	return status;
}




/*
 * =============================================================================
 * 									GYRO CONFIG
 * =============================================================================
 */
HAL_StatusTypeDef ICM42688_Set_GyroConfig(ICM42688_Handle_t* handle, ICM42688_GyroMode_t mode,
										  ICM42688_GyroODR_t odr, ICM42688_GyroFSR_t fsr)
{
	/* Sanity checks */
	if(!handle) return HAL_ERROR;

	/* Validate arguments */
	if(((uint8_t)mode > 3U) || ((uint8_t)mode == 2U)) return HAL_ERROR;

	if((((uint8_t)odr > (uint8_t)GYRO_ODR_500Hz)) ||
		((uint8_t)odr == 0x0CU) ||
		((uint8_t)odr == 0x0DU) ||
		((uint8_t)odr == 0x0EU)) return HAL_ERROR;

	if((uint8_t)fsr > (uint8_t)GYRO_FSR_15dps625) return HAL_ERROR;

	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

	/*-------------------------------------
	 * 1) PWR_MGMT0: Set Gyro Mode
	 * ------------------------------------*/
	ICM42688_GyroMode_t prevMode = handle -> gyro_config.gyro_mode;
	ICM42688_GyroMode_t currMode = mode;
	bool need_write_mode = (!(handle -> is_initialized) || (currMode != prevMode));

	{
		/* Skip if already cached & initialized */
		if(need_write_mode == true){
			/* Extract and read current setting of register PWR_MGMT0 */
			uint8_t reg = 0U;
			status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &reg);
			if(status != HAL_OK) return status;

			/* Prepare and write the new setting to PWR_MGMT0 */
			reg &= (uint8_t)~ICM42688_GYRO_MODE_Msk;
			reg |= (uint8_t)ICM42688_GYRO_MODE_Val(mode);
			status = ICM42688_WriteReg(handle, ICM42688_UB0_PWR_MGMT0, reg);
			if(status != HAL_OK) return status;

			/* Update cached */
			handle -> gyro_config.gyro_mode = mode;

			if((prevMode == GYRO_OFF) && (currMode != GYRO_OFF)){
				/* Add a >= 200us delay according to datasheet */
				HAL_Delay(1); //1000us
			}
		}
	}

	/*----------------------------------------
	 * 2) GYRO_CONF0: Set ODR + FSR together
	 * ---------------------------------------*/
	bool need_write_conf = (!(handle -> is_initialized) ||
						(odr != (handle -> gyro_config.gyro_odr)) ||
						(fsr != (handle -> gyro_config.gyro_fsr)));

	{
		if(need_write_conf == true){
			/* Extract and read current setting of register GYRO_CONFIG0 */
			uint8_t reg = 0U;
			status = ICM42688_ReadReg(handle, ICM42688_UB0_GYRO_CONF0, &reg);
			if(status != HAL_OK) return status;

			/* Prepare and write to register GYRO_CONF0 */
			reg &= (uint8_t)~(ICM42688_GYRO_ODR_Msk | ICM42688_GYRO_FS_SEL_Msk);
			reg |= (uint8_t)ICM42688_GYRO_ODR_Val(odr);
			reg |= (uint8_t)ICM42688_GYRO_FS_SEL_Val(fsr);
			status = ICM42688_WriteReg(handle, ICM42688_UB0_GYRO_CONF0, reg);
			if(status != HAL_OK) return status;

			/* Update cache + scale factor after successful HW write */
			handle -> gyro_config.gyro_odr = odr;
			handle -> gyro_config.gyro_fsr = fsr;
			ICM42688_Update_ScaleFactor(handle, GYRO);
		}
	}
	return status;
}


HAL_StatusTypeDef ICM42688_Get_Gyro_XYZ(ICM42688_Handle_t* handle, int16_t* buf)
{
	if(!handle || !buf) return HAL_ERROR;

	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return HAL_ERROR;
	#endif

	uint8_t raw[6] = {0};
	status = ICM42688_ReadRegs(handle, ICM42688_UB0_GYRO_DATA_X1, raw, 6);
	if(status != HAL_OK) return status;

	/* Extract Gyro X */
	buf[0] = (int16_t)(raw[0] << 8 | raw[1]);

	/* Extract Gyro Y */
	buf[1] = (int16_t)(raw[2] << 8 | raw[3]);

	/* Extract Gyro Z */
	buf[2] = (int16_t)(raw[4] << 8 | raw[5]);

	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Get_Gyro_DPS(ICM42688_Handle_t* handle, float dps[3])
{
	if(!handle || !dps) return HAL_ERROR;

	int16_t raw[3];
	HAL_StatusTypeDef status = ICM42688_Get_Gyro_XYZ(handle, raw);
	if(status != HAL_OK) return status;

	/* Extract gyro X, Y and Z dps */
	const float s = handle -> gyro_dps_per_lsb;
	dps[0] = (float)(raw[0] * s);
	dps[1] = (float)(raw[1] * s);
	dps[2] = (float)(raw[2] * s);

	return HAL_OK;
}



/*
 * =============================================================================
 * 								    ACCEL CONFIG
 * =============================================================================
 */
HAL_StatusTypeDef ICM42688_Set_AccelConfig(ICM42688_Handle_t* handle, ICM42688_AccelMode_t mode,
										   ICM42688_AccelODR_t odr, ICM42688_AccelFSR_t fsr)
{
	/* Sanity checks */
	if(!handle) return HAL_ERROR;

	/* Validate arguments */
	if((uint8_t)mode > 3U) return HAL_ERROR;
	if(((uint8_t)odr > (uint8_t)ACCEL_ODR_500Hz) || ((uint8_t)odr == 0x00U)) return HAL_ERROR;
	if((uint8_t)fsr > (uint8_t)ACCEL_FSR_2g) return HAL_ERROR;

	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

    /* -----------------------------------------
     * 1) PWR_MGMT0: set Accel Mode (RMW)
     * ----------------------------------------- */
	ICM42688_AccelMode_t prevMode = handle -> accel_config.accel_mode;
	ICM42688_AccelMode_t currMode = mode;
	bool need_write_mode = (!(handle -> is_initialized) || (currMode != prevMode));

	{
		if(need_write_mode){
			/* Extract the bit field of the whole register */
			uint8_t reg = 0U;
			status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &reg);
			if(status != HAL_OK) return status;

			/* Start writing */
			reg &= (uint8_t)~ICM42688_ACCEL_MODE_Msk;
			reg |= (uint8_t)ICM42688_ACCEL_MODE_Val(mode);
			status = ICM42688_WriteReg(handle, ICM42688_UB0_PWR_MGMT0, reg);
			if(status != HAL_OK) return status;

			/* Update cache */
			handle -> accel_config.accel_mode = mode;

			/* Wait 200us if mode changes from OFF to any other mode */
			if((prevMode == ACCEL_OFF) && (currMode != ACCEL_OFF)){
				HAL_Delay(1); //1000us > 200us (according to datasheet)
			}
		}
	}

	/*
	 * ----------------------------------------------
	 * 2) ACCEL_CONF0: set ODR and FSR together
	 * ----------------------------------------------*/
	bool need_write_config = (!(handle -> is_initialized) ||
							(odr != handle -> accel_config.accel_odr) ||
							(fsr != handle -> accel_config.accel_fsr));
	{
		if(need_write_config){
			uint8_t reg = 0U;
			status = ICM42688_ReadReg(handle, ICM42688_UB0_ACCEL_CONF0, &reg);
			if(status != HAL_OK) return status;

			/* Start writing */
			reg &= (uint8_t)~(ICM42688_ACCEL_ODR_Msk | ICM42688_ACCEL_FS_SEL_Msk);
			reg |= (uint8_t)ICM42688_ACCEL_ODR_Val(odr);
			reg |= (uint8_t)ICM42688_ACCEL_FS_SEL_Val(fsr);
			status = ICM42688_WriteReg(handle, ICM42688_UB0_ACCEL_CONF0, reg);
			if(status != HAL_OK) return status;

			/* Update cache	and scale factor */
			handle -> accel_config.accel_odr = odr;
			handle -> accel_config.accel_fsr = fsr;
			ICM42688_Update_ScaleFactor(handle, ACCEL);
		}
	}
	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Get_Accel_XYZ(ICM42688_Handle_t* handle, int16_t* buf){
	if(!handle || !buf) return HAL_ERROR;
	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return HAL_ERROR;
	#endif

	uint8_t raw[6] = {0};
	status = ICM42688_ReadRegs(handle, ICM42688_UB0_ACCEL_DATA_X1, raw, 6);
	if(status != HAL_OK) return HAL_ERROR;

	/* Extract Accel X */
	buf[0] = (int16_t)(raw[0] << 8 | raw[1]);

	/* Extract Accel Y */
	buf[1] = (int16_t)(raw[2] << 8 | raw[3]);

	/* Extract Accel Z */
	buf[2] = (int16_t)(raw[4] << 8 | raw[5]);

	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Get_Accel_G(ICM42688_Handle_t* handle, float g[3]){
	if(!handle || !g) return HAL_ERROR;

	int16_t raw[3];
	HAL_StatusTypeDef status = ICM42688_Get_Accel_XYZ(handle, raw);
	if(status != HAL_OK) return status;

	const float s = handle -> accel_g_per_lsb;
	g[0] = (float)(raw[0] * s);
	g[1] = (float)(raw[1] * s);
	g[2] = (float)(raw[2] * s);

	return HAL_OK;
}




/*
 * ==================================================================================
 * 									INTERRUPT CONFIG
 * ==================================================================================
 */
HAL_StatusTypeDef ICM42688_Set_Int1_Config(ICM42688_Handle_t* handle, ICM42688_Int_Polarity_t polarity,
										   ICM42688_Int_Drive_Circuit_t drive, ICM42688_Int_Mode_t mode)
{
	/* Sanity chekcs */
	if((!handle) ||
	   ((uint8_t)polarity >= INT_POL_MAX) ||
	   ((uint8_t)drive >= INT_DRIVE_MAX) ||
	   ((uint8_t)mode >= INT_MODE_MAX)) return HAL_ERROR;

	/* Check if Interrupt 1 is already configured */
	if((handle -> is_initialized) &&
	  ((uint8_t)polarity == (uint8_t)(handle -> int1_config.int1_polarity)) &&
	  ((uint8_t)drive) == ((uint8_t)(handle -> int1_config.int1_drive)) &&
	  ((uint8_t)mode) == ((uint8_t)(handle -> int1_config.int1_mode))){
		return HAL_OK;
	}

	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

	/* Start extracting the current bit field of INT_CONFIG */
	uint8_t reg = 0U;
	status = ICM42688_ReadReg(handle, ICM42688_UB0_INT_CONF, &reg);
	if(status != HAL_OK) return status;

	/* Start to write the target configuration to the register */
	reg &= (uint8_t)~(ICM42688_INT1_POL_Msk | ICM42688_INT1_DRIVE_Msk | ICM42688_INT1_MODE_Msk);
	reg |= (uint8_t)(ICM42688_INT1_POL_Val(polarity) |
					 ICM42688_INT1_DRIVE_Val(drive) |
					 ICM42688_INT1_MODE_Val(mode));
	status = ICM42688_WriteReg(handle, ICM42688_UB0_INT_CONF, reg);
	if(status != HAL_OK) return status;

	/* Update cache */
	handle -> int1_config.int1_polarity = polarity;
	handle -> int1_config.int1_drive = drive;
	handle -> int1_config.int1_mode = mode;

	return HAL_OK;
}


HAL_StatusTypeDef ICM42688_Set_Int2_Config(ICM42688_Handle_t* handle, ICM42688_Int_Polarity_t polarity,
										   ICM42688_Int_Drive_Circuit_t drive, ICM42688_Int_Mode_t mode)
{
	/* Sanity chekcs */
	if((!handle) ||
	   ((uint8_t)polarity >= INT_POL_MAX) ||
	   ((uint8_t)drive >= INT_DRIVE_MAX) ||
	   ((uint8_t)mode >= INT_MODE_MAX)) return HAL_ERROR;

	/* Check if Interrupt 2 is already configured */
	if((handle -> is_initialized) &&
	  ((uint8_t)polarity == (uint8_t)(handle -> int2_config.int2_polarity)) &&
	  ((uint8_t)drive) == ((uint8_t)(handle -> int2_config.int2_drive)) &&
	  ((uint8_t)mode) == ((uint8_t)(handle -> int2_config.int2_mode))){
		return HAL_OK;
	}

	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

	/* Start extracting the current bit field of INT_CONFIG */
	uint8_t reg = 0U;
	status = ICM42688_ReadReg(handle, ICM42688_UB0_INT_CONF, &reg);
	if(status != HAL_OK) return status;

	/* Start to write the target configuration to the register */
	reg &= (uint8_t)~(ICM42688_INT2_POL_Msk | ICM42688_INT2_DRIVE_Msk | ICM42688_INT2_MODE_Msk);
	reg |= (uint8_t)(ICM42688_INT2_POL_Val(polarity) |
					 ICM42688_INT2_DRIVE_Val(drive) |
					 ICM42688_INT2_MODE_Val(mode));
	status = ICM42688_WriteReg(handle, ICM42688_UB0_INT_CONF, reg);
	if(status != HAL_OK) return status;

	/* Update cache */
	handle -> int2_config.int2_polarity = polarity;
	handle -> int2_config.int2_drive = drive;
	handle -> int2_config.int2_mode = mode;

	return HAL_OK;
}




/*
 * ==================================================================================
 * 									TEMPERATURE CONFIG
 * ==================================================================================
 */
HAL_StatusTypeDef ICM42688_Set_Temperature_Enable(ICM42688_Handle_t* handle, ICM42688_Temp_t state)
{
	if(!handle) return HAL_ERROR;

	if(handle -> is_initialized && handle -> temp_config.temp_state == state){
		//Skip
	} else{
		HAL_StatusTypeDef status = HAL_OK;
		#if !ICM42688_WRITE_READ_WITH_BANKED
			status = ICM42688_Set_RegBank(handle, REG_BANK_0);
			if(status != HAL_OK) return status;
		#endif

		uint8_t reg = 0;
		status = ICM42688_ReadReg(handle, ICM42688_UB0_PWR_MGMT0, &reg);
		if(status != HAL_OK) return status;

		reg &= (uint8_t)~(ICM42688_TEMP_Msk);
		reg |= (uint8_t)ICM42688_TEMP_Val(state);
		status = ICM42688_WriteReg(handle, ICM42688_UB0_PWR_MGMT0, reg);
		if(status != HAL_OK) return status;

		/* Update cache */
		handle -> temp_config.temp_state = state;
	}
	return HAL_OK;
}


int16_t ICM42688_Get_Temperature_Raw(ICM42688_Handle_t* handle)
{
	if(!handle) return ICM42688_RAW_INVALID;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		HAL_StatusTypeDef status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return ICM42688_RAW_INVALID;
	#endif

	uint8_t buf[2] = {0};
	HAL_StatusTypeDef status = ICM42688_ReadRegs(handle, ICM42688_UB0_TEMP_DATA1, buf, 2);
	if(status != HAL_OK) return ICM42688_RAW_INVALID;

	return (int16_t)((uint16_t)(buf[0] << 8) | (uint16_t)buf[1]);
}


float ICM42688_Get_Temperature_C(ICM42688_Handle_t* handle)
{
	if(!handle) return NAN;

	int16_t raw = ICM42688_Get_Temperature_Raw(handle);
	if(raw == ICM42688_RAW_INVALID) return NAN;

	return (float)((raw / 132.48f) + 25.0f);
}




/*
 * ==================================================================================
 * 								GYRO FILTERING CONFIG
 * ==================================================================================
 */
HAL_StatusTypeDef ICM42688_Set_Gyro_UIFilt_BW(ICM42688_Handle_t* handle, ICM42688_UIFilt_BW_t bw){
	if(!handle) return HAL_ERROR;
	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

	uint8_t reg = 0U;
	status = ICM42688_ReadReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, &reg);
	if(status != HAL_OK) return status;

	reg &= (uint8_t)~ICM42688_GYRO_UI_FILT_BW_Msk;
	reg |= (uint8_t)~ICM42688_GYRO_UI_FILT_BW_Val(bw);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, reg);
	if(status != HAL_OK) return status;

	/* Update cache */
	handle -> gyro_config.gyro_uifilt_bw = bw;

	return HAL_OK;
}




/*
 * ==================================================================================
 * 								ACCEL FILTERING CONFIG
 * ==================================================================================
 */
HAL_StatusTypeDef ICM42688_Set_Accel_UIFilt_BW(ICM42688_Handle_t* handle, ICM42688_UIFilt_BW_t bw){
	if(!handle) return HAL_ERROR;
	HAL_StatusTypeDef status = HAL_OK;

	#if !ICM42688_WRITE_READ_WITH_BANKED
		status = ICM42688_Set_RegBank(handle, REG_BANK_0);
		if(status != HAL_OK) return status;
	#endif

	uint8_t reg = 0U;
	status = ICM42688_ReadReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, &reg);
	if(status != HAL_OK) return status;

	reg &= (uint8_t)~ICM42688_ACCEL_UI_FILT_BW_Msk;
	reg |= (uint8_t)~ICM42688_ACCEL_UI_FILT_BW_Val(bw);
	status = ICM42688_WriteReg(handle, ICM42688_UB0_GYRO_ACCEL_CONF0, reg);
	if(status != HAL_OK) return status;

	/* Update cache */
	handle -> accel_config.accel_uifilt_bw = bw;

	return HAL_OK;
}

