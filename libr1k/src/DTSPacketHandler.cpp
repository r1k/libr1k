#include "DTSPacketHandler.h"
#include "BitStreamReader.h"

namespace libr1k
{
	

	
	DTSMetadata::DTSMetadata( unsigned char *DTSFrame ):
			SYNCWORD( (DTSFrame[0] << 24) | (DTSFrame[1] << 16) | (DTSFrame[2] << 8) | DTSFrame[3]),
			FTYPE( (DTSFrame[4] & 0x80) >> 7 ),
			NBLKS( ((DTSFrame[4] & 0x01) << 6) | ((DTSFrame[5] & 0xfc) >> 2) ),
			FSIZE( ((DTSFrame[5] & 0x03) << 12) | (DTSFrame[6] << 4) | ((DTSFrame[7] & 0xf0) >> 4) ),
			AMODE( ((DTSFrame[7] & 0x0f) << 2) | ((DTSFrame[8] & 0xc0) >> 6) ),
			SFREQ( (DTSFrame[8] & 0x3c) >> 2 ),
			RATE( ((DTSFrame[8] & 0x03) << 3) | ((DTSFrame[9] & 0xe0) >> 5) ),
			MIX( (DTSFrame[9] & 0x10) >> 4 ),
			DYNF( (DTSFrame[9] & 0x08) >> 3 ),
			TIMEF( (DTSFrame[9] & 0x04) >> 2 ),
			AUXF( (DTSFrame[9] & 0x02) >> 1 ),
			HDCD( (DTSFrame[9] & 0x01) ),
			EXT_AUDIO_ID( (DTSFrame [10] & 0xe0) >> 5 ),
			EXT_AUDIO( (DTSFrame[10] & 0x10) >> 4),
			ASPF((DTSFrame[10] & 0x08) >> 3 ),
			LFF( (DTSFrame[10] & 0x06) >> 1 ),
			HFLAG( (DTSFrame[10] & 0x01) ),
			HCRC( (DTSFrame[11] << 8) | DTSFrame[12] ),
			FILTS( (DTSFrame[13] & 0x80) >> 7 ),
			VERNUM( (DTSFrame[13] & 0x78) >> 3 ),
			CHIST( (DTSFrame[13] & 0x06) >> 1 ),
			PCMR( ((DTSFrame[13] & 0x01) << 2) | ((DTSFrame[14] & 0xc0) >> 6) ),
			SUMF( (DTSFrame[14] & 0x20) >> 5 ),
			SUMS( (DTSFrame[14] & 0x10) >> 4 ),
			DIALNORM( (DTSFrame[14] & 0x0f) ),
			AuxDataFound(false)
	{
		int SamplesPerFrame = ((this->NBLKS + 1) * 32);
		switch(SamplesPerFrame)
		{
		//	case 256:
		//		break;
			case 512:
				this->DTS_FRAME_TYPE = DTS_TYPE_1;
				break;
			case 1024:
				this->DTS_FRAME_TYPE = DTS_TYPE_2;
				break;
			case 2048:
				this->DTS_FRAME_TYPE = DTS_TYPE_3;
				break;
			default:
				this->DTS_FRAME_TYPE = DTS_TYPE_ERROR;
				break;
		}

		SearchForAuxData(DTSFrame);
	}

	void DTSMetadata::DeriveNumDwnMixCodeCoeffs(void)
	{
		unsigned int	NumPriCh;

		NumPriCh = AMODE_CHANNELS[AMODE];
		if (this->LFF > 0 )
			NumPriCh++;

		// recall these tables do NOT include a scale row!
		// Check m_nPrmChDownMixType
		switch ( this->PrmChDownMixType )
		{
			case DTSDOWNMIXTYPE_1_0:
				NumDwnMixCodeCoeffs = NumPriCh;
				NumChPrevHierChSet = 1;
				break;

			case DTSDOWNMIXTYPE_LoRo:
			case DTSDOWNMIXTYPE_LtRt:
				NumDwnMixCodeCoeffs = 2*NumPriCh;
				NumChPrevHierChSet = 2;
				break;

			case DTSDOWNMIXTYPE_3_0:
			case DTSDOWNMIXTYPE_2_1:
				NumDwnMixCodeCoeffs = 3*NumPriCh;
				NumChPrevHierChSet = 3;
				break;

			case DTSDOWNMIXTYPE_2_2:
			case DTSDOWNMIXTYPE_3_1:
				NumDwnMixCodeCoeffs = 4*NumPriCh;
				NumChPrevHierChSet = 4;
				break;

			default:
				NumDwnMixCodeCoeffs = 0;
				NumChPrevHierChSet = 0;
				break;
		}
	}


