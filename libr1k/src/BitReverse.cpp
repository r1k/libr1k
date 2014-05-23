#include "utils\BitReverse.h"
#include "utils\bitOperators.h"

namespace libr1k
{
	inline unsigned int BitReverse8 ( const unsigned int input)
	{
#if 0
		unsigned int reversed = input;
		
		reversed = ((reversed & 0x000000f0) >> 4) | ((reversed & 0x0000000f) << 4);
		reversed = ((reversed & 0x000000cc) >> 2) | ((reversed & 0x00000033) << 2);
		reversed = ((reversed & 0x000000aa) >> 1) | ((reversed & 0x00000055) << 1);
		
		return reversed;
#else
		return BitReverse8Bit[input];
#endif
	}


	unsigned int BitReverse16( const unsigned int input)
	{
		unsigned int newLsb = 0, newMsb = 0, recombined = 0;
		
		newLsb = BitReverse8(BYTE_1(input));
		newMsb = BitReverse8(BYTE_0(input));
		
		recombined = (newMsb << 8) | newLsb;
		
		return recombined;
	}

	unsigned int BitReverse20( const unsigned int input)
	{
#if 0
		unsigned int newUpper8 = 0, newLower8 = 0, middle4 = 0, recombined = 0;
		
		newUpper8 = BitReverse8( (input & 0x0ff) );
		newLower8 = BitReverse8( (input & 0x0ff000) >> 12);
		middle4 = BitReverse8( (input & 0x00f00) >> 6 );	// make the 4 bits we want the middle of the 8 bits to be reversed  ie 0x00f00 becomes 0x0003C0
		
		recombined = (newUpper8 << 12) | (middle4 << 6) | newLower8;
		
		return recombined;
#else
		return BitReverseX( input, 20 );
#endif
	}

	unsigned int BitReverse24( const unsigned int input)
	{
#if 0
		unsigned int newLower8 = 0, middle8 = 0, newUpper8 = 0, recombined;
		
		newLower8 	= BitReverse8(BYTE_2(input));
		middle8 	= BitReverse8(BYTE_1(input));
		newUpper8	= BitReverse8(BYTE_0(input));
		
		recombined	= (newUpper8 << 16) | (middle8 << 8) | newLower8;
		
		return recombined;
#else
		return BitReverseX( input, 24 );
#endif
	}

	unsigned int BitReverse32( const unsigned int input)
	{
		unsigned int newLower16 = 0, newUpper16 = 0, recombined = 0;
		
		newLower16 = BitReverse16(WORD16_1(input));
		newUpper16 = BitReverse16(WORD16_0(input));
		
		recombined = (newUpper16 << 16) | newLower16;
		
		return recombined;
	}

	unsigned int BitReverseX ( const unsigned int input, int numBits)
	{
		int temp = BitReverse32( input );
		temp = temp >> (32 - numBits);
		return temp;
	}

	int SignExtendToInt( const unsigned int input, int sourceBitWidth)
	{
		// assume input is 00xaaa and x is at the bitwidth and 0's to the left
		int temp1 = input << (32 - sourceBitWidth);
		int signedinput = (temp1 >> (32 - sourceBitWidth));

		return signedinput;
	}
}