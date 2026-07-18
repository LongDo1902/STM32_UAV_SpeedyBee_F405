#ifndef IMU_SAMPLE_H
#define IMU_SAMPLE_H

#include <stdbool.h>
#include <stdint.h>

// Faults observed since the previous successfully published sample
typedef enum
{
    IMU_FAULT_NONE             = 0U,
    IMU_FAULT_EXTI_WHILE_DMA   = (1U << 0),
    IMU_FAULT_NO_FREE_DMA_SLOT = (1U << 1),
    IMU_FAULT_DMA_START        = (1U << 2),
    IMU_FAULT_DMA_TRANSFER     = (1U << 3),
    IMU_FAULT_FIFO_PARSE       = (1U << 4),
    IMU_FAULT_BAD_DT           = (1U << 5),
} IMU_Fault_t;

typedef struct
{
    uint32_t irq_timestamp_us;     // Time when the FIFO WTM edge was captured by the MCU
    uint32_t timestamp_us;         // Effective time of the averaged physical measurement
    uint32_t publish_timestamp_us; // Time when parsing and publication completed

    // Interval between consecutive 4kHz outputs
    uint32_t dt_us;
    float    dt_s;

    uint32_t sequence;    // Increments once for each successfully parsed and published batch
    uint32_t fault_flags; // Accumulated faults since the prev published sample

    // Averaged of the two calibrated 8kHz FIFO frames
    float gyro_dps[3];
    float accel_g[3];
    float temp_c;

    bool healthy;
} IMU_Sample_t;

#endif /* IMU_SAMPLE_H */