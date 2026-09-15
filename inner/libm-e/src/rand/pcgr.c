#include <rand/random.h>
#include <rand/randalg.h>
#include <stdlib.h>
#include <stdint.h>
#include "in.h"

void pcgr(randalg_t* randalg) {
    if (!randalg) return;

#if defined(__SIZEOF_INT128__) || defined(__UINT128_MAX__)
    __uint128_t* state_ptr = (__uint128_t*)&(randalg->random_state[0]);
    __uint128_t* inc_ptr   = (__uint128_t*)&(randalg->random_state[2]);

    if (*state_ptr == 0 && *inc_ptr == 0) {
        __uint128_t initstate = (__uint128_t)randalg->random_seed;
        __uint128_t initseq   = (__uint128_t)randalg->mix_seed;

        *inc_ptr = (initseq << 1) | 1U;
        *state_ptr = 0U;

        *state_ptr = (*state_ptr * (__uint128_t)0x5851f42d4c957f2dULL) + *inc_ptr;
        *state_ptr += initstate;
        *state_ptr = (*state_ptr * (__uint128_t)0x5851f42d4c957f2dULL) + *inc_ptr;
    }

    __uint128_t oldstate = *state_ptr;

    *state_ptr = oldstate * (__uint128_t)0x5851f42d4c957f2dULL;
    *state_ptr += *inc_ptr;

    uint64_t xorshifted = (uint64_t)((oldstate ^ (oldstate >> 64)) >> 32);
    unsigned int rot = (unsigned int)(oldstate >> 122);
    uint64_t result = (xorshifted >> rot) | (xorshifted << ((-rot) & 63));

#else
    if (randalg->random_state[0] == 0 && randalg->random_state[1] == 0 &&
        randalg->random_state[2] == 0 && randalg->random_state[3] == 0) {

        uint64_t initseq = (uint64_t)randalg->mix_seed;
        randalg->random_state[2] = (initseq << 1) | 1ULL;
        randalg->random_state[3] = (initseq >> 63);

        randalg->random_state[0] = randalg->random_state[2];
        randalg->random_state[1] = randalg->random_state[3];

        uint64_t initstate = (uint64_t)randalg->random_seed;
        uint64_t prev_low = randalg->random_state[0];
        randalg->random_state[0] += initstate;
        if (randalg->random_state[0] < prev_low) {
            randalg->random_state[1]++;
        }

        uint64_t s_low = randalg->random_state[0];
        uint64_t s_high = randalg->random_state[1];
        uint64_t m_low = 0x5851f42d4c957f2dULL;

        uint64_t a32 = s_low >> 32, a00 = s_low & 0xFFFFFFFFFFFFFFULL;
        uint64_t u1 = (s_low & 0xFFFFFFFFULL) * (m_low & 0xFFFFFFFFULL);
        uint64_t w1 = u1 & 0xFFFFFFFFULL;
        uint64_t k = u1 >> 32;

        u1 = (s_low >> 32) * (m_low & 0xFFFFFFFFULL) + k;
        uint64_t w2 = u1 & 0xFFFFFFFFULL;
        uint64_t w3 = u1 >> 32;

        u1 = (s_low & 0xFFFFFFFFULL) * (m_low >> 32) + w2;
        k = u1 >> 32;

        uint64_t prod_low = (u1 << 32) | w1;
        uint64_t prod_high = (s_low >> 32) * (m_low >> 32) + w3 + k + (s_high * m_low);

        prod_low += randalg->random_state[2];
        if (prod_low < randalg->random_state[2]) prod_high++;
        prod_high += randalg->random_state[3];

        randalg->random_state[0] = prod_low;
        randalg->random_state[1] = prod_high;
    }

    uint64_t old_low = randalg->random_state[0];
    uint64_t old_high = randalg->random_state[1];

    uint64_t mult = 0x5851f42d4c957f2dULL;

    uint64_t x0 = old_low & 0xFFFFFFFFULL;
    uint64_t x1 = old_low >> 32;
    uint64_t y0 = mult & 0xFFFFFFFFULL;
    uint64_t y1 = mult >> 32;

    uint64_t p00 = x0 * y0;
    uint64_t p10 = x1 * y0;
    uint64_t p01 = x0 * y1;
    uint64_t p11 = x1 * y1;

    uint64_t middle = p10 + (p00 >> 32);
    uint64_t carry = middle < p10;
    middle += p01;
    if (middle < p01) carry++;

    uint64_t next_low = (middle << 32) | (p00 & 0xFFFFFFFFULL);
    uint64_t next_high = p11 + (middle >> 32) + (carry << 32) + (old_high * mult);

    next_low += randalg->random_state[2];
    if (next_low < randalg->random_state[2]) next_high++;
    next_high += randalg->random_state[3];

    randalg->random_state[0] = next_low;
    randalg->random_state[1] = next_high;

    uint64_t xorshifted = (old_low ^ old_high) >> 32;
    unsigned int rot = (unsigned int)(old_high >> 58);
    uint64_t result = (xorshifted >> rot) | (xorshifted << ((-rot) & 63));
#endif

    randalg->random_seed = (RANDOM_SEED)result;
}

void pcg32(randalg_t* randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;
    uint64_t state = s[0];
    uint64_t inc = s[1] | 1;

    s[0] = state * 6364136223846793005ULL + inc;
    uint32_t xorshifted = (uint32_t)(((state >> 18) ^ state) >> 27);
    uint32_t rot = (uint32_t)(state >> 59);
    uint32_t result = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));

    randalg->random_out = (RANDOM_OUT)result;
}

void pcg64(randalg_t* randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;
    s[0] = s[0] * 6364136223846793005ULL + (s[1] | 1);
    uint64_t xorshifted = ((s[0] >> 33) ^ s[0]) * 0xff51afd7ed558ccdULL;
    uint64_t result = (xorshifted >> 33) ^ xorshifted;

    randalg->random_out = (RANDOM_OUT)result;
}

void pcg_rxs_m_xs(randalg_t* randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;
    s[0] = s[0] * 6364136223846793005ULL + (s[1] | 1);
    uint64_t xorshifted = ((s[0] >> 22) ^ s[0]) >> 22;
    uint64_t result = xorshifted * 12605981442414356493ULL;

    randalg->random_out = (RANDOM_OUT)result;
}
