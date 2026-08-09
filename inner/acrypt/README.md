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
* [SHA-1](docs/sha1.md) (`docs/sha1.md`): Legacy 160-bit hash function. Deprecated for security use — compatibility only.
* [SHA-224](docs/sha224.md) (`docs/sha224.md`): 224-bit cryptographic hash function (truncated SHA-256).    
* [SHA-256](docs/sha256.md) (`docs/sha256.md`): 256-bit cryptographic hash function.
* [SHA-512](docs/sha512.md) (`docs/sha512.md`): 512-bit cryptographic hash function.
