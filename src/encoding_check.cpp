#include "encoding_check.hpp"
#include "compat.hpp"

// @brief Checks whether an encoding is supported with the current version and platform.
//
// @param[enum class Encoding] encoding The encoding to test
//
// @return
// - 0 if it is fully supported
//
// - 1 if it is ABI-incompatible
//
// - 2 if it is an unsupported encoding
int encoding_is_compatible(Encoding encoding) {
    if (encoding == Encoding::UTF16_BE && abi::is_le()) return 1;
    if (encoding == Encoding::UTF32_BE && abi::is_le()) return 1;
    if (encoding == Encoding::UTF16_LE && abi::is_be()) return 1;
    if (encoding == Encoding::UTF32_LE && abi::is_be()) return 1;
    if (encoding == Encoding::Unsupported)              return 2;

    return 0;
}
