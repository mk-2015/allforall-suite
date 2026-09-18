#pragma once

#include <stdint.h>

/* 8-bit Rotations */
uint8_t rotl8(uint8_t value, unsigned int shift);
uint8_t rotr8(uint8_t value, unsigned int shift);

/* 16-bit Rotations */
uint16_t rotl16(uint16_t value, unsigned int shift);
uint16_t rotr16(uint16_t value, unsigned int shift);

/* 32-bit Rotations */
uint32_t rotl32(uint32_t value, unsigned int shift);
uint32_t rotr32(uint32_t value, unsigned int shift);

/* 64-bit Rotations */
uint64_t rotl64(uint64_t value, unsigned int shift);
uint64_t rotr64(uint64_t value, unsigned int shift);
