#include "Viewports/SceneViewer/SceneViewer.h"
#include "Renderer/GatherScene.h"
#include "imgui/imgui.h"
#include "MiniMath/MiniMath.h"

#include <iostream>
#include <cmath>

void SceneViewer::Resize(int width, int height)
{
    m_width = width;
    m_height = height;

    m_renderer.Resize(width, height);

    if (height > 0)
        m_camera.SetAspect((float)width / (float)height);
}

void SceneViewer::Update()
{
    ImGuiIO& io = ImGui::GetIO();

    if (!ImGui::IsWindowHovered())
        return;

    const float orbitSpeed = 0.005f;
    const float panSpeed = 0.01f;
    const float zoomSpeed = 0.5f;

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        m_camera.Orbit(
            -io.MouseDelta.x * orbitSpeed,
            -io.MouseDelta.y * orbitSpeed
        );
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
    {
        m_camera.Pan(
            -io.MouseDelta.x * panSpeed,
            io.MouseDelta.y * panSpeed
        );
    }

    if (io.MouseWheel != 0.0f)
    {
        m_camera.Zoom(-io.MouseWheel * zoomSpeed);
    }
}

void SceneViewer::Render(SceneFolder& scene,
    const std::string& resourceRoot,
    SceneObject* selectedObject)
{
    static bool initialized = false;

    if (!initialized)
    {
        m_renderer.Init();
        m_renderer.Resize(m_width, m_height);

        if (m_height > 0)
            m_camera.SetAspect((float)m_width / (float)m_height);

        initialized = true;
    }

    std::vector<RenderItem> items;
    GatherScene::Collect(scene, resourceRoot, items);

    m_renderer.Begin(
        m_camera.GetView(),
        m_camera.GetProjection());

    m_renderer.DrawGrid();

    for (const auto& item : items)
    {
        if (item.type == RenderItemType::Mesh)
        {
            if (!item.modelPath.empty())
            {
                m_renderer.DrawModel(
                    item.modelPath,
                    item.model);
            }
        }
        else if (item.type == RenderItemType::CameraGizmo)
        {
            m_renderer.DrawCameraGizmo(
                item.camPos,
                item.camRot);
        }
    }

    m_renderer.End();
}

SceneObject* SceneViewer::PickObject(
    SceneFolder& scene,
    const std::string& resourceRoot,
    float mouseX,
    float mouseY)
{
    ImVec2 windowPos = ImGui::GetWindowPos();

    float localX = mouseX - windowPos.x;
    float localY = mouseY - windowPos.y;

    localY = (float)m_height - localY;

    float ndcX = (2.0f * localX) / (float)m_width - 1.0f;
    float ndcY = (2.0f * localY) / (float)m_height - 1.0f;

    Mat4 view = m_camera.GetView();
    Mat4 proj = m_camera.GetProjection();
    Mat4 invViewProj = Inverse(proj * view);

    Vec4 nearPoint = { ndcX, ndcY, -1.0f, 1.0f };
    Vec4 farPoint = { ndcX, ndcY,  1.0f, 1.0f };

    Vec4 nearWorld = invViewProj * nearPoint;
    Vec4 farWorld = invViewProj * farPoint;

    nearWorld = nearWorld / nearWorld.w;
    farWorld = farWorld / farWorld.w;

    Vec3 rayOrigin = { nearWorld.x, nearWorld.y, nearWorld.z };
    Vec3 rayDir = Normalize(
        Vec3{
            farWorld.x - nearWorld.x,
            farWorld.y - nearWorld.y,
            farWorld.z - nearWorld.z
        });

    std::vector<RenderItem> items;
    GatherScene::Collect(scene, resourceRoot, items);

    float closestT = 1e30f;
    SceneObject* hit = nullptr;

    for (const auto& item : items)
    {
        if (item.modelPath.empty())
            continue;

        Vec3 center = {
            item.model.m[12],
            item.model.m[13],
            item.model.m[14]
        };

        Vec3 halfExtents = { 2.0f, 2.0f, 2.0f };

        Vec3 min = center - halfExtents;
        Vec3 max = center + halfExtents;

        float tMin = 0.0f;
        float tMax = 1e30f;

        for (int axis = 0; axis < 3; ++axis)
        {
            float origin =
                axis == 0 ? rayOrigin.x :
                axis == 1 ? rayOrigin.y :
                rayOrigin.z;

            float direction =
                axis == 0 ? rayDir.x :
                axis == 1 ? rayDir.y :
                rayDir.z;

            float minB =
                axis == 0 ? min.x :
                axis == 1 ? min.y :
                min.z;

            float maxB =
                axis == 0 ? max.x :
                axis == 1 ? max.y :
                max.z;

            if (fabs(direction) < 0.00001f)
            {
                if (origin < minB || origin > maxB)
                    goto nextObject;
            }
            else
            {
                float invD = 1.0f / direction;
                float t1 = (minB - origin) * invD;
                float t2 = (maxB - origin) * invD;

                if (t1 > t2)
                    std::swap(t1, t2);

                if (t1 > tMin)
                    tMin = t1;

                if (t2 < tMax)
                    tMax = t2;

                if (tMax < tMin)
                    goto nextObject;
            }
        }

        if (tMin > 0.0f && tMin < closestT)
        {
            closestT = tMin;
            hit = item.object;
        }

    nextObject:
        continue;
    }

    return hit;
}

unsigned int SceneViewer::GetColorTexture() const
{
    return m_renderer.GetColorTexture();
}