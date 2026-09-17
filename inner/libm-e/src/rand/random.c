#include <rand/random.h>
#include <rand/randalg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "in.h"

static _Thread_local int error = 0;
int pthread_en = 0;

int thread(int th)
{
    if (th == -1) {
        return pthread_en;
    } else if (th == 1) {
        pthread_en = 1;
    } else {
        pthread_en = 0;
    }
    return 0;
}

randalg_t* open_random(ALGORITHM algorithm,
        RANDOM_SEED seed,
        MIX isMixOn, MIX_SEED mix_seed)
{
    randalg_t* rhandle = malloc(sizeof(randalg_t));
    if (!rhandle) {
        error = OPEN_RAND_FAILMALLOC;
        return NULL;
    }

    rhandle->random_state = malloc(STATE_SIZE * sizeof(unsigned long long));
    if (!rhandle->random_state) {
        free(rhandle);
        error = OPEN_RAND_FAILMALLOC;
        return NULL;
    }

    if (pthread_mutex_init(&rhandle->lock, NULL) != 0) {
        free(rhandle->random_state);
        free(rhandle);
        error = OPEN_RAND_FAILMUTEX;
        return NULL;
    }

    rhandle->algorithm = algorithm;
    rhandle->random_seed = seed;
    rhandle->isMixOn = isMixOn;
    rhandle->mix_seed = mix_seed;
    rhandle->random_out = 0;

    for (int i = 0; i < STATE_SIZE; i++) {
        rhandle->random_state[i] = (i * 6364136223846793005ULL) + 1442695040888963407ULL;
    }

    if (algorithm == RANDALG_MERSENNE_TWISTER) {
        mt_seed(rhandle);
    }

    return rhandle;
}

void genrand(randalg_t *randalg)
{
    if (!randalg) {
        error = GENRAND_FAILINVLAIDSTRUCT;
        return;
    }

    if (pthread_en) pthread_mutex_lock(&randalg->lock);

    switch(randalg->algorithm) {
        case RANDALG_LCG:            lcgvalue(randalg); break;
        case RANDALG_LCG_GLIBC:      lcg_glibc(randalg); break;
        case RANDALG_LCG_POSIX:      lcg_posix(randalg); break;
        case RANDALG_XORSHIFT:       xorshift(randalg); break;
        case RANDALG_XOSHIRO256SS:   xoshiro256ss(randalg); break;
        case RANDALG_XOSHIRO256P:    xoshiro256p(randalg); break;
        case RANDALG_SPLITMIX64:     splitmix64(randalg); break;
        case RANDALG_CHACHA20:       chacha20(randalg); break;
        case RANDALG_SFC64:          sfc64(randalg); break;
        case RANDALG_WELL512A:       well512a(randalg); break;
        case RANDALG_HC128:          hc128(randalg); break;
        case RANDALG_XOROSHIRO128P:  xoroshiro128p(randalg); break;
        case RANDALG_XOSHIRO128PP:   xoshiro128pp(randalg); break;
        case RANDALG_MERSENNE_TWISTER: mersennetwist(randalg); break;
        case RANDALG_PCGR:           pcgr(randalg); break;
        case RANDALG_PCG32:          pcg32(randalg); break;
        case RANDALG_PCG64:          pcg64(randalg); break;
        case RANDALG_PCG_RXS_M_XS:   pcg_rxs_m_xs(randalg); break;
        case RANDALG_CSPRNG:         csprng(randalg); break;
        case RANDALG_TRNG:           trng(randalg); break;
        case RANDALG_LOGISTIC_MAP:   logistic_map(randalg); break;
        case RANDALG_PHILOX_4X32:    philox_4x32(randalg); break;
        case RANDALG_KISS:           kiss(randalg); break;
        case RANDALG_XOSHIRO256PP:   xoshiro256pp(randalg); break;
        default:
            if (pthread_en) pthread_mutex_unlock(&randalg->lock);
            error = GENRAND_FAILINVLAIDALGORITHM;
            return;
    }

    if (randalg->isMixOn) {
        randalg->random_seed ^= randalg->mix_seed;
        randalg->mix_seed = (randalg->mix_seed * 6364136223846793005ULL) + 1442695040888963407ULL;
    }

    randalg->random_out = randalg->random_seed;

    if (pthread_en) pthread_mutex_unlock(&randalg->lock);
}

RANDOM_OUT getrand(randalg_t *randalg)
{
    if (!randalg) {
        error = GETRAND_FAILINVLAIDSTRUCT;
        return 0;
    }

    if (pthread_en) pthread_mutex_lock(&randalg->lock);
    RANDOM_OUT val = randalg->random_out;
    if (pthread_en) pthread_mutex_unlock(&randalg->lock);
    return val;
}

void setopt(ALGORITHM algorithm,
    MIX isMixOn,
    MIX_SEED mix_seed,
    RANDOM_SEED seed,
    randalg_t *randalg)
{
    if (!randalg) {
        error = SETOPT_FAILINVLAIDSTRUCT;
        return;
    }
    if (pthread_en) pthread_mutex_lock(&randalg->lock);
    randalg->algorithm = algorithm;
    randalg->isMixOn = isMixOn;
    if (isMixOn && mix_seed != -1) {
        randalg->random_seed ^= mix_seed;
    }
    if (seed != -1) {
        randalg->random_seed = seed;
    }
    if (algorithm == RANDALG_MERSENNE_TWISTER) {
        mt_seed(randalg);
    }
    if (pthread_en) pthread_mutex_unlock(&randalg->lock);
}

void reset(randalg_t *randalg)
{
    if (!randalg) {
        error = RESET_FAILINVLAIDSTRUCT;
        return;
    }
    if (pthread_en) pthread_mutex_lock(&randalg->lock);
    randalg->random_seed = 0;
    randalg->random_out = 0;
    randalg->mix_seed = 0;
    randalg->algorithm = 0;
    randalg->isMixOn = 0;
    for (int i = 0; i < STATE_SIZE; i++) {
        randalg->random_state[i] = (i * 6364136223846793005ULL) + 1442695040888963407ULL;
    }
    if (pthread_en) pthread_mutex_unlock(&randalg->lock);
}

void close_random(randalg_t *randalg)
{
    if (!randalg) {
        error = CLOSERAND_FAILINVLAIDSTRUCT;
        return;
    }
    pthread_mutex_destroy(&randalg->lock);
    if (randalg->random_state) free(randalg->random_state);
    free(randalg);
}

int lasterr()
{
    int err = error;
    error = 0;
    return err;
}
