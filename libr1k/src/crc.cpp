#include "crc.h"

namespace libr1k
{
#if 0
	unsigned int crc16(uint16_t POLYNOM, uint16_t crcValue, uint8_t newByte)
	{
		unsigned char i;

		for (i = 0; i < 8; i++) {

			if (((crcValue & 0x8000) >> 8) ^ (newByte & 0x80)){
				crcValue = (crcValue << 1) ^ POLYNOM;
			}
			else{
				crcValue = (crcValue << 1);
			}

			newByte <<= 1;
		}

		return crcValue;
	}

	uint16_t calc_16_CRC(uint16_t CRC_POLY, const uint8_t *data, int size)
	{
		int i = 0;
		uint16_t crc = 0;
		while (i < size){
			crc = crc16(AC3_CRC_POLY, crc, *data++);
			i++;
		}
		return crc;
	}

#else
	uint16_t calc_16_CRC(uint16_t CRC_POLY, const uint8_t *data, int size)
	{
		uint16_t out = 0;
		int bits_read = 0, bit_flag;

		/* Sanity check: */
		if (data == nullptr)
			return 0;

		while (size > 0)
		{
			bit_flag = out >> 15;

			/* Get next bit: */
			out <<= 1;
			out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

			/* Increment bit counter: */
			bits_read++;
			if (bits_read > 7)
			{
				bits_read = 0;
				data++;
				size--;
			}

			/* Cycle check: */
			if (bit_flag)
				out ^= CRC_POLY;

		}

		// item b) "push out" the last 16 bits
		int i;
		for (i = 0; i < 16; ++i) {
			bit_flag = out >> 15;
			out <<= 1;
			if (bit_flag)
				out ^= CRC_POLY;
		}

		// item c) reverse the bits
		uint16_t crc = 0;
		i = 0x8000;
		int j = 0x0001;
		for (; i != 0; i >>= 1, j <<= 1) {
			if (i & out) crc |= j;
		}

		return crc;
	}
#endif
}