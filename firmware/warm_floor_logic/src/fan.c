#include "fan.h"
#include "platform.h"

void Fan1_On() {
	TIM3->CCR1 = FAN_MAX_VALUE;
}

void Fan1_Set(uint8_t percent) {
	if (percent > 100) {percent = 100;}

	TIM3->CCR1 = FAN_MIN_VALUE+((FAN_MAX_VALUE-FAN_MIN_VALUE)/100)*percent;
}

void Fan1_Off(){
	TIM3->CCR1 = FAN_STOP_VALUE;
}

void Fan1_Init(){
	TIM_TimeBaseInitTypeDef timer;
	TIM_OCInitTypeDef timerPWM;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = FAN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(FAN_GPIO, &GPIO_InitStructure);

	TIM_TimeBaseStructInit(&timer);
	timer.TIM_Prescaler = 2000;
	timer.TIM_Period = 1000;
	timer.TIM_ClockDivision = 0;
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &timer);

	TIM_OCStructInit(&timerPWM);
	timerPWM.TIM_OCMode = TIM_OCMode_PWM1;
	timerPWM.TIM_OutputState = TIM_OutputState_Enable;
	timerPWM.TIM_Pulse = 1000;
	timerPWM.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3, &timerPWM);

	TIM_Cmd(TIM3, ENABLE);
}