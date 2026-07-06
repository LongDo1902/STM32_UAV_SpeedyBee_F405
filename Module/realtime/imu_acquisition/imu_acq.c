#include <string.h>

#include "imu/icm42688_application.h"
#include "imu_acq.h"
#include "imu_sample.h"

/* ======================================================================================
 * Global Defines and Declarations
 * ====================================================================================== */
#define IMU_ODR_HZ              8000U
#define IMU_FAST_LOOP_HZ        4000U
#define IMU_FIFO_PACKET_BYTES   16U
#define IMU_FIFO_BATCH_SAMPLES  (IMU_ODR_HZ / IMU_FAST_LOOP_HZ)
#define IMU_FIFO_WTM_BYTES      (IMU_FIFO_BATCH_SAMPLES * IMU_FIFO_PACKET_BYTES)

#define IMU_SPI_DMA_CMD_BYTES   1U
#define IMU_SPI_DMA_XFER_BYTES  (IMU_SPI_DMA_CMD_BYTES + IMU_FIFO_WTM_BYTES)
#define IMU_SAMPLE_PERIOD_US    (1000000U / IMU_ODR_HZ)

#if (IMU_ODR_HZ % IMU_FAST_LOOP_HZ) != 0U
#error "IMU_ODR_HZ must be divisible by IMU_FAST_LOOP_HZ"
#endif

#if IMU_FIFO_BATCH_SAMPLES != 2U
#error "Expected 2 FIFO samples per 4 kHz IMU acquisition event"
#endif

#if IMU_FIFO_WTM_BYTES != 32U
#error "Expected 32-byte FIFO watermark for 2 normal FIFO packets"
#endif

static ICM42688_Handle_t     icm42688_handle_;
static ICM42688_Offset_Raw_t icm42688_offset_raw_;
static IMU_Sample_t          latest_imu_sample_;
static uint8_t               fifo_dma_tx_[IMU_SPI_DMA_XFER_BYTES];
static uint8_t               fifo_dma_rx_[IMU_SPI_DMA_XFER_BYTES];

static volatile bool     initialized_            = false;
static volatile bool     fifo_dma_active_        = false;
static volatile uint32_t last_exti_timestamp_us_ = 0U;
static volatile uint32_t missed_exti_count_      = 0U;
static volatile uint32_t fifo_dma_error_count_   = 0U;
static volatile uint32_t fifo_parse_error_count_ = 0U;

/* ======================================================================================
 * Private and Public Prototypes
 * ====================================================================================== */


static uint32_t
IMU_ACQ_Frame_Timestamp(uint32_t batchTimestampUs, uint16_t frameIndex);

static void
IMU_ACQ_Fill_Sample(IMU_Sample_t *sample, const ICM42688_Temp_Accel_Gyro_Scaled_t *scaled, uint32_t timestampUs,
                    bool healthy);

static void
IMU_ACQ_Publish_Sample(const IMU_Sample_t *sample);

static void
IMU_ACQ_Publish_Unhealthy(uint32_t timestampUs);

static bool
IMU_ACQ_Decode_FIFO_DMA(IMU_Sample_t *outSample, uint32_t batchTimestampUs);


static uint32_t
IMU_ACQ_Frame_Timestamp(uint32_t batchTimestampUs, uint16_t frameIndex)
{
    uint32_t _samples_after_frame = (uint32_t)((IMU_FIFO_BATCH_SAMPLES - 1U) - frameIndex);
    uint32_t _age_us              = _samples_after_frame * IMU_SAMPLE_PERIOD_US;

    if (batchTimestampUs < _age_us)
        return 0U;

    return batchTimestampUs - _age_us;
}


static void
IMU_ACQ_Fill_Sample(IMU_Sample_t *sample, const ICM42688_Temp_Accel_Gyro_Scaled_t *scaled, uint32_t timestampUs,
                    bool healthy)
{
    if (!sample || !scaled)
        return;

    sample->timestamp = timestampUs;

    for (uint8_t _i = 0U; _i < 3U; _i++) {
        sample->gyro_dps[_i] = scaled->gyro_dps[_i];
        sample->accel_g[_i]  = scaled->accel_g[_i];
    }

    sample->temp_c     = scaled->temp_c;
    sample->healthy    = healthy;
    sample->new_sample = true;
}


