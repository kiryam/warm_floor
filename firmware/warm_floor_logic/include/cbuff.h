#pragma once

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

typedef struct __CircualBuffer {
	 uint8_t * buffer;
	 size_t head;
	 size_t tail;
	 size_t size; //of the buffer

} CircualBuffer;



int circular_buf_reset(CircualBuffer * cbuf);
int circular_buf_put(CircualBuffer * cbuf, uint8_t data);
int circular_buf_get(CircualBuffer * cbuf, uint8_t * data);
bool circular_buf_empty(CircualBuffer cbuf);
bool circular_buf_full(CircualBuffer cbuf);
