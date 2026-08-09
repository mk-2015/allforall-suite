# AES-CBC Mode (`acry/modes/cbc.h`)

Cipher Block Chaining mode with PKCS#7 padding, built on [`acry/aes.h`](aes.md).

> **Security notes:**
> - `iv` must be unpredictable and **never reused with the same key** across different messages — reuse breaks confidentiality guarantees.
> - CBC provides confidentiality only, **no integrity/authenticity check**. An attacker who can modify ciphertext can predictably flip bits in the decrypted plaintext. If your application's behavior differs observably between "padding was valid" and "padding was invalid" (e.g. different error messages or timing), that is a **padding oracle** and can lead to full plaintext recovery by a network attacker (see: POODLE, Lucky Thirteen, and similar historical attacks against real systems).
> - **Recommendation**: don't use CBC alone where an attacker can submit or observe responses to modified ciphertext. Pair it with a separate MAC (encrypt-then-MAC), or prefer an authenticated mode once one is available in this library.

## Functions

### `AES_CBC_encrypt`

```c
int AES_CBC_encrypt(const AES_HANDLE *ctx, const acry_u8 iv[AES_BLOCK_SIZE],
                     const acry_u8 *plaintext, size_t len,
                     acry_u8 **out, size_t *out_len);
```

Pads `plaintext` with PKCS#7 and encrypts it in CBC mode. On success returns `0` and writes a newly allocated ciphertext buffer to `*out` (length always a multiple of `AES_BLOCK_SIZE`, written to `*out_len`; caller must `free`). Returns `-1` on allocation failure.

### `AES_CBC_decrypt`

```c
int AES_CBC_decrypt(const AES_HANDLE *ctx, const acry_u8 iv[AES_BLOCK_SIZE],
                     const acry_u8 *ciphertext, size_t len,
                     acry_u8 **out, size_t *out_len);
```

Decrypts `ciphertext` (`len` must be a nonzero multiple of `AES_BLOCK_SIZE`) and strips PKCS#7 padding. On success returns `0` and writes a newly allocated plaintext buffer to `*out` (caller must `free`). Returns `-1` if the length is invalid, on allocation failure, or if the padding is invalid — which happens whenever the ciphertext, key, or IV don't match (including tampering).

## Example Usage

```c
#include <stdio.h>
#include <stdlib.h>
#include <acry/aes.h>
#include <acry/modes/cbc.h>

int main(void) {
    acry_u8 key[16] = { /* ... */ };
    acry_u8 iv[16]  = { /* unique, unpredictable, per message */ };
    const char *msg = "Attack at dawn.";

    AES_HANDLE ctx = AES_HANDLE_open(key, sizeof(key));

    acry_u8 *ct; size_t ct_len;
    AES_CBC_encrypt(&ctx, iv, (const acry_u8*)msg, strlen(msg), &ct, &ct_len);

    acry_u8 *pt; size_t pt_len;
    if (AES_CBC_decrypt(&ctx, iv, ct, ct_len, &pt, &pt_len) != 0) {
        fprintf(stderr, "decryption failed (bad padding / tampering)\n");
        return 1;
    }
    /* pt[0..pt_len) == "Attack at dawn." */

    free(ct);
    free(pt);
    AES_HANDLE_close(ctx);
    return 0;
}
```