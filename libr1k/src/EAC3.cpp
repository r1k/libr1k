#include "EAC3.h"
#include "bitOperators.h"
#include "crc.h"
#include "BitStreamReader.h"
#include "pcr.h"
#include <math.h>
#include <memory>
#include <iomanip>

#include "liba52_wrapper.h"


namespace libr1k
{
	au_eac3_t::au_eac3_t(shared_ptr<Log> log) :
        au_ac3_t(log)
	{
	}

    au_eac3_t::au_eac3_t(const uint8_t * const buf, const int bufSize, shared_ptr<Log> log) :
        au_ac3_t(buf, bufSize, log)
	{
	}

    bool au_eac3_t::preProcessHeader(const uint8_t *data, const int data_length, int& frame_length) const
    {
        if (data_length < 5)
            return false;

        if (*data == BYTE_1(syncword) && *(data + 1) == BYTE_0(syncword))
        {
            bool error_detected = false;
            const unsigned int fscod = 0;
            const unsigned int frmsizcod = 0;
            const int max_frame_length = 0;

            if (fscod > 2)
            {
                if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_eac3_t::%s - fscod %u illegal value", __FUNCTION__, fscod);
                error_detected = true;
            }

            if (frmsizcod > frmsizcod_max)
            {
                if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_eac3_t::%s - frmsizcod %u > max(%d)", __FUNCTION__, frmsizcod, frmsizcod_max);
                error_detected = true;
            }

            if (!error_detected)
            {
                frame_length = 0;
            }
            if (error_detected || frame_length < 0 || frame_length > max_frame_length)
            {
                if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_eac3_t::%s - strange frame size (%d bytes)this is going to cause problems", __FUNCTION__, frame_length);
                // set bytesPerSyncframe to something small to try and interpret the header but we then jump it and try to resync 
                return false;
            }
            return true;
        }
        return false;
    }

    int au_eac3_t::decode()
    {
        if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_eac3_t::%s - Unable to decode EAC3", __FUNCTION__);

        
        return 0;
    }

	void au_eac3_t::InterpretFrame()
	{
		if (data == nullptr)
		{
			return;
		}
		const uint8_t *ptr = data;

		CRC1 = *(ptr + 2) << 8 | *(ptr + 3);
		ptr += 4;
        bitStreamReader bsr(ptr, data_length);
		bsr.GetXBits(&fscod, 2);
		bsr.GetXBits(&frmsizcod, 6);
		bsr.GetXBits(&bsid, 5);
		bsr.GetXBits(&bsmod, 3);
		bsr.GetXBits(&acmod, 3);

		if ((acmod & 0x01) && (acmod != 0x0))
		{
			bsr.GetXBits(&cmixlev, 2);
		}
		else if (acmod & 0x04)
		{
			bsr.GetXBits(&surmixlev, 2);
		}
		else if (acmod == 0x02)
		{
			bsr.GetXBits(&dsurmod, 2);
		}

		bsr.GetXBits(&lfeon, 1);

		bsr.GetXBits(&dialnorm, 5);

		bsr.GetXBits(&compre, 1);
		if (compre)
		{
			bsr.GetXBits(&compr, 8);
		}

		bsr.GetXBits(&langcode, 1);
		if (langcode)
		{
			bsr.GetXBits(&langcod, 8);
		}

		bsr.GetXBits(&audprodie, 1);
		if (audprodie)
		{
			bsr.GetXBits(&mixlevel, 5);
			bsr.GetXBits(&roomtyp, 2);
		}

		if (acmod == 0)
		{
			bsr.GetXBits(&dialnorm2, 5);
			bsr.GetXBits(&compr2e, 1);
			if (compr2e)
			{
				bsr.GetXBits(&compr2, 8);
			}
			bsr.GetXBits(&langcod2e, 1);
			if (langcod2e)
			{
				bsr.GetXBits(&langcod2, 8);
			}

			bsr.GetXBits(&audprodi2e, 1);
			if (audprodi2e)
			{
				bsr.GetXBits(&mixlevel2, 5);
				bsr.GetXBits(&roomtyp2, 2);
			}
		}
		
		bsr.GetXBits(&copyrightb, 1);
		bsr.GetXBits(&origbs, 1);
		bsr.GetXBits(&xbsi1e, 1);
		if (xbsi1e)
		{
			bsr.GetXBits(&dmixmod, 2);
			bsr.GetXBits(&ltrtcmixlev, 3);
			bsr.GetXBits(&ltrtsurmixlev, 3);
			bsr.GetXBits(&lorocmixlev, 3);
			bsr.GetXBits(&lorosurmixlev, 3);
		}
		bsr.GetXBits(&xbsi2e, 1);
		if (xbsi2e)
		{
			bsr.GetXBits(&dsurexmod, 2);
			bsr.GetXBits(&dheadphonmod, 2);
			bsr.GetXBits(&adconvtyp, 1);
			bsr.GetXBits(&xbsi2, 8);
			bsr.GetXBits(&encinfo, 1);
		}
		bsr.GetXBits(&addbsie, 1);
		if (addbsie)
		{
			bsr.GetXBits(&addbsil, 6);

			/* should loop here to consume number of additional bytes */
			for (unsigned int i = 0; i < (addbsil + 1); i++)
			{
				bsr.GetXBits(&addbsi[i], 8);
			}
		}

	    

		this->syncProcessed = true;
	}

