#pragma once

#include <cstring>
#include <memory>
#include "libr1k.h"
#include "_no_copy.h"

#include "Types.h"

#include <vector>

namespace libr1k
{
    template<typename T>
    class DataBuffer
    {
    public:
        typedef std::vector<T> T_vect;

        DataBuffer() : _data() { }
        DataBuffer(const int size) : _data(size) {}
        DataBuffer(const T *pData, const int dataLength) : _data() 
        { 
            set(pData, dataLength); 
        }

        virtual ~DataBuffer() { }

        virtual DataBuffer<T> &operator=(DataBuffer<T> &src)
        {
            _data = src.vect();

            return *this;
        }

        virtual void set_size(const int newSize)
        {
            _data.resize(newSize);
        }
       
        virtual void set(const T *pData, const int dataLength)
        {
            _data.assign(pData, pData + dataLength);
        }

        virtual void add(const T *pData, const int dataLength)
        {
            _data.insert(_data.end(), pData, pData + dataLength);
        }

        virtual T &operator[](const int i)
        {
            return _data[i];
        }

        virtual typename T_vect::iterator remove(const unsigned int amount)
        {
            if (amount <= _data.size())
            {
                return _data.erase(_data.begin(), _data.begin() + amount);
            }
            return _data.begin();
        }

        virtual T *data() 
        { 
            return _data.data(); 
        }

        virtual T_vect &vect() 
        {
            return _data;
        }

        virtual int size() const 
        { 
            return _data.size(); 
        }

    private:
        T_vect _data;

        // Prevent Copy constructor
        DataBuffer(const DataBuffer<T> &src) = delete;
    };

    // Common data buffer typedefs

    typedef DataBuffer<uint8_t>  DataBuffer_u8;
    typedef DataBuffer<uint16_t> DataBuffer_u16;
    typedef DataBuffer<uint32_t> DataBuffer_u32;
    typedef DataBuffer<int>      DataBuffer_int;
    typedef DataBuffer<double>   DataBuffer_double;
};
