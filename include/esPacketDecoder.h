#pragma once

#include "libr1k.h"
#include "base\_no_copy.h"
#include "DataBuffer.h"
#include "SampleBuffer.h"
#include "Types.h"
#include <queue>
#include <ostream>
#include "utils\Log.h"

namespace libr1k
{
    class au_esPacket_t : public _no_copy
    {
        // Stores only a pointer to the data, data must be maintained elsewhere.
    public:
        au_esPacket_t(shared_ptr<Log> log = nullptr) : logger(log), frame_length(0) {}
        au_esPacket_t(const uint8_t *dat, const int len, shared_ptr<Log> log = nullptr) 
            : 
            logger(log), data(data), data_length(len), frame_length(0) {}

        virtual ~au_esPacket_t() {}

        virtual ostream& write(std::ostream &os) = 0;
        virtual ostream& write_csv(std::ostream &os) = 0;
        virtual ostream& write_csv_header(std::ostream &os) = 0;

        virtual bool preProcessHeader() = 0;
        virtual void setBuffer(const uint8_t *data, const int length) 
        {
            this->data = data; 
            data_length = length;
        }
        virtual int decode() = 0;

        virtual int getFrameSize() const
        {
            return frame_length;
        }

    protected:
        shared_ptr<Log> logger;
        const uint8_t *data;
        int data_length;

        int frame_length;
    private:
    };

    
    class esPacketDecoder : public _no_copy
    {
    public:
        static const bool CREATE_DECODER;
        
        // Maintains a queue of smart pointers to data blocks, which it creates if they
        // don't already exist.

        // Push buffers of elementary stream data to this class via addData member functions
    public:

        esPacketDecoder(shared_ptr<Log> log = nullptr) : 
            logger(nullptr), 
            esBuffer(new DataBuffer_u8()),
            au_frame_decoder(nullptr),
            remaingDataIncomplete(false) {}

        esPacketDecoder(uint8_t * const pData, const int dataLength, shared_ptr<Log> log = nullptr) :
            logger(nullptr),
            esBuffer(new DataBuffer_u8(pData, dataLength)),
            au_frame_decoder(nullptr),
            remaingDataIncomplete(false)
        {
        }

        esPacketDecoder(shared_ptr<DataBuffer_u8> const pData, shared_ptr<Log> log = nullptr) :
            logger(nullptr),
            esBuffer(new DataBuffer_u8(pData->data(), pData->size())),
            au_frame_decoder(nullptr),
            remaingDataIncomplete(false)
        {
        }

        virtual void addData(uint8_t *const pData, const int dataLength)
        {
            if (esBuffer == nullptr)
            {
                esBuffer = shared_ptr<DataBuffer_u8>(new DataBuffer_u8(pData, dataLength));
            }
            else
            {
                esBuffer->add(pData, dataLength);
            }
            remaingDataIncomplete = false;
        }

        virtual void addData(shared_ptr<DataBuffer_u8> const pData)
        {
            if (esBuffer == nullptr)
            {
                esBuffer = shared_ptr<DataBuffer_u8>(new DataBuffer_u8(pData->data(), pData->size()));
            }
            else
            {
                esBuffer->add(pData->data(), pData->size());
            }
            remaingDataIncomplete = false;
        }

        shared_ptr<au_esPacket_t> GetDecoder() { return au_frame_decoder; }

        virtual std::shared_ptr<SampleBuffer> DecodeFrame() = 0;
       
    protected:
        std::shared_ptr<Log> logger;
        std::shared_ptr<DataBuffer_u8> esBuffer;
        std::shared_ptr<au_esPacket_t> au_frame_decoder;

        bool remaingDataIncomplete;
        
        virtual int CopyDataToOutputFrame(shared_ptr<DataBuffer_u8> src, shared_ptr<SampleBuffer> dst, int numBytes) = 0;

    };
};
