#pragma once

#include "stddef.h"
#include "nRF24L01.h"

typedef struct __Radio {
	nrf24l01_dev* instance;
} Radio;

int Radio_Init(Radio* radio);
int Radio_Send(Radio* radio, uint8_t* data, size_t size);
