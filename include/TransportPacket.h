#pragma once

#include "Types.h"
#include "libr1k.h"
#include <stdio.h>

namespace libr1k
{
    class TransportPacket
	{
	public:
	
		enum { PayloadSize = 184, Ts188Byte = 188, Ts204Byte = 204};
		enum { PATPID = 0, NULLPID = 8191 };

		uint16_t GetPID() const;
		void SetPID( const uint16_t );
		
		// Continuity counter
		uint8_t GetCC() const;
		void SetCC( const uint8_t );

		// Payload Unit Start Indicator
		uint8_t GetPUSI() const;
        void SetPUSI(const bool);

		uint8_t GetAdaptationFlags( void ) const;

		bool AdaptationFieldPresent ( void ) const;
		bool PayloadPresent ( void ) const;
        		
		int  GetPacketSize ( void ) const { return packetSize; }
		int  GetPayload( const uint8_t **payload_ptr);
		void SetPayload ( const uint8_t * const newPayload );

		void SetPacket ( uint8_t *data, int packetSze );
		void SetPacketNumber ( const unsigned long pNum );

		void MakeNULLPID ( void );

		bool ConvertTo188 ( void );
		bool ConvertTo204 ( void );

		TransportPacket(uint8_t *pkt, int packetSze);
		TransportPacket () : packetSize(0), Payload(NULL), payloadSize(0), packetNumber(0) {};
		~TransportPacket() {};

		int      packetSize;
		uint8_t* Payload;
		int      payloadSize;
        uint8_t  raw[204];
		uint64_t packetNumber;

	private:
		void findPayload( void );
	};	

    int fwritePacket(TransportPacket *thispacket, FILE **outputfile);
    int freadPacket(TransportPacket *const thispacket, FILE *const inputfile);

    int fgetTSsync(FILE *const);
}
