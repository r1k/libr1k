#include "utils\BigNum.h"

#include <exception>

using namespace std;


namespace libr1k
{
    int64_t multiply(const int16_t a, const int32_t b)
    {
        const int32_t b_upper = b >> 16;
        const int32_t b_lower = b & 0x0ffff;
        const int32_t sum_upper = b_upper * a;
        const int32_t sum_lower = b_lower * a;

        const int64_t answer = (sum_upper << 16) + sum_lower;
        return answer;
    }

    int32_t multiply_16by32to32withlsr(const int16_t a, const int32_t b, const int rs)
    {
        // y = a x b
        // implementation for a machine that has no size greater than 32 bit
        // a = 16 bit
        // b = 32 bit
        // b = b_upper + b_lower where b_upper = b & 0xffff0000, b_lower = b & 0x0ffff
        // y = a x( b_upper + b_lower)
        // y = a x b_upper + a x b_lower
        // still has the problem of a x b_upper > 32 bit
        // a x (b_upper >> 16) will give an answer in 32 bit range, just need to track it.

        if (rs > 16)
        {
            throw exception();
        }
        const int32_t b_u = b >> 16;
        const int32_t b_l = b & 0x0ffff;
        int32_t ab_u = b_u * a;
        int32_t ab_l = b_l * a;

        // handle carry so lower half is only max 16bits needed for shift to work
        int32_t carry = ab_l >> 16;
        ab_l &= 0x0ffff;
        ab_u += carry;
        
        // get the lowest rs bits from the upper word and put them into the top most rs 16bits of upper_bits_for_lower
        uint32_t upper_bits_for_lower = (ab_u << (16 - rs)) & 0x0ffff;

        // combine upper bottom rs bits into lower half
        ab_l = upper_bits_for_lower | (ab_l >> rs);
        // shift upper half down by rs bits and then rescale up by 16 bitsready to add to bottom half
        ab_u = ab_u >> rs;

        return (ab_u << 16) + ab_l;
    }

    int64_t multiply_rs(const int16_t a, const int32_t b, const int rs)
    {
        int64_t r = multiply(a, b);
        return r >> rs;
    }

    int64_t multiply(const int32_t a, const int32_t b)
    {
        // y = a * b
        // a = au + al
        // b = bu + bl
        // y = (au + al) * (bu + bl)
        // y = au * bu + au * bl + al * bu + al * bl

        const int32_t bu = b >> 16;
        const int32_t bl = b & 0x0ffff;
        const int32_t au = a >> 16;
        const int32_t al = a & 0x0ffff;

        const int32_t t1 = au * bu;
        const int32_t t2 = au * bl;
        const int32_t t3 = al * bu;
        const int32_t t4 = al * bl;

        const int64_t answer = (int64_t(t1) << 32)  + (t2 << 16) + (t3 << 16) + t4;
        
        return answer;
    }

    int64_t multiply_rs(const int32_t a, const int32_t b, const int rs)
    {
        int64_t r = multiply(a, b);
        return r >> rs;
    }

    void multiply(BigInt &ret, const int64_t a, const int64_t b)
    {
        // y = a * b
        // a = au + al
        // b = bu + bl
        // y = (au + al) * (bu + bl)
        // y = au * bu + au * bl + al * bu + al * bl

        const int64_t bu = b >> 32 ;
        const int64_t bl = b & 0x0ffffffff;
        const int64_t au = a >> 32;
        const int64_t al = a & 0x0ffffffff;

        const int64_t t1 = au * bu;
        const int64_t t2 = au * bl;
        const int64_t t3 = al * bu;
        const int64_t t4 = al * bl;

        ret.power = 32;

        ret.vals.push_back(t4);
        int carry = 0;
        if ((INT32_MAX - t2) > t3)
        {
            ret.vals.push_back(t3 - (INT32_MAX - t2));
            carry++;
        }
        else
        {
            ret.vals.push_back(t2 + t3);
        }
        ret.vals.push_back(t1 + carry);

        return;
    }

    void multiply_rs(BigInt &ret, const int64_t a, const int64_t b, const int rs)
    {
        multiply(ret, a, b);
        right_shift(ret, rs);
    }
#if 0
    void multiply(BigInt &ret, BigInt &x, BigInt &y)
    {
        BigInt *a;
        BigInt *b;

        if (x.power > 32 || y.power > 32)
            throw exception();

        // currently only manage same powers
        if (x.power != y.power)
            throw exception();
        

        if (x.vals.size > y.vals.size)
        {
            a = &x;
            b = &y;
        }
        else
        {
            a = &y;
            b = &x;
        }

        ret.power = a->power;

        typedef vector<int64_t> inner_vect;
        vector<inner_vect> intermediate;
        intermediate.resize(a->vals.size());

        int count = 0;
        for (auto &a_part : a->vals)
        {
            inner_vect &ref_inner = intermediate[count];

            for (auto &b_part : b->vals)
            {
                ref_inner.push_back( a_part * b_part );
            }

            count++;
        }

    }

    void multiply_rs(BigInt &ret, BigInt &a, BigInt &b, const int rs)
    {
        multiply(ret, a, b);
        right_shift(ret, rs);
    }
#endif
    void right_shift(BigInt &val, const int rs)
    {

    }

    bool operator==(const BigInt &l, const BigInt &r)
    {
        if (r.power != l.power)
        {
            return false;
        }

        if (r.vals != l.vals)
        {
            return false;
        }

        return true;
    }
    bool operator!=(const BigInt &l, const BigInt &r)
    {
        return !(l == r);
    }


    void multiply_test(void)
    {
#if 0
        {
            int32_t a = -125000;
            int16_t b = 1000;

            int64_t y = multiply(b, a);

            if (y != (a*b))
            {
                throw exception();
            }
        }

        {
            int32_t a = -125000;
            int32_t b = 1000;

            int64_t y = multiply(b, a);

            if (y != (a*b))
            {
                throw exception();
            }
        }
#endif

        {
            // this will test lower half overflow and carry
            int16_t a = 32006;
            int32_t b = 163076;
            int right_shift = 3;
            int64_t y_native = (int64_t(a) * b) >> right_shift;

            int32_t y = multiply_16by32to32withlsr(a, b, right_shift);


            if (y != y_native)
            {
                throw exception();
            }
        }
        {
            // this will test lower half overflow and carry and negative
            int16_t a = -32000;
            int32_t b = 163072;
            int right_shift = 6;
            int64_t y_native = (int64_t(a) * b) >> right_shift;

            int32_t y = multiply_16by32to32withlsr(a, b, right_shift);


            if (y != y_native)
            {
                throw exception();
            }
        }
        {
            // this will test lower half overflow and carry and negative other way
            int16_t a = 32000;
            int32_t b = -163072;
            int right_shift = 12;
            int64_t y_native = (int64_t(a) * b) >> right_shift;

            int32_t y = multiply_16by32to32withlsr(a, b, right_shift);


            if (y != y_native)
            {
                throw exception();
            }
        }

#if 0 
        {
            BigInt result;

            int64_t a = 0x1234567890;
            int64_t b = 0x2345678901;

            // expected result = 2895899851425088890
            int64_t expected = 2895899851425088890;
            BigInt expected_result;
            expected_result.power = 32;
            expected_result.vals.push_back(expected & 0x0FFFFFFFF);
            expected_result.vals.push_back((expected >> 32) & 0xFFFFFFFF);

            multiply(result, a, b);

            if ((result != expected_result))
            {
                throw exception();
            }

        }
#endif

    }
}