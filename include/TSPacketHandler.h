#pragma once

#include "libr1k.h"
#include "TransportPacket.h"
#include "Types.h"
#include <deque>
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
			nextFreeByte(NULL),
			Started(false),
			Complete(false),
			bytesStored(0)
			{};
		~PESPacket_t(void)
		{
			if (this->payload) delete this->payload;
		}
		unsigned int type;
		unsigned long long PTS;
		int	pesPacketLength;
		unsigned char*	payload;
		unsigned char*	nextFreeByte;
		bool Started;
		bool Complete;
		int bytesStored;

        void SetDataLength(const int PESPacketSize)
        {
            if (payload != nullptr)
            {
                delete[] payload;
            }
            payload = new uint8_t[PESPacketSize];

            nextFreeByte = payload;

            pesPacketLength = PESPacketSize;
            Started = true;
        }

		int GetCurrentDataLength()
		{
			if (nextFreeByte && payload)
			{
				return nextFreeByte - payload;
			}
			return 0;
		}

        uint8_t *GetPESData()
        {
            if (bytesStored > 10)
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
                return nextFreeByte - dataStart;
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
				memcpy(nextFreeByte, d, s);
				nextFreeByte += s;
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
		TSPacketHandler( void ):
			CurrentState( WaitForPesStart ), 
			NextState( WaitForPesStart ),
			BufferLevel(0),
            ContinuityCount( 0xff ),
			doubleCC(false) {};

		~TSPacketHandler( void );

		virtual void NextPacket( TransportPacket *tsPacket );
		virtual void PCRTick ( unsigned long long PCR );

	protected:
		
		virtual bool			FindPESHeaderAndGetPTS(const uint8_t  *buf, unsigned long long *PTS);
		virtual unsigned int	GetPESPacketSize(const uint8_t * const buf) const;
		virtual bool			NewPESPacketFound(PESPacket_t *pPacket, TransportPacket *tsPacket);
		virtual bool			ContinueLastPESPacket(PESPacket_t *pPacket, TransportPacket *tsPacket);

		virtual void			PESDecode ( PESPacket_t *buf ) = 0;
		virtual bool            CheckCCError ( unsigned int cc );

		enum PacketProcessStateMachine {WaitForPesStart, WaitForPESData, PESDataError, CCError} CurrentState, NextState;

		unsigned int BufferLevel;
		uint8_t stream_id;
		uint8_t ContinuityCount;
		bool doubleCC;

		deque<PESPacket_t*> PESdata;
	};
}
