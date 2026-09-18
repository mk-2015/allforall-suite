#include <rand/random.h>
#include <rand/randalg.h>
#include <stdlib.h>
#include <string.h>
#include "in.h"

void lcgvalue(randalg_t *randalg)
{
    randalg->random_seed = (randalg->random_seed * 6364136223846793005ULL) + 1442695040888963407ULL;
}

void lcg_glibc(randalg_t *randalg)
{
    randalg->random_seed = (randalg->random_seed * 1103515245ULL + 12345ULL) & 0x7FFFFFFF;
}

void lcg_posix(randalg_t *randalg)
{
    randalg->random_seed = (randalg->random_seed * 25214903917ULL + 11ULL) & ((1ULL << 48) - 1);
}
