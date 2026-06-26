#include "ringbufer.h"
 
 
void ring_buffer_init(RingBuffer *rb, uint8_t *buf, uint32_t size)
{
	  memset(buf,0,size);
	
    rb->buffer = buf;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

uint32_t ring_buffer_write(RingBuffer *rb, const uint8_t *data, uint32_t len)
{
    uint32_t bytes_to_write = len;
    if(bytes_to_write > (rb->size - rb->count)) {
        bytes_to_write = rb->size - rb->count;
    }
    
    for(uint32_t i = 0; i < bytes_to_write; i++) {
        rb->buffer[rb->head] = data[i];
        rb->head = (rb->head + 1) % rb->size;
    }
    
    rb->count += bytes_to_write;
    return bytes_to_write;
}

uint32_t ring_buffer_read(RingBuffer *rb, uint8_t *data, uint32_t len)
{
    uint32_t bytes_to_read = len;
    if(bytes_to_read > rb->count) {
        bytes_to_read = rb->count;
    }
    
    for(uint32_t i = 0; i < bytes_to_read; i++) {
        data[i] = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % rb->size;
    }
    
    rb->count -= bytes_to_read;
    return bytes_to_read;
}

bool ring_buffer_get(RingBuffer *rb, uint8_t *byte) {
    if (rb->count == 0) {
        return false;
    }
    
    *byte = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RX_BUFFER_SIZE;
    rb->count--;
    
    return true;
}


uint32_t ring_buffer_space(RingBuffer *rb)
{
    return rb->size - rb->count;
}
