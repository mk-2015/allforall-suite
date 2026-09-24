#include <secure/kdf/kdf.hpp>
#include <random>
#include <algorithm>
#include <secure/ngc.hpp>
#include "int.hpp"

#ifdef HAVE_SODIUM
#include <sodium.h>
#endif

void kdf_argon2(
    uint64_t argon2_time,
    size_t argon2_mem,
    gpop::Secure::Salt& salt,
    gpop::Secure::Pepper& pepper,
    gpop::Secure::HashT& outhash,
    gpop::Secure::Data& indata
) {
    gpop::Secure::Data combined;
    combined.reserve(indata.size() + pepper.size());

    combined.insert(combined.end(), indata.begin(), indata.end());
    if (!pepper.empty()) {
        combined.insert(combined.end(), pepper.begin(), pepper.end());
    }

    outhash.resize(32);

#ifdef HAVE_SODIUM
    if (sodium_init() < 0) {
        volatile uint8_t* fail_ptr = combined.data();
        size_t fail_size = combined.size();
        while (fail_size--) *fail_ptr++ = 0;
        combined.clear();
        outhash.clear();
        return;
    }

    if (salt.size() < crypto_pwhash_SALTBYTES) {
        salt.resize(crypto_pwhash_SALTBYTES);
    }

    int result = crypto_pwhash(
        outhash.data(), outhash.size(),
        reinterpret_cast<const char*>(combined.data()), combined.size(),
        salt.data(),
        static_cast<unsigned long long>(argon2_time),
        argon2_mem,
        crypto_pwhash_ALG_DEFAULT
    );

    if (result != 0) {
        outhash.clear();
    }
#else
    for (size_t i = 0; i < outhash.size(); ++i) {
        uint8_t temp_byte = combined.empty() ? 0 : combined[i % combined.size()];
        uint8_t mix_factor = static_cast<uint8_t>((argon2_time ^ argon2_mem) & 0xFF);
        outhash[i] = static_cast<uint8_t>(temp_byte ^ mix_factor);
    }
#endif

    volatile uint8_t* ptr = combined.data();
    size_t size = combined.size();
    while (size--) {
        *ptr++ = 0;
    }
    combined.clear();
}
