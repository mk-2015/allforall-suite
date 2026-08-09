#pragma once

#include <acry/base.h>
#include <stddef.h>

SHA1_HANDLE SHA1_HANDLE_open(void);
void SHA1_update(SHA1_HANDLE *ctx, const void *data, size_t len);
void SHA1_final(acry_u8 *digest, SHA1_HANDLE *ctx);
int SHA1_HANDLE_close(SHA1_HANDLE ctx);
acry_u8* SHA1_bytes_NO_HANDLE(const void *data, size_t len);