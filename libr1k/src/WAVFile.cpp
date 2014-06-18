#include <fstream>
#include "WAVFile.h"
#include "Types.h"
#include "Exceptions.h"

#include <iostream>
#include <iomanip>

#ifndef WIN32
#include <endian.h>
#endif

#ifndef H2LE_INT
#define H2LE_INT (X) htole32(X)
#endif

#ifndef H2LE_SHORT
#define H2LE_SHORT (X) htole16(X)
#endif

using namespace std;

namespace libr1k
{

    wav_params::wav_params(const wav_params& obj)
    {
        CopyObj(obj);
    }

    wav_params& wav_params::operator=(const wav_params& rhs)
    {
        CopyObj(rhs);
        return *this;
    }

    wav_params& wav_params::CopyObj(const wav_params& obj)
    {
        bit_depth = obj.bit_depth;
        num_channels = obj.num_channels;
        num_samples = obj.num_samples;
        SamplesPerSec = obj.SamplesPerSec;

        return *this;
    }

    AES_Header::AES_Header(unsigned int depth, unsigned int data_type, unsigned int num_samples)
    {
        int burst_info = data_type;
        int length_code = num_samples * depth;
        switch (depth)
        {
        case 24:
            burst_info |= 0x20; // Data mode
            burst_info <<= 8;
            Pa = 0x96F872;
            Pb = 0xA54E1F;
            break;
        case 20:
            burst_info |= 0x10; // Data mode
            burst_info <<= 4;
            Pa = 0x6F872;
            Pb = 0x54E1F;
            break;
        case 16:
        default:
            Pa = 0xF872;
            Pb = 0x4E1F;
            break;
        }
        Pc = burst_info;
        Pd = length_code;
    }


#define SIMPLE_WAV

    WAVFile::WAVFile(ofstream *outstr, wav_params &wv_params) :
        input(nullptr),
        fileSize(0),
        InputReady(false),
        ReadCount(0),
        output(outstr),
        local_params(wv_params)
    {
        switch (local_params.bit_depth)
        {
        case 16:
            ByteShift = 2;
            break;
        case 20:
        case 24:
        default:
            ByteShift = 3;
            break;
        }

        writtenCount = 0;

        WriteRIFFChunk();
#ifdef SIMPLE_WAV
        WriteSimpleWAVChunk();
#else
        WriteExtensibleWAVChunk();
#endif
    }

    WAVFile::WAVFile(ifstream *instr) :
        output(nullptr),
        input(instr),
        fileSize(0),
        InputReady(false),
        ReadCount(0),
        local_params()
    {
        input->seekg(0, ios::beg);

        ReadRIFFChunk();

        ReadWAVChunk();
    }

    WAVFile::~WAVFile(void)
    {
        if (output)
        {
#ifdef SIMPLE_WAV
            unsigned int word = writtenCount;
            word = H2LE_INT(word);
            output->seekp(data_chunk_size_location, ios_base::beg);
            output->write((char *)&word, sizeof(word)); // write in the length into the previously left gap

            word = writtenCount + 8 + 4 + 24;  // number of bytes plus header length
#else
            unsigned int word = this->writtenCount + 8 + 4 + 48 + 12;  // number of bytes plus header length
#endif

            word = H2LE_INT(word);

            output->seekp(file_size_location, ios_base::beg); // jump back to the length location in the output file

            output->write((char *)&word, sizeof(word)); // write in the length into the previously left gap

            word = writtenCount;

            output->seekp(data_chunk_size_location, ios_base::beg);

            output->write((char *)&word, sizeof(word)); // write in the length into the previously left gap

            output->seekp(0, ios::end); // jump back to the end of the file

            output->close(); // close the output file
        }

        if (input)
        {
            input->close();
        }
    }

    void WAVFile::GetParams(wav_params *wv_params)
    {
        *wv_params = local_params;
    }

    void WAVFile::WriteRIFFChunk(void)
    {
        if (!output) return;

        unsigned int	word;

        word = RIFF_MARKER;
        output->write((char*)&word, sizeof(word));

        file_size_location = output->tellp();	// record the position where the size field is to fill it in later
        word = 0;
        output->write((char*)&word, sizeof(word)); // insert a gap for the size to be added later on.
    }

