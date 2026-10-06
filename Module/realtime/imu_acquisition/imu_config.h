#ifndef IMU_CONFIG_H
#define IMU_CONFIG_H

// Sensor acquisition and published output rates; compile-time checks below enforce their fixed ratio
#define IMU_ODR_HZ              8000U // ICM42688 accelerometer/gyroscope ODR: one frame every 125 us
#define IMU_OUTPUT_HZ           4000U // Published rate after averaging two consecutive FIFO frames

// ICM42688 normal FIFO packet type 3 layout used by the real-time decoder
#define IMU_FIFO_PACKET_BYTES   16U // 16 bytes per packet (1B header, 6B accel, 6B gyro, 1B temperature, 2B timestamp)
#define IMU_FIFO_BATCH_SAMPLES  (IMU_ODR_HZ / IMU_OUTPUT_HZ) // Frames averaged into one output
#define IMU_FIFO_WTM_BYTES      (IMU_FIFO_BATCH_SAMPLES * IMU_FIFO_PACKET_BYTES) // FIFO threshold in bytes

// SPI DMA transfers one read-command byte followed by the FIFO payload
#define IMU_SPI_DMA_CMD_BYTES   1U
#define IMU_SPI_DMA_XFER_BYTES  (IMU_SPI_DMA_CMD_BYTES + IMU_FIFO_WTM_BYTES)

// Two DMA slots allow the ISR path to fill one slot while the worker processes the other
#define IMU_ACQ_DMA_SLOT_COUNT  2U
#define IMU_DMA_INVALID_SLOT_IDX  0xFFU // Sentinel outside the valid ping-pong slot index range

// Dedicated 32-bit free-running timer: one counter tick equals one microsecond
#define IMU_TIMEBASE_HZ         1000000U // 1 MHz
#define IMU_SENSOR_PERIOD_US    (IMU_TIMEBASE_HZ / IMU_ODR_HZ) // Nominal interval between FIFO frames
#define IMU_OUTPUT_PERIOD_US    (IMU_TIMEBASE_HZ / IMU_OUTPUT_HZ) // Nominal interval between averaged outputs

// Approximate time from the watermark/final frame to the center of uniformly spaced frames in one batch
#define IMU_BATCH_MIDPOINT_OFFSET_US ((((IMU_FIFO_BATCH_SAMPLES - 1U) * IMU_SENSOR_PERIOD_US) + 1U) / 2U)

// Health limits for the measured 4 kHz interval; unsigned timestamp subtraction supports timer wraparound
#define IMU_DT_MIN_US   175U
#define IMU_DT_MAX_US   350U
#define IMU_TIMEOUT_US  1000000U // One-second timeout threshold reserved for acquisition monitoring

// Number of stationary samples used for blocking startup gyro-bias calibration
#define IMU_GYRO_CALIBRATION_SAMPLE_COUNT 200U

#if (IMU_ODR_HZ == 0U) || (IMU_OUTPUT_HZ == 0U)
#error "IMU rates must be non-zero"
#endif

#if (IMU_ODR_HZ % IMU_OUTPUT_HZ) != 0
#error "IMU_ODR_HZ must be divisible by IMU_OUTPUT_HZ"
#endif

#if (IMU_FIFO_BATCH_SAMPLES != 2U)
#error "This acquisition implementation expects exactly 2 FIFO frames per output"
#endif

#if (IMU_FIFO_WTM_BYTES != 32U)
#error "Expected a 32-byte FIFO WTM for two normal 16-byte FIFO packets"
#endif

#if IMU_TIMEBASE_HZ != 1000000U
#error "This implementation expects a 1MHz timer so one counter tick equals one microsecond"
#endif

#endif /* IMU_CONFIG_H */