#include "imu_acq.h"
#include "imu/icm42688_application.h"
#include "imu_config.h"
#include "imu_sample.h"

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

typedef enum
{
    IMU_DMA_SLOT_FREE = 0,
    IMU_DMA_SLOT_ACTIVE,
    IMU_DMA_SLOT_READY,
    IMU_DMA_SLOT_PROCESSING,
} IMU_DMA_SlotState_t;

typedef struct
{
    uint8_t tx[IMU_SPI_DMA_XFER_BYTES];
    uint8_t rx[IMU_SPI_DMA_XFER_BYTES];

    volatile IMU_DMA_SlotState_t state;
    uint32_t                     irq_timestamp_us;
    uint8_t                      completion_order;
} IMU_DMA_Slot_t;

static ICM42688_Handle_t         icm42688_handle_;
static ICM42688_Offset_Raw_t     icm42688_offset_raw_;
static IMU_ACQ_Config_t          config_;
static IMU_Sample_t              last_sample_;
static volatile IMU_ACQ_Status_t status_;

static IMU_DMA_Slot_t dma_slot_[IMU_ACQ_DMA_SLOT_COUNT];

static volatile uint32_t pending_faults_ = IMU_FAULT_NONE;

/**
 * @brief   Store the current interrupt state (enabled/disabled) and temporarily disable IRQ
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

static uint32_t
IMU_ACQ_NowUs(void)
{
    return (uint32_t)__HAL_TIM_GET_COUNTER(config_.htim_us);
}

static void
IMU_ACQ_LatchFault(uint32_t fault)
{
    uint32_t _primask = IMU_ACQ_EnterCritical();
    pending_faults_ |= fault;
    IMU_ACQ_ExitCritical(_primask);
}

static uint32_t
IMU_ACQ_TakePendingFaults()
{
    uint32_t _primask      = IMU_ACQ_EnterCritical();
    uint32_t _copied_fault = pending_faults_;
    pending_faults_        = IMU_FAULT_NONE;
    IMU_ACQ_ExitCritical(_primask);
    return _copied_fault;
}

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

static bool
IMU_ACQ_Config_HighOdrInterruptTiming(void)
{
    return ICM42688_Set_INT_CONFIG1(&icm42688_handle_, false, true, true);
}

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
    uint8_t  selected_slot  = IMU_DMA_INVALID_SLOT_INDEX;
    uint32_t selected_order = UINT32_MAX;
    uint32_t primask        = IMU_ACQ_EnterCritical();

    for (uint8_t i = 0; i < IMU_ACQ_DMA_SLOT_COUNT; i++) {
        if ((dma_slot_[i].state == IMU_DMA_SLOT_READY) && (dma_slot_[i].completion_order < selected_order)) {
            selected_slot  = i;
            selected_order = dma_slot_[i].completion_order;
        }
    }

    if (selected_slot != IMU_DMA_INVALID_SLOT_INDEX) {
        dma_slot_[selected_slot].state = IMU_DMA_SLOT_PROCESSING;
    }

    IMU_ACQ_ExitCritical(primask);
    return selected_slot;
}



static void
IMU_ACQ_ReleaseSlot(uint8_t slotIndex)
{
    uint32_t primask           = IMU_ACQ_EnterCritical();
    dma_slot_[slotIndex].state = IMU_DMA_SLOT_FREE;
    IMU_ACQ_ExitCritical(primask);
}



/**
 * @brief
 */
static bool
IMU_ACQ_DecodeAndAverage(const IMU_DMA_Slot_t *dmaSlot, IMU_Sample_t *imuOutSample)
{
    if (!dmaSlot || !imuOutSample) {
        return false;
    }

    const uint8_t *fifo_byte   = &dmaSlot->rx[IMU_SPI_DMA_CMD_BYTES];
    uint16_t       parse_pos   = 0U;
    uint16_t       frame_count = 0U;

    float accel_sum[3] = {0.0f, 0.0f, 0.0f};
    float gyro_sum[3]  = {0.0f, 0.0f, 0.0f};
    float temp_sum     = 0.0f;

    while (parse_pos < IMU_FIFO_WTM_BYTES) {
        if (frame_count >= IMU_FIFO_BATCH_SAMPLES) {
            return false;
        }
        // Reset these objects every new loop
        ICM42688_FIFO_Frame_t             frame  = {0};
        ICM42688_Temp_Accel_Gyro_Scaled_t scaled = {0};

        if (!ICM42688_FIFO_Parse_One_Byte_Frame(&icm42688_handle_, &frame, fifo_byte, IMU_FIFO_WTM_BYTES, &parse_pos)) {
            return false;
        }

        if ((frame.packet_size != IMU_FIFO_PACKET_BYTES) || (!frame.accel_valid) || (!frame.gyro_valid) ||
            (!frame.temp_valid)) {
            return false;
        }

        if (!(ICM42688_Calibrate_FIFO_Frame(&icm42688_handle_, &frame, &icm42688_offset_raw_, &scaled))) {
            return false;
        }

        for (uint8_t axis = 0; axis < 3U; axis++) {
            gyro_sum[axis]  = scaled.gyro_dps[axis];
            accel_sum[axis] = scaled.accel_g[axis];
        }

        temp_sum += scaled.temp_c;
        frame_count++;
    }

    if ((frame_count != IMU_FIFO_BATCH_SAMPLES) || (parse_pos != IMU_FIFO_WTM_BYTES)) {
        return false;
    }

    memset(imuOutSample, 0, sizeof(*imuOutSample));

    for (uint8_t axis = 0; axis < 3U; axis++) {
        imuOutSample->accel_g[axis]  = accel_sum[axis] / (float)IMU_FIFO_BATCH_SAMPLES;
        imuOutSample->gyro_dps[axis] = gyro_sum[axis] / (float)IMU_FIFO_BATCH_SAMPLES;
    }

    imuOutSample->temp_c = temp_sum / (float)IMU_FIFO_BATCH_SAMPLES;

    return true;
}