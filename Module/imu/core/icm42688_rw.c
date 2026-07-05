/*
 * icm42688_rw.c
 *
 *  Created on: Mar 5, 2026
 *      Author: dobao
 */
#include "imu/core/icm42688_rw.h"

static inline uint8_t
ICM42688_SPI_Read_Command(ICM42688_Reg_t encodedReg)
{
    return (uint8_t)(ICM42688_REG_ADDR(encodedReg) & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT;
}

static inline uint8_t
ICM42688_SPI_Write_Command(ICM42688_Reg_t encodedReg)
{
    return (uint8_t)(ICM42688_REG_ADDR(encodedReg) & ICM42688_SPI_ADDR_MASK);
}


/* ============================================================================
 *	LOW-LEVEL REGISTER ACCESS
 * ============================================================================ */
void
ICM42688_CS_Pull_Low(GPIO_TypeDef *csPort, uint16_t csPin)
{
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET); // Pull CS low to start the SPI transaction
}


void
ICM42688_CS_Pull_High(GPIO_TypeDef *csPort, uint16_t csPin)
{
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET); // Pull CS high to end the SPI transaction
}



/**
 * @brief   Select the register bank encoded in @p encodedReg before accessing the register
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   encodedReg  Encoded register containing both register address and corresponding bank
 *                      number
 */
