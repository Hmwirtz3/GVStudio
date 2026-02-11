#pragma once

#include "Database/LogicUnitRegistry.h"

#include <string>
#include <vector>

class SceneObject;
struct GV_Logic_Unit;

namespace ObjectXml
{
    bool LoadObjectFromXml(
        SceneObject& obj,
        const std::string& xmlPath,
        LogicUnitRegistry& registry);

    bool SaveObjectToXml(
        const SceneObject& obj,
        const std::string& xmlPath,
        LogicUnitRegistry& registry);

}