	void DTSMetadata::SearchForAuxData( unsigned char *Frame )
	{
		unsigned char *AuxPtr = Frame + this->FSIZE;

		AuxDataFound = false;

		while ((AuxPtr > Frame) && !AuxDataFound)
		{
			if (
				(AuxPtr[0] == 0x9a)
				&&
				(AuxPtr[1] == 0x11)
				&&
				(AuxPtr[2] == 0x05)
				&&
				(AuxPtr[3] == 0xa0)
				)
			{
				bitStreamReader *BitReader = new bitStreamReader(&AuxPtr[4], (Frame + this->FSIZE - &AuxPtr[4]));
				
				unsigned long Field = 0;

				AuxDataFound = true;
				BitReader->GetXBits(&Field, 1);
				if (Field)
				{
					this->AUXTimeStampFlag = true;

					// We have a time code we need to skip over. 36 bits
					// bitStreamReader can currently only read up to 32 bits

					unsigned long dump;
					BitReader->GetXBits(&dump, 18);
					BitReader->GetXBits(&dump, 18);
				}
				BitReader->GetXBits(&Field, 1);
				if (Field)
				{
					unsigned int i = 0;

					AUXDynamCoeffFlag = true;

					BitReader->GetXBits(&PrmChDownMixType, 3);

					DeriveNumDwnMixCodeCoeffs();

					for ( i = 0; i < this->NumDwnMixCodeCoeffs; i++ )
					{
						unsigned long dump;
						BitReader->GetXBits( &dump, 9 );
					}
				}
				BitReader->JumpToNextByteBoundary();

				BitReader->GetXBits(&Field, 16); // skip CRC
			}
			else
			{
				AuxPtr--;
			}
		}
	}

	ostream &operator<<(ostream &stream, DTSMetadata& meta)
	{
		stream << "DTS Metadata" << endl;
		stream << "Syncword : " << hex << meta.SYNCWORD << dec << endl;
		stream << "Frame Type (1,2,3) : " << meta.DTS_FRAME_TYPE << endl;
		stream << "FTYPE : " << (meta.FTYPE ? "Normal" : "Termination") << endl;
		stream << "NBLKS : " << meta.NBLKS << endl;
		stream << "FSIZE : " << meta.FSIZE << endl;
		stream << "AMODE : " << meta.AMODE << endl;
		stream << "LFF   : " << meta.LFF << endl;
		stream << "RATE  : " << meta.RATE << endl;
		stream << "DIALNORM : " << meta.DIALNORM << endl;
	
	  return stream;
	}

