#include "GVStudio/GVStudio.h"
#include "Viewports/StartupDialog.h"


#include "SDL3/SDL.h"
#include "SDL3/SDL_opengl.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/gl.h>



GV_STUDIO::GV_STUDIO()
    : m_state(),
    m_rootFolder(),
    m_logicRegistry(),
    m_sceneManager(m_rootFolder, m_logicRegistry),
    m_assetDataBase(),
    m_selectedObject(nullptr),
    m_showSceneExplorer(true),
    m_showLogicUnitInspector(true),
    m_showResourceExplorer(true)
{
    if (!InitSDLAndGL())
        return;

    if (!InitImGui())
        return;

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
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

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
            ShowStartupDialog(m_state);
        }
        else
        {
            DrawDockspace();
            DrawSceneExplorer();
            DrawLogicUnitExplorer();
            DrawResourceExplorer();
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


void GV_STUDIO::DrawDockspace() {}
void GV_STUDIO::DrawSceneExplorer() {}
void GV_STUDIO::DrawLogicUnitExplorer() {}
void GV_STUDIO::DrawResourceExplorer() {}
