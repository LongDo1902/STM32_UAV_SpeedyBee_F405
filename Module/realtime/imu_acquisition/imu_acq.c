#include "imu_acq.h"
#include "imu/icm42688_application.h"
#include "imu_config.h"
#include "imu_sample.h"

#include <string.h>

/**
 * @todo
 * WTM interrupts occur correctly
 * SPI DMA completes before the next event
 * Interrupts are not blocked too long
 * Parsing completes in time
 * No DMA errors occur
 * No EXTI events are missed
 * The consumer reads every published result
 * Consider to use STM32 hardware timer -> it tells the real time between events
 * ICM42688 Interrupt correction (INT_CONFIG1 such as INT_TPULSE_DURATION = 1, INT_TDEASSERT_DISABLE = 1,
 * INT_ASYNC_RESET = 0) -> required by ICM42688 datasheet for ODR > 4kHz
 */



/**
 * @brief   State flags of DMA slot
 *          They help the program to know which operation/action should be execute next such as probing status, waiting,
 *          processing, releasing, acquiring
 */
typedef enum
{
    IMU_DMA_SLOT_FREE = 0,
    IMU_DMA_SLOT_ACTIVE,
    IMU_DMA_SLOT_READY,
    IMU_DMA_SLOT_PROCESSING,
} IMU_DMA_SlotState_t;



/**
 * @brief   A struct holder of all DMA slot RAM storages and properties
 */
typedef struct
{
    // Including TX/RX RAM buffer for DMA transactions
    uint8_t tx[IMU_SPI_DMA_XFER_BYTES];
    uint8_t rx[IMU_SPI_DMA_XFER_BYTES];

    // DMA properties
    volatile IMU_DMA_SlotState_t state;
    uint32_t                     irq_timestamp_us;
    uint8_t                      completion_order;
} IMU_DMA_Slot_t;



/**
 * @brief   Global struct declarations and initializations
 */
static ICM42688_Handle_t     icm42688_handle_;
static ICM42688_Offset_Raw_t icm42688_offset_raw_;

static IMU_ACQ_Config_t          acq_config_;
static IMU_DMA_Slot_t            dma_slot_[IMU_ACQ_DMA_SLOT_COUNT];
static IMU_Sample_t              latest_sample_;
static volatile IMU_ACQ_Status_t status_;



/**
 * @brief   Global variable declarations
 */
static bool initialized_ = false;

static volatile uint8_t active_dma_slot_ =
    IMU_DMA_INVALID_SLOT_INDEX; // Remember which DMA slot is being filled by SPI DMA
static volatile uint8_t  completion_order_ = 0U;
static volatile uint32_t pending_faults_   = IMU_FAULT_NONE;

static bool     previous_timestamp_valid_ = false;
static uint32_t previous_timestamp_us_    = 0U;

static bool latest_sample_valid_ = false;



/**
 * @brief   Store the current interrupt state (enabled/disabled) and temporarily disable IRQ
 *          This function helps the program not to cross overwrite a variable during the runtime
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
 * @brief   Restore the saved interrupt state
 */
static void
IMU_ACQ_ExitCritical(uint32_t primask)
{
    __DMB();
    __set_PRIMASK(primask);
}



/**
 * @brief   Get the instance timestamp from timer peripheral right at the moment when function is executed
 */
static uint32_t
IMU_ACQ_NowUs(void)
{
    return (uint32_t)__HAL_TIM_GET_COUNTER(acq_config_.htim_us);
}



/**
 * @brief   Records a fatal flag/status safely into a global fault flag
 */
static void
IMU_ACQ_LatchFault(uint32_t fault)
{
    uint32_t _primask = IMU_ACQ_EnterCritical();
    pending_faults_ |= fault;
    IMU_ACQ_ExitCritical(_primask);
}



/**
 * @brief   Collect all IMU faults that happended, return them to the caller and then clear the stored fault list
 *          So, the caller handles the old faults while @c pending_faults_ is ready to collect the new ones.
 */
