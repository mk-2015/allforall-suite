# Random Module (`acry/random.h`)

The random module hooks directly into the host operating system's cryptographic entropy sources (`CryptGenRandom` on Windows, `/dev/urandom` on Unix-like systems) to generate secure random data.

## Functions

### `RANDOM_OPEN_handle`

```c
RANDOM_HANDLE RANDOM_OPEN_handle(void);
```

Acquires an OS-level entropy context and returns an initialized `RANDOM_HANDLE`.

### `RANDOM_LOAD_bytes`

```c
void RANDOM_LOAD_bytes(size_t bytes_count, RANDOM_HANDLE *random);
```

Fills or refreshes the internal buffer of the handle with `bytes_count` secure random bytes.

### `RANDOM_GET_BYTES`

```c
void* RANDOM_GET_BYTES(RANDOM_HANDLE random);
```

Retrieves the internal raw byte pointer from the handle.

### `RANDOM_CLOSE_handle`

```c
int RANDOM_CLOSE_handle(RANDOM_HANDLE random);
```

Releases entropy contexts and frees internal buffers.

### `RANDOM_GET_bytes_NO_HANDLE`

```c
void* RANDOM_GET_bytes_NO_HANDLE(size_t size);
```

A convenient one-shot allocator that fetches `size` random bytes directly without managing handles.

## Example Usage

```c
#include <stdio.h>
#include <stdlib.h>
#include <acry/random.h>

int main(void) {
    // One-shot secure random allocation
    unsigned char *key = (unsigned char*)RANDOM_GET_bytes_NO_HANDLE(32);
    if (key) {
        // Use your 32 random bytes here...
        free(key);
    }
    return 0;
}
```