#include "Viewports/Panels/SceneExplorer.h"
#include "GVStudio/GVStudio.h"

#include "MiniXml/ProjectXml.h"
#include "imgui/imgui.h"

#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

void SceneExplorer::SetVisible(bool visible)
{
    m_isVisible = visible;
}

bool SceneExplorer::IsVisible() const
{
    return m_isVisible;
}

void SceneExplorer::DrawSceneFoldersRecursive(
    SceneFolder& folder,
    SceneManager& sceneManager,
    SceneObject*& selectedObject,
    SceneFolder*& selectedFolder)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

    if (selectedFolder == &folder)
        flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushID(&folder);

    bool open = ImGui::TreeNodeEx(folder.name.c_str(), flags);

    if (ImGui::BeginDragDropSource())
    {
        SceneFolder* ptr = &folder;
        ImGui::SetDragDropPayload("SCENE_FOLDER", &ptr, sizeof(SceneFolder*));
        ImGui::Text("%s", folder.name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("SCENE_FOLDER"))
        {
            SceneFolder* dragged = *(SceneFolder**)payload->Data;

            if (dragged && dragged != &folder && dragged->parent)
            {
                SceneFolder* oldParent = dragged->parent;

                auto& oldList = oldParent->children;

                auto it = std::find_if(
                    oldList.begin(), oldList.end(),
                    [&](const std::unique_ptr<SceneFolder>& f)
                    {
                        return f.get() == dragged;
                    });

                if (it != oldList.end())
                {
                    auto moved = std::move(*it);
                    oldList.erase(it);

                    moved->parent = &folder;
                    folder.children.push_back(std::move(moved));
                }
            }
        }

        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("SCENE_OBJECT"))
        {
            SceneObject* dragged = *(SceneObject**)payload->Data;

            if (dragged)
            {
                SceneFolder* oldParent = nullptr;

                std::function<void(SceneFolder&)> findParent =
                    [&](SceneFolder& f)
                    {
                        for (auto& o : f.objects)
                        {
                            if (o.get() == dragged)
                            {
                                oldParent = &f;
                                return;
                            }
                        }

                        for (auto& c : f.children)
                            findParent(*c);
                    };

                findParent(sceneManager.GetRootFolder());

                if (oldParent && oldParent != &folder)
                {
                    auto& oldList = oldParent->objects;

                    auto it = std::find_if(
                        oldList.begin(), oldList.end(),
                        [&](const std::unique_ptr<SceneObject>& p)
                        {
                            return p.get() == dragged;
                        });

                    if (it != oldList.end())
                    {
                        auto moved = std::move(*it);
                        oldList.erase(it);

                        folder.objects.push_back(std::move(moved));
                    }
                }
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemClicked())
    {
        selectedFolder = &folder;
        selectedObject = nullptr;
    }

    if (open)
    {
        for (auto it = folder.objects.begin(); it != folder.objects.end(); )
        {
            SceneObject* obj = it->get();
            bool selected = (selectedObject == obj);

            ImGui::PushID(obj);

            if (ImGui::Selectable(obj->name.c_str(), selected))
            {
                selectedObject = obj;
                selectedFolder = &folder;
            }

            if (ImGui::BeginDragDropSource())
            {
                SceneObject* ptr = obj;
                ImGui::SetDragDropPayload("SCENE_OBJECT", &ptr, sizeof(SceneObject*));
                ImGui::Text("%s", obj->name.c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("SCENE_OBJECT"))
                {
                    SceneObject* dragged = *(SceneObject**)payload->Data;

                    if (dragged != obj)
                    {
                        auto& list = folder.objects;

                        auto draggedIt = std::find_if(
                            list.begin(), list.end(),
                            [&](const std::unique_ptr<SceneObject>& p)
                            {
                                return p.get() == dragged;
                            });

                        auto targetIt = std::find_if(
                            list.begin(), list.end(),
                            [&](const std::unique_ptr<SceneObject>& p)
                            {
                                return p.get() == obj;
                            });

                        if (draggedIt != list.end() && targetIt != list.end())
                        {
                            int draggedIndex = (int)std::distance(list.begin(), draggedIt);
                            int targetIndex = (int)std::distance(list.begin(), targetIt);

                            auto moved = std::move(*draggedIt);
                            list.erase(draggedIt);

                            targetIt = std::find_if(
                                list.begin(), list.end(),
                                [&](const std::unique_ptr<SceneObject>& p)
                                {
                                    return p.get() == obj;
                                });

                            if (draggedIndex < targetIndex)
                                list.insert(std::next(targetIt), std::move(moved));
                            else
                                list.insert(targetIt, std::move(moved));
                        }
                    }
                }

                ImGui::EndDragDropTarget();
            }

            ImGui::PopID();
            ++it;
        }

        for (auto& child : folder.children)
        {
            DrawSceneFoldersRecursive(
                *child,
                sceneManager,
                selectedObject,
                selectedFolder);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void SceneExplorer::Draw(
    GV_State& state,
    SceneManager& sceneManager,
    SceneObject*& selectedObject,
    SceneFolder*& selectedFolder)
{
    if (!m_isVisible)
        return;

    ImGui::Begin("Scene Explorer", &m_isVisible);

    ImGui::Text("Scenes");
    ImGui::Separator();

    if (ImGui::Button("Add Scene"))
    {
        GV_Scene_Info newScene{};
        newScene.sceneName = "New Scene";
        newScene.scenePath = "Scenes/Scene_" + std::to_string(state.project.scenes.size());

        fs::path fullPath =
            fs::path(state.project.projectRoot) / newScene.scenePath;

        fs::create_directories(fullPath);
        fs::create_directories(fullPath / "Objects");

        state.project.scenes.push_back(newScene);
        state.currentScene = newScene;

        sceneManager.GetRootFolder() = SceneFolder{};
        sceneManager.GetRootFolder().name = "Root";
        sceneManager.GetRootFolder().parent = nullptr;

        sceneManager.SaveScene(fullPath.string());

        ProjectXml::SaveProjectToXml(state.project, state.project.projectPath);
    }

    ImGui::SameLine();

    if (ImGui::Button("Delete Scene"))
    {
        auto& scenes = state.project.scenes;

        for (auto it = scenes.begin(); it != scenes.end(); ++it)
        {
            if (it->scenePath == state.currentScene.scenePath)
            {
                fs::path fullPath =
                    fs::path(state.project.projectRoot) / it->scenePath;

                if (fs::exists(fullPath))
                    fs::remove_all(fullPath);

                scenes.erase(it);

                if (!scenes.empty())
                    state.currentScene = scenes[0];
                else
                    state.currentScene = {};

                break;
            }
        }

        ProjectXml::SaveProjectToXml(state.project, state.project.projectPath);
        m_sceneRenaming = nullptr;
    }

    for (auto& scene : state.project.scenes)
    {
        bool selected = (scene.scenePath == state.currentScene.scenePath);

        ImGui::PushID(&scene);

        if (m_sceneRenaming == &scene)
        {
            if (ImGui::InputText(
                "##RenameScene",
                m_renameBuffer,
                sizeof(m_renameBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue))
            {
                std::string oldPath = scene.scenePath;

                std::string newName = m_renameBuffer;
                std::string safeName = newName;

                for (char& c : safeName)
                {
                    if (!isalnum(c) && c != '_')
                        c = '_';
                }

                std::string newPath = "Scenes/" + safeName;

                fs::path oldFull =
                    fs::path(state.project.projectRoot) / oldPath;

                fs::path newFull =
                    fs::path(state.project.projectRoot) / newPath;

                if (!fs::exists(newFull) && fs::exists(oldFull))
                {
                    fs::rename(oldFull, newFull);

                    scene.sceneName = newName;
                    scene.scenePath = newPath;

                    if (state.currentScene.scenePath == oldPath)
                        state.currentScene = scene;

                    ProjectXml::SaveProjectToXml(state.project, state.project.projectPath);
                }

                m_sceneRenaming = nullptr;
            }
        }
        else
        {
            if (ImGui::Selectable(scene.sceneName.c_str(), selected))
            {
                state.currentScene = scene;

                fs::path full =
                    fs::path(state.project.projectRoot) / scene.scenePath;

                sceneManager.LoadScene(full.string());
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Rename"))
                {
                    m_sceneRenaming = &scene;
                    strcpy_s(m_renameBuffer, scene.sceneName.c_str());
                }
                ImGui::EndPopup();
            }
        }

        ImGui::PopID();
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Objects");
    ImGui::Separator();

    DrawSceneFoldersRecursive(
        sceneManager.GetRootFolder(),
        sceneManager,
        selectedObject,
        selectedFolder);

    ImGui::End();
}