#pragma once

#include "libr1k.h"
#include "_no_copy.h"
#include "DataBuffer.h"
#include "Types.h"
#include <queue>
#include <ostream>
#include "Log.h"

namespace libr1k
{
    class au_esPacket_t : public _no_copy
    {
        // Stores only a pointer to the data, data must be maintained elsewhere.
    public:
        au_esPacket_t(shared_ptr<Log> log = nullptr) : logger(log) {}
        au_esPacket_t(const uint8_t *dat, const int len, shared_ptr<Log> log = nullptr) 
            : 
            logger(log), data(data), data_length(len) {}

        virtual ~au_esPacket_t() {}

        virtual ostream& write(std::ostream &os) = 0;
        virtual ostream& write_csv(std::ostream &os) = 0;
        virtual ostream& write_csv_header(std::ostream &os) = 0;

        virtual bool preProcessHeader(const uint8_t *data, const int length, int& frame_length) const = 0;
        virtual void setBuffer(const uint8_t *data, const int length) 
        {
            this->data = data; 
            data_length = length;
        }
        virtual int decode() = 0;

    protected:
        shared_ptr<Log> logger;
        const uint8_t *data;
        int data_length;
    private:
    };

    class esPacketDecoder : public _no_copy
    {
        // Maintains a queue of smart pointers to data blocks, which it creates if they
        // don't already exist.

        // Push buffers of elementary stream data to this class via addData member functions
    public:

        esPacketDecoder(shared_ptr<Log> log = nullptr) : 
            logger(nullptr), 
            _data(),
            au_frame_decoder(nullptr) {}

        esPacketDecoder(uint8_t * const pData, const int dataLength, shared_ptr<Log> log = nullptr) :
            logger(nullptr),
            _data(),
            au_frame_decoder(nullptr) 
        {
            addData(pData, dataLength);
        }

        esPacketDecoder(shared_ptr<DataBuffer_u8> const pData, shared_ptr<Log> log = nullptr) :
            logger(nullptr),
            _data(),
            au_frame_decoder(nullptr)
        {
            addData(pData);
        }

        virtual void addData(uint8_t *const pData, const int dataLength)
        {
            shared_ptr<DataBuffer_u8> temp(new DataBuffer_u8(pData, dataLength));
            addData(temp);
        }

        virtual void addData(shared_ptr<DataBuffer_u8> const pData)
        {
            _data.push(pData);
        }

        shared_ptr<au_esPacket_t> GetDecoder() { return au_frame_decoder; }
        
    protected:
        shared_ptr<Log> logger;
        std::queue<shared_ptr<DataBuffer_u8>> _data;
        shared_ptr<au_esPacket_t> au_frame_decoder;
    };
};
