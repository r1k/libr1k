#pragma once

#include "libr1k.h"
#include "Types.h"

#include <vector>

namespace libr1k
{
    // This module is for performing shifts for Big numbers, mainly for DSP simulation of fixed point
    // maths hence why some of these aren't really necessary on PC architecture where we have 64 bit 
    // ints and doubles.  This is also the reason why these are done as C functions not C++ classes


    // this type is for holding large integers
    // the vector vals holds the components
    // the power holds the power multiplyer for each part
    //
    // e.g. (vals[0] * pow(2,0*power)) + (vals[1] * pow(2,1*power)) + (vals[2] * pow(2, 2*power)) + ...
    typedef struct
    {
        std::vector<int64_t> vals;
        int power;

    }BigInt;

    // 16 bit * 32 bit with logical shift right to fit result into 32 bit
    int32_t multiply_16by32to32withlsr(const int16_t a, const int32_t b, const int rs);

    int64_t multiply_16_32(const int16_t a, const int32_t b);
    int64_t multiply_16_32_rs(const int16_t a, const int32_t b, const int rs);

    int64_t multiply_32_32(const int32_t a, const int32_t b);
    int64_t multiply_32_32(const int32_t a, const int32_t b, const int rs);

    void multiply(BigInt &ret, const int64_t a, const int64_t b);
    void multiply(BigInt &ret, const int64_t a, const int64_t b, const int rs);
#if 0
    void multiply(BigInt &ret, BigInt &a, BigInt &b);
    void multiply(BigInt &ret, BigInt &a, BigInt &b, const int rs);
#endif
    void right_shift(BigInt &val, const int rs);

    void multiply_test(void);
}