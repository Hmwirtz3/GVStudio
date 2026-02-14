#pragma once

#include <string>
#include <vector>

#include "Database/AssetDatabase.h"
#include "Database/LogicUnitRegistry.h"
#include "GVFramework/Scene/SceneManager.h"

#include "SDL3/SDL.h"

enum class EditorMode
{
    NoProject,
    ProjectOpen,
    OpenRecent
};

struct GV_Scene_Info
{
    std::string scenePath;
    std::string sceneName;
};


struct GV_Project_Info
{
    std::string projectPath;
    std::string projectName;
    std::string dataFolder;
    std::string resourceFolder;
    std::string sourceFolder;

    int projectVersion = 1;

    std::vector<GV_Scene_Info> scenes;
    std::string startupScene;
};


struct GV_State
{
    EditorMode mode = EditorMode::NoProject;

    GV_Project_Info project;
    GV_Scene_Info currentScene;
    

    bool showStartupDialog = true;
    bool quitRequested = false;
    bool openRecent = false;

    GV_Scene_Info recentScene;
};

class GV_STUDIO
{
public:
    GV_STUDIO();
    ~GV_STUDIO();

    int RUN();

private:

    // ---------- Core State ----------
    GV_State m_state;

    
    SceneFolder m_rootFolder;
    LogicUnitRegistry m_logicRegistry;
    SceneManager m_sceneManager;
    AssetDatabase m_assetDataBase;

    SceneObject* m_selectedObject = nullptr;

    bool m_showSceneExplorer = true;
    bool m_showLogicUnitInspector = true;
    bool m_showResourceExplorer = true;

   
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    bool m_isInit = false;
    bool m_imguiInit = false;


private:
    bool InitSDLAndGL();
    bool InitImGui();
    void ShutdownImgui();
    void ShutdownSDLAndGL();

private:
    void ShowStartupDialog(GV_State state);
    void DrawDockspace();
    void DrawSceneExplorer();
    void DrawLogicUnitExplorer();
    void DrawResourceExplorer();
};