	ostream& au_eac3_t::write_csv_header(std::ostream &os)
	{
		os << "PTS(Hex), PTS(Dec), time from start, syncword, CRC1, fscod,frmsizcod, bitrate,bsid,bsmod,acmod,lfeon,dialnorm,compre,langcode,audprodie," << endl;
		return os;
	}

	ostream& au_eac3_t::write_csv(std::ostream &os)
	{

		os.setf(std::ios::hex, std::ios::basefield);
		os << PTS << ",";

		os.setf(std::ios::dec, std::ios::basefield);
		os << PTS << ",";

		StreamTime &st = StreamTime::getInstance();
		StreamTime::print_time_hms_ms(os, st.time_33_in_ms(st.TimeElapsed(PTS)));
		os << ",";

		os.setf(std::ios::hex, std::ios::basefield);
		os << syncword << ",";
		streamsize wdth = os.width();
		os.fill('0');
		os << setw(4) << CRC1 << ",";
		os.fill(' ');
		os << fscod << ",";
		os << frmsizcod << ",";
		os.setf(std::ios::dec, std::ios::basefield);
		os << bitrate_table[frmsizcod] << ",";
		os.setf(std::ios::hex, std::ios::basefield);
		os << bsid << ",";
		os << bsmod << ",";
		os << acmod << ",";
		os << lfeon << ",";
		os << dialnorm << ",";
		os << compre << ",";
		os << langcode << ",";
		os << audprodie << ",";
		os << endl;
		return os;
	}

