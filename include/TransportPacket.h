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

		unsigned int GetPID();
		void SetPID( unsigned int );
		
		// Continuity counter
		unsigned int GetCC();
		void SetCC( unsigned int );

		// Payload Unit Start Indicator
		unsigned int GetPUSI();
		void SetPUSI( unsigned int );

		unsigned int GetAdaptationFlags( void );

		bool AdaptationFieldPresent ( void );
		bool PayloadPresent ( void );

		bool AdjustPTS ( long long Shift );
		
		int GetPacketSize ( void ) { return packetSize; }
		int GetPayload( const uint8_t **payload_ptr);
		void ReplacePayload ( uint8_t *newPayload );

		void SetPacket ( uint8_t *data, int packetSze );
		void SetPacketNumber ( const unsigned long pNum );

		void MakeNULLPID ( void );

		bool ConvertTo188 ( void );
		bool ConvertTo204 ( void );

		TransportPacket(uint8_t *pkt, int packetSze);
		TransportPacket () : packetSize(0), Payload(NULL), payloadSize(0), packetNumber(0) {};
		~TransportPacket() {};

		int packetSize;
		uint8_t* Payload;
		int payloadSize;
        uint8_t raw[204];
		unsigned long packetNumber;
	private:
		void findPayload( void );
	};	

    int fwritePacket(TransportPacket *thispacket, FILE **outputfile);
    int freadPacket(TransportPacket *thispacket, FILE **inputfile);

    void fgetTSsync(FILE *infile, int *packet_size);
}
