#include <rand/random.h>
#include <rand/randalg.h>
#include <stdint.h>
#include <string.h>
#include "in.h"

#define ROTL(v, n) (((v) << (n)) | ((v) >> (32 - (n))))
#define QR(a, b, c, d) \
    a += b; d ^= a; d = ROTL(d, 16); \
    c += d; b ^= c; b = ROTL(b, 12); \
    a += b; d ^= a; d = ROTL(d, 8); \
    c += d; b ^= c; b = ROTL(b, 7)

void chacha20(randalg_t *randalg) {
    uint32_t *state = (uint32_t *)randalg->random_state;

    if (state[0] != 0x61707865) {
        state[0] = 0x61707865;
        state[1] = 0x3320646e;
        state[2] = 0x79622d32;
        state[3] = 0x6b206574;

        state[4] = (uint32_t)(randalg->random_seed & 0xFFFFFFFF);
        state[5] = (uint32_t)(((uint64_t)randalg->random_seed >> 32) & 0xFFFFFFFF);
        state[6] = (uint32_t)randalg->mix_seed;
        state[7] = (uint32_t)(((uint64_t)randalg->mix_seed >> 32) & 0xFFFFFFFF);

        state[8]  = 0x9E3779B9;
        state[9]  = 0x7F4A7C15;
        state[10] = 0x85EBCA6B;
        state[11] = 0xC2B2AE35;

        state[12] = 0;
        state[13] = 0;
        state[14] = 0x12345678;
        state[15] = 0x9ABCDEF0;
    }

    uint32_t working_state[16];
    memcpy(working_state, state, sizeof(working_state));

    for (int i = 0; i < 10; i++) {
        QR(working_state[0], working_state[4], working_state[8],  working_state[12]);
        QR(working_state[1], working_state[5], working_state[9],  working_state[13]);
        QR(working_state[2], working_state[6], working_state[10], working_state[14]);
        QR(working_state[3], working_state[7], working_state[11], working_state[15]);

        QR(working_state[0], working_state[5], working_state[10], working_state[15]);
        QR(working_state[1], working_state[6], working_state[11], working_state[12]);
        QR(working_state[2], working_state[7], working_state[8],  working_state[13]);
        QR(working_state[3], working_state[4], working_state[9],  working_state[14]);
    }

    for (int i = 0; i < 16; i++) {
        working_state[i] += state[i];
    }

    state[12]++;
    if (state[12] == 0) {
        state[13]++;
    }

    randalg->random_seed = (RANDOM_SEED)working_state[0] | ((RANDOM_OUT)working_state[1] << 32);
}
