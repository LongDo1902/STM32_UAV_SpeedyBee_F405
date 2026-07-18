#ifndef IMU_ACQ_H
#define IMU_ACQ_H

#include "imu/core/icm42688_types.h"

typedef struct
{
    ICM42688_Handle_t *imu_handle;    // Pointer directly to real handle
    TIM_HandleTypeDef *htim_us;       // Dedicated 32-bit timer counter configured as a 1MHz free-running
    uint16_t           int1_gpio_pin; // HAL_GPIO pin mask for the ICM42688 INT1 pin
} IMU_ACQ_Config_t;

typedef struct
{
    uint32_t exti_count;                  // How many FIFO WTM interrupts arrived from ICM42688
    uint32_t exti_while_dma_active_count; // How many IMU interrupts arrived while the previous SPI DMA transfer
                                          // was still active
    uint32_t no_free_dma_slo_count;       // How many times an IMU interrupt arrived but neither DMA buffer was free
    uint32_t dma_start_count;             // How many SPI DMA FIFO reads were successfully started
    uint32_t dma_complete_count;          // How many succeed full-duplex SPI DMA transaction

    uint32_t dma_start_error_count;    // How many times the firmware attempted to start SPI DMA but HAL rejected
    uint32_t dma_transfer_error_count; // Counts DMA transaction started but later failed during transfer

    uint32_t parse_error_count; // Count completed DMA buffers that could not be decoded correctly into the expected
                                // FIFO frames

    uint32_t published_sample_count; // How many valid 4kHz averaged IMU samples were successfully published

    uint32_t bad_dt_count; // Counts published sample whose measured time is outside acceptable range

    uint32_t last_dt_us; // Stores the most recently measured interval between two published IMU samples
    uint32_t min_dt_us;  // Stores the smallest dt observed since statistics were reset
    uint32_t max_dt_us;  // Stores the largest dt observed since statistics were reset
} IMU_ACQ_Status_t;      // Diagnostics structure to track if acquisition path is working correctly

typedef enum
{
    IMU_ACQ_PROCESS_NONE = 0,
    IMU_ACQ_PROCESS_PUBLISHED,
    IMU_ACQ_PROCESS_DROPPED,
} IMU_ACQ_ProcessResult_t;

#endif /* IMU_ACQ_H */