#include <acry/base.h>
#include <acry/endec/base32.h>
#include <stdlib.h>

static const char B32_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

char *BASE32_encode(const acry_u8 *data, size_t len) {
    size_t out_len = ((len + 4) / 5) * 8;
    char *out = (char *)malloc(out_len + 1);
    if (!out) return NULL;

    size_t i = 0, j = 0;
    acry_u32 buffer = 0;
    int bits = 0;

    while (i < len) {
        buffer = (buffer << 8) | data[i++];
        bits += 8;
        while (bits >= 5) {
            bits -= 5;
            out[j++] = B32_TABLE[(buffer >> bits) & 0x1F];
        }
    }
    if (bits > 0) {
        out[j++] = B32_TABLE[(buffer << (5 - bits)) & 0x1F];
    }
    while (j < out_len) out[j++] = '=';

    out[out_len] = '\0';
    return out;
}

static int b32_val(char c, int *ok) {
    *ok = 1;
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a'; /* case-insensitive */
    if (c >= '2' && c <= '7') return c - '2' + 26;
    *ok = 0;
    return 0;
}

static int is_b32_skip(char c) {
    return c == '=' || c == '\n' || c == '\r' || c == ' ' || c == '\t';
}

int BASE32_decode(const char *in, size_t in_len, acry_u8 **out_data, size_t *out_len) {
    size_t valid_count = 0;
    for (size_t i = 0; i < in_len; i++) {
        if (!is_b32_skip(in[i])) valid_count++;
    }
    if (valid_count == 0) {
        *out_data = NULL;
        *out_len = 0;
        return 0;
    }
    /* A valid base32 group is 8 chars -> 5 bytes. Within one group, only
     * 2, 4, 5, or 7 significant chars are valid (encoding 1, 2, 3, or 4
     * trailing bytes respectively); 1, 3, and 6 are impossible outputs
     * of the encoder and therefore malformed. */
    size_t rem = valid_count % 8;
    if (rem == 1 || rem == 3 || rem == 6) return -1;

    size_t max_out = (valid_count * 5) / 8 + 1;
    acry_u8 *out = (acry_u8 *)malloc(max_out);
    if (!out) return -1;

    acry_u32 buffer = 0;
    int bits = 0;
    size_t out_pos = 0;

    for (size_t i = 0; i < in_len; i++) {
        char c = in[i];
        if (is_b32_skip(c)) continue;
        int ok;
        int val = b32_val(c, &ok);
        if (!ok) {
            free(out);
            return -1;
        }
        buffer = (buffer << 5) | (acry_u32)val;
        bits += 5;
        if (bits >= 8) {
            bits -= 8;
            out[out_pos++] = (acry_u8)((buffer >> bits) & 0xFF);
        }
    }

    *out_data = out;
    *out_len = out_pos;
    return 0;
}