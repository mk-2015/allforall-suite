#pragma once

#include <stdint.h>
#include <stddef.h>
#include <vector>

namespace gpop::Secure {

#ifndef __ERR_TYPEDEF_DEFINED
typedef unsigned long long err;
#define __ERR_TYPEDEF_DEFINED
#endif

using AlgoNGC = int;

using Salt   = std::vector<uint8_t>;
using Pepper = std::vector<uint8_t>;
using HashT  = std::vector<uint8_t>;
using Data   = std::vector<uint8_t>;

} // namespace gpop::Secure
