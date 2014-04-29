#pragma once

#include "libr1k.h"
#include "TSPacketHandler.h"
#include "TransportPacket.h"
#include "WAVFile.h"
#include "Log.h"
#include "BitReverse.h"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

namespace libr1k
{
    class AES3_data_header
	{
	public:

		AES3_data_header( unsigned char *DTSFrame );
		~AES3_data_header(void ){};
	
	public:

		static const int byteSize = 4;
		int audioPacketSize;
		int numChannels;
		int bitDepth;
		int channelID;
		int alignmentBits;

	private:
		
	};

    class LPCMPacketHandler : public TSPacketHandler
	{
	public:

		LPCMPacketHandler(ofstream *str, bool Debug_on = false);
		~LPCMPacketHandler(void);

		virtual bool DecodeLPCMFrame( unsigned char **LPCMFrame, unsigned int *BufferSize  );
		virtual void PESDecode ( PESPacket_t *buf );

		void SetDebugOutput ( bool On );

		int FrameCount;

	protected:

		ofstream *outStream;

	private:

		wav_params	WAVParams;
		WAVFile		*OutputFile;

		bool DebugOn;
		Log *LogFile;
	};
}
