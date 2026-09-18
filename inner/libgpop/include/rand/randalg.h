#pragma once

#include <stdint.h>

#define RANDALG_LCG                  1
#define RANDALG_LCG_GLIBC            11
#define RANDALG_LCG_POSIX            12
#define RANDALG_XORSHIFT             2
#define RANDALG_XOSHIRO256SS         7
#define RANDALG_SPLITMIX64           8
#define RANDALG_XOROSHIRO128P        9
#define RANDALG_XOSHIRO128PP         10
#define RANDALG_XOSHIRO256P          15
#define RANDALG_PCG_RXS_M_XS         16
#define RANDALG_CHACHA20             17
#define RANDALG_SFC64                20
#define RANDALG_HC128                19
#define RANDALG_WELL512A             18
#define RANDALG_MERSENNE_TWISTER     3
#define RANDALG_PCGR                 4
#define RANDALG_PCG32                13
#define RANDALG_PCG64                14
#define RANDALG_CSPRNG               5
#define RANDALG_TRNG                 6
#define RANDALG_LOGISTIC_MAP         21
#define RANDALG_PHILOX_4X32          22
#define RANDALG_KISS                 23
#define RANDALG_XOSHIRO256PP         24
#define RANDALG_DEFAULT              RANDALG_PCGR

#define OPEN_RAND_FAILMALLOC         -1
#define OPEN_RAND_FAILMUTEX          -8
#define GETRAND_FAILINVLAIDSTRUCT    -2
#define GENRAND_FAILINVLAIDSTRUCT    -3
#define GENRAND_FAILINVLAIDALGORITHM -4
#define SETOPT_FAILINVLAIDSTRUCT     -5
#define RESET_FAILINVLAIDSTRUCT      -6
#define CLOSERAND_FAILINVLAIDSTRUCT  -7

#define MIX_ON                       1
#define MIX_NO                       0

static inline uint64_t rotatel(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}
