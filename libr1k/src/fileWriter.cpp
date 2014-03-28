#include "fileWriter.h"

#include <malloc.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace libr1k
{
	fileWriter::fileWriter( const char *filename, int pkt_len )
	{
		this->pkt_len = pkt_len;

#ifdef WIN32
		if ( fopen_s ( &outfile, filename, "wb" ) != 0 )
		{
			fprintf (stderr, "Error: Unable to open the output file" );
			return;
		}
#else
		if ( !(outfile = fopen ( filename, "wb" ) ))
        {
            fprintf (stderr, "Error: Unable to open the output file" );
            return;
        }
#endif
		
		if (this->pkt_len)
		{
			pkt_buffer = new unsigned char[this->pkt_len * MAX_NUM_PACKETS];
		}

		if (!pkt_buffer)
		{
			this->pkt_len = 0;  // Set this to 0 to imply contruction hasn't worked for this class
		}
		next_available_packet = pkt_buffer;
	}


	fileWriter::~fileWriter(void)
	{
		Block_write();

		fclose (outfile);
		if (pkt_buffer)
		{
			delete[] pkt_buffer;
		}
	}

	int fileWriter::writeTsPacket ( unsigned char* source_packet, int length )
	{
		if (
			(next_available_packet == (pkt_buffer + (this->pkt_len * MAX_NUM_PACKETS)) ) // full
			||
			(length != this->pkt_len) // packet length change !?!
			)
		{
			// Write out the current buffer then 
			Block_write();
		}

		if (length != this->pkt_len) // packet length change !?!
		{
			this->setPacketLength(length);
		}

		memcpy(next_available_packet, source_packet, this->pkt_len);
		next_available_packet += this->pkt_len;
		
		return 0;
	}

	int fileWriter::writeTsPacket ( TransportPacket *TsPacket )
	{
		return this->writeTsPacket( (unsigned char *)&(TsPacket->raw), TsPacket->GetPacketSize());
	}

	void fileWriter::setPacketLength ( int length ) 
	{ 
		if (length != this->pkt_len)
		{
			this->pkt_len = length;

			if (pkt_buffer)
			{
				delete[] pkt_buffer;
			}

			pkt_buffer = new unsigned char[this->pkt_len * MAX_NUM_PACKETS];
			next_available_packet = pkt_buffer;
		}
	}

	void fileWriter::Block_write ( void )
	{
		fwrite( pkt_buffer, 1, next_available_packet - pkt_buffer, this->outfile);
		 next_available_packet = pkt_buffer;
	}

	void fileWriter::close ( void )
	{
		Block_write();
		fclose( this->outfile );
	}

	void fileWriter::flush( void )
	{
		Block_write();
	}
}
