#include "AC3.h"
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
        au_ac3_t(log), nblkscod(0), frmsiz(0), strmtyp(0), substreamid(0)
	{
	}

    au_eac3_t::au_eac3_t(const uint8_t * const buf, const int bufSize, shared_ptr<Log> log) :
        au_ac3_t(buf, bufSize, log), nblkscod(0), frmsiz(0), strmtyp(0), substreamid(0)
	{
	}

    au_eac3_t::au_eac3_t(DataBuffer<uint8_t> * const buffer, shared_ptr<Log> log) :
        au_ac3_t(buffer, log), nblkscod(0), frmsiz(0), strmtyp(0), substreamid(0)
    {
    }

    bool au_eac3_t::preProcessHeader()
    {
        if (data_length < 5)
            return false;

        if (*data == BYTE_1(SYNCWORD) && *(data + 1) == BYTE_0(SYNCWORD))
        {
            bool error_detected = false;
            const uint8_t *ptr = data + 2;
            bitStreamReader bsr(ptr, data_length);
            bsr.GetXBits(&strmtyp, 2);
            bsr.GetXBits(&substreamid, 3);
            bsr.GetXBits(&frmsiz, 11);
            bsr.GetXBits(&fscod, 2);
            
            if ((frmsiz + 1) > MAX_FRAME_LEN)
            {
                if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_eac3_t::%s - frmsiz %u > max(%d)", __FUNCTION__, frmsiz, MAX_FRAME_LEN);
                error_detected = true;
            }

            if (!error_detected)
            {
                frame_length = frmsiz + 1; 
            }

            if (error_detected || frame_length < 0 || frame_length > MAX_FRAME_LEN)
            {
                if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_eac3_t::%s - strange frame size (%d bytes)this is going to cause problems", __FUNCTION__, frame_length);
                // set bytesPerSyncframe to something small to try and interpret the header but we then jump it and try to resync 
                return false;
            }

            // for EAC3 need to find number of blocks contained within this frame.
            if (fscod == 0x03)
            {
                nblkscod = 0x3;
            }
            else
            {
                bsr.GetXBits(&nblkscod, 2);
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
		this->syncProcessed = true;
	}

	ostream& au_eac3_t::write_csv_header(std::ostream &os)
	{
		os << "PTS(Hex), PTS(Dec), time from start, syncword, CRC1, frmsiz, bsid,bsmod,acmod,lfeon,dialnorm,compre,langcode,audprodie," << endl;
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
		os << SYNCWORD << ",";
		streamsize wdth = os.width();
		os.fill('0');
		os << setw(4) << CRC1 << ",";
		os.fill(' ');
		os << frmsiz << ",";
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
			os << "\tfrmsiz : " << frmsiz << "\t framesize - 1 (words)" << "\n";
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

    std::shared_ptr<SampleBuffer> EAC3Decoder::DecodeFrame()
    {
        return nullptr;
    }

    std::shared_ptr<SampleBuffer> EAC3Decoder::DecodeFrame_PassThru()
    {
        if (esBuffer == nullptr || esBuffer->size() < 6 || remaingDataIncomplete)
        {
            // Cannot decode at this time
            return nullptr;
        }

        shared_ptr<SampleBuffer> outputFrame;
        int blksFound = 0;
        if (incompleteOutputFrame != nullptr)
        {
            outputFrame = incompleteOutputFrame;
            blksFound = incompleteBlks;
        }
        else
        {
            outputFrame = std::shared_ptr<SampleBuffer>(new SampleBuffer());
        }

        // Attempt to decode a single frame (6 audio blocks worth) from the buffer      
        bool terminate = false;
        while (blksFound < 6 && !terminate)
        {
            AC3Decoder::SyncStatus success = FindSyncWord(esBuffer);

            switch (success)
            {
            case SYNC_FOUND:
                {
                    // Interpret the frame
                    au_eac3_t packetInterpreter(esBuffer.get());
                    packetInterpreter.preProcessHeader();
                    packetInterpreter.InterpretFrame();

                    // Is the whole frame available
                    int frameSize = packetInterpreter.getFrameSize();
                    if (esBuffer->size() < frameSize)
                    {
                        // Not enough data available
                        remaingDataIncomplete = true;
                        terminate = true;
                    }
                    else
                    {
                        CopyDataToOutputFrame(esBuffer, outputFrame, frameSize);
                        esBuffer->remove(frameSize);
                        blksFound += packetInterpreter.getNumAudioBlocks();
                    }
                }
                break;

            case SYNC_INCOMPLETE:
                // We ran out of data reading the header
                // save the current state and bail out.
                remaingDataIncomplete = true;
                terminate = true;
                break;

            case SYNC_NOT_FOUND:
            default:
                terminate = true;
                break;
            }
        }

        if (blksFound > 6)
        {
            // This shouldn't happen - something went wrong
            if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "EAC3 Passthru - Gathered more than 6 blocks");
            outputFrame = nullptr;
            incompleteOutputFrame = nullptr;
            incompleteBlks = 0;
            return nullptr;
        }
        else if (blksFound < 6)
        {
            incompleteOutputFrame = outputFrame;
            incompleteBlks = blksFound;
            return nullptr;
        }
        else
        {
            return outputFrame;
        }
    }

    AC3Decoder::SyncStatus EAC3Decoder::FindSyncWord(shared_ptr<DataBuffer_u8> buf)
    {
        if (buf->size() < 6)
        {
            return SYNC_NOT_FOUND;
        }

        int i = 0;
        uint16_t syncword_test = (*buf)[i] << 8 | (*buf)[i + 1];

        while (syncword_test != au_eac3_t::SYNCWORD && (i + 1) < buf->size())
        {   
            i++;
            syncword_test = (*buf)[i] << 8 | (*buf)[i+1];
        }

        // remove data before syncword
        buf->remove(i);

        if (buf->size() > 6)
        {
            // stopped early must have found syncword
            // check first bsid to see if correct

            int bsid = ((*buf)[i + 5] >> 3) & 0x01f;

            if (bsid == au_eac3_t::BSID)
            {
                return SYNC_FOUND;
            }
            else
            {
                // Skip over and continue the search
                buf->remove(1);
                return FindSyncWord(buf);
            }
        }
        else
        {
            if (buf->size() == 0)
            {
                return SYNC_NOT_FOUND;
            }
            else
            {
                return SYNC_INCOMPLETE;
            }
        }

        return SYNC_NOT_FOUND;
    }

	EAC3PacketHandler::EAC3PacketHandler(ofstream *str, bool Debug_on)
		:
        AC3PacketHandler(str, Debug_on)
	{
        
	}

	void EAC3PacketHandler::PESDecode(PESPacket_t *buf)
	{
        // Need to use all the bytes in the PES packet
        // 

		// Put data into ES buffer

		uint8_t *PES_data = buf->GetPESData();
		// added to the first byte after the PES header length field
		// Need to adjust PESPacketSize to make it just the payload size
        unsigned int PES_data_size = buf->GetPESDataLength();

        if (!PES_data_size)
            return;

        GetDecoder()->addData(PES_data, PES_data_size);

        // Try to get complete frames out of ES buffer
        shared_ptr<SampleBuffer> decoded_frame;
        while ((decoded_frame = GetDecoder()->DecodeFrame_PassThru()) != nullptr)
        {
            decoded_frame->setBitDepth(16);
            // Each frame should be output every 1536 samples
            const int FrameSamples = AC3Decoder::DECODED_FRAME_SIZE;
            const int NumSubFrames = FrameSamples * 2;
            const int dataType = 1;

            int outputSubFrames = 0;
            int sourceSubFrames = 0;
            const AES_Header AES(24, dataType, NumSubFrames);

            while (sourceSubFrames < decoded_frame->size() && outputSubFrames < NumSubFrames)
            {
                switch (outputSubFrames)
                {
                    case 0:
                        OutputWAV->WriteSample(AES.Pa);
                        break;
                    case 1:
                        OutputWAV->WriteSample(AES.Pb);
                        break;
                    case 2:
                        OutputWAV->WriteSample(AES.Pc);
                        break;
                    case 3:
                        OutputWAV->WriteSample(AES.Pd);
                        break;
                    default:
                        OutputWAV->WriteSample(decoded_frame->get(sourceSubFrames++));
                        break;
                }
                outputSubFrames++;
            }

            while (outputSubFrames < NumSubFrames)
            {
                OutputWAV->WriteSample(0);
                outputSubFrames++;
            }
        }
	}
    
}