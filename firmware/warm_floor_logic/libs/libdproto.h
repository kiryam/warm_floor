#pragma once

#include "stdint.h"

#define DPROTO_VERSION 120

#define DPROTO_MAGIC_SEC_LEN 4

/*
 * MAGIC_SEQ_0 ... MAGIC_SEQ_N DATA_SIZE_BYTE_3 DATA_SIZEE_BYTE_2 DATA_SIZE_BYTE_1 DATA_SIZE_BYTE_0 DATA_BYTE_0 ... DATA_BYTE_N CRC_BYTE MAGIC_SEQ_N ... MAGIC_SEQ_0
 *
 */

int dproto_encode(uint8_t magic_sequence[4], uint8_t* in, int32_t data_size, uint8_t* out);
int dproto_process_byte(uint8_t* buff, uint32_t* length, uint8_t* magic_sequence, uint8_t byte, uint64_t byte_number);
