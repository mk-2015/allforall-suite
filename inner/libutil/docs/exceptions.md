# Exception Handler API

## Overview

The exception handler provides a simple global error-reporting mechanism for library code and application code. It stores a single registered callback and invokes it when an exception is thrown that is considered handleable.

## API

- `extern int except;`
- `#define EXCEPTION_CAN_BE_HANDLED false`
- `#define EXCEPTION_CANNOT_BE_HANDLED true`
- `typedef bool handleable_exception_t;`
- `typedef void (*exception_handler_t)(const char *message, handleable_exception_t code);`
- `void throw_error(const char *exception, const char *message, int code, int unhandleable);`
- `void register_error(exception_handler_t handler);`

## Behavior

`throw_error` prints the exception name, the message, and the numeric error code. It also stores the code in `except`.

If `unhandleable` equals `EXCEPTION_CANNOT_BE_HANDLED`, the application exits immediately with `EXIT_FAILURE` after printing the failure details.

If `unhandleable` equals `EXCEPTION_CAN_BE_HANDLED`, the handler is invoked if one has been registered. If no handler exists, the program exits with `EXIT_FAILURE`.

## Example

```c
#include <ut/exceptions.h>

static void on_error(const char *message, handleable_exception_t code) {
    (void)code;
    printf("Handled: %s\n", message);
}

int main(void) {
    register_error(on_error);
    throw_error("FileError", "Unable to open config", 42, EXCEPTION_CAN_BE_HANDLED);
    return 0;
}
```

## Notes

- `register_error` ignores `NULL` handlers.
- `except` is updated with the last thrown exception code.
- The default handler model is intentionally minimal and global to keep the API lightweight.
