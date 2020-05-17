#include "led.h"
#include "main.h"

void LED_Init(LED_dev* led){
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = led->LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(led->LED_GPIOx, &GPIO_InitStructure);

	LED_Off(led);
	delay_nms(50);
	LED_On(led);
	delay_nms(50);
	LED_Off(led);
	delay_nms(50);
	LED_On(led);
	delay_nms(50);
	LED_Off(led);
}

void LED_On(LED_dev* led){
	GPIO_WriteBit(led->LED_GPIOx, led->LED_PIN, Bit_SET);
}

void LED_Off(LED_dev* led){
	GPIO_WriteBit(led->LED_GPIOx, led->LED_PIN, Bit_RESET);
}


// maximum 255 blinks
void Blink_times(uint8_t n, uint32_t timeout){
	for(int i =0; i<n; i++){
		LED_On(&core.led1);
		delay_nms(timeout);
		LED_Off(&core.led1);
		delay_nms(timeout);
	}
}
