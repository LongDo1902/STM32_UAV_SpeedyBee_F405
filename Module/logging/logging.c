#include "logging.h"

static log_moudle_t logger;

bool
log_init(UART_HandleTypeDef *uart, uint16_t timeout)
{
    if (uart == NULL) {
        return false;
    }

    logger.uart    = uart;
    logger.timeout = timeout;

    return true;
}

log_status
log_write(const char *fmt, ...)
{
    if ((logger.uart == NULL) || (fmt == NULL)) {
        return LOG_ERROR;
    }

    char buffer[BUFFER_SIZE];

    va_list args;
    va_start(args, fmt);

    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len < 0) {
        return LOG_ERROR;
    }

    if ((size_t)len >= sizeof(buffer)) {
        len = (int)sizeof(buffer) - 1;
    }

    buffer[len++] = '\n';

    switch (HAL_UART_Transmit(logger.uart, (uint8_t *)buffer, (uint16_t)len, logger.timeout)) {
        case HAL_OK:
            return LOG_OK;
        case HAL_BUSY:
            return LOG_BUSY;
        case HAL_TIMEOUT:
            return LOG_TIMEOUT;
        case HAL_ERROR:
        default:
            return LOG_ERROR;
    }
}
