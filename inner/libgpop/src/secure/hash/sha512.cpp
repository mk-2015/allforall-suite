#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <secure/hash/hash.hpp>
#include <secure/ngc.hpp>

namespace gpop::Secure::Hash {

namespace {

constexpr size_t DIGEST_SIZE_512 = 64;

void secure_zero(void* p, size_t n)
{
    volatile uint8_t* v = static_cast<volatile uint8_t*>(p);
    while (n--) *v++ = 0;
}

gpop::Secure::err check_args(
    const uint8_t* indata, size_t indata_s,
    const uint8_t* pepper, size_t pepper_s,
    const uint8_t* salt,   size_t salt_s,
    const uint8_t* outhash, size_t outhash_s)
{
    if (outhash == nullptr || outhash_s < DIGEST_SIZE_512)
        return HASH_BADOUTPUT;
    if ((indata == nullptr && indata_s != 0) ||
        (pepper == nullptr && pepper_s != 0) ||
        (salt   == nullptr && salt_s   != 0))
        return HASH_NODATA;
    return HASH_SUCCESS;
}

constexpr size_t SHA512_BLOCK_SIZE = 128;

const uint64_t K512[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

inline uint64_t rotr64(uint64_t x, unsigned n) { return (x >> n) | (x << (64 - n)); }
inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z)  { return (x & y) ^ (~x & z); }
inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z) { return (x & y) ^ (x & z) ^ (y & z); }
inline uint64_t Sig0(uint64_t x) { return rotr64(x, 28) ^ rotr64(x, 34) ^ rotr64(x, 39); }
inline uint64_t Sig1(uint64_t x) { return rotr64(x, 14) ^ rotr64(x, 18) ^ rotr64(x, 41); }
inline uint64_t sig0(uint64_t x) { return rotr64(x, 1)  ^ rotr64(x, 8)  ^ (x >> 7); }
inline uint64_t sig1(uint64_t x) { return rotr64(x, 19) ^ rotr64(x, 61) ^ (x >> 6); }

struct Sha512Ctx {
    uint64_t state[8];
    uint8_t  buf[SHA512_BLOCK_SIZE];
    size_t   buflen;
    uint64_t total;
};

void sha512_init(Sha512Ctx& c)
{
    c.state[0] = 0x6a09e667f3bcc908ULL;
    c.state[1] = 0xbb67ae8584caa73bULL;
    c.state[2] = 0x3c6ef372fe94f82bULL;
    c.state[3] = 0xa54ff53a5f1d36f1ULL;
    c.state[4] = 0x510e527fade682d1ULL;
    c.state[5] = 0x9b05688c2b3e6c1fULL;
    c.state[6] = 0x1f83d9abfb41bd6bULL;
    c.state[7] = 0x5be0cd19137e2179ULL;
    c.buflen = 0;
    c.total  = 0;
    memset(c.buf, 0, sizeof(c.buf));
}

void sha512_compress(uint64_t state[8], const uint8_t block[SHA512_BLOCK_SIZE])
{
    uint64_t W[80];

    for (int i = 0; i < 16; ++i) {
        const uint8_t* p = block + i * 8;
        W[i] = ((uint64_t)p[0] << 56) | ((uint64_t)p[1] << 48) |
               ((uint64_t)p[2] << 40) | ((uint64_t)p[3] << 32) |
               ((uint64_t)p[4] << 24) | ((uint64_t)p[5] << 16) |
               ((uint64_t)p[6] << 8)  |  (uint64_t)p[7];
    }
    for (int i = 16; i < 80; ++i)
        W[i] = sig1(W[i - 2]) + W[i - 7] + sig0(W[i - 15]) + W[i - 16];

    uint64_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint64_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 80; ++i) {
        uint64_t t1 = h + Sig1(e) + Ch(e, f, g) + K512[i] + W[i];
        uint64_t t2 = Sig0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;

    secure_zero(W, sizeof(W));
}

void sha512_update(Sha512Ctx& c, const uint8_t* data, size_t len)
{
    if (data == nullptr || len == 0) return;

    c.total += (uint64_t)len;

    if (c.buflen > 0) {
        size_t take = SHA512_BLOCK_SIZE - c.buflen;
        if (take > len) take = len;
        memcpy(c.buf + c.buflen, data, take);
        c.buflen += take;
        data += take;
        len  -= take;
        if (c.buflen == SHA512_BLOCK_SIZE) {
            sha512_compress(c.state, c.buf);
            c.buflen = 0;
        }
    }

    while (len >= SHA512_BLOCK_SIZE) {
        sha512_compress(c.state, data);
        data += SHA512_BLOCK_SIZE;
        len  -= SHA512_BLOCK_SIZE;
    }

    if (len > 0) {
        memcpy(c.buf, data, len);
        c.buflen = len;
    }
}

void sha512_final(Sha512Ctx& c, uint8_t out[DIGEST_SIZE_512])
{
    const uint64_t bits_hi = c.total >> 61;
    const uint64_t bits_lo = c.total << 3;

    c.buf[c.buflen++] = 0x80;

    if (c.buflen > SHA512_BLOCK_SIZE - 16) {
        memset(c.buf + c.buflen, 0, SHA512_BLOCK_SIZE - c.buflen);
        sha512_compress(c.state, c.buf);
        c.buflen = 0;
    }

    memset(c.buf + c.buflen, 0, SHA512_BLOCK_SIZE - 16 - c.buflen);

    for (int i = 0; i < 8; ++i) {
        c.buf[112 + i] = (uint8_t)(bits_hi >> (56 - 8 * i));
        c.buf[120 + i] = (uint8_t)(bits_lo >> (56 - 8 * i));
    }
    sha512_compress(c.state, c.buf);

    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            out[i * 8 + j] = (uint8_t)(c.state[i] >> (56 - 8 * j));

    secure_zero(&c, sizeof(c));
}

constexpr size_t SHA3_512_RATE = 72;

const uint64_t KECCAK_RC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL, 0x8000000080008000ULL,
    0x000000000000808bULL, 0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
    0x000000000000008aULL, 0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
    0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800aULL, 0x800000008000000aULL,
    0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

const unsigned KECCAK_ROTC[24] = {
    1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14,
    27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44
};

const unsigned KECCAK_PILN[24] = {
    10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4,
    15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1
};

inline uint64_t rotl64(uint64_t x, unsigned n) { return (x << n) | (x >> (64 - n)); }

void keccak_f1600(uint64_t st[25])
{
    uint64_t bc[5];

    for (int round = 0; round < 24; ++round) {
        for (int i = 0; i < 5; ++i)
            bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];
        for (int i = 0; i < 5; ++i) {
            uint64_t t = bc[(i + 4) % 5] ^ rotl64(bc[(i + 1) % 5], 1);
            for (int j = 0; j < 25; j += 5)
                st[j + i] ^= t;
        }

        uint64_t t = st[1];
        for (int i = 0; i < 24; ++i) {
            unsigned j = KECCAK_PILN[i];
            uint64_t tmp = st[j];
            st[j] = rotl64(t, KECCAK_ROTC[i]);
            t = tmp;
        }

        for (int j = 0; j < 25; j += 5) {
            for (int i = 0; i < 5; ++i) bc[i] = st[j + i];
            for (int i = 0; i < 5; ++i)
                st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
        }

        st[0] ^= KECCAK_RC[round];
    }

