#pragma once

#include <acry/encrypto/aes.h>
#include <stddef.h>

int AES_CBC_encrypt(const AES_HANDLE *ctx, const acry_u8 iv[AES_BLOCK_SIZE],
                     const acry_u8 *plaintext, size_t len,
                     acry_u8 **out, size_t *out_len);
int AES_CBC_decrypt(const AES_HANDLE *ctx, const acry_u8 iv[AES_BLOCK_SIZE],
                     const acry_u8 *ciphertext, size_t len,
                     acry_u8 **out, size_t *out_len);