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

static volatile uint8_t  active_dma_slot_  = IMU_DMA_INVALID_SLOT_INDEX;
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



/**
 * @brief   Marks one DMA buffer as free again after the firmware has finished using its data
 */
static void
IMU_ACQ_ReleaseSlot(uint8_t slotIndex)
{
    uint32_t primask           = IMU_ACQ_EnterCritical();
    dma_slot_[slotIndex].state = IMU_DMA_SLOT_FREE;
    IMU_ACQ_ExitCritical(primask);
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

    const uint8_t *fifo_byte   = &pDmaSlot->rx[IMU_SPI_DMA_CMD_BYTES]; // Pointer to the 1st-index-byte in DMA RX buffer
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

    memset(pImuOutSample, 0, sizeof(*pImuOutSample));

    for (uint8_t axis = 0; axis < 3U; axis++) {
        pImuOutSample->accel_g[axis]  = accel_sum[axis] / (float)IMU_FIFO_BATCH_SAMPLES;
        pImuOutSample->gyro_dps[axis] = gyro_sum[axis] / (float)IMU_FIFO_BATCH_SAMPLES;
    }

    pImuOutSample->temp_c = temp_sum / (float)IMU_FIFO_BATCH_SAMPLES;

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
IMU_ACQ_Init(const IMU_ACQ_Config_t *acq_config)
{
    if (!config || (!config->hspi) || (!config->cs_port) || (!config->htim_us) || (config->int1_gpio_pin == 0U)) {
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
    initialized_ = false;
}