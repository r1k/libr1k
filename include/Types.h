#pragma once

#ifdef WIN32
#include <stdint.h>
#else
#include <stdint.h>
#include <linux/types.h>

typedef uint64_t ulong64_t;
typedef int64_t long64_t;
#endif
