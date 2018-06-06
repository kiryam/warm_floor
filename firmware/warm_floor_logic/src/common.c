#include "platform.h"
#include <stdlib.h>
#include <string.h>


#ifdef USE_UMM_MALLOC
#include "umm_malloc.h"
#include "umm_malloc_cfg.h"
char umm_heap_pool[POOL_SIZE];
#endif


#ifdef USE_TLSF
#include "tlsf.h"
static char pool[POOL_SIZE];
#endif

static __IO uint32_t sysTickCounter;

void SysTick_Init(void) {
	/****************************************
	 *SystemFrequency/1000      1ms         *
	 *SystemFrequency/100000    10us        *
	 *SystemFrequency/1000000   1us         *
	 *****************************************/
	 while (SysTick_Config(SystemCoreClock / 1000) != 0) {
	 } // One SysTick interrupt now equals 1us



}

void delay_nus(u32 n) {
	//
	//sysTickCounter = n/10;
	//while (sysTickCounter != 0) {
	//}
}

void delay_nms(u32 n) {
	sysTickCounter = n;
	while (sysTickCounter != 0) {}
	//osDelay(n);
	//delay_nus(n*1000);
}

void delay_1ms(void) {
	delay_nms(1);
}

/*
int timeout_ms(u32 timeout, int *stop_if_raised) {
	sysTickCounter = timeout;
	while (sysTickCounter != 0) {
		if ( *stop_if_raised ){
			return 0;
		}
	}

	return 1;
}

char* timeout_ms_value(u32 timeout, callback_char_pointer get_message) {
	sysTickCounter = timeout;
	char* out;
	while (sysTickCounter != 0) {
		out = get_message();
		if (  out != NULL ){
			return out;
		}
	}

	return NULL;
}
*/

/**
 * This method needs to be called in the SysTick_Handler
 */
void TimeTick_Decrement(void) {
	if (sysTickCounter != 0x00) {
		sysTickCounter--;
	}
}

uint32_t GetTick(void) {
  return sysTickCounter;
}

void SysTick_Handler(void) {
	TimeTick_Decrement();
	osSystickHandler();
}


#ifdef USE_UMM_MALLOC
void Heap_Corruption_Handler(){
	Log_Message("Heap corrupted");
	return;
}
#endif
void memory_init(){
#ifdef USE_UMM_MALLOC
	 umm_init();
#endif
#ifdef USE_TLSF
	init_memory_pool(POOL_SIZE, pool);
#endif
}

#ifdef USE_UMM_MALLOC
_PTR malloc_c(size_t size) {
	return umm_malloc(size);
}
#endif

#ifdef USE_TLSF
_PTR malloc_c(size_t size) {
	return malloc_ex(size, pool);
}
#endif

#ifdef USE_UMM_MALLOC
_PTR realloc_c( void *ptr, size_t size) {
	return umm_realloc(ptr, size);
}
#endif


#ifdef USE_UMM_MALLOC
void free_c(_PTR p) {
	umm_free( p );
}
#endif

#ifdef USE_TLSF
void free_c(_PTR p) {
	free_ex(p, pool);
}
#endif


#ifdef USE_UMM_MALLOC
int get_memory_max(){
	return POOL_SIZE;
}
#endif

#ifdef USE_TLSF
int get_memory_max(){
	return get_max_size(pool);
}
#endif


// sum off all memory allocated
#ifdef USE_UMM_MALLOC
int get_memory_allocated_total(){
	return get_memory_max()-umm_free_heap_size();
}
#endif

#ifdef USE_TLSF
int get_memory_allocated_total(){
	return get_used_size(pool);
}
#endif


int fast_compare( const char *ptr0, const char *ptr1, unsigned int len ){
  unsigned int fast = len/sizeof(size_t) + 1;
  unsigned int offset = (fast-1)*sizeof(size_t);
  unsigned int current_block = 0;

  if( len <= sizeof(size_t)){ fast = 0; }

  size_t *lptr0 = (size_t*)ptr0;
  size_t *lptr1 = (size_t*)ptr1;

  while( current_block < fast ){
    if( (lptr0[current_block] ^ lptr1[current_block] )){
      int pos;
      for(pos = current_block*sizeof(size_t); pos < len ; ++pos ){
        if( (ptr0[pos] ^ ptr1[pos]) || (ptr0[pos] == 0) || (ptr1[pos] == 0) ){
          return  (int)((unsigned char)ptr0[pos] - (unsigned char)ptr1[pos]);
          }
        }
      }

    ++current_block;
    }

  while( len > offset ){
    if( (ptr0[offset] ^ ptr1[offset] )){
      return (int)((unsigned char)ptr0[offset] - (unsigned char)ptr1[offset]);
      }
    ++offset;
    }


  return 0;
 }
