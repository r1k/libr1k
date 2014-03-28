#pragma once

#include "libr1k.h"
#include "Types.h"

namespace libr1k
{

    class bitStreamReader
	{
		private:
			const uint8_t*      originalStart;
			unsigned long		originalLength;
			const uint8_t*		BitStreamSource;
			uint8_t				bitOffset;
			unsigned long		BitStreamLength;

		public:
			bitStreamReader(const uint8_t *source, const unsigned long length) : 
				originalStart(source),
				originalLength(length),
				BitStreamSource(source), 
				bitOffset(0), 
				BitStreamLength(length) {};

			int GetXBits ( unsigned long* ReturnData, uint8_t numBits);
			int GetXBits ( unsigned int* ReturnData, uint8_t numBits) { return this->GetXBits((unsigned long*)ReturnData, numBits);};
			void JumpToNextByteBoundary ( void );
			int GetNumBytesConsumed(void) {
				int length = BitStreamSource - originalStart;
				if (bitOffset)
					length++;
				return length;
			}
	};
}
