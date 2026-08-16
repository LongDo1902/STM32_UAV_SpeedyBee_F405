#include "imu_acq.h"
#include "imu/icm42688_application.h"
#include "imu_config.h"
#include "imu_sample.h"

#include <string.h>

/**
 * @brief   Real-time integration assumptions that must be verified on target hardware.
 *          FIFO-watermark EXTI latency, SPI DMA duration, worker parsing time, and consumer
 *          scheduling must all remain below their respective 8 kHz/4 kHz deadlines. Runtime
 *          counters expose missed timing, occupied buffers, DMA failures, parsing failures, and
 *          recovery outcomes for validation.
 * @note    A dedicated 1 MHz hardware timer timestamps events independently of task scheduling.
 *          INT_CONFIG1 uses TPULSE_DURATION=1, TDEASSERT_DISABLE=1, and INT_ASYNC_RESET=0 for ODR
 *          operation above 4 kHz.
 */


/**
 * @brief   Ownership states for one DMA slot
 *          EXTI moves a free slot to ACTIVE, the DMA-complete callback moves it to READY, and the
 *          worker claims it as PROCESSING before returning it to FREE. State transitions shared
 *          with callbacks require a critical section.
 */
typedef enum
{
    IMU_DMA_SLOT_FREE = 0,
    IMU_DMA_SLOT_ACTIVE,
    IMU_DMA_SLOT_READY,
    IMU_DMA_SLOT_PROCESSING,
} IMU_DMA_SlotState_t;


/**
 * @brief   Worker-side result of evaluating a pending FIFO recovery request.
 *          Recovery waits until DMA releases SPI/CS, then disables the watermark route, flushes
 *          FIFO state, discards stale slots, resets timestamp continuity, and re-enables
 *          acquisition.
 */
typedef enum
{
    IMU_ACQ_RECOVERY_NOT_NEEDED = 0,
    IMU_ACQ_RECOVERY_NEEDED_BUT_WAIT_DMA,
    IMU_ACQ_RECOVERY_DONE,
    IMU_ACQ_RECOVERY_FAILED,
} IMU_ACQ_RecoveryResult_t;


/**
 * @brief   DMA-owned buffers and metadata for one acquisition slot.
 *          tx and rx must remain stable for the full asynchronous SPI transfer. irq_timestamp_us
 *          belongs to the watermark that reserved the slot, while completion_order selects the
 *          oldest READY slot after callbacks.
 */
typedef struct
{
    // HAL DMA reads from tx and writes to rx until the SPI completion/error callback
    uint8_t tx[IMU_SPI_DMA_XFER_BYTES];
    uint8_t rx[IMU_SPI_DMA_XFER_BYTES];

    // Metadata shared between EXTI, DMA callbacks, recovery, and worker processing
    volatile IMU_DMA_SlotState_t state;
    uint32_t                     irq_timestamp_us;
    uint32_t                     completion_order;
} IMU_DMA_Slot_t;


/**
 * @brief   Module-owned sensor, configuration, slot, mailbox, and diagnostic state.
 *          These objects are reset by IMU_ACQ_Init(); objects accessed by both callbacks and worker
 *          code are protected with short PRIMASK-based critical sections.
 */
static ICM42688_Handle_t         icm42688_handle_;
static ICM42688_Offset_Raw_t     icm42688_offset_raw_;
static IMU_ACQ_Config_t          acq_config_;
static IMU_DMA_Slot_t            dma_slot_[IMU_ACQ_DMA_SLOT_COUNT];
static IMU_Sample_t              latest_sample_;
static volatile IMU_ACQ_Status_t status_;


/**
 * @brief   Acquisition lifecycle, ordering, publication, and recovery state.
 *          filling_dma_slot_ identifies the slot whose buffers are currently owned by HAL DMA.
 *          Recovery flags block new EXTI transfers while the worker performs blocking FIFO
 *          resynchronization.
 */
static volatile bool     initialized_      = false;
static volatile uint8_t  filling_dma_slot_ = IMU_DMA_INVALID_SLOT_IDX;
static volatile uint32_t completion_order_ = 0U;

static uint32_t published_sequence_  = 0U;
static bool     latest_sample_valid_ = false;

static bool     previous_timestamp_valid_ = false;
static uint32_t previous_timestamp_us_    = 0U;

static volatile bool recovery_requested_  = false;
static volatile bool recovery_in_process_ = false;
static volatile bool fatal_stopped_       = false;

static volatile uint32_t pending_faults_ = IMU_FAULT_NONE;



/**
 * @brief   Save PRIMASK and temporarily disable maskable interrupts.
 *          The returned value preserves nesting: IMU_ACQ_ExitCritical() restores the caller's
 *          previous interrupt state instead of enabling interrupts unconditionally. Keep protected
 *          regions short and non-blocking.
 * @return  PRIMASK value that must be passed to IMU_ACQ_ExitCritical().
 */
static uint32_t
IMU_ACQ_EnterCritical(void)
{
    uint32_t _primask = __get_PRIMASK();
    __disable_irq();
    __DMB();
    return _primask;
}



/**
 * @brief   Restore the interrupt state saved by IMU_ACQ_EnterCritical().
 *          A data-memory barrier completes shared-state accesses before PRIMASK is restored.
 * @param   primask  Saved PRIMASK value returned by IMU_ACQ_EnterCritical().
 */
