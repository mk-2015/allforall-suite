#pragma once

#include <acry/base.h>
#include <stddef.h>

RANDOM_HANDLE RANDOM_OPEN_handle(void);
void RANDOM_LOAD_bytes(size_t bytes_count, RANDOM_HANDLE *random);
void* RANDOM_GET_bytes(RANDOM_HANDLE random);
int RANDOM_CLOSE_handle(RANDOM_HANDLE random);

void* RANDOM_GET_bytes_NO_HANDLE(size_t size);