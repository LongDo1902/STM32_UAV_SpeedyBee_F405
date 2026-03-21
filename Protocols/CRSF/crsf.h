
#include "crsf_parser.h"

#ifndef _CRSF_H_
#define _CRSF_H_

void crsf_init(UART_HandleTypeDef *huart);
void crsf_update(void);
uint16_t crsf_get_channel(uint8_t ch);

#endif // _CRSF_H_
