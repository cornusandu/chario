#pragma once

#include "get_encoding.hpp"

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
int encoding_is_compatible(Encoding encoding);
