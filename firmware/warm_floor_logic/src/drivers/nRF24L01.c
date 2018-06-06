#include "string.h"
#include "platform.h"
#include "stdio.h"

#include "main.h"

#include "bus_spi.h"
#include "nRF24L01.h"

static nrf24l01_dev NRF24L01_1= {0};
static SPI_HandleTypeDef nrf24_spi1;
static uint8_t RX_BUFFER1[NRF_PAYLOAD_LENGTH] = {0};

static void NRF_CS_SETPIN(nrf24l01_dev* dev) {
	GPIO_WriteBit(dev->NRF_CSN_GPIOx, dev->NRF_CSN_PIN, Bit_SET);
}

static void NRF_CS_RESETPIN(nrf24l01_dev* dev) {
	GPIO_WriteBit(dev->NRF_CSN_GPIOx, dev->NRF_CSN_PIN, Bit_RESET);
}

static void NRF_CE_SETPIN(nrf24l01_dev* dev) {
	GPIO_WriteBit(dev->NRF_CE_GPIOx, dev->NRF_CE_PIN, Bit_SET);
}

static void NRF_CE_RESETPIN(nrf24l01_dev* dev) {
	GPIO_WriteBit(dev->NRF_CE_GPIOx, dev->NRF_CE_PIN, Bit_RESET);
}

