#include <stdio.h>
#include <stdlib.h>
#include "wav.h"

using namespace codeLibraryRiK;

int test ( int argc, const char *argv[] )
{
	int	NumFrames = 0;
	int	BurstFrame = 0;
	double	FrameRate = 25;	/* PAL */
	int	LinearDolby = 1;
	int	num_sample_pairs = 0;
	FILE	*output = NULL;

	if ( argc != 5 )
	{
		printf ( "Incorrect number of arguments!\n" );
		printf ( "Arguments are: CreateWave.exe <output filename> [Num frames] [Burst frame] [frame rate]\n" );
		printf ( "\tValid frame rates are 23.976, 24, 25, 29.97, 50, 59.94 \n" );

		return ( -1 );
	}
	else
	{
		if ( (output = fopen (argv[1], "wb")) == NULL )
		{
			printf ( "Unable to open output file!\n" );
		}

		NumFrames = atoi ( argv[2] );

		BurstFrame = atoi ( argv[3] ) - 1; /* zero indexed */

		FrameRate = atof ( argv[4] );
	}

	/* First need to write RIFF header chunk */
	num_sample_pairs = WriteRIFFChunk ( output, FrameRate, NumFrames );

	/* Next write WAV ID chunk */
	WriteWAVChunk ( output );

	/* Next write data (samples into file) */
	{
		unsigned int	word = 0, temp_long = 0;

		word = data_MARKER;
		fwrite ( &word, sizeof (word), 1, output );
		temp_long = num_sample_pairs * 2 * 3;
		word = H2LE_INT ( temp_long );
		fwrite ( &word, sizeof (word), 1, output );

		for ( int index = 0; index < NumFrames; index++ )
		{
			int		blocks_in_frame = GetNumAESFrames ( FrameRate, index );
			unsigned char	sample[6] = { 0, 0, 0, 0, 0, 0 };

			if (index == BurstFrame)
			{
				sample[0] = 0xaa;
				sample[1] = 0xaa;
				sample[2] = 0xaa;
				sample[3] = 0xaa;
				sample[4] = 0xaa;
				sample[5] = 0xaa;
			}

			for ( int index2 = 0; index2 < blocks_in_frame; index2++ )
			{
				for ( int i = 0; i < 6; i++ )
				{
					fwrite ( &sample[i], sizeof (unsigned char), 1, output );
				}
			}
		}
	}

	return ( 0 );
}

int GetNumAESFrames ( double frame_rate, int FRAME_NUM )
{
	switch ( (int)frame_rate )
	{
		case 29:
			switch ( FRAME_NUM % 5 )
			{
				case 1:
				case 3:
					return ( 1601 );
					break;
				default:
					return ( 1602 );
					break;
				
			}
			break;
		case 59:
			switch ( FRAME_NUM % 10 )
			{
				case 3:
				case 7:
					return ( 800 );
					break;
				default:
					return ( 801 );
					break;
			}
		case 50:
			return ( 960 );
		case 24:
			return ( 2000 );
		case 23:
			return ( 2002 );
		case 25:
		default:
			return ( 1920 );
			break;
	}
}

int WriteRIFFChunk ( FILE *output, double frame_rate, int NumberOfFrames )
{
	unsigned int	word;
	int		AES_frames = 0;

	word = RIFF_MARKER;
	fwrite ( &word, sizeof (word), 1, output );

	switch ( (int)frame_rate )
	{
		case 29:
			AES_frames = ( (NumberOfFrames / 5) * 8008 );
			switch ( NumberOfFrames % 5 )
			{
				case 4:
					AES_frames += 1601;
				case 3:
					AES_frames += 1602;
				case 2:
					AES_frames += 1601;
				case 1:
					AES_frames += 1602;
				case 0:
				default:
					break;
			}
			break;
		case 59:
			AES_frames = ( (NumberOfFrames / 10) * 8008 );
			switch ( NumberOfFrames % 10 )
			{
				case 9:
					AES_frames += 801;
				case 8:
					AES_frames += 800;
				case 7:
					AES_frames += 801;
				case 6:
					AES_frames += 801;
				case 5:
					AES_frames += 801;
				case 4:
					AES_frames += 800;
				case 3:
					AES_frames += 801;
				case 2:
					AES_frames += 801;
				case 1:
					AES_frames += 801;
				case 0:
				default:
					break;
			}
			break;
		case 25:
			AES_frames = NumberOfFrames * 1920;
			break;
		case 50:
			AES_frames = NumberOfFrames * 960;
			break;
		case 23:
			AES_frames = NumberOfFrames * 2002;
			break;
		case 24:
			AES_frames = NumberOfFrames * 2000;
			break;
	}

#if USE_EXTENSIBLE_WAV
	word = 4 + 48 + 12 + ( 8 + 3 /*24 bit*/ * 2 /*stereo*/ * (unsigned int)AES_frames );
#else
	word = 4 + 24 + ( 8 + 3 /*24 bit*/ * 2 /*stereo*/ * (unsigned int)AES_frames );
#endif
	word = H2LE_INT ( word );
	fwrite ( &word, sizeof (word), 1, output );

	return ( AES_frames );
}

