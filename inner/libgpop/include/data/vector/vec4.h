#pragma once

#include <stdint.h>
#include <stdbool.h>

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

typedef struct {
    int64_t x;
    int64_t y;
    int64_t z;
    int64_t w;
    err Error;
} Vector4;

Vector4* Vector4_Create(int64_t x, int64_t y, int64_t z, int64_t w, err* Err);
err Vector4_Set(Vector4* Vector, int64_t x, int64_t y, int64_t z, int64_t w,
                bool xs, bool ys, bool zs, bool ws);
err Vector4_Add(Vector4* a, Vector4* b);
err Vector4_Subtract(Vector4* a, Vector4* b);
err Vector4_Multiply(Vector4* a, Vector4* b);
err Vector4_Scale(Vector4* a, double scalar);
err Vector4_Divide(Vector4* a, Vector4* b);
err Vector4_Negate(Vector4* a);
double Vector4_Length(Vector4* a, err* Err);
double Vector4_LengthSquared(Vector4* a, err* Err);
double Vector4_Distance(Vector4* a, Vector4* b, err* Err);
double Vector4_DistanceSquared(Vector4* a, Vector4* b, err* Err);
err Vector4_Normalize(Vector4* a, Vector4* out);
double Vector4_Dot(Vector4* a, Vector4* b, err* Err);
err Vector4_Cross4D(Vector4* a, Vector4* b, Vector4* c, Vector4* out);

double Vector4_AngleBetween(Vector4* a, Vector4* b, err* Err);
err Vector4_Rotate(Vector4* a, double radians, Vector4* out, err* Err);
err Vector4_Lerp(Vector4* a, Vector4* b, double t, Vector4* out, err* Err);
err Vector4_Destroy(Vector4** Vector);

#ifdef __INCL_TRANS3
Vector3* Vector4_Translate3D(Vector4* Vector);
Vector4* Vector4_FromVector3(Vector3* v3, int64_t w, err* Err);
#endif
