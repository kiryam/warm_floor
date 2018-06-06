#if defined(STM32F10X)
#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_conf.h"


// Chip Unique ID on F103
#define U_ID_0 (*(uint32_t*)0x1FFFF7E8)
#define U_ID_1 (*(uint32_t*)0x1FFFF7EC)
#define U_ID_2 (*(uint32_t*)0x1FFFF7F0)

#define STM32F1

#include "target/board1/target.h"
#endif // STM32F10X

#if defined(STM32F40_41x)
#include "stm32f4xx.h"
#endif

#include "target/common.h"
