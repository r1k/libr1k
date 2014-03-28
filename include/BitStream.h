#pragma once

#include "libr1k.h"
#include "_no_copy.h"
#include "DataBuffer.h"

#include <deque>
#include "Types.h"

namespace libr1k
{
    class BitStream : public _no_copy
    {
    public:

        BitStream() : pQ() 
        {
        }
        BitStream(DataBuffer<uint8_t> *pDataB) : pQ()
        {
            push(pDataB);
        }
        BitStream(const uint8_t *pData, const int dataLength) : pQ()
        {
            DataBuffer<uint8_t>* const temp = new DataBuffer<uint8_t>(pData, dataLength);
            push(temp);
        };

        virtual ~BitStream() {  }
        // Add data in blobs to the back of the stream
        virtual void push(DataBuffer<uint8_t> *pData) { pQ.push_back(pData); }

        // Remove data from the head of the stream in bytes, template 
        // type must support tracking the data used and remvoed from it
        virtual void pop(const int dataLength) {}

        // Get data from the head of the stream
        virtual void get(DataBuffer<uint8_t>* pData, const int dataLength)
        { 
            
        }

    private:
        std::deque<DataBuffer<uint8_t> *>  pQ;
    };
};
