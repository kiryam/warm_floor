#pragma once

#include "platform.h"

typedef struct __LED_dev {
	uint16_t          LED_PIN;
	GPIO_TypeDef*     LED_GPIOx;

} LED_dev;

void LED_Init(LED_dev* led);
void LED_Off(LED_dev* led);
void LED_On(LED_dev* led);
