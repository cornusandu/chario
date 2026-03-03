#include <cstdio>
#include <bit>
#include "compat.hpp"

#ifdef _WIN32
int(*fileno)(FILE*) = _fileno;
#endif

bool abi::is_le() {
    if constexpr (std::endian::native == std::endian::little) {
        return true;
    } else {
        return false;
    }
}

bool abi::is_be() {
    if constexpr (std::endian::native == std::endian::big) {
        return true;
    } else {
        return false;
    }
}

