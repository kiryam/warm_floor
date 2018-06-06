#include "libdproto.h"
#include "string.h"


int dproto_encode(uint8_t magic_sequence[DPROTO_MAGIC_SEC_LEN], uint8_t* in, int32_t data_size, uint8_t* out) {
	uint8_t crc = 0;
	memcpy(out, magic_sequence, DPROTO_MAGIC_SEC_LEN);
	out[DPROTO_MAGIC_SEC_LEN] =   ( data_size >> 0  ) & 0xffffffff;
	out[DPROTO_MAGIC_SEC_LEN+1] = ( data_size >> 8  ) & 0xffffffff;
	out[DPROTO_MAGIC_SEC_LEN+2] = ( data_size >> 16 ) & 0xffffffff;
	out[DPROTO_MAGIC_SEC_LEN+3] = ( data_size >> 24 ) & 0xffffffff;

	memcpy(&out[DPROTO_MAGIC_SEC_LEN+4], in, data_size);

	for (int i=0; i<data_size; i++) {
		crc += in[i];
	}

	out[DPROTO_MAGIC_SEC_LEN+4+data_size] = crc;
	return DPROTO_MAGIC_SEC_LEN+4+data_size+1;
}

int dproto_process_byte(uint8_t* buff, uint32_t* length, uint8_t* magic_sequence, uint8_t byte, uint64_t byte_number) {
	if (byte_number < DPROTO_MAGIC_SEC_LEN) { // checking sequence
		if (byte != magic_sequence[byte_number] ) {
			return -1;
		}
		return 0;
	} else if (byte_number < DPROTO_MAGIC_SEC_LEN + 4 ) { // reading length
		*length |= byte << (8 * (byte_number-DPROTO_MAGIC_SEC_LEN));
		return 0;
	} else if (byte_number < DPROTO_MAGIC_SEC_LEN + 4 + *length ){ // reading data
		buff[byte_number-DPROTO_MAGIC_SEC_LEN-4] = byte;
		return 0;
	} else if ( byte_number < DPROTO_MAGIC_SEC_LEN + 4 + *length + 1) { // reading CRC
		uint8_t crc = 0;
		for(int i=0; i< *length; i++) { // calculating CRC
			crc += buff[i];
		}

		if (crc == byte) { // CRC match
			return 1;
		}
	}

	return -1;
}
