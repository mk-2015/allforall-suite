#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint8_t*** Map3D_Grid;
#ifndef __POS_TYPEDEF_DEFINED
typedef unsigned long long pos;
#define __POS_TYPEDEF_DEFINED
#endif

#ifndef __ERR_TYPEDEF_DEFINED
typedef unsigned long long err;
#define __ERR_TYPEDEF_DEFINED
#endif

#define MAP3D_BYTENOTZERO (err)1
#define MAP3D_BITNOTZERO  (err)2
#define MAP3D_MALLOCFAIL  (err)3
#define MAP3D_OVERFLOW    (err)4
#define MAP3D_FILEERROR   (err)5
#define MAP3D_PARSEERROR  (err)6
#define CSV_INITFAIL      (err)7
#define MAP3D_MALLOCERR   (err)8
#define MAP3D_SUCCESS     (err)0

typedef struct Map3D_s
{
    Map3D_Grid Map;
    err Error;

    pos selected_byte_x;
    pos selected_byte_y;
    pos selected_byte_z;

    pos x;
    pos y;
    pos z;
} Map3D;

Map3D* Map3D_Init(pos x, pos y, pos z);
void Map3D_Resize(Map3D* Map, pos x, pos y, pos z);
void Map3D_Zero(Map3D* Map);
err Map3D_GetLastError(Map3D* Map);

void Map3D_SelectByte(Map3D* Map, pos x, pos y, pos z);
void Map3D_ReplaceByte(Map3D* Map, uint8_t byte);
void Map3D_SetByte(Map3D* Map, uint8_t byte);
void Map3D_ZeroByte(Map3D* Map);
void Map3D_GetByte(Map3D* Map, uint8_t* out);

void Map3D_BitSet(Map3D* Map, int8_t n);
void Map3D_BitClear(Map3D* Map, int8_t n);
void Map3D_BitToggle(Map3D* Map, int8_t n);
bool Map3D_BitGet(Map3D* Map, int8_t n);

void Map3D_Deinit(Map3D* Map);
