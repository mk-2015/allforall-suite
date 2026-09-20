#include <secure/rand/randalg.hpp>
using namespace gpop::Secure::Rand;
#include <secure/rand/random.hpp>
#include <stdlib.h>
#include <stdint.h>
#include "in.hpp"

static inline uint64_t rotl64(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static inline uint32_t rotl32(const uint32_t x, int k) {
    return (x << k) | (x >> (32 - k));
}

void xorshift(gpop::Secure::Rand::randalg_t *randalg) {
    if (!randalg) return;

    uint64_t *s = (uint64_t *)randalg->random_state;

    if (s[0] == 0) {
        s[0] = (uint64_t)randalg->random_seed;
        if (s[0] == 0) s[0] = 0x1234567890ABCDEFULL;
    }

    uint64_t x = s[0];
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    s[0] = x;

    randalg->random_seed = (RANDOM_SEED)x;
}

void xoshiro256ss(gpop::Secure::Rand::randalg_t *randalg) {
    if (!randalg) return;

    uint64_t *s = (uint64_t *)randalg->random_state;

    if ((s[0] | s[1] | s[2] | s[3]) == 0) {
        s[0] = (uint64_t)randalg->random_seed;
        s[1] = (uint64_t)(uint32_t)randalg->mix_seed;
        s[2] = s[0] ^ 0x9E3779B97F4A7C15ULL;
        s[3] = s[1] ^ 0x85EBCA6BCA1AF285ULL;
        if ((s[0] | s[1] | s[2] | s[3]) == 0) s[0] = 1;
    }

    const uint64_t result = rotl64(s[1] * 5, 7) * 9;
    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rotl64(s[3], 45);

    randalg->random_seed = (RANDOM_SEED)result;
}

void xoroshiro128p(gpop::Secure::Rand::randalg_t *randalg) {
    if (!randalg) return;

    uint64_t *s = (uint64_t *)randalg->random_state;

    if ((s[0] | s[1]) == 0) {
        s[0] = (uint64_t)randalg->random_seed;
        s[1] = (uint64_t)(uint32_t)randalg->mix_seed;
        if ((s[0] | s[1]) == 0) s[0] = 1;
    }

    const uint64_t s0 = s[0];
    uint64_t s1 = s[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    s[0] = rotl64(s0, 24) ^ s1 ^ (s1 << 16);
    s[1] = rotl64(s1, 37);

    randalg->random_seed = (RANDOM_SEED)result;
}

void xoshiro128pp(gpop::Secure::Rand::randalg_t *randalg) {
    if (!randalg) return;

    uint32_t *s = (uint32_t *)randalg->random_state;

    if ((s[0] | s[1] | s[2] | s[3]) == 0) {
        s[0] = (uint32_t)(randalg->random_seed & 0xFFFFFFFF);
        s[1] = (uint32_t)(((uint64_t)randalg->random_seed >> 32) & 0xFFFFFFFF);
        s[2] = (uint32_t)randalg->mix_seed;
        // Cast mix_seed to uint64_t before shifting to eliminate compiler warnings
        s[3] = (uint32_t)(((uint64_t)randalg->mix_seed >> 32) & 0xFFFFFFFF);
        if ((s[0] | s[1] | s[2] | s[3]) == 0) s[0] = 1;
    }

    const uint32_t result32 = rotl32(s[0] + s[3], 7) + s[0];
    const uint32_t t = s[1] << 9;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rotl32(s[3], 11);

    randalg->random_seed = (RANDOM_SEED)result32;
}

void xoshiro256p(gpop::Secure::Rand::randalg_t *randalg) {
    if (!randalg) return;

    uint64_t *s = (uint64_t *)randalg->random_state;

    if ((s[0] | s[1] | s[2] | s[3]) == 0) {
        s[0] = (uint64_t)randalg->random_seed;
        s[1] = (uint64_t)(uint32_t)randalg->mix_seed;
        s[2] = s[0] ^ 0x9E3779B97F4A7C15ULL;
        s[3] = s[1] ^ 0x85EBCA6BCA1AF285ULL;
        if ((s[0] | s[1] | s[2] | s[3]) == 0) s[0] = 1;
    }

    const uint64_t result = s[0] + s[3];
    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rotl64(s[3], 45);

    randalg->random_seed = (RANDOM_SEED)result;
}

void xoshiro256pp(gpop::Secure::Rand::randalg_t *randalg) {
    if (!randalg) return;

    uint64_t *s = (uint64_t *)randalg->random_state;

    if ((s[0] | s[1] | s[2] | s[3]) == 0) {
        s[0] = (uint64_t)randalg->random_seed;
        s[1] = (uint64_t)(uint32_t)randalg->mix_seed;
        s[2] = s[0] ^ 0x9E3779B97F4A7C15ULL;
        s[3] = s[1] ^ 0x85EBCA6BCA1AF285ULL;
        if ((s[0] | s[1] | s[2] | s[3]) == 0) s[0] = 1;
    }

    const uint64_t result = rotl64(s[0] + s[3], 23) + s[0];
    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rotl64(s[3], 45);

    randalg->random_seed = (RANDOM_SEED)result;
}
