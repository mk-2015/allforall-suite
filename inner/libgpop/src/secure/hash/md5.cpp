#include "int.hpp"
#include <cstring>

static inline uint32_t left_rotate(uint32_t x, uint32_t c) {
    return (x << c) | (x >> (32 - c));
}

struct MD5Context {
    uint32_t state[4];
    uint64_t count;
    uint8_t  buffer[64];
};

static constexpr uint32_t S[] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

static constexpr uint32_t K[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static void md5_init(MD5Context* ctx) {
    ctx->count = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
}

static void md5_transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t M[16];

    for (int i = 0; i < 16; ++i) {
        M[i] = static_cast<uint32_t>(block[i * 4]) |
              (static_cast<uint32_t>(block[i * 4 + 1]) << 8) |
              (static_cast<uint32_t>(block[i * 4 + 2]) << 16) |
              (static_cast<uint32_t>(block[i * 4 + 3]) << 24);
    }

    for (int i = 0; i < 64; ++i) {
        uint32_t f = 0, g = 0;
        if (i < 16) {
            f = (b & c) | ((~b) & d);
            g = static_cast<uint32_t>(i);
        } else if (i < 32) {
            f = (d & b) | ((~d) & c);
            g = static_cast<uint32_t>((5 * i + 1) % 16);
        } else if (i < 48) {
            f = b ^ c ^ d;
            g = static_cast<uint32_t>((3 * i + 5) % 16);
        } else {
            f = c ^ (b | (~d));
            g = static_cast<uint32_t>((7 * i) % 16);
        }

        uint32_t temp = d;
        d = c;
        c = b;
        b = b + left_rotate(a + f + K[i] + M[g], S[i]);
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

static void md5_update(MD5Context* ctx, const uint8_t* input, std::size_t input_len) {
    std::size_t index = static_cast<std::size_t>((ctx->count >> 3) & 0x3F);
    ctx->count += static_cast<uint64_t>(input_len) << 3;
    std::size_t part_len = 64 - index;

    std::size_t i = 0;
    if (input_len >= part_len) {
        std::memcpy(&ctx->buffer[index], input, part_len);
        md5_transform(ctx->state, ctx->buffer);

        for (i = part_len; i + 63 < input_len; i += 64) {
            md5_transform(ctx->state, &input[i]);
        }
        index = 0;
    }

    if (i < input_len) {
        std::memcpy(&ctx->buffer[index], &input[i], input_len - i);
    }
}

static void md5_final(MD5Context* ctx, uint8_t digest[16]) {
    uint8_t bits[8];
    for (int i = 0; i < 8; ++i) {
        bits[i] = static_cast<uint8_t>((ctx->count >> (i * 8)) & 0xFF);
    }

    std::size_t index = static_cast<std::size_t>((ctx->count >> 3) & 0x3F);
    std::size_t pad_len = (index < 56) ? (56 - index) : (120 - index);

    static constexpr uint8_t PADDING[64] = { 0x80 };
    md5_update(ctx, PADDING, pad_len);
    md5_update(ctx, bits, 8);

    for (int i = 0; i < 4; ++i) {
        digest[i * 4]     = static_cast<uint8_t>(ctx->state[i] & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((ctx->state[i] >> 8) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((ctx->state[i] >> 16) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>((ctx->state[i] >> 24) & 0xFF);
    }
}

extern "C" unsigned long long md5_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s)
{
    if (outhash == nullptr || outhash_s < 16) {
        return gpop::Secure::Hash::HASH_INVALIDPROVIDER;
    }

    if (indata == nullptr && indata_s > 0) {
        return gpop::Secure::Hash::HASH_NODATA;
    }

    MD5Context ctx;
    md5_init(&ctx);

    if (indata && indata_s > 0) {
        md5_update(&ctx, indata, indata_s);
    }

    if (salt && salt_s > 0) {
        md5_update(&ctx, salt, salt_s);
    }

    md5_final(&ctx, outhash);
    return gpop::Secure::Hash::HASH_SUCCESS;
}
