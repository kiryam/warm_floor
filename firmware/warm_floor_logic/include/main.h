#pragma once

#include "led.h"
#include "switch.h"
#include "serial.h"
#include "display.h"
#include "radio.h"
#include "thermometer.h"
#include "floor.pb.h"

// maximum seconds between temp set commands
// if no temp set event between this interval happens - stop heating
// in seconds
#define MAX_TIME_SINCE_TEMP_SET 600
typedef struct __CORE {
	SERIAL_dev usart1;
	LED_dev led1;
#ifdef DISPLAY
	Display display;
#endif

	int32_t current_temp;

	Switch_dev switches[6];

	Thermometer thermometers;

	//uint32_t packets_rx;
	//uint32_t messages_readed;
	//uint32_t messages_failed;

	Radio radio1;

	void* radio_queue;

	TempSet tempSet;
	TempSet tempActual;

	int32_t temp1;
	int32_t temp2;

	// when last temp set command recived that counter updates
	uint32_t lastTempSet;
} Core;

extern Core core;

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