static void
IMU_ACQ_ExitCritical(uint32_t primask)
{
    __DMB();
    __set_PRIMASK(primask);
}



/**
 * @brief   Mask the configured ICM42688 INT1 EXTI line without changing its pending bit.
 *          Recovery clears stale pending state explicitly before the line is enabled again.
 */
static void
IMU_ACQ_EXTI_MaskInt1(void)
{
    const uint32_t _exti_line = (uint32_t)acq_config_.int1_gpio_pin;

    uint32_t _primask = IMU_ACQ_EnterCritical();
    EXTI->IMR &= ~_exti_line;
    IMU_ACQ_ExitCritical(_primask);
}



/**
 * @brief   Enable the configured ICM42688 INT1 EXTI line.
 *          The caller must establish consistent software, sensor FIFO, and pending-interrupt state
 *          before unmasking the line.
 */
static void
IMU_ACQ_EXTI_EnableInt1(void)
{
    const uint32_t _exti_line = (uint32_t)acq_config_.int1_gpio_pin;
    uint32_t       _primask   = IMU_ACQ_EnterCritical();
    EXTI->IMR |= _exti_line;
    IMU_ACQ_ExitCritical(_primask);
}



/**
 * @brief   Request acquisition recovery, latch fault bit(s), and immediately mask IMU INT1 EXTI line
 * @param   fault   Fault bit(s) to latch into @c pending_fault_
 * @warning Caller MUST already hold IMU acquisition critical section
 */
static void
IMU_ACQ_RecoveryRequestedLocked(uint32_t fault)
{
    pending_faults_ |= fault;
    recovery_requested_ = true;
    EXTI->IMR &= ~(uint32_t)acq_config_.int1_gpio_pin;
}



/**
 * @brief   Check whether the GPIO mask represent exactly one physical pin
 * @param   pin     HAL GPIO Pin mask to be validated
 * @return  true only when one bit set, otherwise false
 */
static bool
IMU_ACQ_IsSingleGpioPin(uint16_t pin)
{
    return (pin != 0U) && ((pin & (uint16_t)(pin - 1U)) == 0U);
}



/**
 * @brief   Read the dedicated free-running 1 MHz timer.
 *          The uint32_t result naturally wraps; elapsed-time calculations use unsigned
 *          subtraction so intervals remain valid across one timer rollover.
 * @return  Current timer count in microseconds.
 */
static uint32_t
IMU_ACQ_NowUs(void)
{
    return (uint32_t)__HAL_TIM_GET_COUNTER(acq_config_.htim_us);
}



/**
 * @brief   Atomically latch one or more acquisition fault bits.
 *          Existing bits are preserved until IMU_ACQ_TakePendingFaults() transfers the complete set
 *          to a published sample.
 * @param   fault  IMU_Fault_t bit mask to OR into pending_faults_.
 */
static void
IMU_ACQ_LatchFault(uint32_t fault)
{
    uint32_t _primask = IMU_ACQ_EnterCritical();
    pending_faults_ |= fault;
    IMU_ACQ_ExitCritical(_primask);
}



/**
 * @brief   Atomically take and clear all pending acquisition faults.
 *          Clearing occurs in the same critical section as the copy, allowing callbacks to begin
 *          accumulating the next sample's faults without losing any bit set before the snapshot.
 * @return  IMU_Fault_t bit mask accumulated since the previous take operation.
 */
static uint32_t
IMU_ACQ_TakePendingFaults(void)
{
    uint32_t _primask      = IMU_ACQ_EnterCritical();
    uint32_t _copied_fault = pending_faults_;
    pending_faults_        = IMU_FAULT_NONE;
    IMU_ACQ_ExitCritical(_primask);
    return _copied_fault;
}



/**
 * @brief   Estimate startup gyroscope bias while the vehicle is stationary.
 *          Raw samples are spaced by 1 ms to avoid repeatedly reading the same sensor update, then
 *          averaged into icm42688_offset_raw_. Motion during this blocking startup procedure
 *          becomes calibration error.
 * @param   sampleCounts  Number of raw samples to collect; valid range is 1 through 10000.
 * @return  true when every sample is read and the bias is stored, otherwise false.
 */
static bool
IMU_ACQ_CalibrateGyroBias(uint16_t sampleCounts)
{
    if (sampleCounts == 0U || sampleCounts > 10000U)
        return false;

    int32_t _gyro_sum[3] = {0, 0, 0};

    for (uint16_t i = 0U; i < sampleCounts; i++) {
        ICM42688_Raw_t _raw = {0}; // Clear the destination before each startup sample read

        if (!ICM42688_Get_Temp_Accel_Gyro_Raw(&icm42688_handle_, &_raw)) {
            return false;
        }
        for (uint8_t axis = 0U; axis < 3U; axis++) {
            _gyro_sum[axis] += _raw.raw_gyro[axis];
        }
        // Startup-only delay prevents consecutive reads from averaging the same sensor update
        // repeatedly
        HAL_Delay(1U);
    }

    memset(&icm42688_offset_raw_, 0, sizeof(icm42688_offset_raw_));
    for (uint8_t axis = 0U; axis < 3U; axis++) {
        icm42688_offset_raw_.offset_raw_gyro[axis] = (int32_t)(_gyro_sum[axis] / (int32_t)sampleCounts);
    }

    /**
     * @todo    Accel startup bias is intentionally left at zero. Estimating it correctly requires
     *          known board orientation and proper six-position calibration.
     */
    return true;
}



