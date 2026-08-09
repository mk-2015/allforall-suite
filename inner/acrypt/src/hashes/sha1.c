#include <acry/base.h>
#include <acry/hash/sha1.h>
#include <stdlib.h>
#include <string.h>

#define ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static void sha1_process_block(SHA1_HANDLE *ctx, const acry_u8 block[ACRY_SHA1_BLOCK_SIZE]) {
    acry_u32 w[80];
    for (int i = 0; i < 16; i++) {
        w[i] = ((acry_u32)block[i * 4]     << 24) |
               ((acry_u32)block[i * 4 + 1] << 16) |
               ((acry_u32)block[i * 4 + 2] << 8)  |
               ((acry_u32)block[i * 4 + 3]);
    }
    for (int i = 16; i < 80; i++) {
        w[i] = ROTL32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    acry_u32 a = ctx->state[0];
    acry_u32 b = ctx->state[1];
    acry_u32 c = ctx->state[2];
    acry_u32 d = ctx->state[3];
    acry_u32 e = ctx->state[4];

    for (int i = 0; i < 80; i++) {
        acry_u32 f, k;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999u;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1u;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDCu;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6u;
        }

        acry_u32 temp = ROTL32(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = temp;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
}

SHA1_HANDLE SHA1_HANDLE_open(void) {
    SHA1_HANDLE ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.count = 0;
    ctx.state[0] = 0x67452301u;
    ctx.state[1] = 0xEFCDAB89u;
    ctx.state[2] = 0x98BADCFEu;
    ctx.state[3] = 0x10325476u;
    ctx.state[4] = 0xC3D2E1F0u;
    return ctx;
}

void SHA1_update(SHA1_HANDLE *ctx, const void *data, size_t len) {
    const acry_u8 *bytes = (const acry_u8 *)data;
    size_t buffer_used = (size_t)(ctx->count % ACRY_SHA1_BLOCK_SIZE);
    ctx->count += (acry_u64)len;

    if (buffer_used > 0) {
        size_t needed = ACRY_SHA1_BLOCK_SIZE - buffer_used;
        size_t take = len < needed ? len : needed;
        memcpy(ctx->buffer + buffer_used, bytes, take);
        bytes += take;
        len -= take;
        buffer_used += take;

        if (buffer_used == ACRY_SHA1_BLOCK_SIZE) {
            sha1_process_block(ctx, ctx->buffer);
            buffer_used = 0;
        }
    }

    while (len >= ACRY_SHA1_BLOCK_SIZE) {
        sha1_process_block(ctx, bytes);
        bytes += ACRY_SHA1_BLOCK_SIZE;
        len -= ACRY_SHA1_BLOCK_SIZE;
    }

    if (len > 0) {
        memcpy(ctx->buffer, bytes, len);
    }
}

void SHA1_final(acry_u8 *digest, SHA1_HANDLE *ctx) {
    acry_u64 bit_len = ctx->count * 8;
    size_t buffer_used = (size_t)(ctx->count % ACRY_SHA1_BLOCK_SIZE);

    acry_u8 pad = 0x80;
    SHA1_update(ctx, &pad, 1);

    acry_u8 zero = 0x00;
    /* recompute buffer_used since SHA1_update above changed ctx->count */
    buffer_used = (size_t)(ctx->count % ACRY_SHA1_BLOCK_SIZE);
    while (buffer_used != 56) {
        SHA1_update(ctx, &zero, 1);
        buffer_used = (size_t)(ctx->count % ACRY_SHA1_BLOCK_SIZE);
    }

    acry_u8 len_bytes[8];
    for (int i = 0; i < 8; i++) {
        len_bytes[i] = (acry_u8)(bit_len >> (56 - i * 8));
    }
    /* bypass count update side-effects: append length directly via raw processing */
    {
        size_t used = (size_t)(ctx->count % ACRY_SHA1_BLOCK_SIZE);
        memcpy(ctx->buffer + used, len_bytes, 8);
        sha1_process_block(ctx, ctx->buffer);
    }

    for (int i = 0; i < 5; i++) {
        digest[i * 4]     = (acry_u8)(ctx->state[i] >> 24);
        digest[i * 4 + 1] = (acry_u8)(ctx->state[i] >> 16);
        digest[i * 4 + 2] = (acry_u8)(ctx->state[i] >> 8);
        digest[i * 4 + 3] = (acry_u8)(ctx->state[i]);
    }
}

int SHA1_HANDLE_close(SHA1_HANDLE ctx) {
    memset(&ctx, 0, sizeof(ctx));
    return 0;
}

acry_u8 *SHA1_bytes_NO_HANDLE(const void *data, size_t len) {
    acry_u8 *digest = (acry_u8 *)malloc(ACRY_SHA1_DIGEST_SIZE);
    if (!digest) return NULL;

    SHA1_HANDLE ctx = SHA1_HANDLE_open();
    SHA1_update(&ctx, data, len);
    SHA1_final(digest, &ctx);
    SHA1_HANDLE_close(ctx);

    return digest;
}