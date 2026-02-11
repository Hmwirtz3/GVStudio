#include "GVStudio/GVStudio.h"
#include "MiniXml/MiniXml.h"
#include "MiniXml/SceneXml.h"
#include "MiniXml/ObjectXml.h"
#include "GVFramework/Scene/SceneManager.h"
#include "Database/LogicUnitRegistry.h"

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

SceneManager::SceneManager(SceneFolder& root, LogicUnitRegistry& registry)
    : m_root(root), m_registry(registry)
{
}

std::string SceneManager::GetCurrentSceneDirectory(const GV_State& g_state) const
{
    if (g_state.currentScene.scenePath.empty())
        return {};

    fs::path scenePath(g_state.currentScene.scenePath);
    return scenePath.parent_path().string();
}

bool SceneManager::LoadScene(const std::string& sceneDir)
{
    fs::path sceneFile = fs::path(sceneDir) /
        (fs::path(sceneDir).filename().string() + ".gScene");

    if (!SceneXml::LoadGScene(sceneFile.string(), m_root))
    {
        return false;
    }

    fs::path objectsDir = fs::path(sceneDir) / "Objects";

    LoadAllObjects(m_root, objectsDir.string());

    return true;
}

void SceneManager::LoadAllObjects(SceneFolder& folder, const std::string& objectsDir)
{
    for (auto& objPtr : folder.objects)
    {
        SceneObject& obj = *objPtr;

        fs::path xmlPath = fs::path(objectsDir) /
            (obj.name + ".gObject");

        ObjectXml::LoadObjectFromXml(obj, xmlPath.string(), m_registry);
    }

    for (auto& c : folder.children)
    {
        LoadAllObjects(*c, objectsDir);
    }
}

bool SceneManager::SaveScene(const std::string& sceneDir)
{
    fs::path objectsDir = fs::path(sceneDir) / "Objects";

    if (!fs::exists(objectsDir))
        fs::create_directory(objectsDir);

    SaveAllObjects(m_root, objectsDir.string());

    fs::path sceneFile = fs::path(sceneDir) /
        (fs::path(sceneDir).filename().string() + ".gScene");

    return SceneXml::SaveGScene(sceneFile.string(), m_root);
}

void SceneManager::SaveAllObjects(const SceneFolder& folder,
    const std::string& objectsDir) const
{
    for (const auto& objPtr : folder.objects)
    {
        const SceneObject& obj = *objPtr;

        fs::path xmlPath = fs::path(objectsDir) /
            (obj.name + ".gObject");

        ObjectXml::SaveObjectToXml(obj, xmlPath.string(), m_registry);
    }

    for (const auto& c : folder.children)
    {
        SaveAllObjects(*c, objectsDir);
    }
}
