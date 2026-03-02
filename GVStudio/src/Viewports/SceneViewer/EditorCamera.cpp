#include "Viewports/SceneViewer/EditorCamera.h"
#include <cmath>

void EditorCamera::SetAspect(float aspect)
{
    m_aspect = aspect;
}

void EditorCamera::Orbit(float yawDelta, float pitchDelta)
{
    m_yaw += yawDelta;
    m_pitch += pitchDelta;

    const float limit = 1.55f;

    if (m_pitch > limit) m_pitch = limit;
    if (m_pitch < -limit) m_pitch = -limit;
}

void EditorCamera::Pan(float xDelta, float yDelta)
{
    Vec3 forward;
    forward.x = std::cos(m_pitch) * std::sin(m_yaw);
    forward.y = std::sin(m_pitch);
    forward.z = std::cos(m_pitch) * std::cos(m_yaw);

    forward = Normalize(forward);

    Vec3 right = Normalize(Cross(forward, { 0,1,0 }));
    Vec3 up = Normalize(Cross(right, forward));

    m_target = m_target + right * xDelta + up * yDelta;
}

void EditorCamera::Zoom(float amount)
{
    m_distance += amount;

    if (m_distance < 0.1f)
        m_distance = 0.1f;
}

void EditorCamera::Update()
{
}

Mat4 EditorCamera::GetProjection() const
{
    return Perspective(45.0f * PI / 180.0f, m_aspect, 0.1f, 1000.0f);
}

Mat4 EditorCamera::GetView() const
{
    Vec3 dir;
    dir.x = std::cos(m_pitch) * std::sin(m_yaw);
    dir.y = std::sin(m_pitch);
    dir.z = std::cos(m_pitch) * std::cos(m_yaw);

    Vec3 position = m_target - dir * m_distance;

    return LookAt(position, m_target, { 0,1,0 });
}