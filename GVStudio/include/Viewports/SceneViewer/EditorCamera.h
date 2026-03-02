#pragma once

#include "MiniMath/MiniMath.h"

class EditorCamera
{
public:
    void SetAspect(float aspect);

    void Orbit(float yawDelta, float pitchDelta);
    void Pan(float xDelta, float yDelta);
    void Zoom(float amount);

    void Update();

    Mat4 GetView() const;
    Mat4 GetProjection() const;

private:
    float m_distance = 5.0f;
    float m_yaw = 0.0f;
    float m_pitch = 0.3f;
    float m_aspect = 1.0f;

    Vec3 m_target = { 0, 0, 0 };
};