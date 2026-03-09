#include "Viewports/Toolbars/MainToolbar/FileTab.h"

#include "MiniXml/ProjectXml.h"
#include "Platform/WindowsFileDialog.h"
#include "GVFramework/Scene/SceneManager.h"

#include "imgui/imgui.h"

#include <iostream>

void FileTab::Draw(
    GV_State& state,
    SceneManager& sceneManager)
{
    if (!ImGui::BeginMenu("File"))
        return;

    // -----------------------------
    // Open Project
    // -----------------------------
    if (ImGui::MenuItem("Open Project"))
    {
        std::string path = WindowsFileDialog::OpenProjectFile();

        if (!path.empty())
        {
            std::cout << "[FileTab] Opening Project: "
                << path << "\n";

            GV_Project_Info project;

            if (ProjectXml::LoadProjectFromXml(project, path))
            {
                state.project = project;

                if (!project.startupScene.empty())
                {
                    for (const auto& scene : project.scenes)
                    {
                        if (scene.scenePath == project.startupScene)
                        {
                            state.currentScene = scene;

                            std::string sceneDir =
                                state.project.projectRoot + "/" +
                                scene.scenePath;

                            std::cout << "[FileTab] Loading Scene: "
                                << sceneDir << "\n";

                            sceneManager.LoadScene(sceneDir);

                            break;
                        }
                    }
                }

                state.mode = EditorMode::ProjectOpen;
            }
        }
    }

    ImGui::Separator();

    // -----------------------------
    // Save Scene
    // -----------------------------
    if (ImGui::MenuItem("Save Scene"))
    {
        if (!state.currentScene.scenePath.empty())
        {
            std::string sceneDir =
                state.project.projectRoot + "/" +
                state.currentScene.scenePath;

            std::cout << "[FileTab] Saving Scene: "
                << sceneDir << "\n";

            sceneManager.SaveScene(sceneDir);
        }
    }

    // -----------------------------
    // Save All Scenes
    // -----------------------------
    if (ImGui::MenuItem("Save All Scenes"))
    {
        std::cout << "[FileTab] Save All Scenes\n";

        if (!state.currentScene.scenePath.empty())
        {
            std::string sceneDir =
                state.project.projectRoot + "/" +
                state.currentScene.scenePath;

            sceneManager.SaveScene(sceneDir);
        }
    }

    // -----------------------------
    // Save Project
    // -----------------------------
    if (ImGui::MenuItem("Save Project"))
    {
        std::cout << "[FileTab] Saving Project\n";

        if (!state.currentScene.scenePath.empty())
        {
            std::string sceneDir =
                state.project.projectRoot + "/" +
                state.currentScene.scenePath;

            sceneManager.SaveScene(sceneDir);
        }

        ProjectXml::SaveProjectToXml(
            state.project,
            state.project.projectPath);
    }

    ImGui::EndMenu();
}