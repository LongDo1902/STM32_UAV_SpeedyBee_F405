#ifndef IMU_ACQ_H
#define IMU_ACQ_H

#include "imu/core/icm42688_types.h"
#include "imu_sample.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    // SPI configurations
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *cs_port;
    uint16_t           cs_pin;

    // HAL_GPIO pin mask for ICM42688 INT1 pin
    uint16_t int1_gpio_pin;

    // Dedicated 32-bit timer counter configured as a 1MHz free-running counter
    TIM_HandleTypeDef *htim_us;
} IMU_ACQ_Config_t;

typedef struct
{
    uint32_t exti_count;                  // How many FIFO WTM interrupts arrived from ICM42688
    uint32_t exti_while_dma_active_count; // How many IMU interrupts arrived while the previous SPI DMA
                                          // transfer was still active
    uint32_t
        no_free_dma_slot_count;  // How many times an IMU interrupt arrived but neither DMA buffer was free
    uint32_t dma_start_count;    // How many SPI DMA FIFO reads were successfully started
    uint32_t dma_complete_count; // How many succeed full-duplex SPI DMA transaction

    uint32_t dma_start_error_count; // How many times the firmware attempted to start SPI DMA but HAL rejected
    uint32_t dma_transfer_error_count; // Counts DMA transaction started but later failed during transfer

    uint32_t parse_error_count; // Count completed DMA buffers that could not be decoded correctly into the
                                // expected FIFO frames

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


/* =====================================================================
 *  API DECLARATIONS
 * ===================================================================== */
/**
 * @brief   Initialize the ICM42688 and its interrupt-driven SPI DMA acquisition path.
 *          The function validates all HAL handles, configures the 1 MHz timebase and FIFO watermark path,
 *          performs startup gyro-bias calibration, and enables acquisition only after every step succeeds.
 * @param   pAcqConfig  Pointer to the SPI, GPIO, interrupt, and timebase configuration.
 * @return  true when the complete acquisition path is initialized, otherwise false.
 */
bool IMU_ACQ_Init(const IMU_ACQ_Config_t *pAcqConfig);

/**
 * @brief   Handle an ICM42688 FIFO-watermark EXTI event and start a DMA FIFO read.
 *          It timestamps the interrupt, reserves a free ping-pong slot, and launches a fixed-size FIFO
 *          transfer. The handler never performs a blocking bank-select write, so user bank 0 must remain
 *          selected.
 * @param   gpio_pin  HAL GPIO pin mask reported by the EXTI callback.
 * @return  true when a DMA FIFO read starts successfully, otherwise false.
 * @note    Call this function from the EXTI callback for the configured ICM42688 INT1 pin.
 */
bool IMU_ACQ_On_EXTI(uint16_t gpio_pin);

/**
 * @brief   Complete the active IMU SPI DMA transfer and make its slot ready for processing.
 *          The callback releases chip select, records completion order, and changes slot ownership under a
 *          critical section; it does not parse FIFO data in interrupt context.
 * @param   hspi  Pointer to the SPI handle reported by the DMA completion callback.
 * @return  true when the callback belongs to the active IMU transfer, otherwise false.
 */
bool IMU_ACQ_On_SPI_DMA_Complete(SPI_HandleTypeDef *hspi);

/**
 * @brief   Abort and release the active IMU DMA slot after an SPI DMA transfer error.
 *          It releases chip select, returns the slot to the free pool, and latches a transfer fault for the
 * next published sample. Calls for unrelated SPI handles are ignored.
 * @param   hspi  Pointer to the SPI handle reported by the DMA error callback.
 */
void IMU_ACQ_On_SPI_DMA_Error(SPI_HandleTypeDef *hspi);

/**
 * @brief   Copy the most recently published IMU sample.
 *          The validity check and structure copy occur inside a short critical section, giving the caller a
 *          coherent snapshot while publication can continue from another execution context.
 * @param   pOutSample  Pointer to the returned IMU sample.
 * @return  true when a published sample is available, otherwise false.
 */
bool IMU_ACQ_ReadLatestSample(IMU_Sample_t *pOutSample);

/**
 * @brief   Copy the latest IMU sample only when its sequence differs from the caller's sequence.
 *          This provides non-blocking new-data detection without consuming or clearing the shared sample;
 *          each caller must maintain its own lastSequence value.
 * @param   pOutSample    Pointer to the returned IMU sample.
 * @param   lastSequence  Pointer to the caller's last consumed sequence, updated on success.
 * @return  true when a newer sample is copied, otherwise false.
 */
bool IMU_ACQ_ReadNewSample(IMU_Sample_t *pOutSample, uint32_t *lastSequence);

/**
 * @brief   Decode, average, timestamp, publish, and release the oldest ready DMA batch.
 *          Exactly IMU_FIFO_BATCH_SAMPLES frames are expected; malformed data is dropped and reported as a
 *          latched fault, while valid frames receive midpoint timing and a monotonically increasing sequence.
 * @return  Processing result indicating no work, a published sample, or a dropped batch.
 */
IMU_ACQ_ProcessResult_t IMU_ACQ_ProcessNextBatch(void);

/**
 * @brief   Copy the current IMU acquisition diagnostic counters and timing statistics.
 *          The complete status struct is copied under a critical section so related counters and dt extrema
 *          form one consistent diagnostic snapshot.
 * @param   pOutStatus  Pointer to the returned acquisition status snapshot.
 */
void IMU_ACQ_GetStatus(IMU_ACQ_Status_t *pOutStatus);

/**
 * @brief   Report whether the real-time IMU acquisition module is initialized.
 *          A true result means initialization completed and callbacks may accept work; it does not guarantee
 *          that the latest sample is healthy or that no runtime faults have occurred.
 * @return  true after successful initialization, otherwise false.
 */
bool IMU_ACQ_IsInitialized(void);

#endif /* IMU_ACQ_H */
