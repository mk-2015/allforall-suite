namespace gpop::BitManipulation {
#pragma once

#include <stddef.h>


#define BIT_SET(x, n)     ((x) |= (1U << (n)))
#define BIT_CLEAR(x, n)   ((x) &= ~(1U << (n)))
#define BIT_TOGGLE(x, n)  ((x) ^= (1U << (n)))
#define BIT_CHECK(x, n)   (!!((x) & (1U << (n))))

#define GET_BYTE(x, n)    (((x) >> ((n) * 8)) & 0xFF)
#define IS_POWER_OF_2(x)  ((x) && !((x) & ((x) - 1)))

typedef unsigned long long COUNT;

COUNT bitcount(const unsigned char* bytes, size_t len);
COUNT bytecount(const unsigned char* bytes, size_t len);
COUNT bitreverse(const unsigned char* in, unsigned char* out, size_t len);
COUNT bytereverse(const unsigned char* in, unsigned char* out, size_t len);

} // namespace gpop::BitManipulation
