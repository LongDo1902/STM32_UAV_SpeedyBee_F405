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
#define IMU_SPI_DMA_XFER_BYTES  (IMU_SPI_DMA_CMD_BYTES + IMU_FIFO_WTM_BYTES) // 1 command byte + 32 dummy bytes which means IMU replies with 1st dummy byte and 32 useful bytes

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

static volatile bool     initialized_       = false;
static volatile uint32_t missed_exti_count_ = 0U;

/* ======================================================================================
 * Private and Public Prototypes
 * ====================================================================================== */



bool
IMU_ACQ_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    if (!hspi || !cs_port)
        return false;

    bool _status = false;

    // Initialize objects
    memset(&icm42688_handle_, 0, sizeof(icm42688_handle_));
    memset(&icm42688_offset_raw_, 0, sizeof(icm42688_offset_raw_));

    // Set SPI Handle + cs_pin
    icm42688_handle_.spi_config.hspi    = hspi;
    icm42688_handle_.spi_config.cs_port = cs_port;
    icm42688_handle_.spi_config.cs_pin  = cs_pin;

    // Call ICM42688_Init()
    CHECK_FOR(ICM42688_Init(&icm42688_handle_));

    // Calibrate Gyro/accel while drone is still
    CHECK_FOR(ICM42688_Get_Calibrate_Raw(&icm42688_handle_, &icm42688_offset_raw_, 200U));

    // Set FIFO WTM = 2 samples = 32 bytes
    CHECK_FOR(ICM42688_Set_FIFO_Watermark(&icm42688_handle_, IMU_FIFO_WTM_BYTES));

    // Set FIFO WTM repeat mode
    CHECK_FOR(ICM42688_Set_FIFO_WM_GT_THS(&icm42688_handle_, FIFO_WM_GREATER_THS_REPEAT));

    // Configure INT1 active high / push pull / pulsed
    CHECK_FOR(ICM42688_Set_Int1_Config(&icm42688_handle_, INT_ACTIVE_HIGH, INT_PUSH_PULL, INT_PULSED));

    // Enable FIFO Threshold interrupt
    CHECK_FOR(ICM42688_Set_Int1_FIFO_Threshold_Enable(&icm42688_handle_, true));

    // Flush FIFO
    CHECK_FOR(ICM42688_FIFO_Flush(&icm42688_handle_, true));

    return true;
}


void
IMU_ACQ_On_EXTI(uint16_t gpio_pin, uint32_t timestamp_us)
{
}
