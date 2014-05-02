#pragma once

#include <cstring>
#include <memory>
#include "libr1k.h"
#include "_no_copy.h"

#include "Types.h"
#include "DataBuffer.h"
#include <vector>

namespace libr1k
{
    class SampleBuffer_norm;

    class SampleBuffer : public DataBuffer_int
    {
        // By default the samples are store with the MSB pushed all
        // the way to the left so that they are all 32bit samples
        // original 16/20/24 bit samples will all have the same magnitude
        // they just have different resolution.
        // point will be 32 to reflect this.
        // Call adjustFixedPoint to change this.

    public:
        SampleBuffer() : point(32), bitdepth(32) {}
        SampleBuffer(const int size) : DataBuffer_int(size), point(32), bitdepth(32)  { }
        SampleBuffer(const int *pData, const int dataLength) : DataBuffer_int(pData, dataLength), point(32), bitdepth(32)  {}
        SampleBuffer(SampleBuffer& src);
        virtual ~SampleBuffer() {}

        virtual SampleBuffer &operator=(SampleBuffer_norm &src);
        virtual SampleBuffer &operator=(SampleBuffer &src); // need to implement to add copying point

        virtual void adjustFixedPoint(const int point);

        virtual int getFixedPoint() const { return point; }

        virtual void setBitDepth(const int depth)
        {
            bitdepth = depth;
        }

        virtual const int getBitDepth() const
        {
            return bitdepth;
        }

        virtual int put(const int i, const int val, const int depth)
        {
            (vect()[i]) = val << ((sizeof(int)* 8) - depth);
            return val;
        }

        virtual int get(const int i)
        {
            // This returns the bit shifted value
            
            return (vect()[i]) >> ((sizeof(int)* 8) - bitdepth);
        }

    private:
        int point;
        int bitdepth;

        // SampleBuffer(const SampleBuffer& src) = delete;
    };

    class SampleBuffer_norm : public DataBuffer_double
    {
    public:
        SampleBuffer_norm() { }
        SampleBuffer_norm(const int size) : DataBuffer_double(size) {}
        SampleBuffer_norm(const double *pData, const int dataLength) : DataBuffer_double(pData, dataLength) {}
        SampleBuffer_norm(SampleBuffer_norm &src);
        virtual ~SampleBuffer_norm() {}

        virtual SampleBuffer_norm &operator=(SampleBuffer &src);

    private:
        // SampleBuffer_norm(const SampleBuffer_norm& src) = delete;
    };

};