	DTSPacketHandler::DTSPacketHandler(ofstream *str, bool Debug_on)
	{
		outStream = str;
		stream_id = (char)0xbd;

		WAVParams.bit_depth = 16;
		WAVParams.num_channels = 2;
		WAVParams.SamplesPerSec = 48000;

		OutputFile = new WAVFile(str, &(WAVParams));
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

	DTSPacketHandler::~DTSPacketHandler(void)
	{
		delete this->OutputFile;
	}

	void DTSPacketHandler::SetDebugOutput(bool On)
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
	
	void DTSPacketHandler::PESDecode ( PESPacket_t *buf )
	{
		// Read PES header
		
		unsigned char *PES_data = buf->GetPESData(); // The start of the data is the number of bytes in the PES header length field
																		// added to the first byte after the PES header length field
		// Need to adjust PESPacketSize to make it just the payload size
		unsigned int PESPacketSize = buf->GetPESDataLength();

		// Decode each DTS frame within this PES packet
		while (PESPacketSize && this->DecodeFrame( &PES_data, &PESPacketSize )) 
			;
		if (PESPacketSize != 0)
		{
			// Data remains but not recognisable as DTS
			//cerr << "Duff data left in PES" << endl;
		}
	}

    bool DTSPacketHandler::DecodeFrame(unsigned char **Frame, unsigned int *FrameSize)
	{
		int NumStereoPairsStuffed = 0;
		// Check metadata and extract FSIZE/NBLKS so we know how much stuffing to add between consecutive DTS frames
		DTSMetadata Metadata(*Frame);
		if (Metadata.SYNCWORD != DTS_SYNCWORD)
		{
			// This is not a DTS syncword
            FrameSize = 0;
			return false;
		}
		FrameCount++;

		if (DebugOn)
		{
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "DTS Metadata - Frame %d", FrameCount );
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "Syncword : 0x%X", Metadata.SYNCWORD );
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "Frame Type (1,2,3) : %d", (Metadata.DTS_FRAME_TYPE == DTS_TYPE_1) ? 1 : (Metadata.DTS_FRAME_TYPE == DTS_TYPE_2) ? 2 : (Metadata.DTS_FRAME_TYPE == DTS_TYPE_3) ? 3 : 0);
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "FTYPE : %s",(Metadata.FTYPE ? "Normal" : "Termination"));
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "NBLKS : %d", Metadata.NBLKS);
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "FSIZE : %d", Metadata.FSIZE);
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "AMODE : 0x%X", Metadata.AMODE);
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "LFF   : %d", Metadata.LFF);
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "RATE  : 0x%X", Metadata.RATE);
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "VERNUM	: %d", Metadata.VERNUM);
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "DIALNORM  : %d", Metadata.DIALNORM);
			LogFile->AddMessage( Log::MAX_LOG_LEVEL, "Dialogue Normalisation : -%d", (Metadata.VERNUM == 7 ? Metadata.DIALNORM : (Metadata.DIALNORM + 16)) );
		}

		if (
			(Metadata.DTS_FRAME_TYPE != DTS_TYPE_1) &&
			(Metadata.DTS_FRAME_TYPE != DTS_TYPE_2) &&
			(Metadata.DTS_FRAME_TYPE != DTS_TYPE_3)
			)
		{
			// We should have a known frame type here, but we don't so bail out
			NumStereoPairsStuffed = (Metadata.NBLKS + 1) * 32;
		}
		else
		{
			NumStereoPairsStuffed = Metadata.DTS_FRAME_TYPE;
		}

		// Write 4 sample AES header to WAV file
		int dataType;
		int numSamples = 2 * NumStereoPairsStuffed;
		switch(Metadata.DTS_FRAME_TYPE)
		{
			case DTS_TYPE_1:
				dataType = 11;
				break;
			case DTS_TYPE_2:
				dataType = 12;
				break;
			case DTS_TYPE_3:
				dataType = 13;
				break;
			default:
				dataType = 14;
				break;
		}
		AES_Header AES( 16, dataType, numSamples);

		// Write AES Header to WAV file

		int samplesWritten = 0;
		for (samplesWritten = 0; samplesWritten < 4; samplesWritten++)
		{
			switch (samplesWritten)
			{
				case 0:
					this->OutputFile->WriteSample(AES.Pa);
					break;
				case 1:
					this->OutputFile->WriteSample(AES.Pb);
					break;
				case 2:
					this->OutputFile->WriteSample(AES.Pc);
					break;
				case 3:
					this->OutputFile->WriteSample(AES.Pd);
					break;
			}
		}

		// Write DTS frame to WAV file
		// Take the bytes of DTSFrame in pairs and write each one out as a sample
		int samplesToWrite = (Metadata.FSIZE + 1) / 2; // We take care if there is an odd number of bytes below

        if ((Metadata.FSIZE + 1) > *FrameSize)
		{
            samplesToWrite = (*FrameSize + 1) / 2; /* +1 to roundup when doing integer arithmetic */
		}

		samplesToWrite += samplesWritten; // The size of the AES header - the amount we have already written
		unsigned char *source = *Frame;
		for (;(samplesWritten < samplesToWrite) && (samplesWritten < numSamples); samplesWritten++)
		{
			unsigned short temp = (source[0] << 8) | source[1];
			OutputFile->WriteSample( temp );
			source += 2;
            (*FrameSize) -= 2;
		}

		if ((Metadata.FSIZE + 1) % 2)
		{
			// ODD number so we have 1 more byte to write into a sample
			if (samplesWritten < numSamples)
			{
				OutputFile->WriteSample( (source[0] << 8) );
				source++;
                (*FrameSize) -= 2;
			}
		}

		*Frame = source;

		// Pad out with samples depending on DTS Frame type

		OutputFile->WriteSample(0x0);
		samplesWritten++;
		for ( ; samplesWritten < numSamples; samplesWritten++)
		{
			OutputFile->WriteSample(0x0);
		}

		return true;
	}
}