/**
 * @brief   Configure ICM42688 interrupt timing for operation above 4 kHz.
 *          The selected INT_CONFIG1 values lengthen the pulse, prevent early deassertion, and
 *          disable asynchronous reset behavior as required by the high-ODR interrupt path.
 * @return  true when INT_CONFIG1 is updated successfully, otherwise false.
 */
static bool
IMU_ACQ_Config_HighOdrInterruptTiming(void)
{
    return ICM42688_Set_INT_CONFIG1(&icm42688_handle_, false, true, true);
}



/**
 * @brief   Find the first DMA slot currently marked FREE.
 *          The caller must already hold the acquisition critical section because slot ownership is
 *          shared with DMA callbacks and worker processing.
 * @return  Free slot index, or IMU_DMA_INVALID_SLOT_IDX when every slot is occupied.
 */
static uint8_t
IMU_ACQ_FindFreeSlotLocked(void)
{
    for (uint8_t i = 0; i < IMU_ACQ_DMA_SLOT_COUNT; i++) {
        if (dma_slot_[i].state == IMU_DMA_SLOT_FREE) {
            return i;
        }
    }
    return IMU_DMA_INVALID_SLOT_IDX;
}



/**
 * @brief   Determine whether one completion order occurred before another across uint32_t wrap.
 *          Signed modular comparison preserves the correct order without resetting the free-running
 *          completion counter.
 * @param   first   Completion order being tested as the older value.
 * @param   second  Completion order being tested as the newer value.
 * @return  true when first occurred before second, otherwise false.
 * @note    The values must be separated by fewer than 2^31 completions. With only two DMA slots,
 *          this condition is satisfied for all simultaneously comparable slots.
 */
static bool
IMU_ACQ_OrderBefore(uint32_t first, uint32_t second)
{
    return ((int32_t)(first - second) < 0);
}



/**
 * @brief   Claim the oldest completed DMA slot waiting for worker processing.
 *          READY slots are compared with wraparound-safe completion ordering, and the selected slot
 *          is changed to PROCESSING inside the same critical section so recovery or another worker
 *          cannot claim it.
 * @return  Claimed slot index, or IMU_DMA_INVALID_SLOT_IDX when no slot is ready.
 */
static uint8_t
IMU_ACQ_TakeOldestReadySlot(void)
{
    uint8_t  _selected_slot = IMU_DMA_INVALID_SLOT_IDX;
    uint32_t _primask       = IMU_ACQ_EnterCritical();

    {
        for (uint8_t i = 0; i < IMU_ACQ_DMA_SLOT_COUNT; i++) {
            // Consider only completed slots that have not already been claimed by the worker
            if (dma_slot_[i].state == IMU_DMA_SLOT_READY) {
                if ((_selected_slot == IMU_DMA_INVALID_SLOT_IDX) ||
                    (IMU_ACQ_OrderBefore(dma_slot_[i].completion_order,
                                         dma_slot_[_selected_slot].completion_order))) {
                    _selected_slot = i; // Track the oldest READY slot found so far
                }
            }
        }

        if (_selected_slot != IMU_DMA_INVALID_SLOT_IDX) {
            dma_slot_[_selected_slot].state = IMU_DMA_SLOT_PROCESSING;
        }
    }

    IMU_ACQ_ExitCritical(_primask);
    return _selected_slot;
}



/**
 * @brief   Return one processed or rejected DMA slot to the FREE state.
 *          The state change is protected because a later EXTI callback may immediately reserve the
 *          released slot.
 * @param   slotIndex  Index of a valid slot previously claimed for processing.
 */
static void
IMU_ACQ_ReleaseSlot(uint8_t slotIndex)
{
    uint32_t _primask          = IMU_ACQ_EnterCritical();
    dma_slot_[slotIndex].state = IMU_DMA_SLOT_FREE;
    IMU_ACQ_ExitCritical(_primask);
}



/**
 * @brief   Decode, calibrate, and average one completed FIFO DMA batch.
 *          The payload must contain exactly IMU_FIFO_BATCH_SAMPLES normal packets with valid
 *          accelerometer, gyroscope, and temperature fields and must consume exactly
 *          IMU_FIFO_WTM_BYTES.
 * @param   pDmaSlot        Pointer to the completed DMA slot; rx byte zero is the SPI command-phase
 *                          dummy byte.
 * @param   pImuOutSample   Pointer to the averaged sensor output; timing and health metadata are
 *                          filled later.
 * @return  true when the complete batch has the expected structure and converts successfully,
 *          otherwise false.
 */
