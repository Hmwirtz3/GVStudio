#include "Viewports/Panels/LogicUnitRegistryTab.h"
#include "Database/LogicUnitRegistry.h"
#include "imgui/imgui.h"

#include "GVStudio/GVStudio.h"

#include <iostream>
#include <filesystem>
#include "Viewports/Panels/ResourceDatabaseTab.h"

namespace fs = std::filesystem;

void LogicUnitRegistryTab::Draw(GV_State& state, LogicUnitRegistry& registry)
{
    ImGui::Text("Logic Units");
    ImGui::Separator();

    if (ImGui::Button("Refresh"))
    {
        std::cout << "\n[UI] Logic Unit Refresh Requested\n";

        fs::path root = fs::path(state.project.projectPath).parent_path();
        fs::path fullSource = root / state.project.sourceFolder;

        std::cout << "[UI] Source Folder Resolved: " << fullSource.string() << "\n";

        registry.Clear();
        registry.ParseFolder(fullSource.string());

        std::cout << "[UI] Refresh Complete\n";
    }

    ImGui::Spacing();

    const auto& units = registry.GetAll();

    for (const auto& unit : units)
    {
        ImGui::PushID(unit.typeName.c_str());

        if (ImGui::Selectable(unit.typeName.c_str()))
        {
            std::cout << "[UI] Selected Logic Unit: " << unit.typeName << "\n";
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(
                "LOGIC_UNIT",
                unit.typeName.c_str(),
                unit.typeName.size() + 1);

            ImGui::Text("%s", unit.typeName.c_str());
            ImGui::EndDragDropSource();
        }

        ImGui::SameLine();
        ImGui::TextDisabled("(%d params)", (int)unit.params.size());

        ImGui::PopID();
    }
}


void LogicUnitRegistryTab::SetVisible (bool visible) 
{
    m_isVisible = visible;
}

bool LogicUnitRegistryTab::IsVisible() const
{
    return m_isVisible;
}

