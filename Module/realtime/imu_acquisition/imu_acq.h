#ifndef IMU_ACQ_H
#define IMU_ACQ_H

#include <stdbool.h>

#include "imu_sample.h"
#include "stm32f4xx_hal.h"



bool IMU_ACQ_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);

void IMU_ACQ_On_EXTI(uint16_t gpio_pin, uint32_t timestamp_us);
void IMU_ACQ_On_SPI_DMA_Complete(SPI_HandleTypeDef *hspi);
void IMU_ACQ_On_SPI_DMA_Error(SPI_HandleTypeDef *hspi);

bool IMU_ACQ_GetLatestSample(IMU_Sample_t *out_sample);

#endif