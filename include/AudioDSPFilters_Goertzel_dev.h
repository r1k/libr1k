#pragma once

#include "libr1k.h"
#include "_no_copy.h"
#include "AudioDSPFilters.h"
#include <vector>

namespace libr1k
{

    // Fixed point implementation testing framework
    // see this document for notation
    // http://www.digitalsignallabs.com/fp.pdf
    // Fixed-Point Arithmetic: An Introduction by Randy Yates    //
    // A(x,y) means a signed value using x bits for int part and y for mantissa
    // there fore there required number of bits to store is 1+x+y
    // U(x,y) means an unsigned value using x bits for int part and y for mantissa
    // there fore there required number of bits to store is x+y


    class Audio_Goertzel_DEV_conf : public IFilterConf_t
    {
    public:
        int N;                          // Block length
        int fs;                         // Sampling frequency
        int detection_threshold;        // threshold to recognise a signal
        std::vector<int> frequencies;   // List of frequencies to detect
        
    public:
        std::vector<double> _B;          // pre calculated coefficient
        std::vector<double> _S1;         // filter delay store z-1
        std::vector<double> _S2;         // filter delay store z-2

        std::vector<int16_t> _B_i;          // pre calculated coefficient
        std::vector<int32_t> _S1_i;         // filter delay store z-1
        std::vector<int32_t> _S2_i;         // filter delay store z-2
    };

    class Audio_Goertzel_DEV_results : public IFilterResults_t
    {
    public:
        bool             tone_detected; // was a tone detected
        std::vector<int> magnitude;     // Signal levels coming out
        std::vector<int> magnitude_i;     // Signal levels coming out
    };

    class Audio_Goertzel_DEV_filter :  public IAudioFilter, _no_copy
    {
    public:
        Audio_Goertzel_DEV_filter() :
            configured(false),
            conf(nullptr),
            results_ready(false),
            results(new Audio_Goertzel_DEV_results),
            store_precision(18), 
            coeff_precision(14)
        { }

        Audio_Goertzel_DEV_filter(std::shared_ptr<IFilterConf_t> params) :
            Audio_Goertzel_DEV_filter()
        {
            Configure(params);
        }

        virtual void Configure(std::shared_ptr<IFilterConf_t> params);
        virtual void Process(SampleBuffer &data);
        virtual void Process(SampleBuffer_norm &data);
        virtual void Process(SampleBuffer_norm &data_f, SampleBuffer &data_i);
        virtual std::shared_ptr<IFilterResults_t> GetResults();

        virtual ~Audio_Goertzel_DEV_filter() {}

        virtual void Reset() {}

    protected:
        bool configured;
        std::shared_ptr<Audio_Goertzel_DEV_conf> conf;
        bool results_ready;
        std::shared_ptr<Audio_Goertzel_DEV_results> results;

    private:
        const int store_precision;
        const int coeff_precision;
    };
}