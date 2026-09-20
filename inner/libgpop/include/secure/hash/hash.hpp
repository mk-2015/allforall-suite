#pragma once

#include <secure/ngc.hpp>
#include <stdbool.h>
#include <stdint.h>

namespace gpop::Secure::Hash {

constexpr int HASH_SUCCESS = 0;
constexpr int HASH_NOSALT = 1;
constexpr int HASH_NOPEPPER = 2;
constexpr int HASH_NODATA = 3;
constexpr int HASH_INVALIDPROVIDER = 4;
constexpr int HASH_BADOUTPUT = 5;

constexpr int HASH_MD5 = 0;
constexpr int HASH_MD4 = 1;
constexpr int HASH_MD2 = 2;
constexpr int HASH_SHA2_SHA256 = 3;
constexpr int HASH_SHA3_SHA256 = 4;
constexpr int HASH_SHA2_SHA512 = 5;
constexpr int HASH_SHA3_SHA512 = 6;
constexpr int HASH_SHA2_SHA384 = 7;
constexpr int HASH_SHA3_SHA384 = 8;
constexpr int HASH_SHA2_SHA224 = 9;
constexpr int HASH_SHA3_SHA224 = 10;

class HashProvider {
public:
    gpop::Secure::AlgoNGC algo;
    gpop::Secure::err Error;

    bool needsalt;
    bool needpepper;

    gpop::Secure::Salt salt;
    gpop::Secure::Pepper pepper;
    gpop::Secure::HashT outhash;
    gpop::Secure::Data indata;

    HashProvider(gpop::Secure::AlgoNGC Algo);
    void SetNeeds(bool nsalt, bool npepper);
    void SetData(const gpop::Secure::Data& indata, const gpop::Secure::Pepper& pepper, const gpop::Secure::Salt& salt);
    void GenHash();
    gpop::Secure::HashT GetHash();
    gpop::Secure::HashT GetHash(gpop::Secure::Salt& out_salt);
    ~HashProvider();

    HashProvider(const HashProvider&) = delete;
    HashProvider& operator=(const HashProvider&) = delete;
};

} // namespace gpop::Secure::Hash
