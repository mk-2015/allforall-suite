#include <acry/base.h>
#include <acry/modes/cbc.h>
#include <stdlib.h>
#include <string.h>

int AES_CBC_encrypt(const AES_HANDLE *ctx, const acry_u8 iv[AES_BLOCK_SIZE],
                     const acry_u8 *plaintext, size_t len,
                     acry_u8 **out, size_t *out_len) {
    size_t pad_value = AES_BLOCK_SIZE - (len % AES_BLOCK_SIZE);
    size_t padded_len = len + pad_value;

    acry_u8 *padded = (acry_u8 *)malloc(padded_len);
    if (!padded) return -1;
    memcpy(padded, plaintext, len);
    memset(padded + len, (int)pad_value, pad_value);

    acry_u8 *ciphertext = (acry_u8 *)malloc(padded_len);
    if (!ciphertext) {
        free(padded);
        return -1;
    }

    acry_u8 prev[AES_BLOCK_SIZE];
    memcpy(prev, iv, AES_BLOCK_SIZE);

    for (size_t i = 0; i < padded_len; i += AES_BLOCK_SIZE) {
        acry_u8 block[AES_BLOCK_SIZE];
        for (int j = 0; j < AES_BLOCK_SIZE; j++) {
            block[j] = (acry_u8)(padded[i + j] ^ prev[j]);
        }
        AES_encrypt_block(ctx, block, ciphertext + i);
        memcpy(prev, ciphertext + i, AES_BLOCK_SIZE);
    }

    free(padded);
    *out = ciphertext;
    *out_len = padded_len;
    return 0;
}

int AES_CBC_decrypt(const AES_HANDLE *ctx, const acry_u8 iv[AES_BLOCK_SIZE],
                     const acry_u8 *ciphertext, size_t len,
                     acry_u8 **out, size_t *out_len) {
    if (len == 0 || len % AES_BLOCK_SIZE != 0) return -1;

    acry_u8 *plaintext = (acry_u8 *)malloc(len);
    if (!plaintext) return -1;

    acry_u8 prev[AES_BLOCK_SIZE];
    memcpy(prev, iv, AES_BLOCK_SIZE);

    for (size_t i = 0; i < len; i += AES_BLOCK_SIZE) {
        acry_u8 decrypted[AES_BLOCK_SIZE];
        AES_decrypt_block(ctx, ciphertext + i, decrypted);
        for (int j = 0; j < AES_BLOCK_SIZE; j++) {
            plaintext[i + j] = (acry_u8)(decrypted[j] ^ prev[j]);
        }
        memcpy(prev, ciphertext + i, AES_BLOCK_SIZE);
    }

    acry_u8 pad_value = plaintext[len - 1];
    if (pad_value == 0 || pad_value > AES_BLOCK_SIZE || (size_t)pad_value > len) {
        free(plaintext);
        return -1;
    }
    for (size_t i = len - pad_value; i < len; i++) {
        if (plaintext[i] != pad_value) {
            free(plaintext);
            return -1;
        }
    }

    *out = plaintext;
    *out_len = len - pad_value;
    return 0;
}