void NRF_SetupGPIO(nrf24l01_dev* dev) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = dev->NRF_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(dev->NRF_SCK_GPIOx, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = dev->NRF_MOSI_PIN;
	GPIO_Init(dev->NRF_MOSI_GPIOx, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = dev->NRF_MISO_PIN;
	GPIO_Init(dev->NRF_MISO_GPIOx, &GPIO_InitStructure);

	GPIO_InitTypeDef GPIO_InitStructure2;
	GPIO_StructInit(&GPIO_InitStructure2);
	GPIO_InitStructure2.GPIO_Pin = dev->NRF_CSN_PIN;
	GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(dev->NRF_CSN_GPIOx, &GPIO_InitStructure2);

	GPIO_InitStructure2.GPIO_Pin =  dev->NRF_CE_PIN;
	GPIO_Init(dev->NRF_CE_GPIOx, &GPIO_InitStructure2);

	SPI_StructInit(&dev->spi->Init);
	dev->spi->Init.SPI_Mode = SPI_Mode_Master;
	dev->spi->Init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	dev->spi->Init.SPI_DataSize = SPI_DataSize_8b;
	dev->spi->Init.SPI_CPOL = SPI_CPOL_Low;
	dev->spi->Init.SPI_CPHA = SPI_CPHA_1Edge;
	dev->spi->Init.SPI_NSS = SPI_NSS_Soft;
	dev->spi->Init.SPI_BaudRatePrescaler = dev->SPI_BaudRatePrescaler;
	dev->spi->Init.SPI_FirstBit = SPI_FirstBit_MSB;
	dev->spi->Init.SPI_CRCPolynomial = 7;

	dev->spi->ErrorCode = HAL_SPI_ERROR_NONE;
	dev->spi->State     = HAL_SPI_STATE_READY;
	dev->spi->Lock      = HAL_UNLOCKED;

	SPI_Init(dev->spi->Instance, &dev->spi->Init);
	SPI_CalculateCRC(dev->spi->Instance, DISABLE);
	SPI_Cmd(dev->spi->Instance, ENABLE);

	NRF_CE_SETPIN(dev);
	NRF_CS_SETPIN(dev);
}

void EXTI2_IRQHandler(void){
	EXTI_ClearITPendingBit(EXTI_Line2);
	if (NRF24L01_1.on_event != NULL){
		NRF_IRQ_Handler(&NRF24L01_1);
	}
}

nrf24l01_dev* NRF1_Init(uint8_t* addr_master,uint8_t* addr_slave, NRF_ADDR_WIDTH add_width, NRF_CRC_WIDTH crc_width, NRF_DATA_RATE data_rate, uint8_t rf_channel, uint8_t retransmit_count, uint8_t retransmit_delay, NRF_TX_PWR tx_power, callback on_event) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);


	NRF24L01_1.RX_BUFFER = RX_BUFFER1;
	NRF24L01_1.on_event = on_event;

	NRF24L01_1.NRF_CE_GPIOx  = GPIOB;
	NRF24L01_1.NRF_CE_PIN = GPIO_Pin_0;

	NRF24L01_1.NRF_CSN_GPIOx = GPIOB;
	NRF24L01_1.NRF_CSN_PIN = GPIO_Pin_1;

	NRF24L01_1.NRF_SCK_GPIOx = GPIOA;
	NRF24L01_1.NRF_SCK_PIN = GPIO_Pin_5;

	NRF24L01_1.NRF_MOSI_GPIOx = GPIOA;
	NRF24L01_1.NRF_MOSI_PIN = GPIO_Pin_7;

	NRF24L01_1.NRF_MISO_GPIOx = GPIOA;
	NRF24L01_1.NRF_MISO_PIN = GPIO_Pin_6;

	NRF24L01_1.NRF_IRQ_GPIOx = GPIOB;
	NRF24L01_1.NRF_IRQ_PIN = GPIO_Pin_2;

	NRF24L01_1.spi = &nrf24_spi1;
	NRF24L01_1.spi->Instance = SPI1;
	NRF24L01_1.ADDR_WIDTH = add_width;
	NRF24L01_1.CRC_WIDTH = crc_width;
	NRF24L01_1.DATA_RATE = data_rate;
	NRF24L01_1.RF_CHANNEL = rf_channel;

	NRF24L01_1.MASTER_ADDRESS = addr_master;
	NRF24L01_1.SLAVE_ADDRESS = addr_slave;

	NRF24L01_1.RetransmitCount = retransmit_count;
	NRF24L01_1.RetransmitDelay = retransmit_delay;
	NRF24L01_1.TX_POWER = tx_power;
	NRF24L01_1.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;

	NRF_SetupGPIO(&NRF24L01_1);

	GPIO_InitTypeDef GPIO_InitStructure3;
	GPIO_StructInit(&GPIO_InitStructure3);
	GPIO_InitStructure3.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure3.GPIO_Pin = NRF24L01_1.NRF_IRQ_PIN;
	GPIO_InitStructure3.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(NRF24L01_1.NRF_IRQ_GPIOx, &GPIO_InitStructure3);


	EXTI_InitTypeDef EXTI_InitStruct;
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource2);
	EXTI_InitStruct.EXTI_Line = EXTI_Line2;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);


	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	NVIC_SetPriority(EXTI2_IRQn, 5);

	delay_nms(100);

	NRF_PowerUp(&NRF24L01_1, 1);

	NRF_SetTXPower(&NRF24L01_1, NRF24L01_1.TX_POWER);
	NRF_SetRXPayloadWidth_P1(&NRF24L01_1, NRF_PAYLOAD_LENGTH);
	NRF_SetRXAddress_P1(&NRF24L01_1, NRF24L01_1.SLAVE_ADDRESS);
	NRF_SetTXAddress(&NRF24L01_1, NRF24L01_1.MASTER_ADDRESS);
	NRF_EnableRXDataReadyIRQ(&NRF24L01_1, 1);
	NRF_EnableTXDataSentIRQ(&NRF24L01_1, 1);
	NRF_EnableMaxRetransmitIRQ(&NRF24L01_1, 1);
	NRF_EnableCRC(&NRF24L01_1, 1);
	NRF_SetCRCWidth(&NRF24L01_1, NRF24L01_1.CRC_WIDTH);
	NRF_SetAddressWidth(&NRF24L01_1, NRF24L01_1.ADDR_WIDTH);
	NRF_SetRFChannel(&NRF24L01_1, NRF24L01_1.RF_CHANNEL);
	NRF_SetDataRate(&NRF24L01_1, NRF24L01_1.DATA_RATE);
	NRF_SetRetransmittionCount(&NRF24L01_1, NRF24L01_1.RetransmitCount);
	NRF_SetRetransmittionDelay(&NRF24L01_1, NRF24L01_1.RetransmitDelay);
	//NRF_EnableRXPipe(&NRF24L01_1, 0);
	NRF_EnableRXPipe(&NRF24L01_1, 1);
	NRF_EnableAutoAcknowledgement(&NRF24L01_1, 1);

	NRF_ClearInterrupts(&NRF24L01_1);

	NRF_RXTXControl(&NRF24L01_1, NRF_STATE_RX);
	NRF_FlushRX(&NRF24L01_1);

	return &NRF24L01_1;
}

