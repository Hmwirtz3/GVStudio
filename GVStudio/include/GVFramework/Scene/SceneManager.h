#pragma once

#include <string>
#include <memory>

#include "GVFramework/Scene/SceneObject.h"
#include "Database/LogicUnitRegistry.h"

struct GV_State;

struct SceneFolder
{
    std::string name = "Folder";
    SceneFolder* parent = nullptr;

    std::vector<std::unique_ptr<SceneFolder>> children;
    std::vector<std::unique_ptr<SceneObject>> objects;
};

class SceneManager
{
public:
    explicit SceneManager(LogicUnitRegistry& registry);

    SceneFolder& GetRootFolder();
    const SceneFolder& GetRootFolder() const;

    std::string GetCurrentSceneDirectory(const GV_State& state) const;

    bool LoadScene(const std::string& sceneDir);
    bool SaveScene(const std::string& sceneDir) const;

    SceneObject* CreateObjectFromLogicUnit(
        const std::string& typeName,
        SceneFolder* targetFolder);

private:
    void LoadAllObjects(SceneFolder& folder, const std::string& objectsDir);
    void SaveAllObjects(const SceneFolder& folder, const std::string& objectsDir) const;

private:
    SceneFolder m_root;
    LogicUnitRegistry& m_registry;
};
