////////////////////////////////////////////
//  
//  
//  
//  To do:
//  Abstract away rendering API backends
//  Clean up unused member variables
//  SceneGraph 
//  SceneViewer
//  Exporter templates
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

#include "Database/AssetDatabase.h"
#include "Database/LogicUnitRegistry.h"
#include "Database/ResourceDatabase.h"

#include "GVFramework/Scene/SceneManager.h"

#include "Viewports/Panels/LogicUnitInspector.h"
#include "Viewports/Panels/SceneExplorer.h"
#include "Viewports/Panels/ResourceInspectorPanel.h"
#include "Viewports/Panels/LogicUnitRegistryTab.h"
#include "Viewports/Panels/ResourceDatabaseTab.h"


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

    ResourceDatabaseTab m_resourceTab;

    LogicUnitRegistryTab m_logicUnitTab;

    SceneFolder m_rootFolder;
    SceneFolder* m_selectedFolder = &m_rootFolder;
   
    SceneFolder* m_folderPendingDelete = nullptr;
    SceneObject* m_objectPendingDelete = nullptr;

    SceneFolder* m_folderRenaming = nullptr;
    SceneObject* m_objectRenaming = nullptr;

    SceneFolder* m_newFolderParent = nullptr;


    char m_renameBuffer[256] = {};

    bool m_showSceneExplorer = true;
    bool m_showResourceExplorer = true;


    SceneManager m_sceneManager;

    SceneExplorer m_sceneExplorer;

    LogicUnitRegistry m_logicUnitRegistry;

    LogicUnitInspectorPanel m_logicUnitInspectorPanel;

    ResourceInspectorPanel m_resourceInspectorPanel;

    ResourceDatabase m_resourceDatabase;

    AssetDatabase m_assetDataBase;

    SceneObject* m_selectedObject = nullptr;

   
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
    void DrawResourceNode(const ResourceNode* node);
 

private: 
    
    SceneObject* CreateObjectFromLogicUnit(const std::string& typeName, SceneFolder* targetFolder);

};
