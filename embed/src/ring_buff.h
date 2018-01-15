#ifndef RING_BUFF_H
#define RING_BUFF_H

#include <avr/io.h>

#define MAX_BUFFS 2  // maximum number of ring buffers supported by the array

// ring buffer index within the ring_buff.c file
// defined as a type to avoid type errors. it adds an extra layer of security as
// the compiler will check that the right type is passed to the functions.
typedef unsigned int rbIndex_t;

int ring_buffer_init(rbIndex_t *rbi, uint8_t numberOfElements, uint8_t *buffer);
int ring_buffer_put(rbIndex_t rbi, const uint8_t *data);
int ring_buffer_get(rbIndex_t rbi, uint8_t *data);

#endif