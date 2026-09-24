#pragma once

#include <secure/ngc.hpp>
#include <cstdint>

namespace gpop::Secure::Kdf {

constexpr int Argon2 = 0;
constexpr int Scrypt = 1;
constexpr int Bcrypt = 2;
constexpr int Hkdf   = 3;

constexpr int KDF_NOINPUT     = 1;
constexpr int KDF_INVPROVIDER = 2;
constexpr int KDF_NOBCRYPT    = 3;

struct KdfParams {
    // Common
    uint32_t iterations = 0;

    // Hkdf
    uint8_t* info;
    size_t info_len = 0;

    // Argon2
    uint32_t argon2_time = 0;
    uint32_t argon2_memory = 0;
    uint32_t argon2_parallel = 0;

    // scrypt
    uint64_t scrypt_N = 0;
    uint32_t scrypt_r = 0;
    uint32_t scrypt_p = 0;

    // bcrypt
    uint32_t bcrypt_cost = 0;
};

class KdfProvider {
public:
    gpop::Secure::AlgoNGC algo;
    gpop::Secure::err Error;

    bool needsalt;
    bool needpepper;

    gpop::Secure::Salt salt;
    gpop::Secure::Pepper pepper;
    gpop::Secure::HashT outhash;
    gpop::Secure::Data indata;

    KdfParams params;

    explicit KdfProvider(gpop::Secure::AlgoNGC Algo);

    void SetNeeds(bool nsalt, bool npepper);
    void SetParam(KdfParams params);
    void SetData(const gpop::Secure::Data& indata,
                 const gpop::Secure::Pepper& pepper,
                 const gpop::Secure::Salt& salt);

    void GenPassword();

    gpop::Secure::HashT GetHash() const;
    gpop::Secure::HashT GetHash(gpop::Secure::Salt& out_salt) const;

    ~KdfProvider();

    KdfProvider(const KdfProvider&) = delete;
    KdfProvider& operator=(const KdfProvider&) = delete;
};

} // namespace gpop::Secure::Kdf
