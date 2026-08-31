#ifndef IMU_ACQ_H
#define IMU_ACQ_H

#include "imu/core/icm42688_types.h"
#include "imu_sample.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Hardware resources owned by the real-time IMU acquisition module.
 *          The SPI peripheral and 1 MHz timer must remain valid for the module lifetime, and
 *          int1_gpio_pin must use the same HAL pin mask delivered to the EXTI callback. The timer
 *          counter is treated as a free-running 32-bit microsecond timebase and may wrap normally.
 */
typedef struct
{
    // Blocking configuration accesses and FIFO DMA transfers share this SPI peripheral
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *cs_port;
    uint16_t           cs_pin;

    // HAL_GPIO pin mask for ICM42688 INT1 pin
    uint16_t int1_gpio_pin;

    // Dedicated 32-bit timer counter configured as a 1MHz free-running counter
    TIM_HandleTypeDef *htim_us;
} IMU_ACQ_Config_t;

/**
 * @brief   Cumulative diagnostics for interrupt, DMA, recovery, parsing, publication, and timing
 *          behavior. Counters are reset by IMU_ACQ_Init() and copied atomically by
 *          IMU_ACQ_GetStatus(); they are intended for runtime health monitoring and do not replace
 *          inspection of each published sample's fault_flags and healthy fields.
 */
typedef struct
{
    uint32_t exti_count;                  // FIFO-watermark EXTI events accepted for this IMU pin
    uint32_t exti_while_dma_active_count; // WTMs received before the previous SPI DMA completed
    uint32_t no_free_dma_slot_count;      // Watermarks received while both DMA slots were occupied
    uint32_t dma_start_count;             // SPI DMA FIFO reads successfully accepted by HAL
    uint32_t dma_complete_count;          // Full-duplex SPI DMA transactions completed successfully

    uint32_t dma_start_error_count;    // FIFO DMA start requests rejected immediately by HAL
    uint32_t dma_transfer_error_count; // DMA transactions that started but later entered the error
                                       // callback

    uint32_t parse_error_count; // Completed DMA buffers that did not contain the expected complete
                                // FIFO batch

    uint32_t published_sample_count; // Successfully parsed and published 4 kHz averaged samples

    uint32_t fifo_recovery_count;           // FIFO resynchronization sequences completed successfully
    uint32_t fifo_recovery_error_count;     // Recovery sequences that failed and stopped acquisition
    uint32_t recovery_discarded_slot_count; // Pre-recovery DMA slots discarded to restore timeline
                                            // consistency

    uint32_t bad_dt_count; // Published samples whose measured interval was outside configured
                           // health limits
    uint32_t last_dt_us;   // Most recently measured interval between published measurement timestamps
    uint32_t min_dt_us;    // Smallest observed interval since initialization
    uint32_t max_dt_us;    // Largest observed interval since initialization
} IMU_ACQ_Status_t;

/**
 * @brief   Result of one worker-side processing attempt.
 *          A dropped result includes malformed batches and completed/failed recovery attempts; NONE
 *          indicates that no publishable batch was available yet. Only PUBLISHED guarantees that
 *          the caller's output structure was updated.
 */
typedef enum
{
    IMU_ACQ_PROCESS_NONE = 0,
    IMU_ACQ_PROCESS_PUBLISHED,
    IMU_ACQ_PROCESS_DROPPED,
} IMU_ACQ_ProcessResult_t;

/**
 * @brief   Outcome returned by the EXTI path after inspecting acquisition and recovery state.
 *          The result distinguishes an unrelated/disabled interrupt from a started DMA transfer and
 *          an event that deferred work to the recovery path.
 */
typedef enum
{
    IMU_ACQ_EXTI_IGNORED = 0,
    IMU_ACQ_EXTI_DMA_STARTED,
    IMU_ACQ_EXTI_RECOVERY_REQUESTED,
} IMU_ACQ_EXTI_Result_t;


/* =====================================================================
 *  API DECLARATIONS
 * ===================================================================== */
/**
 * @brief   Initialize the ICM42688 and its interrupt-driven SPI DMA acquisition path.
 *          The function validates all HAL handles, configures the 1 MHz timebase and FIFO watermark
 *          path, performs startup gyro-bias calibration, and enables acquisition only after every
 *          step succeeds. Both SPI DMA streams must use DMA_NORMAL, and the CS/INT1 masks must each
 *          identify exactly one GPIO pin.
 * @param   pAcqConfig  Pointer to the SPI, GPIO, interrupt, and timebase configuration.
 * @return  true when the complete acquisition path is initialized, otherwise false.
 * @warning Keep the vehicle stationary during startup gyro calibration, and do not dispatch
 *          acquisition callbacks until this function returns true. The supplied HAL handles and
 *          GPIO configuration must remain valid and dedicated to this module after initialization.
 * @note    Initialization is accepted only once. An unrecoverable recovery failure latches the
 *          fatal-stop state, which intentionally requires an MCU reset before reinitialization.
 */
bool IMU_ACQ_Init(const IMU_ACQ_Config_t *pAcqConfig);