static uint32_t
IMU_ACQ_TakePendingFaults()
{
    uint32_t _primask      = IMU_ACQ_EnterCritical();
    uint32_t _copied_fault = pending_faults_;
    pending_faults_        = IMU_FAULT_NONE;
    IMU_ACQ_ExitCritical(_primask);
    return _copied_fault;
}



/**
 * @brief   Measures the gyro's small zero error while drone is not moving
 *          Then, stores that error so later gyro readings can be corrected
 */
static bool
IMU_ACQ_CalibrateGyroBias(uint16_t sampleCounts)
{
    if (sampleCounts == 0U || sampleCounts > 10000U)
        return false;

    int32_t _gyro_sum[3] = {0, 0, 0};

    for (uint16_t i = 0U; i < sampleCounts; i++) {
        ICM42688_Raw_t _raw = {0}; // Reset for every new sample count hit

        if (!ICM42688_Get_Temp_Accel_Gyro_Raw(&icm42688_handle_, &_raw)) {
            return false;
        }
        for (uint8_t axis = 0U; axis < 3U; axis++) {
            _gyro_sum[axis] += _raw.raw_gyro[axis];
        }
        // Startup-only path, Small delay avoid averaging the same register sample repeatedly
        HAL_Delay(1U);
    }

    memset(&icm42688_offset_raw_, 0, sizeof(icm42688_offset_raw_));
    for (uint8_t axis = 0U; axis < 3U; axis++) {
        icm42688_offset_raw_.offset_raw_gyro[axis] = (int32_t)(_gyro_sum[axis] / (int32_t)sampleCounts);
    }

    /**
     * @todo    Accel startup bias is intentionally left at zero. Estimating it correctly requires known board
     * orientation and proper six-position calibration.
     */
    return true;
}



/**
 * @brief   Configures ICM42688 Interrupt pin timing so it works reliably at high sample rate
 */
static bool
IMU_ACQ_Config_HighOdrInterruptTiming(void)
{
    return ICM42688_Set_INT_CONFIG1(&icm42688_handle_, false, true, true);
}



/**
 * @brief   Searches for DMA buffers and return the index of the first one that is free
 */
static uint8_t
IMU_ACQ_FindFreeSlotLocked(void)
{
    for (uint8_t i = 0; i < IMU_ACQ_DMA_SLOT_COUNT; i++) {
        if (dma_slot_[i].state == IMU_DMA_SLOT_FREE) {
            return i;
        }
    }
    return IMU_DMA_INVALID_SLOT_INDEX;
}



/**
 * @brief   Choose the oldest completed DMA buffer that is waiting to be processed
 */
static uint8_t
IMU_ACQ_TakeOldestReadySlot(void)
{
    uint8_t  _selected_slot  = IMU_DMA_INVALID_SLOT_INDEX;
    uint32_t _selected_order = UINT32_MAX;
    uint32_t _primask        = IMU_ACQ_EnterCritical();

    for (uint8_t i = 0; i < IMU_ACQ_DMA_SLOT_COUNT; i++) {
        if ((dma_slot_[i].state == IMU_DMA_SLOT_READY) && (dma_slot_[i].completion_order < _selected_order)) {
            _selected_slot  = i;
            _selected_order = dma_slot_[i].completion_order;
        }
    }

    if (_selected_slot != IMU_DMA_INVALID_SLOT_INDEX) {
        dma_slot_[_selected_slot].state = IMU_DMA_SLOT_PROCESSING;
    }

    IMU_ACQ_ExitCritical(_primask);
    return _selected_slot;
}



/**
 * @brief   Marks one DMA buffer as free again after the firmware has finished using its data
 */
static void
IMU_ACQ_ReleaseSlot(uint8_t slotIndex)
{
    uint32_t _primask          = IMU_ACQ_EnterCritical();
    dma_slot_[slotIndex].state = IMU_DMA_SLOT_FREE;
    IMU_ACQ_ExitCritical(_primask);
}



/**
 * @brief   Read all FIFO frames from one completed DMA RX buffer, calibrate each frame , average them, produce one
 *          final IMU sample
 * @param   pDmaSlot        Pointer to DMA Slot struct of IMU
 * @param   pImuOutSample   Pointer to IMU output sample holder/struct.
 */
