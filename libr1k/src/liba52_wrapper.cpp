#include "liba52_wrapper.h"

#include <memory>

namespace libr1k{

    static const int ac3_channels[8] = {
        2, 1, 2, 3, 3, 4, 4, 5
    };

    void liba52_wrapper::interpretFrame(uint8_t *src)
    {
        a52_bytes_to_get = a52_syncinfo(src, &a52_flags, &a52_sample_rate, &a52_bitrate);

        nchannels = ac3_channels[a52_flags & 7];
        if (a52_flags & A52_LFE)
            nchannels++;
    }

    void liba52_wrapper::decode(uint8_t *src, DataBuffer<uint16_t> &buf, int scale)
    {

        if (!a52_bytes_to_get)
            interpretFrame(src);

        if (!a52_in_sync)
            return;

        sample_t level = (sample_t)scale;
        int rv = a52_frame(a52_state, src, &a52_flags, &level, 0);

        for (int i = 0; i < 6; ++i)
        {
            if (a52_block(a52_state) != 0)
            {
                std::shared_ptr<uint16_t> intBuffer(new uint16_t[256 * nchannels]);
                // get samples 256 * nchannels * sample_t
                sample_t *samples = a52_samples(a52_state);

                // done this way because I may need to do some conversion
                for (int i = 0; i < 256 * nchannels; i++)
                {
                    *(intBuffer.get() + i) = (uint16_t)samples[i];
                }
                buf.add(intBuffer.get(), 256 * nchannels * sizeof (uint16_t));
            }
        }
    }
}