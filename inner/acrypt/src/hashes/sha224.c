#include <acry/base.h>
#include <acry/hash/sha224.h>
#include <stdlib.h>
#include <string.h>


static const acry_u32 K256[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

static void sha224_process_block(SHA224_HANDLE *ctx, const acry_u8 block[ACRY_SHA224_BLOCK_SIZE]) {
    acry_u32 w[64];
    for (int i = 0; i < 16; i++) {
        w[i] = ((acry_u32)block[i * 4]     << 24) |
               ((acry_u32)block[i * 4 + 1] << 16) |
               ((acry_u32)block[i * 4 + 2] << 8)  |
               ((acry_u32)block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; i++) {
        w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];
    }

    acry_u32 a = ctx->state[0];
    acry_u32 b = ctx->state[1];
    acry_u32 c = ctx->state[2];
    acry_u32 d = ctx->state[3];
    acry_u32 e = ctx->state[4];
    acry_u32 f = ctx->state[5];
    acry_u32 g = ctx->state[6];
    acry_u32 h = ctx->state[7];

    for (int i = 0; i < 64; i++) {
        acry_u32 t1 = h + EP1(e) + CH(e, f, g) + K256[i] + w[i];
        acry_u32 t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

SHA224_HANDLE SHA224_HANDLE_open(void) {
    SHA224_HANDLE ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.count = 0;
    ctx.state[0] = 0xc1059ed8u;
    ctx.state[1] = 0x367cd507u;
    ctx.state[2] = 0x3070dd17u;
    ctx.state[3] = 0xf70e5939u;
    ctx.state[4] = 0xffc00b31u;
    ctx.state[5] = 0x68581511u;
    ctx.state[6] = 0x64f98fa7u;
    ctx.state[7] = 0xbefa4fa4u;
    return ctx;
}

void SHA224_update(SHA224_HANDLE *ctx, const void *data, size_t len) {
    const acry_u8 *bytes = (const acry_u8 *)data;
    size_t buffer_used = (size_t)(ctx->count % ACRY_SHA224_BLOCK_SIZE);
    ctx->count += (acry_u64)len;

    if (buffer_used > 0) {
        size_t needed = ACRY_SHA224_BLOCK_SIZE - buffer_used;
        size_t take = len < needed ? len : needed;
        memcpy(ctx->buffer + buffer_used, bytes, take);
        bytes += take;
        len -= take;
        buffer_used += take;

        if (buffer_used == ACRY_SHA224_BLOCK_SIZE) {
            sha224_process_block(ctx, ctx->buffer);
            buffer_used = 0;
        }
    }

    while (len >= ACRY_SHA224_BLOCK_SIZE) {
        sha224_process_block(ctx, bytes);
        bytes += ACRY_SHA224_BLOCK_SIZE;
        len -= ACRY_SHA224_BLOCK_SIZE;
    }

    if (len > 0) {
        memcpy(ctx->buffer, bytes, len);
    }
}

void SHA224_final(acry_u8 *digest, SHA224_HANDLE *ctx) {
    acry_u64 bit_len = ctx->count * 8;

    acry_u8 pad = 0x80;
    SHA224_update(ctx, &pad, 1);

    acry_u8 zero = 0x00;
    size_t buffer_used = (size_t)(ctx->count % ACRY_SHA224_BLOCK_SIZE);
    while (buffer_used != 56) {
        SHA224_update(ctx, &zero, 1);
        buffer_used = (size_t)(ctx->count % ACRY_SHA224_BLOCK_SIZE);
    }

    acry_u8 len_bytes[8];
    for (int i = 0; i < 8; i++) {
        len_bytes[i] = (acry_u8)(bit_len >> (56 - i * 8));
    }
    {
        size_t used = (size_t)(ctx->count % ACRY_SHA224_BLOCK_SIZE);
        memcpy(ctx->buffer + used, len_bytes, 8);
        sha224_process_block(ctx, ctx->buffer);
    }

    /* Truncated output: only the first 7 of the 8 state words (28 bytes) */
    for (int i = 0; i < 7; i++) {
        digest[i * 4]     = (acry_u8)(ctx->state[i] >> 24);
        digest[i * 4 + 1] = (acry_u8)(ctx->state[i] >> 16);
        digest[i * 4 + 2] = (acry_u8)(ctx->state[i] >> 8);
        digest[i * 4 + 3] = (acry_u8)(ctx->state[i]);
    }
}

int SHA224_HANDLE_close(SHA224_HANDLE ctx) {
    memset(&ctx, 0, sizeof(ctx));
    return 0;
}

acry_u8 *SHA224_bytes_NO_HANDLE(const void *data, size_t len) {
    acry_u8 *digest = (acry_u8 *)malloc(ACRY_SHA224_DIGEST_SIZE);
    if (!digest) return NULL;

    SHA224_HANDLE ctx = SHA224_HANDLE_open();
    SHA224_update(&ctx, data, len);
    SHA224_final(digest, &ctx);
    SHA224_HANDLE_close(ctx);

    return digest;
}