	ostream& au_eac3_t::write(std::ostream &os)
	{
		if (this->syncProcessed)
		{
			os << "EAC3 Packet found - PTS " << PTS << "\n";
			os.setf(std::ios::hex, std::ios::basefield);
			os << "\tCRC1      : " << CRC1 << "\n";
			os << "\tfscod     : " << fscod << "\t" << fscod_str[fscod] << "\n";
			os << "\tfrmsizcod : " << frmsizcod << "\t framesize (words): " <<  frmsizcod_table[fscod][frmsizcod] << "\n";
			os.setf(std::ios::dec, std::ios::basefield);
			os << "\tbit-rate  : " << bitrate_table[frmsizcod] << "kbps\n";
			os.setf(std::ios::hex, std::ios::basefield);
			os << "\tbsid      : " << bsid << "\n";

			if (bsmod == 0x07 && acmod > 3)
			{
				os << "\tbsmod     : " << bsmod << "\t" << bsmod_str[bsmod + 1] << "\n";
			}
			else
			{
				os << "\tbsmod     : " << bsmod << "\t" << bsmod_str[bsmod] << "\n";
			}

			os << "\tacmod     : " << acmod << "\t" << acmod_str[acmod] << "\n";
			if ((acmod & 0x01) && (acmod != 0x0))
			{
				os << "\tcmixlev   : " << cmixlev << "\t" << cmixlev_str[cmixlev] << "\n";
			}
			else if (acmod & 0x04)
			{
				os << "\tsurmixlev : " << surmixlev << "\t" << surmixlev_str[surmixlev] << "\n";
			}
			else if (acmod == 0x02)
			{
				os << "\tdsurmod   : " << dsurmod << "\t" << dsurmod_str[dsurmod] << "\n";
			}

			os << "\tlfeon   : " << lfeon << "\t" << on_off_str[lfeon] << "\n";
			os << "\tdialnorm  : " << dialnorm << os.setf(std::ios::dec, std::ios::basefield) << "\t-" << dialnorm << "dB\n";
			os.setf(std::ios::hex, std::ios::basefield);

			os << "\tcompre   : " << compre << "\t" << on_off_str[compre] << "\n";
			if (compre)
			{
				os << "\tcompr      : " << compr << "\n";
			}

			os << "\tlangcode   : " << langcode << "\t" << on_off_str[langcode] << "\n";
			if (langcode)
			{
				os << "\tlangcod      : " << langcod << "\n";
			}

			os << "\taudprodie   : " << audprodie << "\t" << on_off_str[audprodie] << "\n";
			if (audprodie)
			{
				os << "\tmixlevel     : " << mixlevel << "\n";
				os << "\troomtyp      : " << roomtyp << "\n";
			}

			if (acmod == 0)
			{
				os << "\tdialnorm2      : " << dialnorm2 << "\n";

				os << "\tcompr2e   : " << compr2e << "\t" << on_off_str[compr2e] << "\n";
				if (compr2e)
				{
					os << "\tcompr2      : " << compr2 << "\n";
				}
				
				os << "\tlangcod2e   : " << langcod2e << "\t" << on_off_str[langcod2e] << "\n";
				if (langcod2e)
				{
					os << "\tlangcod2      : " << langcod2 << "\n";
				}

				os << "\taudprodi2e   : " << audprodi2e << "\t" << on_off_str[audprodi2e] << "\n";
				if (audprodi2e)
				{
					os << "\tmixlevel2      : " << mixlevel2 << "\n";
					os << "\troomtyp2       : " << roomtyp2 << "\n";
				}
			}

			os << "\tcopyrightb   : " << copyrightb << "\t" << on_off_str[copyrightb] << "\n";
			os << "\torigbs   : " << origbs << "\t" << on_off_str[origbs] << "\n";
			os << "\txbsi1e   : " << xbsi1e << "\t" << on_off_str[xbsi1e] << "\n";
			if (xbsi1e)
			{
				os << "\tdmixmod       : " << dmixmod << "\n";
				os << "\tltrtcmixlev   : " << ltrtcmixlev << "\n";
				os << "\tltrtsurmixlev : " << ltrtsurmixlev << "\n";
				os << "\tlorocmixlev   : " << lorocmixlev << "\n";
				os << "\tlorosurmixlev : " << lorosurmixlev << "\n";
			}

			os << "\txbsi2e   : " << xbsi2e << "\t" << on_off_str[xbsi2e] << "\n";
			if (xbsi2e)
			{
				os << "\tdsurexmod    : " << dsurexmod << "\n";
				os << "\tdheadphonmod : " << dheadphonmod << "\n";
				os << "\tadconvtyp    : " << adconvtyp << "\n";
				os << "\txbsi2        : " << xbsi2 << "\n";
				os << "\tencinfo      : " << encinfo << "\n";
			}

			os << "\taddbsie   : " << addbsie << "\t" << on_off_str[addbsie] << "\n";
			if (addbsie)
			{
				os << "\taddbsil      : " << addbsil << "\n";
			}
			os << flush;
		}
		return os;
	}

#if 0
	int au_ac3_t::FindSyncWord(void)
	{
		const uint8_t *ptr = usedPtr;
		bool syncWordFound = false;
		bool syncFound = false;
		int bytesPerSyncframe = 0;

		while (!syncFound && (usedPtr < endPtr - 5))
		{
			while (usedPtr < endPtr - 5)  //  Need to at least be able to read the syncinfo field
			{
				if (*usedPtr == BYTE_1(syncword) && *(usedPtr + 1) == BYTE_0(syncword))
				{
					bool error_detected = false;
					const unsigned int fscod = (*(usedPtr + 4) >> 6) & 0x03;
					const unsigned int frmsizcod = *(usedPtr + 4) & 0x3f;

					if (fscod > 2)
					{
                        if (logger.get()) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_ac3_t::%s - fscod %u illegal value", __FUNCTION__, fscod);
						error_detected = true;
					}
					if (frmsizcod > frmsizcod_max)
					{
                        if (logger.get()) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_ac3_t::%s - frmsizcod %u > max(%d)", __FUNCTION__, frmsizcod, frmsizcod_max);
						error_detected = true;
					}
					
					if (!error_detected)
					{
						bytesPerSyncframe = frmsizcod_table[fscod][frmsizcod] * 2;
					}
					if (error_detected || bytesPerSyncframe < 0 || bytesPerSyncframe > (frmsizcod_table[2][frmsizcod_max] * 2))
					{
                        if (logger.get()) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_ac3_t::%s - strange frame size (%d bytes)this is going to cause problems", __FUNCTION__, bytesPerSyncframe);
						// set bytesPerSyncframe to something small to try and interpret the header but we then jump it and try to resync 
						bytesPerSyncframe = 20;
					}
					syncWordFound = true;
					break;
				}
				++usedPtr;
			}

			// Now check CRC2
			if (syncWordFound)
			{
				if ((usedPtr + bytesPerSyncframe) < endPtr)
				{
					currentFrameByteSize = bytesPerSyncframe;
					// Full frame of data in the buffer
#if 0
					uint16_t CRC1 = *(ptr + 2) << 8 | *(ptr + 3);
					uint16_t CRC2 = *(ptr + bytesPerSyncframe - 2) << 8 | *(ptr + bytesPerSyncframe - 1);
					int framesize_w = bytesPerSyncframe / 2;
					int CRC1_framesize = (int)(framesize_w >> 1) + (int)(framesize_w >> 3);
					int CRC2_framesize = framesize_w;
					
					int calculatedCRC = calc_16_CRC(AC3_CRC_POLY, (ptr + 2), CRC1_framesize - 2); // calculate the check sum from 4th byte along and then compare to the checksum stored in the packet

                    if (logger.get()) logger->AddMessage(Log::DEBUG_LOG_LEVEL, "%s CRC1 0x%04x", __FUNCTION__, calculatedCRC);
#else
					// CRC calculation not currently working
					syncFound = true;
#endif
				}
				else
				{
                    return AU_AC3_INSUFFICIENT_DATA;
				}
				syncPtr = usedPtr;
			}
		}

		this->syncFound = syncFound;
		if (syncFound)
		{
            return AU_AC3_OKAY;
		}
		else
		{
            return AU_AC3_SYNC_NOT_FOUND;
		}
	}
#endif

