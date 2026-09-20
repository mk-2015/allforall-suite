#include "int.hpp"
#include <cstring>

static inline uint32_t sha2_rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

struct SHA256Context {
    uint32_t state[8];
    uint64_t count;
    uint8_t  buffer[64];
};

static constexpr uint32_t SHA2_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef4a3f7, 0xc67178f2
};

static void sha2_256_init(SHA256Context* ctx) {
    ctx->count = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

static void sha2_256_transform(uint32_t state[8], const uint8_t block[64]) {
    uint32_t W[64];
    for (int i = 0; i < 16; ++i) {
        W[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
               (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
               (static_cast<uint32_t>(block[i * 4 + 3]));
    }
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = sha2_rotr(W[i - 15], 7) ^ sha2_rotr(W[i - 15], 18) ^ (W[i - 15] >> 3);
        uint32_t s1 = sha2_rotr(W[i - 2], 17) ^ sha2_rotr(W[i - 2], 19) ^ (W[i - 2] >> 10);
        W[i] = W[i - 16] + s0 + W[i - 7] + s1;
    }

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = sha2_rotr(e, 6) ^ sha2_rotr(e, 11) ^ sha2_rotr(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + S1 + ch + SHA2_K[i] + W[i];
        uint32_t S0 = sha2_rotr(a, 2) ^ sha2_rotr(a, 13) ^ sha2_rotr(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;

        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

static void sha2_256_update(SHA256Context* ctx, const uint8_t* input, std::size_t len) {
    std::size_t index = static_cast<std::size_t>((ctx->count >> 3) & 0x3F);
    ctx->count += static_cast<uint64_t>(len) << 3;
    std::size_t part_len = 64 - index;

    std::size_t i = 0;
    if (len >= part_len) {
        std::memcpy(&ctx->buffer[index], input, part_len);
        sha2_256_transform(ctx->state, ctx->buffer);
        for (i = part_len; i + 63 < len; i += 64) {
            sha2_256_transform(ctx->state, &input[i]);
        }
        index = 0;
    }
    if (i < len) {
        std::memcpy(&ctx->buffer[index], &input[i], len - i);
    }
}

static void sha2_256_final(SHA256Context* ctx, uint8_t digest[32]) {
    uint8_t bits[8];
    for (int i = 0; i < 8; ++i) {
        bits[i] = static_cast<uint8_t>((ctx->count >> ((7 - i) * 8)) & 0xFF);
    }

    std::size_t index = static_cast<std::size_t>((ctx->count >> 3) & 0x3F);
    std::size_t pad_len = (index < 56) ? (56 - index) : (120 - index);

    static constexpr uint8_t PADDING[64] = { 0x80 };
    sha2_256_update(ctx, PADDING, pad_len);
    sha2_256_update(ctx, bits, 8);

    for (int i = 0; i < 8; ++i) {
        digest[i * 4]     = static_cast<uint8_t>((ctx->state[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((ctx->state[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((ctx->state[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>(ctx->state[i] & 0xFF);
    }
}

extern "C" unsigned long long sha2_256_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s)
{
    if (outhash == nullptr || outhash_s < 32) return 4;
    if (indata == nullptr && indata_s > 0) return 3;

    SHA256Context ctx;
    sha2_256_init(&ctx);

    if (indata && indata_s > 0) sha2_256_update(&ctx, indata, indata_s);
    if (salt && salt_s > 0)     sha2_256_update(&ctx, salt, salt_s);

    sha2_256_final(&ctx, outhash);
    return 0;
}

static inline uint64_t sha3_rotl64(uint64_t x, int shift) {
    return (x << shift) | (x >> (64 - shift));
}

struct SHA3Context {
    uint64_t state[25];
    uint8_t  buffer[136];
    std::size_t buf_len;
};

static constexpr uint64_t KeccakF_RoundConstants[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL, 0x8000000080008000ULL,
    0x000000000000808bULL, 0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
    0x000000000000008aULL, 0x0000000000000088ULL, 0x000000008000809eULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
    0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800aULL, 0x800000008000000aULL,
    0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

static constexpr int KeccakF_RotationConstants[25] = {
     0,  1, 62, 28, 27,
    36, 44,  6, 55, 20,
     3, 10, 43, 25, 39,
    18, 39, 61, 14, 19,
    45,  2, 29, 30, 23
};

static constexpr int KeccakF_PiLane[25] = {
    0, 10, 20,  5, 15,
   16,  1, 11, 21,  6,
    7, 17,  2, 12, 22,
   23,  8, 18,  3, 13,
   14, 24,  9, 19,  4
};

static void keccakf1600(uint64_t state[25]) {
    for (int round = 0; round < 24; ++round) {
        uint64_t C[5], D[5];
        for (int i = 0; i < 5; ++i) C[i] = state[i] ^ state[i + 5] ^ state[i + 10] ^ state[i + 15] ^ state[i + 20];
        for (int i = 0; i < 5; ++i) D[i] = C[(i + 4) % 5] ^ sha3_rotl64(C[(i + 1) % 5], 1);
        for (int i = 0; i < 25; ++i) state[i] ^= D[i % 5];

        uint64_t B[25];
        for (int i = 0; i < 25; ++i) B[KeccakF_PiLane[i]] = sha3_rotl64(state[i], KeccakF_RotationConstants[i]);

        for (int i = 0; i < 25; i += 5) {
            state[i]     = B[i]     ^ ((~B[i + 1]) & B[i + 2]);
            state[i + 1] = B[i + 1] ^ ((~B[i + 2]) & B[i + 3]);
            state[i + 2] = B[i + 2] ^ ((~B[i + 3]) & B[i + 4]);
            state[i + 3] = B[i + 3] ^ ((~B[i + 4]) & B[i]);
            state[i + 4] = B[i + 4] ^ ((~B[i])     & B[i + 1]);
        }
        state[0] ^= KeccakF_RoundConstants[round];
    }
}

static void sha3_256_init(SHA3Context* ctx) {
    std::memset(ctx->state, 0, sizeof(ctx->state));
    std::memset(ctx->buffer, 0, sizeof(ctx->buffer));
    ctx->buf_len = 0;
}

static void sha3_256_update(SHA3Context* ctx, const uint8_t* input, std::size_t len) {
    for (std::size_t i = 0; i < len; ++i) {
        ctx->buffer[ctx->buf_len++] = input[i];
        if (ctx->buf_len == 136) {
            for (int j = 0; j < 17; ++j) {
                uint64_t lane = 0;
                for (int k = 0; k < 8; ++k) {
                    lane |= static_cast<uint64_t>(ctx->buffer[j * 8 + k]) << (k * 8);
                }
                ctx->state[j] ^= lane;
            }
            keccakf1600(ctx->state);
            ctx->buf_len = 0;
        }
    }
}

static void sha3_256_final(SHA3Context* ctx, uint8_t digest[32]) {
    ctx->buffer[ctx->buf_len++] ^= 0x06;
    ctx->buffer[135] ^= 0x80;

    for (int j = 0; j < 17; ++j) {
        uint64_t lane = 0;
        for (int k = 0; k < 8; ++k) {
            lane |= static_cast<uint64_t>(ctx->buffer[j * 8 + k]) << (k * 8);
        }
        ctx->state[j] ^= lane;
    }
    keccakf1600(ctx->state);

    for (int i = 0; i < 4; ++i) {
        for (int k = 0; k < 8; ++k) {
            digest[i * 8 + k] = static_cast<uint8_t>((ctx->state[i] >> (k * 8)) & 0xFF);
        }
    }
}

extern "C" unsigned long long sha3_256_hash(
    const uint8_t* indata,  std::size_t indata_s,
    const uint8_t* salt,    std::size_t salt_s,
    uint8_t*       outhash, std::size_t outhash_s)
{
    if (outhash == nullptr || outhash_s < 32) return 4;
    if (indata == nullptr && indata_s > 0) return 3;

    SHA3Context ctx;
    sha3_256_init(&ctx);

    if (indata && indata_s > 0) sha3_256_update(&ctx, indata, indata_s);
    if (salt && salt_s > 0)     sha3_256_update(&ctx, salt, salt_s);

    sha3_256_final(&ctx, outhash);
    return 0;
}
