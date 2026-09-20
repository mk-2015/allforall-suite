#pragma once
#include <stdint.h>
#include <stdbool.h>

namespace gpop { namespace Data { namespace Vector {

#ifndef __ERR_TYPEDEF_DEFINED
typedef unsigned long long err;
#define __ERR_TYPEDEF_DEFINED
#endif

#define EV2_SUCCESS   (err)0
#define EV2_DIVBY0    (err)1
#define EV2_SQRT_N1   (err)2
#define EV2_DESTROY   (err)3
#define EV2_CRTMALLOC (err)4
#define EV2_NULLVEC   (err)5
#define EV2_UB        (err)6
#define EV2_OVERFLOW  EV2_UB
#define EV2_UNDERFLOW EV2_UB

class Vector2 {
public:
    int64_t x;
    int64_t y;
    mutable err Error;

    Vector2(int64_t x, int64_t y);
    err Set(int64_t x, int64_t y, bool xs, bool ys);
    err Add(const Vector2& b);
    err Subtract(const Vector2& b);
    err Multiply(const Vector2& b);
    err Scale(double scalar);
    err Divide(const Vector2& b);
    err Negate();
    double Length(err* Err) const;
    double LengthSquared(err* Err) const;
    double Distance(const Vector2& b, err* Err) const;
    double DistanceSquared(const Vector2& b, err* Err) const;
    err Normalize(int64_t* out_x, int64_t* out_y) const;
    double Dot(const Vector2& b, err* Err) const;
};

}}}
