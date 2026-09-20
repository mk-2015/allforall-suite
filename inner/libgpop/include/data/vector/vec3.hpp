#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <data/vector/vec2.hpp>

namespace gpop { namespace Data { namespace Vector {

#ifndef __ERR_TYPEDEF_DEFINED
typedef unsigned long long err;
#define __ERR_TYPEDEF_DEFINED
#endif

#define EV3_SUCCESS   (err)0
#define EV3_DIVBY0    (err)1
#define EV3_SQRT_N1   (err)2
#define EV3_DESTROY   (err)3
#define EV3_CRTMALLOC (err)4
#define EV3_NULLVEC   (err)5
#define EV3_UB        (err)6
#define EV3_OVERFLOW  EV3_UB
#define EV3_UNDERFLOW EV3_UB

class Vector3 {
public:
    int64_t x, y, z;
    mutable err Error;

    Vector3(int64_t x, int64_t y, int64_t z);
    err Set(int64_t x, int64_t y, int64_t z, bool xs, bool ys, bool zs);
    err Add(const Vector3& b);
    err Subtract(const Vector3& b);
    err Multiply(const Vector3& b);
    err Scale(double scalar);
    err Divide(const Vector3& b);
    err Negate();
    double Length(err* Err) const;
    double LengthSquared(err* Err) const;
    double Distance(const Vector3& b, err* Err) const;
    double DistanceSquared(const Vector3& b, err* Err) const;
    err Normalize(Vector3& out) const;
    double Dot(const Vector3& b, err* Err) const;
    err Cross(const Vector3& b, Vector3& out) const;
    double AngleBetween(const Vector3& b, err* Err) const;
    err Rotate(double radians, Vector3& out, err* Err) const;
    err Lerp(const Vector3& b, double t, Vector3& out, err* Err) const;

    // Translation functions
    Vector2* Translate2D();
    static Vector3* FromVector2(const Vector2& v2, int64_t z);
};

}}}
