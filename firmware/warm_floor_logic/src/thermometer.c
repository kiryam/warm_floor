#include "platform.h"
#include "thermometer.h"
#include "math.h"

#define MEASURE_DEGREE 250
#define MIN_VALUE 0
#define MAX_VALUE 4096
#define POINTS_PER_DEGREE ((MAX_VALUE-MIN_VALUE)/MEASURE_DEGREE)


#define B 3950 // B-коэффициент
#define SERIAL_R 10200 // сопротивление последовательного резистора, 10 кОм
#define THERMISTOR_R 100000 // номинальное сопротивления термистора, 100 кОм
#define NOMINAL_T 25 // номинальная температура (при которой TR = 100 кОм)


int Thermometer_Init(Thermometer *therm) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	ADC1->CR2 |= ADC_CR2_TSVREFE;

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_NbrOfChannel = 6;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_28Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_28Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_28Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_28Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_28Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 6, ADC_SampleTime_239Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 7, ADC_SampleTime_28Cycles5);


	ADC_DiscModeCmd(ADC1, DISABLE);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);

	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ADC1->DR;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)therm->adc_buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 6;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);

	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd ( ADC1 , ENABLE ) ;
	return 0;
}

uint16_t get_avg_from_buff(Thermometer* therm, uint8_t termid){
	therm->cache[termid][((therm->cache_pos[termid]++) % CACHE_SIZE)] = therm->adc_buffer[termid];
	uint32_t sum=0;
	for(unsigned int i=0; i< CACHE_SIZE; i++){
		sum += therm->cache[termid][i];
	}
	return sum/CACHE_SIZE;
}


///#define TEMP110_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7C2))
//#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
//#define VDD_CALIB ((uint16_t) (330))
//#define VDD_APPLI ((uint16_t) (300))
//#define TEMP_SENSOR_AVG_SLOPE_MV_PER_CELSIUS                        2.5f
//#define TEMP_SENSOR_VOLTAGE_MV_AT_25                                760.0f
//#define ADC_REFERENCE_VOLTAGE_MV                                    3300.0f
//#define ADC_MAX_OUTPUT_VALUE                                        4095.0f
//#define TEMP110_CAL_VALUE                                           ((uint16_t*)((uint32_t)0x1FFF7A2E))
//#define TEMP30_CAL_VALUE                                            ((uint16_t*)((uint32_t)0x1FFF7A2C))
//#define TEMP110_CAL_VALUE 1300
//#define TEMP30_CAL_VALUE 1600
//#define TEMP110                                                     110.0f
//#define TEMP30                                                      30.0f
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
#define VDD_CALIB ((uint32_t) (3300))
#define VDD_APPLI ((uint32_t) (3000))
#define AVG_SLOPE ((uint32_t) (5336))


uint16_t get_internal_temp(Thermometer* therm){
	int32_t sensorValue = therm->adc_buffer[5];
	//int32_t temperature = ((therm->adc_buffer[5] * VDD_APPLI / VDD_CALIB) - (int32_t) *TEMP30_CAL_ADDR ) ;
	////temperature = temperature * (int32_t)(110 - 30);
	//temperature = temperature / (int32_t)(*TEMP110_CAL_ADDR - *TEMP30_CAL_ADDR);
	//temperature = temperature + 30;

	//float temperature = (static_cast<float>(adcTempValue) - adcCalTemp30C)/(adcCalTemp110C - adcCalTemp30C) * (110.0F - 30.0F) + 30.0F;

	//float V_sense = therm->adc_buffer[5]/4096.0*Vref;
	//uint16_t temp = (V_25 - V_sense)/Slope + 25.0;

	//int32_t temperature = (int32_t)((TEMP110 - TEMP30) / ((float)(*TEMP110_CAL_VALUE) - (float)(*TEMP30_CAL_VALUE)) * (sensorValue - (float)(*TEMP30_CAL_VALUE)) + TEMP30);
	//sensorValue = sensorValue * ADC_REFERENCE_VOLTAGE_MV / ADC_MAX_OUTPUT_VALUE;
	//int32_t temperature = (int32_t)((TEMP110 - TEMP30) / ((float)(TEMP110_CAL_VALUE) - (float)(TEMP30_CAL_VALUE)) * (sensorValue - (float)(TEMP30_CAL_VALUE)) + TEMP30);

	int32_t temperature; /* will contain the temperature in degrees Celsius */
	temperature = (1580 - ((uint32_t) therm->adc_buffer[5] * VDD_APPLI / VDD_CALIB)) * 1000;

	temperature = (temperature / AVG_SLOPE) + 30;
	return temperature;

}

int16_t Thermometer_GetValue(Thermometer* therm, uint8_t termid){
	uint16_t t = get_avg_from_buff(therm, termid);
	float tr = 4095.0 / t - 1;
	tr = SERIAL_R * tr;
	float steinhart ;
	steinhart = tr / THERMISTOR_R;      // (R/Ro)
	steinhart = log(steinhart) ;         // ln(R/Ro)
	steinhart /= B ;                // 1/B * ln(R/Ro)
	steinhart += 1.0 / (NOMINAL_T + 273.15) ; // + (1/To)
	steinhart = 1.0 / steinhart ;        // Invert
	steinhart -= 273.15 ;                // convert to C
	return steinhart;
}
