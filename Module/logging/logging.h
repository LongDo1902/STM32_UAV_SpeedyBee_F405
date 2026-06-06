#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"

#define BUFFER_SIZE 256

typedef struct
{
    uint16_t            timeout;
    UART_HandleTypeDef *uart;
} log_moudle_t;

typedef enum
{
    LOG_OK,
    LOG_ERROR,
    LOG_BUSY,
    LOG_TIMEOUT
} log_status;

bool       log_init(UART_HandleTypeDef *uart, uint16_t timeout);
log_status log_write(const char *fmt, ...);

#endif // _LOGGING_H_
