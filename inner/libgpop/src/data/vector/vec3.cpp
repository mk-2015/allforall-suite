#include <data/vector/vec3.hpp>
#include <cmath>
#include <algorithm>

namespace gpop { namespace Data { namespace Vector {

Vector3::Vector3(int64_t x, int64_t y, int64_t z) : x(x), y(y), z(z), Error(EV3_SUCCESS) {}

err Vector3::Set(int64_t x, int64_t y, int64_t z, bool xs, bool ys, bool zs) {
    if(!xs) this->x = x;
    if(!ys) this->y = y;
    if(!zs) this->z = z;
    this->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3::Add(const Vector3& b) {
    if ((b.x > 0 && this->x > INT64_MAX - b.x) || (b.x < 0 && this->x < INT64_MIN - b.x) ||
        (b.y > 0 && this->y > INT64_MAX - b.y) || (b.y < 0 && this->y < INT64_MIN - b.y) ||
        (b.z > 0 && this->z > INT64_MAX - b.z) || (b.z < 0 && this->z < INT64_MIN - b.z)) {
        this->Error = EV3_UB;
        return EV3_UB;
    }
    this->x += b.x; this->y += b.y; this->z += b.z;
    this->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3::Subtract(const Vector3& b) {
    if ((b.x < 0 && this->x > INT64_MAX + b.x) || (b.x > 0 && this->x < INT64_MIN + b.x) ||
        (b.y < 0 && this->y > INT64_MAX + b.y) || (b.y > 0 && this->y < INT64_MIN + b.y) ||
        (b.z < 0 && this->z > INT64_MAX + b.z) || (b.z > 0 && this->z < INT64_MIN + b.z)) {
        this->Error = EV3_UB;
        return EV3_UB;
    }
    this->x -= b.x; this->y -= b.y; this->z -= b.z;
    this->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3::Multiply(const Vector3& b) {
    if ((this->x != 0 && b.x != 0 && ( (this->x>0 && b.x>0 && this->x>INT64_MAX/b.x) || (this->x>0 && b.x<0 && b.x<INT64_MIN/this->x) || (this->x<0 && b.x>0 && this->x<INT64_MIN/b.x) || (this->x<0 && b.x<0 && (this->x==INT64_MIN || b.x==INT64_MIN || (-this->x)>INT64_MAX/(-b.x))) )) ||
        (this->y != 0 && b.y != 0 && ( (this->y>0 && b.y>0 && this->y>INT64_MAX/b.y) || (this->y>0 && b.y<0 && b.y<INT64_MIN/this->y) || (this->y<0 && b.y>0 && this->y<INT64_MIN/b.y) || (this->y<0 && b.y<0 && (this->y==INT64_MIN || b.y==INT64_MIN || (-this->y)>INT64_MAX/(-b.y))) )) ||
        (this->z != 0 && b.z != 0 && ( (this->z>0 && b.z>0 && this->z>INT64_MAX/b.z) || (this->z>0 && b.z<0 && b.z<INT64_MIN/this->z) || (this->z<0 && b.z>0 && this->z<INT64_MIN/b.z) || (this->z<0 && b.z<0 && (this->z==INT64_MIN || b.z==INT64_MIN || (-this->z)>INT64_MAX/(-b.z))) ))) {
        this->Error = EV3_UB; return EV3_UB;
    }
    this->x *= b.x; this->y *= b.y; this->z *= b.z;
    this->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3::Scale(double scalar) {
    if (std::isnan(scalar) || std::isinf(scalar)) { this->Error = EV3_UB; return EV3_UB; }
    double sx = (double)this->x * scalar, sy = (double)this->y * scalar, sz = (double)this->z * scalar;
    if (sx < (double)INT64_MIN || sx > (double)INT64_MAX || sy < (double)INT64_MIN || sy > (double)INT64_MAX || sz < (double)INT64_MIN || sz > (double)INT64_MAX) {
        this->Error = EV3_UB; return EV3_UB;
    }
    this->x = (int64_t)sx; this->y = (int64_t)sy; this->z = (int64_t)sz;
    this->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3::Divide(const Vector3& b) {
    if (b.x == 0 || b.y == 0 || b.z == 0 || (this->x == INT64_MIN && b.x == -1) || (this->y == INT64_MIN && b.y == -1) || (this->z == INT64_MIN && b.z == -1)) {
        this->Error = EV3_UB; return EV3_UB;
    }
    this->x /= b.x; this->y /= b.y; this->z /= b.z;
    this->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3::Negate() {
    if (this->x == INT64_MIN || this->y == INT64_MIN || this->z == INT64_MIN) { this->Error = EV3_UB; return EV3_UB; }
    this->x *= -1; this->y *= -1; this->z *= -1;
    this->Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

double Vector3::Length(err* Err) const {
    if (!Err) return -1;
    if (this->x == INT64_MIN || this->y == INT64_MIN || this->z == INT64_MIN) { *Err = EV3_UB; return -3; }
    *Err = EV3_SUCCESS;
    return std::sqrt((double)this->x * this->x + (double)this->y * this->y + (double)this->z * this->z);
}

double Vector3::LengthSquared(err* Err) const {
    if (!Err) return -1;
    if (this->x == INT64_MIN || this->y == INT64_MIN || this->z == INT64_MIN) { *Err = EV3_UB; return -3; }
    *Err = EV3_SUCCESS;
    return (double)this->x * this->x + (double)this->y * this->y + (double)this->z * this->z;
}

double Vector3::Distance(const Vector3& b, err* Err) const {
    if (!Err) return -1;
    int64_t dx = this->x - b.x, dy = this->y - b.y, dz = this->z - b.z;
    if (dx == INT64_MIN || dy == INT64_MIN || dz == INT64_MIN) { *Err = EV3_UB; return -3; }
    *Err = EV3_SUCCESS;
    return std::sqrt((double)dx*dx + (double)dy*dy + (double)dz*dz);
}

double Vector3::DistanceSquared(const Vector3& b, err* Err) const {
    if (!Err) return -1;
    int64_t dx = this->x - b.x, dy = this->y - b.y, dz = this->z - b.z;
    if (dx == INT64_MIN || dy == INT64_MIN || dz == INT64_MIN) { *Err = EV3_UB; return -3; }
    *Err = EV3_SUCCESS;
    return (double)dx*dx + (double)dy*dy + (double)dz*dz;
}

err Vector3::Normalize(Vector3& out) const {
    if (this->x == INT64_MIN || this->y == INT64_MIN || this->z == INT64_MIN) { this->Error = EV3_UB; out.Error = EV3_UB; return EV3_UB; }
    double len = std::sqrt((double)this->x*this->x + (double)this->y*this->y + (double)this->z*this->z);
    if (len == 0.0) { this->Error = EV3_UB; out.Error = EV3_UB; return EV3_UB; }
    out.x = (int64_t)((this->x / len) * 1000.0); out.y = (int64_t)((this->y / len) * 1000.0); out.z = (int64_t)((this->z / len) * 1000.0);
    this->Error = out.Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

double Vector3::Dot(const Vector3& b, err* Err) const {
    if (!Err) return -1;
    int64_t px = this->x * b.x, py = this->y * b.y, pz = this->z * b.z;
    *Err = EV3_SUCCESS;
    return (double)(px + py + pz);
}

err Vector3::Cross(const Vector3& b, Vector3& out) const {
    double cx = (double)this->y * b.z - (double)this->z * b.y;
    double cy = (double)this->z * b.x - (double)this->x * b.z;
    double cz = (double)this->x * b.y - (double)this->y * b.x;
    if (cx < (double)INT64_MIN || cx > (double)INT64_MAX || cy < (double)INT64_MIN || cy > (double)INT64_MAX || cz < (double)INT64_MIN || cz > (double)INT64_MAX) {
        this->Error = out.Error = EV3_UB; return EV3_UB;
    }
    out.x = (int64_t)cx; out.y = (int64_t)cy; out.z = (int64_t)cz;
    this->Error = out.Error = EV3_SUCCESS;
    return EV3_SUCCESS;
}

double Vector3::AngleBetween(const Vector3& b, err* Err) const {
    if (!Err) return -1;
    double dot = this->Dot(b, Err);
    if (*Err != EV3_SUCCESS) return -3;
    double lenA = std::sqrt((double)this->x*this->x + (double)this->y*this->y + (double)this->z*this->z);
    double lenB = std::sqrt((double)b.x*b.x + (double)b.y*b.y + (double)b.z*b.z);
    if (lenA == 0.0 || lenB == 0.0) { *Err = EV3_UB; return -4; }
    double cosTheta = dot / (lenA * lenB);
    *Err = EV3_SUCCESS;
    return std::acos(std::max(-1.0, std::min(1.0, cosTheta)));
}

err Vector3::Rotate(double radians, Vector3& out, err* Err) const {
    double cosR = std::cos(radians), sinR = std::sin(radians);
    double rx = (double)this->x * cosR - (double)this->y * sinR;
    double ry = (double)this->x * sinR + (double)this->y * cosR;
    double rz = (double)this->z;
    if (rx < (double)INT64_MIN || rx > (double)INT64_MAX || ry < (double)INT64_MIN || ry > (double)INT64_MAX || rz < (double)INT64_MIN || rz > (double)INT64_MAX) {
        this->Error = out.Error = EV3_UB; if (Err) *Err = EV3_UB; return EV3_UB;
    }
    out.x = (int64_t)rx; out.y = (int64_t)ry; out.z = (int64_t)rz;
    this->Error = out.Error = EV3_SUCCESS; if (Err) *Err = EV3_SUCCESS;
    return EV3_SUCCESS;
}

err Vector3::Lerp(const Vector3& b, double t, Vector3& out, err* Err) const {
    double lx = (double)this->x + t * ((double)b.x - (double)this->x);
    double ly = (double)this->y + t * ((double)b.y - (double)this->y);
    double lz = (double)this->z + t * ((double)b.z - (double)this->z);
    if (lx < (double)INT64_MIN || lx > (double)INT64_MAX || ly < (double)INT64_MIN || ly > (double)INT64_MAX || lz < (double)INT64_MIN || lz > (double)INT64_MAX) {
        this->Error = out.Error = EV3_UB; if (Err) *Err = EV3_UB; return EV3_UB;
    }
    out.x = (int64_t)lx; out.y = (int64_t)ly; out.z = (int64_t)lz;
    this->Error = out.Error = EV3_SUCCESS; if (Err) *Err = EV3_SUCCESS;
    return EV3_SUCCESS;
}

Vector2* Vector3::Translate2D() {
    if (this->z == 0 || (this->x == INT64_MIN && this->z == -1) || (this->y == INT64_MIN && this->z == -1)) {
        this->Error = EV3_UB; return nullptr;
    }
    this->Error = EV3_SUCCESS;
    return new Vector2(this->x / this->z, this->y / this->z);
}

Vector3* Vector3::FromVector2(const Vector2& v2, int64_t z) {
    return new Vector3(v2.x * z, v2.y * z, z);
}


}}}
