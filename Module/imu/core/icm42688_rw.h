/*
 * icm42688_rw.h
 *
 *  Created on: Mar 5, 2026
 *      Author: dobaolong
 */

#ifndef INC_IMU_ICM42688_RW_H_
#define INC_IMU_ICM42688_RW_H_

#include "imu/core/icm42688_defs.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_types.h"

/**
 * @brief   Pull the ICM42688 chip-select line low to start an SPI transaction.
 *          This helper only drives the GPIO and does not verify bus ownership, so the caller must ensure no
 *          other task or interrupt is using the same SPI peripheral.
 * @param   pCsPort GPIO port connected to the CS signal.
 * @param   csPin GPIO pin mask for the CS signal.
 */
void ICM42688_CS_Pull_Low(GPIO_TypeDef *pCsPort, uint16_t csPin);

/**
 * @brief   Pull the ICM42688 chip-select line high to end an SPI transaction.
 *          Releasing CS terminates the current device transaction; call it on every completion and error path
 *          after CS has been asserted.
 * @param   pCsPort GPIO port connected to the CS signal.
 * @param   csPin GPIO pin mask for the CS signal.
 */
void ICM42688_CS_Pull_High(GPIO_TypeDef *pCsPort, uint16_t csPin);

/**
 * @brief   Write one byte to an encoded ICM42688 register.
 *          The encoded bank is selected automatically before the blocking SPI transfer, and the handle's
 *          cached bank is updated only when bank selection succeeds.
 * @param   pHandle Pointer to an initialized ICM42688 handle.
 * @param   encodedReg Register encoded with ICM42688_REG(), including bank and address.
 * @param   data Byte to write.
 * @return  true if bank selection and SPI transfer succeed, otherwise false.
 */
bool ICM42688_WriteReg(ICM42688_Handle_t *pHandle, ICM42688_Reg_t encodedReg, uint8_t data);

/**
 * @brief   Read one byte from an encoded ICM42688 register.
 *          The encoded bank is selected automatically and the received byte is stored only through the
 *          caller's output pointer, which must remain valid for the complete blocking transaction.
 * @param   pHandle Pointer to an initialized ICM42688 handle.
 * @param   encodedReg Register encoded with ICM42688_REG(), including bank and address.
 * @param   pOutData Destination for the received byte.
 * @return  true if bank selection and SPI transfer succeed, otherwise false.
 */
bool ICM42688_ReadReg(ICM42688_Handle_t *pHandle, ICM42688_Reg_t encodedReg, uint8_t *pOutData);

/**
 * @brief   Read a contiguous register range, or stream bytes from FIFO_DATA.
 *          Normal registers must remain within one user bank and the valid address range; FIFO_DATA is the
 *          special streaming exception where repeated bytes are consumed from the same register address.
 * @param   pHandle Pointer to an initialized ICM42688 handle.
 * @param   startEncodedReg First encoded register to read.
 * @param   pBuf Destination buffer.
 * @param   bufLength Number of payload bytes to read.
 * @return  true if the read succeeds, otherwise false.
 */
bool ICM42688_ReadRegs(ICM42688_Handle_t *pHandle, ICM42688_Reg_t startEncodedReg, uint8_t *pBuf,
                       uint16_t bufLength);

/**
 * @brief   Update selected bits in one register with a read-modify-write sequence.
 *          Bits outside mask are preserved, but the operation is not atomic across the SPI bus; callers must
 *          prevent concurrent writers from changing the same register between the read and write phases.
 * @param   pHandle Pointer to an initialized ICM42688 handle.
 * @param   encodedReg Register encoded with ICM42688_REG(), including bank and address.
 * @param   mask Bit mask selecting fields to update.
 * @param   valueMasked New field value, already shifted and masked.
 * @return  true if the read and write both succeed, otherwise false.
 */
bool ICM42688_Update_Reg_Bits(ICM42688_Handle_t *pHandle, ICM42688_Reg_t encodedReg, uint8_t mask,
                              uint8_t valueMasked);

