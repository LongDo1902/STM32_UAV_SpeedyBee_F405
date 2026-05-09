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
    if (logger.uart == NULL) {
        return LOG_ERROR;
    }

    char buffer[BUFFER_SIZE];

    va_list args;
    va_start(args, fmt);

    int16_t len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len < 0) {
        return LOG_ERROR;
    }

    buffer[len++] = '\n';

    return HAL_UART_Transmit(logger.uart, (uint8_t *)buffer, len, logger.timeout);
}
