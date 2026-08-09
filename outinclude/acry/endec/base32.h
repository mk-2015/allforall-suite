#pragma once

#include <acry/base.h>
#include <stddef.h>

char *BASE32_encode(const acry_u8 *data, size_t len);
int BASE32_decode(const char *in, size_t in_len, acry_u8 **out_data, size_t *out_len);