static bool
IMU_ACQ_DecodeAndAverage(const IMU_DMA_Slot_t *pDmaSlot, IMU_Sample_t *pImuOutSample)
{
    if (!pDmaSlot || !pImuOutSample) {
        return false;
    }

    const uint8_t *_p_fifo_byte = &pDmaSlot->rx[IMU_SPI_DMA_CMD_BYTES]; // Pointer to the 1st-index-byte DMA RX buffer
    uint16_t       _parse_pos   = 0U;
    uint16_t       _frame_count = 0U;

    float _accel_sum[3] = {0.0f, 0.0f, 0.0f};
    float _gyro_sum[3]  = {0.0f, 0.0f, 0.0f};
    float _temp_sum     = 0.0f;

    while (_parse_pos < IMU_FIFO_WTM_BYTES) {
        if (_frame_count >= IMU_FIFO_BATCH_SAMPLES) {
            return false;
        }
        // Reset these objects every new loop
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
            _gyro_sum[axis]  = _scaled.gyro_dps[axis];
            _accel_sum[axis] = _scaled.accel_g[axis];
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
 * @brief   Update the interval time dt (microsecond)
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
 * @brief
 */
static void
IMU_ACQ_Publish(const IMU_Sample_t *pImuSample)
{
    uint32_t _primask    = IMU_ACQ_EnterCritical();
    latest_sample_       = *pImuSample;
    latest_sample_valid_ = true;
    IMU_ACQ_ExitCritical(_primask);
}



/**
 * @brief
 */
bool
IMU_ACQ_Init(const IMU_ACQ_Config_t *pAcqConfig)
{
    if (!pAcqConfig || (!pAcqConfig->hspi) || (!pAcqConfig->cs_port) || (!pAcqConfig->htim_us) ||
        (pAcqConfig->int1_gpio_pin == 0U)) {
        return false;
    }

    // Reset all structs
    memset(&icm42688_handle_, 0, sizeof(icm42688_handle_));
    memset(&icm42688_offset_raw_, 0, sizeof(icm42688_offset_raw_));
    memset(&acq_config_, 0, sizeof(acq_config_));
    memset(dma_slot_, 0, sizeof(dma_slot_));
    memset(&latest_sample_, 0, sizeof(latest_sample_));
    memset((void *)&status_, 0, sizeof(status_));

    // Reset all global parameters
    initialized_              = false;
    active_dma_slot_          = IMU_DMA_INVALID_SLOT_INDEX;
    completion_order_         = 0U;
    pending_faults_           = IMU_FAULT_NONE;
    previous_timestamp_valid_ = false;
    previous_timestamp_us_    = 0U;
    latest_sample_valid_      = false;

    // Free DMA slot
    for (uint8_t i = 0U; i < IMU_ACQ_DMA_SLOT_COUNT; i++) {
        dma_slot_[i].state = IMU_DMA_SLOT_FREE;
    }

    __HAL_TIM_SET_COUNTER(acq_config_.htim_us, 0U);          // Reset timer counter
    if (HAL_TIM_Base_Start(acq_config_.htim_us) != HAL_OK) { // Start timer counter
        return false;
    }

    icm42688_handle_.spi_config.hspi    = acq_config_.hspi;
    icm42688_handle_.spi_config.cs_port = acq_config_.cs_port;
    icm42688_handle_.spi_config.cs_pin  = acq_config_.cs_pin;

    if (!ICM42688_Init(&icm42688_handle_)) {
        return false;
    }

    if (!IMU_ACQ_CalibrateGyroBias(IMU_GYRO_CALIBRATION_SAMPLE_COUNT)) {
        return false;
    }

    // Configure every INT timing field before routing FIFO threshold to INT1
    if (!ICM42688_Set_Int1_FIFO_Threshold_Enable(&icm42688_handle_, false) ||
        !ICM42688_Set_Int1_DataReady_Enable(&icm42688_handle_, false) ||
        !ICM42688_Set_Int1_FIFO_Full_Enable(&icm42688_handle_, false) ||
        !ICM42688_Set_Int1_ResetDone_Enable(&icm42688_handle_, false) || !IMU_ACQ_Config_HighOdrInterruptTiming() ||
        !ICM42688_Set_Int1_Config(&icm42688_handle_, INT_ACTIVE_HIGH, INT_PUSH_PULL, INT_PULSED) ||
        !ICM42688_Set_FIFO_Watermark(&icm42688_handle_, IMU_FIFO_WTM_BYTES) ||
        !ICM42688_Set_FIFO_WM_GT_THS(&icm42688_handle_, FIFO_WM_GREATER_THS_REPEAT) ||
        !ICM42688_FIFO_Flush(&icm42688_handle_, true) ||
        !ICM42688_Set_Int1_FIFO_Threshold_Enable(&icm42688_handle_, true)) {
        return false;
    }

    initialized_ = true;
    return true;
}



/**
 * @brief   Handle ICM42688 INT1, usually caused by FIFO reaching its watermark
 */
bool
IMU_ACQ_On_EXTI(uint16_t gpio_pin)
{
    if (!initialized_ || gpio_pin != acq_config_.int1_gpio_pin) {
        return false;
    }

    // Capture time before any bookkeeping to minimize timestamp jitter
    const uint32_t _irq_timestamp_us = IMU_ACQ_NowUs();
    status_.exti_count++;

    uint32_t _primask = IMU_ACQ_EnterCritical();

    // Check if prev DMA transfer still using this slot
    if (active_dma_slot_ != IMU_DMA_INVALID_SLOT_INDEX) {
        // e.g active_dma_slot = 1, DMA is receiving new data
        status_.exti_while_dma_active_count++;
        pending_faults_ |= IMU_FAULT_EXTI_WHILE_DMA;
        IMU_ACQ_ExitCritical(_primask);
        return false;
    }

    // Find free slot
    const uint8_t _free_slot_index = IMU_ACQ_FindFreeSlotLocked();
    if (_free_slot_index == IMU_DMA_INVALID_SLOT_INDEX) {
        status_.no_free_dma_slot_count++;
        pending_faults_ |= IMU_FAULT_NO_FREE_DMA_SLOT;
        IMU_ACQ_ExitCritical(_primask);
        return false;
    }

    dma_slot_[_free_slot_index].state            = IMU_DMA_SLOT_ACTIVE;
    dma_slot_[_free_slot_index].irq_timestamp_us = _irq_timestamp_us;
    active_dma_slot_                             = _free_slot_index;

    IMU_ACQ_ExitCritical(_primask);

    IMU_DMA_Slot_t *slot = &dma_slot_[_free_slot_index];

    /**
     * @note    @c autoBankSelect is false to avoid a blocking SPI write in EXTI context.
     *          This module therefore owns the ICM42688 bus after initialization and keeps the device in user bank 0
     *          during real-time acquisition
     */
    if (!ICM42688_ReadRegs_DMA_Start(&icm42688_handle_, ICM42688_UB0_FIFO_DATA, slot->tx, (uint16_t)sizeof(slot->tx),
                                     slot->rx, (uint16_t)sizeof(slot->rx), IMU_FIFO_WTM_BYTES, false)) {

        _primask         = IMU_ACQ_EnterCritical();
        slot->state      = IMU_DMA_SLOT_FREE;
        active_dma_slot_ = IMU_DMA_INVALID_SLOT_INDEX;
        status_.dma_start_error_count++;
        pending_faults_ |= IMU_FAULT_DMA_START;
        IMU_ACQ_ExitCritical(_primask);
        return false;
    }

    status_.dma_start_count++;
    return true;
}

bool
IMU_ACQ_On_SPI_DMA_Complete(SPI_HandleTypeDef *hspi)
{
    if (!initialized_ || (hspi != acq_config_.hspi)) {
        return false;
    }

    uint32_t      _primask         = IMU_ACQ_EnterCritical();
    const uint8_t _active_dma_slot = active_dma_slot_;
    IMU_ACQ_ExitCritical(_primask);

    if (_active_dma_slot == IMU_DMA_INVALID_SLOT_INDEX) {
        return false;
    }

    return true;
}