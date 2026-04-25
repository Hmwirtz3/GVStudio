#pragma once

#include <cmath>

constexpr float PI = 3.14159265359f;

struct Vec3
{
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    Vec3 operator+(const Vec3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vec3 operator-(const Vec3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
};

struct Vec2
{
    float x;
    float y;

    Vec2() : x(0), y(0) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}
};

inline float Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 Cross(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline float Length(const Vec3& v)
{
    return std::sqrt(Dot(v, v));
}

inline Vec3 Normalize(const Vec3& v)
{
    float len = Length(v);
    if (len <= 0.00001f) return v;
    return v * (1.0f / len);
}

struct Vec4
{
    float x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float X, float Y, float Z, float W)
        : x(X), y(Y), z(Z), w(W) {
    }

    Vec4 operator/(float s) const
    {
        return { x / s, y / s, z / s, w / s };
    }
};

struct Mat4
{
    float m[16];

    static Mat4 Identity()
    {
        Mat4 r{};
        r.m[0] = 1.0f;
        r.m[5] = 1.0f;
        r.m[10] = 1.0f;
        r.m[15] = 1.0f;
        return r;
    }
};

inline Mat4 operator*(const Mat4& a, const Mat4& b)
{
    Mat4 r{};

    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            r.m[col * 4 + row] =
                a.m[0 * 4 + row] * b.m[col * 4 + 0] +
                a.m[1 * 4 + row] * b.m[col * 4 + 1] +
                a.m[2 * 4 + row] * b.m[col * 4 + 2] +
                a.m[3 * 4 + row] * b.m[col * 4 + 3];
        }
    }

    return r;
}

inline Vec4 operator*(const Mat4& m, const Vec4& v)
{
    Vec4 r;

    r.x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w;
    r.y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w;
    r.z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w;
    r.w = m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w;

    return r;
}

inline Mat4 Translate(const Vec3& t)
{
    Mat4 r = Mat4::Identity();
    r.m[12] = t.x;
    r.m[13] = t.y;
    r.m[14] = t.z;
    return r;
}

inline Mat4 Scale(const Vec3& s)
{
    Mat4 r = Mat4::Identity();
    r.m[0] = s.x;
    r.m[5] = s.y;
    r.m[10] = s.z;
    return r;
}

inline Mat4 RotateX(float radians)
{
    Mat4 r = Mat4::Identity();
    float c = std::cos(radians);
    float s = std::sin(radians);

    r.m[5] = c;
    r.m[6] = s;
    r.m[9] = -s;
    r.m[10] = c;

    return r;
}

inline Mat4 RotateY(float radians)
{
    Mat4 r = Mat4::Identity();
    float c = std::cos(radians);
    float s = std::sin(radians);

    r.m[0] = c;
    r.m[2] = -s;
    r.m[8] = s;
    r.m[10] = c;

    return r;
}

inline Mat4 RotateZ(float radians)
{
    Mat4 r = Mat4::Identity();
    float c = std::cos(radians);
    float s = std::sin(radians);

    r.m[0] = c;
    r.m[1] = s;
    r.m[4] = -s;
    r.m[5] = c;

    return r;
}

inline Mat4 Perspective(float fovRadians, float aspect, float nearZ, float farZ)
{
    float tanHalfFov = std::tan(fovRadians * 0.5f);

    Mat4 r{};
    r.m[0] = 1.0f / (aspect * tanHalfFov);
    r.m[5] = 1.0f / tanHalfFov;
    r.m[10] = -(farZ + nearZ) / (farZ - nearZ);
    r.m[11] = -1.0f;
    r.m[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);

    return r;
}

inline Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
{
    Vec3 forward = Normalize(target - eye);
    Vec3 right = Normalize(Cross(forward, up));
    Vec3 newUp = Cross(right, forward);

    Mat4 r = Mat4::Identity();

    r.m[0] = right.x;
    r.m[4] = right.y;
    r.m[8] = right.z;

    r.m[1] = newUp.x;
    r.m[5] = newUp.y;
    r.m[9] = newUp.z;

    r.m[2] = -forward.x;
    r.m[6] = -forward.y;
    r.m[10] = -forward.z;

    r.m[12] = -Dot(right, eye);
    r.m[13] = -Dot(newUp, eye);
    r.m[14] = Dot(forward, eye);

    return r;
}

