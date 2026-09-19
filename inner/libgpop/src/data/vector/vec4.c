#define __INCL_TRANS3
#define __INCL_TRANS2D_TRANS3D
#include <data/vector/vec3.h>
#include <data/vector/vec4.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EV4_NORM_SCALE 1000

static inline bool __check_mul_overflow(int64_t va, int64_t vb)
{
    if (va != 0 && vb != 0) {
        if (va > 0) {
            if (vb > 0 && va > INT64_MAX / vb) return true;
            if (vb < 0 && vb < INT64_MIN / va) return true;
        } else {
            if (vb > 0 && va < INT64_MIN / vb) return true;
            if (vb < 0 && (va == INT64_MIN || vb == INT64_MIN || (-va) > INT64_MAX / (-vb))) return true;
        }
    }
    return false;
}

Vector4* Vector4_Create(int64_t x, int64_t y, int64_t z, int64_t w, err* Err)
{
    Vector4* vector = calloc(1, sizeof(Vector4));

    if (vector == NULL)
    {
        if (Err != NULL) *Err = EV4_CRTMALLOC;
        return NULL;
    }

    vector->x = x;
    vector->y = y;
    vector->z = z;
    vector->w = w;
    vector->Error = EV4_SUCCESS;

    if (Err != NULL) *Err = EV4_SUCCESS;
    return vector;
}