/**
 * @brief   Handle an ICM42688 FIFO-watermark EXTI event and start a DMA FIFO read.
 *          It timestamps the interrupt, reserves a free ping-pong slot, and launches a fixed-size
 *          FIFO transfer. The handler never performs a blocking bank-select write, so user bank 0
 *          must remain selected.
 * @param   gpio_pin  HAL GPIO pin mask reported by the EXTI callback.
 * @return  IMU_ACQ_EXTI_DMA_STARTED when DMA begins, IMU_ACQ_EXTI_RECOVERY_REQUESTED when
 *          acquisition must resynchronize, or IMU_ACQ_EXTI_IGNORED when the callback is unrelated
 *          or acquisition is disabled.
 * @note    Call this function from the EXTI callback for the configured ICM42688 INT1 pin. Parsing,
 *          FIFO flushing, and other blocking sensor accesses are deliberately deferred to worker
 *          context.
 */
IMU_ACQ_EXTI_Result_t IMU_ACQ_On_EXTI(uint16_t gpio_pin);

/**
 * @brief   Complete the active IMU SPI DMA transfer and make its slot ready for processing.
 *          The callback releases chip select, records completion order, and changes slot ownership
 *          under a critical section; it does not parse FIFO data in interrupt context.
 * @param   hspi  Pointer to the SPI handle reported by the DMA completion callback.
 * @return  true when the callback belongs to the active IMU transfer, otherwise false.
 * @note    This callback only transfers slot ownership; the worker must be notified separately to
 *          parse and publish the completed batch.
 */
bool IMU_ACQ_On_SPI_DMA_Complete(SPI_HandleTypeDef *hspi);

/**
 * @brief   Handle an error reported for the active IMU SPI DMA transfer.
 *          It releases chip select and software ownership of the slot, latches a transfer fault,
 *          and requests worker-side FIFO/SPI timeline recovery before another batch is accepted.
 *          Calls for unrelated SPI handles or transfers with no active slot are ignored.
 * @param   hspi  Pointer to the SPI handle reported by the DMA error callback.
 * @return  true when the callback owns and releases the active IMU transfer, otherwise false.
 * @note    The callback does not perform blocking recovery. The worker must be notified separately
 *          so IMU_ACQ_ProcessNextBatch() can service the pending recovery request.
 */
bool IMU_ACQ_On_SPI_DMA_Error(SPI_HandleTypeDef *hspi);

/**
 * @brief   Copy the most recently published IMU sample.
 *          The validity check and structure copy occur inside a short critical section, giving the
 *          caller a coherent snapshot while publication can continue from another execution
 *          context.
 * @param   pOutSample  Pointer to the returned IMU sample.
 * @return  true when a published sample is available, otherwise false.
 * @note    A successful recovery invalidates the mailbox until a clean post-recovery sample is
 *          published.
 */
bool IMU_ACQ_ReadLatestSample(IMU_Sample_t *pOutSample);

/**
 * @brief   Copy the latest IMU sample only when its sequence differs from the caller's sequence.
 *          This provides non-blocking new-data detection without consuming or clearing the shared
 *          sample; each caller must maintain its own lastSequence value.
 * @param   pOutSample    Pointer to the returned IMU sample.
 * @param   lastSequence  Pointer to the caller's last consumed sequence, updated on success.
 * @return  true when a newer sample is copied, otherwise false.
 * @note    Each consumer must retain an independent lastSequence value; reading does not consume or
 *          clear the shared mailbox.
 */
bool IMU_ACQ_ReadNewSample(IMU_Sample_t *pOutSample, uint32_t *lastSequence);

/**
 * @brief   Decode, average, timestamp, publish, and release the oldest ready DMA batch.
 *          Exactly IMU_FIFO_BATCH_SAMPLES frames are expected; malformed data is dropped and
 *          reported as a latched fault, while valid frames receive midpoint timing and a
 *          monotonically increasing sequence. Pending recovery is handled before ready slots; a
 *          completed recovery intentionally reports DROPPED because pre-recovery data and timing
 *          history were discarded.
 * @param   pOutSample  Pointer to the synchronous real-time output written only when a sample is
 *          published.
 * @return  Processing result indicating no work, a published sample, or a dropped batch/recovery
 *          event.
 * @warning Call from one worker/task context only. This function performs FIFO parsing and may run
 *          blocking recovery register transactions; it is not intended for ISR use.
 */
IMU_ACQ_ProcessResult_t IMU_ACQ_ProcessNextBatch(IMU_Sample_t *pOutSample);

/**
 * @brief   Copy the current IMU acquisition diagnostic counters and timing statistics.
 *          The complete status struct is copied under a critical section so related counters and dt
 *          extrema form one consistent diagnostic snapshot.
 * @param   pOutStatus  Pointer to the returned acquisition status snapshot.
 * @note    A NULL output pointer is ignored. Counters are cumulative from the most recent
 *          IMU_ACQ_Init() call and may wrap after extended operation.
 */
void IMU_ACQ_GetStatus(IMU_ACQ_Status_t *pOutStatus);

/**
 * @brief   Report whether the real-time IMU acquisition module is initialized.
 *          A true result means initialization completed and callbacks may accept work; it does not
 *          guarantee that the latest sample is healthy or that no runtime faults have occurred.
 * @return  true after successful initialization, otherwise false. Runtime recovery failure also
 *          returns the module to the false state.
 */
bool IMU_ACQ_IsInitialized(void);

/**
 * @brief   Report whether acquisition stopped after an unrecoverable SPI-DMA or FIFO recovery
 *          failure.
 * @return  true when only a deliberate MCU reset may rearm acquisition, otherwise false.
 * @note    The fatal state is persistent so repeated initialization attempts cannot reuse DMA or
 *          sensor state whose ownership could not be restored safely.
 */
bool IMU_ACQ_IsFatalStopped(void);

#endif /* IMU_ACQ_H */
