#include <stdio.h>
#include <stdlib.h>
#include "platform.h"

#include "diag/Trace.h"
#include "led.h"
#include "switch.h"
#include "serial.h"
#ifdef DISPLAY
#include "display.h"
#endif
#include "radio.h"
#include "misc.h"
#include "main.h"

#include "libdproto.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "floor.pb.h"
#include "DS18B20.h"

#include "cmsis_os.h"
#include "timers.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

Core core;

#define WATCHDOG_ENABLED

#define FAN_GPIO GPIOA
#define FAN_PIN GPIO_Pin_10

#define MIN_TEMP -30
#define MAX_TEMP 80
#define MAX_INSIDE_TEMP 80
#define FAN_ON_TEMP 50
#define MAX_ZONES_AT_THE_MOMENT 4

#define RADION_MAX_PACKETS_IN_MESSAGE 10
#define RADIO_BUFFER_SIZE RADION_MAX_PACKETS_IN_MESSAGE*NRF_PAYLOAD_LENGTH

static uint8_t magic_sequence[DPROTO_MAGIC_SEC_LEN] = {4, 8, 15, 16};
static uint8_t radio_buffer[RADIO_BUFFER_SIZE];
uint8_t pb_buffer[1024];
uint8_t zones_on;

#if configUSE_TIMERS == 1
static TimerHandle_t fanTimer;
static void fanTimerCallback( TimerHandle_t xExpiredTimer );
#endif

#ifdef DISPLAY
void DisplayRenderTask(void const * argument);
#endif
void MessageReadTask(void const * argument);
void SwitchStateTask(void const * argument);
void SendStateTask(void const * argument);


void USART1_IRQHandler() {
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		Serial_Callback callback;
		callback.dev = (void*)&core.usart1;
		callback.c = (char) USART_ReceiveData(USART1);
		core.usart1.RX_HANDLER((void*)&callback);
	}
}

void Led1_Init(LED_dev* led){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	led->LED_GPIOx = GPIOB;
	led->LED_PIN = GPIO_Pin_14;
	LED_Init(led);
}

void Switch1_Init(Switch_dev* swtch){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	swtch->Switch_GPIOx = GPIOB;
	swtch->Switch_PIN = GPIO_Pin_8;
	swtch->start_on = false;
	Switch_Init(swtch);
}

void Switch2_Init(Switch_dev* swtch){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	swtch->Switch_GPIOx = GPIOB;
	swtch->Switch_PIN = GPIO_Pin_9;
	swtch->start_on = false;
	Switch_Init(swtch);
}

void Switch3_Init(Switch_dev* swtch){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	swtch->Switch_GPIOx = GPIOB;
	swtch->Switch_PIN = GPIO_Pin_10;
	swtch->start_on = false;
	Switch_Init(swtch);
}

void Switch4_Init(Switch_dev* swtch){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	swtch->Switch_GPIOx = GPIOB;
	swtch->Switch_PIN = GPIO_Pin_11;
	swtch->start_on = false;
	Switch_Init(swtch);
}

void Switch5_Init(Switch_dev* swtch){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	swtch->Switch_GPIOx = GPIOB;
	swtch->Switch_PIN = GPIO_Pin_12;
	swtch->start_on = false;
	Switch_Init(swtch);
}

void Switch6_Init(Switch_dev* swtch){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	swtch->Switch_GPIOx = GPIOB;
	swtch->Switch_PIN = GPIO_Pin_13;
	swtch->start_on = false;
	Switch_Init(swtch);
}

void Serial1_Init(SERIAL_dev* usart){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_PinRemapConfig(GPIO_Remap_USART1, DISABLE);

	usart->usart->Init.USART_BaudRate = 115200;
	usart->usart->Instance = USART1;
	usart->USART_TX_GPIOx = GPIOA;
	usart->USART_TX_PIN = GPIO_Pin_9;

	usart->USART_RX_GPIOx = GPIOA;
	usart->USART_RX_PIN = GPIO_Pin_10;
	Serial_Init(usart);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_EnableIRQ(USART1_IRQn);
}

bool is_timer_started = false;
void Fan1_On() {
	//Switch_On(&core.switches[0]);
	GPIO_WriteBit(FAN_GPIO, FAN_PIN, Bit_SET);
}

void Fan1_Off(){
	//Switch_Off(&core.switches[0]);
	GPIO_WriteBit(FAN_GPIO, FAN_PIN, Bit_RESET);
}

void Fan1_Init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = FAN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(FAN_GPIO, &GPIO_InitStructure);
}


void Display1_Init(Display* display){
	Display_Init(display);
}

void DS18B20_Init(){
	if(ds18b20_init() == 0)  {
		//ERROR NO SENSORS
	}
}

