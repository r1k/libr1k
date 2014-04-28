#include "AC3.h"
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
	const int au_ac3_t::frmsizcod_table[3][frmsizcod_max + 1] =
	{
		/* 48 Khz   */{ 64, 64, 80, 80, 96, 96, 112, 112, 128, 128, 160, 160, 192, 192, 224, 224, 256, 256, 320, 320, 384, 384, 448, 448, 512, 512, 640, 640, 768, 768, 896, 896, 1024, 1024, 1152, 1152, 1280, 1280 },
		/* 44.1 Khz */{ 69, 70, 87, 88, 104, 105, 121, 122, 139, 140, 174, 175, 208, 209, 243, 244, 278, 279, 348, 349, 417, 418, 487, 488, 557, 558, 696, 697, 835, 836, 975, 976, 1114, 1115, 1253, 1254, 1393, 1394 },
		/* 32 Khz   */{ 96, 96, 120, 120, 144, 144, 168, 168, 192, 192, 240, 240, 288, 288, 336, 336, 384, 384, 480, 480, 576, 576, 672, 672, 768, 768, 960, 960, 1152, 1152, 1344, 1344, 1536, 1536, 1728, 1728, 1920, 1920 }		
	};

	const int au_ac3_t::bitrate_table[frmsizcod_max + 1] =
	{ 32, 32, 40, 40, 48, 48, 56, 56, 64, 64, 80, 80, 96, 96, 112, 112, 128, 128, 160, 160, 192, 192, 224, 224, 256, 256, 320, 320, 384, 384, 448, 448, 512, 512, 576, 576, 640, 640 };

	const char *au_ac3_t::fscod_str[] = { "48 kHz", "44.1kHz", "32kHz", "reserved" };

	const char *au_ac3_t::bsmod_str[] = {
		"main audio service : complete main(CM)",
		"main audio service : music and effects(ME)",
		"associated service : visually impaired(VI)",
		"associated service : hearing impaired(HI)",
		"associated service : dialogue(D)",
		"associated service : commentary(C)",
		"associated service : emergency(E)",
		"associated service : voice over(VO)",
		"main audio service : karaoke" };

	const char *au_ac3_t::acmod_str[] = {
		"1 + 1 Ch1, Ch2",
		"1 / 0 C",
		"2 / 0 L, R",
		"3 / 0 L, C, R",
		"2 / 1 L, R, S",
		"3 / 1 L, C, R, S",
		"2 / 2 L, R, SL, SR",
		"3 / 2 L, C, R, SL, SR"
	};

	const char *au_ac3_t::cmixlev_str[] = {
		"0.707 (–3.0 dB)",
		"0.595 (–4.5 dB)",
		"0.500 (–6.0 dB)",
		"reserved"
	};

	const char *au_ac3_t::surmixlev_str[] = {
		"0.707 (–3.0 dB)",
		"0.500 (–6.0 dB)",
		"0",
		"reserved"
	};

	const char *au_ac3_t::dsurmod_str[] = {
		"not indicated",
		"Not Dolby Surround encoded",
		"Dolby Surround encoded",
		"reserved"
	};

	const char *au_ac3_t::roomtyp_str[] = {
		"not indicated",
		"large room, X curve monitor",
		"small room, flat monitor",
		"reserved"
	};
	

	const char *au_ac3_t::on_off_str[] = { "Off", "On"};

	au_ac3_t::au_ac3_t(shared_ptr<Log> log) :
        au_esPacket_t(log),
		CRC1(0),
		fscod(0), frmsizcod(0), bsid(0),
		bsmod(0), acmod(0), cmixlev(0),
		surmixlev(0), dsurmod(0), lfeon(0),
		dialnorm(0), compre(0), compr(0),
		langcode(0), langcod(0), audprodie(0),
		mixlevel(0), roomtyp(0), dialnorm2(0),
		compr2e(0), compr2(0), langcod2e(0),
		langcod2(0), audprodi2e(0), mixlevel2(0),
		roomtyp2(0), copyrightb(0), origbs(0),
		xbsi1e(0), dmixmod(0), ltrtcmixlev(0),
		ltrtsurmixlev(0), lorocmixlev(0), lorosurmixlev(0),
		xbsi2e(0), dsurexmod(0), dheadphonmod(0),
		adconvtyp(0), xbsi2(0), encinfo(0),
		addbsie(0), addbsil(0),
		CRC2(0)
	{
		memset(addbsi,0, additional_bsi_max);
        if (logger.get()) logger->AddMessage(Log::MIN_LOG_LEVEL, "au_ac3_t constructor - no buffer");

        liba52 = std::make_shared<liba52_wrapper>();
	}

    au_ac3_t::au_ac3_t(const uint8_t * const buf, const int bufSize, shared_ptr<Log> log) :
        au_esPacket_t(log),
        CRC1(0),
        fscod(0), frmsizcod(0), bsid(0),
        bsmod(0), acmod(0), cmixlev(0),
        surmixlev(0), dsurmod(0), lfeon(0),
        dialnorm(0), compre(0), compr(0),
        langcode(0), langcod(0), audprodie(0),
        mixlevel(0), roomtyp(0), dialnorm2(0),
        compr2e(0), compr2(0), langcod2e(0),
        langcod2(0), audprodi2e(0), mixlevel2(0),
        roomtyp2(0), copyrightb(0), origbs(0),
        xbsi1e(0), dmixmod(0), ltrtcmixlev(0),
        ltrtsurmixlev(0), lorocmixlev(0), lorosurmixlev(0),
        xbsi2e(0), dsurexmod(0), dheadphonmod(0),
        adconvtyp(0), xbsi2(0), encinfo(0),
        addbsie(0), addbsil(0),
        CRC2(0)
	{
		memset(addbsi, 0, additional_bsi_max);
		setBuffer(buf, bufSize);
        if (logger.get()) logger->AddMessage(Log::MIN_LOG_LEVEL, "au_ac3_t constructor - with buffer");

        liba52 = std::make_shared<liba52_wrapper>();

        int length = 0;
        if (preProcessHeader(buf, bufSize, length))
        {
            decode();
        }
	}

    bool au_ac3_t::preProcessHeader(const uint8_t *data, const int data_length, int& frame_length) const
    {
        if (data_length < 5)
            return false;

        if (*data == BYTE_1(syncword) && *(data + 1) == BYTE_0(syncword))
        {
            bool error_detected = false;
            const unsigned int fscod = (*(data + 4) >> 6) & 0x03;
            const unsigned int frmsizcod = *(data + 4) & 0x3f;
            const int max_frame_length = frmsizcod_table[2][frmsizcod_max] * 2;

            if (fscod > 2)
            {
                if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_ac3_t::%s - fscod %u illegal value", __FUNCTION__, fscod);
                error_detected = true;
            }

            if (frmsizcod > frmsizcod_max)
            {
                if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_ac3_t::%s - frmsizcod %u > max(%d)", __FUNCTION__, frmsizcod, frmsizcod_max);
                error_detected = true;
            }

            if (!error_detected)
            {
                frame_length = frmsizcod_table[fscod][frmsizcod] * 2;
            }
            if (error_detected || frame_length < 0 || frame_length > max_frame_length)
            {
                if (logger) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_ac3_t::%s - strange frame size (%d bytes)this is going to cause problems", __FUNCTION__, frame_length);
                // set bytesPerSyncframe to something small to try and interpret the header but we then jump it and try to resync 
                return false;
            }
            return true;
        }
        return false;
    }

    int au_ac3_t::decode()
    {
        if (!liba52->a52_in_sync)
        {
            uint8_t *frame_ptr = const_cast<uint8_t *>(data);
            liba52->interpretFrame(frame_ptr);
            
            if (!liba52->a52_in_sync)
            {
                return -1;
            }
        }

        uint8_t *frame_ptr = const_cast<uint8_t *>(data);
        int level = (int)pow(2, 15);
        DataBuffer<uint16_t> outputSamples;
        liba52->decode(frame_ptr, outputSamples, level);

        return 0;
    }

	void au_ac3_t::InterpretFrame()
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

	    // now lets try the a52 library

        uint8_t *frame_ptr = const_cast<uint8_t *>(data);
        liba52->interpretFrame(frame_ptr);

        if (liba52->a52_bytes_to_get == 0)
        {
            if (logger.get()) logger->AddMessage(Log::MIN_LOG_LEVEL, "%s:: a52_syncinfo returned %d bytes to get", __FUNCTION__, liba52->a52_bytes_to_get);
        }
        else
        {
            if (logger.get()) logger->AddMessage(Log::MIN_LOG_LEVEL, "%s:: a52_syncinfo returned %d bytes to get, flags %x, sample rate %d, bitrate %d", 
                __FUNCTION__, liba52->a52_bytes_to_get, liba52->a52_flags, liba52->a52_sample_rate, liba52->a52_bitrate);
        }

		this->syncProcessed = true;
	}

	ostream& au_ac3_t::write_csv_header(std::ostream &os)
	{
		os << "PTS(Hex), PTS(Dec), time from start, syncword, CRC1, fscod,frmsizcod, bitrate,bsid,bsmod,acmod,lfeon,dialnorm,compre,langcode,audprodie," << endl;
		return os;
	}

	ostream& au_ac3_t::write_csv(std::ostream &os)
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

	ostream& au_ac3_t::write(std::ostream &os)
	{
		if (this->syncProcessed)
		{
			os << "AC3 Packet found - PTS " << PTS << "\n";
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

	ostream &operator<<(ostream &stream, au_ac3_t& meta)
	{
		return meta.write(stream);
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

	AC3PacketHandler::AC3PacketHandler(ofstream **str, bool Debug_on)
		:
		FrameCount(0),
		DebugOn(Debug_on),
        ac3Decoder(new AC3Decoder()),
		outStream(*str),
		LogFile(nullptr),
		firstPTS(0),
		PacketSpansPES(false)
	{
		stream_id = (char)0xbd;
		//OutputFile = new WAVFile(*str, &(this->WAVParams));

		if (DebugOn)
		{
            LogFile = std::shared_ptr<Log>(new Log(Log::MIN_LOG_LEVEL));
		}
        initDecoder();
        ac3Decoder->initDecoder();

		ac3Decoder->ac3_decoder->write_csv_header(*outStream);

        
	}

	void AC3PacketHandler::SetDebugOutput(bool On)
	{
		if (DebugOn != On)
		{
			if (On)
			{
				LogFile.reset(new Log());
				DebugOn = On;
			}
			else
			{
				LogFile = nullptr;
				DebugOn = On;
			}
		}
	}

	void AC3PacketHandler::PESDecode(PESPacket_t *buf)
	{
#if 0
		// Read PES header
		StreamTime::getInstance().StartTime_Set(buf->PTS);

		uint8_t *PES_data = &(buf->payload[9]) + buf->payload[8]; // The start of the data is the number of bytes in the PES header length field
		// added to the first byte after the PES header length field
		// Need to adjust PESPacketSize to make it just the payload size
		int PESPacketSize = buf->nextFreeByte - PES_data;
		
		// add data to decoder buffer
		ac3Decoder->newData(PES_data, PESPacketSize);	

        DecodeFrame(&PES_data, &PESPacketSize);
#endif
	}

    bool AC3PacketHandler::DecodeFrame(unsigned char **Frame, unsigned int *FrameSize)
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