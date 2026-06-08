#include "imu/icm42688_task.h"
#include "imu/icm42688_application.h"
#include "logging.h"

#include "main.h"
#include "spi.h"


extern osThreadId_t ICM42688TaskHandle; // Reuse @p ICM42688TaskHandle from freertoc.c



/**
 * @brief   Defines:
 *          IMU Output Data Rate
 *          Maximum FreeRTOS operation frequency
 *          FIFO configurations
 *          IMU FIFO ready flag
 */
#define IMU_ODR             8000U
#define FREERTOS_CTRL_FREQ  1000U

#define FIFO_PACKET_BYTE        16U // You have to change this when ICM42688 HIRES is enabled
#define FIFO_SAMPLES_PER_BATCH  (IMU_ODR / FREERTOS_CTRL_FREQ)
#define FIFO_WTM_BYTE           (FIFO_SAMPLES_PER_BATCH * FIFO_PACKET_BYTE)
#define FIFO_BUFFER_SIZE        (FIFO_WTM_BYTE * 2U) // Twice FIFO watermark level, just in case the FIFO is still above watermark after the first trigger

#define IMU_FLAG_FIFO_READY     (1U << 0U)



/**
 * @brief   Global variables for storing essential IMU's configurations and measured data
 *          All are initialized to zeros
 */
static ICM42688_Handle_t               icm42688_handle_     = {0};
static ICM42688_Offset_Raw_t           icm42688_offset_raw_ = {0};
static ICM42688_Est_Angle_complement_t icm42688_est_angle_  = {0};

static uint8_t                           icm42688_fifo_raw_buf_[FIFO_BUFFER_SIZE] = {0};
static ICM42688_FIFO_Frame_t             icm42688_frame_                          = {0};
static ICM42688_Temp_Accel_Gyro_Scaled_t icm42688_fifo_scaled_                    = {0};



/**
 * @todo
 * ICM42688_ODR:        4 kHz or 8 kHz(done)
 * IMU_FIFO:            enabled (done)
 * IMU interrupt:       data ready / FIFO (done)
 * watermark: (done)
 * FreeRTOS IMU task:   wakes
 * from notification SPI read: burst-read FIFO
 * Attitude/control:    1 kHz or 2 kHz
 */


/**
 * @brief
 */
void
HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == IMU_INT_Pin) {
        if (ICM42688TaskHandle != NULL) {
            osThreadFlagsSet(ICM42688TaskHandle, IMU_FLAG_FIFO_READY);
        }
    }
}



/**
 * @brief   Main ICM42688 executation task function
 *              * ICM42688 Initialization
 *                  + Check if ICM42688 is alive/active
 *                  + Reset sensor for fresh startup
 *                  + SPI interface configuration
 *                  + Accel configuration
 *                  + Gyro configuration
 *                  + Temperature configuration
 *                  + Basic FIFO configuration
 *              * Configure FIFO so it can be like a temporary buffer storing a fix IMU samples (aka watermark) during
 *                system is busy processing another tasks
 *              * Enables interrupt INT1 when FIFO buffer reach predefined watermark level so system can process
 *                immediately
 */
void
ICM42688_Main_Task(void *argument)
{
    icm42688_handle_.spi_config.hspi    = &hspi1;
    icm42688_handle_.spi_config.cs_port = GPIOA;
    icm42688_handle_.spi_config.cs_pin  = GPIO_PIN_4;

    // Initialize some essential ICM42688 configurations
    if (!ICM42688_Init(&icm42688_handle_)) {
        for (;;) {
            // Add a logging error here
            osDelay(1000);
        }
    }

    // Get the offset raw data for later calibration
    if (!ICM42688_Get_Calibrate_Raw(&icm42688_handle_, &icm42688_offset_raw_, 200)) {
        for (;;) {
            // Add a logging error here
            osDelay(1000);
        }
    }

    // Configure FIFO watermark
    if (!ICM42688_Set_FIFO_Watermark(&icm42688_handle_, FIFO_WTM_BYTE)) {
        for (;;) {
            // Add a logging error here
            osDelay(1000);
        }
    }

    // Configure FIFO watermark interrupt to be repeated mode, so that the interrupt will fire again if the FIFO buffer
    // is still above wtm
    if (!ICM42688_Set_FIFO_WM_GT_THS(&icm42688_handle_, FIFO_WM_GREATER_THS_REPEAT)) {
        for (;;) {
            // Add a logging error here
            osDelay(1000);
        }
    }

    // Configure the interrupt 1
    if (!ICM42688_Set_Int1_Config(&icm42688_handle_, INT_ACTIVE_HIGH, INT_PUSH_PULL, INT_PULSED)) {
        for (;;) {
            // Add a logging error here
            osDelay(1000);
        }
    }

    // Enable FIFO watermark interrupt on interrupt 1
    if (!ICM42688_Set_Int1_FIFO_Threshold_Enable(&icm42688_handle_, true)) {
        for (;;) {
            // Add a logging error here
            osDelay(1000);
        }
    }

    osDelay(10);

    const float _dt = 1.0f / (float)IMU_ODR;

    for (;;) {
        uint32_t _flags = osThreadFlagsWait(IMU_FLAG_FIFO_READY, osFlagsWaitAny, osWaitForever);
        if ((_flags & osFlagsError)) {
            continue; // Jump back to the nearest osThreadFlagWait if there is an error
        }

        /**
         * @brief   1. Read all raw FIFO data into a buffer by burst read
         *          2. Parse the raw FIFO data in the buffer and extract each FIFO frame
         *          3. Calibrate each FIFO frame and get the scaled data in physical unit
         *          4. Compute estimated angle with complementary filter
         */
        if (!ICM42688_Get_FIFO_Frame_In_Byte(&icm42688_handle_, icm42688_fifo_raw_buf_, FIFO_BUFFER_SIZE)) {
            // Add a logging error here
            continue;
        }

        uint16_t _fifo_count_bytes =
            icm42688_handle_.fifo_config.fifo_count; // Actual number of FIFO byte that is available and waiting to read
        uint16_t _current_pos = 0U;                  // Current point/index in the raw FIFO buffer that is being parsed

        while (_current_pos < _fifo_count_bytes) {
            if (!ICM42688_FIFO_Parse_One_Byte_Frame(&icm42688_handle_, &icm42688_frame_, icm42688_fifo_raw_buf_,
                                                    _fifo_count_bytes, &_current_pos)) {
                // Add a logging error here
                break;
            }

            if (ICM42688_Calibrate_FIFO_Frame(&icm42688_handle_, &icm42688_frame_, &icm42688_offset_raw_,
                                              &icm42688_fifo_scaled_)) {
                (void)ICM42688_Get_Est_Angle_Complement(&icm42688_handle_, IMU_ORIENT_NEGY_NEGX_NEGZ,
                                                        &icm42688_fifo_scaled_, &icm42688_est_angle_, _dt);
            }
        }
    }
}



void
ICM42688_Logging_Task(void *argument)
{
    for (;;) {
        osDelay(1000);
    }
}
