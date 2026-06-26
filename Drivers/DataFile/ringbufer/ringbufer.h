#ifndef __RINGBUFER_H
#define __RINGBUFER_H
#include "sys.h"

typedef struct {
    uint8_t *buffer;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} RingBuffer;
#define RX_BUFFER_SIZE 512

void ring_buffer_init(RingBuffer *rb, uint8_t *buf, uint32_t size);
uint32_t ring_buffer_write(RingBuffer *rb, const uint8_t *data, uint32_t len);
uint32_t ring_buffer_read(RingBuffer *rb, uint8_t *data, uint32_t len);
uint32_t ring_buffer_space(RingBuffer *rb);
bool ring_buffer_get(RingBuffer *rb, uint8_t *byte);
#endif
