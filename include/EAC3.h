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
#include "AC3.h"

namespace libr1k
{
    class liba52_wrapper;

    class au_eac3_t : public au_ac3_t
    {
    public:

        au_eac3_t(shared_ptr<Log> log = nullptr);
        au_eac3_t(const uint8_t * const buf, const int bufSize, shared_ptr<Log> log = nullptr);
        au_eac3_t(DataBuffer<uint8_t> * const buffer, shared_ptr<Log> log = nullptr);

        ~au_eac3_t() {}
        
        enum { AU_AC3_OKAY, AU_AC3_INSUFFICIENT_DATA, AU_AC3_SYNC_NOT_FOUND };

        void InterpretFrame();

        virtual bool preProcessHeader();

        virtual int decode();

        virtual int getNumAudioBlocks() const
        {
            switch (nblkscod)
            {
            case 0:
                return 1;
            case 1:
                return 2;
            case 2:
                return 3;
            case 3:
            default:
                return 6;
            }
        }

        virtual ostream& write(std::ostream &os);
        virtual ostream& write_csv(std::ostream &os);
        virtual ostream& write_csv_header(std::ostream &os);

        enum { BSID = 16 };
        enum { MAX_FRAME_LEN = 2048};

    protected:
        unsigned int nblkscod;
        unsigned int frmsiz;
        unsigned int strmtyp;
        unsigned int substreamid;

    private:


    };
 
    class EAC3Decoder : public AC3Decoder
    {
    public:

        // For instanciation e.g.
        // new EAC3Decoder(esPacketDecoder::CREATE_DECODER)

        EAC3Decoder(const flag_t createDecoder) :
            AC3Decoder(), incompleteOutputFrame(nullptr), incompleteBlks(0)
        {
            au_frame_decoder = shared_ptr<au_eac3_t>(new au_eac3_t());
        }

        EAC3Decoder(uint8_t * const pData, const int dataLength, const flag_t createDecoder) :
            AC3Decoder(pData, dataLength), incompleteOutputFrame(nullptr), incompleteBlks(0)
        {
            au_frame_decoder = shared_ptr<au_eac3_t>(new au_eac3_t());
        }

        EAC3Decoder(shared_ptr<DataBuffer_u8> const pData, const flag_t createDecoder) :
            AC3Decoder(pData), incompleteOutputFrame(nullptr), incompleteBlks(0)
        {
            au_frame_decoder = shared_ptr<au_eac3_t>(new au_eac3_t());
        }

        // Constructors to call when inheriting from this class to make sure decoder isn't created
        EAC3Decoder() :
            AC3Decoder() { }

        EAC3Decoder(uint8_t * const pData, const int dataLength) :
            AC3Decoder(pData, dataLength) { }

        EAC3Decoder(shared_ptr<DataBuffer_u8> const pData) :
            AC3Decoder(pData) { }

        virtual ~EAC3Decoder() {}

        virtual AC3Decoder::SyncStatus FindSyncWord(shared_ptr<DataBuffer_u8>);

        bool init()
        {
            if (au_frame_decoder == nullptr)
                au_frame_decoder = std::shared_ptr<au_eac3_t>(new au_eac3_t());
        }

        shared_ptr<au_eac3_t> GetDecoder()
        {
            return std::static_pointer_cast<au_eac3_t>(au_frame_decoder);
        }

        virtual std::shared_ptr<SampleBuffer> DecodeFrame();

        virtual std::shared_ptr<SampleBuffer> DecodeFrame_PassThru();

    protected:

        std::shared_ptr<SampleBuffer> incompleteOutputFrame;
        int incompleteBlks;
        
    private:
        
    };
	
    class EAC3PacketHandler : public AC3PacketHandler
	{
	public:
		EAC3PacketHandler(ofstream *str, bool Debug_on = false);
        ~EAC3PacketHandler(void) {}

		virtual void PESDecode(PESPacket_t *buf);

		void SetDebugOutput(bool On);

		int FrameCount;

        bool init() 
        {
            if (esDecoder == nullptr)
                esDecoder = std::shared_ptr<EAC3Decoder>(new EAC3Decoder(esPacketDecoder::CREATE_DECODER));

            return (esDecoder != nullptr);
        }

        shared_ptr<EAC3Decoder> GetDecoder()
        {
            return static_pointer_cast<EAC3Decoder>(esDecoder);
        }

    protected:

	private:

	};
}