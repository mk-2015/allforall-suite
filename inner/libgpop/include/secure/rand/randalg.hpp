#pragma once
#include <stdint.h>

namespace gpop::Secure::Rand {

constexpr int RANDALG_LCG = 1;
constexpr int RANDALG_LCG_GLIBC = 11;
constexpr int RANDALG_LCG_POSIX = 12;
constexpr int RANDALG_XORSHIFT = 2;
constexpr int RANDALG_XOSHIRO256SS = 7;
constexpr int RANDALG_SPLITMIX64 = 8;
constexpr int RANDALG_XOROSHIRO128P = 9;
constexpr int RANDALG_XOSHIRO128PP = 10;
constexpr int RANDALG_XOSHIRO256P = 15;
constexpr int RANDALG_PCG_RXS_M_XS = 16;
constexpr int RANDALG_CHACHA20 = 17;
constexpr int RANDALG_SFC64 = 20;
constexpr int RANDALG_HC128 = 19;
constexpr int RANDALG_WELL512A = 18;
constexpr int RANDALG_MERSENNE_TWISTER = 3;
constexpr int RANDALG_PCGR = 4;
constexpr int RANDALG_PCG32 = 13;
constexpr int RANDALG_PCG64 = 14;
constexpr int RANDALG_CSPRNG = 5;
constexpr int RANDALG_TRNG = 6;
constexpr int RANDALG_LOGISTIC_MAP = 21;
constexpr int RANDALG_PHILOX_4X32 = 22;
constexpr int RANDALG_KISS = 23;
constexpr int RANDALG_XOSHIRO256PP = 24;
constexpr int RANDALG_DEFAULT = RANDALG_PCGR;

constexpr int OPEN_RAND_FAILMALLOC = -1;
constexpr int OPEN_RAND_FAILMUTEX = -8;
constexpr int GETRAND_FAILINVLAIDSTRUCT = -2;
constexpr int GENRAND_FAILINVLAIDSTRUCT = -3;
constexpr int GENRAND_FAILINVLAIDALGORITHM = -4;
constexpr int SETOPT_FAILINVLAIDSTRUCT = -5;
constexpr int RESET_FAILINVLAIDSTRUCT = -6;
constexpr int CLOSERAND_FAILINVLAIDSTRUCT = -7;

constexpr int MIX_ON = 1;
constexpr int MIX_NO = 0;

static inline uint64_t rotatel(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}
} // namespace gpop::Secure::Rand