    secure_zero(bc, sizeof(bc));
}

struct Sha3Ctx {
    uint64_t st[25];
    uint8_t  buf[SHA3_512_RATE];
    size_t   buflen;
};

void sha3_init(Sha3Ctx& c)
{
    memset(c.st,  0, sizeof(c.st));
    memset(c.buf, 0, sizeof(c.buf));
    c.buflen = 0;
}

void sha3_absorb_block(Sha3Ctx& c)
{
    for (size_t i = 0; i < SHA3_512_RATE; ++i)
        c.st[i / 8] ^= (uint64_t)c.buf[i] << (8 * (i % 8));
    keccak_f1600(c.st);
}

void sha3_update(Sha3Ctx& c, const uint8_t* data, size_t len)
{
    if (data == nullptr || len == 0) return;

    while (len > 0) {
        size_t take = SHA3_512_RATE - c.buflen;
        if (take > len) take = len;
        memcpy(c.buf + c.buflen, data, take);
        c.buflen += take;
        data += take;
        len  -= take;
        if (c.buflen == SHA3_512_RATE) {
            sha3_absorb_block(c);
            c.buflen = 0;
        }
    }
}

void sha3_final(Sha3Ctx& c, uint8_t out[DIGEST_SIZE_512])
{
    memset(c.buf + c.buflen, 0, SHA3_512_RATE - c.buflen);
    c.buf[c.buflen] ^= 0x06;
    c.buf[SHA3_512_RATE - 1] ^= 0x80;
    sha3_absorb_block(c);

    for (size_t i = 0; i < DIGEST_SIZE_512; ++i)
        out[i] = (uint8_t)(c.st[i / 8] >> (8 * (i % 8)));

    secure_zero(&c, sizeof(c));
}

}

gpop::Secure::err sha2_512_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* pepper,  std::size_t pepper_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s)
{
    gpop::Secure::err e = check_args(indata, indata_s, pepper, pepper_s,
                                     salt, salt_s, outhash, outhash_s);
    if (e != HASH_SUCCESS) return e;

    Sha512Ctx ctx;
    sha512_init(ctx);
    sha512_update(ctx, indata, indata_s);
    sha512_update(ctx, pepper, pepper_s);
    sha512_update(ctx, salt,   salt_s);
    sha512_final(ctx, outhash);

    return HASH_SUCCESS;
}

gpop::Secure::err sha3_512_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* pepper,  std::size_t pepper_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s)
{
    gpop::Secure::err e = check_args(indata, indata_s, pepper, pepper_s,
                                     salt, salt_s, outhash, outhash_s);
    if (e != HASH_SUCCESS) return e;

    Sha3Ctx ctx;
    sha3_init(ctx);
    sha3_update(ctx, indata, indata_s);
    sha3_update(ctx, pepper, pepper_s);
    sha3_update(ctx, salt,   salt_s);
    sha3_final(ctx, outhash);

    return HASH_SUCCESS;
}

}
