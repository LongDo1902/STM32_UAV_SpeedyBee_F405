#pragma once

#include "cmsis_os.h"
#include "dshot.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

#define DSHOT_THROTTLE_MIN  48
#define DSHOT_THROTTLE_MAX  2047

void DshotMotorControlTask(void *argument);
