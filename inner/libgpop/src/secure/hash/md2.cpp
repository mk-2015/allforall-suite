#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define HASH_SUCCESS   0ULL
#define HASH_NODATA    3ULL
#define HASH_BADOUTPUT 5ULL

#define DIGEST_SIZE_MD2 16
#define MD2_BLOCK_SIZE  16

static void secure_zero(void* p, size_t n)
{
    volatile uint8_t* v = (volatile uint8_t*)p;
    while (n--) *v++ = 0;
}

static const uint8_t PI_SUBST[256] = {
     41,  46,  67, 201, 162, 216, 124,   1,  61,  54,  84, 161, 236, 240,   6,  19,
     98, 167,   5, 243, 192, 199, 115, 140, 152, 147,  43, 217, 188,  76, 130, 202,
     30, 155,  87,  60, 253, 212, 224,  22, 103,  66, 111,  24, 138,  23, 229,  18,
    190,  78, 196, 214, 218, 158, 222,  73, 160, 251, 245, 142, 187,  47, 238, 122,
    169, 104, 121, 145,  21, 178,   7,  63, 148, 194,  16, 137,  11,  34,  95,  33,
    128, 127,  93, 154,  90, 144,  50,  39,  53,  62, 204, 231, 191, 247, 151,   3,
    255,  25,  48, 179,  72, 165, 181, 209, 215,  94, 146,  42, 172,  86, 170, 198,
     79, 184,  56, 210, 150, 164, 125, 182, 118, 252, 107, 226, 156, 116,   4, 241,
     69, 157, 112,  89, 100, 113, 135,  32, 134,  91, 207, 101, 230,  45, 168,   2,
     27,  96,  37, 173, 174, 176, 185, 246,  28,  70,  97, 105,  52,  64, 126,  15,
     85,  71, 163,  35, 221,  81, 175,  58, 195,  92, 249, 206, 186, 197, 234,  38,
     44,  83,  13, 110, 133,  40, 132,   9, 211, 223, 205, 244,  65, 129,  77,  82,
    106, 220,  55, 200, 108, 193, 171, 250,  36, 225, 123,   8,  12, 189, 177,  74,
    120, 136, 149, 139, 227,  99, 232, 109, 233, 203, 213, 254,  59,   0,  29,  57,
    242, 239, 183,  14, 102,  88, 208, 228, 166, 119, 114, 248, 235, 117,  75,  10,
     49,  68,  80, 180, 143, 237,  31,  26, 219, 153, 141,  51, 159,  17, 131,  20,
};

typedef struct {
    uint8_t X[48];
    uint8_t C[16];
    uint8_t L;
    uint8_t buf[MD2_BLOCK_SIZE];
    size_t  buflen;
} Md2Ctx;

static void md2_init(Md2Ctx* c)
{
    memset(c, 0, sizeof(*c));
}

static void md2_checksum(Md2Ctx* c, const uint8_t block[MD2_BLOCK_SIZE])
{
    int j;
    for (j = 0; j < 16; ++j) {
        c->C[j] ^= PI_SUBST[block[j] ^ c->L];
        c->L = c->C[j];
    }
}

static void md2_transform(Md2Ctx* c, const uint8_t block[MD2_BLOCK_SIZE])
{
    int j, k;
    unsigned t;

    for (j = 0; j < 16; ++j) {
        c->X[16 + j] = block[j];
        c->X[32 + j] = (uint8_t)(c->X[16 + j] ^ c->X[j]);
    }

    t = 0;
    for (j = 0; j < 18; ++j) {
        for (k = 0; k < 48; ++k) {
            c->X[k] ^= PI_SUBST[t];
            t = c->X[k];
        }
        t = (t + (unsigned)j) & 0xff;
    }
}

static void md2_update(Md2Ctx* c, const uint8_t* data, size_t len)
{
    if (data == NULL || len == 0) return;

    while (len > 0) {
        size_t take = MD2_BLOCK_SIZE - c->buflen;
        if (take > len) take = len;
        memcpy(c->buf + c->buflen, data, take);
        c->buflen += take;
        data += take;
        len  -= take;
        if (c->buflen == MD2_BLOCK_SIZE) {
            md2_checksum(c, c->buf);
            md2_transform(c, c->buf);
            c->buflen = 0;
        }
    }
}

static void md2_final(Md2Ctx* c, uint8_t out[DIGEST_SIZE_MD2])
{
    uint8_t padn = (uint8_t)(MD2_BLOCK_SIZE - c->buflen);
    uint8_t sum[16];

    memset(c->buf + c->buflen, padn, padn);
    md2_checksum(c, c->buf);
    md2_transform(c, c->buf);

    memcpy(sum, c->C, sizeof(sum));
    md2_transform(c, sum);

    memcpy(out, c->X, DIGEST_SIZE_MD2);

    secure_zero(sum, sizeof(sum));
    secure_zero(c, sizeof(*c));
}

unsigned long long md2_hash(
    const uint8_t* indata,  size_t indata_s,
    const uint8_t* salt,    size_t salt_s,
    uint8_t*       outhash, size_t outhash_s)
{
    Md2Ctx ctx;

    if (outhash == NULL || outhash_s < DIGEST_SIZE_MD2)
        return HASH_BADOUTPUT;
    if ((indata == NULL && indata_s != 0) ||
        (salt   == NULL && salt_s   != 0))
        return HASH_NODATA;

    md2_init(&ctx);
    md2_update(&ctx, indata, indata_s);
    md2_update(&ctx, salt,   salt_s);
    md2_final(&ctx, outhash);

    return HASH_SUCCESS;
}