NRF_RESULT NRF_SendCommand(nrf24l01_dev* dev, uint8_t cmd, uint8_t* tx, uint8_t* rx, uint8_t len) {
	uint8_t myTX[len + 1];
	uint8_t myRX[len + 1];
	int i;

	memset(myTX, 0, len+1);
	memset(myRX, 0, len+1);

	myTX[0] = cmd;

	for (i = 0; i < len; i++) {
		myTX[i+1] = tx[i];
	}

	NRF_CS_RESETPIN(dev);
	delay_nus(20);
	if (SPI_TransmitReceive(dev->spi, myTX, myRX, 1 + len, NRF_SPI_TIMEOUT) != HAL_OK) {
		return NRF_ERROR;
	}

	for (i = 0; i < len; i++) {
		rx[i] = myRX[1 + i];
	}

	NRF_CS_SETPIN(dev);

	return NRF_OK;
}

void NRF_IRQ_Handler(nrf24l01_dev* dev) {
	delay_nus(10);
	uint8_t status = 0;
	if (NRF_ReadRegister(dev, NRF_STATUS, &status) != NRF_OK) {
		return;
	}

	if ((status & (1 << 6))) {	// RX FIFO Interrupt
		uint8_t fifo_status = 0;
		NRF_CE_RESETPIN(dev);
		NRF_WriteRegister(dev, NRF_STATUS, &status);
		NRF_ReadRegister(dev, NRF_FIFO_STATUS, &fifo_status);


		if ( (fifo_status & 1) == 0) {
			NRF_ReadRXPayload(dev, dev->RX_BUFFER);
			dev->on_event(dev);
			status |= 1 << 6;
			NRF_WriteRegister(dev, NRF_STATUS, &status);
			NRF_FlushRX(dev);
		}
		NRF_CE_SETPIN(dev);
	}

	if ((status & (1 << 5))) {	// TX Data Sent Interrupt
		status |= 1 << 5;	// clear the interrupt flag
		NRF_CE_RESETPIN(dev);
		NRF_RXTXControl(dev, NRF_STATE_RX);
		dev->STATE = NRF_STATE_RX;
		NRF_CE_SETPIN(dev);
		NRF_WriteRegister(dev, NRF_STATUS, &status);
		dev->BUSY_FLAG=0;
	}
	if ((status & (1 << 4))) {	// MaxRetransmits reached
		//core.stats.tx_failed++;
		//LED_On(&core.led3);
		//delay_nms(50);
		// TODO
		status |= 1 << 4;

		NRF_FlushTX(dev);
		//NRF_PowerUp(dev,0);	// power down
		//NRF_PowerUp(dev,1);	// power up

		NRF_CE_RESETPIN(dev);
		NRF_RXTXControl(dev, NRF_STATE_RX);
		dev->STATE = NRF_STATE_RX;
		NRF_CE_SETPIN(dev);

		NRF_WriteRegister(dev, NRF_STATUS, &status);
		dev->BUSY_FLAG=0;
		//LED_Off(&core.led3);

	}
}


