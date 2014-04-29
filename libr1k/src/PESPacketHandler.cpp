#include "PESPacketHandler.h"
#include "BitStreamReader.h"

#include <string.h>
using namespace std;
namespace libr1k
{
	PESPacketHandler::PESPacketHandler(ofstream *str, bool Debug_on)
		:
		outStream(str),
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
		memset( base_filename, '\0', MAX_FILENAME_LENGTH);
#ifdef WIN32
		strncpy_s( base_filename, str, MAX_FILENAME_LENGTH);
#else
		strncpy(this->base_filename, str, MAX_FILENAME_LENGTH);
#endif
		if (DebugOn)
		{
			LogFile = new Log();
		}
		else
		{
			LogFile = NULL;
		}
	}


	PESPacketHandler::~PESPacketHandler(void)
	{
		if (OutputFile)
			delete OutputFile;
		if (this->LogFile)
			delete LogFile;
	}

	void PESPacketHandler::SetDebugOutput(bool On)
	{
		if (DebugOn != On)
		{
			if (On)
			{
				// Was off now need to turn it on
				if (LogFile != NULL)
				{
					// this should have been null because we are not logging, so we probably should delete this
					delete LogFile;
				}
				LogFile = new Log();
				DebugOn = On;
			}
			else
			{
				// Was on now need to turn it off
				if (LogFile != NULL)
				{
					// this should have been null because we are not logging, so we probably should delete this
					delete LogFile;
				}
				DebugOn = On;
			}
		}
	}

	void PESPacketHandler::PESDecode ( PESPacket_t *const buf )
	{
		// Read PES header
		// Here we have a full PES packet just dump it out.

		const unsigned char * const PES_data = buf->GetPESData(); // The start of the data is the number of bytes in the PES header length field
																		// added to the first byte after the PES header length field
		// Need to adjust PESPacketSize to make it just the payload size
		const unsigned int PESPacketSize = buf->GetPESDataLength();

		LogFile->AddMessage( Log::MAX_LOG_LEVEL, "PESDump - Frame");
		LogFile->AddMessage( Log::MAX_LOG_LEVEL, "\tPTS - %d", buf->PTS );

		// if this->outStream is NULL then we are doing 1 pes packet per file
		if (filePerPes)
		{
			char combinedFilename[2*MAX_FILENAME_LENGTH];
			index++;

#ifdef WIN32
			sprintf_s(combinedFilename, "%s_num_%08u_pts_%llu.pes", base_filename, index, buf->PTS);
#else
			sprintf(combinedFilename, "%s_num_%08u_pts_%llu.pes", base_filename, index, buf->PTS);
#endif

			outStream = new ofstream(combinedFilename, ios::binary | ios::out);
		}

		// Decode each ESDump frame within this PES packet
		if (this->outStream)
		{
			unsigned int size = buf->payload.size();
			this->outStream->write((char*)&buf->payload[0], size);
		}

		if (this->filePerPes && this->outStream)
		{
			this->outStream->close();
			delete this->outStream;
		}
	}
}