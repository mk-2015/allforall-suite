#pragma once

#include <acry/base.h>
#include <stddef.h>

SHA256_HANDLE SHA256_HANDLE_open(void);
void SHA256_update(SHA256_HANDLE *ctx, const void *data, size_t len);
void SHA256_final(acry_u8 *digest, SHA256_HANDLE *ctx);
int SHA256_HANDLE_close(SHA256_HANDLE ctx);

acry_u8* SHA256_bytes_NO_HANDLE(const void *data, size_t len);