err Vector4_Set(Vector4* Vector, int64_t x, int64_t y, int64_t z, int64_t w,
                bool xs, bool ys, bool zs, bool ws)
{
    if (Vector == NULL) return EV4_NULLVEC;

    if (!xs) Vector->x = x;
    if (!ys) Vector->y = y;
    if (!zs) Vector->z = z;
    if (!ws) Vector->w = w;

    Vector->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4_Add(Vector4* a, Vector4* b)
{
    if (a == NULL || b == NULL)
        return EV4_NULLVEC;

    if ((b->x > 0 && a->x > INT64_MAX - b->x) || (b->x < 0 && a->x < INT64_MIN - b->x) ||
        (b->y > 0 && a->y > INT64_MAX - b->y) || (b->y < 0 && a->y < INT64_MIN - b->y) ||
        (b->z > 0 && a->z > INT64_MAX - b->z) || (b->z < 0 && a->z < INT64_MIN - b->z) ||
        (b->w > 0 && a->w > INT64_MAX - b->w) || (b->w < 0 && a->w < INT64_MIN - b->w))
    {
        a->Error = EV4_UB; b->Error = EV4_UB;
        return EV4_UB;
    }

    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
    a->w += b->w;

    a->Error = EV4_SUCCESS;
    b->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4_Subtract(Vector4* a, Vector4* b)
{
    if (a == NULL || b == NULL)
        return EV4_NULLVEC;

    if ((b->x < 0 && a->x > INT64_MAX + b->x) || (b->x > 0 && a->x < INT64_MIN + b->x) ||
        (b->y < 0 && a->y > INT64_MAX + b->y) || (b->y > 0 && a->y < INT64_MIN + b->y) ||
        (b->z < 0 && a->z > INT64_MAX + b->z) || (b->z > 0 && a->z < INT64_MIN + b->z) ||
        (b->w < 0 && a->w > INT64_MAX + b->w) || (b->w > 0 && a->w < INT64_MIN + b->w))
    {
        a->Error = EV4_UB; b->Error = EV4_UB;
        return EV4_UB;
    }

    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
    a->w -= b->w;

    a->Error = EV4_SUCCESS;
    b->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4_Multiply(Vector4* a, Vector4* b)
{
    if (a == NULL || b == NULL)
        return EV4_NULLVEC;

    if (__check_mul_overflow(a->x, b->x) ||
        __check_mul_overflow(a->y, b->y) ||
        __check_mul_overflow(a->z, b->z) ||
        __check_mul_overflow(a->w, b->w))
    {
        a->Error = EV4_UB;
        b->Error = EV4_UB;
        return EV4_UB;
    }

    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
    a->w *= b->w;

    a->Error = EV4_SUCCESS;
    b->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

double Vector4_Dot(Vector4* a, Vector4* b, err* Err)
{
    if (Err == NULL) return -1;
    if (a == NULL || b == NULL) { *Err = EV4_NULLVEC; return -2; }

    if (__check_mul_overflow(a->x, b->x) ||
        __check_mul_overflow(a->y, b->y) ||
        __check_mul_overflow(a->z, b->z) ||
        __check_mul_overflow(a->w, b->w))
    {
        *Err = EV4_UB;
        return -3;
    }

    int64_t px = a->x * b->x;
    int64_t py = a->y * b->y;
    int64_t pz = a->z * b->z;
    int64_t pw = a->w * b->w;

    if ((py > 0 && px > INT64_MAX - py) || (py < 0 && px < INT64_MIN - py)) { *Err = EV4_UB; return -3; }
    int64_t sum_xy = px + py;

    if ((pz > 0 && sum_xy > INT64_MAX - pz) || (pz < 0 && sum_xy < INT64_MIN - pz)) { *Err = EV4_UB; return -3; }
    int64_t sum_xyz = sum_xy + pz;

    if ((pw > 0 && sum_xyz > INT64_MAX - pw) || (pw < 0 && sum_xyz < INT64_MIN - pw)) { *Err = EV4_UB; return -3; }

    a->Error = EV4_SUCCESS;
    b->Error = EV4_SUCCESS;
    *Err = EV4_SUCCESS;
    return (double)(sum_xyz + pw);
}

err Vector4_Scale(Vector4* a, double scalar)
{
    if (a == NULL)
        return EV4_NULLVEC;

    if (isnan(scalar) || isinf(scalar))
    {
        a->Error = EV4_UB;
        return EV4_UB;
    }

    double sx = (double)a->x * scalar;
    double sy = (double)a->y * scalar;
    double sz = (double)a->z * scalar;
    double sw = (double)a->w * scalar;

    if (sx < (double)INT64_MIN || sx > (double)INT64_MAX ||
        sy < (double)INT64_MIN || sy > (double)INT64_MAX ||
        sz < (double)INT64_MIN || sz > (double)INT64_MAX ||
        sw < (double)INT64_MIN || sw > (double)INT64_MAX)
    {
        a->Error = EV4_UB;
        return EV4_UB;
    }

    a->x = (int64_t)sx;
    a->y = (int64_t)sy;
    a->z = (int64_t)sz;
    a->w = (int64_t)sw;

    a->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4_Divide(Vector4* a, Vector4* b)
{
    if (a == NULL || b == NULL)
        return EV4_NULLVEC;

    if (b->x == 0 || b->y == 0 || b->z == 0 || b->w == 0 ||
        (a->x == INT64_MIN && b->x == -1) ||
        (a->y == INT64_MIN && b->y == -1) ||
        (a->z == INT64_MIN && b->z == -1) ||
        (a->w == INT64_MIN && b->w == -1))
    {
        a->Error = EV4_UB; b->Error = EV4_UB;
        return EV4_UB;
    }

    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;
    a->w /= b->w;

    a->Error = EV4_SUCCESS;
    b->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4_Negate(Vector4* a)
{
    if (a == NULL)
        return EV4_NULLVEC;

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN || a->w == INT64_MIN)
    {
        a->Error = EV4_UB;
        return EV4_UB;
    }

    a->x = -a->x;
    a->y = -a->y;
    a->z = -a->z;
    a->w = -a->w;

    a->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

double Vector4_Length(Vector4* a, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL)
    {
        *Err = EV4_NULLVEC;
        return -2;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN || a->w == INT64_MIN)
    {
        a->Error = EV4_UB;
        *Err = EV4_UB;
        return -3;
    }

    a->Error = EV4_SUCCESS;
    *Err = EV4_SUCCESS;

    double dx = (double)a->x;
    double dy = (double)a->y;
    double dz = (double)a->z;
    double dw = (double)a->w;

    return sqrt((dx * dx) + (dy * dy) + (dz * dz) + (dw * dw));
}

double Vector4_LengthSquared(Vector4* a, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL)
    {
        *Err = EV4_NULLVEC;
        return -2;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN || a->w == INT64_MIN)
    {
        a->Error = EV4_UB;
        *Err = EV4_UB;
        return -3;
    }

    a->Error = EV4_SUCCESS;
    *Err = EV4_SUCCESS;

    return ((double)a->x * a->x) + ((double)a->y * a->y) + ((double)a->z * a->z) + ((double)a->w * a->w);
}

double Vector4_Distance(Vector4* a, Vector4* b, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL || b == NULL)
    {
        *Err = EV4_NULLVEC;
        return -2;
    }

    if ((b->x < 0 && a->x > INT64_MAX + b->x) || (b->x > 0 && a->x < INT64_MIN + b->x) ||
        (b->y < 0 && a->y > INT64_MAX + b->y) || (b->y > 0 && a->y < INT64_MIN + b->y) ||
        (b->z < 0 && a->z > INT64_MAX + b->z) || (b->z > 0 && a->z < INT64_MIN + b->z) ||
        (b->w < 0 && a->w > INT64_MAX + b->w) || (b->w > 0 && a->w < INT64_MIN + b->w))
    {
        a->Error = EV4_UB; b->Error = EV4_UB; *Err = EV4_UB; return -3;
    }

    int64_t dx = a->x - b->x;
    int64_t dy = a->y - b->y;
    int64_t dz = a->z - b->z;
    int64_t dw = a->w - b->w;

    if (dx == INT64_MIN || dy == INT64_MIN || dz == INT64_MIN || dw == INT64_MIN)
    {
        a->Error = EV4_UB; b->Error = EV4_UB; *Err = EV4_UB; return -3;
    }

    a->Error = EV4_SUCCESS;
    b->Error = EV4_SUCCESS;
    *Err = EV4_SUCCESS;

    double f_dx = (double)dx;
    double f_dy = (double)dy;
    double f_dz = (double)dz;
    double f_dw = (double)dw;

    return sqrt((f_dx * f_dx) + (f_dy * f_dy) + (f_dz * f_dz) + (f_dw * f_dw));
}

double Vector4_DistanceSquared(Vector4* a, Vector4* b, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL || b == NULL)
    {
        *Err = EV4_NULLVEC;
        return -2;
    }

    if ((b->x < 0 && a->x > INT64_MAX + b->x) || (b->x > 0 && a->x < INT64_MIN + b->x) ||
        (b->y < 0 && a->y > INT64_MAX + b->y) || (b->y > 0 && a->y < INT64_MIN + b->y) ||
        (b->z < 0 && a->z > INT64_MAX + b->z) || (b->z > 0 && a->z < INT64_MIN + b->z) ||
        (b->w < 0 && a->w > INT64_MAX + b->w) || (b->w > 0 && a->w < INT64_MIN + b->w))
    {
        a->Error = EV4_UB; b->Error = EV4_UB; *Err = EV4_UB; return -3;
    }

    int64_t dx = a->x - b->x;
    int64_t dy = a->y - b->y;
    int64_t dz = a->z - b->z;
    int64_t dw = a->w - b->w;

    if (dx == INT64_MIN || dy == INT64_MIN || dz == INT64_MIN || dw == INT64_MIN)
    {
        a->Error = EV4_UB; b->Error = EV4_UB; *Err = EV4_UB; return -3;
    }

    a->Error = EV4_SUCCESS;
    b->Error = EV4_SUCCESS;
    *Err = EV4_SUCCESS;

    return ((double)dx * dx) + ((double)dy * dy) + ((double)dz * dz) + ((double)dw * dw);
}

err Vector4_Cross4D(Vector4* a, Vector4* b, Vector4* c, Vector4* out)
{
    if (a == NULL || b == NULL || c == NULL || out == NULL) return EV4_NULLVEC;

    double cx =  (double)a->y * (b->z * c->w - b->w * c->z)
               - (double)a->z * (b->y * c->w - b->w * c->y)
               + (double)a->w * (b->y * c->z - b->z * c->y);

    double cy = -(double)a->x * (b->z * c->w - b->w * c->z)
               + (double)a->z * (b->x * c->w - b->w * c->x)
               - (double)a->w * (b->x * c->z - b->z * c->x);

    double cz =  (double)a->x * (b->y * c->w - b->w * c->y)
               - (double)a->y * (b->x * c->w - b->w * c->x)
               + (double)a->w * (b->x * c->y - b->y * c->x);

    double cw = -(double)a->x * (b->y * c->z - b->z * c->y)
               + (double)a->y * (b->x * c->z - b->z * c->x)
               - (double)a->z * (b->x * c->y - b->y * c->x);

    if (cx < (double)INT64_MIN || cx > (double)INT64_MAX ||
        cy < (double)INT64_MIN || cy > (double)INT64_MAX ||
        cz < (double)INT64_MIN || cz > (double)INT64_MAX ||
        cw < (double)INT64_MIN || cw > (double)INT64_MAX)
    {
        a->Error = EV4_UB; b->Error = EV4_UB; c->Error = EV4_UB; out->Error = EV4_UB;
        return EV4_UB;
    }

    out->x = (int64_t)cx;
    out->y = (int64_t)cy;
    out->z = (int64_t)cz;
    out->w = (int64_t)cw;

    a->Error = EV4_SUCCESS; b->Error = EV4_SUCCESS; c->Error = EV4_SUCCESS; out->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4_Normalize(Vector4* a, Vector4* out)
{
    if (a == NULL || out == NULL)
    {
        return EV4_NULLVEC;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN || a->w == INT64_MIN)
    {
        a->Error = EV4_UB;
        out->Error = EV4_UB;
        return EV4_UB;
    }

    double length = sqrt(((double)a->x * a->x) +
                         ((double)a->y * a->y) +
                         ((double)a->z * a->z) +
                         ((double)a->w * a->w));

    if (length == 0.0)
    {
        a->Error = EV4_UB;
        out->Error = EV4_UB;
        return EV4_UB;
    }

    out->x = (int64_t)(((double)a->x / length) * EV4_NORM_SCALE);
    out->y = (int64_t)(((double)a->y / length) * EV4_NORM_SCALE);
    out->z = (int64_t)(((double)a->z / length) * EV4_NORM_SCALE);
    out->w = (int64_t)(((double)a->w / length) * EV4_NORM_SCALE);

    a->Error = EV4_SUCCESS;
    out->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4_Cross(Vector4* a, Vector4* b, Vector4* out)
{
    if (a == NULL || b == NULL || out == NULL) return EV4_NULLVEC;

    double cx = ((double)a->y * b->z) - ((double)a->z * b->y);
    double cy = ((double)a->z * b->x) - ((double)a->x * b->z);
    double cz = ((double)a->x * b->y) - ((double)a->y * b->x);

    if (cx < (double)INT64_MIN || cx > (double)INT64_MAX ||
        cy < (double)INT64_MIN || cy > (double)INT64_MAX ||
        cz < (double)INT64_MIN || cz > (double)INT64_MAX)
    {
        a->Error = EV4_UB;
        b->Error = EV4_UB;
        out->Error = EV4_UB;
        return EV4_UB;
    }

    out->x = (int64_t)cx;
    out->y = (int64_t)cy;
    out->z = (int64_t)cz;
    out->w = 0;

    a->Error = EV4_SUCCESS;
    b->Error = EV4_SUCCESS;
    out->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

double Vector4_AngleBetween(Vector4* a, Vector4* b, err* Err)
{
    if (Err == NULL) return -1.0;
    if (a == NULL || b == NULL)
    {
        *Err = EV4_NULLVEC;
        return -2.0;
    }

    double dot = Vector4_Dot(a, b, Err);
    if (*Err != EV4_SUCCESS) return -3.0;

    double len_a_sq = ((double)a->x * a->x) + ((double)a->y * a->y) +
                       ((double)a->z * a->z) + ((double)a->w * a->w);
    double len_b_sq = ((double)b->x * b->x) + ((double)b->y * b->y) +
                       ((double)b->z * b->z) + ((double)b->w * b->w);

    if (len_a_sq == 0.0 || len_b_sq == 0.0)
    {
        *Err = EV4_UB;
        return -4.0;
    }

    double len_a = sqrt(len_a_sq);
    double len_b = sqrt(len_b_sq);
    double cos_theta = dot / (len_a * len_b);

    if (cos_theta > 1.0) cos_theta = 1.0;
    if (cos_theta < -1.0) cos_theta = -1.0;

    *Err = EV4_SUCCESS;
    return acos(cos_theta);
}

err Vector4_Rotate(Vector4* a, double radians, Vector4* out, err* Err)
{
    if (a == NULL || out == NULL)
    {
        if (Err != NULL) *Err = EV4_NULLVEC;
        return EV4_NULLVEC;
    }

    if (isnan(radians) || isinf(radians))
    {
        a->Error = EV4_UB;
        out->Error = EV4_UB;
        if (Err != NULL) *Err = EV4_UB;
        return EV4_UB;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN || a->w == INT64_MIN)
    {
        a->Error = EV4_UB;
        out->Error = EV4_UB;
        if (Err != NULL) *Err = EV4_UB;
        return EV4_UB;
    }

    double cos_r = cos(radians);
    double sin_r = sin(radians);

    double rx = ((double)a->x * cos_r) - ((double)a->y * sin_r);
    double ry = ((double)a->x * sin_r) + ((double)a->y * cos_r);

    if (rx < (double)INT64_MIN || rx > (double)INT64_MAX ||
        ry < (double)INT64_MIN || ry > (double)INT64_MAX)
    {
        a->Error = EV4_UB;
        out->Error = EV4_UB;
        if (Err != NULL) *Err = EV4_UB;
        return EV4_UB;
    }

    out->x = (int64_t)rx;
    out->y = (int64_t)ry;
    out->z = a->z;
    out->w = a->w;

    a->Error = EV4_SUCCESS;
    out->Error = EV4_SUCCESS;
    if (Err != NULL) *Err = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4_Lerp(Vector4* a, Vector4* b, double t, Vector4* out, err* Err)
{
    if (a == NULL || b == NULL || out == NULL)
    {
        if (Err != NULL) *Err = EV4_NULLVEC;
        return EV4_NULLVEC;
    }

    if (isnan(t) || isinf(t))
    {
        a->Error = EV4_UB;
        b->Error = EV4_UB;
        out->Error = EV4_UB;
        if (Err != NULL) *Err = EV4_UB;
        return EV4_UB;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN || a->w == INT64_MIN ||
        b->x == INT64_MIN || b->y == INT64_MIN || b->z == INT64_MIN || b->w == INT64_MIN)
    {
        a->Error = EV4_UB;
        b->Error = EV4_UB;
        out->Error = EV4_UB;
        if (Err != NULL) *Err = EV4_UB;
        return EV4_UB;
    }

    double lx = (double)a->x + t * ((double)b->x - (double)a->x);
    double ly = (double)a->y + t * ((double)b->y - (double)a->y);
    double lz = (double)a->z + t * ((double)b->z - (double)a->z);
    double lw = (double)a->w + t * ((double)b->w - (double)a->w);

    if (lx < (double)INT64_MIN || lx > (double)INT64_MAX ||
        ly < (double)INT64_MIN || ly > (double)INT64_MAX ||
        lz < (double)INT64_MIN || lz > (double)INT64_MAX ||
        lw < (double)INT64_MIN || lw > (double)INT64_MAX)
    {
        a->Error = EV4_UB;
        b->Error = EV4_UB;
        out->Error = EV4_UB;
        if (Err != NULL) *Err = EV4_UB;
        return EV4_UB;
    }

    out->x = (int64_t)lx;
    out->y = (int64_t)ly;
    out->z = (int64_t)lz;
    out->w = (int64_t)lw;

    a->Error = EV4_SUCCESS;
    b->Error = EV4_SUCCESS;
    out->Error = EV4_SUCCESS;
    if (Err != NULL) *Err = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4_Destroy(Vector4** Vector)
{
    if (Vector == NULL || *Vector == NULL)
        return EV4_SUCCESS; // Already free'd

    free(*Vector);

    *Vector = NULL;

    return EV4_SUCCESS;
}

Vector3* Vector4_Translate3D(Vector4* Vector)
{
    if (Vector == NULL)
    {
        return NULL;
    }

    int64_t x = Vector->x;
    int64_t y = Vector->y;
    int64_t z = Vector->z;

    if (Vector->w != (int64_t)W_POSVECTOR && Vector->w != (int64_t)W_DIRVECTOR)
    {
        if (Vector->w == 0)
        {
            Vector->Error = EV4_DIVBY0;
            return NULL;
        }

        if (Vector->w == -1 && (x == INT64_MIN || y == INT64_MIN || z == INT64_MIN))
        {
            Vector->Error = EV4_UB;
            return NULL;
        }

        x /= Vector->w;
        y /= Vector->w;
        z /= Vector->w;
    }

    err NErr = EV3_SUCCESS;
    Vector3* NVector = Vector3_Create(x, y, z, &NErr);

    if (NVector == NULL || NErr != EV3_SUCCESS)
    {
        Vector->Error = EV4_CRTMALLOC;
        return NULL;
    }

    Vector->Error = EV4_SUCCESS;
    return NVector;
}

Vector4* Vector4_FromVector3(Vector3* v3, int64_t w, err* Err)
{
    if (v3 == NULL)
    {
        if (Err != NULL) *Err = EV4_NULLVEC;
        return NULL;
    }

    Vector4* v4 = Vector4_Create(v3->x, v3->y, v3->z, w, Err);
    if (v4 == NULL)
    {
        if (Err != NULL) *Err = EV4_CRTMALLOC;
        return NULL;
    }

    if (Err != NULL) *Err = EV4_SUCCESS;
    return v4;
}
