#pragma once

#include "libr1k.h"

namespace libr1k
{
#define BYTE_0( X )	(  (X) & 0x00000000ff )
#define BYTE_1( X )	( ((X) & 0x000000ff00) >> 8 )
#define BYTE_2( X )	( ((X) & 0x0000ff0000) >> 16 )
#define BYTE_3( X ) 	( ((X) & 0x00ff000000) >> 24 )

#define WORD16_0( X )	(  (X) & 0x0000ffff )
#define WORD16_1( X )	( ((X) & 0xffff0000) >> 16 )
}
