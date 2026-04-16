#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <stdint.h>

typedef struct {
    uint8_t *rx_buffer_data;
    uint32_t rx_buffer_size; 
    volatile uint32_t rx_buffer_head;
    volatile uint32_t rx_buffer_tail;
}ring_buffer_data_t;

void ring_buffer_int();
bool ring_buffer_push(ring_buffer_data_t *ring_buffer, const char data);
bool ring_buffer_pop();

#endif // _RING_BUFFER_H_
