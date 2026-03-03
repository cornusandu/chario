#pragma once

#include <cstdio>
#include <bit>

#ifdef _WIN32
extern int(*fileno)(FILE*);
#endif

typedef unsigned char uchar;

namespace abi {
    bool is_le();
    bool is_be();
}
