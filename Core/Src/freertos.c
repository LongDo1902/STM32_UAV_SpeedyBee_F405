/* USER CODE BEGIN Header */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dshot_task.h" // DshotMotorControlTask(void *argument);
#include "realtime/imu_acquisition/imu_acq.h"
#include "realtime/imu_acquisition/imu_sample.h"

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
static TaskHandle_t imu_task_handle_ = NULL; // Native FreeRTOS TaskHandle, the task itself is the same task

/* USER CODE END Variables */
/* Definitions for IMU_Task */
osThreadId_t         IMU_TaskHandle;
const osThreadAttr_t IMU_Task_attributes = {
    .name       = "IMU_Task",
    .stack_size = 1024 * 4,
    .priority   = (osPriority_t)osPriorityRealtime,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void IMU_Task_NotifyFromISR(void);

/* USER CODE END FunctionPrototypes */

void Start_IMU_Task(void *argument);

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
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* creation of IMU_Task */
    IMU_TaskHandle = osThreadNew(Start_IMU_Task, NULL, &IMU_Task_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    // dshotMotorTaskHandle = osThreadNew(DshotMotorControlTask, NULL, &dshotMotorTask_attributes);
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_Start_IMU_Task */
/**
 * @brief  Function implementing the IMU_Task thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_Start_IMU_Task */
void
Start_IMU_Task(void *argument)
{
    /* USER CODE BEGIN Start_IMU_Task */
    (void)argument;

    // Handle of 'IMU_Task'
    imu_task_handle_ = xTaskGetCurrentTaskHandle();

    // IMU Init
    if (!IMU_ACQ_Init(&imu_acq_config_)) {
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 sec
        }
    }

    IMU_Sample_t _imu_sample = {0};

    /* Infinite loop */
    for (;;) {
        // Sleep and wait until an ISR wakes this 'IMU_Task'
        // Or "if nobody has notified me, BLOCK this task forever" so processor can run other tasks
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Someone just called 'IMU_Task'
        // Drain everything before sleeping again
        for (;;) {
            IMU_ACQ_ProcessResult_t result = IMU_ACQ_ProcessNextBatch(&_imu_sample);

            if (result == IMU_ACQ_PROCESS_NONE) {
                break;
            }

            if (result == IMU_ACQ_PROCESS_PUBLISHED) {
                // Valid 4KHz output here
            }
        }
    }
    /* USER CODE END Start_IMU_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
static void
IMU_Task_NotifyFromISR(void)
{
    if (imu_task_handle_ == NULL) {
        return;
    }

    BaseType_t high_priority_task_woken = pdFALSE;

    vTaskNotifyGiveFromISR(imu_task_handle_, &high_priority_task_woken);

    portYIELD_FROM_ISR(high_priority_task_woken);
}

void
HAL_GPIO_EXTI_Callback(uint16_t gpioPin)
{
    IMU_ACQ_EXTI_Result_t result = IMU_ACQ_On_EXTI(gpioPin);
    // if result == IMU_ACQ_EXTI_DMA_STARTED, DON'T wake 'IMU_Task' because it must wait for DMA complete ISR

    if (result == IMU_ACQ_EXTI_RECOVERY_REQUESTED) {
        IMU_Task_NotifyFromISR();
    }
}

void
HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (IMU_ACQ_On_SPI_DMA_Complete(hspi)) {
        IMU_Task_NotifyFromISR();
    }
}

void
HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (IMU_ACQ_On_SPI_DMA_Error(hspi)) {
        IMU_Task_NotifyFromISR();
    }
}
/* USER CODE END Application */
