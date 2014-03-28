#include "PESPacketHandler.h"
#include "BitStreamReader.h"

#include <string.h>
using namespace std;
namespace libr1k
{
	PESPacketHandler::PESPacketHandler(ofstream **str, bool Debug_on)
		:
		outStream(*str),
		filePerPes(false),
		index(0),
		DebugOn(Debug_on)
	{
		stream_id = -1;
		if (this->DebugOn)
		{
			this->LogFile = new Log();
		}
		else
		{
			this->LogFile = NULL;
		}
	}

	PESPacketHandler::PESPacketHandler(const char *str, bool Debug_on)
		:
		outStream(NULL),
		filePerPes(true),
		index(0),
		DebugOn(Debug_on)
	{		
		stream_id = -1;
		memset( this->base_filename, '\0', MAX_FILENAME_LENGTH);
#ifdef WIN32
		strncpy_s(this->base_filename, str, MAX_FILENAME_LENGTH);
#else
		strncpy(this->base_filename, str, MAX_FILENAME_LENGTH);
#endif
		if (this->DebugOn)
		{
			this->LogFile = new Log();
		}
		else
		{
			this->LogFile = NULL;
		}
	}


	PESPacketHandler::~PESPacketHandler(void)
	{
		if (this->OutputFile)
			delete this->OutputFile;
		if (this->LogFile)
			delete this->LogFile;
	}

	void PESPacketHandler::SetDebugOutput(bool On)
	{
		if (this->DebugOn != On)
		{
			if (On)
			{
				// Was off now need to turn it on
				if (this->LogFile != NULL)
				{
					// this should have been null because we are not logging, so we probably should delete this
					delete this->LogFile;
				}
				this->LogFile = new Log();
				this->DebugOn = On;
			}
			else
			{
				// Was on now need to turn it off
				if (this->LogFile != NULL)
				{
					// this should have been null because we are not logging, so we probably should delete this
					delete this->LogFile;
				}
				this->DebugOn = On;
			}
		}
	}

	void PESPacketHandler::PESDecode ( PESPacket_t *const buf )
	{
		// Read PES header
		// Here we have a full PES packet just dump it out.

		const unsigned char * const PES_data = &(buf->payload[9]) + buf->payload[8]; // The start of the data is the number of bytes in the PES header length field
																		// added to the first byte after the PES header length field
		// Need to adjust PESPacketSize to make it just the payload size
		const unsigned int PESPacketSize = buf->nextFreeByte - PES_data;

		this->LogFile->AddMessage( Log::MAX_LOG_LEVEL, "PESDump - Frame");
		this->LogFile->AddMessage( Log::MAX_LOG_LEVEL, "\tPTS - %d", buf->PTS );

		// if this->outStream is NULL then we are doing 1 pes packet per file
		if (this->filePerPes)
		{
			char combinedFilename[2*MAX_FILENAME_LENGTH];
			this->index++;

#ifdef WIN32
			sprintf_s(combinedFilename, "%s_num_%08u_pts_%llu.pes", this->base_filename, this->index, buf->PTS);
#else
			sprintf(combinedFilename, "%s_num_%08u_pts_%llu.pes", this->base_filename, this->index, buf->PTS);
#endif

			this->outStream = new ofstream(combinedFilename, ios::binary | ios::out);
		}

		// Decode each ESDump frame within this PES packet
		if (this->outStream)
		{
			unsigned int size = buf->nextFreeByte - buf->payload;
			this->outStream->write((char*)buf->payload, size);
		}

		if (this->filePerPes && this->outStream)
		{
			this->outStream->close();
			delete this->outStream;
		}
	}
}