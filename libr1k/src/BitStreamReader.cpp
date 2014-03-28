#include "BitStreamReader.h"

namespace libr1k
{
	int bitStreamReader::GetXBits ( unsigned long* ReturnData, uint8_t numBits)
	{
		unsigned int	mask = 0;
		unsigned int	temp = 0;
		unsigned int	fullBytesUsed = 0;
		const uint8_t*	localBitStreamSource_ptr = BitStreamSource;

		if ( !numBits )
		{ 
			// Asked for 0 bits !?!?
			return -1;
		}

		if ( numBits > ((BitStreamLength * 8) - bitOffset) )
		{
			// Asked for more bits than we have !?!?
			return -1;
		}

		temp = localBitStreamSource_ptr[0] << 24;
		temp |= localBitStreamSource_ptr[1] << 16;
		temp |= localBitStreamSource_ptr[2] << 8;
		temp |= localBitStreamSource_ptr[3];

		temp = temp >> ( 32 - (numBits + bitOffset) );

		for ( int i = 0; i < numBits; i++ )
		{
			mask = mask << 1;
			mask |= 1;
		}

		*ReturnData = temp & mask;

		fullBytesUsed = ( (bitOffset + numBits) / 8 );
		BitStreamLength -= fullBytesUsed;	// remove any whole bytes used
		bitOffset = ( bitOffset + numBits ) % 8;
		BitStreamSource = &( localBitStreamSource_ptr[fullBytesUsed] );
		return 0;
	}

	void bitStreamReader::JumpToNextByteBoundary ( void )
	{
		bitOffset = 0;
		BitStreamSource++;
	}
}