#ifndef ICM42688_TASK_H
#define ICM42688_TASK_H

#include "cmsis_os.h"

/**
 * @brief   Function declarations
 */
void ICM42688_Main_Task(void *argument);
void ICM42688_Logging_Task(void *argument);

#endif