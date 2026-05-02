/*
 * icm42688_rw.c
 *
 *  Created on: Mar 5, 2026
 *      Author: dobao
 */
#include "imu/core/icm42688_rw.h"

/* ============================================================================
 *	PRIVATE HELPERS
 * ============================================================================ */
static inline void
ICM42688_CS_Low(ICM42688_Handle_t *handle)
{
    HAL_GPIO_WritePin(handle->spi_config.cs_port, handle->spi_config.cs_pin,
                      GPIO_PIN_RESET); // Pull CS low to start an SPI transaction
}



static inline void
ICM42688_CS_High(ICM42688_Handle_t *handle)
{
    HAL_GPIO_WritePin(handle->spi_config.cs_port, handle->spi_config.cs_pin,
                      GPIO_PIN_SET); // Pull CS high to end an SPI transaction
}



/* ============================================================================
 *	LOW-LEVEL REGISTER ACCESS
 * ============================================================================ */
/**
 * @brief   Automatically write bank number to the corresponding input encoded register
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   encodedReg  Encoded register containing both register adress and corresponding bank
 *                      number
 */
ICM42688_Status_t
ICM42688_WriteBankAuto(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg)
{
    ICM42688_RegBank_t _bank = ICM42688_REG_BANK(encodedReg);
    if ((_bank > REG_BANK_4) || (_bank == REG_BANK_3))
        return ICM42688_ERROR;

    uint8_t _bank_sel_addr = ICM42688_REG_ADDR(ICM42688_UB0_REG_BANK_SEL);
    uint8_t _bank_tx[2]    = {/* 1st sent is register address + write command bit
                               * 2nd sent is data byte */
                           (uint8_t)(_bank_sel_addr & ICM42688_SPI_ADDR_MASK), (uint8_t)(_bank)};

    ICM42688_CS_Low(handle);

    HAL_StatusTypeDef _status =
        HAL_SPI_Transmit(handle->spi_config.hspi, _bank_tx, 2, ICM42688_SPI_TIMEOUT_MS);

    ICM42688_CS_High(handle);

    if (_status != HAL_OK)
        return ICM42688_ERROR;

    return ICM42688_OK;
}



/**
 * @brief   Write one byte to the target ICM42688 register over SPI
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   encodedReg  Encoded register containing both register address and corresponding bank
 *                      number
 * @param   val         Data byte to be written into the target register
 */
ICM42688_Status_t
ICM42688_WriteReg(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t val)
{
    if ((!handle) || (!handle->spi_config.hspi) || (!handle->spi_config.cs_port))
        return ICM42688_ERROR;

    HAL_StatusTypeDef _status = ICM42688_WriteBankAuto(handle, encodedReg);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    // Extract register address information and start writing to desired register
    /* 1st sent is write command + register address
     * 2nd sent is data byte */
    uint8_t _reg_addr = ICM42688_REG_ADDR(encodedReg);
    uint8_t _tx[2]    = {(uint8_t)(_reg_addr & ICM42688_SPI_ADDR_MASK), val};

    ICM42688_CS_Low(handle);

    _status = HAL_SPI_Transmit(handle->spi_config.hspi, _tx, 2, ICM42688_SPI_TIMEOUT_MS);

    ICM42688_CS_High(handle);

    if (_status != HAL_OK)
        return ICM42688_ERROR;

    return ICM42688_OK;
}



/**
 * @brief   Read one byte from ICM42688 register over SPI
 * @param   encodedReg      Encoded register containing register address and corresponding bank
 *                          number
 * @param   outVal          Pointer to a variable that stores the read register value
 */
