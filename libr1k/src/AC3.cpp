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
	}

    au_ac3_t::au_ac3_t(DataBuffer_u8 * const buffer, shared_ptr<Log> log) :
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

        setBuffer(buffer->data(), buffer->size());

        if (logger.get()) logger->AddMessage(Log::MIN_LOG_LEVEL, "au_ac3_t constructor - with buffer");

        liba52 = std::make_shared<liba52_wrapper>();
    }

    bool au_ac3_t::preProcessHeader()
    {
        if (data_length < 5)
            return false;

        if (*data == BYTE_1(SYNCWORD) && *(data + 1) == BYTE_0(SYNCWORD))
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
		os << SYNCWORD << ",";
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

    AC3Decoder::SyncStatus AC3Decoder::FindSyncWord(shared_ptr<DataBuffer_u8> buf)
	{
        if (buf->size() < 6)
        {
            return SYNC_NOT_FOUND;
        }

        int i = 0;
        uint16_t syncword_test = (*buf)[i] << 8 | (*buf)[i + 1];

        while (syncword_test != au_ac3_t::SYNCWORD && (i + 1) < buf->size())
        {
            i++;
            syncword_test = (*buf)[i] << 8 | (*buf)[i + 1];
        }

        // remove data before syncword
        buf->remove(i);
        if (buf->size() > 6)
        {
            // stopped early must have found syncword
            // check first bsid to see if correct
            bool errorFound = false;
            const int bsid = ((*buf)[5] >> 3) & 0x01f;
            const unsigned int fscod = ((*buf)[4] >> 6) & 0x03;
            const unsigned int frmsizcod = (*buf)[4] & 0x3f;

            if (bsid != au_ac3_t::BSID)
            {
                errorFound = true;
            }

            if (fscod > 2)
            {
                if (logger.get()) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_ac3_t::%s - fscod %u illegal value", __FUNCTION__, fscod);
                errorFound = true;
            }

            if (frmsizcod > au_ac3_t::frmsizcod_max)
            {
                if (logger.get()) logger->AddMessage(Log::DEFAULT_LOG_LEVEL, "au_ac3_t::%s - frmsizcod %u > max(%d)", __FUNCTION__, frmsizcod, au_ac3_t::frmsizcod_max);
                errorFound = true;
            }
            
            if (errorFound)
            {
                // Skip over and continue the search
                buf->remove(1);
                return FindSyncWord(buf);
            }
            else
            {
                return SYNC_FOUND;
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

    int AC3Decoder::CopyDataToOutputFrame(shared_ptr<DataBuffer_u8> src, shared_ptr<SampleBuffer> dst, int numBytes)
    {
        // 16 bit copy so round up the number of bytes when converting to number of words
        int NumWords = (numBytes + 1) / 2;
        int srcIndex = 0;

        dst->setBitDepth(16);

        for (int i = 0; i < NumWords; i++)
        {
            int word = (*src)[srcIndex++];
            word <<= 8;
            word |= (*src)[srcIndex++];
            dst->add(&word, 1, 16);
        }

        return NumWords;
    }

    int AC3Decoder::MoveDataToOutputFrame(shared_ptr<DataBuffer_u8> src, shared_ptr<SampleBuffer> dst, int numBytes)
    {
        int NumWords = (numBytes + 1) / 2;
        int srcIndex = 0;

        dst->setBitDepth(16);

        for (int i = 0; i < NumWords; i++)
        {
            int word = (*src)[srcIndex++];
            word <<= 8;
            word |= (*src)[srcIndex++];
            dst->add(&word, 1, 16);
        }

        src->remove(srcIndex);

        return NumWords;
    }

    std::shared_ptr<SampleBuffer> AC3Decoder::DecodeFrame_PassThru()
    {
        if (esBuffer->size() == 0)
        {
            return nullptr;
        }

        shared_ptr<SampleBuffer> AC3AccessUnitOutputFrame;
        AC3AccessUnitOutputFrame = std::shared_ptr<SampleBuffer>(new SampleBuffer());

        bool error = false;
        bool incomplete = false;
        int auSize = 0;

        LogMessage(Log::DEFAULT_LOG_LEVEL, "AC3 Passthru collect AU");
        
        AC3Decoder::SyncStatus success = FindSyncWord(esBuffer);

        switch (success)
        {
            case SYNC_FOUND:
                {
                    // Interpret the frame
                    au_ac3_t packetInterpreter(esBuffer.get());
                    packetInterpreter.preProcessHeader();
                    packetInterpreter.InterpretFrame();

                    // Is the whole frame available
                    const int frameSize = packetInterpreter.getFrameSize();
                    auSize += frameSize;
                    if (esBuffer->size() < frameSize)
                    {
                        // Not enough data available
                        incomplete = true;
                    }
                    else
                    {
                        CopyDataToOutputFrame(esBuffer, AC3AccessUnitOutputFrame, frameSize);
                        esBuffer->remove(frameSize);
                    }
                }
                break;
            case SYNC_INCOMPLETE:
                // We ran out of data reading the header
                // save the current state and bail out.
                LogMessage(Log::DEFAULT_LOG_LEVEL, "AC3 SYNC_INCOMPLETE");
                incomplete = true;
                break;

            case SYNC_NOT_FOUND:
            default:
                LogMessage(Log::DEFAULT_LOG_LEVEL, "AC3 SYNC_NOT_FOUND");
                error = true;
                break;
        }
        

        if (incomplete)
        {
            AC3AccessUnitOutputFrame = nullptr; // we should return null as we don't have a correct frame
        }

        if (error)
        {
            esBuffer->clear();
            AC3AccessUnitOutputFrame = nullptr; // we should return null as we don't have a correct frame
        }

        return AC3AccessUnitOutputFrame;
    }
    
	AC3PacketHandler::AC3PacketHandler(ofstream *str, bool Debug_on)
		:
		FrameCount(0),
		DebugOn(Debug_on),
		outStream(str),
        OutputWAV(nullptr),
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
       
        wav_params wv_params;
        wv_params.bit_depth = 16;
        wv_params.num_channels = 2;
        wv_params.SamplesPerSec = 48000;

        OutputWAV = shared_ptr<WAVFile>(new WAVFile(outStream, wv_params));
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
        // Need to use all the bytes in the PES packet or they will
        // 
        // Read PES header

        uint8_t *PES_data = buf->GetPESData();
        //// added to the first byte after the PES header length field
        //// Need to adjust PESPacketSize to make it just the payload size
        unsigned int PES_data_size = buf->GetPESDataLength();

        if (!PES_data_size)
            return;

        GetDecoder()->addData(PES_data, PES_data_size);

        shared_ptr<SampleBuffer> decoded_frame;
        while ((decoded_frame = GetDecoder()->DecodeFrame_PassThru()) != nullptr)
        {
            decoded_frame->setBitDepth(16);
            // Each frame should be output every 1536 samples
            const int FrameSamples = AC3Decoder::DECODED_FRAME_SIZE;
            const int NumSubFrames = FrameSamples * 2;
            const int dataType = 1;
            const int dataLength = decoded_frame->size();

            int outputSubFrames = 0;
            int sourceSubFrames = 0;
            const AES_Header AES(16, dataType, dataLength);
            
            while (sourceSubFrames < decoded_frame->size() && outputSubFrames < NumSubFrames)
            {
                switch (outputSubFrames)
                {
                case 0:
                    OutputWAV->WriteSample16(AES.Pa);
                    break;
                case 1:
                    OutputWAV->WriteSample16(AES.Pb);
                    break;
                case 2:
                    OutputWAV->WriteSample16(AES.Pc);
                    break;
                case 3:
                    OutputWAV->WriteSample16(AES.Pd);
                    break;
                default:
                    OutputWAV->WriteSample16(decoded_frame->get(sourceSubFrames++));
                    break;
                }
                outputSubFrames++;
            }

            // Pad out to fill spacing
            while (outputSubFrames < NumSubFrames)
            {
                OutputWAV->WriteSample16(0);
                outputSubFrames++;
            }
        }
	}

#if 0
    bool AC3PacketHandler::DecodeFrame(unsigned char **Frame, unsigned int *FrameSize)
	{

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
			ac3Decoder->write_csv(outStream);

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
		


	}
#endif
}