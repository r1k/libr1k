#pragma once

#include <stdint.h>
#include "libr1k.h"

namespace libr1k
{
	const uint16_t AC3_CRC_POLY = 0x8005;
	uint16_t calc_16_CRC(uint16_t CRC_POLY, const uint8_t *buf, int length);
}