int main(int argc, char* argv[]) {
	SysTick_Init();
	NVIC_SetPriority(SysTick_IRQn, 15);

	Fan1_Init();

	Switch1_Init(&core.switches[0]);
	Switch2_Init(&core.switches[1]);
	Switch3_Init(&core.switches[2]);
	Switch4_Init(&core.switches[3]);
	Switch5_Init(&core.switches[4]);
	Switch6_Init(&core.switches[5]);

#ifdef WATCHDOG_ENABLED
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256); // 4, 8, 16 ... 256
	IWDG_SetReload(0x0FFF);//This parameter must be a number between 0 and 0x0FFF.
	IWDG_ReloadCounter();
	IWDG_Enable();
#endif
	Led1_Init(&core.led1);
	Fan1_On();

	DS18B20_Init();
	ds18b20_start_convert();

	delay_nms(1000);
	core.temp1 = (int32_t)ds18b20_get_temp(0);

	for(int i =0; i<(core.temp1/10); i++){
		LED_On(&core.led1);
		delay_nms(500);
		LED_Off(&core.led1);
		delay_nms(500);
	}

#ifdef WATCHDOG_ENABLED
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_16);
	IWDG_SetReload(0x0FFF);//This parameter must be a number between 0 and 0x0FFF.
	IWDG_ReloadCounter();
	IWDG_Enable();
#endif

	Fan1_Init();

	//USART_HandleTypeDef usart_usart;
	//core.usart1.usart = &usart_usart;
	//Serial1_Init(&core.usart1);
	//Serial_Send_Str(&core.usart1, "Initializing\r\n");

#ifdef DISPLAY
	Display1_Init(&core.display);
	//Serial_Send_Str(&core.usart1, "Display init ok\r\n");
#endif

	Radio_Init(&core.radio1);
	//Serial_Send_Str(&core.usart1, "Radio init ok\r\n");

	Thermometer_Init(&core.thermometers);
	//Serial_Send_Str(&core.usart1, "Thermometer init ok\r\n");

	//LED_Off(&core.led1);

	//Serial_Send_Str(&core.usart1, "DS18B20 init ok\r\n");

	core.radio_queue = xQueueCreate(32, sizeof(uint8_t));

#if configUSE_TIMERS == 1
	fanTimer = xTimerCreate( "fanTimer", 5000/portTICK_RATE_MS, pdFALSE, ( void * )0, fanTimerCallback );
	xTimerStart( fanTimer, 0 );
#endif
	osThreadDef(switchStateTask, SwitchStateTask, osPriorityHigh, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(switchStateTask), NULL);

	osThreadDef(messageReadTask, MessageReadTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(messageReadTask), NULL);

	osThreadDef(sendStateTask, SendStateTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(sendStateTask), NULL);


#ifdef DISPLAY
	osThreadDef(displayRenderTask, DisplayRenderTask, osPriorityLow, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(displayRenderTask), NULL);
#endif

	osKernelStart();
	while (1) {
	}
}

#if configUSE_TIMERS == 1
static void fanTimerCallback( TimerHandle_t xExpiredTimer ) {
	Fan1_Off();
	xTimerStop( xExpiredTimer, (TickType_t) 10 );
}
#endif

void MessageReadTask(void const * argument){
	uint32_t buffer_pos = 0;
	uint32_t length=0;
	uint8_t data=0;
	for(;;){
		if( xQueueReceive( (xQueueHandle)core.radio_queue, &( data ), ( TickType_t ) 100 ) ) {
			if( buffer_pos >= RADIO_BUFFER_SIZE ) {
				buffer_pos=0;
				length = 0;
				//core.messages_failed++;
				break;
			}

			int result = dproto_process_byte(radio_buffer, &length, magic_sequence, data, buffer_pos++);

			if (result == -1) { // decode error
				buffer_pos=0;
				length = 0;
				//core.messages_failed++;
			} else if (result == 1 ){ // read message done
				pb_istream_t stream = pb_istream_from_buffer(radio_buffer, length);

				//core.messages_readed++;
				buffer_pos=0;
				length = 0;

				pb_decode(&stream, TempSet_fields, &core.tempSet);
				LED_On(&core.led1);
				osDelay(100);
				LED_Off(&core.led1);
			}
		}
	}
}

#ifdef DISPLAY
void DisplayRenderTask(void const * argument){
	 for(;;) {
		SSD1306_Fill(SSD1306_COLOR_BLACK);

		Display_DrawTemp(&core.display, core.tempSet.temp);
		char message[16]={0};
		sprintf(message, "Recv: %d", core.packets_rx);
		Display_DrawMessage(&core.display, 64, message);

		sprintf(message, "Read: %d Failed: %d", core.messages_readed, core.messages_failed);
		Display_DrawMessage(&core.display, 50, message);

		SSD1306_UpdateScreen();
		osDelay(100);
	  }
}
#endif


