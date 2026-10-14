#include <ut/exceptions.h>
#include <ut/types.h>
#include <stdio.h>
#include <stdlib.h>

int except = 0;
exception_handler_t handler = NULL;

void throw_error(const char* exception, const char *message, int code, int unhandleable)
{
    printf("%s: %s\n", exception, message);
    printf("Exception code: %d\n", code);
    except = code;

    if (unhandleable == EXCEPTION_CANNOT_BE_HANDLED) {
        printf("This exception cannot be handled. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    if (handler != NULL) {
        handler(message, unhandleable);
        return;
    }

    printf("Exception cannot be handled because no handler was registered!.\n");
    exit(EXIT_FAILURE);
}

void register_error(exception_handler_t h) {
    if (h == NULL) {
        return;
    }

    handler = h;
}
