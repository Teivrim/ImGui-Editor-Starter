#pragma once
#include "Types.h"
#include <cmath>
#include <algorithm>

struct Vec2 {
    f32 x = 0, y = 0;

    Vec2() = default;
    Vec2(f32 x, f32 y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(f32 s) const { return {x * s, y * s}; }
    Vec2 operator/(f32 s) const { return {x / s, y / s}; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return !(*this == o); }

    f32 length() const { return std::sqrt(x * x + y * y); }
    f32 lengthSq() const { return x * x + y * y; }
    Vec2 normalized() const {
        f32 l = length();
        return l > 0 ? *this / l : Vec2{};
    }
    f32 dot(const Vec2& o) const { return x * o.x + y * o.y; }
    f32 cross(const Vec2& o) const { return x * o.y - y * o.x; }
};

struct Vec3 {
    f32 x = 0, y = 0, z = 0;

    Vec3() = default;
    Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(f32 s) const { return {x * s, y * s, z * s}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }

    f32 length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalized() const {
        f32 l = length();
        return l > 0 ? *this / l : Vec3{};
    }
    Vec3 operator/(f32 s) const { return {x / s, y / s, z / s}; }
    f32 dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
};

struct Vec4 {
    f32 x = 0, y = 0, z = 0, w = 1;

    Vec4() = default;
    Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
};

struct Mat4 {
    f32 m[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    Mat4() = default;

    static Mat4 identity() { return Mat4{}; }

    static Mat4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
        Mat4 r;
        r.m[0]  = 2 / (right - left);
        r.m[5]  = 2 / (top - bottom);
        r.m[10] = -2 / (far - near);
        r.m[12] = -(right + left) / (right - left);
        r.m[13] = -(top + bottom) / (top - bottom);
        r.m[14] = -(far + near) / (far - near);
        return r;
    }

    static Mat4 scale(f32 sx, f32 sy, f32 sz) {
        Mat4 r;
        r.m[0] = sx; r.m[5] = sy; r.m[10] = sz;
        return r;
    }

    static Mat4 translate(f32 tx, f32 ty, f32 tz) {
        Mat4 r;
        r.m[12] = tx; r.m[13] = ty; r.m[14] = tz;
        return r;
    }

    static Mat4 rotateZ(f32 angle) {
        f32 c = std::cos(angle), s = std::sin(angle);
        Mat4 r;
        r.m[0] = c; r.m[1] = s;
        r.m[4] = -s; r.m[5] = c;
        return r;
    }

    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (i32 i = 0; i < 4; i++)
            for (i32 j = 0; j < 4; j++) {
                r.m[i * 4 + j] = 0;
                for (i32 k = 0; k < 4; k++)
                    r.m[i * 4 + j] += m[i * 4 + k] * o.m[k * 4 + j];
            }
        return r;
    }

    f32* data() { return m; }
    const f32* data() const { return m; }
};

struct Transform {
    Vec2 position;
    Vec2 scale{1, 1};
    f32 rotation = 0;
    Vec2 anchor{0.5f, 0.5f};

    Mat4 matrix() const {
        return Mat4::translate(position.x, position.y, 0) *
               Mat4::rotateZ(rotation) *
               Mat4::scale(scale.x, scale.y, 1);
    }
};

struct Rect {
    f32 x = 0, y = 0, w = 0, h = 0;

    bool contains(f32 px, f32 py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
    Vec2 center() const { return {x + w / 2, y + h / 2}; }
    Rect inset(f32 amount) const {
        return {x + amount, y + amount, w - amount * 2, h - amount * 2};
    }
};
