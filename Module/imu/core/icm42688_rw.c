/*
 * icm42688_rw.c
 *
 *  Created on: Mar 5, 2026
 *      Author: dobao
 */
#include "imu/core/icm42688_rw.h"

#include <string.h>

/* ============================================================================
 *  PRIVATE APIs - LOW-LEVEL REGISTER ACCESS
 * ============================================================================ */

/**
 * @brief   Build the command byte for an SPI register read.
 * @param   encodedReg Register encoded with bank and address.
 * @return  Register address with the ICM42688 SPI read bit set.
 */
static inline uint8_t
ICM42688_SPI_Read_Command(ICM42688_Reg_t encodedReg)
{
    return (uint8_t)((ICM42688_REG_ADDR(encodedReg) & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT);
}



/**
 * @brief   Build the command byte for an SPI register write.
 * @param   encodedReg Register encoded with bank and address.
 * @return  Register address with the ICM42688 SPI read bit cleared.
 */
static inline uint8_t
ICM42688_SPI_Write_Command(ICM42688_Reg_t encodedReg)
{
    return (uint8_t)(ICM42688_REG_ADDR(encodedReg) & ICM42688_SPI_ADDR_MASK);
}



/**
 * @brief   Select the register bank encoded in @p encodedReg before register access.
 * @param   pHandle     Pointer to ICM42688 handle.
 * @param   encodedReg  Encoded register containing the target bank.
 * @return  true if the bank is valid and REG_BANK_SEL is written successfully, otherwise false.
 * @note    REG_BANK_3 is rejected because this driver has no bank-3 register definitions.
 */
static bool
ICM42688_WriteBankAuto(ICM42688_Handle_t *pHandle, ICM42688_Reg_t encodedReg)
{
    ICM42688_RegBank_t _bank = ICM42688_REG_BANK(encodedReg);
    if ((_bank > REG_BANK_4) || (_bank == REG_BANK_3))
        return false;

    uint8_t _bank_sel_addr = ICM42688_REG_ADDR(ICM42688_UB0_REG_BANK_SEL);
    uint8_t _bank_tx[2]    = {/* First byte: REG_BANK_SEL address with SPI write bit cleared
                               * Second byte: target bank number */
                           (uint8_t)(_bank_sel_addr & ICM42688_SPI_ADDR_MASK), (uint8_t)(_bank)};

    ICM42688_CS_Pull_Low(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_Transmit(pHandle->spi_config.hspi, _bank_tx, 2, ICM42688_SPI_TIMEOUT_MS);

    ICM42688_CS_Pull_High(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    if (_status != HAL_OK)
        return false;

    return true;
}



/* ============================================================================
 *  PUBLIC APIs - LOW-LEVEL REGISTER ACCESS
 * ============================================================================ */

/**
 * @brief   Pull CS low to start an SPI transaction.
 * @param   pCsPort GPIO port connected to the ICM42688 CS signal.
 * @param   csPin   GPIO pin mask for the CS signal.
 */
void
ICM42688_CS_Pull_Low(GPIO_TypeDef *pCsPort, uint16_t csPin)
{
    HAL_GPIO_WritePin(pCsPort, csPin, GPIO_PIN_RESET);
}



/**
 * @brief   Pull CS high to end an SPI transaction.
 * @param   pCsPort GPIO port connected to the ICM42688 CS signal.
 * @param   csPin   GPIO pin mask for the CS signal.
 */
void
ICM42688_CS_Pull_High(GPIO_TypeDef *pCsPort, uint16_t csPin)
{
    HAL_GPIO_WritePin(pCsPort, csPin, GPIO_PIN_SET);
}



/**
 * @brief   Write one byte to an encoded ICM42688 register over SPI.
 * @param   pHandle     Pointer to ICM42688 handle.
 * @param   encodedReg  Register encoded with bank and address.
 * @param   data        Data byte to write.
 * @return  true if bank selection and SPI transfer succeed, otherwise false.
 */
bool
ICM42688_WriteReg(ICM42688_Handle_t *pHandle, ICM42688_Reg_t encodedReg, uint8_t data)
{
    if ((!pHandle) || (!pHandle->spi_config.hspi) || (!pHandle->spi_config.cs_port))
        return false;

    if (!ICM42688_WriteBankAuto(pHandle, encodedReg))
        return false;

    /* First byte: register address with SPI write bit cleared
     * Second byte: data byte */
    uint8_t _reg_addr = ICM42688_REG_ADDR(encodedReg);
    uint8_t _tx[2]    = {(uint8_t)(_reg_addr & ICM42688_SPI_ADDR_MASK), data};

    ICM42688_CS_Pull_Low(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_Transmit(pHandle->spi_config.hspi, _tx, 2, ICM42688_SPI_TIMEOUT_MS);

    ICM42688_CS_Pull_High(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    if (_status != HAL_OK)
        return false;

    return true;
}



/**
 * @brief   Read one byte from an encoded ICM42688 register over SPI.
 * @param   pHandle     Pointer to ICM42688 handle.
 * @param   encodedReg  Register encoded with bank and address.
 * @param   pOutData    Destination for the received byte.
 * @return  true if bank selection and SPI transfer succeed, otherwise false.
 */
bool
ICM42688_ReadReg(ICM42688_Handle_t *pHandle, ICM42688_Reg_t encodedReg, uint8_t *pOutData)
{
    if ((!pHandle) || (!pOutData) || (!pHandle->spi_config.hspi) || (!pHandle->spi_config.cs_port))
        return false;

    if (!ICM42688_WriteBankAuto(pHandle, encodedReg))
        return false;

    /* First byte: register address with SPI read bit set
     * Second byte: dummy byte used to clock out the register data */
    uint8_t _reg_addr = ICM42688_REG_ADDR(encodedReg);
    uint8_t _tx[2]    = {
        (uint8_t)((_reg_addr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT),
        0x00U // Dummy to clock out data
    };
    uint8_t _rx[2] = {0};

    ICM42688_CS_Pull_Low(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_TransmitReceive(pHandle->spi_config.hspi, _tx, _rx, 2, ICM42688_SPI_TIMEOUT_MS);

    ICM42688_CS_Pull_High(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    if (_status != HAL_OK)
        return false;

    *pOutData = _rx[1]; // rx[0] is captured during the address phase and is undefined

    return true;
}



/**
 * @brief   Read consecutive ICM42688 registers, or stream bytes from FIFO_DATA.
 * @param   pHandle          Pointer to ICM42688 handle.
 * @param   startEncodedReg  First encoded register to read.
 * @param   pBuf             Destination buffer for payload bytes.
 * @param   bufLength        Number of payload bytes to read.
 * @return  true if the read succeeds, otherwise false.
 */
bool
ICM42688_ReadRegs(ICM42688_Handle_t *pHandle, ICM42688_Reg_t startEncodedReg, uint8_t *pBuf, uint16_t bufLength)
{
    if ((!pHandle) || (!pBuf) || (bufLength == 0))
        return false;

    if (!pHandle->spi_config.hspi || !pHandle->spi_config.cs_port)
        return false;

    if (!ICM42688_WriteBankAuto(pHandle, startEncodedReg))
        return false;

    // FIFO_DATA is a stream port, so the normal register-bank boundary check is skipped for it.
    bool _is_fifo_stream = (startEncodedReg == ICM42688_UB0_FIFO_DATA);

    // Non-FIFO burst reads must stay inside the selected 128-byte register bank.
    uint8_t _reg_addr = ICM42688_REG_ADDR(startEncodedReg);
    if (!_is_fifo_stream && (((uint16_t)_reg_addr + bufLength) > 0x80U))
        return false;

    // Send the read command byte, then keep CS low while clocking out the payload.
    uint8_t _addr = (uint8_t)((_reg_addr & ICM42688_SPI_ADDR_MASK) | ICM42688_SPI_READ_BIT);

    ICM42688_CS_Pull_Low(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_Transmit(pHandle->spi_config.hspi, &_addr, 1, ICM42688_SPI_TIMEOUT_MS);
    if (_status == HAL_OK) {
        _status = HAL_SPI_Receive(pHandle->spi_config.hspi, pBuf, bufLength, ICM42688_SPI_TIMEOUT_MS);
    }

    ICM42688_CS_Pull_High(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    if (_status != HAL_OK)
        return false;

    return true;
}



/**
 * @brief   Update selected bits of one register with a read-modify-write operation.
 * @param   pHandle      Pointer to ICM42688 handle.
 * @param   encodedReg   Register encoded with bank and address.
 * @param   mask         Bit mask selecting fields to update.
 * @param   valueMasked  New field value, already shifted and masked.
 * @return  true if the read and write both succeed, otherwise false.
 */
bool
ICM42688_Update_Reg_Bits(ICM42688_Handle_t *pHandle, ICM42688_Reg_t encodedReg, uint8_t mask, uint8_t valueMasked)
{
    if (!pHandle)
        return false;

    if ((valueMasked & (uint8_t)~mask) != 0U)
        return false;

    uint8_t _current_reg = 0U;
    if (!ICM42688_ReadReg(pHandle, encodedReg, &_current_reg))
        return false;

    _current_reg = (uint8_t)((_current_reg & (uint8_t)~mask) | valueMasked);
    if (!ICM42688_WriteReg(pHandle, encodedReg, _current_reg))
        return false;

    return true;
}



/**
 * @brief   Start an SPI DMA burst read from consecutive registers or FIFO_DATA.
 * @param   pHandle          Pointer to ICM42688 handle.
 * @param   startEncodedReg  First encoded register to read.
 * @param   pTxBuf           DMA TX scratch buffer. Byte 0 becomes the read command.
 * @param   txBufLength      Size of pTxBuf in bytes.
 * @param   pRxBuf           DMA RX buffer. Byte 0 is dummy; bytes 1..dataLength are payload.
 * @param   rxBufLength      Size of pRxBuf in bytes.
 * @param   dataLength       Number of payload bytes to read.
 * @param   autoBankSelect   Select the encoded bank before DMA, or false if caller preselected it.
 * @return  true if the DMA transfer starts, otherwise false.
 * @warning CS remains low until the DMA completion path calls ICM42688_DMA_End().
 */
bool
ICM42688_ReadRegs_DMA_Start(ICM42688_Handle_t *pHandle, ICM42688_Reg_t startEncodedReg, uint8_t *pTxBuf,
                            uint16_t txBufLength, uint8_t *pRxBuf, uint16_t rxBufLength, uint16_t dataLength,
                            bool autoBankSelect)
{
    if (!pHandle || !pTxBuf || !pRxBuf)
        return false;

    if ((dataLength == 0U) || (txBufLength == 0U) || (rxBufLength == 0U))
        return false;

    if (dataLength == UINT16_MAX)
        return false;

    if (!pHandle->spi_config.hspi || !pHandle->spi_config.cs_port)
        return false;

    // FIFO_DATA is a stream port; all other burst reads must stay inside one bank.
    bool    _is_fifo_data_reg = (startEncodedReg == ICM42688_UB0_FIFO_DATA);
    uint8_t _start_addr       = ICM42688_REG_ADDR(startEncodedReg);
    if (!_is_fifo_data_reg && (((uint16_t)_start_addr + dataLength) > ICM42688_REG_ADDR_LIMIT)) {
        return false;
    }

    /**
     * TX[0] = read command
     * TX[1...N] = dummy bytes used to clock incoming data
     *
     * RX[0] = command-phase dummy byte
     * RX[1...N] = payload bytes
     *
     * totalLen = 1 command byte + N useful data bytes
     */
    uint16_t _total_length = (uint16_t)(dataLength + 1U);

    if ((txBufLength < _total_length) || (rxBufLength < _total_length))
        return false;

    // Disable autoBankSelect only when the caller has already selected the target bank.
    if (autoBankSelect) {
        if (!ICM42688_WriteBankAuto(pHandle, startEncodedReg)) {
            return false;
        }
    }

    // Clear only the bytes used by this transaction; caller-owned tail bytes are left untouched.
    memset(pTxBuf, 0, _total_length);
    memset(pRxBuf, 0, _total_length);

    pTxBuf[0] = ICM42688_SPI_Read_Command(startEncodedReg);

    ICM42688_CS_Pull_Low(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_TransmitReceive_DMA(pHandle->spi_config.hspi, pTxBuf, pRxBuf, _total_length);
    if (_status != HAL_OK) {
        ICM42688_CS_Pull_High(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);
        return false;
    }

    // DMA is still running; release CS from the completion callback with ICM42688_DMA_End().
    return true;
}


/**
 * @brief   Start an SPI DMA write of one byte to an encoded register.
 * @param   pHandle         Pointer to ICM42688 handle.
 * @param   encodedReg      Register encoded with bank and address.
 * @param   data            Byte to write.
 * @param   pTxBuf          DMA TX buffer; must remain valid until transfer completion.
 * @param   txBufLength     Size of pTxBuf in bytes.
 * @param   autoBankSelect  Select the encoded bank before DMA, or false if caller preselected it.
 * @return  true if the DMA transfer starts, otherwise false.
 * @warning CS remains low until the DMA completion path calls ICM42688_DMA_End().
 */
bool
ICM42688_WriteReg_DMA_Start(ICM42688_Handle_t *pHandle, ICM42688_Reg_t encodedReg, uint8_t data, uint8_t *pTxBuf,
                            uint16_t txBufLength, bool autoBankSelect)
{
    if (!pHandle || !pTxBuf)
        return false;

    if (txBufLength < 2U)
        return false;

    if (!pHandle->spi_config.hspi || !pHandle->spi_config.cs_port)
        return false;

    if (autoBankSelect) {
        if (!ICM42688_WriteBankAuto(pHandle, encodedReg)) {
            return false;
        }
    }

    // TX[0] is the write command; TX[1] is the payload byte.
    pTxBuf[0] = ICM42688_SPI_Write_Command(encodedReg);
    pTxBuf[1] = data;

    ICM42688_CS_Pull_Low(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_Transmit_DMA(pHandle->spi_config.hspi, pTxBuf, 2U);
    if (_status != HAL_OK) {
        ICM42688_CS_Pull_High(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);
        return false;
    }

    // DMA is still running; release CS from the completion callback with ICM42688_DMA_End().
    return true;
}



/**
 * @brief   Start an SPI DMA burst write to consecutive registers.
 * @param   pHandle          Pointer to ICM42688 handle.
 * @param   startEncodedReg  First encoded register to write.
 * @param   pData            Payload bytes to copy into pTxBuf.
 * @param   pTxBuf           DMA TX buffer; must remain valid until transfer completion.
 * @param   txBufLength      Size of pTxBuf in bytes.
 * @param   dataLength       Number of payload bytes to write.
 * @param   autoBankSelect   Select the encoded bank before DMA, or false if caller preselected it.
 * @return  true if the DMA transfer starts, otherwise false.
 * @warning CS remains low until the DMA completion path calls ICM42688_DMA_End().
 */
bool
ICM42688_WriteRegs_DMA_Start(ICM42688_Handle_t *pHandle, ICM42688_Reg_t startEncodedReg, const uint8_t *pData,
                             uint8_t *pTxBuf, uint16_t txBufLength, uint16_t dataLength, bool autoBankSelect)
{
    if (!pHandle || !pData || (dataLength == 0U) || !pTxBuf)
        return false;

    if (!pHandle->spi_config.hspi || !pHandle->spi_config.cs_port)
        return false;

    // FIFO_DATA is read-only from this driver's point of view; normal writes must stay inside one bank.
    bool    _is_fifo_data_reg = (startEncodedReg == ICM42688_UB0_FIFO_DATA);
    uint8_t _start_addr       = ICM42688_REG_ADDR(startEncodedReg);
    if (_is_fifo_data_reg || (((uint16_t)_start_addr + dataLength) > ICM42688_REG_ADDR_LIMIT)) {
        return false;
    }

    uint16_t _total_length = (uint16_t)(dataLength + 1U);
    if (txBufLength < _total_length)
        return false;

    // Disable autoBankSelect only when the caller has already selected the target bank.
    if (autoBankSelect) {
        if (!ICM42688_WriteBankAuto(pHandle, startEncodedReg)) {
            return false;
        }
    }

    // TX[0] is the write command; TX[1...N] are payload bytes.
    pTxBuf[0] = ICM42688_SPI_Write_Command(startEncodedReg);

    for (uint16_t i = 0; i < dataLength; i++) {
        pTxBuf[i + 1U] = pData[i];
    }

    ICM42688_CS_Pull_Low(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    HAL_StatusTypeDef _status = HAL_SPI_Transmit_DMA(pHandle->spi_config.hspi, pTxBuf, _total_length);
    if (_status != HAL_OK) {
        ICM42688_CS_Pull_High(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);
        return false;
    }

    // DMA is still running; release CS from the completion callback with ICM42688_DMA_End().

    return true;
}

/**
 * @brief   Release CS after a DMA SPI transaction has completed.
 * @param   pHandle Pointer to ICM42688 handle.
 * @return  true if the handle contains valid SPI and CS configuration, otherwise false.
 */
bool
ICM42688_DMA_End(ICM42688_Handle_t *pHandle)
{
    if (!pHandle || !pHandle->spi_config.hspi || !pHandle->spi_config.cs_port)
        return false;

    ICM42688_CS_Pull_High(pHandle->spi_config.cs_port, pHandle->spi_config.cs_pin);

    return true;
}