NRF_RESULT NRF_ReadRegister(nrf24l01_dev* dev, uint8_t reg, uint8_t* data) {
	uint8_t tx[2] = {0};
	if (NRF_SendCommand(dev, NRF_CMD_R_REGISTER | reg, tx, data, 1) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_WriteRegister(nrf24l01_dev* dev, uint8_t reg, uint8_t* data) {
	uint8_t rx[2] = {0};
	if (NRF_SendCommand(dev, NRF_CMD_W_REGISTER | reg, data, rx, 1) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_ReadRXPayload(nrf24l01_dev* dev, uint8_t* data) {
	uint8_t rx[NRF_PAYLOAD_LENGTH] = {0};
	if (NRF_SendCommand(dev, NRF_CMD_R_RX_PAYLOAD, rx, data, NRF_PAYLOAD_LENGTH) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_WriteTXPayload(nrf24l01_dev* dev, uint8_t* data) {
	uint8_t tx[NRF_PAYLOAD_LENGTH] = {0};
	if (NRF_SendCommand(dev, NRF_CMD_W_TX_PAYLOAD, data, tx, NRF_PAYLOAD_LENGTH) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_FlushTX(nrf24l01_dev* dev) {
	uint8_t rx = 0; //TODO fixme
	uint8_t tx = 0; // fixme
	if (NRF_SendCommand(dev, NRF_CMD_FLUSH_TX, &tx, &rx, 0) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_FlushRX(nrf24l01_dev* dev) {
	uint8_t rx = 0; // fixme
	uint8_t tx = 0; // fixme
	if (NRF_SendCommand(dev, NRF_CMD_FLUSH_RX, &tx, &rx, 0) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_SetDataRate(nrf24l01_dev* dev, NRF_DATA_RATE rate) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	if (rate & 1) {	// low bit set
		reg |= 1 << 5;
	} else {	// low bit clear
		reg &= ~(1 << 5);
	}

	if (rate & 2) {	// high bit set
		reg |= 1 << 3;
	} else {	// high bit clear
		reg &= ~(1 << 3);
	}
	if (NRF_WriteRegister(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	dev->DATA_RATE = rate;
	return NRF_OK;
}

NRF_RESULT NRF_SetTXPower(nrf24l01_dev* dev, NRF_TX_PWR pwr) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	reg &= 0xF9;	// clear bits 1,2
	reg |= pwr << 1;	// set bits 1,2
	if (NRF_WriteRegister(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_SetCCW(nrf24l01_dev* dev, uint8_t activate) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	if (activate) {
		reg |= 0x80;
	} else {
		reg &= 0x7F;
	}

	if (NRF_WriteRegister(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_ClearInterrupts(nrf24l01_dev* dev) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_STATUS, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	reg |= 7 << 4;	// setting bits 4,5,6

	if (NRF_WriteRegister(dev, NRF_STATUS, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_SetRFChannel(nrf24l01_dev* dev, uint8_t ch) {
	ch &= 0x7F;
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_RF_CH, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	reg |= ch;	// setting channel

	if (NRF_WriteRegister(dev, NRF_RF_CH, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	dev->RF_CHANNEL = ch;
	return NRF_OK;
}

NRF_RESULT NRF_SetRetransmittionCount(nrf24l01_dev* dev, uint8_t count) {
	count &= 0x0F;
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_SETUP_RETR, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	reg &= 0xF0;	// clearing bits 0,1,2,3
	reg |= count;	// setting count

	if (NRF_WriteRegister(dev, NRF_SETUP_RETR, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	dev->RetransmitCount = count;
	return NRF_OK;
}

NRF_RESULT NRF_SetRetransmittionDelay(nrf24l01_dev* dev, uint8_t delay) {
	delay &= 0x0F;
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_SETUP_RETR, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	reg &= 0x0F;	// clearing bits 1,2,6,7
	reg |= delay << 4;	// setting delay

	if (NRF_WriteRegister(dev, NRF_SETUP_RETR, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	dev->RetransmitDelay = delay;
	return NRF_OK;
}

NRF_RESULT NRF_SetAddressWidth(nrf24l01_dev* dev, NRF_ADDR_WIDTH width) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_SETUP_AW, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	reg &= 0x03;	// clearing bits 0,1
	reg |= width;	// setting delay

	if (NRF_WriteRegister(dev, NRF_SETUP_AW, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	dev->ADDR_WIDTH = width;
	return NRF_OK;
}

NRF_RESULT NRF_EnableRXPipe(nrf24l01_dev* dev, uint8_t pipe) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_EN_RXADDR, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	reg |= 1 << pipe;

	if (NRF_WriteRegister(dev, NRF_EN_RXADDR, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_EnableAutoAcknowledgement(nrf24l01_dev* dev, uint8_t pipe) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_EN_AA, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	reg |= 1 << pipe;

	if (NRF_WriteRegister(dev, NRF_EN_AA, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_EnableCRC(nrf24l01_dev* dev, uint8_t activate) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	if (activate) {
		reg |= 1 << 3;
	} else {
		reg &= ~(1 << 3);
	}

	if (NRF_WriteRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_SetCRCWidth(nrf24l01_dev* dev, NRF_CRC_WIDTH width) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	if (width == NRF_CRC_WIDTH_2B) {
		reg |= 1 << 2;
	} else {
		reg &= ~(1 << 3);
	}

	if (NRF_WriteRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	dev->CRC_WIDTH = width;
	return NRF_OK;
}

NRF_RESULT NRF_PowerUp(nrf24l01_dev* dev, uint8_t powerUp) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	if (powerUp) {
		reg |= 1 << 1;
	} else {
		reg &= ~(1 << 1);
	}

	if (NRF_WriteRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	uint8_t config = 0;

	while((config&2) == 0) {	// wait for powerup
		NRF_ReadRegister(dev, NRF_CONFIG, &config);
	}
	return NRF_OK;
}

NRF_RESULT NRF_RXTXControl(nrf24l01_dev* dev, NRF_TXRX_STATE rx) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	if (rx) {
		reg |= 1;
	} else {
		reg &= ~(1);
	}

	if (NRF_WriteRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_EnableRXDataReadyIRQ(nrf24l01_dev* dev, uint8_t activate) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}

	if (!activate) {
		reg |= 1 << 6;
	} else {
		reg &= ~(1 << 6);
	}

	if (NRF_WriteRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_EnableTXDataSentIRQ(nrf24l01_dev* dev, uint8_t activate) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	if (!activate) {
		reg |= 1 << 5;
	} else {
		reg &= ~(1 << 5);
	}
	if (NRF_WriteRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_EnableMaxRetransmitIRQ(nrf24l01_dev* dev, uint8_t activate) {
	uint8_t reg = 0;
	if (NRF_ReadRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	if (!activate) {
		reg |= 1 << 4;
	} else {
		reg &= ~(1 << 4);
	}
	if (NRF_WriteRegister(dev, NRF_CONFIG, &reg) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}


NRF_RESULT NRF_SetRXAddress_P1(nrf24l01_dev* dev, uint8_t* address) {
	uint8_t rx[5]={0};
	if (NRF_SendCommand(dev, NRF_CMD_W_REGISTER | NRF_RX_ADDR_P1, address, rx, 5) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_SetRXAddress_P0(nrf24l01_dev* dev, uint8_t* address) {
	uint8_t rx[5]={0};
	if (NRF_SendCommand(dev, NRF_CMD_W_REGISTER | NRF_RX_ADDR_P0, address, rx, 5) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}

NRF_RESULT NRF_SetTXAddress(nrf24l01_dev* dev, uint8_t* address) {
	uint8_t tx[5]={0};
	if (NRF_SendCommand(dev, NRF_CMD_W_REGISTER | NRF_TX_ADDR, address, tx, 5) != NRF_OK) {
		return NRF_ERROR;
	}
	return NRF_OK;
}


NRF_RESULT NRF_SetRXPayloadWidth_P1(nrf24l01_dev* dev, uint8_t width) {
	width &= 0x3F;
	if (NRF_WriteRegister(dev, NRF_RX_PW_P1, &width) != NRF_OK) {
		//dev->PayloadLength = 0;
		return NRF_ERROR;
	}
	//dev->PayloadLength = width;
	return NRF_OK;
}

NRF_RESULT NRF_SetRXPayloadWidth_P0(nrf24l01_dev* dev, uint8_t width) {
	width &= 0x3F;
	if (NRF_WriteRegister(dev, NRF_RX_PW_P0, &width) != NRF_OK) {
		//dev->PayloadLength = 0;
		return NRF_ERROR;
	}
	//dev->PayloadLength = width;
	return NRF_OK;
}

NRF_RESULT NRF_SendPacket(nrf24l01_dev* dev, uint8_t* data) {
	dev->BUSY_FLAG = 1;

	NRF_CE_RESETPIN(dev);
	if ( NRF_RXTXControl(dev, NRF_STATE_TX) == NRF_ERROR ) {
		NRF_CE_SETPIN(dev);
		dev->BUSY_FLAG = 0;
		return NRF_ERROR;
	}


	if (NRF_WriteTXPayload(dev, data) == NRF_ERROR ){
		NRF_CE_SETPIN(dev);
		dev->BUSY_FLAG = 0;
		return NRF_ERROR;
	}
	NRF_CE_SETPIN(dev);
	return NRF_OK;
}


/*
NRF_RESULT NRF_ReceivePacket(nrf24l01_dev* dev, uint8_t* data) {
	dev->BUSY_FLAG = 1;

	NRF_CE_RESETPIN(dev);
	NRF_RXTXControl(dev, NRF_STATE_RX);
	NRF_CE_SETPIN(dev);

	while (dev->BUSY_FLAG == 1) {;}	// wait for reception

	int i = 0;
	for (i = 0; i < dev->PayloadLength; i++) {
		data[i] = dev->RX_BUFFER[i];
	}

	return NRF_OK;
}
*/

NRF_RESULT NRF_PushPacket(nrf24l01_dev* dev, uint8_t* data) {

	if(dev->BUSY_FLAG==1) {
		NRF_FlushTX(dev);
	} else {
		dev->BUSY_FLAG = 1;
	}
	NRF_CE_RESETPIN(dev);
	NRF_RXTXControl(dev, NRF_STATE_TX);
	NRF_WriteTXPayload(dev, data);
	NRF_CE_SETPIN(dev);

	return NRF_OK;
}

NRF_RESULT NRF_PullPacket(nrf24l01_dev* dev, uint8_t* data) {
	int i = 0;
	for (i = 0; i < NRF_PAYLOAD_LENGTH; i++) {
		data[i] = dev->RX_BUFFER[i];
	}


	return NRF_OK;
}

