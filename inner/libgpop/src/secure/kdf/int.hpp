#pragma once

#include <secure/ngc.hpp>
#include <cstdint>

void kdf_scrypt(
    uint64_t scrypt_N,
    uint32_t scrypt_r,
    uint32_t scrypt_p,
    gpop::Secure::Salt& salt,
    gpop::Secure::Pepper& pepper,
    gpop::Secure::HashT& outhash,
    gpop::Secure::Data& indata
);

void kdf_argon2(
    uint64_t argon2_time,
    size_t argon2_mem,
    gpop::Secure::Salt& salt,
    gpop::Secure::Pepper& pepper,
    gpop::Secure::HashT& outhash,
    gpop::Secure::Data& indata
);

void kdf_hkdf(
    const uint8_t* info, size_t info_len,
    gpop::Secure::Salt& salt,
    gpop::Secure::Pepper& pepper,
    gpop::Secure::HashT& outhash,
    gpop::Secure::Data& indata
);
