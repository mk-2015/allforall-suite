#include <bitm/rotate.hpp>
using namespace gpop::BitManipulation;

/* 8-bit Implementation */
uint8_t rotl8(uint8_t value, unsigned int shift) {
    shift &= 7;
    if (shift == 0) return value;
    return (value << shift) | (value >> (8 - shift));
}

uint8_t rotr8(uint8_t value, unsigned int shift) {
    shift &= 7;
    if (shift == 0) return value;
    return (value >> shift) | (value << (8 - shift));
}

/* 16-bit Implementation */
uint16_t rotl16(uint16_t value, unsigned int shift) {
    shift &= 15;
    if (shift == 0) return value;
    return (value << shift) | (value >> (16 - shift));
}

uint16_t rotr16(uint16_t value, unsigned int shift) {
    shift &= 15;
    if (shift == 0) return value;
    return (value >> shift) | (value << (16 - shift));
}

/* 32-bit Implementation */
uint32_t rotl32(uint32_t value, unsigned int shift) {
    shift &= 31;
    if (shift == 0) return value;
    return (value << shift) | (value >> (32 - shift));
}

uint32_t rotr32(uint32_t value, unsigned int shift) {
    shift &= 31;
    if (shift == 0) return value;
    return (value >> shift) | (value << (32 - shift));
}

/* 64-bit Implementation */
uint64_t rotl64(uint64_t value, unsigned int shift) {
    shift &= 63;
    if (shift == 0) return value;
    return (value << shift) | (value >> (64 - shift));
}

uint64_t rotr64(uint64_t value, unsigned int shift) {
    shift &= 63;
    if (shift == 0) return value;
    return (value >> shift) | (value << (64 - shift));
}
