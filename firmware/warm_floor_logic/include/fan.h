#pragma once

#include "platform.h"

#define FAN_GPIO GPIOB
#define FAN_PIN GPIO_Pin_4

#define FAN_MIN_DEGREE 35 // minimal degree that fan should be set to FAN_MIN_VALUE eg. if degree < than FAN_MIN_DEGREE fan stays at FAN_STOP_VALUE
#define FAN_MAX_DEGREE 60 // at that degree fan should be set to FAN_MAX_VALUE
#define FAN_STOP_VALUE 200
#define FAN_MIN_VALUE 200
#define FAN_MAX_VALUE 1000


void Fan1_On(void);
void Fan1_Set(uint8_t percent);
void Fan1_Init();