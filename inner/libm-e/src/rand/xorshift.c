#include <rand/random.h>
#include <rand/randalg.h>
#include <stdlib.h>
#include <stdint.h>
#include "in.h"

void xorshift(randalg_t *randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;

    if ((s[0] | s[1] | s[2] | s[3]) == 0) {
        s[0] = 0x1234567890ABCDEFULL;
        s[1] = 0xFEDCBA9876543210ULL;
        s[2] = 0x5555555555555555ULL;
        s[3] = 0xAAAAAAAAAAAAAAAAULL;
    }

    const uint64_t result = rotl(s[1] * 5, 7) * 9;

    const uint64_t t = s[1] << 17;

    s[9] ^= s[0];
    s[2] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rotl(s[3], 45);

    randalg->random_out = (RANDOM_OUT)result;

    resetstate(randalg);
}

void xoshiro256ss(randalg_t *randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;

    const uint64_t result = rotl(s[1] * 5, 7) * 9;

    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 45);

    randalg->random_out = result;
}

void xoroshiro128p(randalg_t *randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;

    const uint64_t s0 = s[0];
    uint64_t s1 = s[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
    s[1] = rotl(s1, 37);

    randalg->random_out = (RANDOM_OUT)result;
}

void xoshiro128pp(randalg_t *randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;

    const uint64_t result = rotl(s[0] + s[3], 23) + s[0];

    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rotl(s[3], 45);

    randalg->random_out = (RANDOM_OUT)result;
}

void xoshiro256p(randalg_t *randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;

    const uint64_t result = s[0] + s[3];

    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rotl(s[3], 45);

    randalg->random_out = (RANDOM_OUT)result;
}