inline Mat4 Inverse(const Mat4& m)
{
    Mat4 inv;
    float* invOut = inv.m;
    const float* a = m.m;

    invOut[0] = a[5] * a[10] * a[15] -
        a[5] * a[11] * a[14] -
        a[9] * a[6] * a[15] +
        a[9] * a[7] * a[14] +
        a[13] * a[6] * a[11] -
        a[13] * a[7] * a[10];

    invOut[1] = -a[1] * a[10] * a[15] +
        a[1] * a[11] * a[14] +
        a[9] * a[2] * a[15] -
        a[9] * a[3] * a[14] -
        a[13] * a[2] * a[11] +
        a[13] * a[3] * a[10];

    invOut[2] = a[1] * a[6] * a[15] -
        a[1] * a[7] * a[14] -
        a[5] * a[2] * a[15] +
        a[5] * a[3] * a[14] +
        a[13] * a[2] * a[7] -
        a[13] * a[3] * a[6];

    invOut[3] = -a[1] * a[6] * a[11] +
        a[1] * a[7] * a[10] +
        a[5] * a[2] * a[11] -
        a[5] * a[3] * a[10] -
        a[9] * a[2] * a[7] +
        a[9] * a[3] * a[6];

    invOut[4] = -a[4] * a[10] * a[15] +
        a[4] * a[11] * a[14] +
        a[8] * a[6] * a[15] -
        a[8] * a[7] * a[14] -
        a[12] * a[6] * a[11] +
        a[12] * a[7] * a[10];

    invOut[5] = a[0] * a[10] * a[15] -
        a[0] * a[11] * a[14] -
        a[8] * a[2] * a[15] +
        a[8] * a[3] * a[14] +
        a[12] * a[2] * a[11] -
        a[12] * a[3] * a[10];

    invOut[6] = -a[0] * a[6] * a[15] +
        a[0] * a[7] * a[14] +
        a[4] * a[2] * a[15] -
        a[4] * a[3] * a[14] -
        a[12] * a[2] * a[7] +
        a[12] * a[3] * a[6];

    invOut[7] = a[0] * a[6] * a[11] -
        a[0] * a[7] * a[10] -
        a[4] * a[2] * a[11] +
        a[4] * a[3] * a[10] +
        a[8] * a[2] * a[7] -
        a[8] * a[3] * a[6];

    invOut[8] = a[4] * a[9] * a[15] -
        a[4] * a[11] * a[13] -
        a[8] * a[5] * a[15] +
        a[8] * a[7] * a[13] +
        a[12] * a[5] * a[11] -
        a[12] * a[7] * a[9];

    invOut[9] = -a[0] * a[9] * a[15] +
        a[0] * a[11] * a[13] +
        a[8] * a[1] * a[15] -
        a[8] * a[3] * a[13] -
        a[12] * a[1] * a[11] +
        a[12] * a[3] * a[9];

    invOut[10] = a[0] * a[5] * a[15] -
        a[0] * a[7] * a[13] -
        a[4] * a[1] * a[15] +
        a[4] * a[3] * a[13] +
        a[12] * a[1] * a[7] -
        a[12] * a[3] * a[5];

    invOut[11] = -a[0] * a[5] * a[11] +
        a[0] * a[7] * a[9] +
        a[4] * a[1] * a[11] -
        a[4] * a[3] * a[9] -
        a[8] * a[1] * a[7] +
        a[8] * a[3] * a[5];

    invOut[12] = -a[4] * a[9] * a[14] +
        a[4] * a[10] * a[13] +
        a[8] * a[5] * a[14] -
        a[8] * a[6] * a[13] -
        a[12] * a[5] * a[10] +
        a[12] * a[6] * a[9];

    invOut[13] = a[0] * a[9] * a[14] -
        a[0] * a[10] * a[13] -
        a[8] * a[1] * a[14] +
        a[8] * a[2] * a[13] +
        a[12] * a[1] * a[10] -
        a[12] * a[2] * a[9];

    invOut[14] = -a[0] * a[5] * a[14] +
        a[0] * a[6] * a[13] +
        a[4] * a[1] * a[14] -
        a[4] * a[2] * a[13] -
        a[12] * a[1] * a[6] +
        a[12] * a[2] * a[5];

    invOut[15] = a[0] * a[5] * a[10] -
        a[0] * a[6] * a[9] -
        a[4] * a[1] * a[10] +
        a[4] * a[2] * a[9] +
        a[8] * a[1] * a[6] -
        a[8] * a[2] * a[5];

    float det = a[0] * invOut[0] + a[1] * invOut[4] + a[2] * invOut[8] + a[3] * invOut[12];

    if (fabs(det) < 0.000001f)
        return Mat4::Identity();

    float invDet = 1.0f / det;

    for (int i = 0; i < 16; i++)
        invOut[i] *= invDet;

    return inv;
}