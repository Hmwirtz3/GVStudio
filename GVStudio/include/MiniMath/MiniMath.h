#pragma once 

#include <cmath>

constexpr float PI = 3.14159;

struct Vec3
{
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    Vec3 operator+(const Vec3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vec3 operator-(const Vec3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
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

struct Mat4
{
    float m[16]; 

    static Mat4 Identity()
    {
        Mat4 r{};
        r.m[0] = 1; r.m[5] = 1; r.m[10] = 1; r.m[15] = 1;
        return r;
    }
};

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
