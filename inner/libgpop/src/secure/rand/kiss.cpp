#include <secure/rand/randalg.hpp>
using namespace gpop::Secure::Rand;
#include <secure/rand/random.hpp>
#include <stdint.h>
#include "in.hpp"

void kiss(gpop::Secure::Rand::randalg_t *randalg) {
    uint32_t *s = (uint32_t*)randalg->random_state;

    if (s[0] == 0 && s[1] == 0 && s[2] == 0 && s[3] == 0) {
        s[0] = (uint32_t)(randalg->random_seed & 0xFFFFFFFF);
        s[1] = (uint32_t)((randalg->random_seed >> 32) & 0xFFFFFFFF);
        s[2] = (uint32_t)(randalg->mix_seed & 0xFFFFFFFF);
        s[3] = (uint32_t)(((uint64_t)randalg->mix_seed >> 32) & 0xFFFFFFFF);

        if (s[0] == 0) s[0] = 123456789;
        if (s[1] == 0) s[1] = 362436000;
        if (s[2] == 0) s[2] = 521288629;
        if (s[3] == 0) s[3] = 88675123;
    }

    uint64_t t = 698769069ULL * s[0] + s[1];
    s[1] = (uint32_t)(t >> 32);
    s[0] = (uint32_t)t;

    s[2] ^= s[2] << 13;
    s[2] ^= s[2] >> 17;
    s[2] ^= s[2] << 5;

    s[3] = 69069U * s[3] + 1234567U;

    uint32_t kiss32 = s[0] + s[2] + s[3];
    randalg->random_seed = (RANDOM_SEED)kiss32 | ((RANDOM_SEED)(s[1] ^ s[2]) << 32);
}
