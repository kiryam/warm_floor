#pragma once

#include "stdint.h"

#define CACHE_SIZE 32
typedef struct __Thermometer {
	uint16_t          adc_buffer[6];

	uint16_t cache[6][CACHE_SIZE];
	uint16_t cache_pos[6];
} Thermometer;


int Thermometer_Init(Thermometer* therm);
int16_t Thermometer_GetValue(Thermometer* therm, uint8_t termid);
uint16_t get_internal_temp(Thermometer* therm);
