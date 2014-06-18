#pragma once

#include <fstream>
#include "Types.h"
#include "libr1k.h"
#include "_no_copy.h"
#include "SampleBuffer.h"

using namespace std;
namespace libr1k
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

    class wav_params
	{
	public:
		wav_params( void ){};
		wav_params(const wav_params& obj) ;
		~wav_params( void ){};
		unsigned int bit_depth;
		unsigned int num_samples;
		unsigned int num_channels;
		unsigned int SamplesPerSec;

		wav_params& operator= (const wav_params& rhs);
	private:

		wav_params& CopyObj(const wav_params& obj);
	};

    class AES_Header
	{
	public:
		AES_Header( unsigned int depth, unsigned int data_type, unsigned int num_samples );
		~AES_Header( void ){};
		unsigned int Pa;
		unsigned int Pb;
		unsigned int Pc;
		unsigned int Pd;
	};

	
    class WAVFile : public _no_copy
	{
	public:
        WAVFile(ofstream *outstr, wav_params &wv_params);
		WAVFile(ifstream *instr);
		~WAVFile ();

        virtual void WriteSample(const uint32_t sample);
        virtual void WriteSample16(const uint16_t sample);

        void GetParams ( wav_params *wv_params );
	
		bool InputReady;
		
		// Wrapper for the horrid syntax of calling a member function pointer
        inline int32_t ReadSample(void) { return (this->*this->Read__Sample)(); };

        void ReadSamples(SampleBuffer &buf, const int num);
        void ReadSamples(SampleBuffer_norm &buf, const int num);

        void WriteSamples(SampleBuffer &buf);

		unsigned int ReadCount;
		int writtenCount;

	private:

		WAVFile() : 
            output(nullptr),
            input(nullptr),
            fileSize(0),
            InputReady(false),
            ReadCount(0),
            local_params() {}

		ofstream * const output;
		ifstream * const input;
        wav_params local_params;

		
		int ByteShift;

		unsigned long fileSize;

#ifdef WIN32
		std::ostream::pos_type file_size_location;
        std::ostream::pos_type data_chunk_size_location;
#else
		int  file_size_location;
        int data_chunk_size_location;
#endif

        int32_t (WAVFile::*Read__Sample) (void);

		void	WriteExtensibleWAVChunk ( void );
		void	WriteSimpleWAVChunk ( void );
		void	WriteRIFFChunk ( void );

		uint32_t ReadWord ( void );
		uint16_t ReadShort ( void );

		void	ReadRIFFChunk ( void );
		void	ReadWAVChunk ( void );
		void	ReadExtensibleChunk( void );
		void	ReadSimpleChunk( void );


		
        // Alll these return 32 bit values with the 
        // samples left shifted up to the far left
        int32_t ReadSample16(void);
        int32_t ReadSample20(void);
        int32_t ReadSample24(void);
	};
}
