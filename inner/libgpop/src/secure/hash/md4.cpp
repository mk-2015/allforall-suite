#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define HASH_SUCCESS   0ULL
#define HASH_NODATA    3ULL
#define HASH_BADOUTPUT 5ULL

#define DIGEST_SIZE_MD4 16
#define MD4_BLOCK_SIZE  64

static void secure_zero(void* p, size_t n)
{
    volatile uint8_t* v = (volatile uint8_t*)p;
    while (n--) *v++ = 0;
}

static inline uint32_t rotl32(uint32_t x, unsigned n) { return (x << n) | (x >> (32 - n)); }
static inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
static inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (x & z) | (y & z); }
static inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }

typedef struct {
    uint32_t state[4];
    uint8_t  buf[MD4_BLOCK_SIZE];
    size_t   buflen;
    uint64_t total;
} Md4Ctx;

static void md4_init(Md4Ctx* c)
{
    c->state[0] = 0x67452301;
    c->state[1] = 0xefcdab89;
    c->state[2] = 0x98badcfe;
    c->state[3] = 0x10325476;
    c->buflen = 0;
    c->total  = 0;
    memset(c->buf, 0, sizeof(c->buf));
}

static void md4_compress(uint32_t state[4], const uint8_t block[MD4_BLOCK_SIZE])
{
    static const unsigned S1[4] = { 3, 7, 11, 19 };
    static const unsigned S2[4] = { 3, 5, 9, 13 };
    static const unsigned S3[4] = { 3, 9, 11, 15 };
    static const unsigned O3[16] = { 0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15 };

    uint32_t X[16];
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t t;
    int i;

    for (i = 0; i < 16; ++i) {
        const uint8_t* p = block + i * 4;
        X[i] = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
               ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    }

    for (i = 0; i < 16; ++i) {
        t = rotl32(a + F(b, c, d) + X[i], S1[i % 4]);
        a = d; d = c; c = b; b = t;
    }

    for (i = 0; i < 16; ++i) {
        t = rotl32(a + G(b, c, d) + X[(i % 4) * 4 + i / 4] + 0x5a827999, S2[i % 4]);
        a = d; d = c; c = b; b = t;
    }

    for (i = 0; i < 16; ++i) {
        t = rotl32(a + H(b, c, d) + X[O3[i]] + 0x6ed9eba1, S3[i % 4]);
        a = d; d = c; c = b; b = t;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;

    secure_zero(X, sizeof(X));
}

static void md4_update(Md4Ctx* c, const uint8_t* data, size_t len)
{
    if (data == NULL || len == 0) return;

    c->total += (uint64_t)len;

    if (c->buflen > 0) {
        size_t take = MD4_BLOCK_SIZE - c->buflen;
        if (take > len) take = len;
        memcpy(c->buf + c->buflen, data, take);
        c->buflen += take;
        data += take;
        len  -= take;
        if (c->buflen == MD4_BLOCK_SIZE) {
            md4_compress(c->state, c->buf);
            c->buflen = 0;
        }
    }

    while (len >= MD4_BLOCK_SIZE) {
        md4_compress(c->state, data);
        data += MD4_BLOCK_SIZE;
        len  -= MD4_BLOCK_SIZE;
    }

    if (len > 0) {
        memcpy(c->buf, data, len);
        c->buflen = len;
    }
}

static void md4_final(Md4Ctx* c, uint8_t out[DIGEST_SIZE_MD4])
{
    const uint64_t bits = c->total << 3;
    int i;

    c->buf[c->buflen++] = 0x80;

    if (c->buflen > MD4_BLOCK_SIZE - 8) {
        memset(c->buf + c->buflen, 0, MD4_BLOCK_SIZE - c->buflen);
        md4_compress(c->state, c->buf);
        c->buflen = 0;
    }

    memset(c->buf + c->buflen, 0, MD4_BLOCK_SIZE - 8 - c->buflen);

    for (i = 0; i < 8; ++i)
        c->buf[56 + i] = (uint8_t)(bits >> (8 * i));
    md4_compress(c->state, c->buf);

    for (i = 0; i < 4; ++i) {
        out[i * 4 + 0] = (uint8_t)(c->state[i]);
        out[i * 4 + 1] = (uint8_t)(c->state[i] >> 8);
        out[i * 4 + 2] = (uint8_t)(c->state[i] >> 16);
        out[i * 4 + 3] = (uint8_t)(c->state[i] >> 24);
    }

    secure_zero(c, sizeof(*c));
}

unsigned long long md4_hash(
    const uint8_t* indata,  size_t indata_s,
    const uint8_t* salt,    size_t salt_s,
    uint8_t*       outhash, size_t outhash_s)
{
    Md4Ctx ctx;

    if (outhash == NULL || outhash_s < DIGEST_SIZE_MD4)
        return HASH_BADOUTPUT;
    if ((indata == NULL && indata_s != 0) ||
        (salt   == NULL && salt_s   != 0))
        return HASH_NODATA;

    md4_init(&ctx);
    md4_update(&ctx, indata, indata_s);
    md4_update(&ctx, salt,   salt_s);
    md4_final(&ctx, outhash);

    return HASH_SUCCESS;
}
