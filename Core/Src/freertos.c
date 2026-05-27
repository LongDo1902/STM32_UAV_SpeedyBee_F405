/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "imu/icm42688_application.h"
#include "logging.h"
#include "spi.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
ICM42688_Handle_t                 icm42688_handle     = {0};
ICM42688_Offset_Raw_t             icm42688_offset_raw = {0};
ICM42688_Temp_Accel_Gyro_Scaled_t icm42688_scaled     = {0};
ICM42688_Est_Angle_complement_t   icm42688_est_angle  = {0};

osThreadId_t         IMUTask;
const osThreadAttr_t IMUTaskAttributes = {
    .name       = "IMUTask",
    .stack_size = 128 * 4,
    .priority   = osPriorityHigh7,
};

osThreadId_t         LoggingTask;
const osThreadAttr_t LoggingTaskAttributes = {
    .name       = "LoggingTask",
    .stack_size = 128 * 1,
    .priority   = osPriorityNormal,
};
/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartIMUTask(void *argument);
void StartLoggingTask(void *argument);
/* USER CODE END FunctionPrototypes */

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void
MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    IMUTask     = osThreadNew(StartIMUTask, NULL, &IMUTaskAttributes);
    LoggingTask = osThreadNew(StartLoggingTask, NULL, &LoggingTaskAttributes);

    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void
StartIMUTask(void *argument)
{
    icm42688_handle.spi_config.hspi    = &hspi1;
    icm42688_handle.spi_config.cs_port = GPIOA;
    icm42688_handle.spi_config.cs_pin  = GPIO_PIN_4;

    if (!ICM42688_Init(&icm42688_handle))
        return;
    osDelay(5);

    if (!ICM42688_Get_Calibrate_Raw(&icm42688_handle, &icm42688_offset_raw, 200))
        return;
    osDelay(5);

    for (;;) {
        if (!ICM42688_Get_Temp_Accel_Gyro_Scaled(&icm42688_handle, &icm42688_offset_raw,
                                                 &icm42688_scaled))
            return;

        if (!ICM42688_Get_Est_Angle_Complement(&icm42688_handle, IMU_ORIENT_NEGY_NEGX_NEGZ,
                                               &icm42688_scaled, &icm42688_est_angle, ))
            return;
    }
}

void
StartLoggingTask(void *argument)
{
    for (;;) {
    }
}
/* USER CODE END Application */
