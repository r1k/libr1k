#pragma once

#include "libr1k.h"
#include "base\_no_copy.h"
#include "AudioDSPFilters.h"
#include <vector>

namespace libr1k
{
    class Audio_Goertzel_conf : public IFilterConf_t
    {
    public:
        int N;                          // Block length
        int fs;                         // Sampling frequency
        int detection_threshold;        // threshold to recognise a signal
        std::vector<int> frequencies;   // List of frequencies to detect
        

    public:
        std::vector<double> _k;          // pre calculated coefficient
        std::vector<double> _A;          // pre calculated coefficient
        std::vector<double> _B;          // pre calculated coefficient
        std::vector<double> _S1;         // filter delay store z-1
        std::vector<double> _S2;         // filter delay store z-2
    };

    class Audio_Goertzel_results : public IFilterResults_t
    {
    public:
        bool             tone_detected; // was a tone detected
        std::vector<int> magnitude;     // Signal levels coming out
    };

    class Audio_Goertzel_filter :  public IAudioFilter, _no_copy
    {
    public:
        Audio_Goertzel_filter() :
            configured(false),
            conf(nullptr),
            results_ready(false),
            results(new Audio_Goertzel_results)
        { }

        Audio_Goertzel_filter(std::shared_ptr<IFilterConf_t> params) :
            Audio_Goertzel_filter()
        {
            Configure(params);
        }

        virtual void Configure(std::shared_ptr<IFilterConf_t> params);
        virtual void Process(SampleBuffer &data);
        virtual void Process(SampleBuffer_norm &data);
        virtual std::shared_ptr<IFilterResults_t> GetResults();

        virtual ~Audio_Goertzel_filter() {}

        virtual void Reset() {}

    protected:
        bool configured;
        std::shared_ptr<Audio_Goertzel_conf> conf;
        bool results_ready;
        std::shared_ptr<Audio_Goertzel_results> results;

    private:
        
    };
}