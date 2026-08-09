#pragma once

#include <acry/base.h>
#include <stddef.h>

SHA224_HANDLE SHA224_HANDLE_open(void);
void SHA224_update(SHA224_HANDLE *ctx, const void *data, size_t len);
void SHA224_final(acry_u8 *digest, SHA224_HANDLE *ctx);
int SHA224_HANDLE_close(SHA224_HANDLE ctx);

acry_u8* SHA224_bytes_NO_HANDLE(const void *data, size_t len);