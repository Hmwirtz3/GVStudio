#pragma once

#include <string>
#include <vector>

#include "Database/AssetDatabase.h"
#include "Database/LogicUnitRegistry.h"
#include "Database/ResourceDatabase.h"
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
    std::string projectRoot;

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

    
    GV_State m_state;

    
    SceneFolder m_rootFolder;
    SceneFolder* m_selectedFolder = nullptr;
    SceneManager m_sceneManager;

    LogicUnitRegistry m_logicRegistry;
    ResourceDatabase m_resourceDatabase;
    AssetDatabase m_assetDataBase;

    SceneObject* m_selectedObject = nullptr;

    bool m_showSceneExplorer = true;
    bool m_showLogicUnitInspector = true;
    bool m_showResourceExplorer = true;

   
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    bool m_isInit = false;
    bool m_imguiInit = false;
    bool m_projectInitialized;


private:
    bool InitSDLAndGL();
    void InitProject();
    bool InitImGui();
    void ShutdownImgui();
    void ShutdownSDLAndGL();

    void DebugPrintProjectInfo() const;
    



private:
    void ShowStartupDialog(GV_State state);
    void DrawLogicUnitRegistry();
    void DrawResourceFolderTree();
    void DrawResourceNode(const ResourceNode* node);
    void DrawDockspace();
    void DrawSceneExplorer();
    void DrawSceneFolderRecursive(SceneFolder& folder);
    void DrawLogicUnitInspector();
    void DrawResourceExplorer();

private: 
    
    SceneObject* CreateObjectFromLogicUnit(const std::string& typeName);

};
