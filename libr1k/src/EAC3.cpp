#include "AC3.h"
#include "EAC3.h"
#include "bitOperators.h"
#include "crc.h"
#include "BitStreamReader.h"
#include "pcr.h"
#include <math.h>
#include <memory>
#include <iomanip>
#include <sstream>

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
                frame_length = (frmsiz + 1) * 2; // frmsiz is one less than the overall size of the coded frame in 16 bit words
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
        if (PESQueue.size() == 0)
        {
            return nullptr;
        }

        shared_ptr<SampleBuffer> outputFrame;
        outputFrame = std::shared_ptr<SampleBuffer>(new SampleBuffer());

        esBuffer = PESQueue.front(); // get the current PES packet

        return nullptr;
    }

    std::shared_ptr<SampleBuffer> EAC3Decoder::DecodeFrame_PassThru()
    {
        if (PESQueue.size() == 0)
        {
            return nullptr;
        }

        shared_ptr<SampleBuffer> EAC3AccessUnitOutputFrame;
        EAC3AccessUnitOutputFrame = std::shared_ptr<SampleBuffer>(new SampleBuffer());

        esBuffer = PESQueue.front(); // get the current PES packet

       /*
         Attempt to assemble a single AU from the buffer
        
         The PES can contain 1 or more AUs.
         One AU can contain 1 or more substreams
         The first substream will be streamtype 0 or 2 and substream id 0
         Finding a substream with streamtype 0 or 2 and substream id of 0 
         will indicate the start of a new AU and the end of the last if 
         we are already assembling one.
        
         esBuffer is a shared pointer to the first PES packet in the queue
         the packet is left in the PESqueue until it is finished with
         it may be needed for subsequent calls to this function

         Up to 8 independent substreams each followed by up to 8 
         dependent substreams sequentially numbered.
         independent substream = In
         dependent substream = Dn
         substreamsBlksFound =
         { {I0, D0, D1, D2, D3, D4, D5, D6, D7},
           {I1, D0, D1, D2, D3, D4, D5, D6, D7},
           {I2, D0, D1, D2, D3, D4, D5, D6, D7},
           {I3, D0, D1, D2, D3, D4, D5, D6, D7},
           {I4, D0, D1, D2, D3, D4, D5, D6, D7},
           {I5, D0, D1, D2, D3, D4, D5, D6, D7},
           {I6, D0, D1, D2, D3, D4, D5, D6, D7},
           {I7, D0, D1, D2, D3, D4, D5, D6, D7}
         }  Each slot tracks the number of blocks found
         We need to track this as the blocks can be fragmented
         across the AU e.g.
         First Substream - TYPE 0 ID 0 Blocks 6
               Substream - TYPE 1 ID 0 Blocks 6
               Substream - TYPE 1 ID 1 Blocks 6
               Substream - TYPE 0 ID 1 Blocks 6
               Substream - TYPE 1 ID 0 Blocks 6
               Substream - TYPE 1 ID 1 Blocks 6
               Substream - TYPE 0 ID 2 Blocks 6
               Substream - TYPE 1 ID 0 Blocks 6
               Substream - TYPE 1 ID 1 Blocks 6         would give the table         { {6,6,6,0,0,0,0,0,0},           {6,6,6,0,0,0,0,0,0},           {6,6,6,0,0,0,0,0,0},           {0,0,...         }
         For the AU the total for I0 must be matched by I(1-7) and the total for
         each dependant substream D0 must match its corresponding independant substream
       */

        int substreamsBlksFound[MAX_NUM_SUBSTREAM_IDS][MAX_NUM_SUBSTREAM_IDS + 1] = { 0 };
        bool terminate = false;
        bool error = false;
        bool frameBlockEnded = false;
        int currentIndependentStream = -1;
        int auSize = 0;

        LogMessage(Log::DEFAULT_LOG_LEVEL, "EAC3 Passthru collect AU");
        while (!terminate)
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

                    const bool independentSubstream = (packetInterpreter.strmtyp == 0 || packetInterpreter.strmtyp == 2) ? true : false;
                    if (independentSubstream)
                    {
                        currentIndependentStream = packetInterpreter.substreamid;
                    }
                    
                    if (independentSubstream && currentIndependentStream == 0)
                    {
                        // First Independant substream
                        if (substreamsBlksFound[currentIndependentStream][0] < 6)
                        {
                            LogMessage(Log::DEFAULT_LOG_LEVEL, "EAC3 Passthru First Substream - TYPE %d ID %d Blocks %d Frame Size %d bytes",
                                                               packetInterpreter.strmtyp, 
                                                               packetInterpreter.substreamid, 
                                                               packetInterpreter.getNumAudioBlocks(),
                                                               packetInterpreter.getFrameSize());

                            substreamsBlksFound[currentIndependentStream][0] += packetInterpreter.getNumAudioBlocks();
                        }
                        else
                        {
                            // This should be the start of the next time period so we have received all
                            // the data for this 1536 sample period, the frame we have synced-to is for
                            // the next period
                            LogMessage(Log::DEFAULT_LOG_LEVEL, "EAC3 Passthru next AU Substream found - TYPE %d ID %d Blocks %d Frame Size %d bytes", 
                                                               packetInterpreter.strmtyp, 
                                                               packetInterpreter.substreamid,
                                                               packetInterpreter.getNumAudioBlocks(),
                                                               packetInterpreter.getFrameSize());
                            terminate = true;
                            frameBlockEnded = true;
                            break;
                        }
                    }
                    else
                    {
                        LogMessage(Log::DEFAULT_LOG_LEVEL, "EAC3 Passthru       Substream - TYPE %d ID %d Blocks %d Frame Size %d bytes", 
                                                            packetInterpreter.strmtyp, 
                                                            packetInterpreter.substreamid,
                                                            packetInterpreter.getNumAudioBlocks(),
                                                            packetInterpreter.getFrameSize());
                        if (independentSubstream)
                        {
                            substreamsBlksFound[currentIndependentStream][0] += packetInterpreter.getNumAudioBlocks();
                        }
                        else
                        {
                            substreamsBlksFound[currentIndependentStream][packetInterpreter.substreamid + 1] += packetInterpreter.getNumAudioBlocks();
                        }
                    }

#if 0
                    if (substreamsBlksFound[substream_ref] > 6)
                    {
                        // Error this shouldn't happen
                        terminate = true;
                        error = true;
                        break;
                    }
#endif
                    // Is the whole frame available
                    const int frameSize = packetInterpreter.getFrameSize();
                    auSize += frameSize;
                    if (esBuffer->size() < frameSize)
                    {
                        // Not enough data available
                        terminate = true;
                        error = true;
                    }
                    else
                    {
                        CopyDataToOutputFrame(esBuffer, EAC3AccessUnitOutputFrame, frameSize);
                        esBuffer->remove(frameSize);

                        if (esBuffer->size() < 6)  // Beware magic numbers....
                        {
                            // Data has been used up no more AU in this PES and they're not allowed to span PES packets
                            terminate = true;
                            frameBlockEnded = true; // Assume we got the last packet as we ran out of PES
                        }
                    }
                }
                break;

            case SYNC_INCOMPLETE:
                // We ran out of data reading the header
                // save the current state and bail out.
                LogMessage(Log::DEFAULT_LOG_LEVEL, "EAC3 SYNC_INCOMPLETE");
                terminate = true;
                error = true;
                break;

            case SYNC_NOT_FOUND:
            default:
                LogMessage(Log::DEFAULT_LOG_LEVEL, "EAC3 SYNC_NOT_FOUND");
                terminate = true;
                error = true;
                break;
            }
        }

        bool removePESpacket = false;

        if (frameBlockEnded)
        {
            // Check all substream units for correct block configuration
            LogMessage(Log::DEFAULT_LOG_LEVEL, "Number of blocks found per frame:");
            

            bool blockError = false;
            const int blocksRequired = substreamsBlksFound[0][0];

            for (int i = 0; i < MAX_NUM_SUBSTREAM_IDS; i++)
            {
                if (substreamsBlksFound[i][0] == 0)
                    break;
                stringstream ss("\tIndependent substream: ", ios_base::app | ios_base::out);
                ss << i << " : ";

                for (int k = 0; k <= MAX_NUM_SUBSTREAM_IDS; k++)
                {
                    ss << substreamsBlksFound[i][k] << ", ";
                    if (substreamsBlksFound[i][k] > 0 && substreamsBlksFound[i][k] != blocksRequired)
                    {
                        blockError = true;
                        error = true;
                    }
                }
                LogMessage(Log::DEFAULT_LOG_LEVEL, ss.str());
            }

            if (blockError)
            {
                LogMessage(Log::DEFAULT_LOG_LEVEL, "EAC3 Too many blocks found in this AU");
            }

            if (esBuffer->size() < 6)
            {
                // No more data left in this PES packet we should remove it from the queue
                removePESpacket = true;
            }
        }
        else
        {
            LogMessage(Log::DEFAULT_LOG_LEVEL, "EAC3Decoder::%s - Didn't get enough data in this PES packet", __FUNCTION__);

            // ran out of data in this PES packet should throw it away and start fresh next time
            removePESpacket = true;
        }

        LogMessage(Log::DEFAULT_LOG_LEVEL, "EAC3Decoder::%s AU size = %d", __FUNCTION__, auSize);

        if (error)
        {
            removePESpacket = true;
            EAC3AccessUnitOutputFrame = nullptr; // we should return null as we don't have a correct frame
        }

        if (removePESpacket)
        {
            PESQueue.pop();
        }

        return EAC3AccessUnitOutputFrame;
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
        LogMessage(Log::DEFAULT_LOG_LEVEL, "PES Packet : PTS %d", buf->PTS);
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
            const AES_Header AES(16, dataType, NumSubFrames);

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