	EAC3PacketHandler::EAC3PacketHandler(ofstream **str, bool Debug_on)
		:
        AC3PacketHandler(str, Debug_on)
	{
		
	}


	void EAC3PacketHandler::PESDecode(PESPacket_t *buf)
	{

		// Read PES header

		uint8_t *PES_data = buf->GetPESData();
		//// added to the first byte after the PES header length field
		//// Need to adjust PESPacketSize to make it just the payload size
		unsigned int PESDataSize = buf->GetPESDataLength();

        if (!PESDataSize)
            return;

		DecodeFrame(&PES_data, &PESDataSize);

	}

    bool EAC3PacketHandler::DecodeFrame(unsigned char **Frame, unsigned int *FrameSize)
	{
#if 0
        int return_val = au_ac3_t::AU_AC3_SYNC_NOT_FOUND;
		
        while ((return_val = ac3Decoder->FindSyncWord()) == au_ac3_t::AU_AC3_OKAY)
		{
			if (PacketSpansPES)
			{
				// AU spans from last frame so use PTS from last frame but indicate to use the new one next time.
				PacketSpansPES = false;
			}
			else
			{
				// AU is completely contained in this AU then update the PTS
				ac3Decoder->SetPTS(PTS);
			}
			
			ac3Decoder->InterpretFrame();
			ac3Decoder->write_csv(*outStream);

            if (ac3Decoder->a52_locked())
            {
                ac3Decoder->decode();
            }
			ac3Decoder->JumpOverCurrentFrame();
		}

        if (return_val == au_ac3_t::AU_AC3_INSUFFICIENT_DATA)
		{
			PacketSpansPES = true;
		}
		else
		{
			PacketSpansPES = false;
		}

		return;
#endif		
#if 0
		int NumStereoPairsStuffed = 0;
		// Check metadata and extract FSIZE/NBLKS so we know how much stuffing to add between consecutive DTS frames
		au_ac3_t Metadata(*AC3Frame, *BufferSize);
		if (Metadata.SYNCWORD != DTS_SYNCWORD)
		{
			// This is not a DTS syncword
			BufferSize = 0;
			return false;
		}
		FrameCount++;

		NumStereoPairsStuffed = 1536;
		

		// Write 4 sample AES header to WAV file
		const int dataType = 1;
		const int numSamples = 2 * NumStereoPairsStuffed;
		
		AES_Header AES(16, dataType, numSamples);

		// Write AES Header to WAV file

		int samplesWritten = 0;
		for (samplesWritten = 0; samplesWritten < 4; samplesWritten++)
		{
			switch (samplesWritten)
			{
			case 0:
				OutputFile->WriteSample(AES.Pa);
				break;
			case 1:
				OutputFile->WriteSample(AES.Pb);
				break;
			case 2:
				OutputFile->WriteSample(AES.Pc);
				break;
			case 3:
				OutputFile->WriteSample(AES.Pd);
				break;
			}
		}

		// Write DTS frame to WAV file
		// Take the bytes of DTSFrame in pairs and write each one out as a sample
		int samplesToWrite = (Metadata.FSIZE + 1) / 2; // We take care if there is an odd number of bytes below

		if ((Metadata.FSIZE + 1) > *BufferSize)
		{
			samplesToWrite = (*BufferSize + 1) / 2; /* +1 to roundup when doing integer arithmetic */
		}

		samplesToWrite += samplesWritten; // The size of the AES header - the amount we have already written
		unsigned char *source = *DTSFrame;
		for (; (samplesWritten < samplesToWrite) && (samplesWritten < numSamples); samplesWritten++)
		{
			unsigned short temp = (source[0] << 8) | source[1];
			OutputFile->WriteSample(temp);
			source += 2;
			(*BufferSize) -= 2;
		}

		if ((Metadata.FSIZE + 1) % 2)
		{
			// ODD number so we have 1 more byte to write into a sample
			if (samplesWritten < numSamples)
			{
				OutputFile->WriteSample((source[0] << 8));
				source++;
				(*BufferSize) -= 2;
			}
		}

		*DTSFrame = source;

		// Pad out with samples depending on DTS Frame type

		OutputFile->WriteSample(0x0);
		samplesWritten++;
		for (; samplesWritten < numSamples; samplesWritten++)
		{
			OutputFile->WriteSample(0x0);
		}
#endif
		return false;
	}
}