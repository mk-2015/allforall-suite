#include <rand/random.h>
#include <rand/randalg.h>
#include <stdint.h>
#include "in.h"

// SFC64 state: 4 * uint64_t
void sfc64(randalg_t *randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;
    // Initialization (simplified)
    if (s[0] == 0 && s[1] == 0 && s[2] == 0) {
        s[0] = (uint64_t)randalg->random_seed;
        s[1] = (uint64_t)randalg->mix_seed;
        s[2] = 1;
        s[3] = 1;
    }
    
    uint64_t tmp = s[0] + s[1] + s[3]++;
    s[0] = s[1] ^ (s[1] >> 11);
    s[1] = s[2] + (s[2] << 3);
    s[2] = (s[2] << 24) | (s[2] >> 40);
    s[2] += tmp;
    
    randalg->random_out = (RANDOM_OUT)tmp;
}
