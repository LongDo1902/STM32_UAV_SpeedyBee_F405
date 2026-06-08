#include "stm32f4xx.h"

#include <stdint.h>
#include <stdio.h>

#define USE_PARAMETER_GROUPS

#define IOCFG_OUT_PP 0
#define IOCFG_OUT_OD 0
#define IOCFG_AF_PP 0
#define IOCFG_AF_OD 0
#define IOCFG_IPD 0
#define IOCFG_IPU 0
#define IOCFG_IN_FLOATING 0

#define U_ID_0 0
#define U_ID_1 1
#define U_ID_2 2

#define NOINLINE
#define FAST_CODE
#define FAST_CODE_NOINLINE
#define FAST_CODE_PREF
#define FAST_DATA_ZERO_INIT
#define FAST_DATA

#define PID_PROFILE_COUNT 4
#define CONTROL_RATE_PROFILE_COUNT 4
#define BATTERY_PROFILE_COUNT 3
#define USE_MAG
#define USE_BARO
#define USE_GPS
#define USE_DASHBOARD
#define USE_TELEMETRY
#define USE_LED_STRIP
#define USE_SERVOS
#define USE_TRANSPONDER

#ifndef LED_STRIP_MAX_LENGTH
#ifdef USE_LED_STRIP_64
#define LED_STRIP_MAX_LENGTH 64
#else
#define LED_STRIP_MAX_LENGTH 32
#endif
#endif // #ifndef LED_STRIP_MAX_LENGTH

struct spiResource_s;
struct quadSpiResource_s;
struct octoSpiResource_s;
struct i2cResource_s;

#define SPIDEV_COUNT 0
#define I2CDEV_COUNT 0
#define GYRO_COUNT 1

#define WS2811_DMA_TC_FLAG (void *)1
#define WS2811_DMA_HANDLER_IDENTIFER 0
#define NVIC_PriorityGroup_2 0x500
#define NVIC_PRIORITY_GROUPING NVIC_PriorityGroup_2
#define NVIC_BUILD_PRIORITY(base, sub)                                         \
  (((((base) << (4 - (7 - (NVIC_PRIORITY_GROUPING >> 8)))) |                   \
     ((sub) & (0x0f >> (7 - (NVIC_PRIORITY_GROUPING >> 8)))))                  \
    << 4) &                                                                    \
   0xf0)
#define NVIC_PRIORITY_BASE(prio)                                               \
  (((prio) >> (4 - (7 - (NVIC_PRIORITY_GROUPING >> 8)))) >> 4)
#define NVIC_PRIORITY_SUB(prio)                                                \
  (((prio) >> 4) & (0x0f >> (7 - (NVIC_PRIORITY_GROUPING >> 8))))

// #define MCU_TYPE_NAME "UNIT_TEST"