static bool
ICM42688_WriteBankAuto(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg)
{
    ICM42688_RegBank_t _bank = ICM42688_REG_BANK(encodedReg);
    if ((_bank > REG_BANK_4) || (_bank == REG_BANK_3))
        return false;

    uint8_t _bank_sel_addr = ICM42688_REG_ADDR(ICM42688_UB0_REG_BANK_SEL);
    uint8_t _bank_tx[2]    = {/* First byte: REG_BANK_SEL address with SPI write bit cleared
                               * Second byte: target bank number */
                           (uint8_t)(_bank_sel_addr & ICM42688_SPI_ADDR_MASK), (uint8_t)(_bank)};

    ICM42688_CS_Pull_Low(handle->spi_config.cs_port, handle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_Transmit(handle->spi_config.hspi, _bank_tx, 2, ICM42688_SPI_TIMEOUT_MS);

    ICM42688_CS_Pull_High(handle->spi_config.cs_port, handle->spi_config.cs_pin);

    if (_status != HAL_OK)
        return false;

    return true;
}



/**
 * @brief   Write one byte to the target ICM42688 register over SPI
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   encodedReg  Encoded register containing both register address and corresponding bank
 *                      number
 * @param   val         Data byte to be written into the target register
 */
bool
ICM42688_WriteReg(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t val)
{
    if ((!handle) || (!handle->spi_config.hspi) || (!handle->spi_config.cs_port))
        return false;

    if (!ICM42688_WriteBankAuto(handle, encodedReg))
        return false;

    /* First byte: register address with SPI write bit cleared
     * Second byte: data byte */
    uint8_t _reg_addr = ICM42688_REG_ADDR(encodedReg);
    uint8_t _tx[2]    = {(uint8_t)(_reg_addr & ICM42688_SPI_ADDR_MASK), val};

    ICM42688_CS_Pull_Low(handle->spi_config.cs_port, handle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_Transmit(handle->spi_config.hspi, _tx, 2, ICM42688_SPI_TIMEOUT_MS);

    ICM42688_CS_Pull_High(handle->spi_config.cs_port, handle->spi_config.cs_pin);

    if (_status != HAL_OK)
        return false;

    return true;
}



/**
 * @brief   Read one byte from ICM42688 register over SPI
 * @param   handle          Pointer to ICM42688 Handle struct
 * @param   encodedReg      Encoded register containing register address and corresponding bank
 *                          number
 * @param   outVal          Pointer to a variable that stores the read register value
 */
bool
ICM42688_ReadReg(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t *outVal)
{
    if ((!handle) || (!outVal) || (!handle->spi_config.hspi) || (!handle->spi_config.cs_port))
        return false;

    if (!ICM42688_WriteBankAuto(handle, encodedReg))
        return false;

    /* First byte: register address with SPI read bit set
     * Second byte: dummy byte used to clock out the register data */
    uint8_t _reg_addr = ICM42688_REG_ADDR(encodedReg);
    uint8_t _tx[2]    = {
        (uint8_t)((_reg_addr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT),
        0x00U // Dummy to clock out data
    };
    uint8_t _rx[2] = {0};

    ICM42688_CS_Pull_Low(handle->spi_config.cs_port, handle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_TransmitReceive(handle->spi_config.hspi, _tx, _rx, 2, ICM42688_SPI_TIMEOUT_MS);

    ICM42688_CS_Pull_High(handle->spi_config.cs_port, handle->spi_config.cs_pin);

    if (_status != HAL_OK)
        return false;

    *outVal = _rx[1]; // rx[0] is captured during the address phase and is undefined

    return true;
}



/**
 * @brief   Read multiple consecutive registers from ICM42688 over SPI
 * @param   handle              Pointer to ICM42688 Handle struct
 * @param   startEncodedReg     Encoded start register
 * @param   buf                 Pointer to the buffer that stores the received bytes
 * @param   bufLength           Number of consecutive bytes to read
 */
bool
ICM42688_ReadRegs(ICM42688_Handle_t *handle, ICM42688_Reg_t startEncodedReg, uint8_t *buf, uint16_t bufLength)
{
    if ((!handle) || (!buf) || (bufLength == 0))
        return false;

    if (!handle->spi_config.hspi || !handle->spi_config.cs_port)
        return false;

    if (!ICM42688_WriteBankAuto(handle, startEncodedReg))
        return false;

    // FIFO_DATA can burst up to the 2 KB FIFO depth, so the normal register-bank boundary check is skipped for it.
    bool _is_fifo_stream = (startEncodedReg == ICM42688_UB0_FIFO_DATA);

    // Non-FIFO burst reads must stay inside the selected 128-byte register bank.
    uint8_t _reg_addr = ICM42688_REG_ADDR(startEncodedReg);
    if (!_is_fifo_stream && (((uint16_t)_reg_addr + bufLength) > 0x80U))
        return false;

    /* Send read command + start address, then burst receive. */
    uint8_t _addr = (uint8_t)((_reg_addr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT);

    ICM42688_CS_Pull_Low(handle->spi_config.cs_port, handle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_Transmit(handle->spi_config.hspi, &_addr, 1, ICM42688_SPI_TIMEOUT_MS);
    if (_status == HAL_OK) {
        _status = HAL_SPI_Receive(handle->spi_config.hspi, buf, bufLength, ICM42688_SPI_TIMEOUT_MS);
    }

    ICM42688_CS_Pull_High(handle->spi_config.cs_port, handle->spi_config.cs_pin);

    if (_status != HAL_OK)
        return false;

    return true;
}



/**
 * @brief   Updates selected bit fields of a target register using read-modify-write operation
 * @param   handle      Pointer to ICM42688 Handle struct
 * @param   encodedReg  Encoded register value
 * @param   mask        Bit mask indicating which register bits will be updated
 * @param   valueMasked New field value already shifted and masked to match the mask
 */
bool
ICM42688_Update_Reg_Bits(ICM42688_Handle_t *handle, ICM42688_Reg_t encodedReg, uint8_t mask, uint8_t valueMasked)
{
    if (!handle)
        return false;

    if ((valueMasked & (uint8_t)~mask) != 0U)
        return false;

    uint8_t _current_reg = 0U;
    if (!ICM42688_ReadReg(handle, encodedReg, &_current_reg))
        return false;

    _current_reg = (uint8_t)((_current_reg & (uint8_t)~mask) | valueMasked);
    if (!ICM42688_WriteReg(handle, encodedReg, _current_reg))
        return false;

    return true;
}
