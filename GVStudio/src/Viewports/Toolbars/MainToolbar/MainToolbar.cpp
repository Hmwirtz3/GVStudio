#include "Viewports/Toolbars/MainToolbar/MainToolbar.h"

#include "GVStudio/GVStudio.h"
#include "GVFramework/Scene/SceneManager.h"

#include "imgui/imgui.h"

void Toolbar::Draw(GV_State& state, SceneManager& sceneManager)
{
    if (!ImGui::BeginMainMenuBar())
        return;

    m_fileTab.Draw(state, sceneManager);
    m_exportTab.Draw(state, sceneManager);

    ImGui::EndMainMenuBar();
}

bool Toolbar::ConsumeExportRequest()
{

    return m_exportTab.ConsumeExportRequest();

}