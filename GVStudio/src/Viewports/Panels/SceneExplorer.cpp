#include "Viewports/Panels/SceneExplorer.h"

#include "imgui/imgui.h"

#include <iostream>
#include <filesystem>

/* namespace fs = std::filesystem;

void SceneExplorer::SetVisible(bool visible) 
{
	m_isVisible = visible;

}

bool SceneExplorer::IsVisible() const
{
	return m_isVisible;
}

void SceneExplorer::DrawSceneFoldersRecursive(SceneFolder& folder, SceneManager sceneManager, LogicUnitRegistry& logicUnitRegistry)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

    if (m_selectedFolder == &folder)
        flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushID(&folder);

    bool open = ImGui::TreeNodeEx(folder.name.c_str(), flags);

    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("New Folder"))
        {
            m_newFolderParent = &folder;
        }

        if (ImGui::MenuItem("Rename"))
        {
            m_folderRenaming = &folder;
            m_objectRenaming = nullptr;
            strcpy_s(m_renameBuffer, folder.name.c_str());
        }

        if (&folder != &m_rootFolder)
        {
            if (ImGui::MenuItem("Delete"))
            {
                m_folderPendingDelete = &folder;
            }
        }

        ImGui::EndPopup();
    }

    if (ImGui::IsItemClicked())
    {
        m_selectedFolder = &folder;
        m_selectedObject = nullptr;
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("LOGIC_UNIT"))
        {
            const char* typeName = (const char*)payload->Data;

            SceneObject* obj =
                sceneManager.CreateObjectFromLogicUnit(typeName, &folder, logicUnitRegistry);

            if (obj)
                m_selectedObject = obj;
        }

        ImGui::EndDragDropTarget();
    }

    if (m_folderRenaming == &folder)
    {
        ImGui::SameLine();
        if (ImGui::InputText("##RenameFolder",
            m_renameBuffer,
            sizeof(m_renameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue))
        {
            folder.name = m_renameBuffer;
            m_folderRenaming = nullptr;
        }
    }

    if (open)
    {
        for (auto it = folder.objects.begin(); it != folder.objects.end(); )
        {
            SceneObject* obj = it->get();
            bool selected = (m_selectedObject == obj);

            ImGui::PushID(obj);

            if (m_objectRenaming == obj)
            {
                if (ImGui::InputText("##RenameObject",
                    m_renameBuffer,
                    sizeof(m_renameBuffer),
                    ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    obj->name = m_renameBuffer;
                    m_objectRenaming = nullptr;
                }
            }
            else
            {
                if (ImGui::Selectable(obj->name.c_str(), selected))
                {
                    m_selectedObject = obj;
                    m_selectedFolder = &folder;
                }

                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Rename"))
                    {
                        m_objectRenaming = obj;
                        m_folderRenaming = nullptr;
                        strcpy_s(m_renameBuffer, obj->name.c_str());
                    }

                    if (ImGui::MenuItem("Delete"))
                    {
                        m_objectPendingDelete = obj;
                    }

                    ImGui::EndPopup();
                }
            }

            ImGui::PopID();

            if (m_objectPendingDelete == obj)
            {
                if (m_selectedObject == obj)
                    m_selectedObject = nullptr;

                it = folder.objects.erase(it);
                m_objectPendingDelete = nullptr;
                continue;
            }

            ++it;
        }

        for (auto it = folder.children.begin(); it != folder.children.end(); )
        {
            SceneFolder* child = it->get();

            if (m_folderPendingDelete == child)
            {
                if (m_selectedFolder == child)
                {
                    m_selectedFolder = &m_rootFolder;
                    m_selectedObject = nullptr;
                }

                it = folder.children.erase(it);
                m_folderPendingDelete = nullptr;
                continue;
            }

            DrawSceneFoldersRecursive(*child, sceneManager, logicUnitRegistry);
            ++it;
        }

        ImGui::TreePop();
    }

    ImGui::PopID();

    // ======= HANDLE NEW FOLDER CREATION =======
    if (m_newFolderParent == &folder)
    {
        auto newFolder = std::make_unique<SceneFolder>();
        newFolder->name = "New Folder";
        folder.children.push_back(std::move(newFolder));

        m_newFolderParent = nullptr;
    }
}

void SceneExplorer::Draw(GV_State state, SceneManager sceneManager, LogicUnitRegistry& logicUnitRegistry)
{
    if (!m_isVisible)
        return;

    ImGui::Begin("Scene Explorer", &m_isVisible);

    ImGui::Text("Scenes");
    ImGui::Separator();

    for (const auto& scene : state.project.scenes)
    {
        bool isSelected =
            (scene.scenePath == state.currentScene.scenePath);

        if (ImGui::Selectable(scene.sceneName.c_str(), isSelected))
        {
            if (!isSelected)
            {
                state.currentScene = scene;

                std::cout << "[SceneExplorer] Switching Scene:\n";
                std::cout << "  Name: " << scene.sceneName << "\n";
                std::cout << "  Path: " << scene.scenePath << "\n";

                fs::path projectRoot =
                    fs::path(state.project.projectPath).parent_path();

                fs::path fullScenePath =
                    projectRoot / scene.scenePath;

                sceneManager.LoadScene(fullScenePath.string());

                m_selectedObject = nullptr;
                m_selectedFolder = &m_rootFolder;

                m_folderPendingDelete = nullptr;
                m_objectPendingDelete = nullptr;
                m_folderRenaming = nullptr;
                m_objectRenaming = nullptr;
                m_renameBuffer[0] = 0;
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Objects");
    ImGui::Separator();

    DrawSceneFoldersRecursive(m_rootFolder, sceneManager, logicUnitRegistry);

    ImGui::End();
}

*/