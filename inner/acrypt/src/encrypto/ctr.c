#include <acry/base.h>
#include <acry/modes/ctr.h>
#include <stdlib.h>
#include <string.h>

static void increment_counter(acry_u8 counter[AES_BLOCK_SIZE]) {
    for (int i = AES_BLOCK_SIZE - 1; i >= 0; i--) {
        if (++counter[i] != 0) break; /* stop unless this byte wrapped (0xFF -> 0x00) */
    }
}

int AES_CTR_crypt(const AES_HANDLE *ctx, const acry_u8 nonce_counter[AES_BLOCK_SIZE],
                   const acry_u8 *data, size_t len,
                   acry_u8 **out, size_t *out_len) {
    acry_u8 *result = (acry_u8 *)malloc(len > 0 ? len : 1);
    if (!result) return -1;

    acry_u8 counter[AES_BLOCK_SIZE];
    memcpy(counter, nonce_counter, AES_BLOCK_SIZE);

    size_t pos = 0;
    while (pos < len) {
        acry_u8 keystream[AES_BLOCK_SIZE];
        AES_encrypt_block(ctx, counter, keystream);

        size_t chunk = (len - pos) < AES_BLOCK_SIZE ? (len - pos) : AES_BLOCK_SIZE;
        for (size_t j = 0; j < chunk; j++) {
            result[pos + j] = (acry_u8)(data[pos + j] ^ keystream[j]);
        }

        increment_counter(counter);
        pos += chunk;
    }

    *out = result;
    *out_len = len;
    return 0;
}