static bool
IMU_ACQ_DecodeAndAverage(const IMU_DMA_Slot_t *pDmaSlot, IMU_Sample_t *pImuOutSample)
{
    if (!pDmaSlot || !pImuOutSample) {
        return false;
    }

    const uint8_t *_p_fifo_byte = &pDmaSlot->rx[IMU_SPI_DMA_CMD_BYTES]; // Skip the command-phase dummy byte
                                                                        // before FIFO payload parsing
    uint16_t _parse_pos   = 0U;
    uint16_t _frame_count = 0U;

    float _accel_sum[3] = {0.0f, 0.0f, 0.0f};
    float _gyro_sum[3]  = {0.0f, 0.0f, 0.0f};
    float _temp_sum     = 0.0f;

    while (_parse_pos < IMU_FIFO_WTM_BYTES) {
        if (_frame_count >= IMU_FIFO_BATCH_SAMPLES) {
            return false;
        }
        // Clear per-packet decode objects before parsing the next FIFO frame
        ICM42688_FIFO_Frame_t             _frame  = {0};
        ICM42688_Temp_Accel_Gyro_Scaled_t _scaled = {0};

        if (!ICM42688_FIFO_Parse_One_Byte_Frame(&icm42688_handle_, &_frame, _p_fifo_byte, IMU_FIFO_WTM_BYTES,
                                                &_parse_pos)) {
            return false;
        }

        if ((_frame.packet_size != IMU_FIFO_PACKET_BYTES) || (!_frame.accel_valid) || (!_frame.gyro_valid) ||
            (!_frame.temp_valid)) {
            return false;
        }

        if (!(ICM42688_Calibrate_FIFO_Frame(&icm42688_handle_, &_frame, &icm42688_offset_raw_, &_scaled))) {
            return false;
        }

        for (uint8_t axis = 0; axis < 3U; axis++) {
            _gyro_sum[axis] += _scaled.gyro_dps[axis];
            _accel_sum[axis] += _scaled.accel_g[axis];
        }

        _temp_sum += _scaled.temp_c;
        _frame_count++;
    }

    if ((_frame_count != IMU_FIFO_BATCH_SAMPLES) || (_parse_pos != IMU_FIFO_WTM_BYTES)) {
        return false;
    }

    memset(pImuOutSample, 0, sizeof(*pImuOutSample));

    for (uint8_t axis = 0; axis < 3U; axis++) {
        pImuOutSample->accel_g[axis]  = _accel_sum[axis] / (float)IMU_FIFO_BATCH_SAMPLES;
        pImuOutSample->gyro_dps[axis] = _gyro_sum[axis] / (float)IMU_FIFO_BATCH_SAMPLES;
    }

    pImuOutSample->temp_c = _temp_sum / (float)IMU_FIFO_BATCH_SAMPLES;

    return true;
}



/**
 * @brief   Update the latest, minimum, and maximum observed output interval.
 *          A zero minimum marks the uninitialized state; all values use microseconds from
 *          measurement timestamps.
 * @param   dt_us  Current interval between consecutive published measurement timestamps.
 */
static void
IMU_ACQ_UpdateDtStatus(uint32_t dt_us)
{
    status_.last_dt_us = dt_us;

    if ((status_.min_dt_us == 0U) || (status_.min_dt_us > dt_us)) {
        status_.min_dt_us = dt_us;
    }

    if (status_.max_dt_us < dt_us) {
        status_.max_dt_us = dt_us;
    }
}



/**
 * @brief   Publish one processed IMU sample atomically for readers.
 *          The sample copy and validity flag update share one critical section so readers cannot
 *          observe a partially updated structure.
 * @param   pImuSample  Pointer to the sample to publish.
 */
static void
IMU_ACQ_Publish(const IMU_Sample_t *pImuSample)
{
    uint32_t _primask = IMU_ACQ_EnterCritical();
    {
        latest_sample_       = *pImuSample;
        latest_sample_valid_ = true;
    }
    IMU_ACQ_ExitCritical(_primask);
}



/**
 * @brief   Stop both streams of the full-duplex SPI DMA transaction and restore HAL ownership.
 * @return  true only when the SPI and both DMA handles report READY after the blocking stop/abort
 *          sequence.
 * @note    STM32 HAL starts RX DMA before TX DMA, so a rejected start can leave RX active. An RX
 *          transfer error can also leave the paired TX stream active. Recovery must stop both
 *          streams before making blocking SPI register accesses.
 */
static bool
IMU_ACQ_StopSpiDma(void)
{
    SPI_HandleTypeDef *_p_hspi = acq_config_.hspi;
    if ((!_p_hspi) || (!_p_hspi->hdmarx) || (!_p_hspi->hdmatx)) {
        return false;
    }

    (void)HAL_SPI_DMAStop(_p_hspi);
    (void)HAL_SPI_Abort(_p_hspi);

    return (HAL_SPI_GetState(_p_hspi) == HAL_SPI_STATE_READY) &&
           (HAL_DMA_GetState(_p_hspi->hdmarx) == HAL_DMA_STATE_READY) &&
           (HAL_DMA_GetState(_p_hspi->hdmatx) == HAL_DMA_STATE_READY);
}



/**
 * @brief   Run a pending FIFO resynchronization from worker context when SPI DMA is idle.
 *          The routine blocks new EXTI transfers, disables watermark routing, flushes sensor FIFO
 *          data, clears a pending MCU EXTI edge, discards all pre-recovery slots, resets timestamp
 *          continuity, and then restores watermark routing. A hardware-access failure stops
 *          acquisition rather than continuing from unknown state.
 * @return  Recovery state indicating no request, a wait for active DMA, successful
 *          resynchronization, or failure.
 * @warning Call only from worker/task context because recovery performs blocking SPI register
 *          transactions.
 */
