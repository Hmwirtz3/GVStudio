#include "Viewports/Panels/ResourceInspectorPanel.h"

#include "Viewports/Panels/ResourceDatabaseTab.h"
#include "Viewports/Panels/LogicUnitRegistryTab.h"

#include "Database/LogicUnitRegistry.h"
#include "Database/ResourceDatabase.h"

#include "imgui/imgui.h"


void ResourceInspectorPanel::Draw(
    GV_State& state,
    LogicUnitRegistry& logicUnitRegistry,
    ResourceDatabase& resourceDatabase)
{
    if (!m_isVisible)
        return;

    if (!ImGui::Begin("Resource Explorer", &m_isVisible))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("ResourceTabs"))
    {
        if (ImGui::BeginTabItem("Logic Units"))
        {
            m_logicUnitTab.Draw(state, logicUnitRegistry);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Resources"))
        {
            m_resourceTab.DrawResourceFolderTree(resourceDatabase);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}


void ResourceInspectorPanel::SetVisible(bool visible)
{
    m_isVisible = visible;
}


bool ResourceInspectorPanel::IsVisible() const
{
    return m_isVisible;
}
