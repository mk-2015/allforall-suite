#define __INCL_TRANS2D_TRANS3D
#include <data/vector/vec2.h>
#include <data/vector/vec3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EV3_NORM_SCALE 1000

Vector3* Vector3_Create(int64_t x, int64_t y, int64_t z, err* Err)
{
    Vector3* vector = calloc(1, sizeof(Vector3));

    if (vector == NULL)
    {
        if (Err != NULL)
            *Err = EV3_CRTMALLOC;
        return NULL;
    }

    vector->x = x;
    vector->y = y;
    vector->z = z;
    vector->Error = EV3_SUCCESS;
    if (Err != NULL)
        *Err = EV3_SUCCESS;
    return vector;
}

err Vector3_Set(Vector3* Vector, int64_t x, int64_t y, int64_t z,
    bool xs, bool ys, bool zs)
{
    if(!xs) Vector->x = x;
    if(!ys) Vector->y = y;
    if(!zs) Vector->z = z;
    Vector->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3_Add(Vector3* a, Vector3* b)
{
    if (a == NULL || b == NULL)
        return EV3_NULLVEC;

    if ((b->x > 0 && a->x > INT64_MAX - b->x) ||
        (b->x < 0 && a->x < INT64_MIN - b->x))
    {
        a->Error = EV3_UB; b->Error = EV3_UB;
        return EV3_UB;
    }

    if ((b->y > 0 && a->y > INT64_MAX - b->y) ||
        (b->y < 0 && a->y < INT64_MIN - b->y))
    {
        a->Error = EV3_UB; b->Error = EV3_UB;
        return EV3_UB;
    }

    if ((b->z > 0 && a->z > INT64_MAX - b->z) ||
        (b->z < 0 && a->z < INT64_MIN - b->z))
    {
        a->Error = EV3_UB; b->Error = EV3_UB;
        return EV3_UB;
    }

    a->x += b->x;
    a->y += b->y;
    a->z += b->z;

    a->Error = EV3_SUCCESS;
    b->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3_Subtract(Vector3* a, Vector3* b)
{
    if (a == NULL || b == NULL)
        return EV3_NULLVEC;

    if ((b->x < 0 && a->x > INT64_MAX + b->x) ||
        (b->x > 0 && a->x < INT64_MIN + b->x))
    {
        a->Error = EV3_UB; b->Error = EV3_UB;
        return EV3_UB;
    }

    if ((b->y < 0 && a->y > INT64_MAX + b->y) ||
        (b->y > 0 && a->y < INT64_MIN + b->y))
    {
        a->Error = EV3_UB; b->Error = EV3_UB;
        return EV3_UB;
    }

    if ((b->z < 0 && a->z > INT64_MAX + b->z) ||
        (b->z > 0 && a->z < INT64_MIN + b->z))
    {
        a->Error = EV3_UB; b->Error = EV3_UB;
        return EV3_UB;
    }

    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;

    a->Error = EV3_SUCCESS;
    b->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3_Multiply(Vector3* a, Vector3* b)
{
    if (a == NULL || b == NULL)
        return EV3_NULLVEC;

    if (a->x != 0 && b->x != 0)
    {
        if (a->x > 0) {
            if (b->x > 0 && a->x > INT64_MAX / b->x) return EV3_UB;
            if (b->x < 0 && b->x < INT64_MIN / a->x) return EV3_UB;
        } else {
            if (b->x > 0 && a->x < INT64_MIN / b->x) return EV3_UB;
            if (b->x < 0 && (a->x == INT64_MIN || b->x == INT64_MIN || (-a->x) > INT64_MAX / (-b->x))) {
                a->Error = EV3_UB; b->Error = EV3_UB; return EV3_UB;
            }
        }
    }

    if (a->y != 0 && b->y != 0)
    {
        if (a->y > 0) {
            if (b->y > 0 && a->y > INT64_MAX / b->y) return EV3_UB;
            if (b->y < 0 && b->y < INT64_MIN / a->y) return EV3_UB;
        } else {
            if (b->y > 0 && a->y < INT64_MIN / b->y) return EV3_UB;
            if (b->y < 0 && (a->y == INT64_MIN || b->y == INT64_MIN || (-a->y) > INT64_MAX / (-b->y))) {
                a->Error = EV3_UB; b->Error = EV3_UB; return EV3_UB;
            }
        }
    }

    if (a->z != 0 && b->z != 0)
    {
        if (a->z > 0) {
            if (b->z > 0 && a->z > INT64_MAX / b->z) return EV3_UB;
            if (b->z < 0 && b->z < INT64_MIN / a->z) return EV3_UB;
        } else {
            if (b->z > 0 && a->z < INT64_MIN / b->z) return EV3_UB;
            if (b->z < 0 && (a->z == INT64_MIN || b->z == INT64_MIN || (-a->z) > INT64_MAX / (-b->z))) {
                a->Error = EV3_UB; b->Error = EV3_UB; return EV3_UB;
            }
        }
    }

    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;

    a->Error = EV3_SUCCESS;
    b->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3_Scale(Vector3* a, double scalar)
{
    if (a == NULL)
        return EV3_NULLVEC;

    if (isnan(scalar) || isinf(scalar))
    {
        a->Error = EV3_UB;
        return EV3_UB;
    }

    double scaled_x = (double)a->x * scalar;
    if (scaled_x < (double)INT64_MIN || scaled_x > (double)INT64_MAX)
    {
        a->Error = EV3_UB;
        return EV3_UB;
    }

    double scaled_y = (double)a->y * scalar;
    if (scaled_y < (double)INT64_MIN || scaled_y > (double)INT64_MAX)
    {
        a->Error = EV3_UB;
        return EV3_UB;
    }

    double scaled_z = (double)a->z * scalar;
    if (scaled_z < (double)INT64_MIN || scaled_z > (double)INT64_MAX)
    {
        a->Error = EV3_UB;
        return EV3_UB;
    }

    a->x = (int64_t)scaled_x;
    a->y = (int64_t)scaled_y;
    a->z = (int64_t)scaled_z;

    a->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3_Divide(Vector3* a, Vector3* b)
{
    if (a == NULL || b == NULL)
        return EV3_NULLVEC;

    if (b->x == 0 || (a->x == INT64_MIN && b->x == -1))
    {
        a->Error = EV3_UB; b->Error = EV3_UB;
        return EV3_UB;
    }

    if (b->y == 0 || (a->y == INT64_MIN && b->y == -1))
    {
        a->Error = EV3_UB; b->Error = EV3_UB;
        return EV3_UB;
    }

    if (b->z == 0 || (a->z == INT64_MIN && b->z == -1))
    {
        a->Error = EV3_UB; b->Error = EV3_UB;
        return EV3_UB;
    }

    a->x /= b->x;
    a->y /= b->y;
    a->z /= b->z;

    a->Error = EV3_SUCCESS;
    b->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3_Negate(Vector3* a)
{
    if (a == NULL)
        return EV3_NULLVEC;

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN)
    {
        a->Error = EV3_UB;
        return EV3_UB;
    }

    a->x *= -1;
    a->y *= -1;
    a->z *= -1;

    a->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

double Vector3_Length(Vector3* a, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL)
    {
        *Err = EV3_NULLVEC;
        return -2;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN)
    {
        a->Error = EV3_UB;
        *Err = EV3_UB;
        return -3;
    }

    a->Error = EV3_SUCCESS;
    *Err = EV3_SUCCESS;

    double dx = (double)a->x;
    double dy = (double)a->y;
    double dz = (double)a->z;

    return sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

double Vector3_LengthSquared(Vector3* a, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL)
    {
        *Err = EV3_NULLVEC;
        return -2;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN)
    {
        a->Error = EV3_UB;
        *Err = EV3_UB;
        return -3;
    }

    a->Error = EV3_SUCCESS;
    *Err = EV3_SUCCESS;

    return ((double)a->x * a->x) + ((double)a->y * a->y) + ((double)a->z * a->z);
}

double Vector3_Distance(Vector3* a, Vector3* b, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL || b == NULL)
    {
        *Err = EV3_NULLVEC;
        return -2;
    }

    if ((b->x < 0 && a->x > INT64_MAX + b->x) || (b->x > 0 && a->x < INT64_MIN + b->x))
    {
        a->Error = EV3_UB; b->Error = EV3_UB; *Err = EV3_UB; return -3;
    }

    if ((b->y < 0 && a->y > INT64_MAX + b->y) || (b->y > 0 && a->y < INT64_MIN + b->y))
    {
        a->Error = EV3_UB; b->Error = EV3_UB; *Err = EV3_UB; return -3;
    }

    if ((b->z < 0 && a->z > INT64_MAX + b->z) || (b->z > 0 && a->z < INT64_MIN + b->z))
    {
        a->Error = EV3_UB; b->Error = EV3_UB; *Err = EV3_UB; return -3;
    }

    int64_t dx = a->x - b->x;
    int64_t dy = a->y - b->y;
    int64_t dz = a->z - b->z;

    if (dx == INT64_MIN || dy == INT64_MIN || dz == INT64_MIN)
    {
        a->Error = EV3_UB; b->Error = EV3_UB; *Err = EV3_UB; return -3;
    }

    a->Error = EV3_SUCCESS;
    b->Error = EV3_SUCCESS;
    *Err = EV3_SUCCESS;

    double f_dx = (double)dx;
    double f_dy = (double)dy;
    double f_dz = (double)dz;

    return sqrt((f_dx * f_dx) + (f_dy * f_dy) + (f_dz * f_dz));
}

double Vector3_DistanceSquared(Vector3* a, Vector3* b, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL || b == NULL)
    {
        *Err = EV3_NULLVEC;
        return -2;
    }

    if ((b->x < 0 && a->x > INT64_MAX + b->x) || (b->x > 0 && a->x < INT64_MIN + b->x))
    {
        a->Error = EV3_UB; b->Error = EV3_UB; *Err = EV3_UB; return -3;
    }

    if ((b->y < 0 && a->y > INT64_MAX + b->y) || (b->y > 0 && a->y < INT64_MIN + b->y))
    {
        a->Error = EV3_UB; b->Error = EV3_UB; *Err = EV3_UB; return -3;
    }

    if ((b->z < 0 && a->z > INT64_MAX + b->z) || (b->z > 0 && a->z < INT64_MIN + b->z))
    {
        a->Error = EV3_UB; b->Error = EV3_UB; *Err = EV3_UB; return -3;
    }

    int64_t dx = a->x - b->x;
    int64_t dy = a->y - b->y;
    int64_t dz = a->z - b->z;

    if (dx == INT64_MIN || dy == INT64_MIN || dz == INT64_MIN)
    {
        a->Error = EV3_UB; b->Error = EV3_UB; *Err = EV3_UB; return -3;
    }

    a->Error = EV3_SUCCESS;
    b->Error = EV3_SUCCESS;
    *Err = EV3_SUCCESS;

    return ((double)dx * dx) + ((double)dy * dy) + ((double)dz * dz);
}

err Vector3_Normalize(Vector3* a, Vector3* out)
{
    if (a == NULL || out == NULL)
    {
        return EV3_NULLVEC;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN)
    {
        a->Error = EV3_UB;
        out->Error = EV3_UB;
        return EV3_UB;
    }

    double length = sqrt(((double)a->x * a->x) + ((double)a->y * a->y) + ((double)a->z * a->z));

    if (length == 0.0)
    {
        a->Error = EV3_UB;
        out->Error = EV3_UB;
        return EV3_UB;
    }

    out->x = (int64_t)(((double)a->x / length) * EV3_NORM_SCALE);
    out->y = (int64_t)(((double)a->y / length) * EV3_NORM_SCALE);
    out->z = (int64_t)(((double)a->z / length) * EV3_NORM_SCALE);

    a->Error = EV3_SUCCESS;
    out->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

double Vector3_Dot(Vector3* a, Vector3* b, err* Err)
{
    if (Err == NULL) return -1;
    if (a == NULL || b == NULL) { *Err = EV3_NULLVEC; return -2; }

    if (a->x != 0 && b->x != 0) {
        if (a->x > 0) {
            if (b->x > 0 && a->x > INT64_MAX / b->x) { *Err = EV3_UB; return -3; }
            if (b->x < 0 && b->x < INT64_MIN / a->x) { *Err = EV3_UB; return -3; }
        } else {
            if (b->x > 0 && a->x < INT64_MIN / b->x) { *Err = EV3_UB; return -3; }
            if (b->x < 0 && (a->x == INT64_MIN || b->x == INT64_MIN || (-a->x) > INT64_MAX / (-b->x))) { *Err = EV3_UB; return -3; }
        }
    }
    if (a->y != 0 && b->y != 0) {
        if (a->y > 0) {
            if (b->y > 0 && a->y > INT64_MAX / b->y) { *Err = EV3_UB; return -3; }
            if (b->y < 0 && b->y < INT64_MIN / a->y) { *Err = EV3_UB; return -3; }
        } else {
            if (b->y > 0 && a->y < INT64_MIN / b->y) { *Err = EV3_UB; return -3; }
            if (b->y < 0 && (a->y == INT64_MIN || b->y == INT64_MIN || (-a->y) > INT64_MAX / (-b->y))) { *Err = EV3_UB; return -3; }
        }
    }
    if (a->z != 0 && b->z != 0) {
        if (a->z > 0) {
            if (b->z > 0 && a->z > INT64_MAX / b->z) { *Err = EV3_UB; return -3; }
            if (b->z < 0 && b->z < INT64_MIN / a->z) { *Err = EV3_UB; return -3; }
        } else {
            if (b->z > 0 && a->z < INT64_MIN / b->z) { *Err = EV3_UB; return -3; }
            if (b->z < 0 && (a->z == INT64_MIN || b->z == INT64_MIN || (-a->z) > INT64_MAX / (-b->z))) { *Err = EV3_UB; return -3; }
        }
    }

    int64_t px = a->x * b->x;
    int64_t py = a->y * b->y;
    int64_t pz = a->z * b->z;

    if ((py > 0 && px > INT64_MAX - py) || (py < 0 && px < INT64_MIN - py)) { *Err = EV3_UB; return -3; }
    int64_t sum_xy = px + py;
    if ((pz > 0 && sum_xy > INT64_MAX - pz) || (pz < 0 && sum_xy < INT64_MIN - pz)) { *Err = EV3_UB; return -3; }

    a->Error = EV3_SUCCESS; b->Error = EV3_SUCCESS; *Err = EV3_SUCCESS;
    return (double)(sum_xy + pz);
}

err Vector3_Cross(Vector3* a, Vector3* b, Vector3* out)
{
    if (a == NULL || b == NULL || out == NULL) return EV3_NULLVEC;

    double cx = ((double)a->y * b->z) - ((double)a->z * b->y);
    double cy = ((double)a->z * b->x) - ((double)a->x * b->z);
    double cz = ((double)a->x * b->y) - ((double)a->y * b->x);

    if (cx < (double)INT64_MIN || cx > (double)INT64_MAX ||
        cy < (double)INT64_MIN || cy > (double)INT64_MAX ||
        cz < (double)INT64_MIN || cz > (double)INT64_MAX)
    {
        a->Error = EV3_UB; b->Error = EV3_UB; out->Error = EV3_UB;
        return EV3_UB;
    }

    out->x = (int64_t)cx;
    out->y = (int64_t)cy;
    out->z = (int64_t)cz;

    a->Error = EV3_SUCCESS; b->Error = EV3_SUCCESS; out->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

double Vector3_AngleBetween(Vector3* a, Vector3* b, err* Err)
{
    if (Err == NULL) return -1;
    if (a == NULL || b == NULL) { *Err = EV3_NULLVEC; return -2; }

    double dot = Vector3_Dot(a, b, Err);
    if (*Err != EV3_SUCCESS) return -3;

    double len_a = sqrt(((double)a->x * a->x) + ((double)a->y * a->y) + ((double)a->z * a->z));
    double len_b = sqrt(((double)b->x * b->x) + ((double)b->y * b->y) + ((double)b->z * b->z));

    if (len_a == 0.0 || len_b == 0.0) { *Err = EV3_UB; return -4; }

    double cos_theta = dot / (len_a * len_b);

    if (cos_theta > 1.0) cos_theta = 1.0;
    if (cos_theta < -1.0) cos_theta = -1.0;

    *Err = EV3_SUCCESS;
    return acos(cos_theta);
}

err Vector3_Rotate(Vector3* a, double radians, Vector3* out, err* Err)
{
    if (a == NULL || out == NULL)
    {
        if (Err != NULL) *Err = EV3_NULLVEC;
        return EV3_NULLVEC;
    }

    if (isnan(radians) || isinf(radians))
    {
        a->Error = EV3_UB;
        out->Error = EV3_UB;
        if (Err != NULL) *Err = EV3_UB;
        return EV3_UB;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN)
    {
        a->Error = EV3_UB;
        out->Error = EV3_UB;
        if (Err != NULL) *Err = EV3_UB;
        return EV3_UB;
    }

    double cos_r = cos(radians);
    double sin_r = sin(radians);

    double rx = ((double)a->x * cos_r) - ((double)a->y * sin_r);
    double ry = ((double)a->x * sin_r) + ((double)a->y * cos_r);
    double rz = (double)a->z;

    if (rx < (double)INT64_MIN || rx > (double)INT64_MAX ||
        ry < (double)INT64_MIN || ry > (double)INT64_MAX ||
        rz < (double)INT64_MIN || rz > (double)INT64_MAX)
    {
        a->Error = EV3_UB;
        out->Error = EV3_UB;
        if (Err != NULL) *Err = EV3_UB;
        return EV3_UB;
    }

    out->x = (int64_t)rx;
    out->y = (int64_t)ry;
    out->z = (int64_t)rz;

    a->Error = EV3_SUCCESS;
    out->Error = EV3_SUCCESS;
    if (Err != NULL) *Err = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3_Lerp(Vector3* a, Vector3* b, double t, Vector3* out, err* Err)
{
    if (a == NULL || b == NULL || out == NULL)
    {
        if (Err != NULL) *Err = EV3_NULLVEC;
        return EV3_NULLVEC;
    }

    if (isnan(t) || isinf(t))
    {
        a->Error = EV3_UB;
        b->Error = EV3_UB;
        out->Error = EV3_UB;
        if (Err != NULL) *Err = EV3_UB;
        return EV3_UB;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN || a->z == INT64_MIN ||
        b->x == INT64_MIN || b->y == INT64_MIN || b->z == INT64_MIN)
    {
        a->Error = EV3_UB;
        b->Error = EV3_UB;
        out->Error = EV3_UB;
        if (Err != NULL) *Err = EV3_UB;
        return EV3_UB;
    }

    double lx = (double)a->x + t * ((double)b->x - (double)a->x);
    double ly = (double)a->y + t * ((double)b->y - (double)a->y);
    double lz = (double)a->z + t * ((double)b->z - (double)a->z);

    if (lx < (double)INT64_MIN || lx > (double)INT64_MAX ||
        ly < (double)INT64_MIN || ly > (double)INT64_MAX ||
        lz < (double)INT64_MIN || lz > (double)INT64_MAX)
    {
        a->Error = EV3_UB;
        b->Error = EV3_UB;
        out->Error = EV3_UB;
        if (Err != NULL) *Err = EV3_UB;
        return EV3_UB;
    }

    out->x = (int64_t)lx;
    out->y = (int64_t)ly;
    out->z = (int64_t)lz;

    a->Error = EV3_SUCCESS;
    b->Error = EV3_SUCCESS;
    out->Error = EV3_SUCCESS;
    if (Err != NULL) *Err = EV3_SUCCESS;
    return EV3_SUCCESS;
}

Vector2* Vector3_Translate2D(Vector3* Vector)
{
    if (Vector == NULL)
    {
        return NULL;
    }

    if (Vector->z == 0)
    {
        Vector->Error = EV3_UB;
        return NULL;
    }

    if ((Vector->x == INT64_MIN && Vector->z == -1) ||
        (Vector->y == INT64_MIN && Vector->z == -1))
    {
        Vector->Error = EV3_UB;
        return NULL;
    }

    int64_t Nx = Vector->x / Vector->z;
    int64_t Ny = Vector->y / Vector->z;

    err E3err = EV3_SUCCESS;
    Vector2* NVector = Vector2_Create(Nx, Ny, &E3err);

    if (NVector == NULL || E3err != EV3_SUCCESS)
    {
        Vector->Error = EV3_CRTMALLOC;
        return NULL;
    }

    Vector->Error = EV3_SUCCESS;
    return NVector;
}

Vector3* Vector3_FromVector2(Vector2* v2, int64_t z, err* Err)
{
    if (v2 == NULL)
    {
        if (Err != NULL) *Err = EV3_NULLVEC;
        return NULL;
    }

    if (v2->x != 0 && z != 0)
    {
        if ((v2->x > 0 && z > 0 && v2->x > INT64_MAX / z) ||
            (v2->x < 0 && z < 0 && (v2->x == INT64_MIN || z == INT64_MIN || (-v2->x) > INT64_MAX / (-z))))
        {
            if (Err != NULL) *Err = EV3_UB;
            return NULL;
        }
    }

    if (v2->y != 0 && z != 0)
    {
        if ((v2->y > 0 && z > 0 && v2->y > INT64_MAX / z) ||
            (v2->y < 0 && z < 0 && (v2->y == INT64_MIN || z == INT64_MIN || (-v2->y) > INT64_MAX / (-z))))
        {
            if (Err != NULL) *Err = EV3_UB;
            return NULL;
        }
    }

    return Vector3_Create(v2->x * z, v2->y * z, z, Err);
}

err Vector3_Destroy(Vector3** Vector)
{
    if (Vector == NULL || *Vector == NULL)
    {
        return EV3_NULLVEC;
    }

    free(*Vector);
    *Vector = NULL;

    return EV3_SUCCESS;
}
