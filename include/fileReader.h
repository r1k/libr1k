#pragma once

#include <stdio.h>
#include "libr1k.h"
#include "TransportPacket.h"

namespace libr1k
{
    class fileReader
	{
	private:
		static const int MAX_NUM_PACKETS = 20000;
		FILE *infile;
		int pkt_len;
		bool sync_found;
		
		unsigned char *pkt_buffer;
		int pkts_available;
		unsigned char *next_available_packet;
		
		void Block_read ( void );

		void SyncToTSHeader( void );

		long long readPackets;

	public:
		fileReader( const char *filename);

		~fileReader (void);

		int readTsPacket ( unsigned char* dest_packet );

		int readTsPacket ( TransportPacket *TsPacket );

		int getPacketLength ( void ) { return this->pkt_len; }

		long long getTotalNumberOfPackets ( void );
		int getPercentageUsed ( void );

		bool SyncFound ( void ) { return this->sync_found;}
	};
}
