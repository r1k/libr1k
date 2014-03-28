#include "AudioDSPFilters_Goertzel.h"
#include "Exceptions.h"
#include "maths.h"
#include <memory>
#include <algorithm>

using namespace std;
namespace libr1k
{
    void Audio_Goertzel_filter::Configure(std::shared_ptr<IFilterConf_t> params)
    {
        conf = dynamic_pointer_cast<Audio_Goertzel_conf>(params);

        if (!conf) throw std::bad_cast();

        int num_freqs = conf->frequencies.size();

        // pre-allocate the coeffiecient buffers
        conf->_k.resize(num_freqs);
        conf->_A.resize(num_freqs);
        conf->_B.resize(num_freqs);
        conf->_S1.resize(num_freqs);
        conf->_S2.resize(num_freqs);
        
        results->magnitude.resize(num_freqs);

        for (int i = 0; i < num_freqs; i++)
        {
            const double freq = double (conf->frequencies[i]);

            // calculate coefficients upfront
            conf->_k[i] = conf->frequencies[i] * conf->N / double(conf->fs);
            conf->_A[i] = 2 * PI * conf->_k[i] / conf->N;
            conf->_B[i] = 2 * cos(conf->_A[i]);
            // reset store of filter history
            conf->_S1.assign(num_freqs, 0);
            conf->_S2.assign(num_freqs, 0);
        }
        configured = true;
    }

    void Audio_Goertzel_filter::Process(SampleBuffer &data)
    {
        // This filter needs to be fed with normalised data
        throw bad_filter_format();
    }

    void Audio_Goertzel_filter::Process(SampleBuffer_norm &data)
    {
        results_ready = false;

        if (!configured) throw filter_not_configured();

        int num_freqs = conf->frequencies.size();

        // IIR filter so reset before use
        conf->_S1.assign(num_freqs, 0);
        conf->_S2.assign(num_freqs, 0);

        for (auto i = data.vect().cbegin(); i < data.vect().cend(); i += 2) // jump 2 only looking at left sample
        {
            for (int j = 0; j < num_freqs; j++)
            {
                double s0 = 0;
                s0 = *i + conf->_B[j] * conf->_S1[j] - conf->_S2[j];
                conf->_S2[j] = conf->_S1[j];
                conf->_S1[j] = s0;
            }
        }

        for (int j = 0; j < num_freqs; j++)
        {
            results->magnitude[j] = int32_t( (conf->_S1[j] * conf->_S1[j]) + (conf->_S2[j] * conf->_S2[j]) - (conf->_B[j] * conf->_S1[j] * conf->_S2[j]) );
        }

        results_ready = true;
    }

    std::shared_ptr<IFilterResults_t> Audio_Goertzel_filter::GetResults()
    { 
        if (!results_ready)
            return nullptr;
        else
            return results; 
    }

}