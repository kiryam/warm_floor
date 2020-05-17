#pragma once

#include "platform.h"


typedef struct __Serial_Callback {
	void* dev;
	char c;
} Serial_Callback;


#define MAX_STRING_LEN 1024

typedef enum
{
  USART_OK       = 0x00U,
  USART_ERROR    = 0x01U,
  USART_BUSY     = 0x02U,
  USART_TIMEOUT  = 0x03U
} USART_StatusTypeDef;


typedef struct __USART_HandleTypeDef {
	USART_TypeDef               *Instance;
	USART_InitTypeDef           Init;
} USART_HandleTypeDef;

typedef struct __SERIAL_dev {
	USART_HandleTypeDef* usart;

	uint16_t		USART_RX_PIN;
	GPIO_TypeDef*	USART_RX_GPIOx;

	uint16_t		USART_TX_PIN;
	GPIO_TypeDef*	USART_TX_GPIOx;

	callback      RX_HANDLER;
} SERIAL_dev;


USART_StatusTypeDef Serial_Init(SERIAL_dev *husart);
USART_StatusTypeDef Serial_Send_Str(SERIAL_dev *husart, char* str);
USART_StatusTypeDef Serial_Send_Bytes(SERIAL_dev *husart, uint8_t* bytes, uint32_t len);

#ifdef SERIAL_DEBUG
#define DEBUG(u, s) Serial_Send_Str(u, s)
#ifdef INTERNAL_TEMP_SENSOR
# error "INTERNAL_TEMP_SENSOR must be undefined if SERIAL_DEBUG was enabled"
#endif
#else
#define DEBUG(u, s)
#endif