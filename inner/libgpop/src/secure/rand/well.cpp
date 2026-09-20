#include <secure/rand/randalg.hpp>
using namespace gpop::Secure::Rand;
#include <secure/rand/random.hpp>
#include <secure/rand/randalg.hpp>
#include <stdint.h>
#include "in.hpp"

#define MAT0POS(t, v) (v ^ (v >> t))
#define MAT0NEG(t, v) (v ^ (v << t))
#define MAT3POS(t, v) (v >> t)
#define MAT4POS(t, b, v) (v ^ ((v >> t) & b))

void well512a(gpop::Secure::Rand::randalg_t *randalg) {
    if (!randalg) return;

    uint32_t *s = (uint32_t *)randalg->random_state;

    // Initialize state using random_seed and mix_seed if state is unseeded
    if (s[0] == 0 && s[1] == 0 && s[2] == 0 && s[15] == 0) {
        uint64_t seed = (uint64_t)randalg->random_seed;
        uint64_t mix = (uint64_t)randalg->mix_seed;

        if (seed == 0) seed = 0x85EBCA6BU;

        for (int i = 0; i < 16; i++) {
            seed = seed * 6364136223846793005ULL + mix + i;
            s[i] = (uint32_t)(seed ^ (seed >> 32));
        }
        s[16] = 0; // Index offset
    }

    uint32_t idx = s[16] & 15;

    uint32_t z0 = s[(idx + 15) & 15];
    uint32_t z1 = MAT0POS(8, s[idx]) ^ MAT0POS(19, s[(idx + 13) & 15]);
    uint32_t z2 = MAT0POS(11, s[(idx + 9) & 15]);

    s[idx] = z1 ^ z2;

    s[(idx + 15) & 15] = MAT0NEG(2, z0) ^
                         MAT0NEG(18, z1) ^
                         MAT3POS(28, z2) ^
                         MAT4POS(5, 0xda442d24U, s[idx]);

    idx = (idx + 15) & 15;
    s[16] = idx;

    randalg->random_seed = (RANDOM_SEED)s[idx] | ((RANDOM_SEED)s[(idx + 1) & 15] << 32);
}