void SwitchStateTask(void const * argument){
	for(;;) {

		core.tempActual.zone_1 = -273;
		core.tempActual.zone_2 = Thermometer_GetValue(&core.thermometers, 1);
		core.tempActual.zone_3 = Thermometer_GetValue(&core.thermometers, 2);
		core.tempActual.zone_4 = Thermometer_GetValue(&core.thermometers, 3);
		core.tempActual.zone_5 = Thermometer_GetValue(&core.thermometers, 4);
		core.tempActual.zone_6 = Thermometer_GetValue(&core.thermometers, 5);

		core.temp1 = (int32_t)ds18b20_get_temp(0);
		core.temp2 = (int32_t)ds18b20_get_temp(1);
		ds18b20_start_convert();

		//core.tempSet.temp = get_internal_temp(&core.thermometers);

		if( core.temp1 > MAX_INSIDE_TEMP || core.temp2 > MAX_INSIDE_TEMP || core.tempActual.zone_2 > MAX_TEMP || core.tempActual.zone_3 > MAX_TEMP
				|| core.tempActual.zone_4 > MAX_TEMP || core.tempActual.zone_5 > MAX_TEMP || core.tempActual.zone_6 > MAX_TEMP) {
			NVIC_SystemReset();
		}

		bool can_heat = false;
		if ( (core.temp1 > -40 && core.temp1 < 60) &&
		     (core.temp2 > -40 && core.temp2 < 60)
				&& abs(core.temp1 - core.temp2) < 20 ) { // check inside temp
			can_heat = true;
		}

		if( core.temp1 >= FAN_ON_TEMP || core.temp2 >= FAN_ON_TEMP ) {
#if configUSE_TIMERS == 1
			if( xTimerIsTimerActive( fanTimer ) != pdFALSE ) {
				 xTimerReset(fanTimer, ( TickType_t )1);
			} else {
				Fan1_On();
				xTimerStart( fanTimer, 0 );
			}
#else
		Fan1_On();
#endif
		} else {
#if configUSE_TIMERS == 0
			Fan1_Off();
#endif
		}

		zones_on=0;

		if(core.tempSet.zone_1 > core.tempActual.zone_1 && core.tempActual.zone_1 > MIN_TEMP && can_heat && zones_on < MAX_ZONES_AT_THE_MOMENT ) {
			Switch_On(&core.switches[0]);
		}else {
			Switch_Off(&core.switches[0]);
		}

		if(core.tempSet.zone_2 > core.tempActual.zone_2 && core.tempActual.zone_2 > MIN_TEMP && can_heat && zones_on < MAX_ZONES_AT_THE_MOMENT ) {
			Switch_On(&core.switches[1]);
			zones_on++;
			LED_On(&core.led1);
		}else {
			Switch_Off(&core.switches[1]);
		}

		if(core.tempSet.zone_3 > core.tempActual.zone_3 && core.tempActual.zone_3 > MIN_TEMP && can_heat && zones_on < MAX_ZONES_AT_THE_MOMENT ) {
			Switch_On(&core.switches[2]);
			zones_on++;
		}else {
			Switch_Off(&core.switches[2]);
		}

		if(core.tempSet.zone_4 > core.tempActual.zone_4 && core.tempActual.zone_4 > MIN_TEMP && can_heat && zones_on < MAX_ZONES_AT_THE_MOMENT) {
			Switch_On(&core.switches[3]);
			zones_on++;
		}else {
			Switch_Off(&core.switches[3]);
		}

		if(core.tempSet.zone_5 > core.tempActual.zone_5 && core.tempActual.zone_5 > MIN_TEMP && can_heat && zones_on < MAX_ZONES_AT_THE_MOMENT) {
			Switch_On(&core.switches[4]);
			zones_on++;
		}else {
			Switch_Off(&core.switches[4]);
		}

		if(core.tempSet.zone_6 > core.tempActual.zone_6 && core.tempActual.zone_6 > MIN_TEMP && can_heat && zones_on < MAX_ZONES_AT_THE_MOMENT) {
			Switch_On(&core.switches[5]);
			zones_on++;
		}else {
			Switch_Off(&core.switches[5]);
		}

#ifdef WATCHDOG_ENABLED
		IWDG_ReloadCounter();
#endif
		osDelay(100);
	  }
}

void SendStateTask(void const * argument){
	for(;;){
		State state = State_init_zero;
		state.temp1 = core.temp1;
		state.temp2 = core.temp2;
		state.tempSet = core.tempSet;
		state.tempActual = core.tempActual;

		pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer, sizeof(pb_buffer));
		pb_encode(&stream, State_fields, &state);
		Radio_Send(&core.radio1, pb_buffer, stream.bytes_written);
		osDelay(2000);
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line){
  while(1){}
}


#pragma GCC diagnostic pop