static void
IMU_ACQ_Publish_Sample(const IMU_Sample_t *sample)
{
    if (!sample)
        return;

    latest_imu_sample_ = *sample;
}


static void
IMU_ACQ_Publish_Unhealthy(uint32_t timestampUs)
{
    latest_imu_sample_.timestamp  = timestampUs;
    latest_imu_sample_.healthy    = false;
    latest_imu_sample_.new_sample = true;
}


static bool
IMU_ACQ_Decode_FIFO_DMA(IMU_Sample_t *outSample, uint32_t batchTimestampUs)
{
    if (!outSample)
        return false;

    /* RX[0] is the dummy response to the FIFO_DATA command byte; FIFO payload starts at RX[1]. */
    const uint8_t *_fifo_bytes = &fifo_dma_rx_[IMU_SPI_DMA_CMD_BYTES];
    uint16_t       _parse_pos  = 0U;
    uint16_t       _frames     = 0U;
    IMU_Sample_t   _sample     = {0};

    while (_parse_pos < IMU_FIFO_WTM_BYTES) {
        ICM42688_FIFO_Frame_t             _frame  = {0};
        ICM42688_Temp_Accel_Gyro_Scaled_t _scaled = {0};

        if (!ICM42688_FIFO_Parse_One_Byte_Frame(&icm42688_handle_, &_frame, _fifo_bytes, IMU_FIFO_WTM_BYTES,
                                                &_parse_pos)) {
            return false;
        }

        if ((_frame.packet_size != IMU_FIFO_PACKET_BYTES) || !_frame.accel_valid || !_frame.gyro_valid ||
            !_frame.temp_valid) {
            return false;
        }

        if (!ICM42688_Calibrate_FIFO_Frame(&icm42688_handle_, &_frame, &icm42688_offset_raw_, &_scaled)) {
            return false;
        }

        /* The 4 kHz consumer publishes the newest sample from each 2-frame FIFO batch. */
        IMU_ACQ_Fill_Sample(&_sample, &_scaled, IMU_ACQ_Frame_Timestamp(batchTimestampUs, _frames), true);
        _frames++;
    }

    if ((_frames != IMU_FIFO_BATCH_SAMPLES) || (_parse_pos != IMU_FIFO_WTM_BYTES))
        return false;

    *outSample = _sample;

    return true;
}


bool
IMU_ACQ_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    if (!hspi || !cs_port)
        return false;

    bool _status = false;

    /* Reset acquisition state before configuring the shared ICM42688 driver handle. */
    memset(&icm42688_handle_, 0, sizeof(icm42688_handle_));
    memset(&icm42688_offset_raw_, 0, sizeof(icm42688_offset_raw_));
    memset(&latest_imu_sample_, 0, sizeof(latest_imu_sample_));
    memset(fifo_dma_tx_, 0, sizeof(fifo_dma_tx_));
    memset(fifo_dma_rx_, 0, sizeof(fifo_dma_rx_));

    initialized_            = false;
    fifo_dma_active_        = false;
    last_exti_timestamp_us_ = 0U;
    missed_exti_count_      = 0U;
    fifo_dma_error_count_   = 0U;
    fifo_parse_error_count_ = 0U;

    icm42688_handle_.spi_config.hspi    = hspi;
    icm42688_handle_.spi_config.cs_port = cs_port;
    icm42688_handle_.spi_config.cs_pin  = cs_pin;

    /* ICM42688_Init() sets FIFO byte-count stream mode and 8 kHz accel/gyro ODR. */
    CHECK_FOR(ICM42688_Init(&icm42688_handle_));

    /* Raw gyro/accel offsets are applied later to each decoded FIFO frame. */
    CHECK_FOR(ICM42688_Get_Calibrate_Raw(&icm42688_handle_, &icm42688_offset_raw_, 200U));

    /* Watermark is two normal 16-byte packets: 8 kHz sensor data consumed by a 4 kHz loop. */
    CHECK_FOR(ICM42688_Set_FIFO_Watermark(&icm42688_handle_, IMU_FIFO_WTM_BYTES));

    /* Repeat mode keeps INT1 pulsing whenever the FIFO rises above the watermark again. */
    CHECK_FOR(ICM42688_Set_FIFO_WM_GT_THS(&icm42688_handle_, FIFO_WM_GREATER_THS_REPEAT));

    CHECK_FOR(ICM42688_Set_Int1_Config(&icm42688_handle_, INT_ACTIVE_HIGH, INT_PUSH_PULL, INT_PULSED));

    CHECK_FOR(ICM42688_Set_Int1_FIFO_Threshold_Enable(&icm42688_handle_, true));

    /* Start from an empty FIFO so the first EXTI maps to a fresh two-sample batch. */
    CHECK_FOR(ICM42688_FIFO_Flush(&icm42688_handle_, true));

    initialized_ = true;

    return true;
}


