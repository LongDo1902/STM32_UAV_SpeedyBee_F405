#ifndef IMU_SAMPLE_H
#define IMU_SAMPLE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Fault bits accumulated between published IMU samples.
 *          Runtime faults remain latched until the worker attaches them to a sample. A recovery failure is
 * also counted in IMU_ACQ_Status_t and stops acquisition, so no later sample may be available to carry its
 * bit.
 */
typedef enum
{
    IMU_FAULT_NONE                  = 0U,        // No fault was latched for this publication interval
    IMU_FAULT_EXTI_WHILE_DMA_ACTIVE = (1U << 0), // A new watermark arrived before the prior DMA completed
    IMU_FAULT_NO_FREE_DMA_SLOT      = (1U << 1), // Both ping-pong slots were unavailable at a watermark
    IMU_FAULT_DMA_START             = (1U << 2), // HAL rejected the FIFO DMA start request
    IMU_FAULT_DMA_TRANSFER          = (1U << 3), // An accepted DMA transaction later failed
    IMU_FAULT_FIFO_PARSE            = (1U << 4), // DMA payload did not match the required FIFO batch format
    IMU_FAULT_BAD_DT                = (1U << 5), // Measurement interval was outside IMU_DT_MIN/MAX_US
    IMU_FAULT_FIFO_RECOVERY_FAILED  = (1U << 6), // FIFO resynchronization failed and acquisition stopped
} IMU_Fault_t;

/**
 * @brief   One calibrated IMU output produced from a complete FIFO batch.
 *          Sensor values are averages of the configured 8 kHz input frames, while timestamps describe the
 *          watermark edge, estimated measurement midpoint, and eventual publication time.
 */
typedef struct
{
    uint32_t irq_timestamp_us;     // MCU time when the FIFO watermark EXTI edge was captured
    uint32_t timestamp_us;         // Estimated midpoint time of the averaged physical measurement
    uint32_t publish_timestamp_us; // MCU time after parsing, calibration, and averaging completed

    // Interval between consecutive 4 kHz measurement timestamps
    uint32_t dt_us;
    float    dt_s;

    uint32_t sequence;    // Increments once for each successfully parsed and published batch
    uint32_t fault_flags; // IMU_Fault_t bits accumulated since the previous published sample

    // Average of the calibrated FIFO frames represented by this output
    float gyro_dps[3];
    float accel_g[3];
    float temp_c;

    bool healthy; // True only when dt is in range and fault_flags is IMU_FAULT_NONE
} IMU_Sample_t;

#endif /* IMU_SAMPLE_H */