#pragma once

#include <stdbool.h>
#include <ut/types.h>

extern int except;

#define EXCEPTION_CAN_BE_HANDLED    false
#define EXCEPTION_CANNOT_BE_HANDLED true

typedef bool handleable_exception_t;
typedef void (*exception_handler_t)(const char *message, handleable_exception_t code);

void throw_error(const char* exception, const char *message, int code, int unhandleable);
void register_error(exception_handler_t handler);
