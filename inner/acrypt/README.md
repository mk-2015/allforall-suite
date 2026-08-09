# Acrypt (acry)

A lightweight, high-performance cryptographic and utility library written in C, featuring secure random generation, bit/byte rotations, and classic ciphers.

---

## Installation & Setup

Include the headers and link against the library:

```c
#include <acry/base.h>
#include <acry/random.h>
```

## Modules

* [Random Generation](docs/random.md) (`docs/random.md`): Secure OS-level entropy gathering via handles and one-shot functions.
* [Rotation Ciphers](docs/rot.md) (`docs/rot.md`): Alphabetic and numerical string rotation utilities with handle-based management.
* [SHA-1](docs/hash/sha1.md) (`docs/sha1.md`): Legacy 160-bit hash function. Deprecated for security use — compatibility only.
* [SHA-224](docs/hash/sha224.md) (`docs/sha224.md`): 224-bit cryptographic hash function (truncated SHA-256).    
* [SHA-256](docs/hash/sha256.md) (`docs/sha256.md`): 256-bit cryptographic hash function.
* [SHA-512](docs/hash/sha512.md) (`docs/sha512.md`): 512-bit cryptographic hash function.
* [PEM](docs/pem.md) (`docs/pem.md`): PEM container parsing and serialization (certs, keys, multi-block chains).
* [Base64](docs/endec/base64.md) (`docs/endec/base64.md`): RFC 4648 base64 encode/decode.
* [Base32](docs/endec/base32.md) (`docs/endec/base32.md`): RFC 4648 base32 encode/decode.
* [AES](docs/encrypto/aes.md) (`docs/encrypto/aes.md`): AES-128/192/256 block cipher core (key schedule + single-block encrypt/decrypt).
* [AES-CBC](docs/encrypto/cbc.md) (`docs/encrypto/cbc.md`): CBC mode with PKCS#7 padding, built on AES.
* [AES-CTR](docs/encrypto/ctr.md) (`docs/encrypto/ctr.md`): CTR mode (stream-cipher style), built on AES.
