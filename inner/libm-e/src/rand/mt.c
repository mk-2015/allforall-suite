#include <stddef.h>
#include <stdint.h>
#include <rand/random.h>
#include <rand/randalg.h>
#include "in.h"

#define MT_N 312
#define MT_M 156U
#define MT_MATRIX_A UINT64_C(0xB5026F5AA96619E9)
#define MT_UPPER_MASK UINT64_C(0xFFFFFFFF80000000)
#define MT_LOWER_MASK UINT64_C(0x000000007FFFFFFF)

void mt_seed(randalg_t *randalg) {
    unsigned int i;

    randalg->random_state[0] = (unsigned long long)randalg->random_seed & UINT64_MAX;

    for (i = 1U; i < MT_N; ++i) {
        randalg->random_state[i] =
            UINT64_C(6364136223846793005) *
            (randalg->random_state[i - 1U] ^
             (randalg->random_state[i - 1U] >> 62U)) +
            i;
    }
}

static void mt_twist(randalg_t *randalg) {
    unsigned int i;

    for (i = 0U; i < MT_N; ++i) {
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

static RANDOM_OUT mt_temper(unsigned long long value) {
    value ^= (value >> 29U) & UINT64_C(0x5555555555555555);
    value ^= (value << 17U) & UINT64_C(0x71D67FFFEDA60000);
    value ^= (value << 37U) & UINT64_C(0xFFF7EEE000000000);
    value ^= value >> 43U;

    return (RANDOM_OUT)value;
}

void mersennetwist(randalg_t *randalg) {
    unsigned int idx;

    if (randalg == NULL) {
        return;
    }

    randalg->algorithm = RANDALG_MERSENNE_TWISTER;

    if (randalg->random_seed == 0LL) {
        randalg->random_seed = 5489LL;
    }

    if (randalg->random_state[0] == 0ULL &&
        randalg->random_state[1] == 0ULL &&
        randalg->random_state[MT_N - 1U] == 0ULL) {
        mt_seed(randalg);
    }

    idx = (unsigned int)(randalg->random_seed % MT_N);

    if (idx == 0U) {
        mt_twist(randalg);
    }

    randalg->random_out = mt_temper(randalg->random_state[idx]);

    if (randalg->isMixOn) {
        randalg->random_out ^= (RANDOM_OUT)randalg->mix_seed;
    }

    randalg->random_seed += 1LL;
}
