#include "GVStudio/GVStudio.h"
#include "MiniXml/MiniXml.h"
#include "MiniXml/SceneXml.h"
#include "MiniXml/ObjectXml.h"
#include "GVFramework/Scene/SceneManager.h"

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::string SceneManager::GetCurrentSceneDirectory(GV_State g_state)
{
	if (g_state.currentScene.scenePath.empty())
	{
		return {};
	}
	std::filesystem::path scenePath(g_state.currentScene.scenePath);
	return scenePath.parent_path().string();
}

bool SceneManager::LoadScene(const std::string& sceneDir)
{
	fs::path sceneFile = fs::path(sceneDir) /
		(fs::path(sceneDir).filename().string() + ".gScene");

	if (!SceneXml::LoadGScene(sceneFile.string(), m_root))
	return false;

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

		ObjectXml::LoadObjectFromXml(obj, xmlPath.string(), m_defs);
	}

	for (auto& c : folder.children)
		LoadAllObjects(*c, objectsDir);
}


SceneManager::SceneManager(SceneFolder& root, const std::vector<GV_Logic_Unit>& defs)
	:m_root(root), m_defs(defs) {}

