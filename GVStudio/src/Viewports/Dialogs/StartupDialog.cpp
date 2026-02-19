#include "Viewports//Dialogs/StartupDialog.h"
#include "Platform/WindowsFileDialog.h"
#include "MiniXml/ProjectXml.h"
#include "MiniXml/SceneXml.h"
#include "imgui/imgui.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace StartupDialog
{
    static char s_projectNameBuffer[128] = "NewProject";
    static char s_projectPathBuffer[512] = "";
    static bool s_showCreatePopup = false;

    void Draw(GV_State& state)
    {
        if (!state.showStartupDialog)
            return;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 center = viewport->GetCenter();

        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(640, 460), ImGuiCond_Always);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar;

        ImGui::Begin("Startup", nullptr, flags);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 24.0f);

        ImGui::Text("GRAVITAS");
        ImGui::Text("Version 0.1 Pre-Alpha");

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
        ImGui::Separator();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 24.0f);

        ImGui::SetCursorPosX(40.0f);

        if (ImGui::Button("Create New Project", ImVec2(560, 48)))
        {
            std::string folder = WindowsFileDialog::SelectFolder();

            if (!folder.empty())
            {
                std::cout << "\n[StartupDialog] Create Project Selected\n";
                std::cout << "  Base Folder: " << folder << "\n";

                strncpy_s(
                    s_projectPathBuffer,
                    folder.c_str(),
                    sizeof(s_projectPathBuffer) - 1);

                s_showCreatePopup = true;
            }
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16.0f);
        ImGui::SetCursorPosX(40.0f);

        if (ImGui::Button("Open Existing Project", ImVec2(560, 44)))
        {
            std::string path = WindowsFileDialog::OpenProjectFile();

            if (!path.empty())
            {
                std::cout << "\n[StartupDialog] Opening Project: "
                    << path << "\n";

                GV_Project_Info project;

                if (ProjectXml::LoadProjectFromXml(project, path))
                {
                    std::cout << "[StartupDialog] Project Loaded Successfully\n";
                    std::cout << "  Scenes Found: "
                        << project.scenes.size() << "\n";

                    state.project = project;

                    // Restore startup scene
                    if (!project.startupScene.empty())
                    {
                        std::cout << "  Restoring Startup Scene: "
                            << project.startupScene << "\n";

                        for (const auto& scene : project.scenes)
                        {
                            if (scene.scenePath == project.startupScene)
                            {
                                state.currentScene = scene;

                                std::cout << "  Active Scene Restored:\n";
                                std::cout << "    Name: "
                                    << scene.sceneName << "\n";
                                std::cout << "    Path: "
                                    << scene.scenePath << "\n";
                                break;
                            }
                        }
                    }
                    else
                    {
                        std::cout << "  No StartupScene defined.\n";
                    }

                    state.mode = EditorMode::ProjectOpen;
                    state.showStartupDialog = false;

                    std::cout << "[StartupDialog] Project Open Complete\n\n";
                }
                else
                {
                    std::cout << "[StartupDialog] Project Load Failed\n";
                }
            }
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16.0f);
        ImGui::SetCursorPosX(40.0f);

        if (ImGui::Button("Quit", ImVec2(560, 40)))
        {
            std::cout << "[StartupDialog] Quit Requested\n";
            state.quitRequested = true;
        }

        if (s_showCreatePopup)
        {
            ImGui::OpenPopup("CreateProjectPopup");
            s_showCreatePopup = false;
        }

        if (ImGui::BeginPopupModal("CreateProjectPopup", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Create New Project");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::InputText(
                "Project Name",
                s_projectNameBuffer,
                sizeof(s_projectNameBuffer));

            ImGui::Spacing();
            ImGui::Text("Location:");
            ImGui::Text("%s", s_projectPathBuffer);
            ImGui::Spacing();

            if (ImGui::Button("Create", ImVec2(120, 0)))
            {
                fs::path basePath = s_projectPathBuffer;
                fs::path projectDir = basePath / s_projectNameBuffer;

                std::cout << "\n[StartupDialog] Creating Project\n";
                std::cout << "  Project Directory: "
                    << projectDir << "\n";

                fs::create_directories(projectDir);
                fs::create_directories(projectDir / "Data");
                fs::create_directories(projectDir / "Resources");
                fs::create_directories(projectDir / "Source");
                fs::create_directories(projectDir / "Scenes");

                fs::path sceneFolder =
                    projectDir / "Scenes" / "Default";

                fs::create_directories(sceneFolder);
                fs::create_directories(sceneFolder / "Objects");

                fs::path defaultSceneFile =
                    sceneFolder / "Default.gScene";

                std::cout << "  Default Scene Folder: "
                    << sceneFolder << "\n";
                std::cout << "  Default Scene File: "
                    << defaultSceneFile << "\n";

                SceneFolder rootFolder;
                rootFolder.name = "Root";
                rootFolder.parent = nullptr;

                SceneXml::SaveGScene(
                    defaultSceneFile.string(),
                    rootFolder);

                GV_Project_Info project;
                project.projectName = s_projectNameBuffer;
                project.projectPath =
                    (projectDir /
                        (project.projectName + ".gProject")).string();

                project.dataFolder = "Data";
                project.resourceFolder = "Resources";
                project.sourceFolder = "Source";

                GV_Scene_Info sceneInfo;
                sceneInfo.scenePath = "Scenes/Default";
                sceneInfo.sceneName = "Default";

                project.scenes.push_back(sceneInfo);
                project.startupScene = sceneInfo.scenePath;

                ProjectXml::SaveProjectToXml(
                    project,
                    project.projectPath);

                state.project = project;
                state.currentScene = sceneInfo;
                state.mode = EditorMode::ProjectOpen;
                state.showStartupDialog = false;

                std::cout << "[StartupDialog] Project Creation Complete\n";
                std::cout << "  Active Scene: "
                    << state.currentScene.sceneName << "\n\n";

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                std::cout << "[StartupDialog] Create Project Cancelled\n";
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }
}
