#include "Renderer/GatherScene.h"
#include "GVFramework/Scene/SceneObject.h"
#include "GVFramework/Scene/SceneManager.h"
#include "Renderer/Renderer.h"

#include <filesystem>

namespace fs = std::filesystem;

/*===========================================================
NEW: STATIC STORAGE FOR LIGHTS
===========================================================*/

static std::vector<BakedLight> g_bakedLights;

/*===========================================================
NEW: ACCESSOR
===========================================================*/

const std::vector<BakedLight>& GatherScene::GetBakedLights()
{
    return g_bakedLights;
}

/*===========================================================
ORIGINAL COLLECT (UNCHANGED)
===========================================================*/

void GatherScene::Collect(SceneFolder& root,
    const std::string& resourceRoot,
    std::vector<RenderItem>& outItems)
{
    g_bakedLights.clear(); // 🔥 reset each gather

    CollectFolder(root, resourceRoot, outItems);
}

/*===========================================================
COLLECT FOLDER (ONLY ADDITION IS LIGHT BLOCK)
===========================================================*/

void GatherScene::CollectFolder(SceneFolder& folder,
    const std::string& resourceRoot,
    std::vector<RenderItem>& outItems)
{
    for (auto& objPtr : folder.objects)
    {
        SceneObject* obj = objPtr.get();
        if (!obj || !obj->def || !obj->def->def)
            continue;

        const GV_Logic_Unit& lu = *obj->def->def;

        /*===================================================
        NEW: BAKED LIGHT COLLECTION
        ===================================================*/
        if (lu.chunkType == GV_CHUNK_BAKED_LIGHT)
        {
            BakedLight light{};

            for (size_t i = 0; i < obj->def->values.size(); ++i)
            {
                const auto& paramDef = obj->def->def->params[i];
                const auto& value = obj->def->values[i];
                const std::string& name = paramDef.name;

                if (name == "posX") light.position.x = value.fval;
                else if (name == "posY") light.position.y = value.fval;
                else if (name == "posZ") light.position.z = value.fval;

                else if (name == "dirX") light.direction.x = value.fval;
                else if (name == "dirY") light.direction.y = value.fval;
                else if (name == "dirZ") light.direction.z = value.fval;

                else if (name == "colorR") light.color.x = value.fval;
                else if (name == "colorG") light.color.y = value.fval;
                else if (name == "colorB") light.color.z = value.fval;

                else if (name == "intensity") light.intensity = value.fval;
                else if (name == "range") light.range = value.fval;
                else if (name == "falloff") light.falloff = value.fval;

                else if (name == "lightType") light.type = (int)value.fval;
            }

            g_bakedLights.push_back(light);
        }

        /*===================================================
        ORIGINAL LOGIC (UNCHANGED)
        ===================================================*/

        RenderItem item{};
        item.object = obj;

        if (lu.chunkType == GV_CHUNK_CAMERA)
        {
            item.type = RenderItemType::CameraGizmo;

            ExtractCameraFromLogicUnit(
                obj->def.get(),
                item.camPos,
                item.camRot
            );
        }
        else
        {
            item.type = RenderItemType::Mesh;

            item.model = BuildModelFromLogicUnit(
                obj->def.get(),
                resourceRoot,
                item.modelPath
            );
        }

        outItems.push_back(item);
    }

    for (auto& child : folder.children)
        CollectFolder(*child, resourceRoot, outItems);
}

/*===========================================================
UNCHANGED BELOW
===========================================================*/

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
            fs::path full = fs::path(resourceRoot) / value.sval;
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

void GatherScene::ExtractCameraFromLogicUnit(
    GV_Logic_Unit_Instance* inst,
    Vec3& outPos,
    Vec3& outRot)
{
    if (!inst || !inst->def)
        return;

    for (size_t i = 0; i < inst->values.size(); ++i)
    {
        const auto& paramDef = inst->def->params[i];
        const auto& value = inst->values[i];
        const std::string& name = paramDef.name;

        if (name == "positionX") outPos.x = value.fval;
        else if (name == "positionY") outPos.y = value.fval;
        else if (name == "positionZ") outPos.z = value.fval;

        else if (name == "rotationPitch") outRot.x = value.fval;
        else if (name == "rotationYaw")   outRot.y = value.fval;
        else if (name == "rotationRoll")  outRot.z = value.fval;
    }
}