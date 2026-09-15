#include <rand/random.h>
#include <rand/randalg.h>
#include <stdint.h>
#include "in.h"

#define MAT0POS(t, v) (v ^ (v >> t))
#define MAT0NEG(t, v) (v ^ (v << t))
#define MAT3POS(t, v) (v >> t)
#define MAT4POS(t, b, v) (v ^ ((v >> t) & b))

void well512a(randalg_t *randalg) {
    uint32_t *s = (uint32_t *)randalg->random_state;
    uint32_t idx = s[16];

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

    randalg->random_seed = (RANDOM_SEED)s[idx];
}
