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
#include <memory>
#include <cstdarg>
#include "pcr.h"
#include "utils\Log.h"

namespace libr1k
{
    class liba52_wrapper;

    class au_ac3_t : public au_esPacket_t
    {
    public:

        au_ac3_t(shared_ptr<Log> log = nullptr);
        au_ac3_t(const uint8_t * const buf, const int bufSize, shared_ptr<Log> log = nullptr);
        au_ac3_t(DataBuffer<uint8_t> * const buffer, shared_ptr<Log> log = nullptr);

        ~au_ac3_t() {}
        
        enum { AU_AC3_OKAY, AU_AC3_INSUFFICIENT_DATA, AU_AC3_SYNC_NOT_FOUND };

        void InterpretFrame();

        virtual bool preProcessHeader();

        virtual int decode();

        virtual ostream& write(std::ostream &os);
        virtual ostream& write_csv(std::ostream &os);
        virtual ostream& write_csv_header(std::ostream &os);

        uint64_t PTS;

        enum { SYNCWORD = 0xB77 };
        enum { BSID = 8 };

    protected:

        uint8_t *buf;
        int bufLength;

        bool syncFound;
        bool syncProcessed;

        shared_ptr<liba52_wrapper> liba52;
               
        enum { frmsizcod_max = 37 };
        static const int frmsizcod_table[3][frmsizcod_max + 1];
        static const int bitrate_table[frmsizcod_max + 1];
        enum { additional_bsi_max = 2000 };

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

        enum SyncStatus{ SYNC_NOT_FOUND = 0, SYNC_FOUND, SYNC_INCOMPLETE };
        // For instanciation e.g.
        // new AC3Decoder(esPacketDecoder::CREATE_DECODER)
        static const int DECODED_FRAME_SIZE = 1536;

        AC3Decoder(const flag_t createDecoder) :
            esPacketDecoder()
        {
            au_frame_decoder = shared_ptr<au_ac3_t>(new au_ac3_t());
        }
        AC3Decoder(uint8_t * const pData, const int dataLength, const flag_t createDecoder) :
            esPacketDecoder(pData, dataLength)
        {
            au_frame_decoder = shared_ptr<au_ac3_t>(new au_ac3_t());
        }
        AC3Decoder(shared_ptr<DataBuffer_u8> const pData, const flag_t createDecoder) :
            esPacketDecoder(pData)
        {
            au_frame_decoder = shared_ptr<au_ac3_t>(new au_ac3_t());
        }

        // Constructors to call when inheriting from this class to make sure decoder isn't created 
        AC3Decoder() :
            esPacketDecoder() { }
        AC3Decoder(uint8_t * const pData, const int dataLength) :
            esPacketDecoder(pData, dataLength) { }
        AC3Decoder(shared_ptr<DataBuffer_u8> const pData) :
            esPacketDecoder(pData) { }

        virtual ~AC3Decoder() {}

        virtual SyncStatus FindSyncWord(shared_ptr<DataBuffer_u8>) { return SYNC_NOT_FOUND; }

        bool init()
        {
            if (au_frame_decoder == nullptr)
                au_frame_decoder = std::shared_ptr<au_ac3_t>(new au_ac3_t());
        }

        shared_ptr<au_ac3_t> GetDecoder()
        {
            return static_pointer_cast<au_ac3_t>(au_frame_decoder);
        }

        virtual std::shared_ptr<SampleBuffer> DecodeFrame()
        {
            return nullptr;
        }

        virtual std::shared_ptr<SampleBuffer> DecodeFrame_PassThru()
        {
            return nullptr;
        }

        virtual int CopyDataToOutputFrame(shared_ptr<DataBuffer_u8> src, shared_ptr<SampleBuffer> dst, int numBytes);
        virtual int MoveDataToOutputFrame(shared_ptr<DataBuffer_u8> src, shared_ptr<SampleBuffer> dst, int numBytes);

    private:

    };

    class AC3PacketHandler : public TSPacketHandler
    {
    public:
        AC3PacketHandler(ofstream *str, bool Debug_on = false);
        ~AC3PacketHandler(void) {}

        virtual void PESDecode(PESPacket_t *buf);

        void SetDebugOutput(bool On);

        int FrameCount;

        bool init()
        {
            if (esDecoder == nullptr)
                esDecoder = std::shared_ptr<AC3Decoder>(new AC3Decoder(esPacketDecoder::CREATE_DECODER));

            esDecoder->GetDecoder()->write_csv_header(*outStream);

            return (esDecoder != nullptr);
        }

        shared_ptr<AC3Decoder> GetDecoder()
        {
            return static_pointer_cast<AC3Decoder>(esDecoder);
        }

        void LogMessage(const int errorLevel, const char *message, ...)
        {
            if (LogFile != nullptr)
            {
                static const int bufLength = 2000;
                char formatted_string[bufLength];

                va_list args;
                va_start(args, message);
                vsnprintf_s(formatted_string, bufLength, message, args);
                LogFile->AddMessage(errorLevel, formatted_string);
                va_end(args);
            }
        }

        void LogMessage(const int errorLevel, string message)
        {
            if (LogFile != nullptr)
            {
                LogFile->AddMessage(errorLevel, message);
            }
        }

	protected:

        ofstream *outStream;
        std::shared_ptr<WAVFile> OutputWAV;
        uint64_t firstPTS;

        bool DebugOn;
        bool PacketSpansPES;

        shared_ptr<Log> LogFile;

	private:
	};
}