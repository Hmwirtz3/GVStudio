#pragma once
#include <string>

struct SceneFolder;

namespace SceneXml
{
    bool LoadGScene(const std::string& path, SceneFolder& rootFolder);
    bool SaveGScene(const std::string& path, const SceneFolder& rootFolder);
}
