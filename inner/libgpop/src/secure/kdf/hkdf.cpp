#include <secure/kdf/kdf.hpp>
#include <random>
#include <algorithm>
#include <secure/ngc.hpp>
#include <cstring>

#ifdef HAVE_SODIUM
#include <sodium.h>
#endif

void kdf_hkdf(
    const uint8_t* info, size_t info_len,
    gpop::Secure::Salt& salt,
    gpop::Secure::Pepper& pepper,
    gpop::Secure::HashT& outhash,
    gpop::Secure::Data& indata
) {
    gpop::Secure::Data ikm;
    ikm.reserve(indata.size() + pepper.size());
    ikm.insert(ikm.end(), indata.begin(), indata.end());
    if (!pepper.empty()) {
        ikm.insert(ikm.end(), pepper.begin(), pepper.end());
    }

    if (outhash.empty()) {
        outhash.resize(32);
    }

#ifdef HAVE_SODIUM
    if (sodium_init() < 0) {
        volatile uint8_t* fail_ptr = ikm.data();
        size_t fail_size = ikm.size();
        while (fail_size--) *fail_ptr++ = 0;
        ikm.clear();
        outhash.clear();
        return;
    }

    unsigned char prk[crypto_kdf_hkdf_sha256_KEYBYTES];

    const unsigned char* salt_ptr = salt.empty() ? nullptr : salt.data();
    size_t salt_len = salt.empty() ? 0 : salt.size();

    if (crypto_kdf_hkdf_sha256_extract(prk, salt_ptr, salt_len, ikm.data(), ikm.size()) != 0) {
        outhash.clear();
        volatile uint8_t* fail_ptr = ikm.data();
        size_t fail_size = ikm.size();
        while (fail_size--) *fail_ptr++ = 0;
        ikm.clear();
        return;
    }

    const char* default_info = "gpop-hkdf-default";
    const unsigned char* info_ptr = (info && info_len > 0) ? info : reinterpret_cast<const unsigned char*>(default_info);
    size_t final_info_len = (info && info_len > 0) ? info_len : strlen(default_info);

    if (crypto_kdf_hkdf_sha256_expand(outhash.data(), outhash.size(), reinterpret_cast<const char*>(info_ptr), final_info_len, prk) != 0) {
        outhash.clear();
    }

    sodium_memzero(prk, sizeof(prk));

#else
    for (size_t i = 0; i < outhash.size(); ++i) {
        uint8_t temp_byte = ikm.empty() ? 0 : ikm[i % ikm.size()];
        outhash[i] = static_cast<uint8_t>(temp_byte ^ 0x42);
    }
#endif

    volatile uint8_t* ptr = ikm.data();
    size_t size = ikm.size();
    while (size--) {
        *ptr++ = 0;
    }
    ikm.clear();
}