void
IMU_ACQ_On_EXTI(uint16_t gpio_pin, uint32_t timestamp_us)
{
    (void)gpio_pin;

    if (!initialized_)
        return;

    if (fifo_dma_active_) {
        missed_exti_count_++;
        return;
    }

    last_exti_timestamp_us_ = timestamp_us;
    fifo_dma_active_        = true;

    /*
     * Burst is 33 bytes total: one command byte plus 32 dummy bytes, producing
     * one dummy RX byte followed by the FIFO payload.
     * autoBankSelect stays false because init ends with a bank-0 FIFO flush; this
     * avoids a blocking bank-select SPI write in the EXTI path.
     */
    if (!ICM42688_ReadRegs_DMA_Start(&icm42688_handle_, ICM42688_UB0_FIFO_DATA, fifo_dma_tx_,
                                     (uint16_t)sizeof(fifo_dma_tx_), fifo_dma_rx_, (uint16_t)sizeof(fifo_dma_rx_),
                                     IMU_FIFO_WTM_BYTES, false)) {
        fifo_dma_active_ = false;
        fifo_dma_error_count_++;
        IMU_ACQ_Publish_Unhealthy(timestamp_us);
    }
}


void
IMU_ACQ_On_SPI_DMA_Complete(SPI_HandleTypeDef *hspi)
{
    if (!initialized_ || !fifo_dma_active_ || (hspi != icm42688_handle_.spi_config.hspi))
        return;

    (void)ICM42688_DMA_End(&icm42688_handle_);
    fifo_dma_active_ = false;

    IMU_Sample_t _sample = {0};
    if (!IMU_ACQ_Decode_FIFO_DMA(&_sample, last_exti_timestamp_us_)) {
        fifo_parse_error_count_++;
        IMU_ACQ_Publish_Unhealthy(last_exti_timestamp_us_);
        return;
    }

    IMU_ACQ_Publish_Sample(&_sample);
}


void
IMU_ACQ_On_SPI_DMA_Error(SPI_HandleTypeDef *hspi)
{
    if (!initialized_ || (hspi != icm42688_handle_.spi_config.hspi))
        return;

    if (fifo_dma_active_) {
        (void)ICM42688_DMA_End(&icm42688_handle_);
    }

    fifo_dma_active_ = false;
    fifo_dma_error_count_++;
    IMU_ACQ_Publish_Unhealthy(last_exti_timestamp_us_);
}


bool
IMU_ACQ_GetLatestSample(IMU_Sample_t *out_sample)
{
    if (!out_sample)
        return false;

    uint32_t _primask = __get_PRIMASK();
    __disable_irq();

    *out_sample = latest_imu_sample_;
    latest_imu_sample_.new_sample = false;

    __set_PRIMASK(_primask);

    return out_sample->healthy;
}
