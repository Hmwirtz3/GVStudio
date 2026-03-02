#include "Renderer/GatherScene.h"
#include "GVFramework/Scene/SceneObject.h"
#include "GVFramework/Scene/SceneManager.h"
#include <filesystem>
#include <iostream>

void GatherScene::Collect(SceneFolder& root,
    const std::string& resourceRoot,
    std::vector<RenderItem>& outItems)
{
    CollectFolder(root, resourceRoot, outItems);
}

void GatherScene::CollectFolder(SceneFolder& folder,
    const std::string& resourceRoot,
    std::vector<RenderItem>& outItems)
{
    for (auto& objPtr : folder.objects)
    {
        SceneObject* obj = objPtr.get();
        if (!obj || !obj->def)
            continue;

        RenderItem item{};
        item.object = obj;

        item.model = BuildModelFromLogicUnit(
            obj->def.get(),
            resourceRoot,
            item.modelPath
        );

        outItems.push_back(item);
    }

    for (auto& child : folder.children)
        CollectFolder(*child, resourceRoot, outItems);
}

Mat4 GatherScene::BuildModelFromLogicUnit(
    GV_Logic_Unit_Instance* inst,
    const std::string& resourceRoot,
    std::string& outModelPath)
{
    Vec3 position{ 0,0,0 };
    Vec3 rotation{ 0,0,0 };
    Vec3 scale{ 1,1,1 };

    if (!inst || !inst->def)
        return Mat4::Identity();

    for (size_t i = 0; i < inst->values.size(); ++i)
    {
        const auto& paramDef = inst->def->params[i];
        const auto& value = inst->values[i];
        const std::string& name = paramDef.name;

        if (name == "posX") position.x = value.fval;
        else if (name == "posY") position.y = value.fval;
        else if (name == "posZ") position.z = value.fval;

        else if (name == "rotX") rotation.x = value.fval;
        else if (name == "rotY") rotation.y = value.fval;
        else if (name == "rotZ") rotation.z = value.fval;

        else if (name == "scaleX") scale.x = value.fval;
        else if (name == "scaleY") scale.y = value.fval;
        else if (name == "scaleZ") scale.z = value.fval;

        else if (name == "modelPath" && !value.sval.empty())
        {
            std::filesystem::path full =
                std::filesystem::path(resourceRoot) / value.sval;

            outModelPath = full.string();

            
        }
    }

    return
        Translate(position) *
        RotateY(rotation.y) *
        RotateX(rotation.x) *
        RotateZ(rotation.z) *
        Scale(scale);
}