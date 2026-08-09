# Base32 Module (`acry/endec/base32.h`)

RFC 4648 base32 (standard alphabet: `A-Z`, `2-7`, with `=` padding). Commonly used where a case-insensitive, more human-typeable encoding is wanted (e.g. TOTP/2FA secret keys, some DNS-safe identifiers).

## Functions

### `BASE32_encode`

```c
char *BASE32_encode(const acry_u8 *data, size_t len);
```

Encodes `len` bytes of `data` into base32 text. Returns a newly allocated, NUL-terminated string (caller must `free`), or `NULL` on allocation failure.

### `BASE32_decode`

```c
int BASE32_decode(const char *in, size_t in_len, acry_u8 **out_data, size_t *out_len);
```

Decodes base32 text into raw bytes. **Case-insensitive**; whitespace, newlines, and `=` padding are tolerated anywhere in the input. On success writes a malloc'd buffer to `*out_data` and its length to `*out_len` (a zero-length or all-whitespace input decodes to `*out_data = NULL`, `*out_len = 0` — not an error) and returns `0`. Returns `-1` on malformed input (invalid characters or an invalid input length).

## Example Usage

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acry/endec/base32.h>

int main(void) {
    char *enc = BASE32_encode((const acry_u8*)"foobar", 6);
    printf("%s\n", enc); // MZXW6YTBOI======

    acry_u8 *dec; size_t dec_len;
    BASE32_decode(enc, strlen(enc), &dec, &dec_len);
    // dec[0..dec_len) == "foobar"

    free(enc);
    free(dec);
    return 0;
}
```