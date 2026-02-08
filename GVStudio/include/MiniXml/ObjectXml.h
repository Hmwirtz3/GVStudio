#pragma once
#include <string>
#include <vector>

struct SceneObject;
struct GV_Logic_Unit;

namespace ObjectXml
{
    bool LoadObjectFromXml(SceneObject& obj,
        const std::string& path,
        const std::vector<GV_Logic_Unit>& definitions);

    bool SaveObjectToXml(const SceneObject& obj,
        const std::string& path,
        const std::vector<GV_Logic_Unit>& definitions);
}
