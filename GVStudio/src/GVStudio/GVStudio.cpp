
#include "GVStudio/GVStudio.h"
#include "Viewports/Dialogs/StartupDialog.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_opengl.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/gl.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <cstring>





namespace fs = std::filesystem;

GV_STUDIO::GV_STUDIO()
    : m_state(),
    m_rootFolder(),
    m_logicUnitRegistry(),
    m_sceneManager(m_logicUnitRegistry),
    m_assetDataBase(),
    m_logicUnitInspectorPanel(),
    m_selectedObject(nullptr),
    m_showSceneExplorer(true),
    //m_showLogicUnitInspector(true),
    m_showResourceExplorer(true)
{
    if (!InitSDLAndGL())
        return;

    if (!InitImGui())
        return;

    m_selectedFolder = &m_rootFolder;

    m_folderPendingDelete = nullptr;
    m_objectPendingDelete = nullptr;
    m_folderRenaming = nullptr;
    m_objectRenaming = nullptr;
    m_renameBuffer[0] = 0;

    m_isInit = true;
}

GV_STUDIO::~GV_STUDIO()
{
    ShutdownImgui();
    ShutdownSDLAndGL();
}

bool GV_STUDIO::InitSDLAndGL()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    m_window = SDL_CreateWindow(
        "Gravitas Studio",
        1600,
        900,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!m_window)
    {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);

    if (!m_glContext)
    {
        printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }

    if (!SDL_GL_MakeCurrent(m_window, m_glContext))
        return false;

    SDL_GL_SetSwapInterval(1);

    return true;
}

bool GV_STUDIO::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext))
        return false;

    if (!ImGui_ImplOpenGL3_Init("#version 330"))
        return false;

    m_imguiInit = true;
    return true;
}

void GV_STUDIO::ShutdownImgui()
{
    if (!m_imguiInit)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    m_imguiInit = false;
}

void GV_STUDIO::ShutdownSDLAndGL()
{
    if (m_glContext)
    {
        SDL_GL_DestroyContext(m_glContext);
        m_glContext = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}

void GV_STUDIO::DebugPrintProjectInfo() const
{
    const GV_Project_Info& project = m_state.project;

    std::cout << "\n=====================================\n";
    std::cout << "[GV_STUDIO] Project Debug Dump\n";
    std::cout << "-------------------------------------\n";

    std::cout << "Project Name:   " << project.projectName << "\n";
    std::cout << "Project Path:   " << project.projectPath << "\n";
    std::cout << "Project Root:   " << project.projectRoot << "\n";

    std::cout << "\nFolders:\n";
    std::cout << "  DataFolder:      " << project.dataFolder << "\n";
    std::cout << "  ResourceFolder:  " << project.resourceFolder << "\n";
    std::cout << "  SourceFolder:    " << project.sourceFolder << "\n";

    std::cout << "\nScenes (" << project.scenes.size() << "):\n";

    for (size_t i = 0; i < project.scenes.size(); ++i)
    {
        const GV_Scene_Info& scene = project.scenes[i];

        std::cout << "  Scene " << i << "\n";
        std::cout << "    Name: " << scene.sceneName << "\n";
        std::cout << "    Path: " << scene.scenePath << "\n";
    }

    std::cout << "\nStartup Scene: " << project.startupScene << "\n";

    std::cout << "=====================================\n\n";
}

void GV_STUDIO::InitProject()
{
    fs::path root = fs::path(m_state.project.projectPath).parent_path();
    fs::path fullSource = root / m_state.project.sourceFolder;

    std::cout << "[GV_STUDIO] InitProject\n";
    std::cout << "  ProjectRoot:  " << root.string() << "\n";
    std::cout << "  SourcePath:   " << fullSource.string() << "\n";

    m_logicUnitRegistry.Clear();
    m_logicUnitRegistry.ParseFolder(fullSource.string());

    fs::path fullResources = root / m_state.project.resourceFolder;
    std::cout << "  ResourcePath: " << fullResources.string() << "\n";

    m_resourceDatabase.BuildFolderTree(fullResources.string());

    fs::path fullScenePath = root / m_state.currentScene.scenePath;
    std::cout << "  ScenePath:    " << fullScenePath.string() << "\n";

    m_sceneManager.LoadScene(fullScenePath.string());

    m_selectedObject = nullptr;
    m_selectedFolder = &m_rootFolder;

    m_folderPendingDelete = nullptr;
    m_objectPendingDelete = nullptr;
    m_folderRenaming = nullptr;
    m_objectRenaming = nullptr;
    m_renameBuffer[0] = 0;

    DebugPrintProjectInfo();
}

int GV_STUDIO::RUN()
{
    if (!m_isInit)
        return -1;

    bool running = true;

    while (running && !m_state.quitRequested)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            ImGui_ImplSDL3_ProcessEvent(&e);

            if (e.type == SDL_EVENT_QUIT)
                running = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (m_state.showStartupDialog)
        {
            ShowStartupDialog(m_state); // Abstract this away from GV_Studio class

            if (!m_state.showStartupDialog &&
                m_state.mode == EditorMode::ProjectOpen &&
                !m_projectInitialized)
            {
                std::cout << "[GV_STUDIO] Project opened. Initializing...\n";
                InitProject();
                m_projectInitialized = true;
            }
        }
        else
        {
            if (m_state.mode == EditorMode::ProjectOpen && !m_projectInitialized)
            {
                std::cout << "[GV_STUDIO] Project open state detected. Initializing...\n";
                InitProject();
                m_projectInitialized = true;
            }

            
            m_sceneExplorer.Draw(m_state, m_sceneManager, m_selectedObject, m_selectedFolder);
            m_logicUnitInspectorPanel.Draw(m_selectedObject);
            m_resourceInspectorPanel.Draw(m_state,  m_logicUnitRegistry, m_resourceDatabase);
        }

        ImGui::Render();

        int w, h;
        SDL_GetWindowSize(m_window, &w, &h);

        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(m_window);
    }

    return 0;
}

void GV_STUDIO::ShowStartupDialog(GV_State)
{
    StartupDialog::Draw(m_state);
}









