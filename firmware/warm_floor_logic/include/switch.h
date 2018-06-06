#pragma once

#include "platform.h"
#include "stdbool.h"

typedef struct __Switch_dev {
	uint16_t          Switch_PIN;
	GPIO_TypeDef*     Switch_GPIOx;
	bool              start_on;
	bool              is_on;
} Switch_dev;

void Switch_Init(Switch_dev* swtch);
void Switch_Off(Switch_dev* swtch);
void Switch_On(Switch_dev* swtch);
