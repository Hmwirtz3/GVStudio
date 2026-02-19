#include "MiniXml/SceneXml.h"
#include "MiniXml/ObjectXml.h"
#include "GVFramework/Scene/SceneManager.h"
#include "GVStudio/GVStudio.h"

#include <filesystem>

namespace fs = std::filesystem;

SceneManager::SceneManager(LogicUnitRegistry& registry)
    : m_registry(registry)
{
}

SceneFolder& SceneManager::GetRootFolder()
{
    return m_root;
}

const SceneFolder& SceneManager::GetRootFolder() const
{
    return m_root;
}

std::string SceneManager::GetCurrentSceneDirectory(const GV_State& state) const
{
    if (state.currentScene.scenePath.empty())
        return {};

    fs::path scenePath(state.currentScene.scenePath);
    return scenePath.parent_path().string();
}

bool SceneManager::LoadScene(const std::string& sceneDir)
{
    m_root = SceneFolder{}; // reset root

    fs::path sceneFile = fs::path(sceneDir) /
        (fs::path(sceneDir).filename().string() + ".gScene");

    if (!SceneXml::LoadGScene(sceneFile.string(), m_root))
        return false;

    fs::path objectsDir = fs::path(sceneDir) / "Objects";

    LoadAllObjects(m_root, objectsDir.string());

    return true;
}

void SceneManager::LoadAllObjects(SceneFolder& folder,
    const std::string& objectsDir)
{
    for (auto& objPtr : folder.objects)
    {
        SceneObject& obj = *objPtr;

        fs::path xmlPath = fs::path(objectsDir) /
            (obj.name + ".gObject");

        ObjectXml::LoadObjectFromXml(obj, xmlPath.string(), m_registry);
    }

    for (auto& child : folder.children)
    {
        LoadAllObjects(*child, objectsDir);
    }
}

bool SceneManager::SaveScene(const std::string& sceneDir) const
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

    for (const auto& child : folder.children)
    {
        SaveAllObjects(*child, objectsDir);
    }
}

SceneObject* SceneManager::CreateObjectFromLogicUnit(
    const std::string& typeName,
    SceneFolder* targetFolder)
{
    if (!targetFolder)
        return nullptr;

    const GV_Logic_Unit* def = m_registry.Find(typeName);
    if (!def)
        return nullptr;

    auto obj = std::make_unique<SceneObject>();
    obj->name = typeName;

    obj->def = std::make_unique<GV_Logic_Unit_Instance>();
    obj->def->def = const_cast<GV_Logic_Unit*>(def);
    obj->def->values.resize(def->params.size());

    for (size_t i = 0; i < def->params.size(); ++i)
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
