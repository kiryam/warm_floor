#pragma once

#include "ssd1306.h"

#define DISPLAY_REDRAW_MAX 2

typedef struct __Display {
	//TM74HC595* instance;

} Display;

int Display_Init(Display* display);
int Display_WriteNumber(Display* display, int digits, uint8_t dots);
int Display_Write(Display* display, char chars[4]);
int Display_DrawMessage(Display* display, uint8_t height_from, char* msg);
int Display_DrawTemp(Display* display, int temp);

