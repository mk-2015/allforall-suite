#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint8_t** Map2D_Grid;
#ifndef __ERR_TYPEDEF_DEFINED
typedef unsigned long long err;
#define __ERR_TYPEDEF_DEFINED
#endif

#define MAP2D_BYTENOTZERO (err)1
#define MAP2D_BITNOTZERO  (err)2
#define MAP2D_MALLOCFAIL  (err)3
#define MAP2D_OVERFLOW    (err)4
#define MAP2D_FILEERROR   (err)5
#define MAP2D_PARSEERROR  (err)6
#define CSV_INITFAIL      (err)7
#define MAP2D_MALLOCERR   (err)8
#define MAP2D_SUCCESS     (err)0

typedef struct Map2D_s
{
    Map2D_Grid Map;
    err Error;

    unsigned long long selected_byte_x;
    unsigned long long selected_byte_y;

    unsigned long long x;
    unsigned long long y;
} Map2D;

Map2D* Map2D_Init(unsigned long long x, unsigned long long y);
void Map2D_Resize(Map2D* Map, unsigned long long x, unsigned long long y);
void Map2D_Zero(Map2D* Map);
err Map2D_SaveCSV(Map2D* Map, const char* filename);
Map2D* Map2D_LoadCSV(const char* filename, err* Error);
err Map2D_GetLastError(Map2D* Map);

void Map2D_SelectByte(Map2D* Map, unsigned long long x, unsigned long long y);
void Map2D_ReplaceByte(Map2D* Map, uint8_t byte);
void Map2D_SetByte(Map2D* Map, uint8_t byte);
void Map2D_ZeroByte(Map2D* Map);
void Map2D_GetByte(Map2D* Map, uint8_t* out);

void Map2D_BitSet(Map2D* Map, int8_t n);
void Map2D_BitClear(Map2D* Map, int8_t n);
void Map2D_BitToggle(Map2D* Map, int8_t n);
bool Map2D_BitGet(Map2D* Map, int8_t n);

void Map2D_Deinit(Map2D* Map);
