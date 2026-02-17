#include "GVStudio/GVStudio.h"
#include "Viewports/StartupDialog.h"

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

    m_logicRegistry.Clear();
    m_logicRegistry.ParseFolder(fullSource.string());

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
            ShowStartupDialog(m_state);

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

            DrawDockspace();
            DrawSceneExplorer();
            DrawLogicUnitInspector();
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

void GV_STUDIO::DrawSceneFolderRecursive(SceneFolder& folder)
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
                CreateObjectFromLogicUnit(typeName, &folder);

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

            DrawSceneFolderRecursive(*child);
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


void GV_STUDIO::DrawDockspace()
{
}

void GV_STUDIO::DrawSceneExplorer()
{
    if (!m_showSceneExplorer)
        return;

    ImGui::Begin("Scene Explorer", &m_showSceneExplorer);

    ImGui::Text("Scenes");
    ImGui::Separator();

    for (const auto& scene : m_state.project.scenes)
    {
        bool isSelected =
            (scene.scenePath == m_state.currentScene.scenePath);

        if (ImGui::Selectable(scene.sceneName.c_str(), isSelected))
        {
            if (!isSelected)
            {
                m_state.currentScene = scene;

                std::cout << "[SceneExplorer] Switching Scene:\n";
                std::cout << "  Name: " << scene.sceneName << "\n";
                std::cout << "  Path: " << scene.scenePath << "\n";

                fs::path projectRoot =
                    fs::path(m_state.project.projectPath).parent_path();

                fs::path fullScenePath =
                    projectRoot / scene.scenePath;

                m_sceneManager.LoadScene(fullScenePath.string());

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

    DrawSceneFolderRecursive(m_rootFolder);

    ImGui::End();
}

void GV_STUDIO::DrawLogicUnitInspector()
{
    if (!m_showLogicUnitInspector)
        return;

    ImGui::Begin("Logic Unit Editor", &m_showLogicUnitInspector);

    if (!m_selectedObject)
    {
        ImGui::Text("No object selected.");
        ImGui::End();
        return;
    }

    if (!m_selectedObject->def || !m_selectedObject->def->def)
    {
        ImGui::Text("No Logic Unit attached.");
        ImGui::End();
        return;
    }

    GV_Logic_Unit_Instance& inst = *m_selectedObject->def;
    GV_Logic_Unit& def = *inst.def;

    ImGui::Text("Type: %s", def.typeName.c_str());
    ImGui::Separator();

    for (size_t i = 0; i < def.params.size(); i++)
    {
        const LU_Param_Def& paramDef = def.params[i];
        LU_Param_Val& value = inst.values[i];

        switch (paramDef.type)
        {
        case ParamType::Separator:
            ImGui::SeparatorText(paramDef.defaultValue.c_str());
            break;

        case ParamType::Float:
            ImGui::DragFloat(paramDef.name.c_str(), &value.fval, 0.1f);
            break;

        case ParamType::Int:
            ImGui::DragInt(paramDef.name.c_str(), &value.ival, 1);
            break;

        case ParamType::Bool:
            ImGui::Checkbox(paramDef.name.c_str(), &value.bval);
            break;

        case ParamType::String:
        case ParamType::Event:
        case ParamType::Message:
        {
            char buffer[256];
            buffer[0] = 0;
            strncpy_s(buffer, value.sval.c_str(), sizeof(buffer) - 1);

            if (ImGui::InputText(paramDef.name.c_str(), buffer, sizeof(buffer)))
            {
                value.sval = buffer;
            }
            break;
        }

        default:
            break;
        }

        if (!paramDef.hint.empty() && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", paramDef.hint.c_str());
    }

    ImGui::End();
}

void GV_STUDIO::DrawLogicUnitRegistry()
{
    ImGui::Text("Logic Units");
    ImGui::Separator();

    if (ImGui::Button("Refresh"))
    {
        std::cout << "\n[UI] Logic Unit Refresh Requested\n";

        fs::path root = fs::path(m_state.project.projectPath).parent_path();
        fs::path fullSource = root / m_state.project.sourceFolder;

        std::cout << "[UI] Source Folder Resolved: " << fullSource.string() << "\n";

        m_logicRegistry.Clear();
        m_logicRegistry.ParseFolder(fullSource.string());

        std::cout << "[UI] Refresh Complete\n";
    }

    ImGui::Spacing();

    const auto& units = m_logicRegistry.GetAll();

    for (const auto& unit : units)
    {
        ImGui::PushID(unit.typeName.c_str());

        if (ImGui::Selectable(unit.typeName.c_str()))
        {
            std::cout << "[UI] Selected Logic Unit: " << unit.typeName << "\n";
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(
                "LOGIC_UNIT",
                unit.typeName.c_str(),
                unit.typeName.size() + 1);

            ImGui::Text("%s", unit.typeName.c_str());
            ImGui::EndDragDropSource();
        }

        ImGui::SameLine();
        ImGui::TextDisabled("(%d params)", (int)unit.params.size());

        ImGui::PopID();
    }
}

void GV_STUDIO::DrawResourceFolderTree()
{
    const ResourceNode* root = m_resourceDatabase.GetRoot();
    if (!root)
        return;

    ImGui::Text("Resource Folder");
    ImGui::Separator();

    for (const auto& child : root->children)
        DrawResourceNode(child.get());
}

void GV_STUDIO::DrawResourceNode(const ResourceNode* node)
{
    if (!node)
        return;

    ImGui::PushID(node->fullPath.c_str());

    if (node->isFolder)
    {
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanFullWidth;

        bool open = ImGui::TreeNodeEx(node->name.c_str(), flags);

        if (open)
        {
            for (const auto& child : node->children)
                DrawResourceNode(child.get());

            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::Selectable(node->name.c_str());

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(
                "RESOURCE_FILE",
                node->fullPath.c_str(),
                node->fullPath.size() + 1);

            ImGui::Text("%s", node->name.c_str());
            ImGui::EndDragDropSource();
        }
    }

    ImGui::PopID();
}

void GV_STUDIO::DrawResourceExplorer()
{
    if (!m_showResourceExplorer)
        return;

    ImGui::Begin("Resource Explorer", &m_showResourceExplorer);

    if (ImGui::BeginTabBar("ResourceTabs"))
    {
        if (ImGui::BeginTabItem("Logic Units"))
        {
            DrawLogicUnitRegistry();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Resources"))
        {
            DrawResourceFolderTree();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

SceneObject* GV_STUDIO::CreateObjectFromLogicUnit(const std::string& typeName, SceneFolder* targetFolder)
{
    const GV_Logic_Unit* def = m_logicRegistry.Find(typeName);
    if (!def)
        return nullptr;

    auto obj = std::make_unique<SceneObject>();
    obj->name = typeName;

    obj->def = std::make_unique<GV_Logic_Unit_Instance>();
    obj->def->def = const_cast<GV_Logic_Unit*>(def);
    obj->def->values.resize(def->params.size());

    for (size_t i = 0; i < def->params.size(); i++)
    {
        const LU_Param_Def& pDef = def->params[i];
        LU_Param_Val& val = obj->def->values[i];

        switch (pDef.type)
        {
        case ParamType::Float:
            val.fval = pDef.defaultValue.empty() ? 0.0f : std::stof(pDef.defaultValue);
            break;

        case ParamType::Int:
            val.ival = pDef.defaultValue.empty() ? 0 : std::stoi(pDef.defaultValue);
            break;

        case ParamType::Bool:
            val.bval = (pDef.defaultValue == "true" || pDef.defaultValue == "1");
            break;

        case ParamType::String:
        case ParamType::Event:
        case ParamType::Message:
            val.sval = pDef.defaultValue;
            break;

        default:
            break;
        }
    }

    SceneObject* ptr = obj.get();
    targetFolder->objects.push_back(std::move(obj));

    return ptr;
}
