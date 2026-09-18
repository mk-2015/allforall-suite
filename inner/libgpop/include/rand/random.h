#pragma once
#include <pthread.h>

#define STATE_SIZE 512
typedef int ALGORITHM;
typedef long long RANDOM_SEED;
typedef unsigned long long RANDOM_OUT;
typedef int MIX;
typedef int MIX_SEED;
typedef unsigned long long *RANDOM_STATE;

extern int pthread_en;

typedef struct randalg_t {
    ALGORITHM algorithm;
    RANDOM_OUT random_out;
    RANDOM_SEED random_seed;
    RANDOM_STATE random_state;

    MIX isMixOn;
    MIX_SEED mix_seed;
    pthread_mutex_t lock;
} randalg_t;

randalg_t* open_random(ALGORITHM algorithm, RANDOM_SEED seed, MIX isMixOn, MIX_SEED mix_seed);
void genrand(randalg_t *randalg);
int lasterr();
RANDOM_OUT getrand(randalg_t *randalg);
void reset(randalg_t *randalg);
void setopt(ALGORITHM algorithm, MIX isMixOn, MIX_SEED mix_seed, RANDOM_SEED seed, randalg_t *randalg);
int thread(int th);
void close_random(randalg_t *randalg);
