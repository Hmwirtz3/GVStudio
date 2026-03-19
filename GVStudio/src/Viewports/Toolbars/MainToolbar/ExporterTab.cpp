#include "Viewports/Toolbars/MainToolbar/ExporterTab.h"

#include "imgui/imgui.h"
#include <iostream>

void ExportTab::Draw(
    GV_State& state,
    SceneManager& sceneManager)
{
    if (!ImGui::BeginMenu("Export"))
        return;

    if (ImGui::MenuItem("Export Scene"))
    {
        std::cout << "[ExportTab] Export Scene Requested\n";
        m_exportRequested = true;
    }

    ImGui::EndMenu();
}



bool ExportTab::ConsumeExportRequest()
{
    if (m_exportRequested)
    {
        m_exportRequested = false;
        return true;
    }
    return false;
}