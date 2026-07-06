#ifndef IMU_ACQ_H
#define IMU_ACQ_H

#include <stdbool.h>
#include <stdint.h>

#include "imu_sample.h"
#include "stm32f4xx_hal.h"

/**
 * @brief Initialize the ICM42688 acquisition path.
 * @note The sensor is configured by the ICM42688 driver for 8 kHz accel/gyro FIFO
 *       samples. This module consumes two 16-byte FIFO packets per 4 kHz event.
 */
bool IMU_ACQ_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);

/**
 * @brief Start one FIFO DMA read after an IMU interrupt edge.
 * @param gpio_pin EXTI pin reported by HAL; currently accepted for any pin.
 * @param timestamp_us Timestamp associated with the interrupt edge.
 */
void IMU_ACQ_On_EXTI(uint16_t gpio_pin, uint32_t timestamp_us);

/**
 * @brief Finish and decode the pending FIFO DMA read for this IMU SPI handle.
 */
void IMU_ACQ_On_SPI_DMA_Complete(SPI_HandleTypeDef *hspi);

/**
 * @brief Abort the pending FIFO DMA read for this IMU SPI handle.
 */
void IMU_ACQ_On_SPI_DMA_Error(SPI_HandleTypeDef *hspi);

/**
 * @brief Copy the latest decoded IMU sample.
 * @param out_sample Destination sample.
 * @return true when the copied sample is healthy, otherwise false.
 */
bool IMU_ACQ_GetLatestSample(IMU_Sample_t *out_sample);

#endif
