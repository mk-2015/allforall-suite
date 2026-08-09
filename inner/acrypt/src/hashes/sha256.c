#include <acry/hash/sha256.h>
#include <stdlib.h>
#include <string.h>

static const acry_u32 k[64] = {
   0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
   0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
   0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
   0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
   0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
   0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
   0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
   0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

SHA256_HANDLE SHA256_HANDLE_open(void) {
    SHA256_HANDLE ctx;
    ctx.count = 0;
    ctx.state[0] = 0x6a09e667;
    ctx.state[1] = 0xbb67ae85;
    ctx.state[2] = 0x3c6ef372;
    ctx.state[3] = 0xa54ff53a;
    ctx.state[4] = 0x510e527f;
    ctx.state[5] = 0x9b05688c;
    ctx.state[6] = 0x1f83d9ab;
    ctx.state[7] = 0x5be0cd19;
    return ctx;
}

static void sha256_transform(SHA256_HANDLE *ctx, const acry_u8 data[]) {
    acry_u32 a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = ((acry_u32)data[j] << 24) | ((acry_u32)data[j + 1] << 16) |
               ((acry_u32)data[j + 2] << 8) | ((acry_u32)data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
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

void SHA256_update(SHA256_HANDLE *ctx, const void *data, size_t len) {
    const acry_u8 *bytes = (const acry_u8*)data;
    size_t i, index, part_len;

    index = (size_t)((ctx->count >> 3) & 0x3F);
    ctx->count += (acry_u64)len << 3;
    part_len = 64 - index;

    if (len >= part_len) {
        memcpy(&ctx->buffer[index], bytes, part_len);
        sha256_transform(ctx, ctx->buffer);
        for (i = part_len; i + 63 < len; i += 64) {
            sha256_transform(ctx, &bytes[i]);
        }
        index = 0;
    } else {
        i = 0;
    }

    memcpy(&ctx->buffer[index], &bytes[i], len - i);
}

void SHA256_final(acry_u8 *digest, SHA256_HANDLE *ctx) {
    acry_u8 pad[64];
    acry_u64 saved_count = ctx->count;
    size_t index = (size_t)((ctx->count >> 3) & 0x3F);
    size_t pad_len = (index < 56) ? (56 - index) : (120 - index);

    pad[0] = 0x80;
    memset(pad + 1, 0, 63);
    SHA256_update(ctx, pad, pad_len);

    acry_u8 bits[8];
    for (int i = 7; i >= 0; --i) {
        bits[i] = (acry_u8)(saved_count & 0xFF);
        saved_count >>= 8;
    }
    SHA256_update(ctx, bits, 8);

    for (int i = 0; i < 4; ++i) {
        digest[i]      = (acry_u8)(ctx->state[0] >> (24 - i * 8));
        digest[i + 4]  = (acry_u8)(ctx->state[1] >> (24 - i * 8));
        digest[i + 8]  = (acry_u8)(ctx->state[2] >> (24 - i * 8));
        digest[i + 12] = (acry_u8)(ctx->state[3] >> (24 - i * 8));
        digest[i + 16] = (acry_u8)(ctx->state[4] >> (24 - i * 8));
        digest[i + 20] = (acry_u8)(ctx->state[5] >> (24 - i * 8));
        digest[i + 24] = (acry_u8)(ctx->state[6] >> (24 - i * 8));
        digest[i + 28] = (acry_u8)(ctx->state[7] >> (24 - i * 8));
    }
}

int SHA256_HANDLE_close(SHA256_HANDLE ctx) {
    (void)ctx;
    return 0;
}

acry_u8* SHA256_bytes_NO_HANDLE(const void *data, size_t len) {
    acry_u8 *digest = malloc(ACRY_SHA256_DIGEST_SIZE);
    if (!digest) return NULL;

    SHA256_HANDLE ctx = SHA256_HANDLE_open();
    SHA256_update(&ctx, data, len);
    SHA256_final(digest, &ctx);
    SHA256_HANDLE_close(ctx);

    return digest;
}