#include <string.h>
#include <secure/hash/hash.hpp>
#include <secure/ngc.hpp>
#include <stdbool.h>
#include "int.hpp"

namespace gpop::Secure::Hash {

HashProvider::HashProvider(AlgoNGC Algo) {
    this->algo = Algo;
    this->Error = 0;
    this->needsalt = false;
    this->needpepper = false;
}

void HashProvider::SetNeeds(bool nsalt, bool npepper)
{
    this->needsalt = nsalt;
    this->needpepper = npepper;
}

void HashProvider::SetData(const gpop::Secure::Data& indata,
                           const gpop::Secure::Pepper& pepper,
                           const gpop::Secure::Salt& salt)
{
    if (this->needsalt && salt.empty()) {
        this->Error = gpop::Secure::Hash::HASH_NOSALT;
        return;
    }

    if (this->needpepper && pepper.empty()) {
        this->Error = gpop::Secure::Hash::HASH_NOPEPPER;
        return;
    }

    if (indata.empty()) {
        this->Error = gpop::Secure::Hash::HASH_NODATA;
        return;
    }

    if (this->needsalt) this->salt = salt;
    if (this->needpepper) this->pepper = pepper;
    this->indata = indata;
    this->Error = HASH_SUCCESS;
}

void HashProvider::GenHash()
{
    const uint8_t* in_ptr     = this->indata.empty() ? nullptr : this->indata.data();
    const uint8_t* pepper_ptr = this->pepper.empty() ? nullptr : this->pepper.data();
    const uint8_t* salt_ptr   = this->salt.empty()   ? nullptr : this->salt.data();

    err status = HASH_SUCCESS;

    switch (this->algo)
    {
    case HASH_MD5:
        this->outhash.resize(16);
        status = md5_hash(
            in_ptr,   this->indata.size(),
            salt_ptr, this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    case HASH_MD4:
        this->outhash.resize(16);
        status = md4_hash(
            in_ptr,   this->indata.size(),
            salt_ptr, this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    case HASH_MD2:
        this->outhash.resize(16);
        status = md2_hash(
            in_ptr,   this->indata.size(),
            salt_ptr, this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    case HASH_SHA2_SHA256:
        this->outhash.resize(32);
        status = sha2_256_hash(
            in_ptr,   this->indata.size(),
            salt_ptr, this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    case HASH_SHA3_SHA256:
        this->outhash.resize(32);
        status = sha3_256_hash(
            in_ptr,   this->indata.size(),
            salt_ptr, this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    case HASH_SHA2_SHA512:
        this->outhash.resize(64);
        status = sha2_512_hash(
            in_ptr,     this->indata.size(),
            pepper_ptr, this->pepper.size(),
            salt_ptr,   this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    case HASH_SHA3_SHA512:
        this->outhash.resize(64);
        status = sha3_512_hash(
            in_ptr,     this->indata.size(),
            pepper_ptr, this->pepper.size(),
            salt_ptr,   this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;


    case HASH_SHA2_SHA384:
        this->outhash.resize(48);
        status = sha2_384_hash(
            in_ptr,     this->indata.size(),
            pepper_ptr, this->pepper.size(),
            salt_ptr,   this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    case HASH_SHA3_SHA384:
        this->outhash.resize(48);
        status = sha3_384_hash(
            in_ptr,     this->indata.size(),
            pepper_ptr, this->pepper.size(),
            salt_ptr,   this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    case HASH_SHA2_SHA224:
        this->outhash.resize(28);
        status = sha2_224_hash(
            in_ptr,     this->indata.size(),
            pepper_ptr, this->pepper.size(),
            salt_ptr,   this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    case HASH_SHA3_SHA224:
        this->outhash.resize(28);
        status = sha3_224_hash(
            in_ptr,     this->indata.size(),
            pepper_ptr, this->pepper.size(),
            salt_ptr,   this->salt.size(),
            this->outhash.data(), this->outhash.size()
        );
        break;

    default:
        this->Error = HASH_INVALIDPROVIDER;
        this->outhash.clear();
        return;
    }

    if (status != HASH_SUCCESS) {
        this->Error = status;
        this->outhash.clear();
        return;
    }

    this->Error = HASH_SUCCESS;
}

gpop::Secure::HashT HashProvider::GetHash()
{
    return this->outhash;
}

gpop::Secure::HashT HashProvider::GetHash(gpop::Secure::Salt& out_salt)
{
    out_salt = this->salt;
    return this->outhash;
}

HashProvider::~HashProvider() {
    // Do nothing
}

} // namespace gpop::Secure::Hash
