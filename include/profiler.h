#pragma once

#include "libr1k.h"
#include "_no_copy.h"
#include <ctime>

namespace libr1k
{
    class block_timer : public _no_copy
    {
    public:
        block_timer(clock_t &time_store) : duration(time_store)
        {
            duration = 0;
            start = clock();
        }

        ~block_timer()
        {
            duration = clock() - start;
        }
    private:
        clock_t start;
        clock_t &duration;
    };
};