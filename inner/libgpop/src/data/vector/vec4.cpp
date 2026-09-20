#include <data/vector/vec4.hpp>
#include <cmath>
#include <algorithm>

namespace gpop { namespace Data { namespace Vector {

static inline bool __check_mul_overflow(int64_t va, int64_t vb) {
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

Vector4::Vector4(int64_t x, int64_t y, int64_t z, int64_t w) : x(x), y(y), z(z), w(w), Error(EV4_SUCCESS) {}

err Vector4::Set(int64_t x, int64_t y, int64_t z, int64_t w, bool xs, bool ys, bool zs, bool ws) {
    if (!xs) this->x = x;
    if (!ys) this->y = y;
    if (!zs) this->z = z;
    if (!ws) this->w = w;
    this->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4::Add(const Vector4& b) {
    if ((b.x > 0 && this->x > INT64_MAX - b.x) || (b.x < 0 && this->x < INT64_MIN - b.x) ||
        (b.y > 0 && this->y > INT64_MAX - b.y) || (b.y < 0 && this->y < INT64_MIN - b.y) ||
        (b.z > 0 && this->z > INT64_MAX - b.z) || (b.z < 0 && this->z < INT64_MIN - b.z) ||
        (b.w > 0 && this->w > INT64_MAX - b.w) || (b.w < 0 && this->w < INT64_MIN - b.w)) {
        this->Error = EV4_UB;
        return EV4_UB;
    }
    this->x += b.x; this->y += b.y; this->z += b.z; this->w += b.w;
    this->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4::Subtract(const Vector4& b) {
    if ((b.x < 0 && this->x > INT64_MAX + b.x) || (b.x > 0 && this->x < INT64_MIN + b.x) ||
        (b.y < 0 && this->y > INT64_MAX + b.y) || (b.y > 0 && this->y < INT64_MIN + b.y) ||
        (b.z < 0 && this->z > INT64_MAX + b.z) || (b.z > 0 && this->z < INT64_MIN + b.z) ||
        (b.w < 0 && this->w > INT64_MAX + b.w) || (b.w > 0 && this->w < INT64_MIN + b.w)) {
        this->Error = EV4_UB;
        return EV4_UB;
    }
    this->x -= b.x; this->y -= b.y; this->z -= b.z; this->w -= b.w;
    this->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4::Multiply(const Vector4& b) {
    if (__check_mul_overflow(this->x, b.x) || __check_mul_overflow(this->y, b.y) ||
        __check_mul_overflow(this->z, b.z) || __check_mul_overflow(this->w, b.w)) {
        this->Error = EV4_UB;
        return EV4_UB;
    }
    this->x *= b.x; this->y *= b.y; this->z *= b.z; this->w *= b.w;
    this->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4::Scale(double scalar) {
    if (std::isnan(scalar) || std::isinf(scalar)) { this->Error = EV4_UB; return EV4_UB; }
    double sx = (double)this->x * scalar, sy = (double)this->y * scalar, sz = (double)this->z * scalar, sw = (double)this->w * scalar;
    if (sx < (double)INT64_MIN || sx > (double)INT64_MAX || sy < (double)INT64_MIN || sy > (double)INT64_MAX ||
        sz < (double)INT64_MIN || sz > (double)INT64_MAX || sw < (double)INT64_MIN || sw > (double)INT64_MAX) {
        this->Error = EV4_UB; return EV4_UB;
    }
    this->x = (int64_t)sx; this->y = (int64_t)sy; this->z = (int64_t)sz; this->w = (int64_t)sw;
    this->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4::Divide(const Vector4& b) {
    if (b.x == 0 || b.y == 0 || b.z == 0 || b.w == 0 || (this->x == INT64_MIN && b.x == -1) ||
        (this->y == INT64_MIN && b.y == -1) || (this->z == INT64_MIN && b.z == -1) || (this->w == INT64_MIN && b.w == -1)) {
        this->Error = EV4_UB; return EV4_UB;
    }
    this->x /= b.x; this->y /= b.y; this->z /= b.z; this->w /= b.w;
    this->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4::Negate() {
    if (this->x == INT64_MIN || this->y == INT64_MIN || this->z == INT64_MIN || this->w == INT64_MIN) { this->Error = EV4_UB; return EV4_UB; }
    this->x = -this->x; this->y = -this->y; this->z = -this->z; this->w = -this->w;
    this->Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

double Vector4::Length(err* Err) const {
    if (!Err) return -1.0;
    if (this->x == INT64_MIN || this->y == INT64_MIN || this->z == INT64_MIN || this->w == INT64_MIN) { *Err = EV4_UB; return -3.0; }
    *Err = EV4_SUCCESS;
    return std::sqrt((double)this->x*this->x + (double)this->y*this->y + (double)this->z*this->z + (double)this->w*this->w);
}

double Vector4::LengthSquared(err* Err) const {
    if (!Err) return -1.0;
    if (this->x == INT64_MIN || this->y == INT64_MIN || this->z == INT64_MIN || this->w == INT64_MIN) { *Err = EV4_UB; return -3.0; }
    *Err = EV4_SUCCESS;
    return (double)this->x*this->x + (double)this->y*this->y + (double)this->z*this->z + (double)this->w*this->w;
}

double Vector4::Distance(const Vector4& b, err* Err) const {
    if (!Err) return -1.0;
    int64_t dx = this->x - b.x, dy = this->y - b.y, dz = this->z - b.z, dw = this->w - b.w;
    if (dx == INT64_MIN || dy == INT64_MIN || dz == INT64_MIN || dw == INT64_MIN) { *Err = EV4_UB; return -3.0; }
    *Err = EV4_SUCCESS;
    return std::sqrt((double)dx*dx + (double)dy*dy + (double)dz*dz + (double)dw*dw);
}

double Vector4::DistanceSquared(const Vector4& b, err* Err) const {
    if (!Err) return -1.0;
    int64_t dx = this->x - b.x, dy = this->y - b.y, dz = this->z - b.z, dw = this->w - b.w;
    if (dx == INT64_MIN || dy == INT64_MIN || dz == INT64_MIN || dw == INT64_MIN) { *Err = EV4_UB; return -3.0; }
    *Err = EV4_SUCCESS;
    return (double)dx*dx + (double)dy*dy + (double)dz*dz + (double)dw*dw;
}

err Vector4::Normalize(Vector4& out) const {
    if (this->x == INT64_MIN || this->y == INT64_MIN || this->z == INT64_MIN || this->w == INT64_MIN) { this->Error = out.Error = EV4_UB; return EV4_UB; }
    double len = std::sqrt((double)this->x*this->x + (double)this->y*this->y + (double)this->z*this->z + (double)this->w*this->w);
    if (len == 0.0) { this->Error = out.Error = EV4_UB; return EV4_UB; }
    out.x = (int64_t)((this->x / len) * 1000.0); out.y = (int64_t)((this->y / len) * 1000.0);
    out.z = (int64_t)((this->z / len) * 1000.0); out.w = (int64_t)((this->w / len) * 1000.0);
    this->Error = out.Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

double Vector4::Dot(const Vector4& b, err* Err) const {
    if (!Err) return -1.0;
    int64_t px = this->x * b.x, py = this->y * b.y, pz = this->z * b.z, pw = this->w * b.w;
    *Err = EV4_SUCCESS;
    return (double)(px + py + pz + pw);
}

err Vector4::Cross4D(const Vector4& b, const Vector4& c, Vector4& out) const {
    double cx =  (double)this->y * (b.z * c.w - b.w * c.z) - (double)this->z * (b.y * c.w - b.w * c.y) + (double)this->w * (b.y * c.z - b.z * c.y);
    double cy = -(double)this->x * (b.z * c.w - b.w * c.z) + (double)this->z * (b.x * c.w - b.w * c.x) - (double)this->w * (b.x * c.z - b.z * c.x);
    double cz =  (double)this->x * (b.y * c.w - b.w * c.y) - (double)this->y * (b.x * c.w - b.w * c.x) + (double)this->w * (b.x * c.y - b.y * c.x);
    double cw = -(double)this->x * (b.y * c.z - b.z * c.y) + (double)this->y * (b.x * c.z - b.z * c.x) - (double)this->z * (b.x * c.y - b.y * c.x);
    if (cx < (double)INT64_MIN || cx > (double)INT64_MAX || cy < (double)INT64_MIN || cy > (double)INT64_MAX || cz < (double)INT64_MIN || cz > (double)INT64_MAX || cw < (double)INT64_MIN || cw > (double)INT64_MAX) {
        this->Error = out.Error = EV4_UB; return EV4_UB;
    }
    out.x = (int64_t)cx; out.y = (int64_t)cy; out.z = (int64_t)cz; out.w = (int64_t)cw;
    this->Error = out.Error = EV4_SUCCESS;
    return EV4_SUCCESS;
}

double Vector4::AngleBetween(const Vector4& b, err* Err) const {
    if (!Err) return -1.0;
    double dot = this->Dot(b, Err);
    if (*Err != EV4_SUCCESS) return -3.0;
    double lenA = std::sqrt((double)this->x*this->x + (double)this->y*this->y + (double)this->z*this->z + (double)this->w*this->w);
    double lenB = std::sqrt((double)b.x*b.x + (double)b.y*b.y + (double)b.z*b.z + (double)b.w*b.w);
    if (lenA == 0.0 || lenB == 0.0) { *Err = EV4_UB; return -4.0; }
    double cosTheta = dot / (lenA * lenB);
    *Err = EV4_SUCCESS;
    return std::acos(std::max(-1.0, std::min(1.0, cosTheta)));
}

err Vector4::Rotate(double radians, Vector4& out, err* Err) const {
    double cosR = std::cos(radians), sinR = std::sin(radians);
    double rx = (double)this->x * cosR - (double)this->y * sinR;
    double ry = (double)this->x * sinR + (double)this->y * cosR;
    if (rx < (double)INT64_MIN || rx > (double)INT64_MAX || ry < (double)INT64_MIN || ry > (double)INT64_MAX) {
        this->Error = out.Error = EV4_UB; if (Err) *Err = EV4_UB; return EV4_UB;
    }
    out.x = (int64_t)rx; out.y = (int64_t)ry; out.z = this->z; out.w = this->w;
    this->Error = out.Error = EV4_SUCCESS; if (Err) *Err = EV4_SUCCESS;
    return EV4_SUCCESS;
}

err Vector4::Lerp(const Vector4& b, double t, Vector4& out, err* Err) const {
    double lx = (double)this->x + t * ((double)b.x - (double)this->x);
    double ly = (double)this->y + t * ((double)b.y - (double)this->y);
    double lz = (double)this->z + t * ((double)b.z - (double)this->z);
    double lw = (double)this->w + t * ((double)b.w - (double)this->w);
    if (lx < (double)INT64_MIN || lx > (double)INT64_MAX || ly < (double)INT64_MIN || ly > (double)INT64_MAX || lz < (double)INT64_MIN || lz > (double)INT64_MAX || lw < (double)INT64_MIN || lw > (double)INT64_MAX) {
        this->Error = out.Error = EV4_UB; if (Err) *Err = EV4_UB; return EV4_UB;
    }
    out.x = (int64_t)lx; out.y = (int64_t)ly; out.z = (int64_t)lz; out.w = (int64_t)lw;
    this->Error = out.Error = EV4_SUCCESS; if (Err) *Err = EV4_SUCCESS;
    return EV4_SUCCESS;
}

Vector3* Vector4::Translate3D() {
    int64_t x = this->x, y = this->y, z = this->z;
    if (this->w != 0) {
        if (this->w == 0 || (this->w == -1 && (x == INT64_MIN || y == INT64_MIN || z == INT64_MIN))) {
            this->Error = EV4_UB; return nullptr;
        }
        x /= this->w; y /= this->w; z /= this->w;
    }
    this->Error = EV4_SUCCESS;
    return new Vector3(x, y, z);
}

Vector4* Vector4::FromVector3(const Vector3& v3, int64_t w) {
    return new Vector4(v3.x * w, v3.y * w, v3.z * w, w);
}

}}}
