#pragma once

#include <acry/base.h>
#include <stddef.h>

AES_HANDLE AES_HANDLE_open(const acry_u8 *key, size_t key_len);
int AES_HANDLE_close(AES_HANDLE ctx);
void AES_encrypt_block(const AES_HANDLE *ctx, const acry_u8 in[AES_BLOCK_SIZE], acry_u8 out[AES_BLOCK_SIZE]);
void AES_decrypt_block(const AES_HANDLE *ctx, const acry_u8 in[AES_BLOCK_SIZE], acry_u8 out[AES_BLOCK_SIZE]);