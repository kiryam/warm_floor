#include "serial.h"

void USART_Rx_Handler(Serial_Callback *callback){
	char str[] = "Echo >> {b}\r\n";
	str[9] = callback->c;
	Serial_Send_Str(callback->dev, str);
}

USART_StatusTypeDef Serial_Init(SERIAL_dev *husart) {
	GPIO_InitTypeDef gpio_port;
	gpio_port.GPIO_Pin   = husart->USART_RX_PIN;
	gpio_port.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(husart->USART_RX_GPIOx, &gpio_port);

	GPIO_InitTypeDef gpio_port1;
	gpio_port1.GPIO_Pin   = husart->USART_TX_PIN;
	gpio_port1.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_port1.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(husart->USART_TX_GPIOx, &gpio_port1);

	husart->usart->Init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	husart->usart->Init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	husart->usart->Init.USART_Parity = USART_Parity_No;
	husart->usart->Init.USART_StopBits = USART_StopBits_1;
	husart->usart->Init.USART_WordLength = USART_WordLength_8b;

	USART_Init(husart->usart->Instance, &husart->usart->Init);
	USART_Cmd(husart->usart->Instance, ENABLE);

	USART_ITConfig(husart->usart->Instance, USART_IT_RXNE, ENABLE);

	husart->RX_HANDLER = USART_Rx_Handler;
	return USART_OK;
}


USART_StatusTypeDef Serial_Send_Str(SERIAL_dev *husart, char* str){
	int i = -1;
	while((i++)< MAX_STRING_LEN){
		if (str[i] == '\0') {
			return USART_OK;
		};

		USART_SendData(husart->usart->Instance, (uint16_t)str[i]);
		while(USART_GetFlagStatus(husart->usart->Instance, USART_FLAG_TXE) == RESET){}
	}

	return USART_ERROR;
}

USART_StatusTypeDef Serial_Send_Bytes(SERIAL_dev *husart, uint8_t* bytes, uint32_t len) {
	for(unsigned int i=0; i< len; i++ ){
		USART_SendData(husart->usart->Instance, bytes[i]);
		while(USART_GetFlagStatus(husart->usart->Instance, USART_FLAG_TXE) == RESET){}
	}
	return USART_OK;
}
