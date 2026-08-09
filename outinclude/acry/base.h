#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// Platform detection macros
#if defined(_WIN32) || defined(_WIN64)
    #define ACRYPT_WINDOWS 1
    #define ACRYPT_EXPORT __declspec(dllexport)
    #define ACRYPT_IMPORT __declspec(dllimport)
#else
    #define ACRYPT_UNIX 1
    #define ACRYPT_EXPORT __attribute__((visibility("default")))
    #define ACRYPT_IMPORT __attribute__((visibility("default")))
#endif

#if defined(ACRYPT_WINDOWS)
typedef void* HANDLE_t;
#else
typedef int* HANDLE_t;
#endif

#define ROTBIT_LEFT(bits, shift)  (((bits) << ((shift) % (sizeof(bits) * 8))) | ((bits) >> ((sizeof(bits) * 8) - ((shift) % (sizeof(bits) * 8)))))
#define ROTBIT_RIGHT(bits, shift) (((bits) >> ((shift) % (sizeof(bits) * 8))) | ((bits) << ((sizeof(bits) * 8) - ((shift) % (sizeof(bits) * 8)))))

#define ROTBYTE_LEFT(bytes, shift)  ROTBIT_LEFT(bytes, (shift) * 8)
#define ROTBYTE_RIGHT(bytes, shift) ROTBIT_RIGHT(bytes, (shift) * 8)

#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

#define ROTR64(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
#define SHFR64(x, n) ((x) >> (n))
#define S0_512(x) (ROTR64(x, 28) ^ ROTR64(x, 34) ^ ROTR64(x, 39))
#define S1_512(x) (ROTR64(x, 14) ^ ROTR64(x, 18) ^ ROTR64(x, 41))
#define s0_512(x) (ROTR64(x, 1)  ^ ROTR64(x, 8)  ^ SHFR64(x, 7))
#define s1_512(x) (ROTR64(x, 19) ^ ROTR64(x, 61) ^ SHFR64(x, 6))
#define CH64(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ64(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define ACRY_SHA256_BLOCK_SIZE 64
#define ACRY_SHA256_DIGEST_SIZE 32

#define ACRY_SHA512_BLOCK_SIZE 128
#define ACRY_SHA512_DIGEST_SIZE 64

#define ACRY_SHA1_BLOCK_SIZE 64
#define ACRY_SHA1_DIGEST_SIZE 20

#define ACRY_SHA224_BLOCK_SIZE 64
#define ACRY_SHA224_DIGEST_SIZE 28

typedef uint8_t  acry_u8;
typedef uint16_t acry_u16;
typedef uint32_t acry_u32;
typedef uint64_t acry_u64;

typedef int8_t   acry_i8;
typedef int16_t  acry_i16;
typedef int32_t  acry_i32;
typedef int64_t  acry_i64;

typedef struct 
{
    HANDLE_t handle;
    void* bytes;
} RANDOM_HANDLE;

typedef struct 
{
    HANDLE_t handle;
    ssize_t amount_to_rotate;
} ROTATION_HANDLE;

typedef struct {
    acry_u32 state[8];
    acry_u64 count;
    acry_u8 buffer[ACRY_SHA256_BLOCK_SIZE];
} SHA256_HANDLE;

typedef struct {
    acry_u64 state[8];
    acry_u64 count[2];
    acry_u8 buffer[ACRY_SHA512_BLOCK_SIZE];
} SHA512_HANDLE;

typedef struct {
    acry_u32 state[5];
    acry_u64 count;
    acry_u8 buffer[ACRY_SHA1_BLOCK_SIZE];
} SHA1_HANDLE;

typedef struct {
    acry_u32 state[8];
    acry_u64 count;
    acry_u8 buffer[ACRY_SHA224_BLOCK_SIZE];
} SHA224_HANDLE;

typedef struct {
    char *label;
    acry_u8 *data;
    size_t data_len;
} PEM_BLOCK;

typedef struct {
    PEM_BLOCK *blocks;
    size_t count;
} PEM_DOCUMENT;