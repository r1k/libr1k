#pragma once

#include "libr1k.h"
#include "TSPacketHandler.h"
#include "TransportPacket.h"
#include "Log.h"
#include "BitReverse.h"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

namespace libr1k
{
#define MAX_FILENAME_LENGTH 1024
    class ESDumpPacketHandler : public TSPacketHandler
	{
	public:
		
		ESDumpPacketHandler(ofstream **str, bool Debug_on = false);
		ESDumpPacketHandler(const char *str, bool Debug_on = false);
		~ESDumpPacketHandler(void);

		virtual void PESDecode ( PESPacket_t *buf );

		void SetDebugOutput ( bool On );

		int FrameCount;

	protected:

		ofstream *outStream;

	private:

		FILE		*OutputFile;
		
		char base_filename[MAX_FILENAME_LENGTH];
		bool filePerPes;
		bool DebugOn;
		unsigned int index;
		Log *LogFile;
	};
}