static IMU_ACQ_RecoveryResult_t
IMU_ACQ_RunRecoveryIfNeeded(void)
{
    uint32_t _primask = IMU_ACQ_EnterCritical();

    if (!recovery_requested_) {
        IMU_ACQ_ExitCritical(_primask);
        return IMU_ACQ_RECOVERY_NOT_NEEDED;
    }

    /**
     * Do not flush the ICM42688 FIFO while SPI DMA is active. The active DMA transaction owns the
     * bus and keeps CS asserted until a completion or error callback releases it.
     *
     * Wait for DMA-complete or DMA-error callback to:
     *  1. Release CS
     *  2. Clear filling_dma_slot_
     *  3. Notify the worker task
     *
     * The worker can then retry recovery and safely perform blocking ICM42688 register accesses.
     */

    if (filling_dma_slot_ != IMU_DMA_INVALID_SLOT_IDX) {
        IMU_ACQ_ExitCritical(_primask);
        return IMU_ACQ_RECOVERY_NEEDED_BUT_WAIT_DMA;
    }

    // Prevent EXTI from starting a new DMA read while the worker performs blocking register
    // accesses
    recovery_in_process_ = true;
    IMU_ACQ_ExitCritical(_primask);

    // Prevent INT1 from entering the CPU handler; a new edge may remain pending until recovery
    // explicitly clears it
    IMU_ACQ_EXTI_MaskInt1();

    // HAL may still own one DMA stream after a paired-stream error or partial DMA start. Restore
    // SPI/DMA ownership before issuing any blocking sensor register transaction.
    if (!IMU_ACQ_StopSpiDma()) {
        goto recovery_failed;
    }

    // Disable new sensor FIFO-threshold events before changing FIFO and software timeline state
    if (!ICM42688_Set_Int1_FIFO_Threshold_Enable(&icm42688_handle_, false)) {
        goto recovery_failed;
    }

    // DMA is idle; discard every pre-recovery slot so old samples cannot be published after
    // resynchronization
    uint32_t _discarded_slot_count = 0;

    _primask = IMU_ACQ_EnterCritical();
    {
        for (uint8_t i = 0; i < IMU_ACQ_DMA_SLOT_COUNT; i++) {
            if (dma_slot_[i].state != IMU_DMA_SLOT_FREE) {
                dma_slot_[i].state = IMU_DMA_SLOT_FREE;
                _discarded_slot_count++;
            }
        }

        // The next clean sample establishes a new timing baseline and must not be compared with
        // pre-flush data
        previous_timestamp_valid_ = false;
        previous_timestamp_us_    = 0U;
        latest_sample_valid_      = false;

        status_.recovery_discarded_slot_count += _discarded_slot_count;
    }
    IMU_ACQ_ExitCritical(_primask);

    // Discard all unread sensor FIFO bytes so the next watermark begins a clean batch timeline
    if (!ICM42688_FIFO_Flush(&icm42688_handle_, true)) {
        goto recovery_failed;
    }

    // Remove any MCU EXTI event latched before or during recovery so stale work cannot restart DMA
    __HAL_GPIO_EXTI_CLEAR_IT(acq_config_.int1_gpio_pin);

    // Re-enable sensor watermark routing only after stale slots, EXTI state, and timestamp
    // continuity are cleared
    if (!ICM42688_Set_Int1_FIFO_Threshold_Enable(&icm42688_handle_, true)) {
        goto recovery_failed;
    }

    _primask = IMU_ACQ_EnterCritical();
    {
        recovery_in_process_ = false;
        recovery_requested_  = false;
        status_.fifo_recovery_count++;
    }
    IMU_ACQ_ExitCritical(_primask);

    // Software state and sensor routing are consistent before EXTI is enabled
    IMU_ACQ_EXTI_EnableInt1();

    return IMU_ACQ_RECOVERY_DONE;

recovery_failed:
    _primask = IMU_ACQ_EnterCritical();
    {
        status_.fifo_recovery_error_count++;
        pending_faults_ |= IMU_FAULT_FIFO_RECOVERY_FAILED;

        // Stop acquisition instead of accepting callbacks with unknown FIFO, interrupt, or SPI
        // state
        initialized_              = false;
        latest_sample_valid_      = false;
        previous_timestamp_valid_ = false;
        recovery_requested_       = false;
        recovery_in_process_      = false;
        filling_dma_slot_         = IMU_DMA_INVALID_SLOT_IDX;
        fatal_stopped_            = true;

        for (uint8_t i = 0U; i < IMU_ACQ_DMA_SLOT_COUNT; i++) {
            dma_slot_[i].state = IMU_DMA_SLOT_FREE;
        }
    }
    IMU_ACQ_ExitCritical(_primask);

    // Remove stale pending state, but leave INT1 EXTI masked
    __HAL_GPIO_EXTI_CLEAR_IT(acq_config_.int1_gpio_pin);

    return IMU_ACQ_RECOVERY_FAILED;
}



