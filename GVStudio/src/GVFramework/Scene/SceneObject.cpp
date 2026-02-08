#include "GVFramework/Scene/SceneObject.h"
#include "GVFramework\LogicUnit\LogicUnit.h"

#include <memory>

std::unique_ptr<SceneObject> SceneObject::BuildSceneObject(const std::string objectName, std::unique_ptr<GV_Logic_Unit_Instance> logicUnit)
{
	auto obj = std::make_unique<SceneObject>();
	obj->name = objectName;
	obj->def = std::move(logicUnit);
	
	return obj;
}



