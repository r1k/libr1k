#pragma once

#include "libr1k.h"
#include "TSPacketHandler.h"
#include "TransportPacket.h"
#include "utils\Log.h"
#include "utils\BitReverse.h"
#include "pcr.h"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

namespace libr1k
{
#define MAX_FILENAME_LENGTH 1024

    class PTSPacketHandler : public TSPacketHandler
	{
	public:

		PTSPacketHandler(ofstream **str, pcr *, bool Debug_on = false);
		~PTSPacketHandler(void);

		virtual void PESDecode ( PESPacket_t *buf );

		void SetDebugOutput ( bool On );

		virtual void NextPacket( TransportPacket *tsPacket );
		virtual bool NewPESPacketFound(PESPacket_t *pPacket, TransportPacket *tsPacket);
		virtual bool ContinueLastPESPacket(PESPacket_t *pPacket, TransportPacket *tsPacket);

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
		pcr *PCR;
	};
}
