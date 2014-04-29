#pragma once

#include "libr1k.h"
#include "TSPacketHandler.h"
#include "TransportPacket.h"
#include "WAVFile.h"
#include "Log.h"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

namespace libr1k
{
	const unsigned int DTS_SYNCWORD = 0x7FFE8001;
	enum {DTS_TYPE_1 = 512, DTS_TYPE_2 = 1024, DTS_TYPE_3 = 2048, DTS_TYPE_ERROR};
	const unsigned char AMODE_CHANNELS[] = {1,2,2,2,2,3,3,4,4,5,6,6,6,7,8,8};
	enum DOWNMIX_TYPE {DTSDOWNMIXTYPE_1_0, DTSDOWNMIXTYPE_LoRo, DTSDOWNMIXTYPE_LtRt, DTSDOWNMIXTYPE_3_0, DTSDOWNMIXTYPE_2_1, DTSDOWNMIXTYPE_2_2, DTSDOWNMIXTYPE_3_1};

    class DTSMetadata
	{
	public:
		DTSMetadata( unsigned char *DTSFrame );
		~DTSMetadata(void ){};
		void SearchForAuxData( unsigned char *Frame );
	public:
		unsigned int SYNCWORD, FTYPE;
		unsigned int NBLKS, FSIZE, AMODE, SFREQ, RATE, MIX, DYNF, TIMEF, AUXF, HDCD;
		unsigned int EXT_AUDIO_ID, EXT_AUDIO, ASPF, LFF, HFLAG, HCRC, FILTS, VERNUM;
		unsigned int CHIST, PCMR, SUMF, SUMS, DIALNORM;
		unsigned int DTS_FRAME_TYPE;

		bool AUXTimeStampFlag;
		bool AUXDynamCoeffFlag;

		unsigned long PrmChDownMixType;
		unsigned long NumDwnMixCodeCoeffs;
		unsigned long NumChPrevHierChSet;
		bool AuxDataFound;

		friend ostream& operator << (ostream& os, const DTSMetadata& meta);

		
	private:
		void DeriveNumDwnMixCodeCoeffs(void);
	};

	


    class DTSPacketHandler : public TSPacketHandler
	{
	public:
		DTSPacketHandler(ofstream *str, bool Debug_on = false);
		~DTSPacketHandler(void);

        virtual bool DecodeFrame(unsigned char **Frame, unsigned int *FrameSize);
		virtual void PESDecode ( PESPacket_t *buf );

		void SetDebugOutput ( bool On );

		int FrameCount;

	protected :

		ofstream *outStream;

	private :

		wav_params	WAVParams;
		WAVFile		*OutputFile;

		bool DebugOn;
		Log *LogFile;
	};
}
