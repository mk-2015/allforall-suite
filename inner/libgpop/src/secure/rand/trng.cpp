#include <secure/rand/randalg.hpp>
using namespace gpop::Secure::Rand;
#include <stdint.h>
#include <secure/rand/random.hpp>


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

void trng(gpop::Secure::Rand::randalg_t* randalg) {
    if (!randalg) return;

    uint64_t crypto_val = 0;
    int success = 0;

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));

    if (ecx & (1U << 30)) {
        for (int retry = 0; retry < 10; retry++) {
            #if defined(__x86_64__) || defined(_M_X64)
            unsigned long long tmp;
            unsigned char ok;
            __asm__ volatile("rdrand %0; setc %1" : "=r"(tmp), "=qm"(ok));
            if (ok) { crypto_val = (uint64_t)tmp; success = 1; break; }
            #else
            unsigned int low, high;
            unsigned char s1 = 0, s2 = 0;
            __asm__ volatile("rdrand %0; setc %1" : "=r"(low), "=qm"(s1));
            __asm__ volatile("rdrand %0; setc %1" : "=r"(high), "=qm"(s2));
            if (s1 && s2) { crypto_val = ((uint64_t)high << 32) | low; success = 1; break; }
            #endif
        }
    }

#elif defined(__aarch64__) || defined(_M_ARM64)
    uint64_t val;
    unsigned char ok;
    __asm__ volatile("mrs %0, s3_3_c2_c4_0\n\t" "cset %w1, ne\n\t" : "=r"(val), "=r"(ok) :: "cc");
    if (ok) { crypto_val = val; success = 1; }

#elif defined(__riscv)
    uint64_t final_val = 0;
    int valid_chunks = 0;
    for (int i = 0; i < 4; i++) {
        unsigned long csr_val;
        __asm__ volatile("csrrw %0, 0x015, x0" : "=r"(csr_val) :: "memory");
        if (((csr_val >> 30) & 0x3) == 3) {
            final_val |= ((uint64_t)(csr_val & 0xFFFF)) << (i * 16);
            valid_chunks++;
        } else { break; }
    }
    if (valid_chunks == 4) { crypto_val = final_val; success = 1; }
#endif

    if (!success) {
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
    }

    if (success) {
        randalg->random_seed = (RANDOM_SEED)crypto_val;
    } else {
        randalg->random_seed ^= (randalg->random_seed >> 17);
        randalg->random_seed *= (randalg->random_seed << 3);
        randalg->random_seed -= (randalg->random_seed >> 7);
    }
}