void WriteExtensibleWAVChunk ( FILE *output )
{
	unsigned int	word = 0, temp_long = 0;
	short		a_short = 0, temp_short = 0;

	word = WAVE_MARKER;
	fwrite ( &word, sizeof (word), 1, output );

	word = fmt_MARKER;			/* ckID */
	fwrite ( &word, sizeof (word), 1, output );

	word = H2LE_INT ( 40 );			/* cksize */
	fwrite ( &word, sizeof (word), 1, output );

	a_short = H2LE_SHORT ( (short)0xFFFE ); /* wFormatTag */
	fwrite ( &a_short, sizeof (a_short), 1, output );

	a_short = H2LE_SHORT ( 2 );		/* nChannels */
	fwrite ( &a_short, sizeof (a_short), 1, output );

	word = H2LE_INT ( 48000 );		/* nSamplesPerSec */
	fwrite ( &word, sizeof (word), 1, output );

	temp_long = 48000 * 3 * 2;
	word = H2LE_INT ( temp_long );		/* nAvgBytesPerSec */
	fwrite ( &word, sizeof (word), 1, output );

	temp_short = 3 * 2;
	a_short = H2LE_SHORT ( temp_short );	/* nBlockAlign	*/
	fwrite ( &a_short, sizeof (a_short), 1, output );

	temp_short = 8 * 3;
	a_short = H2LE_SHORT ( temp_short );	/* wBitsPerSample */
	fwrite ( &a_short, sizeof (a_short), 1, output );

	a_short = H2LE_SHORT ( 22 );		/* cbSize - 22 */
	fwrite ( &a_short, sizeof (a_short), 1, output );

	a_short = H2LE_SHORT ( 24 );		/* wValidBitsPerSample */
	fwrite ( &a_short, sizeof (a_short), 1, output );

	word = 0;			/* dwChannelMask */
	fwrite ( &word, sizeof (word), 1, output );

	/* then 16 bytes of the SubFormat */
	word = H2LE_INT ( 0x0001 );	/* us WAVE_FORMAT_PCM */
	fwrite ( &word, sizeof (word), 1, output );
	word = H2LE_INT ( 0x0000 );
	fwrite ( &word, sizeof (word), 1, output );
	word = H2LE_INT ( 0x0000 );
	fwrite ( &word, sizeof (word), 1, output );
	word = H2LE_INT ( 0x1000 );
	fwrite ( &word, sizeof (word), 1, output );
	word = H2LE_INT ( 0x8000 );
	fwrite ( &word, sizeof (word), 1, output );
	word = H2LE_INT ( 0x00AA );
	fwrite ( &word, sizeof (word), 1, output );
	word = H2LE_INT ( 0x0038 );
	fwrite ( &word, sizeof (word), 1, output );
	word = H2LE_INT ( 0x9B71 );
	fwrite ( &word, sizeof (word), 1, output );
}

void WriteSimpleWAVChunk ( FILE *output )
{
	unsigned int	word = 0, temp_long = 0;
	short		a_short = 0, temp_short = 0;

	word = WAVE_MARKER;
	fwrite ( &word, sizeof (word), 1, output );

	word = fmt_MARKER;		/* ckID */
	fwrite ( &word, sizeof (word), 1, output );

	word = H2LE_INT ( 16 );		/* cksize */
	fwrite ( &word, sizeof (word), 1, output );

	a_short = H2LE_SHORT ( (short)0x0001 ); /* wFormatTag */
	fwrite ( &a_short, sizeof (a_short), 1, output );

	a_short = H2LE_SHORT ( 2 );		/* nChannels */
	fwrite ( &a_short, sizeof (a_short), 1, output );

	word = H2LE_INT ( 48000 );		/* nSamplesPerSec */
	fwrite ( &word, sizeof (word), 1, output );

	temp_long = 48000 * 3 * 2;
	word = H2LE_INT ( temp_long );		/* nAvgBytesPerSec */
	fwrite ( &word, sizeof (word), 1, output );

	temp_short = 3 * 2;
	a_short = H2LE_SHORT ( temp_short );	/* nBlockAlign	*/
	fwrite ( &a_short, sizeof (a_short), 1, output );

	temp_short = 8 * 3;
	a_short = H2LE_SHORT ( temp_short );	/* wBitsPerSample */
	fwrite ( &a_short, sizeof (a_short), 1, output );
}
