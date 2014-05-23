#include "AudioDSPFilters_Goertzel_dev.h"
#include "Exceptions.h"
#include "maths.h"
#include "utils\BigNum.h"
#include <memory>
#include <algorithm>

using namespace std;
namespace libr1k
{
    void Audio_Goertzel_DEV_filter::Configure(std::shared_ptr<IFilterConf_t> params)
    {
        conf = dynamic_pointer_cast<Audio_Goertzel_DEV_conf>(params);

        if (!conf) throw std::bad_cast();

        const int num_freqs = conf->frequencies.size();

        // pre-allocate the coeffiecient buffers
        conf->_B.resize(num_freqs);
        conf->_S1.resize(num_freqs);
        conf->_S2.resize(num_freqs);
        results->magnitude.resize(num_freqs);

        conf->_B_i.resize(num_freqs);
        conf->_S1_i.resize(num_freqs);
        conf->_S2_i.resize(num_freqs);
        results->magnitude_i.resize(num_freqs);
        

        double *B_f = conf->_B.data();

        int16_t *B_i = conf->_B_i.data();
        
        for (int i = 0; i < num_freqs; i++)
        {
            const double freq = static_cast<double> (conf->frequencies[i]);

            // calculate coefficients upfront
            //k[i] = conf->frequencies[i] * conf->N / double(conf->fs);
            //A[i] = 2 * PI * k[i] / conf->N;
            // for generalised geortzel algorithm where we don't round k 
            // to an int we can combine the the k component into the 
            // A calculation and cancel out the N value
            const double A = 2 * PI * freq / conf->fs;

            B_f[i] = 2 * cos(A);
            //Fixed point representation of B.
            // A(1,14) signed therefore requiring 16 bits
            // 14 bits for precision seems to give an accurate enough frequency
            // response from calculations.  Any more gives problems with multiplys
            // further down, Going to have to use 40bit int multiply result
            // as it is.
            B_i[i] = static_cast<int16_t>((2 * cos(A) * (1 << coeff_precision)) + 0.5);
            B_f[i] = static_cast<double>(B_i[i]) / (1 << coeff_precision);
        }
        configured = true;
    }

    void Audio_Goertzel_DEV_filter::Process(SampleBuffer &data)
    {
        throw bad_filter_format();
    }

    void Audio_Goertzel_DEV_filter::Process(SampleBuffer_norm &data)
    {
        // This filter needs to be fed with normalised data
        throw bad_filter_format();
    }

    void Audio_Goertzel_DEV_filter::Process(SampleBuffer_norm &data_f, SampleBuffer &data_i)
    {
        results_ready = false;

        if (!configured) throw filter_not_configured();

        const int num_freqs = conf->frequencies.size();

        // IIR filter so reset before use
        conf->_S1.assign(num_freqs, 0);
        conf->_S2.assign(num_freqs, 0);

        // IIR filter so reset before use
        // asssuming range -2048 + 2047.9.. so A(11, 18) to fit into 32 bit. 
        // 
        conf->_S1_i.assign(num_freqs, 0); 
        conf->_S2_i.assign(num_freqs, 0); 

        // improve performance in debugger
        double * pB = conf->_B.data();
        double *pS1 = conf->_S1.data();
        double *pS2 = conf->_S2.data();
        const double *pSample = data_f.data();
        const int loopLimit = data_f.size();
        double s0 = 0;

        int16_t * pB_i = conf->_B_i.data();
        int32_t *pS1_i = conf->_S1_i.data();
        int32_t *pS2_i = conf->_S2_i.data();
        const int32_t *pSample_i = data_i.data();
        const int loopLimit_i = data_i.size();

        int32_t s0_i = 0;

        for (int i = 0; i < loopLimit; i += 2) // jump 2, only interested in left sample
        {
            for (int j = 0; j < num_freqs; j++)
            {
                double B = pB[j];
                double inter1 = B * pS1[j];
                s0 = (double(pSample_i[i]) / 0x8000) + inter1 - pS2[j];
                pS2[j] = pS1[j];
                pS1[j] = s0;

              
                // A(1, 14) * A(11, 18) = A(1+11+1,14+18) need output to be A(11, 18)
                int32_t intermediate = multiply_16by32to32withlsr(pB_i[j], pS1_i[j], coeff_precision);
                
                
                // Fixed point addition, all terms need a common scale so shift the sample to match
                // A(0, 18) + A(11,18) - A(11,18) - must all use same format for addition so shift sample
                s0_i = (pSample_i[i] << (store_precision -15)) + intermediate - pS2_i[j];

                pS2_i[j] = pS1_i[j];
                pS1_i[j] = s0_i;
            }
        }

        for (int j = 0; j < num_freqs; j++)
        {
            results->magnitude[j] = static_cast<int32_t>( (conf->_S1[j] * conf->_S1[j]) + (conf->_S2[j] * conf->_S2[j]) - (conf->_B[j] * conf->_S1[j] * conf->_S2[j]) );
#if 1
            // Convert the store values back away from fixed point ot whole
            // integers, we don't care about any fraction parts of these by now.
            // A(11,18
            const int32_t S1 = conf->_S1_i[j] >> store_precision;
            const int32_t S2 = conf->_S2_i[j] >> store_precision;
            // Need to calculate an intermediate term because it still uses B
            // can then shift back so we have only whole integers and get away 
            // from this whole fixed point mess
            // term is A(1,14) * A(11,0) * A(11,0) = A(1+11+11+1,14), shifting removes fixed point interpretation
            // intermediate1 = A(11,0) * A(11,0) = A(1 + 11,0)
            int32_t intermediate1 = S1 * S2;
            // intermediate2 = A(1,14) * A(12,0);
            int32_t intermediate2 = multiply_16by32to32withlsr(conf->_B_i[j], intermediate1, coeff_precision);
            results->magnitude_i[j] = (S1 * S1) + (S2 * S2) - intermediate2;
#else

            const double S1 = double(conf->_S1_i[j]) / (1 << store_precision);
            const double S2 = double(conf->_S2_i[j]) / (1 << store_precision);
            const double B = double(conf->_B_i[j]) / (1 << coeff_precision);
            results->magnitude_i[j] = static_cast<int32_t>((S1 * S1) + (S2 * S2) - (B*S1*S2));
#endif
            
        }

        results_ready = true;
    }

    std::shared_ptr<IFilterResults_t> Audio_Goertzel_DEV_filter::GetResults()
    { 
        if (!results_ready)
            return nullptr;
        else
            return results; 
    }

}