bool
IMU_ACQ_Init(const IMU_ACQ_Config_t *pAcqConfig)
{
    if (initialized_ || fatal_stopped_) {
        return false;
    }

    if (!pAcqConfig || (!pAcqConfig->hspi) || (!pAcqConfig->cs_port) || (!pAcqConfig->htim_us) ||
        (!pAcqConfig->hspi->hdmarx) || (!pAcqConfig->hspi->hdmatx) ||
        (!IMU_ACQ_IsSingleGpioPin(pAcqConfig->cs_pin)) ||
        (!IMU_ACQ_IsSingleGpioPin(pAcqConfig->int1_gpio_pin)) ||
        (pAcqConfig->hspi->hdmarx->Init.Mode != DMA_NORMAL) ||
        (pAcqConfig->hspi->hdmatx->Init.Mode != DMA_NORMAL)) {
        return false;
    }

    // Clear module-owned structs before copying configuration or exposing initialized state
    memset(&icm42688_handle_, 0, sizeof(icm42688_handle_));
    memset(&icm42688_offset_raw_, 0, sizeof(icm42688_offset_raw_));
    memset(&acq_config_, 0, sizeof(acq_config_));
    memset(dma_slot_, 0, sizeof(dma_slot_));
    memset(&latest_sample_, 0, sizeof(latest_sample_));
    memset((void *)&status_, 0, sizeof(status_));

    // Reset lifecycle, slot ownership, ordering, fault, publication, and recovery state
    initialized_      = false;
    filling_dma_slot_ = IMU_DMA_INVALID_SLOT_IDX;
    completion_order_ = 0U;

    published_sequence_  = 0U;
    latest_sample_valid_ = false;

    previous_timestamp_valid_ = false;
    previous_timestamp_us_    = 0U;

    recovery_requested_  = false;
    recovery_in_process_ = false;

    pending_faults_ = IMU_FAULT_NONE;

    // Mark every zeroed DMA slot explicitly FREE before interrupts can reserve one
    for (uint8_t i = 0U; i < IMU_ACQ_DMA_SLOT_COUNT; i++) {
        dma_slot_[i].state = IMU_DMA_SLOT_FREE;
    }

    // Copy the validated hardware configuration; the pointed-to HAL objects must remain valid
    // after return
    acq_config_ = *pAcqConfig;

    // Mask MCU INT1 EXTI
    IMU_ACQ_EXTI_MaskInt1();

    __HAL_TIM_SET_COUNTER(acq_config_.htim_us, 0U);          // Establish the acq timestamp epoch
    if (HAL_TIM_Base_Start(acq_config_.htim_us) != HAL_OK) { // Start free-running 1 MHz timebase
        goto init_failed;
    }

    icm42688_handle_.spi_config.hspi    = acq_config_.hspi;
    icm42688_handle_.spi_config.cs_port = acq_config_.cs_port;
    icm42688_handle_.spi_config.cs_pin  = acq_config_.cs_pin;

    if (!ICM42688_Init(&icm42688_handle_)) {
        goto init_failed;
    }

    if (!IMU_ACQ_CalibrateGyroBias(IMU_GYRO_CALIBRATION_SAMPLE_COUNT)) {
        goto init_failed;
    }

    // Configure interrupt timing, FIFO watermark, and a clean FIFO before exposing threshold
    // events on INT1
    if (!ICM42688_Set_Int1_FIFO_Threshold_Enable(&icm42688_handle_, false) ||
        !ICM42688_Set_Int1_DataReady_Enable(&icm42688_handle_, false) ||
        !ICM42688_Set_Int1_FIFO_Full_Enable(&icm42688_handle_, false) ||
        !ICM42688_Set_Int1_ResetDone_Enable(&icm42688_handle_, false) ||
        !IMU_ACQ_Config_HighOdrInterruptTiming() ||
        !ICM42688_Set_Int1_Config(&icm42688_handle_, INT_ACTIVE_HIGH, INT_PUSH_PULL, INT_PULSED) ||
        !ICM42688_Set_FIFO_Watermark(&icm42688_handle_, IMU_FIFO_WTM_BYTES) ||
        !ICM42688_Set_FIFO_WM_GT_THS(&icm42688_handle_, FIFO_WM_GREATER_THS_REPEAT) ||
        !ICM42688_FIFO_Flush(&icm42688_handle_, true)) {
        goto init_failed;
    }

    // Remove any stale EXTI event that occurred before or during initialization.
    // Sensor watermark routing is still disabled here
    __HAL_GPIO_EXTI_CLEAR_IT(acq_config_.int1_gpio_pin);

    // Enable fresh sensor watermark generation
    if (!ICM42688_Set_Int1_FIFO_Threshold_Enable(&icm42688_handle_, true)) {
        goto init_failed;
    }

    // Software state and sensor routing are now ready for watermark callbacks
    initialized_ = true;

    // Restore the EXTI line. A fresh pending watermark can now enter the callback while
    // initialized_ is already true
    IMU_ACQ_EXTI_EnableInt1();
    return true;

init_failed:
    initialized_ = false;
    __HAL_GPIO_EXTI_CLEAR_IT(acq_config_.int1_gpio_pin);
    return false;
}



