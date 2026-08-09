#pragma once

#include <acry/base.h>
#include <stddef.h>

SHA512_HANDLE SHA512_HANDLE_open(void);
void SHA512_update(SHA512_HANDLE *ctx, const void *data, size_t len);
void SHA512_final(acry_u8 *digest, SHA512_HANDLE *ctx);
int SHA512_HANDLE_close(SHA512_HANDLE ctx);
acry_u8* SHA512_bytes_NO_HANDLE(const void *data, size_t len);