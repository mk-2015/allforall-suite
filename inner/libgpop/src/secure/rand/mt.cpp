#include <secure/rand/randalg.hpp>
using namespace gpop::Secure::Rand;
#include <stddef.h>
#include <stdint.h>
#include <secure/rand/random.hpp>
#include "in.hpp"

#define MT_N 312
#define MT_M 156U
#define MT_MATRIX_A UINT64_C(0xB5026F5AA96619E9)
#define MT_UPPER_MASK UINT64_C(0xFFFFFFFF80000000)
#define MT_LOWER_MASK UINT64_C(0x000000007FFFFFFF)

void mt_seed(gpop::Secure::Rand::randalg_t *randalg) {
    if (randalg == NULL || randalg->random_state == NULL) {
        return;
    }

    uint64_t seed_val = (uint64_t)randalg->random_seed;
    if (seed_val == 0ULL) {
        seed_val = 5489ULL;
    }

    randalg->random_state[0] = seed_val;

    for (unsigned int i = 1U; i < MT_N; ++i) {
        randalg->random_state[i] =
            UINT64_C(6364136223846793005) *
            (randalg->random_state[i - 1U] ^
             (randalg->random_state[i - 1U] >> 62U)) +
            i;
    }

    randalg->random_state[MT_N] = MT_N;
}

static void mt_twist(gpop::Secure::Rand::randalg_t *randalg) {
    for (unsigned int i = 0U; i < MT_N; ++i) {
        unsigned long long x =
            (randalg->random_state[i] & MT_UPPER_MASK) |
            (randalg->random_state[(i + 1U) % MT_N] & MT_LOWER_MASK);

        randalg->random_state[i] =
            randalg->random_state[(i + MT_M) % MT_N] ^ (x >> 1U);

        if (x & 1ULL) {
            randalg->random_state[i] ^= MT_MATRIX_A;
        }
    }
}

static RANDOM_SEED mt_temper(unsigned long long value) {
    value ^= (value >> 29U) & UINT64_C(0x5555555555555555);
    value ^= (value << 17U) & UINT64_C(0x71D67FFFEDA60000);
    value ^= (value << 37U) & UINT64_C(0xFFF7EEE000000000);
    value ^= value >> 43U;

    return (RANDOM_SEED)value;
}

void mersennetwist(gpop::Secure::Rand::randalg_t *randalg) {
    if (randalg == NULL || randalg->random_state == NULL) {
        return;
    }

    unsigned int idx = (unsigned int)randalg->random_state[MT_N];

    if (idx >= MT_N) {
        if (randalg->random_state[0] == 0ULL &&
            randalg->random_state[1] == 0ULL) {
            mt_seed(randalg);
        }
        mt_twist(randalg);
        idx = 0U;
    }

    randalg->random_seed = mt_temper(randalg->random_state[idx]);
    randalg->random_state[MT_N] = idx + 1U;
}