IMU_ACQ_EXTI_Result_t
IMU_ACQ_On_EXTI(uint16_t gpio_pin)
{
    if (!initialized_ || gpio_pin != acq_config_.int1_gpio_pin) {
        return IMU_ACQ_EXTI_IGNORED;
    }

    // Capture the watermark edge time before bookkeeping to minimize timestamp jitter
    const uint32_t _irq_timestamp_us = IMU_ACQ_NowUs();
    status_.exti_count++;

    uint32_t _primask = IMU_ACQ_EnterCritical();

    // Recovery owns or will soon own the SPI/FIFO timeline; do not admit another DMA
    // transaction
    if (recovery_requested_ || recovery_in_process_) {
        IMU_ACQ_ExitCritical(_primask);
        return IMU_ACQ_EXTI_RECOVERY_REQUESTED;
    }

    // A second watermark arrived before the previous DMA completed, so FIFO batch timing is no
    // longer trusted
    if (filling_dma_slot_ != IMU_DMA_INVALID_SLOT_IDX) {
        status_.exti_while_dma_active_count++; // The previous watermark's DMA slot is still
                                               // being filled

        IMU_ACQ_RecoveryRequestedLocked((uint32_t)IMU_FAULT_EXTI_WHILE_DMA_ACTIVE);
        IMU_ACQ_ExitCritical(_primask);
        return IMU_ACQ_EXTI_RECOVERY_REQUESTED;
    }

    // Both slots occupied means this watermark cannot be drained on schedule; request timeline
    // resynchronization
    const uint8_t _free_slot_index = IMU_ACQ_FindFreeSlotLocked();
    if (_free_slot_index == IMU_DMA_INVALID_SLOT_IDX) {
        status_.no_free_dma_slot_count++;
        IMU_ACQ_RecoveryRequestedLocked((uint32_t)IMU_FAULT_NO_FREE_DMA_SLOT);
        IMU_ACQ_ExitCritical(_primask);
        return IMU_ACQ_EXTI_RECOVERY_REQUESTED;
    }

    dma_slot_[_free_slot_index].state            = IMU_DMA_SLOT_ACTIVE;
    dma_slot_[_free_slot_index].irq_timestamp_us = _irq_timestamp_us;
    filling_dma_slot_                            = _free_slot_index;

    IMU_ACQ_ExitCritical(_primask);

    IMU_DMA_Slot_t *slot = &dma_slot_[_free_slot_index];

    /**
     * @note    @c autoBankSelect is false to avoid a blocking SPI write in EXTI context.
     *          This module therefore owns the ICM42688 bus after initialization and keeps the
     *          device in user bank 0 during real-time acquisition.
     */
    if (!ICM42688_ReadRegs_DMA_Start(&icm42688_handle_, ICM42688_UB0_FIFO_DATA, slot->tx,
                                     (uint16_t)sizeof(slot->tx), slot->rx, (uint16_t)sizeof(slot->rx),
                                     IMU_FIFO_WTM_BYTES, false)) {
        _primask = IMU_ACQ_EnterCritical();

        {
            slot->state       = IMU_DMA_SLOT_FREE;
            filling_dma_slot_ = IMU_DMA_INVALID_SLOT_IDX;
            status_.dma_start_error_count++;

            IMU_ACQ_RecoveryRequestedLocked((uint32_t)IMU_FAULT_DMA_START);
        }

        IMU_ACQ_ExitCritical(_primask);
        return IMU_ACQ_EXTI_RECOVERY_REQUESTED;
    }

    status_.dma_start_count++;
    return IMU_ACQ_EXTI_DMA_STARTED;
}



bool
IMU_ACQ_On_SPI_DMA_Complete(SPI_HandleTypeDef *hspi)
{
    if (!initialized_ || (hspi != acq_config_.hspi)) {
        return false;
    }

    // Claim the recorded active slot while interrupts are masked so ownership cannot change
    // mid-callback
    uint32_t _primask = IMU_ACQ_EnterCritical();

    const uint8_t _active_dma_slot = filling_dma_slot_;
    if (_active_dma_slot == IMU_DMA_INVALID_SLOT_IDX) {
        IMU_ACQ_ExitCritical(_primask);
        return false;
    }

    {
        (void)ICM42688_DMA_End(&icm42688_handle_);

        dma_slot_[_active_dma_slot].completion_order = completion_order_++;
        dma_slot_[_active_dma_slot].state            = IMU_DMA_SLOT_READY;

        filling_dma_slot_ = IMU_DMA_INVALID_SLOT_IDX;
        status_.dma_complete_count++;
    }

    IMU_ACQ_ExitCritical(_primask);
    return true;
}



bool
IMU_ACQ_On_SPI_DMA_Error(SPI_HandleTypeDef *hspi)
{
    if (!initialized_ || hspi != acq_config_.hspi) {
        return false;
    }

    uint32_t _primask = IMU_ACQ_EnterCritical();
    {
        const uint8_t _active_dma_slot = filling_dma_slot_;
        if (_active_dma_slot == IMU_DMA_INVALID_SLOT_IDX) {
            IMU_ACQ_ExitCritical(_primask);
            return false;
        }

        // End the SPI transaction by releasing CS
        (void)ICM42688_DMA_End(&icm42688_handle_);

        // Release software ownership. Acquisition must request recovery and must not leave the DMA slot
        // ACTIVE after an error callback
        dma_slot_[_active_dma_slot].state = IMU_DMA_SLOT_FREE;
        filling_dma_slot_                 = IMU_DMA_INVALID_SLOT_IDX;
        status_.dma_transfer_error_count++;

        IMU_ACQ_RecoveryRequestedLocked((uint32_t)IMU_FAULT_DMA_TRANSFER);
    }

    IMU_ACQ_ExitCritical(_primask);
    return true;
}



