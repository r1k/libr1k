#pragma once

#include <stdio.h>
#include "libr1k.h"
#include "TransportPacket.h"

namespace libr1k
{
    class fileWriter
	{
	private:
		static const int MAX_NUM_PACKETS = 20000;
		FILE *outfile;
		int pkt_len;
		
		unsigned char *pkt_buffer;
		unsigned char *next_available_packet;
		
		void Block_write ( void );
	public:
		fileWriter( const char *filename, int pkt_len = 188  );
		~fileWriter(void);

		int writeTsPacket ( unsigned char* dest_packet, int length = 188 );

		int writeTsPacket ( TransportPacket *TsPacket );

		void setPacketLength ( int length );

		void close ( void );

		void flush( void );
	};

}
