#include <data/vector/vec2.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EV2_NORM_SCALE 1000

Vector2* Vector2_Create(int64_t x, int64_t y, err* Err)
{
    Vector2 *vector = calloc(1, sizeof(Vector2));

    if (vector == NULL)
    {
        if (Err != NULL)
            *Err = EV2_CRTMALLOC;
        return NULL;
    }

    vector->x = x;
    vector->y = y;
    vector->Error = EV2_SUCCESS;
    if (Err != NULL)
        *Err = EV2_SUCCESS;
    return vector;
}

err Vector2_Set(Vector2* Vector, int64_t x, int64_t y, bool xs, bool ys)
{
    if (Vector == NULL)
        return EV2_NULLVEC;

    Vector->x = (x * !xs) + (Vector->x * xs);
    Vector->y = (y * !ys) + (Vector->y * ys);

    Vector->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2_Add(Vector2* a, Vector2* b)
{
    if (a == NULL || b == NULL)
        return EV2_NULLVEC;

    if ((b->x > 0 && a->x > INT64_MAX - b->x) ||
        (b->x < 0 && a->x < INT64_MIN - b->x))
    {
        a->Error = EV2_OVERFLOW;
        b->Error = EV2_OVERFLOW;
        return EV2_OVERFLOW;
    }

    if ((b->y > 0 && a->y > INT64_MAX - b->y) ||
        (b->y < 0 && a->y < INT64_MIN - b->y))
    {
        a->Error = EV2_OVERFLOW;
        b->Error = EV2_OVERFLOW;
        return EV2_OVERFLOW;
    }

    a->x += b->x;
    a->y += b->y;

    a->Error = EV2_SUCCESS;
    b->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}


err Vector2_Subtract(Vector2* a, Vector2* b)
{
    if (a == NULL || b == NULL)
        return EV2_NULLVEC;

    if ((b->x < 0 && a->x > INT64_MAX + b->x) ||
        (b->x > 0 && a->x < INT64_MIN + b->x))
    {
        a->Error = EV2_UNDERFLOW;
        b->Error = EV2_UNDERFLOW;
        return EV2_UNDERFLOW;
    }

    if ((b->y < 0 && a->y > INT64_MAX + b->y) ||
        (b->y > 0 && a->y < INT64_MIN + b->y))
    {
        a->Error = EV2_UNDERFLOW;
        b->Error = EV2_UNDERFLOW;
        return EV2_UNDERFLOW;
    }

    a->x -= b->x;
    a->y -= b->y;

    a->Error = EV2_SUCCESS;
    b->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2_Multiply(Vector2* a, Vector2* b)
{
    if (a == NULL || b == NULL)
        return EV2_NULLVEC;

    if (a->x != 0 && b->x != 0)
    {
        if (a->x > 0) {
            if (b->x > 0 && a->x > INT64_MAX / b->x) return EV2_OVERFLOW;
            if (b->x < 0 && b->x < INT64_MIN / a->x) return EV2_OVERFLOW;
        } else {
            if (b->x > 0 && a->x < INT64_MIN / b->x) return EV2_OVERFLOW;
            if (b->x < 0 && (a->x == INT64_MIN || b->x == INT64_MIN || (-a->x) > INT64_MAX / (-b->x))) {
                a->Error = EV2_OVERFLOW; b->Error = EV2_OVERFLOW; return EV2_OVERFLOW;
            }
        }
    }

    if (a->y != 0 && b->y != 0)
    {
        if (a->y > 0) {
            if (b->y > 0 && a->y > INT64_MAX / b->y) return EV2_OVERFLOW;
            if (b->y < 0 && b->y < INT64_MIN / a->y) return EV2_OVERFLOW;
        } else {
            if (b->y > 0 && a->y < INT64_MIN / b->y) return EV2_OVERFLOW;
            if (b->y < 0 && (a->y == INT64_MIN || b->y == INT64_MIN || (-a->y) > INT64_MAX / (-b->y))) {
                a->Error = EV2_OVERFLOW; b->Error = EV2_OVERFLOW; return EV2_OVERFLOW;
            }
        }
    }

    a->x *= b->x;
    a->y *= b->y;

    a->Error = EV2_SUCCESS;
    b->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2_Scale(Vector2* a, double scalar)
{
    if (a == NULL)
        return EV2_NULLVEC;

    if (isnan(scalar) || isinf(scalar))
    {
        a->Error = EV2_OVERFLOW;
        return EV2_OVERFLOW;
    }

    double scaled_x = (double)a->x * scalar;
    if (scaled_x < (double)INT64_MIN || scaled_x > (double)INT64_MAX)
    {
        a->Error = EV2_OVERFLOW;
        return EV2_OVERFLOW;
    }

    double scaled_y = (double)a->y * scalar;
    if (scaled_y < (double)INT64_MIN || scaled_y > (double)INT64_MAX)
    {
        a->Error = EV2_OVERFLOW;
        return EV2_OVERFLOW;
    }

    a->x = (int64_t)scaled_x;
    a->y = (int64_t)scaled_y;

    a->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2_Divide(Vector2* a, Vector2* b)
{
    if (a == NULL || b == NULL)
        return EV2_NULLVEC;

    if (b->x == 0)
    {
        a->Error = EV2_DIVBY0; b->Error = EV2_DIVBY0;
        return EV2_DIVBY0;
    }
    if (a->x == INT64_MIN && b->x == -1)
    {
        a->Error = EV2_DIVBY0; b->Error = EV2_DIVBY0;
        return EV2_DIVBY0;
    }

    if (b->y == 0)
    {
        a->Error = EV2_DIVBY0; b->Error = EV2_DIVBY0;
        return EV2_DIVBY0;
    }
    if (a->y == INT64_MIN && b->y == -1)
    {
        a->Error = EV2_DIVBY0; b->Error = EV2_DIVBY0;
        return EV2_DIVBY0;
    }

    a->x /= b->x;
    a->y /= b->y;

    a->Error = EV2_SUCCESS;
    b->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}


err Vector2_Negate(Vector2* a)
{
    if (a == NULL)
        return EV2_NULLVEC;

    if (a->x == INT64_MIN || a->y == INT64_MIN)
    {
        a->Error = EV2_UNDERFLOW;
        return EV2_UNDERFLOW;
    }

    a->x *= -1;
    a->y *= -1;

    a->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

double Vector2_Length(Vector2* a, err* Err)
{
    if(Err == NULL) return -1;
    if (a == NULL)
        {*Err = EV2_NULLVEC; return -2;}

    if (a->x == INT64_MIN || a->y == INT64_MIN)
    {
        a->Error = EV2_UNDERFLOW;
        *Err = EV2_UNDERFLOW;
        return -3;
    }

    return sqrt(pow(a->x, 2) + pow(a->y, 2));
}

double Vector2_LengthSquared(Vector2* a, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL)
    {
        *Err = EV2_NULLVEC;
        return -2;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN)
    {
        a->Error = EV2_OVERFLOW;
        *Err = EV2_OVERFLOW;
        return -3;
    }

    a->Error = EV2_SUCCESS;
    *Err = EV2_SUCCESS;

    return ((double)a->x * a->x) + ((double)a->y * a->y);
}

double Vector2_Distance(Vector2* a, Vector2* b, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL || b == NULL)
    {
        *Err = EV2_NULLVEC;
        return -2;
    }

    if ((b->x < 0 && a->x > INT64_MAX + b->x) ||
        (b->x > 0 && a->x < INT64_MIN + b->x))
    {
        a->Error = EV2_UB;
        b->Error = EV2_UB;
        *Err = EV2_UB;
        return -3;
    }

    if ((b->y < 0 && a->y > INT64_MAX + b->y) ||
        (b->y > 0 && a->y < INT64_MIN + b->y))
    {
        a->Error = EV2_UB;
        b->Error = EV2_UB;
        *Err = EV2_UB;
        return -3;
    }

    int64_t dx = a->x - b->x;
    int64_t dy = a->y - b->y;

    if (dx == INT64_MIN || dy == INT64_MIN)
    {
        a->Error = EV2_UB;
        b->Error = EV2_UB;
        *Err = EV2_UB;
        return -3;
    }

    a->Error = EV2_SUCCESS;
    b->Error = EV2_SUCCESS;
    *Err = EV2_SUCCESS;

    return hypot((double)dx, (double)dy);
}

double Vector2_DistanceSquared(Vector2* a, Vector2* b, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL || b == NULL)
    {
        *Err = EV2_NULLVEC;
        return -2;
    }

    if ((b->x < 0 && a->x > INT64_MAX + b->x) ||
        (b->x > 0 && a->x < INT64_MIN + b->x))
    {
        a->Error = EV2_UB;
        b->Error = EV2_UB;
        *Err = EV2_UB;
        return -3;
    }

    if ((b->y < 0 && a->y > INT64_MAX + b->y) ||
        (b->y > 0 && a->y < INT64_MIN + b->y))
    {
        a->Error = EV2_UB;
        b->Error = EV2_UB;
        *Err = EV2_UB;
        return -3;
    }

    int64_t dx = a->x - b->x;
    int64_t dy = a->y - b->y;

    if (dx == INT64_MIN || dy == INT64_MIN)
    {
        a->Error = EV2_UB;
        b->Error = EV2_UB;
        *Err = EV2_UB;
        return -3;
    }

    a->Error = EV2_SUCCESS;
    b->Error = EV2_SUCCESS;
    *Err = EV2_SUCCESS;

    return ((double)dx * dx) + ((double)dy * dy);
}

err Vector2_Normalize(Vector2* a, int64_t* out_x, int64_t* out_y)
{

    if (a == NULL || out_x == NULL || out_y == NULL)
    {
        return -1;
    }

    if (a->x == INT64_MIN || a->y == INT64_MIN)
    {
        a->Error = EV2_UB;
        return -2;
    }

    double length = hypot((double)a->x, (double)a->y);

    if (length == 0.0)
    {
        a->Error = EV2_UB;
        return -3;
    }

    *out_x = (int64_t)(((double)a->x / length) * EV2_NORM_SCALE);
    *out_y = (int64_t)(((double)a->y / length) * EV2_NORM_SCALE);

    a->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

double Vector2_Dot(Vector2* a, Vector2* b, err* Err)
{
    if (Err == NULL) return -1;

    if (a == NULL || b == NULL)
    {
        *Err = EV2_NULLVEC;
        return -2;
    }

    if (a->x != 0 && b->x != 0)
    {
        if (a->x > 0) {
            if (b->x > 0 && a->x > INT64_MAX / b->x) { *Err = EV2_UB; return -3; }
            if (b->x < 0 && b->x < INT64_MIN / a->x) { *Err = EV2_UB; return -3; }
        } else {
            if (b->x > 0 && a->x < INT64_MIN / b->x) { *Err = EV2_UB; return -3; }
            if (b->x < 0 && (a->x == INT64_MIN || b->x == INT64_MIN || (-a->x) > INT64_MAX / (-b->x))) {
                *Err = EV2_UB; return -3;
            }
        }
    }

    if (a->y != 0 && b->y != 0)
    {
        if (a->y > 0) {
            if (b->y > 0 && a->y > INT64_MAX / b->y) { *Err = EV2_UB; return -3; }
            if (b->y < 0 && b->y < INT64_MIN / a->y) { *Err = EV2_UB; return -3; }
        } else {
            if (b->y > 0 && a->y < INT64_MIN / b->y) { *Err = EV2_UB; return -3; }
            if (b->y < 0 && (a->y == INT64_MIN || b->y == INT64_MIN || (-a->y) > INT64_MAX / (-b->y))) {
                *Err = EV2_UB; return -3;
            }
        }
    }

    int64_t prod_x = a->x * b->x;
    int64_t prod_y = a->y * b->y;

    if ((prod_y > 0 && prod_x > INT64_MAX - prod_y) ||
        (prod_y < 0 && prod_x < INT64_MIN - prod_y))
    {
        a->Error = EV2_UB;
        b->Error = EV2_UB;
        *Err = EV2_UB;
        return -3;
    }

    a->Error = EV2_SUCCESS;
    b->Error = EV2_SUCCESS;
    *Err = EV2_SUCCESS;

    return (double)(prod_x + prod_y);
}


err Vector2_Destroy(Vector2** Vector)
{
    if (Vector == NULL || *Vector == NULL)
        return EV2_SUCCESS;

    (*Vector)->x = 0;
    (*Vector)->y = 0;
    (*Vector)->Error = 0;

    free(*Vector);

    *Vector = NULL;

    return EV2_SUCCESS;
}