ICM42688_Status_t
ICM42688_ReadReg(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t *outVal)
{
    if ((!handle) || (!outVal) || (!handle->spi_config.hspi) || (!handle->spi_config.cs_port))
        return ICM42688_ERROR;

    HAL_StatusTypeDef _status = ICM42688_WriteBankAuto(handle, encodedReg);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    // Extract register address information and start reading the desired register
    /* 1st sent is write command + register address
     * 2nd sent is data byte */
    uint8_t _reg_addr = ICM42688_REG_ADDR(encodedReg);
    uint8_t _tx[2]    = {
        (uint8_t)((_reg_addr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT),
        0x00U // Dummy to clock out data
    };
    uint8_t _rx[2] = {0};

    ICM42688_CS_Low(handle);

    _status =
        HAL_SPI_TransmitReceive(handle->spi_config.hspi, _tx, _rx, 2, ICM42688_SPI_TIMEOUT_MS);

    ICM42688_CS_High(handle);

    if (_status != HAL_OK)
        return ICM42688_ERROR;

    *outVal = _rx[1]; // rx[0] corresponding to address phase (dummy)/undefined value

    return ICM42688_OK;
}



/**
 * @brief   Read multiple consecutive registers from ICM42688 over SPI
 * @param   handle              Pointer to ICM42688 Handle struct
 * @param   startEncodedReg     Encoded start register
 * @param   buf                 Pointer to the buffer that stores the received bytes
 * @param   bufLength           Number of consecutuve bytes to read
 */
ICM42688_Status_t
ICM42688_ReadRegs(ICM42688_Handle_t *handle, ICM42688_Reg_t startEncodedReg, uint8_t *buf,
                  uint16_t bufLength)
{
    if ((!handle) || (!buf) || (bufLength == 0))
        return ICM42688_ERROR;

    if (!handle->spi_config.hspi || !handle->spi_config.cs_port)
        return ICM42688_ERROR;

    HAL_StatusTypeDef _status = ICM42688_WriteBankAuto(handle, startEncodedReg);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    // Maximum FIFO byte is 2KB, so rejecting the total length > 0x80 is wrong for FIFO stream
    bool _is_fifo_stream = (startEncodedReg == ICM42688_UB0_FIFO_DATA);

    // Extract register address information and start reading to desired register
    /* 1st sent is write command + register address
     * 2nd sent is data byte */
    uint8_t _reg_addr = ICM42688_REG_ADDR(startEncodedReg);
    if (!_is_fifo_stream && (((uint16_t)_reg_addr + bufLength) > 0x80U))
        return ICM42688_ERROR;

    /* Send read command + start addr, then burst receive */
    uint8_t _addr = (uint8_t)((_reg_addr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT);

    ICM42688_CS_Low(handle);

    _status = HAL_SPI_Transmit(handle->spi_config.hspi, &_addr, 1, ICM42688_SPI_TIMEOUT_MS);
    if (_status == HAL_OK) {
        _status = HAL_SPI_Receive(handle->spi_config.hspi, buf, bufLength, ICM42688_SPI_TIMEOUT_MS);
    }

    ICM42688_CS_High(handle);

    if (_status != HAL_OK)
        return ICM42688_ERROR;

    return ICM42688_OK;
}



/**
 * @brief   Updates selected bit fields of a target register using read-modify-write operation
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   encodedReg  Encoded register value
 * @param   mask        Bit mask indicates which register bits will be updated
 * @param   valueMasked New field value already shifted and masked to match the mask
 */
ICM42688_Status_t
ICM42688_Update_Reg_Bits(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t mask,
                         uint8_t valueMasked)
{
    if (!handle)
        return ICM42688_ERROR;

    if ((valueMasked & (uint8_t)~mask) != 0U)
        return ICM42688_ERROR;

    uint8_t           _current_reg = 0U;
    HAL_StatusTypeDef _status      = ICM42688_ReadReg(handle, encodedReg, &_current_reg);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    _current_reg = (uint8_t)((_current_reg & (uint8_t)~mask) | valueMasked);
    _status      = ICM42688_WriteReg(handle, encodedReg, _current_reg);
    if (_status != HAL_OK)
        return ICM42688_ERROR;

    return ICM42688_OK;
}
