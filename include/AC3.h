#pragma once

#include "libr1k.h"
#include "esPacketDecoder.h"
#include "TSPacketHandler.h"
#include "TransportPacket.h"
#include "stdint.h"
#include "WAVFile.h"
#include <ostream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "pcr.h"
#include "Log.h"

namespace libr1k
{
    class liba52_wrapper;

    class au_ac3_t : public au_esPacket_t
    {
    public:

        au_ac3_t(shared_ptr<Log> log = nullptr);
        au_ac3_t(const uint8_t * const buf, const int bufSize, shared_ptr<Log> log = nullptr);
        au_ac3_t(const DataBuffer<uint8_t> * buffer, shared_ptr<Log> log = nullptr);

        ~au_ac3_t() {}
        
        enum { AU_AC3_OKAY, AU_AC3_INSUFFICIENT_DATA, AU_AC3_SYNC_NOT_FOUND };

        void InterpretFrame();

        virtual bool preProcessHeader(const uint8_t *data, const int length, int& frame_length) const;

        virtual int decode();

        virtual ostream& write(std::ostream &os);
        virtual ostream& write_csv(std::ostream &os);
        virtual ostream& write_csv_header(std::ostream &os);

        uint64_t PTS;

    protected:

        int bufferSize;
        bool syncFound;
        bool syncProcessed;

        shared_ptr<liba52_wrapper> liba52;

        static const uint16_t syncword = 0xB77;
        static const int frmsizcod_max = 37;
        static const int frmsizcod_table[3][frmsizcod_max + 1];
        static const int bitrate_table[frmsizcod_max + 1];
        static const int additional_bsi_max = 2000;

        uint16_t CRC1;
        unsigned int fscod;
        unsigned int frmsizcod;
        unsigned int bsid;
        unsigned int bsmod;
        unsigned int acmod;
        unsigned int cmixlev;
        unsigned int surmixlev;
        unsigned int dsurmod;
        unsigned int lfeon;
        unsigned int dialnorm;
        unsigned int compre;
        unsigned int compr;
        unsigned int langcode;
        unsigned int langcod;
        unsigned int audprodie;
        unsigned int mixlevel;
        unsigned int roomtyp;
        unsigned int dialnorm2;
        unsigned int compr2e;
        unsigned int compr2;
        unsigned int langcod2e;
        unsigned int langcod2;
        unsigned int audprodi2e;
        unsigned int mixlevel2;
        unsigned int roomtyp2;
        unsigned int copyrightb;
        unsigned int origbs;
        unsigned int xbsi1e;
        unsigned int dmixmod;
        unsigned int ltrtcmixlev;
        unsigned int ltrtsurmixlev;
        unsigned int lorocmixlev;
        unsigned int lorosurmixlev;
        unsigned int xbsi2e;
        unsigned int dsurexmod;
        unsigned int dheadphonmod;
        unsigned int adconvtyp;
        unsigned int xbsi2;
        unsigned int encinfo;
        unsigned int addbsie;
        unsigned int addbsil;
        unsigned int addbsi[additional_bsi_max];
        uint16_t CRC2;

        static const char *fscod_str[];
        static const char *bsmod_str[];
        static const char *acmod_str[];
        static const char *cmixlev_str[];
        static const char *surmixlev_str[];
        static const char *dsurmod_str[];
        static const char *on_off_str[];
        static const char *roomtyp_str[];
    };
 
    class AC3Decoder : public esPacketDecoder
    {
    public:
        AC3Decoder() :
            ac3_frame(nullptr) {}
        virtual ~AC3Decoder() {}

    protected:
        shared_ptr<au_ac3_t> ac3_frame;

    private:

    };
	
    class AC3PacketHandler : public TSPacketHandler
	{
	public:
		AC3PacketHandler(ofstream **str, bool Debug_on = false);
        ~AC3PacketHandler(void) {}

		virtual void DecodeAC3Frame(const uint64_t PTS);
		virtual void PESDecode(PESPacket_t *buf);

		void SetDebugOutput(bool On);

		int FrameCount;

	protected:

        ofstream *outStream;

        uint64_t firstPTS;
        shared_ptr<au_ac3_t> ac3Decoder;

        bool DebugOn;
        bool PacketSpansPES;
        shared_ptr<Log> LogFile;

	private:
	};
}