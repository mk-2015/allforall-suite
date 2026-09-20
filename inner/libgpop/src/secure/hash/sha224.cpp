#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define HASH_SUCCESS   0ULL
#define HASH_NODATA    3ULL
#define HASH_BADOUTPUT 5ULL

#define DIGEST_SIZE_224   28
#define SHA256_BLOCK_SIZE 64
#define SHA3_224_RATE     144

static void secure_zero(void* p, size_t n)
{
    volatile uint8_t* v = (volatile uint8_t*)p;
    while (n--) *v++ = 0;
}

static unsigned long long check_args(
    const uint8_t* indata, size_t indata_s,
    const uint8_t* pepper, size_t pepper_s,
    const uint8_t* salt,   size_t salt_s,
    const uint8_t* outhash, size_t outhash_s)
{
    if (outhash == NULL || outhash_s < DIGEST_SIZE_224)
        return HASH_BADOUTPUT;
    if ((indata == NULL && indata_s != 0) ||
        (pepper == NULL && pepper_s != 0) ||
        (salt   == NULL && salt_s   != 0))
        return HASH_NODATA;
    return HASH_SUCCESS;
}

static const uint32_t K256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static inline uint32_t rotr32(uint32_t x, unsigned n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z)  { return (x & y) ^ (~x & z); }
static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t Sig0(uint32_t x) { return rotr32(x, 2)  ^ rotr32(x, 13) ^ rotr32(x, 22); }
static inline uint32_t Sig1(uint32_t x) { return rotr32(x, 6)  ^ rotr32(x, 11) ^ rotr32(x, 25); }
static inline uint32_t sig0(uint32_t x) { return rotr32(x, 7)  ^ rotr32(x, 18) ^ (x >> 3); }
static inline uint32_t sig1(uint32_t x) { return rotr32(x, 17) ^ rotr32(x, 19) ^ (x >> 10); }

typedef struct {
    uint32_t state[8];
    uint8_t  buf[SHA256_BLOCK_SIZE];
    size_t   buflen;
    uint64_t total;
} Sha224Ctx;

static void sha224_init(Sha224Ctx* c)
{
    c->state[0] = 0xc1059ed8;
    c->state[1] = 0x367cd507;
    c->state[2] = 0x3070dd17;
    c->state[3] = 0xf70e5939;
    c->state[4] = 0xffc00b31;
    c->state[5] = 0x68581511;
    c->state[6] = 0x64f98fa7;
    c->state[7] = 0xbefa4fa4;
    c->buflen = 0;
    c->total  = 0;
    memset(c->buf, 0, sizeof(c->buf));
}

