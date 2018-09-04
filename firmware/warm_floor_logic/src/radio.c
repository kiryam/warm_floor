#include "stdint.h"
#include "main.h"
#include "radio.h"
#include "string.h"
#include "drivers/nRF24L01.h"
#include "libdproto.h"

#include "cmsis_os.h"

static uint8_t ADDR_MASTER[] = { 'm', 's', '0', '0', '1' };
static uint8_t ADDR_SLAVE[] = { 'n', 'R', 'F', '2', '5' };
static uint8_t magic_sequence[DPROTO_MAGIC_SEC_LEN] = {4, 8, 15, 16};

static uint8_t data[NRF_PAYLOAD_LENGTH] = {0};
static uint8_t data_encoded[NRF_PAYLOAD_LENGTH*6] = {0};

void Radio_On_Event(nrf24l01_dev* instance) {
	BaseType_t xHigherPriorityTaskWokenByPost = pdFALSE;
	//core.packets_rx++;
	NRF_PullPacket(instance, data);
	uint8_t length  = data[0];
	for(int i=0; i<length; i++) {
		xQueueGenericSendFromISR( core.radio_queue, &data[i+1], &xHigherPriorityTaskWokenByPost, queueSEND_TO_BACK );
	}

	portEND_SWITCHING_ISR(xHigherPriorityTaskWokenByPost);
}

int Radio_Init(Radio* radio) {
	radio->instance = NRF1_Init(ADDR_MASTER, ADDR_SLAVE, NRF_ADDR_WIDTH_5, NRF_CRC_WIDTH_2B, NRF_DATA_RATE_2MBPS, 2, 15, 15, NRF_TX_PWR_0dBm, (void*)Radio_On_Event);
	return 0;
}


int Radio_Send(Radio* radio, uint8_t* data, size_t size) {
	int bytes_to_sent = dproto_encode(magic_sequence, data, size, data_encoded);

	int bytes_sent = 0;
	while(bytes_sent < bytes_to_sent) {
		uint8_t packet[NRF_PAYLOAD_LENGTH] = {0};

		packet[0] = NRF_PAYLOAD_LENGTH-1;
		if ( bytes_to_sent - bytes_sent < (NRF_PAYLOAD_LENGTH-1)  ){
			packet[0] = bytes_to_sent - bytes_sent;
		}
		for(int i=0; i< packet[0]; i++) {
			packet[i+1] = data_encoded[bytes_sent++];
		}

		if (NRF_SendPacket(radio->instance, packet) != NRF_OK ) {
			//LED_On(&core.led1);
			return 1;
		}

		uint8_t retry_count = 10;
		while(radio->instance->BUSY_FLAG == 1){
			if(--retry_count == 0){
				// reset NRF?
				LED_On(&core.led1);
				return 1;
			}
			osDelay(10);
		}

	}

	return 0;
}
