# SHA-1 Module (`acry/hash/sha1.h`)

Implements the SHA-1 cryptographic hash function (FIPS 180-4), producing a 160-bit (20-byte) digest. Supports both stateful, streaming handles and quick one-shot functions.

> **Security note:** SHA-1 is cryptographically broken — practical collision attacks exist. It's provided here for compatibility with legacy formats/protocols only. Prefer `SHA256` or `SHA512` for anything security-sensitive.

## Functions

### `SHA1_HANDLE_open`

```c
SHA1_HANDLE SHA1_HANDLE_open(void);
```

Initializes a new SHA-1 context with the standard initial hash state.

### `SHA1_update`

```c
void SHA1_update(SHA1_HANDLE *ctx, const void *data, size_t len);
```

Feeds `len` bytes of `data` into the hash. Can be called any number of times with arbitrarily sized chunks — internally buffers partial blocks, so streaming input (e.g. reading a file incrementally) works correctly.

### `SHA1_final`

```c
void SHA1_final(acry_u8 *digest, SHA1_HANDLE *ctx);
```

Finalizes the hash (applies padding and the length suffix) and writes the `ACRY_SHA1_DIGEST_SIZE` (20-byte) digest into the caller-supplied `digest` buffer. After calling this, the context should not be reused for further `SHA1_update` calls.

### `SHA1_HANDLE_close`

```c
int SHA1_HANDLE_close(SHA1_HANDLE ctx);
```

Releases the handle. Returns `0` on success.

### `SHA1_bytes_NO_HANDLE`

```c
acry_u8* SHA1_bytes_NO_HANDLE(const void *data, size_t len);
```

One-shot hashing without managing handle lifecycle. Returns a newly allocated `ACRY_SHA1_DIGEST_SIZE`-byte buffer (caller must `free`), or `NULL` on allocation failure.

## Example Usage

```c
#include <stdio.h>
#include <stdlib.h>
#include <acry/hash/sha1.h>

int main(void) {
    // One-shot
    acry_u8 *digest = SHA1_bytes_NO_HANDLE("abc", 3);
    for (size_t i = 0; i < ACRY_SHA1_DIGEST_SIZE; i++) printf("%02x", digest[i]);
    printf("\n"); // a9993e364706816aba3e25717850c26c9cd0d89
    free(digest);

    // Streaming
    SHA1_HANDLE ctx = SHA1_HANDLE_open();
    SHA1_update(&ctx, "ab", 2);
    SHA1_update(&ctx, "c", 1);
    acry_u8 out[ACRY_SHA1_DIGEST_SIZE];
    SHA1_final(out, &ctx);
    SHA1_HANDLE_close(ctx);

    return 0;
}
```