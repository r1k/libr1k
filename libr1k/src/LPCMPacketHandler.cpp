#include "LPCMPacketHandler.h"
#include "BitStreamReader.h"
namespace libr1k
{
	AES3_data_header::AES3_data_header( unsigned char *AESFrame ):
		audioPacketSize((AESFrame[0] << 8) | AESFrame[1] ),
		numChannels((AESFrame[2] & 0xc0) >> 6),
        bitDepth((AESFrame[3] & 0x30) >> 4),
		channelID(((AESFrame[2] & 0x3f) << 2) | ((AESFrame[3] & 0xc0) >> 6)),
		alignmentBits(AESFrame[3] & 0x04)
	{
		// Now translate to useful numbers
		switch(numChannels)
		{
			case 0x00:
				numChannels = 2;
				break;
			case 0x01:
				numChannels = 4;
				break;
			case 0x02:
				numChannels = 6;
				break;
			case 0x03:
				numChannels = 8;
				break;
			default:
				numChannels = 2;
				break;
		}
		switch(bitDepth)
		{
			case 0x00:
				bitDepth = 16;
				break;
			case 0x01:
				bitDepth = 20;
				break;
			case 0x02:
				bitDepth = 24;
				break;
			case 0x03:
			default:
				bitDepth = 20;
				break;
		}
	}

	LPCMPacketHandler::LPCMPacketHandler(ofstream *str, bool Debug_on)
	{
		outStream = str;
		stream_id = (char) 0xbd;

		WAVParams.bit_depth = 16;
		WAVParams.num_channels = 2;
		WAVParams.SamplesPerSec = 48000;

		OutputFile = new WAVFile(str, WAVParams);
		FrameCount = 0;
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


	LPCMPacketHandler::~LPCMPacketHandler(void)
	{
		delete OutputFile;
	}

	void LPCMPacketHandler::SetDebugOutput(bool On)
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

	void LPCMPacketHandler::PESDecode ( PESPacket_t *buf )
	{
		// Read PES header
		
		unsigned char *PES_data = buf->GetPESData(); // The start of the data is the number of bytes in the PES header length field
																		// added to the first byte after the PES header length field
		// Need to adjust PESPacketSize to make it just the payload size
        unsigned int PESPacketSize = buf->GetPESDataLength();

		LogFile->AddMessage( Log::MAX_LOG_LEVEL, "LPCM - Frame %d", this->FrameCount );
		LogFile->AddMessage( Log::MAX_LOG_LEVEL, "\tPTS - %d", buf->PTS );
		// Decode each LPCM frame within this PES packet
		while (PESPacketSize && DecodeLPCMFrame( &PES_data, &PESPacketSize )) 
			;
		if (PESPacketSize != 0)
		{
			// Data remains but not recognisable as LPCM
			//cerr << "Duff data left in PES" << endl;
		}
	}

	bool LPCMPacketHandler::DecodeLPCMFrame( unsigned char **LPCMFrame, unsigned int *BufferSize )
	{
		AES3_data_header AESHeader(*LPCMFrame);
		*LPCMFrame += AESHeader.byteSize;

		//int payloadSize = *BufferSize - 4; // less AES packet header size
		(*BufferSize) -= 4;

		int NumStereoPairsStuffed = AESHeader.audioPacketSize / (AESHeader.numChannels * ((AESHeader.bitDepth + 4) + 7 / 8));  // +7 to make sure we round up

		FrameCount++;

		if (DebugOn)
		{
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "\t\tAES Header - audio_packet_size %d", AESHeader.audioPacketSize );
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "\t\tAES Header - number channels %d", AESHeader.numChannels );
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "\t\tAES Header - channel_identification %d", AESHeader.channelID );
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "\t\tAES Header - bits per sample %d", AESHeader.bitDepth);
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "\t\tAES Header - alignement bits %d", AESHeader.alignmentBits );
		}

		int numSamples = 2 * NumStereoPairsStuffed;
		
		// Write AES Header to WAV file

		int samplesWritten = 0;
		int samplesToWrite = 2 * NumStereoPairsStuffed;

		samplesToWrite += samplesWritten; // The size of the AES header - the amount we have already written
		unsigned char *source = *LPCMFrame;
		
		for (;(samplesWritten < samplesToWrite) && (samplesWritten < numSamples); samplesWritten++)
		{
			switch(AESHeader.bitDepth)
			{
				case 16:
					LogFile->AddMessage( Log::MAX_LOG_LEVEL, "16 bit Not yet implemented");
					break;
				case 20:
					{
						unsigned int sample = 0;
						// 20 bit audio + 4 bit flags for every sample of every sample
						sample  = source[0] << 16;
						sample |= (source[1] & 0x0ff) << 8;
						sample |= source[2] & 0x0ff;
				
						sample = BitReverse24(sample);
						sample = sample & 0x0fffff; // strip flags
						sample = sample & 0x000ffff0; // convert to 16 bit
						sample = sample << 12; // Left justify the sample
						OutputFile->WriteSample( sample );
						source += 3;
						(*BufferSize) -= 3;
					}
					break;
				case 24:
					LogFile->AddMessage( Log::MAX_LOG_LEVEL, "24 bit Not yet implemented");
					break;
				default:
					LogFile->AddMessage( Log::MAX_LOG_LEVEL, "Unrecognised bit depth");
					break;
			}
		}

		*LPCMFrame = source;
		return true;
	}

}