#include "GVStudio/GVStudio.h"
#include "GVFramework/Scene/SceneManager.h"

#include <filesystem>
#include <string>
#include <vector>

std::string SceneManager::GetCurrentSceneDirectory(GV_State g_state)
{
	if (g_state.currentScene.scenePath.empty())
	{
		return {};
	}
	std::filesystem::path scenePath(g_state.currentScene.scenePath);
	return scenePath.parent_path().string();
}

SceneManager::SceneManager(SceneFolder& root, const std::vector<GV_Logic_Unit>& defs)
	:m_root(root), m_defs(defs) {}

