#include "libr1k.h"
#include "SampleBuffer.h"

namespace libr1k
{
    void SampleBuffer::adjustFixedPoint(const int point)
    {
        // Adjust the fixed point of the audio samples, by default
        // they are left shifted up to the full left of the 32 bit 
        // int they are stored in this function will adjust that.
        // e.g. if called with 16 then this will adjust the samples 
        // so the maximum width they can use is the bottom 16.
        if (point == this->point)
        {
            // no shift required just return
            return;
        }
        else if (point < this->point)
        {
            // right shift required
            const int right_shift = this->point - point;

            T_vect &vec = vect();
            for (auto &i : vec)
            {
                i = i >> right_shift;
            }
        }
        else
        {
            // left shift required
            const int left_shift = point - this->point;
            
            T_vect &vec = vect();
            for (auto &i : vec)
            {
                i = i << left_shift;
            }
        }

        this->point = point;
        return;
    }

    SampleBuffer::SampleBuffer(SampleBuffer& src) :
        point(src.getFixedPoint()), bitdepth(32)
    {
        SampleBuffer::operator=(src);
    }

    SampleBuffer &SampleBuffer::operator= (SampleBuffer_norm &src)
    {
        T_vect &vec = vect();

        vec.resize(src.size());

        auto *p = data();
        const int Multiplyer = 0x7fffffff;

        for (auto i : src.vect())
        {
            *p++ = static_cast<int32_t>(i * Multiplyer);
        }

        return *this;
    }

    SampleBuffer_norm::SampleBuffer_norm(SampleBuffer_norm &src)
    {
        SampleBuffer_norm::operator=(src);
    }

    SampleBuffer &SampleBuffer::operator= (SampleBuffer &src)
    {
        DataBuffer_int::operator=(src);
        point = src.getFixedPoint();
        return *this;
    }

    SampleBuffer_norm &SampleBuffer_norm::operator= (SampleBuffer &src)
    {
        T_vect &vec = vect();

        vec.resize(src.size());

        auto *p = data();
        const double divisor = 0x7fffffff;

        for (auto i : src.vect())
        {
            *p++ = i / divisor;
        }

        return *this;
    }
};