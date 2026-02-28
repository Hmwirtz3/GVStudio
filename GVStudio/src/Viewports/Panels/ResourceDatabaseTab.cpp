#include "Viewports/Panels/ResourceDatabaseTab.h"

#include "imgui/imgui.h"

void ResourceDatabaseTab::DrawResourceFolderTree(const ResourceDatabase& resourceDatabase)
{
    const ResourceNode* root = resourceDatabase.GetRoot();
    if (!root)
        return;

    ImGui::Text("Resource Folder");
    ImGui::Separator();

    for (const auto& child : root->children)
        DrawResourceNode(child.get());
}

void ResourceDatabaseTab::DrawResourceNode(const ResourceNode* node)
{
    if (!node)
        return;

    ImGui::PushID(node->fullPath.c_str());

    if (node->isFolder)
    {
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanFullWidth;

        bool open = ImGui::TreeNodeEx(node->name.c_str(), flags);

        if (open)
        {
            for (const auto& child : node->children)
                DrawResourceNode(child.get());

            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::Selectable(node->name.c_str());

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(
                "RESOURCE_FILE",
                node->fullPath.c_str(),
                node->fullPath.size() + 1);

            ImGui::Text("%s", node->name.c_str());
            ImGui::EndDragDropSource();
        }
    }

    ImGui::PopID();
}




void ResourceDatabaseTab::Setvisible(bool visible)
{
	m_isVisible = visible;
}

bool ResourceDatabaseTab::IsVisible()
{
	return m_isVisible;
}
