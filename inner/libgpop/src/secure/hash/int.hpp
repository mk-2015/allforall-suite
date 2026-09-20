#pragma once

#include <cstddef>
#include <cstdint>
#include <secure/ngc.hpp>
#include <secure/hash/hash.hpp>

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long md5_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s
);

unsigned long long md2_hash(
    const uint8_t* indata,  size_t indata_s,
    const uint8_t* salt,    size_t salt_s,
    uint8_t*       outhash, size_t outhash_s
);

unsigned long long md4_hash(
    const uint8_t* indata,  size_t indata_s,
    const uint8_t* salt,    size_t salt_s,
    uint8_t*       outhash, size_t outhash_s
);

unsigned long long sha2_256_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s
);

unsigned long long sha3_256_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s
);

unsigned long long sha2_512_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* pepper,  std::size_t pepper_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s
);

unsigned long long sha3_512_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* pepper,  std::size_t pepper_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s
);

unsigned long long sha2_384_hash(
    const uint8_t* indata,  size_t indata_s,
    const uint8_t* pepper,  size_t pepper_s,
    const uint8_t* salt,    size_t salt_s,
    uint8_t*       outhash, size_t outhash_s
);

unsigned long long sha3_384_hash(
    const uint8_t* indata,  size_t indata_s,
    const uint8_t* pepper,  size_t pepper_s,
    const uint8_t* salt,    size_t salt_s,
    uint8_t*       outhash, size_t outhash_s
);

unsigned long long sha2_224_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* pepper,  std::size_t pepper_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s
);

unsigned long long sha3_224_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* pepper,  std::size_t pepper_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s
);

#ifdef __cplusplus
}
#endif
