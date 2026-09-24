#include <secure/kdf/kdf.hpp>
#include <random>
#include <algorithm>
#include <stdexcept>
#include "int.hpp"

namespace gpop::Secure::Kdf {

KdfProvider::KdfProvider(gpop::Secure::AlgoNGC Algo)
    : algo(Algo),
      Error(0),
      needsalt(false),
      needpepper(false) {}

void KdfProvider::SetNeeds(bool nsalt, bool npepper) {
    this->needsalt = nsalt;
    this->needpepper = npepper;
}

void KdfProvider::SetParam(KdfParams p) {
    this->params = p;
}

void KdfProvider::SetData(const gpop::Secure::Data& idata,
                          const gpop::Secure::Pepper& p,
                          const gpop::Secure::Salt& s) {
    this->indata = idata;
    this->pepper = p;
    this->salt = s;
}

void KdfProvider::GenPassword() {
    this->Error = 0;

    if (this->indata.empty()) {
        this->Error = KDF_NOINPUT;
        return;
    }

    switch (this->algo) {
        case Hkdf: {
            #ifdef HAVE_SODIUM
            kdf_hkdf(
                (this->params.info && this->params.info_len > 0) ? this->params.info : nullptr,
                (this->params.info && this->params.info_len > 0) ? this->params.info_len : 0,
                this->salt,
                this->pepper,
                this->outhash,
                this->indata
            );
            #else
            #warning "Cannot implement HKDF without sodium"
            this->Error = KDF_INVPROVIDER;
            #endif
            break;
        }

        case Argon2: {
            #ifdef HAVE_SODIUM
            kdf_argon2(
                this->params.argon2_time,
                this->params.argon2_memory,
                this->salt,
                this->pepper,
                this->outhash,
                this->indata
            );
            #else
            #warning "Cannot implement Argon2 without sodium"
            this->Error = KDF_INVPROVIDER;
            #endif
            break;
        }

        case Scrypt: {
            #ifdef HAVE_SODIUM
            kdf_scrypt(
                this->params.scrypt_N,
                this->params.scrypt_r,
                this->params.scrypt_p,
                this->salt,
                this->pepper,
                this->outhash,
                this->indata
            );
            #else
            #warning "Cannot implement scrypt without sodium"
            this->Error = KDF_INVPROVIDER;
            #endif
            break;
        }

        case Bcrypt: {
            this->Error = KDF_NOBCRYPT;
            #ifdef STAK_EXCEPTIONS
            throw std::runtime_error("No bcrypt is here");
            #endif
            break;
        }

        default:
            this->Error = KDF_INVPROVIDER;
            return;
    }
}

gpop::Secure::HashT KdfProvider::GetHash() const {
    return this->outhash;
}

gpop::Secure::HashT KdfProvider::GetHash(gpop::Secure::Salt& out_salt) const {
    out_salt = this->salt;
    return this->outhash;
}

KdfProvider::~KdfProvider() {
    auto secure_wipe = [](auto& vec) {
        volatile uint8_t* ptr = vec.data();
        size_t size = vec.size();
        while (size--) {
            *ptr++ = 0;
        }
        vec.clear();
    };

    secure_wipe(this->outhash);
    secure_wipe(this->salt);
    secure_wipe(this->pepper);
    secure_wipe(this->indata);
}

} // namespace gpop::Secure::Kdf
