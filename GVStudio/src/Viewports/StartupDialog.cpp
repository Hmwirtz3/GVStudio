#include "Viewports/StartupDialog.h"
#include "Platform/WindowsFileDialog.h"
#include "MiniXml/ProjectXml.h"
#include "MiniXml/SceneXml.h"
#include "imgui/imgui.h"

#include <filesystem>

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
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove;

        ImGui::Begin("Gravitas Studio", nullptr, flags);

        ImGui::Spacing();
        ImGui::Text("GRAVITAS STUDIO");
        ImGui::Text("Version 0.1 Pre Pre Alpha");
        ImGui::Separator();
        ImGui::Spacing();

      
        if (ImGui::Button("Create New Project", ImVec2(-1, 40)))
        {
            std::string folder = WindowsFileDialog::SelectFolder();

            if (!folder.empty())
            {
                strncpy_s(s_projectPathBuffer,
                    folder.c_str(),
                    sizeof(s_projectPathBuffer) - 1);

                s_showCreatePopup = true;
            }
        }

        ImGui::Spacing();

        
        if (ImGui::Button("Open Existing Project", ImVec2(-1, 40)))
        {
            std::string path = WindowsFileDialog::OpenProjectFile();

            if (!path.empty())
            {
                GV_Project_Info project;

                if (ProjectXml::LoadProjectFromXml(project, path))
                {
                    state.project = project;
                    state.mode = EditorMode::ProjectOpen;
                    state.showStartupDialog = false;
                }
            }
        }

        
        if (s_showCreatePopup)
        {
            ImGui::OpenPopup("CreateProjectPopup");
            s_showCreatePopup = false;
        }

        if (ImGui::BeginPopupModal("CreateProjectPopup", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText("Project Name",
                s_projectNameBuffer,
                sizeof(s_projectNameBuffer));

            ImGui::TextWrapped("Location:");
            ImGui::Text("%s", s_projectPathBuffer);

            if (ImGui::Button("Create"))
            {
                fs::path basePath = s_projectPathBuffer;
                fs::path projectDir = basePath / s_projectNameBuffer;

                fs::create_directories(projectDir);
                fs::create_directories(projectDir / "Data");
                fs::create_directories(projectDir / "Resources");
                fs::create_directories(projectDir / "Source");
                fs::create_directories(projectDir / "Scenes");
                fs::create_directories(projectDir / "Objects");

                
                fs::path defaultScenePath = projectDir / "Scenes" / "Default.gScene";

                SceneFolder rootFolder;
                rootFolder.name = "Root";
                rootFolder.parent = nullptr;

                SceneXml::SaveGScene(defaultScenePath.string(), rootFolder);

                
                GV_Project_Info project;
                project.projectName = s_projectNameBuffer;
                project.projectPath =
                    (projectDir / (project.projectName + ".gProject")).string();

                project.dataFolder = "Data";
                project.resourceFolder = "Resources";
                project.sourceFolder = "Source";

                GV_Scene_Info sceneInfo;
                sceneInfo.scenePath = "Scenes/Default.gScene";
                sceneInfo.sceneName = "Default";

                project.scenes.push_back(sceneInfo);
                project.startupScene = sceneInfo.scenePath;

                ProjectXml::SaveProjectToXml(project, project.projectPath);

                state.project = project;
                state.currentScene = sceneInfo;
                state.mode = EditorMode::ProjectOpen;
                state.showStartupDialog = false;

                ImGui::CloseCurrentPopup();
            }


            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Quit", ImVec2(-1, 30)))
        {
            state.quitRequested = true;
        }

        ImGui::End();
    }
}
