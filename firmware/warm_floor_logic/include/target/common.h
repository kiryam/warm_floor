#pragma once

#include <stdlib.h>
#include "_ansi.h"

#define UNUSED(x) ((void)(x))


typedef void (*callback) (void *);


#define POOL_SIZE 1024 * 20

//#define USE_UMM_MALLOC
//#define USE_TLSF

void memory_init();
void sleepMs(unsigned int);
_PTR malloc_c(size_t size);
void free_c(_PTR);
void SysTick_Init(void);
void TimeTick_Decrement(void);
uint32_t GetTick(void);
void delay_nus(u32 n);
void delay_nms(u32 n);
void delay_1ms(void);
int timeout_ms(u32 timeout, int *stop_if_raised);

int get_memory_allocated_total();
int fast_compare( const char *ptr0, const char *ptr1,unsigned int len );

//char* timeout_ms_value(u32 timeout, callback_char_pointer get_message);
