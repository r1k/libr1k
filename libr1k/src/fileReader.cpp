#include "fileReader.h"

#include <malloc.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace libr1k
{
	fileReader::fileReader( const char *filename)
	{
		this->pkt_len = 0;
		this->sync_found = false;
		this->pkts_available = 0;

#ifdef WIN32
		if ( fopen_s ( &infile, filename, "rb" ) != 0 )
		{
			fprintf (stderr, "Error: Unable to open the input file" );
			return;
		}
#else
        if ( !(infile = fopen ( filename, "rb" )))
        {
            fprintf (stderr, "Error: Unable to open the input file" );
            return;
        }
#endif
		this->SyncToTSHeader();

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

	fileReader::~fileReader (void) 
	{ 
		fclose ( infile ); 
		if (pkt_buffer)
		{
			delete[] pkt_buffer;
		}
	}

	int fileReader::readTsPacket ( unsigned char *dest_packet )
	{
		if (pkts_available == 0)
		{
			Block_read(); // Block_read modifies pkts_available
		}

		if (pkts_available != 0)
		{
			memcpy ( dest_packet, next_available_packet, this->pkt_len );

			pkts_available--;
			next_available_packet += this->pkt_len;

			this->readPackets++;

			return 1;
		}
		else
		{
			return 0;
		}
	}

	int fileReader::readTsPacket ( TransportPacket *TsPacket )
	{
		if (pkts_available == 0)
		{
			Block_read(); // Block_read modifies pkts_available
		}

		if (pkts_available != 0)
		{
			TsPacket->SetPacket ( next_available_packet, this->pkt_len );

			pkts_available--;
			next_available_packet += this->pkt_len;

			this->readPackets++;

			return 1;
		}
		else
		{
			return 0;
		}
	}

	void fileReader::SyncToTSHeader( void )
	{
		char	test_buffer[2000];
		int		index = 0;

		this->pkt_len = 0;

		// find first packet start
		fread ( test_buffer, sizeof (char), 2000, infile );

		while ( sync_found == false )
		{
			if ( (test_buffer[index] == 0x47) && (test_buffer[index + 188] == 0x47) && (test_buffer[index + 376] == 0x47) )
			{
				sync_found = true;
				this->pkt_len = 188;
			}
			else if ( (test_buffer[index] == 0x47) && (test_buffer[index + 204] == 0x47) && (test_buffer[index + 408] == 0x47) )
			{
				sync_found = true;
				this->pkt_len = 204;
			}
			else
			{
				index++;
			}
		}

		// rewind then jump to first sync
		rewind ( infile );
		fseek ( infile, index, SEEK_SET );
	}

	void fileReader::Block_read ( void )
	{
		pkts_available = fread (pkt_buffer, this->pkt_len, MAX_NUM_PACKETS, infile);
		next_available_packet = pkt_buffer;
	}

	long long fileReader::getTotalNumberOfPackets ( void )
	{
#ifdef WIN32
		// Get the file size 
		int64_t current_position = ftell( this->infile );
		_fseeki64 (this->infile, 0, SEEK_END);
		int64_t end_position = _ftelli64 (this->infile);

		int64_t filesize = (long long )end_position / this->pkt_len;

		_fseeki64 (this->infile, current_position, SEEK_SET);
#else
        // Get the file size
        int64_t current_position = ftell( this->infile );
        fseeko (this->infile, 0, SEEK_END);
        int64_t end_position = ftello64 (this->infile);

        int64_t filesize = (long long )end_position / this->pkt_len;

        fseeko (this->infile, current_position, SEEK_SET);
#endif
		return (filesize);
	}

	int fileReader::getPercentageUsed ( void ) 
	{ 
		return  (int) ((((double) this->readPackets) / ((double)this->getTotalNumberOfPackets()) * 100)); 
	}
}