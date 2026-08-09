# AES Module (`acry/aes.h`)

Implements the AES block cipher (FIPS-197) — key schedule and single-block encrypt/decrypt for AES-128, AES-192, and AES-256. This is the raw 16-byte primitive only; it is **not** a usable encryption scheme by itself. Pair it with a mode from `acry/modes/` ([CBC](cbc.md), [CTR](ctr.md)) to actually encrypt data.

> **Security notes:**
> - This implementation uses table lookups indexed by secret data (S-box substitution) and is **not constant-time**. It may be vulnerable to cache-timing side-channel attacks on shared/hostile hardware. It has not been audited.
> - Never use the raw block primitive directly as "ECB mode" on real data — identical plaintext blocks always produce identical ciphertext blocks, which leaks structure (the classic "ECB penguin" problem).

## Types

```c
#define AES_BLOCK_SIZE 16

typedef struct {
    acry_u32 round_keys[60];
    int num_rounds; // 10 (AES-128), 12 (AES-192), 14 (AES-256); 0 if key_len was invalid
} AES_HANDLE;
```

## Functions

### `AES_HANDLE_open`

```c
AES_HANDLE AES_HANDLE_open(const acry_u8 *key, size_t key_len);
```

Expands `key` into a round-key schedule. `key_len` must be `16`, `24`, or `32` bytes, selecting AES-128, AES-192, or AES-256 respectively. **On an invalid `key_len`, returns a handle with `num_rounds == 0`** — always check this before using the handle.

### `AES_HANDLE_close`

```c
int AES_HANDLE_close(AES_HANDLE ctx);
```

Releases the handle. Returns `0` on success. Note: since `ctx` is passed by value, this cannot zero the caller's copy of the round-key schedule — if you need to guarantee key material is wiped from memory, `memset` your own handle to zero after use.

### `AES_encrypt_block` / `AES_decrypt_block`

```c
void AES_encrypt_block(const AES_HANDLE *ctx, const acry_u8 in[AES_BLOCK_SIZE], acry_u8 out[AES_BLOCK_SIZE]);
void AES_decrypt_block(const AES_HANDLE *ctx, const acry_u8 in[AES_BLOCK_SIZE], acry_u8 out[AES_BLOCK_SIZE]);
```

Encrypts/decrypts exactly one 16-byte block. `in` and `out` may safely point to the same buffer.

## Example Usage

```c
#include <stdio.h>
#include <acry/aes.h>

int main(void) {
    acry_u8 key[16] = { 0x00, 0x01, 0x02, /* ... 16 bytes total ... */ };
    acry_u8 plaintext[16] = { /* ... 16 bytes ... */ };
    acry_u8 ciphertext[16];

    AES_HANDLE ctx = AES_HANDLE_open(key, sizeof(key));
    if (ctx.num_rounds == 0) {
        fprintf(stderr, "invalid key length\n");
        return 1;
    }

    AES_encrypt_block(&ctx, plaintext, ciphertext);

    acry_u8 recovered[16];
    AES_decrypt_block(&ctx, ciphertext, recovered);
    /* recovered == plaintext */

    AES_HANDLE_close(ctx);
    return 0;
}
```

For encrypting real (non-16-byte-exact) data, see [CBC mode](cbc.md) or [CTR mode](ctr.md).