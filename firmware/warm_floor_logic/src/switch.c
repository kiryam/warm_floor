#include "switch.h"

void Switch_Init(Switch_dev* swtch){
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = swtch->Switch_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(swtch->Switch_GPIOx, &GPIO_InitStructure);

	if (swtch->start_on == true) {
		Switch_On(swtch);
	} else {
		Switch_Off(swtch);
	}
}

void Switch_On(Switch_dev* swtch){
	GPIO_WriteBit(swtch->Switch_GPIOx, swtch->Switch_PIN, Bit_SET);
	swtch->is_on = true;

}

void Switch_Off(Switch_dev* swtch){
	GPIO_WriteBit(swtch->Switch_GPIOx, swtch->Switch_PIN, Bit_RESET);
	swtch->is_on = false;
}
