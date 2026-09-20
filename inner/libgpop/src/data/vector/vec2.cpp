#include <data/vector/vec2.hpp>
#include <cmath>
#include <algorithm>

namespace gpop { namespace Data { namespace Vector {

Vector2::Vector2(int64_t x, int64_t y) : x(x), y(y), Error(EV2_SUCCESS) {}

err Vector2::Set(int64_t x, int64_t y, bool xs, bool ys) {
    this->x = (x * !xs) + (this->x * xs);
    this->y = (y * !ys) + (this->y * ys);
    this->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2::Add(const Vector2& b) {
    if ((b.x > 0 && this->x > INT64_MAX - b.x) ||
        (b.x < 0 && this->x < INT64_MIN - b.x) ||
        (b.y > 0 && this->y > INT64_MAX - b.y) ||
        (b.y < 0 && this->y < INT64_MIN - b.y)) {
        this->Error = EV2_OVERFLOW;
        return EV2_OVERFLOW;
    }
    this->x += b.x;
    this->y += b.y;
    this->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2::Subtract(const Vector2& b) {
    if ((b.x < 0 && this->x > INT64_MAX + b.x) ||
        (b.x > 0 && this->x < INT64_MIN + b.x) ||
        (b.y < 0 && this->y > INT64_MAX + b.y) ||
        (b.y > 0 && this->y < INT64_MIN + b.y)) {
        this->Error = EV2_UNDERFLOW;
        return EV2_UNDERFLOW;
    }
    this->x -= b.x;
    this->y -= b.y;
    this->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2::Multiply(const Vector2& b) {
    if (this->x != 0 && b.x != 0) {
        if (this->x > 0) {
            if (b.x > 0 && this->x > INT64_MAX / b.x) return EV2_OVERFLOW;
            if (b.x < 0 && b.x < INT64_MIN / this->x) return EV2_OVERFLOW;
        } else {
            if (b.x > 0 && this->x < INT64_MIN / b.x) return EV2_OVERFLOW;
            if (b.x < 0 && (this->x == INT64_MIN || b.x == INT64_MIN || (-this->x) > INT64_MAX / (-b.x))) {
                this->Error = EV2_OVERFLOW; return EV2_OVERFLOW;
            }
        }
    }

    if (this->y != 0 && b.y != 0) {
        if (this->y > 0) {
            if (b.y > 0 && this->y > INT64_MAX / b.y) return EV2_OVERFLOW;
            if (b.y < 0 && b.y < INT64_MIN / this->y) return EV2_OVERFLOW;
        } else {
            if (b.y > 0 && this->y < INT64_MIN / b.y) return EV2_OVERFLOW;
            if (b.y < 0 && (this->y == INT64_MIN || b.y == INT64_MIN || (-this->y) > INT64_MAX / (-b.y))) {
                this->Error = EV2_OVERFLOW; return EV2_OVERFLOW;
            }
        }
    }

    this->x *= b.x;
    this->y *= b.y;
    this->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2::Scale(double scalar) {
    if (std::isnan(scalar) || std::isinf(scalar)) {
        this->Error = EV2_OVERFLOW;
        return EV2_OVERFLOW;
    }
    double scaled_x = (double)this->x * scalar;
    double scaled_y = (double)this->y * scalar;
    if (scaled_x < (double)INT64_MIN || scaled_x > (double)INT64_MAX ||
        scaled_y < (double)INT64_MIN || scaled_y > (double)INT64_MAX) {
        this->Error = EV2_OVERFLOW;
        return EV2_OVERFLOW;
    }
    this->x = (int64_t)scaled_x;
    this->y = (int64_t)scaled_y;
    this->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2::Divide(const Vector2& b) {
    if (b.x == 0 || b.y == 0 || (this->x == INT64_MIN && b.x == -1) || (this->y == INT64_MIN && b.y == -1)) {
        this->Error = EV2_DIVBY0;
        return EV2_DIVBY0;
    }
    this->x /= b.x;
    this->y /= b.y;
    this->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

err Vector2::Negate() {
    if (this->x == INT64_MIN || this->y == INT64_MIN) {
        this->Error = EV2_UNDERFLOW;
        return EV2_UNDERFLOW;
    }
    this->x *= -1;
    this->y *= -1;
    this->Error = EV2_SUCCESS;
    return EV2_SUCCESS;
}

double Vector2::Length(err* Err) const {
    if (!Err) return -1;
    if (this->x == INT64_MIN || this->y == INT64_MIN) {
        *Err = EV2_UNDERFLOW;
        return -3;
    }
    *Err = EV2_SUCCESS;
    return std::hypot((double)this->x, (double)this->y);
}

double Vector2::LengthSquared(err* Err) const {
    if (!Err) return -1;
    if (this->x == INT64_MIN || this->y == INT64_MIN) {
        *Err = EV2_OVERFLOW;
        return -3;
    }
    *Err = EV2_SUCCESS;
    return ((double)this->x * this->x) + ((double)this->y * this->y);
}

double Vector2::Distance(const Vector2& b, err* Err) const {
    if (!Err) return -1;
    int64_t dx = this->x - b.x;
    int64_t dy = this->y - b.y;
    if (dx == INT64_MIN || dy == INT64_MIN) {
        *Err = EV2_UB;
        return -3;
    }
    *Err = EV2_SUCCESS;
    return std::hypot((double)dx, (double)dy);
}

double Vector2::DistanceSquared(const Vector2& b, err* Err) const {
    if (!Err) return -1;
    int64_t dx = this->x - b.x;
    int64_t dy = this->y - b.y;
    if (dx == INT64_MIN || dy == INT64_MIN) {
        *Err = EV2_UB;
        return -3;
    }
    *Err = EV2_SUCCESS;
    return ((double)dx * dx) + ((double)dy * dy);
}

err Vector2::Normalize(int64_t* out_x, int64_t* out_y) const {
    if (!out_x || !out_y) return -1;
    if (this->x == INT64_MIN || this->y == INT64_MIN) return -2;
    double length = std::hypot((double)this->x, (double)this->y);
    if (length == 0.0) return -3;
    *out_x = (int64_t)((this->x / length) * 1000.0);
    *out_y = (int64_t)((this->y / length) * 1000.0);
    return EV2_SUCCESS;
}

double Vector2::Dot(const Vector2& b, err* Err) const {
    if (!Err) return -1;

    // Check multiplication overflows
    int64_t prod_x = 0;
    int64_t prod_y = 0;

    // Safe multiply x
    if (this->x != 0 && b.x != 0) {
        if ((this->x > 0 && b.x > 0 && this->x > INT64_MAX / b.x) ||
            (this->x > 0 && b.x < 0 && b.x < INT64_MIN / this->x) ||
            (this->x < 0 && b.x > 0 && this->x < INT64_MIN / b.x) ||
            (this->x < 0 && b.x < 0 && (this->x == INT64_MIN || b.x == INT64_MIN || (-this->x) > INT64_MAX / (-b.x)))) {
            *Err = EV2_UB; return -3;
        }
        prod_x = this->x * b.x;
    }

    // Safe multiply y
    if (this->y != 0 && b.y != 0) {
        if ((this->y > 0 && b.y > 0 && this->y > INT64_MAX / b.y) ||
            (this->y > 0 && b.y < 0 && b.y < INT64_MIN / this->y) ||
            (this->y < 0 && b.y > 0 && this->y < INT64_MIN / b.y) ||
            (this->y < 0 && b.y < 0 && (this->y == INT64_MIN || b.y == INT64_MIN || (-this->y) > INT64_MAX / (-b.y)))) {
            *Err = EV2_UB; return -3;
        }
        prod_y = this->y * b.y;
    }

    // Safe add
    if ((prod_y > 0 && prod_x > INT64_MAX - prod_y) ||
        (prod_y < 0 && prod_x < INT64_MIN - prod_y)) {
        *Err = EV2_UB; return -3;
    }

    *Err = EV2_SUCCESS;
    return (double)(prod_x + prod_y);
}


}}}
