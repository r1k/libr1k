#include "ESDumpPacketHandler.h"
#include "BitStreamReader.h"

#ifndef WIN32
#include <string.h>
#endif
namespace libr1k
{
	ESDumpPacketHandler::ESDumpPacketHandler(ofstream *str, bool Debug_on)
	{
		outStream = str;
		stream_id = -1;
        this->filePerPes = false;

		DebugOn = Debug_on;
		if (DebugOn)
		{
			LogFile = new Log();
		}
		else
		{
			LogFile = NULL;
		}

	}
	ESDumpPacketHandler::ESDumpPacketHandler(const char *str, bool Debug_on)
	{
		this->outStream = NULL;
		this->stream_id = -1;
		this->filePerPes = false;
		
		memset( this->base_filename, '\0', MAX_FILENAME_LENGTH);
#ifdef WIN32
		strncpy_s(this->base_filename, str, MAX_FILENAME_LENGTH);
#else
		strncpy(this->base_filename, str, MAX_FILENAME_LENGTH);
#endif

		this->DebugOn = Debug_on;
		if (this->DebugOn)
		{
			this->LogFile = new Log();
		}
		else
		{
			this->LogFile = NULL;
		}

	}


	ESDumpPacketHandler::~ESDumpPacketHandler(void)
	{
		if (this->OutputFile)
			delete this->OutputFile;
		if (this->LogFile)
			delete this->LogFile;
	}

	void ESDumpPacketHandler::SetDebugOutput(bool On)
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

	void ESDumpPacketHandler::PESDecode ( PESPacket_t *const buf )
	{
		// Read PES header
		// Here we have a full PES packet just dump it out.

		const unsigned char * const PES_data = buf->GetPESData(); // The start of the data is the number of bytes in the PES header length field
																		// added to the first byte after the PES header length field
		// Need to adjust PESPacketSize to make it just the payload size
		const unsigned int PESPacketSize = buf->GetPESDataLength();

		this->LogFile->AddMessage( Log::MAX_LOG_LEVEL, "ESDump - Frame");
		this->LogFile->AddMessage( Log::MAX_LOG_LEVEL, "\tPTS - %d", buf->PTS );
		
		// if this->outStream is NULL then we are doing 1 pes packet per file
		if (this->filePerPes)
		{
			char combinedFilename[2*MAX_FILENAME_LENGTH];
			this->index++;
#ifdef WIN32
			sprintf_s(combinedFilename, "%s_num_%08u_pts_%llu.es", this->base_filename, this->index, buf->PTS);
#else
			sprintf(combinedFilename, "%s_num_%08u_pts_%llu.es", this->base_filename, this->index, buf->PTS);
#endif

			this->outStream = new ofstream(combinedFilename, ios::binary | ios::out);
		}
		
		// Decode each ESDump frame within this PES packet
		for (unsigned int i = 0; i < PESPacketSize; i++)
		{
			this->outStream->write((char*)&PES_data[i], sizeof(*PES_data));
		}
		
		if (this->filePerPes && this->outStream)
		{
			this->outStream->close();
			delete this->outStream;
		}
	}
}