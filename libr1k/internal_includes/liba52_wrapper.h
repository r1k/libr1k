#pragma once

#include "base\_no_copy.h"
#include "Types.h"
#include "DataBuffer.h"

extern "C"{
#include "a52.h"
#include "mm_accel.h"
}

namespace libr1k
{
    class liba52_wrapper : public _no_copy
    {
    public:
        liba52_wrapper() :
            a52_state(nullptr),
            a52_in_sync(false),
            a52_flags(0),
            a52_sample_rate(0),
            a52_bitrate(0),
            a52_bytes_to_get(0),
            nchannels(0)
        {
            a52_state = a52_init(MM_ACCEL_ALL);
            decoded_samples = a52_samples(a52_state);
        }

        liba52_wrapper(uint8_t *src) :
            a52_state(nullptr),
            a52_in_sync(false),
            a52_flags(0),
            a52_sample_rate(0),
            a52_bitrate(0),
            a52_bytes_to_get(0),
            nchannels(0)
        {
            a52_state = a52_init(MM_ACCEL_ALL);
            decoded_samples = a52_samples(a52_state);

            interpretFrame(src);
        }

        ~liba52_wrapper() { }

        virtual void interpretFrame(uint8_t *src);

        bool a52_in_sync;
        int a52_flags;
        int a52_sample_rate;
        int a52_bitrate;
        int a52_bytes_to_get;

        int nchannels;

        virtual void decode(uint8_t *src, DataBuffer<uint16_t> &buf, int scale);

    private:
        sample_t *decoded_samples;
        a52_state_t *a52_state;
        
    };
}
