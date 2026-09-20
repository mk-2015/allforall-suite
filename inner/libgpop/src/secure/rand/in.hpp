#pragma once

#include <cstddef>
#include <secure/rand/random.hpp>

void lcgvalue(gpop::Secure::Rand::randalg_t *randalg);
void lcg_glibc(gpop::Secure::Rand::randalg_t *randalg);
void lcg_posix(gpop::Secure::Rand::randalg_t *randalg);
void xorshift(gpop::Secure::Rand::randalg_t *randalg);
void xoshiro256ss(gpop::Secure::Rand::randalg_t *randalg);
void xoshiro256p(gpop::Secure::Rand::randalg_t *randalg);
void xoroshiro128p(gpop::Secure::Rand::randalg_t *randalg);
void xoshiro128pp(gpop::Secure::Rand::randalg_t *randalg);
void splitmix64(gpop::Secure::Rand::randalg_t *randalg);
void chacha20(gpop::Secure::Rand::randalg_t *randalg);
void sfc64(gpop::Secure::Rand::randalg_t *randalg);
void well512a(gpop::Secure::Rand::randalg_t *randalg);
void hc128(gpop::Secure::Rand::randalg_t *randalg);

void mersennetwist(gpop::Secure::Rand::randalg_t *randalg);
void mt_seed(gpop::Secure::Rand::randalg_t *randalg);

void pcgr(gpop::Secure::Rand::randalg_t* randalg);
void pcg32(gpop::Secure::Rand::randalg_t* randalg);
void pcg64(gpop::Secure::Rand::randalg_t* randalg);
void pcg_rxs_m_xs(gpop::Secure::Rand::randalg_t* randalg);
void csprng(gpop::Secure::Rand::randalg_t* randalg);
void trng(gpop::Secure::Rand::randalg_t* randalg);
void logistic_map(gpop::Secure::Rand::randalg_t* randalg);
void philox_4x32(gpop::Secure::Rand::randalg_t* randalg);
void kiss(gpop::Secure::Rand::randalg_t* randalg);
void xoshiro256pp(gpop::Secure::Rand::randalg_t* randalg);

static inline void resetstate(gpop::Secure::Rand::randalg_t *rhandle) {
    uint64_t current = (uint64_t)rhandle->random_seed;

    for (int i = 0; i < 10; i++) {
        current = (current * 6364136223846793005ULL) + 1442695040888963407ULL;
        rhandle->random_state[i] = current;
    }
}
