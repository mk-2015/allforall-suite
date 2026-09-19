#pragma once

#include <data/vector/vec2.h>
#include <stdint.h>
#include <stdbool.h>

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

typedef struct {
    int64_t x;
    int64_t y;
    int64_t z;
    err Error;
} Vector3;

Vector3* Vector3_Create(int64_t x, int64_t y, int64_t z, err* Err);
err Vector3_Set(Vector3* Vector, int64_t x, int64_t y, int64_t z, bool xs, bool ys, bool zs);
err Vector3_Add(Vector3* a, Vector3* b);
err Vector3_Subtract(Vector3* a, Vector3* b);
err Vector3_Multiply(Vector3* a, Vector3* b);
err Vector3_Scale(Vector3* a, double scalar);
err Vector3_Divide(Vector3* a, Vector3* b);
err Vector3_Negate(Vector3* a);
double Vector3_Length(Vector3* a, err* Err);
double Vector3_LengthSquared(Vector3* a, err* Err);
double Vector3_Distance(Vector3* a, Vector3* b, err* Err);
double Vector3_DistanceSquared(Vector3* a, Vector3* b, err* Err);
err Vector3_Normalize(Vector3* a, Vector3* out);
double Vector3_Dot(Vector3* a, Vector3* b, err* Err);
err Vector3_Cross(Vector3* a, Vector3* b, Vector3* out);
double Vector3_AngleBetween(Vector3* a, Vector3* b, err* Err);
err Vector3_Rotate(Vector3* a, double radians, Vector3* out, err* Err);
err Vector3_Lerp(Vector3* a, Vector3* b, double t, Vector3* out, err* Err);
err Vector3_Destroy(Vector3** Vector);
#ifdef __INCL_TRANS2D_TRANS3D
Vector2* Vector3_Translate2D(Vector3* Vector);
Vector3* Vector3_FromVector2(Vector2* v2, int64_t z, err* Err);
#endif
