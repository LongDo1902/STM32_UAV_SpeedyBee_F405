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
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
static int task_count_debug = 0;

osThreadId_t defaultTestTaskHandle_02;
const osThreadAttr_t defaultTestTask_attributes_02 = {
  .name = "defaultTestTask_02",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal7,
};
/* USER CODE END Variables */
/* Definitions for defaultTestTask */
osThreadId_t defaultTestTaskHandle;
const osThreadAttr_t defaultTestTask_attributes = {
  .name = "defaultTestTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartDefaultTestTask_02(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTestTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
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
  /* creation of defaultTestTask */
  defaultTestTaskHandle = osThreadNew(StartDefaultTestTask, NULL, &defaultTestTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  defaultTestTaskHandle_02 = osThreadNew(StartDefaultTestTask_02, NULL, &defaultTestTask_attributes_02);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTestTask */
/**
 * @brief  Function implementing the defaultTestTask thread.
 * @param  argument: Not used
 * @retval None
 */

void StartDefaultTestTask_02(void *argument)
{
  /* USER CODE BEGIN StartDefaultTestTask */
    /* Infinite loop */
	while(1) {
		 task_count_debug = !task_count_debug;
		 osDelay(1000);
	}
  /* USER CODE END StartDefaultTestTask */
}

/* USER CODE END Header_StartDefaultTestTask */
void StartDefaultTestTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTestTask */
    /* Infinite loop */
	ICM42688_main();
  /* USER CODE END StartDefaultTestTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