bool
IMU_ACQ_ReadLatestSample(IMU_Sample_t *pOutSample)
{
    if (!pOutSample) {
        return false;
    }

    uint32_t   _primask = IMU_ACQ_EnterCritical();
    const bool _valid   = latest_sample_valid_;

    if (_valid) {
        *pOutSample = latest_sample_;
    }

    IMU_ACQ_ExitCritical(_primask);
    return _valid;
}



bool
IMU_ACQ_ReadNewSample(IMU_Sample_t *pOutSample, uint32_t *lastSequence)
{
    if (!pOutSample || !lastSequence) {
        return false;
    }

    IMU_Sample_t _imu_sample = {0};
    if (!IMU_ACQ_ReadLatestSample(&_imu_sample)) {
        return false;
    }

    if (_imu_sample.sequence == *lastSequence) {
        return false;
    }

    *pOutSample   = _imu_sample;
    *lastSequence = _imu_sample.sequence;

    return true;
}



IMU_ACQ_ProcessResult_t
IMU_ACQ_ProcessNextBatch(IMU_Sample_t *pOutSample)
{
    if (!initialized_ || !pOutSample) {
        return IMU_ACQ_PROCESS_NONE;
    }

    const IMU_ACQ_RecoveryResult_t _recovery_result = IMU_ACQ_RunRecoveryIfNeeded();

    switch (_recovery_result) {
        case IMU_ACQ_RECOVERY_NEEDED_BUT_WAIT_DMA:
            return IMU_ACQ_PROCESS_NONE;

        case IMU_ACQ_RECOVERY_FAILED:
            return IMU_ACQ_PROCESS_DROPPED;

        case IMU_ACQ_RECOVERY_DONE:
            return IMU_ACQ_PROCESS_DROPPED;

        case IMU_ACQ_RECOVERY_NOT_NEEDED:
        default:
            break; // Continue below with normal ready-slot processing
    }

    // Claim the oldest ready DMA batch to preserve acquisition order if both slots completed
    // before processing
    const uint8_t _oldest_ready_slot = IMU_ACQ_TakeOldestReadySlot();
    if (_oldest_ready_slot == IMU_DMA_INVALID_SLOT_IDX) {
        return IMU_ACQ_PROCESS_NONE;
    }

    // The PROCESSING state now gives the worker exclusive access to this slot's buffers and
    // metadata
    IMU_DMA_Slot_t *_p_dma_slot = &dma_slot_[_oldest_ready_slot];
    IMU_Sample_t    _imu_sample = {0};

    if (!IMU_ACQ_DecodeAndAverage(_p_dma_slot, &_imu_sample)) {
        uint32_t _primask = IMU_ACQ_EnterCritical();

        {
            status_.parse_error_count++;
            IMU_ACQ_RecoveryRequestedLocked((uint32_t)IMU_FAULT_FIFO_PARSE);
        }

        IMU_ACQ_ExitCritical(_primask);
        IMU_ACQ_ReleaseSlot(_oldest_ready_slot);
        return IMU_ACQ_PROCESS_DROPPED;
    }

    _imu_sample.irq_timestamp_us     = _p_dma_slot->irq_timestamp_us;
    _imu_sample.timestamp_us         = _p_dma_slot->irq_timestamp_us - IMU_BATCH_MIDPOINT_OFFSET_US;
    _imu_sample.publish_timestamp_us = IMU_ACQ_NowUs();

    bool _dt_valid = true;

    if (!previous_timestamp_valid_) {
        _imu_sample.dt_us         = IMU_OUTPUT_PERIOD_US;
        previous_timestamp_valid_ = true;
    }
    else {
        _imu_sample.dt_us = _imu_sample.timestamp_us - previous_timestamp_us_;

        if ((_imu_sample.dt_us < IMU_DT_MIN_US) || (_imu_sample.dt_us > IMU_DT_MAX_US)) {
            _dt_valid = false;
            status_.bad_dt_count++;
            IMU_ACQ_LatchFault(IMU_FAULT_BAD_DT);
        }
    }

    previous_timestamp_us_ = _imu_sample.timestamp_us;
    _imu_sample.dt_s       = (float)_imu_sample.dt_us * 0.000001f;
    _imu_sample.sequence   = ++published_sequence_;

    IMU_ACQ_UpdateDtStatus(_imu_sample.dt_us);

    _imu_sample.fault_flags = IMU_ACQ_TakePendingFaults();
    _imu_sample.healthy     = _dt_valid && (_imu_sample.fault_flags == IMU_FAULT_NONE);

    /**
     * Publish through two paths:
     *  - pOutSample returns the result synchronously to the real-time worker.
     *  - latest_sample_ is an atomic mailbox snapshot for telemetry, logging, or slower
     * consumers.
     */
    *pOutSample = _imu_sample;
    IMU_ACQ_Publish(&_imu_sample);
    status_.published_sample_count++;

    IMU_ACQ_ReleaseSlot(_oldest_ready_slot);
    return IMU_ACQ_PROCESS_PUBLISHED;
}



void
IMU_ACQ_GetStatus(IMU_ACQ_Status_t *pOutStatus)
{
    if (!pOutStatus) {
        return;
    }

    uint32_t _primask = IMU_ACQ_EnterCritical();
    *pOutStatus       = status_;
    IMU_ACQ_ExitCritical(_primask);
}



bool
IMU_ACQ_IsInitialized(void)
{
    return initialized_;
}



bool
IMU_ACQ_IsFatalStopped(void)
{
    return fatal_stopped_;
}