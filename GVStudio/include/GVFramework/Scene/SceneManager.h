#pragma once
#include <string>
#include <vector>
#include <memory>

#include "GVFramework/Scene/SceneObject.h"
#include "GVFramework/LogicUnit/LogicUnit.h"
#include "GVFramework/Scene/SceneObject.h"

#include "GVStudio/GVStudio.h"


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
	SceneManager(SceneFolder& root, const std::vector<GV_Logic_Unit>& defs);

	std::string GetCurrentSceneDirectory(GV_State g_State);

	bool LoadScene(const std::string& sceneDir);
	bool SaveScene(const std::string& sceneDir);

private:
	SceneFolder& m_root;
	const std::vector<GV_Logic_Unit>& m_defs;

	void LoadAllObjects(SceneFolder& folder, const std::string& objectsDir);
	void SaveAllObjects(SceneFolder& folder, const std::string& objectsDir) const;
};