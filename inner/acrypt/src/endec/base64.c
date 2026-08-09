#include <acry/base.h>
#include <acry/endec/base64.h>
#include <stdlib.h>

static const char B64_TABLE[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *BASE64_encode(const acry_u8 *data, size_t len) {
    size_t out_len = ((len + 2) / 3) * 4;
    char *out = (char *)malloc(out_len + 1);
    if (!out) return NULL;

    size_t i = 0, j = 0;
    while (i < len) {
        acry_u32 a = data[i++];
        acry_u32 b = i < len ? data[i++] : 0;
        acry_u32 c = i < len ? data[i++] : 0;
        acry_u32 triple = (a << 16) | (b << 8) | c;

        out[j++] = B64_TABLE[(triple >> 18) & 0x3F];
        out[j++] = B64_TABLE[(triple >> 12) & 0x3F];
        out[j++] = B64_TABLE[(triple >> 6) & 0x3F];
        out[j++] = B64_TABLE[triple & 0x3F];
    }

    size_t mod = len % 3;
    if (mod == 1) {
        out[out_len - 1] = '=';
        out[out_len - 2] = '=';
    } else if (mod == 2) {
        out[out_len - 1] = '=';
    }

    out[out_len] = '\0';
    return out;
}

static int b64_val(char c, int *ok) {
    *ok = 1;
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    *ok = 0;
    return 0;
}

static int is_b64_skip(char c) {
    return c == '=' || c == '\n' || c == '\r' || c == ' ' || c == '\t';
}

int BASE64_decode(const char *in, size_t in_len, acry_u8 **out_data, size_t *out_len) {
    size_t valid_count = 0;
    for (size_t i = 0; i < in_len; i++) {
        if (!is_b64_skip(in[i])) valid_count++;
    }
    if (valid_count == 0) {
        *out_data = NULL;
        *out_len = 0;
        return 0;
    }
    if (valid_count % 4 == 1) return -1;

    size_t max_out = (valid_count / 4 + 1) * 3;
    acry_u8 *out = (acry_u8 *)malloc(max_out);
    if (!out) return -1;

    acry_u32 buffer = 0;
    int bits = 0;
    size_t out_pos = 0;

    for (size_t i = 0; i < in_len; i++) {
        char c = in[i];
        if (is_b64_skip(c)) continue;
        int ok;
        int val = b64_val(c, &ok);
        if (!ok) {
            free(out);
            return -1;
        }
        buffer = (buffer << 6) | (acry_u32)val;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out[out_pos++] = (acry_u8)((buffer >> bits) & 0xFF);
        }
    }

    *out_data = out;
    *out_len = out_pos;
    return 0;
}