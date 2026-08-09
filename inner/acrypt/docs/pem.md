# PEM Module (`acry/pem.h`)

PEM (RFC 7468) is a text container format: base64-encoded binary data (typically DER) wrapped between `-----BEGIN <label>-----` and `-----END <label>-----` marker lines. Certificates, private keys, public keys, and CSRs are all conventionally stored this way. A single file can contain several concatenated blocks — e.g. a leaf certificate followed by intermediate certificates in a chain.

This module is not handle-based like the hash/random/rotation modules — PEM documents are parsed and produced in one shot rather than streamed.

## Types

```c
typedef struct {
    char *label;        // e.g. "CERTIFICATE", "RSA PRIVATE KEY", "PUBLIC KEY"
    acry_u8 *data;       // decoded (DER) bytes
    size_t data_len;
} PEM_BLOCK;

typedef struct {
    PEM_BLOCK *blocks;
    size_t count;
} PEM_DOCUMENT;
```

## Functions

### `PEM_parse`

```c
PEM_DOCUMENT PEM_parse(const char *text, size_t len);
```

Parses one or more PEM blocks out of a buffer. Accepts both LF and CRLF line endings, and tolerates surrounding non-PEM text (e.g. comments before/after the marker lines). Supports multiple concatenated blocks in a single buffer.

On success, returns a document with `count >= 1`. On any parse error — malformed markers, a `BEGIN`/`END` label mismatch, or invalid base64 — the **entire parse fails**: returns `{ .blocks = NULL, .count = 0 }` rather than a partial result.

### `PEM_DOCUMENT_free`

```c
void PEM_DOCUMENT_free(PEM_DOCUMENT *doc);
```

Frees everything owned by the document — each block's `label` and `data`, and the block array itself. Safe to call on an already-empty (`count == 0`) document.

### `PEM_encode`

```c
char *PEM_encode(const char *label, const acry_u8 *data, size_t data_len);
```

Encodes a single binary blob into PEM text, base64-wrapped at 64 characters per line (standard convention). Returns a newly allocated, NUL-terminated string (caller must `free`), or `NULL` on allocation failure.

### `PEM_encode_multi`

```c
char *PEM_encode_multi(const PEM_BLOCK *blocks, size_t count);
```

Encodes several blocks into one concatenated PEM text — e.g. writing out a certificate chain. Returns a newly allocated, NUL-terminated string (caller must `free`), or `NULL` on failure.

## Example Usage

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acry/pem.h>

int main(void) {
    acry_u8 der[] = { 0x30, 0x82, 0x01, 0x0a /* ...DER bytes... */ };

    // Encode
    char *pem_text = PEM_encode("RSA PRIVATE KEY", der, sizeof(der));
    printf("%s", pem_text);

    // Parse it back
    PEM_DOCUMENT doc = PEM_parse(pem_text, strlen(pem_text));
    if (doc.count == 0) {
        fprintf(stderr, "invalid PEM\n");
    } else {
        printf("label: %s, %zu bytes\n", doc.blocks[0].label, doc.blocks[0].data_len);
    }

    PEM_DOCUMENT_free(&doc);
    free(pem_text);
    return 0;
}
```

### Multi-block (certificate chain)

```c
PEM_BLOCK chain[2] = {
    { "CERTIFICATE", leaf_der,  leaf_der_len },
    { "CERTIFICATE", inter_der, inter_der_len },
};
char *chain_pem = PEM_encode_multi(chain, 2);
// chain_pem now holds both blocks concatenated, ready to write to a .pem file

PEM_DOCUMENT parsed = PEM_parse(chain_pem, strlen(chain_pem));
// parsed.count == 2
```

## Notes

- **Not yet implemented**: encrypted private key headers (`Proc-Type: 4,ENCRYPTED` / `DEK-Info`), which some legacy PEM-encoded keys use for passphrase protection. Currently only plain (unencrypted) PEM bodies are supported.
- **DER interpretation is out of scope here**: this module only handles the PEM text container (base64 + markers). Actually parsing the ASN.1/DER structure inside (e.g. pulling an RSA modulus and exponent out of a `RSA PRIVATE KEY` block) is a separate concern for a future ASN.1/DER module.