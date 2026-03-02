#include "Viewports/SceneViewer/SceneViewer.h"
#include "Renderer/GatherScene.h"
#include "imgui/imgui.h"
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
    const std::string& resourceRoot)
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

    m_renderer.Begin(m_camera.GetView(), m_camera.GetProjection());

    m_renderer.DrawGrid();

    for (const auto& item : items)
    {
        if (!item.modelPath.empty())
            m_renderer.DrawModel(item.modelPath, item.model);
    }

    m_renderer.End();
}

unsigned int SceneViewer::GetColorTexture() const
{
    return m_renderer.GetColorTexture();
}