/**
 * @brief   Start an SPI DMA burst read from a register range or FIFO_DATA.
 *          The function prepares the command and dummy TX bytes, asserts CS, and returns as soon as HAL
 *          accepts the asynchronous transfer. Both DMA buffers must remain unchanged until the
 *          completion/error callback.
 * @param   pHandle Pointer to an initialized ICM42688 handle.
 * @param   startEncodedReg First encoded register to read.
 * @param   pTxBuf DMA TX scratch buffer; byte 0 is filled with the read command.
 * @param   txBufLength Size of pTxBuf in bytes; must be at least dataLength + 1.
 * @param   pRxBuf DMA RX buffer; byte 0 is dummy, bytes 1..dataLength are payload.
 * @param   rxBufLength Size of pRxBuf in bytes; must be at least dataLength + 1.
 * @param   dataLength Number of payload bytes to read.
 * @param   autoBankSelect Select the encoded bank before starting DMA; if false, caller must preselect the
 *          bank.
 * @return  true if the DMA transfer starts, otherwise false.
 * @warning CS remains low until ICM42688_DMA_End() is called after DMA completion.
 */
bool ICM42688_ReadRegs_DMA_Start(ICM42688_Handle_t *pHandle, ICM42688_Reg_t startEncodedReg, uint8_t *pTxBuf,
                                 uint16_t txBufLength, uint8_t *pRxBuf, uint16_t rxBufLength,
                                 uint16_t dataLength, bool autoBankSelect);

/**
 * @brief   Start an SPI DMA write of one byte to an encoded register.
 *          The function builds the command/data pair, asserts CS, and returns after HAL accepts the
 *          asynchronous transfer. The caller owns completion handling and must keep pTxBuf valid until DMA
 *          finishes.
 * @param   pHandle Pointer to an initialized ICM42688 handle.
 * @param   encodedReg Register encoded with ICM42688_REG(), including bank and address.
 * @param   data Byte to write.
 * @param   pTxBuf DMA TX buffer; must remain valid until transfer completion.
 * @param   txBufLength Size of pTxBuf in bytes; must be at least 2.
 * @param   autoBankSelect Select the encoded bank before starting DMA; if false, caller must preselect the
 *          bank.
 * @return  true if the DMA transfer starts, otherwise false.
 * @warning CS remains low until ICM42688_DMA_End() is called after DMA completion.
 */
bool ICM42688_WriteReg_DMA_Start(ICM42688_Handle_t *pHandle, ICM42688_Reg_t encodedReg, uint8_t data,
                                 uint8_t *pTxBuf, uint16_t txBufLength, bool autoBankSelect);

/**
 * @brief   Start an SPI DMA burst write to consecutive registers.
 *          Payload bytes are copied behind the command byte before CS is asserted; the range must fit within
 *          one bank and pTxBuf must remain valid until the asynchronous transfer completes.
 * @param   pHandle Pointer to an initialized ICM42688 handle.
 * @param   startEncodedReg First encoded register to write.
 * @param   pData Payload bytes to copy into pTxBuf after the command byte.
 * @param   pTxBuf DMA TX buffer; must remain valid until transfer completion.
 * @param   txBufLength Size of pTxBuf in bytes; must be at least dataLength + 1.
 * @param   dataLength Number of payload bytes to write.
 * @param   autoBankSelect Select the encoded bank before starting DMA; if false, caller must preselect the
 *          bank.
 * @return  true if the DMA transfer starts, otherwise false.
 * @warning CS remains low until ICM42688_DMA_End() is called after DMA completion.
 */
bool ICM42688_WriteRegs_DMA_Start(ICM42688_Handle_t *pHandle, ICM42688_Reg_t startEncodedReg,
                                  const uint8_t *pData, uint8_t *pTxBuf, uint16_t txBufLength,
                                  uint16_t dataLength, bool autoBankSelect);

/**
 * @brief   Finish a DMA transaction by releasing the ICM42688 chip-select line.
 *          Call this from both DMA completion and DMA error handling so the device does not remain selected
 *          after the HAL transaction ends.
 * @param   pHandle Pointer to an initialized ICM42688 handle.
 * @return  true if the handle contains valid SPI and CS configuration, otherwise false.
 */
bool ICM42688_DMA_End(ICM42688_Handle_t *pHandle);

#endif /* INC_IMU_ICM42688_RW_H_ */
