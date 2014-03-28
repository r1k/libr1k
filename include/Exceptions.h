#pragma once

#include "libr1k.h"

#include <string>
#include <exception>
#include <functional>
#include <stdexcept>

namespace libr1k
{
    class no_data_remaining : public std::out_of_range 
    {
    public:
        no_data_remaining(const std::string& _Message) : out_of_range(_Message){}
        no_data_remaining(const char *_Message) : out_of_range(_Message){}
    };
 
    class filter_not_configured : public std::exception
    {
    public:
        filter_not_configured() {}

        virtual const char *what() const _THROW0()
        {	// return pointer to message string
            return ("Filter used before being configured");
        }
    };

    class bad_filter_format : public std::exception
    {
    public:
        bad_filter_format() {}

        virtual const char *what() const _THROW0()
        {
            return ("Filter used incorrectly");
        }
    };
}