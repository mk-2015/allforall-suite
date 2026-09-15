#include <rand/random.h>
#include <rand/randalg.h>
#include <stdint.h>
#include "in.h"

void splitmix64(randalg_t *randalg) {
    uint64_t *s = (uint64_t *)randalg->random_state;

    if (s[0] == 0) {
        s[0] = (uint64_t)randalg->random_seed;
    }

    uint64_t x = s[0] += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    randalg->random_out = (RANDOM_OUT)(x ^ (x >> 31));
}
