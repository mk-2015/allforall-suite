#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <data/vector/vec3.hpp>

namespace gpop { namespace Data { namespace Vector {

#ifndef __ERR_TYPEDEF_DEFINED
typedef unsigned long long err;
#define __ERR_TYPEDEF_DEFINED
#endif

#define EV4_SUCCESS   (err)0
#define EV4_DIVBY0    (err)1
#define EV4_SQRT_N1   (err)2
#define EV4_DESTROY   (err)3
#define EV4_CRTMALLOC (err)4
#define EV4_NULLVEC   (err)5
#define EV4_UB        (err)6
#define EV4_OVERFLOW  EV4_UB
#define EV4_UNDERFLOW EV4_UB

#define W_POSVECTOR    1.0
#define W_DIRVECTOR    0.0
#define W_PREVECTOR(x) (x)

class Vector4 {
public:
    int64_t x, y, z, w;
    mutable err Error;

    Vector4(int64_t x, int64_t y, int64_t z, int64_t w);
    err Set(int64_t x, int64_t y, int64_t z, int64_t w, bool xs, bool ys, bool zs, bool ws);
    err Add(const Vector4& b);
    err Subtract(const Vector4& b);
    err Multiply(const Vector4& b);
    err Scale(double scalar);
    err Divide(const Vector4& b);
    err Negate();
    double Length(err* Err) const;
    double LengthSquared(err* Err) const;
    double Distance(const Vector4& b, err* Err) const;
    double DistanceSquared(const Vector4& b, err* Err) const;
    err Normalize(Vector4& out) const;
    double Dot(const Vector4& b, err* Err) const;
    err Cross4D(const Vector4& b, const Vector4& c, Vector4& out) const;

    double AngleBetween(const Vector4& b, err* Err) const;
    err Rotate(double radians, Vector4& out, err* Err) const;
    err Lerp(const Vector4& b, double t, Vector4& out, err* Err) const;

    // Translation
    Vector3* Translate3D();
    static Vector4* FromVector3(const Vector3& v3, int64_t w);
};

}}}
