#include <rand/random.h>
#include <rand/randalg.h>
#include <stdint.h>
#include "in.h"

void hc128(randalg_t *randalg) {
    uint32_t *state = (uint32_t *)randalg->random_state;
    uint32_t *P = state;
    uint32_t *Q = state + 512;
    uint32_t *counter = state + 1024;

    if (*counter == 0) {
        uint32_t key_low = (uint32_t)(randalg->random_seed & 0xFFFFFFFF);
        uint32_t key_high = (uint32_t)((randalg->random_seed >> 32) & 0xFFFFFFFF);
        uint32_t mix_low = (uint32_t)(randalg->mix_seed & 0xFFFFFFFF);
        uint32_t mix_high = (uint32_t)(((uint64_t)randalg->mix_seed >> 32) & 0xFFFFFFFF);

        for (int i = 0; i < 512; i++) {
            P[i] = key_low ^ (i * 0x9E3779B9U) ^ mix_high;
            Q[i] = key_high ^ (i * 0x85EBCA6BU) ^ mix_low;
        }
        *counter = 1;
    }

    uint32_t i = (*counter - 1) & 511;
    uint32_t step = (*counter - 1) & 1023;
    uint32_t output;

    if (step < 512) {
        uint32_t i3 = (i - 3) & 511;
        uint32_t i10 = (i - 10) & 511;
        uint32_t i511 = (i - 511) & 511;
        uint32_t i12 = (i - 12) & 511;

        P[i] += (P[i3] ^ P[i10]) + (P[i511] ^ P[(i - 1) & 511]);
        uint32_t index = (P[i12] & 0xFF) + ((P[i12] >> 24) & 0xFF);
        output = Q[index & 511] ^ P[i];
    } else {
        uint32_t i3 = (i - 3) & 511;
        uint32_t i10 = (i - 10) & 511;
        uint32_t i511 = (i - 511) & 511;
        uint32_t i12 = (i - 12) & 511;

        Q[i] += (Q[i3] ^ Q[i10]) + (Q[i511] ^ Q[(i - 1) & 511]);
        uint32_t index = (Q[i12] & 0xFF) + ((Q[i12] >> 24) & 0xFF);
        output = P[index & 511] ^ Q[i];
    }

    (*counter)++;
    if (*counter == 0) *counter = 1;

    randalg->random_seed = (RANDOM_SEED)output | ((RANDOM_SEED)P[i] << 32);
}
