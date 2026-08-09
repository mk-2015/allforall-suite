# Rotation Module (`acry/rot.h`)

The rotation module provides flexible shift ciphers for both alphabetic characters and numbers, supporting both stateful handles and quick one-shot functions.

## Functions

### `ROTATION_HANDLE_open`

```c
ROTATION_HANDLE ROTATION_HANDLE_open(ssize_t rotamount);
```

Initializes a rotation handle with a specified shift amount.

### `ROTATION_string`

```c
char* ROTATION_string(ROTATION_HANDLE rot, char* string);
```

Applies an alphabetic Caesar shift (A-Z, a-z) based on the handle's rotation amount. Returns a newly allocated string (caller must `free`).

### `ROTATION_string_numerical`

```c
char* ROTATION_string_numerical(ROTATION_HANDLE rot, char* string);
```

Shifts digits (`0`-`9`) within the string based on the rotation amount (modulo 10). Returns a newly allocated string.

### `ROTATION_string_NO_HANDLE`

```c
char* ROTATION_string_NO_HANDLE(ssize_t rotamount, char* string);
```

One-shot alphabetic rotation without needing explicit handle lifecycle management.

### `ROTATION_string_numerical_NO_HANDLE`

```c
char* ROTATION_string_numerical_NO_HANDLE(ssize_t rotamount, char* string);
```

One-shot numerical digit rotation.

## Example Usage

```c
#include <stdio.h>
#include <stdlib.h>
#include <acry/rot.h>

int main(void) {
    ROTATION_HANDLE rot = ROTATION_HANDLE_open(13);

    char *alpha = ROTATION_string(rot, "mystr12345");
    char *numeric = ROTATION_string_numerical(rot, "mystr12345");

    printf("Alpha rotated: %s\n", alpha);     // zlfge12345
    printf("Numeric rotated: %s\n", numeric); // mystr23456

    free(alpha);
    free(numeric);
    ROTATION_HANDLE_close(rot);
    return 0;
}
```