#pragma once

#include "GVFramework/LogicUnit/LogicUnit.h"

#include <memory>

#include <memory>
#include <string>
#include <vector>

class SceneObject
{
public:
	std::string name;
	std::string assetPath;
	std::unique_ptr<GV_Logic_Unit_Instance> def;

	std::unique_ptr<SceneObject> BuildSceneObject(const std::string objectName, std::unique_ptr<GV_Logic_Unit_Instance> logicUnit);

};