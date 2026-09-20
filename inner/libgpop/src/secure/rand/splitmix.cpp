#include <secure/rand/randalg.hpp>
using namespace gpop::Secure::Rand;
#include <secure/rand/random.hpp>
#include <stdint.h>
#include "in.hpp"

void splitmix64(gpop::Secure::Rand::randalg_t *randalg) {
    if (!randalg) return;

    uint64_t *s = (uint64_t *)randalg->random_state;

    if (s[0] == 0) {
        s[0] = (uint64_t)randalg->random_seed;
        if (s[0] == 0) {
            s[0] = 0x9E3779B97F4A7C15ULL;
        }
    }

    uint64_t z = (s[0] += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    z = z ^ (z >> 31);

    randalg->random_seed = (RANDOM_SEED)z;
}
