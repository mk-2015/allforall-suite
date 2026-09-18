#include <rand/random.h>
#include <rand/randalg.h>
#include <stdint.h>
#include "in.h"

#define PHILOX_M4x32_0 0xD2511F53U
#define PHILOX_M4x32_1 0xCD9E8D57U
#define PHILOX_W32_0   0x9E3779B9U
#define PHILOX_W32_1   0xBB67AE85U

static inline void philox_4x32_round(uint32_t ctr[4], uint32_t key[2]) {
    uint64_t prod0 = (uint64_t)ctr[0] * PHILOX_M4x32_0;
    uint64_t prod1 = (uint64_t)ctr[2] * PHILOX_M4x32_1;

    ctr[0] = (uint32_t)(prod1 >> 32) ^ ctr[1] ^ key[0];
    ctr[1] = (uint32_t)prod1;
    ctr[2] = (uint32_t)(prod0 >> 32) ^ ctr[3] ^ key[1];
    ctr[3] = (uint32_t)prod0;
}

void philox_4x32(randalg_t *randalg) {
    if (!randalg) return;

    uint32_t *state = (uint32_t*)randalg->random_state;
    uint32_t *ctr = state;
    uint32_t *key = state + 4;

    if (ctr[0] == 0 && ctr[1] == 0 && ctr[2] == 0 && ctr[3] == 0 && key[0] == 0 && key[1] == 0) {
        key[0] = (uint32_t)(randalg->random_seed & 0xFFFFFFFF);
        key[1] = (uint32_t)((randalg->random_seed >> 32) & 0xFFFFFFFF);
        ctr[0] = (uint32_t)(randalg->mix_seed & 0xFFFFFFFF);
        ctr[1] = (uint32_t)(((uint64_t)randalg->mix_seed >> 32) & 0xFFFFFFFF);
        ctr[2] = 0x12345678U;
        ctr[3] = 0x9ABCDEFU;
    }

    uint32_t out[4] = { ctr[0], ctr[1], ctr[2], ctr[3] };
    uint32_t k[2]   = { key[0], key[1] };

    for (int i = 0; i < 10; i++) {
        philox_4x32_round(out, k);
        k[0] += PHILOX_W32_0;
        k[1] += PHILOX_W32_1;
    }

    ctr[0]++;
    if (ctr[0] == 0) {
        ctr[1]++;
        if (ctr[1] == 0) {
            ctr[2]++;
            if (ctr[2] == 0) {
                ctr[3]++;
            }
        }
    }

    randalg->random_seed = (RANDOM_SEED)(((uint64_t)out[0] << 32) | out[1]);
}
