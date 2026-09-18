#include <rand/random.h>
#include <rand/randalg.h>
#include <stdint.h>
#include "in.h"

// Standard SFC64 implementation (Small Fast Counting PRNG by Chris Doty-Humphrey)
void sfc64(randalg_t *randalg) {
    if (!randalg) return;

    uint64_t *s = (uint64_t *)randalg->random_state;

    // Initialize state if empty
    if (s[0] == 0 && s[1] == 0 && s[2] == 0 && s[3] == 0) {
        s[0] = (uint64_t)randalg->random_seed;
        s[1] = (uint64_t)randalg->mix_seed;
        s[2] = (uint64_t)randalg->random_seed ^ (uint64_t)randalg->mix_seed;
        s[3] = 1;

        // Warm up the state to fully mix seeds
        for (int i = 0; i < 12; i++) {
            uint64_t tmp = s[0] + s[1] + s[3]++;
            s[0] = s[1] ^ (s[1] >> 11);
            s[1] = s[2] + (s[2] << 3);
            s[2] = ((s[2] << 24) | (s[2] >> 40)) + tmp;
        }
    }

    uint64_t tmp = s[0] + s[1] + s[3]++;
    s[0] = s[1] ^ (s[1] >> 11);
    s[1] = s[2] + (s[2] << 3);
    s[2] = ((s[2] << 24) | (s[2] >> 40)) + tmp;

    randalg->random_seed = (RANDOM_SEED)tmp;
}
