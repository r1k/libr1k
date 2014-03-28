#pragma once

#include <memory>
#include "libr1k.h"
#include "SampleBuffer.h"

namespace libr1k
{
    class IFilterConf_t
    {
    public:
        virtual ~IFilterConf_t() {};
    };

    class IFilterResults_t
    {
    public:
        virtual ~IFilterResults_t() {};
    };

    class IAudioFilter
    {
    public:
        virtual void Configure(std::shared_ptr<IFilterConf_t> conf) = 0;
        virtual void Process(SampleBuffer &data) = 0;
        virtual void Process(SampleBuffer_norm &data_double) = 0;
        virtual std::shared_ptr<IFilterResults_t> GetResults() = 0;
        virtual void Reset() = 0;

        virtual ~IAudioFilter() {};
    };
}