/* USER CODE BEGIN Header */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "imu/icm42688_task.h"
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
/* USER CODE END Variables */
/* Definitions for ICM42688Task */
osThreadId_t         ICM42688TaskHandle;
const osThreadAttr_t ICM42688Task_attributes = {
    .name       = "ICM42688Task",
    .stack_size = 1024 * 4,
    .priority   = (osPriority_t)osPriorityHigh,
};
/* Definitions for ICM42688LogTask */
osThreadId_t         ICM42688LogTaskHandle;
const osThreadAttr_t ICM42688LogTask_attributes = {
    .name       = "ICM42688LogTask",
    .stack_size = 1024 * 4,
    .priority   = (osPriority_t)osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* USER CODE END FunctionPrototypes */

void Start_ICM42688Task(void *argument);
void Start_ICM42688LogTask(void *argument);

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
    /* creation of ICM42688Task */
    ICM42688TaskHandle = osThreadNew(Start_ICM42688Task, NULL, &ICM42688Task_attributes);

    /* creation of ICM42688LogTask */
    ICM42688LogTaskHandle = osThreadNew(Start_ICM42688LogTask, NULL, &ICM42688LogTask_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_Start_ICM42688Task */
/**
 * @brief  Function implementing the ICM42688Task thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_Start_ICM42688Task */
void
Start_ICM42688Task(void *argument)
{
    /* USER CODE BEGIN Start_ICM42688Task */
    ICM42688_Main_Task(argument);
    /* USER CODE END Start_ICM42688Task */
}

/* USER CODE BEGIN Header_Start_ICM42688LogTask */
/**
 * @brief Function implementing the ICM42688LogTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Start_ICM42688LogTask */
void
Start_ICM42688LogTask(void *argument)
{
    /* USER CODE BEGIN Start_ICM42688LogTask */
    ICM42688_Logging_Task(argument);
    /* USER CODE END Start_ICM42688LogTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */
