#include <stdint.h>
#include <rand/random.h>
#include <rand/randalg.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <bcrypt.h>
    #pragma comment(lib, "bcrypt.lib")
#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #if defined(__linux__)
        #include <sys/random.h>
    #endif
#endif

void csprng(randalg_t* randalg) {
    if (!randalg) return;

    uint64_t crypto_val = 0;
    int success = 0;

#if defined(_WIN32) || defined(_WIN64)
    if (BCRYPT_SUCCESS(BCryptGenRandom(NULL, (PUCHAR)&crypto_val, sizeof(crypto_val), BCRYPT_USE_SYSTEM_PREFERRED_RNG))) {
        success = 1;
    }
#else
    #if defined(__linux__)
    long bytes = getrandom(&crypto_val, sizeof(crypto_val), 0);
    if (bytes == sizeof(crypto_val)) {
        success = 1;
    }
    #endif

    if (!success) {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd >= 0) {
            if (read(fd, &crypto_val, sizeof(crypto_val)) == sizeof(crypto_val)) {
                success = 1;
            }
            close(fd);
        }
    }
#endif

    if (success) {
        randalg->random_seed = (RANDOM_SEED)crypto_val;
    } else {
        uint64_t hash = (uint64_t)randalg->random_seed ^ 0x9E3779B97F4A7C15ULL;
        hash = (hash ^ (hash >> 30)) * 0xBF58476D1CE4E5B9ULL;
        hash = (hash ^ (hash >> 27)) * 0x94D049BB133111EBULL;
        crypto_val = hash ^ (hash >> 31);

        randalg->random_seed = (RANDOM_SEED)crypto_val;
    }
}
