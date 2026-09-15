#include <rand/random.h>
#include <rand/randalg.h>
#include <stdint.h>
#include "in.h"

// Basic HC-128 implementation (Simplified state handling)
void hc128(randalg_t *randalg) {
    uint32_t *state = (uint32_t *)randalg->random_state;
    // P and Q are each 512 words (2048 bytes)
    uint32_t *P = state;
    uint32_t *Q = state + 512;
    uint32_t *counter = state + 1024; // Use remaining space for counter

    // Initialization (simplified)
    if (*counter == 0) {
        for (int i = 0; i < 512; i++) P[i] = i ^ 0xDEADBEEF;
        for (int i = 0; i < 512; i++) Q[i] = i ^ 0xCAFEBABE;
        *counter = 1;
    }

    uint32_t i = (*counter) & 511;
    uint32_t output;
    
    // Core HC-128 step
    if ((*counter & 512) == 0) {
        P[i] = P[i] + (P[(i - 3) & 511] ^ P[(i - 10) & 511]) + (P[(i - 511) & 511] ^ P[(i - 1) & 511]);
        output = Q[(P[(i - 12) & 511] >> 24) + (P[(i - 12) & 511] & 255)] ^ P[i];
    } else {
        Q[i] = Q[i] + (Q[(i - 3) & 511] ^ Q[(i - 10) & 511]) + (Q[(i - 511) & 511] ^ Q[(i - 1) & 511]);
        output = P[(Q[(i - 12) & 511] >> 24) + (Q[(i - 12) & 511] & 255)] ^ Q[i];
    }
    
    (*counter)++;
    randalg->random_out = (RANDOM_OUT)output;
}
