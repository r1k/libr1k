#ifndef _WAV_H_
#define _WAV_H_
namespace codeLibraryRiK
{
	#define CPU_IS_LITTLE_ENDIAN	1
	#define CPU_IS_BIG_ENDIAN	0

#if ( CPU_IS_LITTLE_ENDIAN == 1 )
	#define MAKE_MARKER( a, b, c, d )	( (a) | ((b) << 8) | ((c) << 16) | ((d) << 24) )
#elif ( CPU_IS_BIG_ENDIAN == 1 )
	#define MAKE_MARKER( a, b, c, d )	( ((a) << 24) | ((b) << 16) | ((c) << 8) | (d) )
#else
	#error "Cannot determine endian-ness of processor."
#endif
#define RIFF_MARKER		( MAKE_MARKER ('R', 'I', 'F', 'F') )
	#define WAVE_MARKER		( MAKE_MARKER ('W', 'A', 'V', 'E') )
	#define fmt_MARKER		( MAKE_MARKER ('f', 'm', 't', ' ') )
	#define data_MARKER		( MAKE_MARKER ('d', 'a', 't', 'a') )
	#define ENDSWAP_SHORT( x )	( (((x) >> 8) & 0xFF) | (((x) & 0xFF) << 8) )
	#define ENDSWAP_INT( x )	( (((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) & 0xFF00) << 8) | (((x) & 0xFF) << 24) )
#if ( CPU_IS_LITTLE_ENDIAN == 1 )
	#define H2LE_SHORT( x ) ( x )
	#define H2LE_INT( x )	( x )
	#define LE2H_SHORT( x ) ( x )
	#define LE2H_INT( x )	( x )
	#define BE2H_INT( x )	ENDSWAP_INT ( x )
	#define BE2H_SHORT( x ) ENDSWAP_SHORT ( x )
	#define H2BE_INT( x )	ENDSWAP_INT ( x )
	#define H2BE_SHORT( x ) ENDSWAP_SHORT ( x )
#elif ( CPU_IS_BIG_ENDIAN == 1 )
	#define H2LE_SHORT( x ) ENDSWAP_SHORT ( x )
	#define H2LE_INT( x )	ENDSWAP_INT ( x )
	#define LE2H_SHORT( x ) ENDSWAP_SHORT ( x )	
	#define LE2H_INT( x )	ENDSWAP_INT ( x )
	#define BE2H_INT( x )	( x )
	#define BE2H_SHORT( x ) ( x )
	#define H2BE_INT( x )	( x )
	#define H2BE_SHORT( x ) ( x )
#else
	#error "Cannot determine endian-ness of processor."
#endif
	int	GetNumAESFrames ( double frame_rate, int FRAME_NUM );
	int	WriteRIFFChunk ( FILE *output, double frame_rate, int NumberOfFrames );

#undef USE_EXTENSIBLE_WAV
#if USE_EXTENSIBLE_WAV
	#define WriteWAVChunk( X )	WriteExtensibleWAVCHUNK ( X )
#else
	#define WriteWAVChunk( X )	WriteSimpleWAVChunk ( X )
#endif
	void	WriteExtensibleWAVChunk ( FILE *output );
	void	WriteSimpleWAVChunk ( FILE *output );

}

#endif