#pragma once



#include <string>
#include <vector>
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
    
    SceneManager(SceneFolder& root, LogicUnitRegistry& registry);

    std::string GetCurrentSceneDirectory(const GV_State& state) const;
    SceneObject* CreateObjectFromLogicUnit(const std::string& typeName, SceneFolder* targetFolder, LogicUnitRegistry& logicUnitRegistry);

    bool LoadScene(const std::string& sceneDir);
    bool SaveScene(const std::string& sceneDir);

private:
    SceneFolder& m_root;
    LogicUnitRegistry& m_registry;

    

    void LoadAllObjects(SceneFolder& folder, const std::string& objectsDir);
    void SaveAllObjects(const SceneFolder& folder, const std::string& objectsDir) const;
};
