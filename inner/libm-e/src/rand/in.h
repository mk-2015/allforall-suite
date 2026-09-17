#pragma once

void lcgvalue(randalg_t *randalg);
void lcg_glibc(randalg_t *randalg);
void lcg_posix(randalg_t *randalg);
void xorshift(randalg_t *randalg);
void xoshiro256ss(randalg_t *randalg);
void xoshiro256p(randalg_t *randalg);
void xoroshiro128p(randalg_t *randalg);
void xoshiro128pp(randalg_t *randalg);
void splitmix64(randalg_t *randalg);
void chacha20(randalg_t *randalg);
void sfc64(randalg_t *randalg);
void well512a(randalg_t *randalg);
void hc128(randalg_t *randalg);

void mersennetwist(randalg_t *randalg);
void mt_seed(randalg_t *randalg);

void pcgr(randalg_t* randalg);
void pcg32(randalg_t* randalg);
void pcg64(randalg_t* randalg);
void pcg_rxs_m_xs(randalg_t* randalg);
void csprng(randalg_t* randalg);
void trng(randalg_t* randalg);
void logistic_map(randalg_t* randalg);
void philox_4x32(randalg_t* randalg);
void kiss(randalg_t* randalg);
void xoshiro256pp(randalg_t* randalg);

static inline void resetstate(randalg_t *rhandle) {
    uint64_t current = (uint64_t)rhandle->random_seed;

    for (int i = 0; i < 10; i++) {
        current = (current * 6364136223846793005ULL) + 1442695040888963407ULL;
        rhandle->random_state[i] = current;
    }
}