    void WAVFile::WriteExtensibleWAVChunk(void)
    {
        if (!output) return;

        unsigned int	word = 0, temp_long = 0;
        unsigned int	bytes_per_sample = 3;
        short		a_short = 0, temp_short = 0;

        word = WAVE_MARKER;
        output->write((char*)&word, sizeof(word));

        word = fmt_MARKER;			/* ckID */
        output->write((char*)&word, sizeof(word));

        word = H2LE_INT(40);			/* cksize */
        output->write((char*)&word, sizeof(word));

        a_short = H2LE_SHORT((short)0xFFFE); /* wFormatTag */
        output->write((char*)&a_short, sizeof(a_short));

        a_short = H2LE_SHORT(local_params.num_channels);		/* nChannels */
        output->write((char*)&a_short, sizeof(a_short));

        word = H2LE_INT(local_params.SamplesPerSec);		/* nSamplesPerSec */
        output->write((char*)&word, sizeof(word));

        temp_long = local_params.SamplesPerSec * ByteShift * local_params.num_channels;
        word = H2LE_INT(temp_long);		/* nAvgBytesPerSec */
        output->write((char*)&word, sizeof(word));

        temp_short = this->ByteShift * local_params.num_channels;
        a_short = H2LE_SHORT(temp_short);	/* nBlockAlign	*/
        output->write((char*)&a_short, sizeof(a_short));

        temp_short = 8 * this->ByteShift;
        a_short = H2LE_SHORT(temp_short);	/* wBitsPerSample */
        output->write((char*)&a_short, sizeof(a_short));

        a_short = H2LE_SHORT(22);		/* cbSize - 22 */
        output->write((char*)&a_short, sizeof(a_short));

        a_short = H2LE_SHORT(24);		/* wValidBitsPerSample */
        output->write((char*)&a_short, sizeof(a_short));

        word = 0;			/* dwChannelMask */
        output->write((char*)&word, sizeof(word));

        /* then 16 bytes of the SubFormat */
        word = H2LE_INT(0x0001);	/* us WAVE_FORMAT_PCM */
        output->write((char*)&word, sizeof(word));

        word = H2LE_INT(0x0000);
        output->write((char*)&word, sizeof(word));

        word = H2LE_INT(0x0000);
        output->write((char*)&word, sizeof(word));

        word = H2LE_INT(0x1000);
        output->write((char*)&word, sizeof(word));

        word = H2LE_INT(0x8000);
        output->write((char*)&word, sizeof(word));

        word = H2LE_INT(0x00AA);
        output->write((char*)&word, sizeof(word));

        word = H2LE_INT(0x0038);
        output->write((char*)&word, sizeof(word));

        word = H2LE_INT(0x9B71);
        output->write((char*)&word, sizeof(word));

    }

    void WAVFile::WriteSimpleWAVChunk(void)
    {
        if (!output) return;

        unsigned int	word = 0, temp_long = 0;
        unsigned int	bytes_per_sample = 3;
        int16_t	        a_short = 0, temp_short = 0;

        word = WAVE_MARKER;
        output->write((char*)&word, sizeof(word));

        word = fmt_MARKER;		/* ckID */
        output->write((char*)&word, sizeof(word));

        word = H2LE_INT(16);		/* cksize */
        output->write((char*)&word, sizeof(word));

        a_short = H2LE_SHORT((short)0x0001); /* wFormatTag */
        output->write((char*)&a_short, sizeof(a_short));

        a_short = H2LE_SHORT(local_params.num_channels);		/* nChannels */
        output->write((char*)&a_short, sizeof(a_short));

        word = H2LE_INT(local_params.SamplesPerSec);		/* nSamplesPerSec */
        output->write((char*)&word, sizeof(word));

        temp_long = local_params.SamplesPerSec * ByteShift * local_params.num_channels;
        word = H2LE_INT(temp_long);		/* nAvgBytesPerSec */
        output->write((char*)&word, sizeof(word));

        temp_short = ByteShift * local_params.num_channels;
        a_short = H2LE_SHORT(temp_short);	/* nBlockAlign	*/
        output->write((char*)&a_short, sizeof(a_short));

        temp_short = 8 * ByteShift;
        a_short = H2LE_SHORT(temp_short);	/* wBitsPerSample */
        output->write((char*)&a_short, sizeof(a_short));

        word = data_MARKER;
        output->write((char*)&word, sizeof(word));

        data_chunk_size_location = output->tellp();	// record the position where the size field is to fill it in later
        word = 0;
        output->write((char*)&word, sizeof(word)); // insert a gap for the size to be added later on.
    }

    void WAVFile::WriteSample(const uint32_t sample)
    {
        if (!output) return;

        int8_t byte;
        uint32_t word = sample;

        switch (local_params.bit_depth)
        {
        case 20:// 20 bits stored in the lowest 20 bits needs to be output as 24 bit with data shifted << 4
        case 24:
            word = word >> 8;
            break;
        case 16:
        default:
            word = word >> 16;
            break;
        }

        for (int index = 0; index < ByteShift; index++)
        {
            byte = word & 0x000000ff;
            output->write((const char *)&byte, 1);
            word = word >> 8;
        }

        writtenCount += ByteShift;
    }

    void WAVFile::WriteSample16(const uint16_t sample)
    {
        uint8_t a_byte = 0;

        a_byte = sample & 0x000000ff;
        output->write((const char *)&a_byte, 1);

        a_byte = (sample & 0x0000ff00) >> 8;
        output->write((const char *)&a_byte, 1);

        writtenCount += 2;
    }

    uint32_t WAVFile::ReadWord(void)
    {
        uint32_t word = 0;

        input->read((char *)&word, sizeof(word));

        if (*input)
            return LE2H_INT(word);
        else
        {
            InputReady = false;
            return 0;
        }
    }
    
    uint16_t WAVFile::ReadShort(void)
    {
        uint16_t word = 0;

        input->read((char *)&word, sizeof(word));

        if (*input)
            return LE2H_SHORT(word);
        else
        {
            InputReady = false;
            return 0;
        }
    }

