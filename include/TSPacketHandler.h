#pragma once

#include "libr1k.h"
#include "TransportPacket.h"
#include "Types.h"
#include "esPacketDecoder.h"
#include <deque>
#include <vector>
using namespace std;

namespace libr1k
{
    class PESPacket_t
	{
	public:
		PESPacket_t():
			type(0),
			PTS(0),
			pesPacketLength(0),
			payload(NULL),
			Started(false),
			Complete(false)
			{};
		~PESPacket_t(void)
		{
		}
		unsigned int type;
		unsigned long long PTS;
		int	pesPacketLength;
		vector<unsigned char> payload;
		bool Started;
		bool Complete;

        void SetDataLength(const int PESPacketSize)
        {
            if (payload.size() != 0)
            {
                payload.clear();
            }
            payload.reserve (PESPacketSize);

            pesPacketLength = PESPacketSize;
            Started = true;
        }

		int GetCurrentDataLength()
		{
            return payload.size();
		}

        uint8_t *GetPESData()
        {
            if (payload.size() > 10)
            {
                // The start of the data is the number of bytes in the PES header length field
                return &(payload[9]) + payload[8]; 
            }
            else
            {
                return nullptr;
            }
        }

        int GetPESDataLength()
        {
            uint8_t* dataStart = GetPESData();
            if (dataStart != nullptr)
                return payload.size() - (dataStart - &payload[0]);
            else
                return 0;
        }

		void AddNewData(const uint8_t *d, int s)
		{
			if (s > 0
				&& 
				d 
				&& 
				s <= pesPacketLength - GetCurrentDataLength()
			   )
			{
                payload.insert(payload.end(), d, d + s);
			}

            if (GetCurrentDataLength() < pesPacketLength)
            {
                Complete = false;
            }
            else if (GetCurrentDataLength() == pesPacketLength)
            {
                Complete = true;
            }
		}
	};

    class TSPacketHandler
    {
    public:
        TSPacketHandler(void) :
            CurrentState(WaitForPesStart),
            NextState(WaitForPesStart),
            BufferLevel(0),
            ContinuityCount(0xff),
            doubleCC(false),
            esDecoder(nullptr) {};

        ~TSPacketHandler(void);

        virtual void NextPacket(TransportPacket *tsPacket);
        virtual void PCRTick(unsigned long long PCR);

        virtual bool init()
        {
            return (esDecoder != nullptr);
        }

        shared_ptr<esPacketDecoder> GetDecoder()
        {
            return static_pointer_cast<esPacketDecoder>(esDecoder);
        }

    protected:

        virtual bool			FindPESHeaderAndGetPTS(const uint8_t  *buf, unsigned long long *PTS);
        virtual unsigned int	GetPESPacketSize(const uint8_t * const buf) const;
        virtual bool			NewPESPacketFound(PESPacket_t *pPacket, TransportPacket *tsPacket);
        virtual bool			ContinueLastPESPacket(PESPacket_t *pPacket, TransportPacket *tsPacket);

        virtual void			PESDecode(PESPacket_t *buf);
        virtual bool            CheckCCError(unsigned int cc);

		enum PacketProcessStateMachine {WaitForPesStart, WaitForPESData, PESDataError, CCError} CurrentState, NextState;

		unsigned int BufferLevel;
		uint8_t stream_id;
		uint8_t ContinuityCount;
		bool doubleCC;

		deque<PESPacket_t*> PESdata;
 
        shared_ptr<esPacketDecoder> esDecoder;
	};
}
