#pragma once

#include <stdint.h>
#include <stdbool.h>

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

typedef struct {
    int64_t x;
    int64_t y;
    err Error;
} Vector2;

Vector2* Vector2_Create(int64_t x, int64_t y, err* Err);
err Vector2_Set(Vector2* Vector, int64_t x, int64_t y, bool xs, bool ys);
err Vector2_Add(Vector2* a, Vector2* b);
err Vector2_Subtract(Vector2* a, Vector2* b);
err Vector2_Multiply(Vector2* a, Vector2* b);
err Vector2_Scale(Vector2* a, double scalar);
err Vector2_Divide(Vector2* a, Vector2* b);
err Vector2_Negate(Vector2* a);
double Vector2_Length(Vector2* a, err* Err);
double Vector2_LengthSquared(Vector2* a, err* Err);
double Vector2_Distance(Vector2* a, Vector2* b, err* Err);
double Vector2_DistanceSquared(Vector2* a, Vector2* b, err* Err);
err Vector2_Normalize(Vector2* a, int64_t* out_x, int64_t* out_y);
double Vector2_Dot(Vector2* a, Vector2* b, err* Err);
double Vector2_Cross(Vector2* a, Vector2* b, err* Err);
double Vector2_AngleBetween(Vector2* a, Vector2* b, err* Err);
err Vector2_Rotate(Vector2* a, double radians, int64_t* out_x, int64_t* out_y, err* Err);
err Vector2_Lerp(Vector2* a, Vector2* b, double t, int64_t* out_x, int64_t* out_y, err* Err);
err Vector2_Destroy(Vector2** Vector);
