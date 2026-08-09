#include <acry/base.h>
#include <acry/pem.h>
#include <acry/endec/base64.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char *mem_find(const char *hay, size_t hay_len, const char *needle) {
    size_t needle_len = strlen(needle);
    if (needle_len == 0 || needle_len > hay_len) return NULL;
    for (size_t i = 0; i + needle_len <= hay_len; i++) {
        if (memcmp(hay + i, needle, needle_len) == 0) return hay + i;
    }
    return NULL;
}

#define PEM_BEGIN_TAG "-----BEGIN "
#define PEM_END_TAG   "-----END "
#define PEM_DASH_TAG  "-----"

static void free_blocks(PEM_BLOCK *blocks, size_t count) {
    if (!blocks) return;
    for (size_t i = 0; i < count; i++) {
        free(blocks[i].label);
        free(blocks[i].data);
    }
    free(blocks);
}

PEM_DOCUMENT PEM_parse(const char *text, size_t len) {
    PEM_DOCUMENT failed_doc = { NULL, 0 };

    size_t capacity = 4;
    size_t count = 0;
    PEM_BLOCK *blocks = (PEM_BLOCK *)malloc(capacity * sizeof(PEM_BLOCK));
    if (!blocks) return failed_doc;

    size_t pos = 0;
    while (pos < len) {
        const char *begin_found = mem_find(text + pos, len - pos, PEM_BEGIN_TAG);
        if (!begin_found) break; /* no more blocks; stop scanning */

        size_t label_start = (size_t)(begin_found - text) + strlen(PEM_BEGIN_TAG);
        if (label_start > len) { free_blocks(blocks, count); return failed_doc; }

        const char *dashes = mem_find(text + label_start, len - label_start, PEM_DASH_TAG);
        if (!dashes) { free_blocks(blocks, count); return failed_doc; }

        size_t label_end = (size_t)(dashes - text);
        size_t label_len = label_end - label_start;
        if (label_len == 0) { free_blocks(blocks, count); return failed_doc; }

        char *label = (char *)malloc(label_len + 1);
        if (!label) { free_blocks(blocks, count); return failed_doc; }
        memcpy(label, text + label_start, label_len);
        label[label_len] = '\0';

        /* skip to the end of the BEGIN line */
        size_t data_start = label_end + strlen(PEM_DASH_TAG);
        while (data_start < len && text[data_start] != '\n') data_start++;
        if (data_start < len) data_start++; /* consume the '\n' itself */

        size_t end_marker_len = strlen(PEM_END_TAG) + label_len + strlen(PEM_DASH_TAG);
        char *end_marker = (char *)malloc(end_marker_len + 1);
        if (!end_marker) { free(label); free_blocks(blocks, count); return failed_doc; }
        snprintf(end_marker, end_marker_len + 1, "%s%s%s", PEM_END_TAG, label, PEM_DASH_TAG);

        const char *end_found = data_start <= len
            ? mem_find(text + data_start, len - data_start, end_marker)
            : NULL;
        if (!end_found) {
            free(label);
            free(end_marker);
            free_blocks(blocks, count);
            return failed_doc;
        }
        size_t data_end = (size_t)(end_found - text);
        free(end_marker);

        acry_u8 *decoded = NULL;
        size_t decoded_len = 0;
        if (BASE64_decode(text + data_start, data_end - data_start, &decoded, &decoded_len) != 0) {
            free(label);
            free_blocks(blocks, count);
            return failed_doc;
        }

        if (count == capacity) {
            capacity *= 2;
            PEM_BLOCK *grown = (PEM_BLOCK *)realloc(blocks, capacity * sizeof(PEM_BLOCK));
            if (!grown) {
                free(label);
                free(decoded);
                free_blocks(blocks, count);
                return failed_doc;
            }
            blocks = grown;
        }

        blocks[count].label = label;
        blocks[count].data = decoded;
        blocks[count].data_len = decoded_len;
        count++;

        pos = data_end + end_marker_len;
    }

    if (count == 0) {
        free_blocks(blocks, count);
        return failed_doc;
    }

    PEM_DOCUMENT doc;
    doc.blocks = blocks;
    doc.count = count;
    return doc;
}

void PEM_DOCUMENT_free(PEM_DOCUMENT *doc) {
    if (!doc) return;
    free_blocks(doc->blocks, doc->count);
    doc->blocks = NULL;
    doc->count = 0;
}

char *PEM_encode(const char *label, const acry_u8 *data, size_t data_len) {
    char *b64 = BASE64_encode(data, data_len);
    if (!b64) return NULL;
    size_t b64_len = strlen(b64);

    size_t label_len = strlen(label);
    size_t num_lines = b64_len == 0 ? 0 : (b64_len + 63) / 64;

    size_t header_len = strlen(PEM_BEGIN_TAG) + label_len + strlen(PEM_DASH_TAG) + 1; /* +1 for '\n' */
    size_t footer_len = strlen(PEM_END_TAG) + label_len + strlen(PEM_DASH_TAG) + 1;
    size_t body_len = b64_len + num_lines; /* one '\n' per line */

    size_t total = header_len + body_len + footer_len + 1; /* +1 for NUL */
    char *out = (char *)malloc(total);
    if (!out) {
        free(b64);
        return NULL;
    }

    size_t pos = 0;
    pos += (size_t)sprintf(out + pos, "%s%s%s\n", PEM_BEGIN_TAG, label, PEM_DASH_TAG);

    for (size_t i = 0; i < b64_len; i += 64) {
        size_t chunk = (b64_len - i) < 64 ? (b64_len - i) : 64;
        memcpy(out + pos, b64 + i, chunk);
        pos += chunk;
        out[pos++] = '\n';
    }

    pos += (size_t)sprintf(out + pos, "%s%s%s\n", PEM_END_TAG, label, PEM_DASH_TAG);
    out[pos] = '\0';

    free(b64);
    return out;
}

char *PEM_encode_multi(const PEM_BLOCK *blocks, size_t count) {
    if (count == 0) {
        char *out = (char *)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }

    char **parts = (char **)malloc(count * sizeof(char *));
    if (!parts) return NULL;

    size_t total_len = 0;
    size_t built = 0;
    for (; built < count; built++) {
        parts[built] = PEM_encode(blocks[built].label, blocks[built].data, blocks[built].data_len);
        if (!parts[built]) break;
        total_len += strlen(parts[built]);
    }

    if (built != count) {
        for (size_t i = 0; i < built; i++) free(parts[i]);
        free(parts);
        return NULL;
    }

    char *out = (char *)malloc(total_len + 1);
    if (!out) {
        for (size_t i = 0; i < count; i++) free(parts[i]);
        free(parts);
        return NULL;
    }

    size_t pos = 0;
    for (size_t i = 0; i < count; i++) {
        size_t l = strlen(parts[i]);
        memcpy(out + pos, parts[i], l);
        pos += l;
        free(parts[i]);
    }
    out[pos] = '\0';

    free(parts);
    return out;
}