    void WAVFile::ReadRIFFChunk(void)
    {
        if (!input) return;
        unsigned int word;

        word = ReadWord();

        if (word != RIFF_MARKER) return;

        fileSize = ReadWord();
    }

    void WAVFile::ReadWAVChunk(void)
    {
        if (!input) return;
        unsigned int word;

        word = ReadWord();

        if (word != WAVE_MARKER) return;

        word = ReadWord();

        if (word != fmt_MARKER) return;

        word = ReadWord();

        switch (word)
        {
        case 16:
            return ReadSimpleChunk();
        case 40:
            return ReadExtensibleChunk();
        default:
            return;
        }
    }

    void WAVFile::ReadSimpleChunk(void)
    {
        if (!input) return;
        unsigned short data_short = 0;

        data_short = ReadShort(); /* wFormatTag */

        data_short = ReadShort(); /* nChannels */

        local_params.num_channels = data_short;

        local_params.SamplesPerSec = ReadShort(); /* nSamplesPerSec */
        data_short = ReadShort(); /* nAvgBytesPerSec */
        data_short = ReadShort(); /* nBlockAlign	*/
        data_short = ReadShort(); /* wBitsPerSample */

        local_params.bit_depth = data_short * 8;

        bool data_Marker_found = false;
        while (!input->eof() && !data_Marker_found)
        {
            unsigned int word;
            input->read((char *)&word, sizeof(word));
            word = LE2H_INT(word);
            if (word == data_MARKER)
            {
                data_Marker_found = true;
            }
        }
        if (!data_Marker_found) return;

        local_params.num_samples = ReadWord();

        InputReady = true;
        switch (local_params.bit_depth)
        {
        case 24:
            this->Read__Sample = &WAVFile::ReadSample24;
            break;
        case 20:
            this->Read__Sample = &WAVFile::ReadSample20;
            break;
        case 16:
            this->Read__Sample = &WAVFile::ReadSample16;
            break;
        default:
            break;
        }
    }

    void WAVFile::ReadExtensibleChunk(void)
    {
        if (!input) return;
        uint16_t data_short = 0;
        uint32_t data_word = 0;

        data_short = ReadShort(); /* wFormatTag */

        data_short = ReadShort(); /* nChannels */

        local_params.num_channels = data_short;

        data_short = ReadShort(); /* nSamplesPerSec */
        data_short = ReadShort(); /* nAvgBytesPerSec */
        data_short = ReadShort(); /* nBlockAlign	*/
        data_short = ReadShort(); /* wBitsPerSample */
        data_short = ReadShort(); /* cbSize - 22 */
        data_short = ReadShort(); /* wValidBitsPerSample */
        data_word = ReadWord();   /* dwChannelMask */

        for (int i = 0; i < 8; i++)
        {
            data_word = ReadWord();
        }

        InputReady = true;
    }

    int32_t WAVFile::ReadSample16(void)
    {
        if (!InputReady) return 0;
        if (ReadCount >= local_params.num_samples)
        {
            InputReady = false;
            throw no_data_remaining("Run out of samples");
        }
        unsigned int sample = 0;
        sample = ReadShort();
        sample <<= 16;

        if (!InputReady)
        {
            throw no_data_remaining("Run out of samples");
        }

        ReadCount += 2;
        return sample;
    }

    int32_t WAVFile::ReadSample20(void)
    {
        return (ReadSample24() << 4);
    }

    int32_t WAVFile::ReadSample24(void)
    {
        if (!input || !InputReady) return 0;
        if (ReadCount >= local_params.num_samples)
        {
            InputReady = false;
            throw no_data_remaining("Run out of samples");
        }

        uint8_t a_byte = 0;
        unsigned int sample = 0;

        input->read((char *)&a_byte, 1);
        sample = a_byte;

        input->read((char *)&a_byte, 1);
        sample = (sample << 8) | a_byte;

        input->read((char *)&a_byte, 1);
        sample = (sample << 8) | a_byte;

        if (!*input)
        {
            InputReady = false;
            throw no_data_remaining("Run out of samples");
        }
        ReadCount += 3;
        return sample;
    }

    void WAVFile::ReadSamples(SampleBuffer &buf, const int num)
    {
        // auto then if SampleBuffer changes to use say 16 bit samples this'll still work
        auto *dst = buf.data();

        for (int i = 0; i < num; i++)
        {
            *dst++ = ReadSample();
        }
    }

    void WAVFile::ReadSamples(SampleBuffer_norm &buf, const int num)
    {
        // auto then if SampleBuffer changes to use say 16 bit samples this'll still work
        auto *dst = buf.data();

        double divisor = 0x7fffffff;

        for (int i = 0; i < num; i++)
        {
            *dst++ = ReadSample() / divisor;
        }
    }

    void WAVFile::WriteSamples(SampleBuffer &buf)
    {
        for (int i = 0; i < buf.size(); i++)
        {
            WriteSample(buf.get(i));
        }
        buf.clear();
    }
}