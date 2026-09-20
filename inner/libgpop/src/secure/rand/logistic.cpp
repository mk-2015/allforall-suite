#include <secure/rand/randalg.hpp>
using namespace gpop::Secure::Rand;
#include <secure/rand/random.hpp>
#include <stdint.h>
#include "in.hpp"

void logistic_map(gpop::Secure::Rand::randalg_t *randalg) {
    if (randalg->random_seed == 0) {
        randalg->random_seed = 0x123456789ABCDEF0ULL;
    }

    double x_norm = (double)randalg->random_seed / 18446744073709551616.0;

    if (x_norm <= 0.0 || x_norm >= 1.0) {
        x_norm = 0.5;
    }

    x_norm = 3.99999 * x_norm * (1.0 - x_norm);

    randalg->random_seed = (RANDOM_SEED)(x_norm * 18446744073709551616.0);
}
