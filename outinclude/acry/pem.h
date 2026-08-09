#pragma once

#include <acry/base.h>
#include <stddef.h>


PEM_DOCUMENT PEM_parse(const char *text, size_t len);
void PEM_DOCUMENT_free(PEM_DOCUMENT *doc);
char *PEM_encode(const char *label, const acry_u8 *data, size_t data_len);
char *PEM_encode_multi(const PEM_BLOCK *blocks, size_t count);