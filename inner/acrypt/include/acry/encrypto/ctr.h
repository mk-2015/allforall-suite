#pragma once

#include <acry/aes.h>
#include <stddef.h>

int AES_CTR_crypt(const AES_HANDLE *ctx, const acry_u8 nonce_counter[AES_BLOCK_SIZE],
                   const acry_u8 *data, size_t len,
                   acry_u8 **out, size_t *out_len);