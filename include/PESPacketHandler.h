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

    class PESPacketHandler : public TSPacketHandler
	{
	public:

		PESPacketHandler(ofstream **str, bool Debug_on = false);
		PESPacketHandler(const char *str, bool Debug_on = false);
		~PESPacketHandler(void);

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
