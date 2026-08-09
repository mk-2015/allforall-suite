# Base64 Module (`acry/endec/base64.h`)

RFC 4648 base64 (standard alphabet: `A-Z`, `a-z`, `0-9`, `+`, `/`, with `=` padding). Used internally by the [PEM module](pem.md), and available standalone.

## Functions

### `BASE64_encode`

```c
char *BASE64_encode(const acry_u8 *data, size_t len);
```

Encodes `len` bytes of `data` into base64 text, unwrapped (no line breaks). Returns a newly allocated, NUL-terminated string (caller must `free`), or `NULL` on allocation failure.

### `BASE64_decode`

```c
int BASE64_decode(const char *in, size_t in_len, acry_u8 **out_data, size_t *out_len);
```

Decodes base64 text into raw bytes. Whitespace, newlines, and `=` padding are tolerated anywhere in the input. On success writes a malloc'd buffer to `*out_data` and its length to `*out_len` (a zero-length or all-whitespace input decodes to `*out_data = NULL`, `*out_len = 0` — not an error) and returns `0`. Returns `-1` on malformed input (invalid characters or an invalid input length).

## Example Usage

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acry/endec/base64.h>

int main(void) {
    char *enc = BASE64_encode((const acry_u8*)"foobar", 6);
    printf("%s\n", enc); // Zm9vYmFy

    acry_u8 *dec; size_t dec_len;
    BASE64_decode(enc, strlen(enc), &dec, &dec_len);
    // dec[0..dec_len) == "foobar"

    free(enc);
    free(dec);
    return 0;
}
```