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
        au_eac3_t(const DataBuffer<uint8_t> * buffer, shared_ptr<Log> log = nullptr);

        ~au_eac3_t() {}
        
        enum { AU_AC3_OKAY, AU_AC3_INSUFFICIENT_DATA, AU_AC3_SYNC_NOT_FOUND };

        void InterpretFrame();

        virtual bool preProcessHeader(const uint8_t *data, const int length, int& frame_length) const;

        virtual int decode();

        virtual ostream& write(std::ostream &os);
        virtual ostream& write_csv(std::ostream &os);
        virtual ostream& write_csv_header(std::ostream &os);


    private:


    };
 
    class EAC3Decoder : public AC3Decoder
    {
    public:
        EAC3Decoder() {}
        virtual ~EAC3Decoder() {}

        shared_ptr<au_eac3_t> GetDecoder()
        {
            return std::static_pointer_cast<au_eac3_t>(au_frame_decoder);
        }

    protected:
        
    private:
        
    };
	
    class EAC3PacketHandler : public AC3PacketHandler
	{
	public:
		EAC3PacketHandler(ofstream **str, bool Debug_on = false);
        ~EAC3PacketHandler(void) {}

        virtual bool DecodeFrame(unsigned char **Frame, unsigned int *FrameSize);
		virtual void PESDecode(PESPacket_t *buf);

		void SetDebugOutput(bool On);

		int FrameCount;

        shared_ptr<EAC3Decoder> GetDecoder()
        {
            return static_pointer_cast<EAC3Decoder>(esDecoder);
        }

    protected:

	private:

	};
}