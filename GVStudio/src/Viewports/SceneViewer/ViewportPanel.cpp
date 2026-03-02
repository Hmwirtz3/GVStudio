#include "Viewports/SceneViewer/ViewportPanel.h"

#include "imgui/imgui.h"

void ViewportPanel::Draw(SceneFolder& scene)
{
    ImGui::Begin("SceneViewport");

    ImVec2 size = ImGui::GetContentRegionAvail();

    int w = (int)size.x;
    int h = (int)size.y;

    if (w > 0 && h > 0)
    {
        if (w != m_width || h != m_height)
        {
            m_width = w;
            m_height = h;
            m_viewer.Resize(w, h);
        }

        m_viewer.Update();
        m_viewer.Render(scene);

        ImGui::Image(
            (ImTextureID)(uintptr_t)m_viewer.GetColorTexture(),
            size,
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
    }

    ImGui::End();
}