static void sha224_compress(uint32_t state[8], const uint8_t block[SHA256_BLOCK_SIZE])
{
    uint32_t W[64];
    int i;

    for (i = 0; i < 16; ++i) {
        const uint8_t* p = block + i * 4;
        W[i] = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
               ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
    }
    for (i = 16; i < 64; ++i)
        W[i] = sig1(W[i - 2]) + W[i - 7] + sig0(W[i - 15]) + W[i - 16];

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (i = 0; i < 64; ++i) {
        uint32_t t1 = h + Sig1(e) + Ch(e, f, g) + K256[i] + W[i];
        uint32_t t2 = Sig0(a) + Maj(a, b, c);
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

static void sha224_update(Sha224Ctx* c, const uint8_t* data, size_t len)
{
    if (data == NULL || len == 0) return;

    c->total += (uint64_t)len;

    if (c->buflen > 0) {
        size_t take = SHA256_BLOCK_SIZE - c->buflen;
        if (take > len) take = len;
        memcpy(c->buf + c->buflen, data, take);
        c->buflen += take;
        data += take;
        len  -= take;
        if (c->buflen == SHA256_BLOCK_SIZE) {
            sha224_compress(c->state, c->buf);
            c->buflen = 0;
        }
    }

    while (len >= SHA256_BLOCK_SIZE) {
        sha224_compress(c->state, data);
        data += SHA256_BLOCK_SIZE;
        len  -= SHA256_BLOCK_SIZE;
    }

    if (len > 0) {
        memcpy(c->buf, data, len);
        c->buflen = len;
    }
}

static void sha224_final(Sha224Ctx* c, uint8_t out[DIGEST_SIZE_224])
{
    const uint64_t bits = c->total << 3;
    int i, j;

    c->buf[c->buflen++] = 0x80;

    if (c->buflen > SHA256_BLOCK_SIZE - 8) {
        memset(c->buf + c->buflen, 0, SHA256_BLOCK_SIZE - c->buflen);
        sha224_compress(c->state, c->buf);
        c->buflen = 0;
    }

    memset(c->buf + c->buflen, 0, SHA256_BLOCK_SIZE - 8 - c->buflen);

    for (i = 0; i < 8; ++i)
        c->buf[56 + i] = (uint8_t)(bits >> (56 - 8 * i));
    sha224_compress(c->state, c->buf);

    for (i = 0; i < 7; ++i)
        for (j = 0; j < 4; ++j)
            out[i * 4 + j] = (uint8_t)(c->state[i] >> (24 - 8 * j));

    secure_zero(c, sizeof(*c));
}

static const uint64_t KECCAK_RC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL, 0x8000000080008000ULL,
    0x000000000000808bULL, 0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
    0x000000000000008aULL, 0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
    0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800aULL, 0x800000008000000aULL,
    0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

static const unsigned KECCAK_ROTC[24] = {
    1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14,
    27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44
};

static const unsigned KECCAK_PILN[24] = {
    10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4,
    15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1
};

static inline uint64_t rotl64(uint64_t x, unsigned n) { return (x << n) | (x >> (64 - n)); }

static void keccak_f1600(uint64_t st[25])
{
    uint64_t bc[5];
    int round, i, j;

    for (round = 0; round < 24; ++round) {
        for (i = 0; i < 5; ++i)
            bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];
        for (i = 0; i < 5; ++i) {
            uint64_t t = bc[(i + 4) % 5] ^ rotl64(bc[(i + 1) % 5], 1);
            for (j = 0; j < 25; j += 5)
                st[j + i] ^= t;
        }

        uint64_t t = st[1];
        for (i = 0; i < 24; ++i) {
            unsigned k = KECCAK_PILN[i];
            uint64_t tmp = st[k];
            st[k] = rotl64(t, KECCAK_ROTC[i]);
            t = tmp;
        }

        for (j = 0; j < 25; j += 5) {
            for (i = 0; i < 5; ++i) bc[i] = st[j + i];
            for (i = 0; i < 5; ++i)
                st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
        }

        st[0] ^= KECCAK_RC[round];
    }

    secure_zero(bc, sizeof(bc));
}

typedef struct {
    uint64_t st[25];
    uint8_t  buf[SHA3_224_RATE];
    size_t   buflen;
} Sha3Ctx;

static void sha3_init(Sha3Ctx* c)
{
    memset(c->st,  0, sizeof(c->st));
    memset(c->buf, 0, sizeof(c->buf));
    c->buflen = 0;
}

static void sha3_absorb_block(Sha3Ctx* c)
{
    size_t i;
    for (i = 0; i < SHA3_224_RATE; ++i)
        c->st[i / 8] ^= (uint64_t)c->buf[i] << (8 * (i % 8));
    keccak_f1600(c->st);
}

static void sha3_update(Sha3Ctx* c, const uint8_t* data, size_t len)
{
    if (data == NULL || len == 0) return;

    while (len > 0) {
        size_t take = SHA3_224_RATE - c->buflen;
        if (take > len) take = len;
        memcpy(c->buf + c->buflen, data, take);
        c->buflen += take;
        data += take;
        len  -= take;
        if (c->buflen == SHA3_224_RATE) {
            sha3_absorb_block(c);
            c->buflen = 0;
        }
    }
}

static void sha3_final(Sha3Ctx* c, uint8_t out[DIGEST_SIZE_224])
{
    size_t i;

    memset(c->buf + c->buflen, 0, SHA3_224_RATE - c->buflen);
    c->buf[c->buflen] ^= 0x06;
    c->buf[SHA3_224_RATE - 1] ^= 0x80;
    sha3_absorb_block(c);

    for (i = 0; i < DIGEST_SIZE_224; ++i)
        out[i] = (uint8_t)(c->st[i / 8] >> (8 * (i % 8)));

    secure_zero(c, sizeof(*c));
}

unsigned long long sha2_224_hash(
    const uint8_t* indata,  size_t indata_s,
    const uint8_t* pepper,  size_t pepper_s,
    const uint8_t* salt,    size_t salt_s,
    uint8_t*       outhash, size_t outhash_s)
{
    Sha224Ctx ctx;
    unsigned long long e = check_args(indata, indata_s, pepper, pepper_s,
                                      salt, salt_s, outhash, outhash_s);
    if (e != HASH_SUCCESS) return e;

    sha224_init(&ctx);
    sha224_update(&ctx, indata, indata_s);
    sha224_update(&ctx, pepper, pepper_s);
    sha224_update(&ctx, salt,   salt_s);
    sha224_final(&ctx, outhash);

    return HASH_SUCCESS;
}

unsigned long long sha3_224_hash(
    const uint8_t* indata,  size_t indata_s,
    const uint8_t* pepper,  size_t pepper_s,
    const uint8_t* salt,    size_t salt_s,
    uint8_t*       outhash, size_t outhash_s)
{
    Sha3Ctx ctx;
    unsigned long long e = check_args(indata, indata_s, pepper, pepper_s,
                                      salt, salt_s, outhash, outhash_s);
    if (e != HASH_SUCCESS) return e;

    sha3_init(&ctx);
    sha3_update(&ctx, indata, indata_s);
    sha3_update(&ctx, pepper, pepper_s);
    sha3_update(&ctx, salt,   salt_s);
    sha3_final(&ctx, outhash);

    return HASH_SUCCESS;
}
