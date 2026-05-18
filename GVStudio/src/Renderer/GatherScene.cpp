#include "Renderer/GatherScene.h"
#include "GVFramework/Scene/SceneObject.h"
#include "GVFramework/Scene/SceneManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/TerrainManager.h"
#include "Renderer/TerrainPaintMap.h"

#include <filesystem>

namespace fs = std::filesystem;

static std::vector<BakedLight> g_bakedLights;

const std::vector<BakedLight>& GatherScene::GetBakedLights()
{
    return g_bakedLights;
}

void GatherScene::Collect(
    SceneFolder& root,
    const std::string& resourceRoot,
    std::vector<RenderItem>& outItems)
{
    g_bakedLights.clear();

    CollectFolder(
        root,
        resourceRoot,
        outItems
    );
}

void GatherScene::CollectFolder(
    SceneFolder& folder,
    const std::string& resourceRoot,
    std::vector<RenderItem>& outItems)
{
    for (auto& objPtr : folder.objects)
    {
        SceneObject* obj = objPtr.get();

        if (!obj ||
            !obj->def ||
            !obj->def->def)
        {
            continue;
        }

        const GV_Logic_Unit& lu =
            *obj->def->def;

        if (lu.chunkType == GV_CHUNK_BAKED_LIGHT)
        {
            BakedLight light{};

            for (size_t i = 0;
                i < obj->def->values.size();
                ++i)
            {
                const auto& paramDef =
                    obj->def->def->params[i];

                const auto& value =
                    obj->def->values[i];

                const std::string& name =
                    paramDef.name;

                if (name == "posX")
                    light.position.x = value.fval;
                else if (name == "posY")
                    light.position.y = value.fval;
                else if (name == "posZ")
                    light.position.z = value.fval;
                else if (name == "dirX")
                    light.direction.x = value.fval;
                else if (name == "dirY")
                    light.direction.y = value.fval;
                else if (name == "dirZ")
                    light.direction.z = value.fval;
                else if (name == "colorR")
                    light.color.x = value.fval;
                else if (name == "colorG")
                    light.color.y = value.fval;
                else if (name == "colorB")
                    light.color.z = value.fval;
                else if (name == "intensity")
                    light.intensity = value.fval;
                else if (name == "range")
                    light.range = value.fval;
                else if (name == "falloff")
                    light.falloff = value.fval;
                else if (name == "lightType")
                    light.type = (int)value.fval;
            }

            g_bakedLights.push_back(light);
        }

        if (lu.chunkType == GV_CHUNK_HEIGHTMAP)
        {
            TerrainParams params{};
            std::string heightmapPath;

            for (size_t i = 0;
                i < obj->def->values.size();
                ++i)
            {
                const auto& paramDef =
                    obj->def->def->params[i];

                const auto& value =
                    obj->def->values[i];

                const std::string& name =
                    paramDef.name;

                if (name == "sampleSpacing")
                    params.sampleSpacing = value.fval;
                else if (name == "heightScale")
                    params.heightScale = value.fval;
                else if (name == "baseHeight")
                    params.baseHeight = value.fval;
                else if (name == "texture" &&
                    !value.sval.empty())
                {
                    fs::path full =
                        fs::path(resourceRoot) /
                        value.sval;

                    params.texturePath =
                        full.string();
                }
                else if (name == "heightmapTexture" &&
                    !value.sval.empty())
                {
                    fs::path full =
                        fs::path(resourceRoot) /
                        value.sval;

                    heightmapPath =
                        full.string();
                }
            }

            if (params.sampleSpacing == 0.0f)
                params.sampleSpacing = 128.0f;

            if (params.heightScale == 0.0f)
                params.heightScale = 50.0f;

            params.heightmapPath =
                heightmapPath;

            if (!params.heightmapPath.empty())
            {
                const TerrainParams& current =
                    TerrainManager::GetParams();

                if (current.heightmapPath != params.heightmapPath ||
                    current.sampleSpacing != params.sampleSpacing ||
                    current.heightScale != params.heightScale ||
                    current.baseHeight != params.baseHeight)
                {
                    TerrainManager::SetTerrain(
                        nullptr,
                        0,
                        params
                    );
                }
            }

            continue;
        }

        RenderItem item{};
        item.object = obj;

        if (lu.chunkType == GV_CHUNK_CAMERA)
        {
            item.type =
                RenderItemType::CameraGizmo;

            ExtractCameraFromLogicUnit(
                obj->def.get(),
                item.camPos,
                item.camRot
            );
        }
        else if (lu.chunkType == GV_CHUNK_TEXTURE)
        {
            item.type =
                RenderItemType::TexturedQuad;

            for (size_t i = 0;
                i < obj->def->values.size();
                ++i)
            {
                const auto& paramDef =
                    obj->def->def->params[i];

                const auto& value =
                    obj->def->values[i];

                const std::string& name =
                    paramDef.name;

                if (name == "posX")
                    item.posX = value.fval;
                else if (name == "posY")
                    item.posY = value.fval;
                else if (name == "width")
                    item.width = value.ival;
                else if (name == "height")
                    item.height = value.ival;
                else if (name == "visible")
                    item.visible = value.bval;
                else if (name == "texture" &&
                    !value.sval.empty())
                {
                    fs::path full =
                        fs::path(resourceRoot) /
                        value.sval;

                    item.texturePath =
                        full.string();
                }
            }
        }
        else if (lu.chunkType == GV_CHUNK_AREA_TRIGGER_BOX)
        {
            item.type =
                RenderItemType::trigger;

            item.model =
                Mat4::Identity();

            item.visible = true;

            for (size_t i = 0;
                i < obj->def->values.size();
                ++i)
            {
                const auto& paramDef =
                    obj->def->def->params[i];

                const auto& value =
                    obj->def->values[i];

                const std::string& name =
                    paramDef.name;

                if (name == "posX")
                    item.posX = value.fval;
                else if (name == "posY")
                    item.posY = value.fval;
                else if (name == "posZ")
                    item.posZ = value.fval;
                else if (name == "sizeX")
                    item.width = (int)value.fval;
                else if (name == "sizeY")
                    item.sizeY = (int)value.fval;
                else if (name == "sizeZ")
                    item.height = (int)value.fval;
                else if (name == "visible")
                    item.visible = value.bval;
            }
        }
        else if (lu.chunkType == GV_CHUNK_TERRAIN_ATLAS)
        {
            item.type =
                RenderItemType::TerrainAtlas;

            TerrainPaintMap& paintMap =
                TerrainPaintMap::GetActive();

            paintMap.ClearMaterials();

            int atlasColumns = 4;
            int atlasRows = 1;

            TerrainMaterialDef grass{};
            grass.id = 0;
            grass.name = "Grass";

            grass.atlasX = 0;
            grass.atlasY = 0;
            grass.atlasW = 1;
            grass.atlasH = 1;

            grass.uvScaleX = 1.0f;
            grass.uvScaleY = 1.0f;

            grass.uvOffsetX = 0.0f;
            grass.uvOffsetY = 0.0f;

            TerrainMaterialDef dirt{};
            dirt.id = 1;
            dirt.name = "Dirt";

            dirt.atlasX = 1;
            dirt.atlasY = 0;
            dirt.atlasW = 1;
            dirt.atlasH = 1;

            dirt.uvScaleX = 1.0f;
            dirt.uvScaleY = 1.0f;

            dirt.uvOffsetX = 0.0f;
            dirt.uvOffsetY = 0.0f;

            TerrainMaterialDef rock{};
            rock.id = 2;
            rock.name = "Rock";

            rock.atlasX = 2;
            rock.atlasY = 0;
            rock.atlasW = 1;
            rock.atlasH = 1;

            rock.uvScaleX = 1.0f;
            rock.uvScaleY = 1.0f;

            rock.uvOffsetX = 0.0f;
            rock.uvOffsetY = 0.0f;

            for (size_t i = 0;
                i < obj->def->values.size();
                ++i)
            {
                const auto& paramDef =
                    obj->def->def->params[i];

                const auto& value =
                    obj->def->values[i];

                const std::string& name =
                    paramDef.name;

                if (name == "texture" &&
                    !value.sval.empty())
                {
                    fs::path full =
                        fs::path(resourceRoot) /
                        value.sval;

                    item.texturePath =
                        full.string();

                    paintMap.SetAtlasTexture(
                        full.string()
                    );
                }
                else if (name == "tileSize")
                {
                    item.width = value.ival;
                }
                else if (name == "atlasColumns")
                {
                    atlasColumns = value.ival;
                }
                else if (name == "atlasRows")
                {
                    atlasRows = value.ival;
                }

                //
                // GRASS
                //

                else if (name == "grassAtlasX")
                    grass.atlasX = value.ival;

                else if (name == "grassAtlasY")
                    grass.atlasY = value.ival;

                else if (name == "grassAtlasW")
                    grass.atlasW = value.ival;

                else if (name == "grassAtlasH")
                    grass.atlasH = value.ival;

                else if (name == "grassUVScaleX")
                    grass.uvScaleX = value.fval;

                else if (name == "grassUVScaleY")
                    grass.uvScaleY = value.fval;

                else if (name == "grassUVOffsetX")
                    grass.uvOffsetX = value.fval;

                else if (name == "grassUVOffsetY")
                    grass.uvOffsetY = value.fval;

                //
                // DIRT
                //

                else if (name == "dirtAtlasX")
                    dirt.atlasX = value.ival;

                else if (name == "dirtAtlasY")
                    dirt.atlasY = value.ival;

                else if (name == "dirtAtlasW")
                    dirt.atlasW = value.ival;

                else if (name == "dirtAtlasH")
                    dirt.atlasH = value.ival;

                else if (name == "dirtUVScaleX")
                    dirt.uvScaleX = value.fval;

                else if (name == "dirtUVScaleY")
                    dirt.uvScaleY = value.fval;

                else if (name == "dirtUVOffsetX")
                    dirt.uvOffsetX = value.fval;

                else if (name == "dirtUVOffsetY")
                    dirt.uvOffsetY = value.fval;

                //
                // ROCK
                //

                else if (name == "rockAtlasX")
                    rock.atlasX = value.ival;

                else if (name == "rockAtlasY")
                    rock.atlasY = value.ival;

                else if (name == "rockAtlasW")
                    rock.atlasW = value.ival;

                else if (name == "rockAtlasH")
                    rock.atlasH = value.ival;

                else if (name == "rockUVScaleX")
                    rock.uvScaleX = value.fval;

                else if (name == "rockUVScaleY")
                    rock.uvScaleY = value.fval;

                else if (name == "rockUVOffsetX")
                    rock.uvOffsetX = value.fval;

                else if (name == "rockUVOffsetY")
                    rock.uvOffsetY = value.fval;
            }

            paintMap.SetAtlasGridSize(
                atlasColumns,
                atlasRows
            );

            paintMap.AddMaterial(grass);
            paintMap.AddMaterial(dirt);
            paintMap.AddMaterial(rock);
        }
        else
        {
            item.type =
                RenderItemType::Mesh;

            item.model =
                BuildModelFromLogicUnit(
                    obj->def.get(),
                    resourceRoot,
                    item.modelPath
                );
        }

        outItems.push_back(item);
    }

    for (auto& child : folder.children)
    {
        CollectFolder(
            *child,
            resourceRoot,
            outItems
        );
    }
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

    for (size_t i = 0;
        i < inst->values.size();
        ++i)
    {
        const auto& paramDef =
            inst->def->params[i];

        const auto& value =
            inst->values[i];

        const std::string& name =
            paramDef.name;

        if (name == "posX")
            position.x = value.fval;
        else if (name == "posY")
            position.y = value.fval;
        else if (name == "posZ")
            position.z = value.fval;
        else if (name == "rotX")
            rotation.x = value.fval;
        else if (name == "rotY")
            rotation.y = value.fval;
        else if (name == "rotZ")
            rotation.z = value.fval;
        else if (name == "scaleX")
            scale.x = value.fval;
        else if (name == "scaleY")
            scale.y = value.fval;
        else if (name == "scaleZ")
            scale.z = value.fval;
        else if (name == "modelPath" &&
            !value.sval.empty())
        {
            fs::path full =
                fs::path(resourceRoot) /
                value.sval;

            outModelPath =
                full.string();
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

    for (size_t i = 0;
        i < inst->values.size();
        ++i)
    {
        const auto& paramDef =
            inst->def->params[i];

        const auto& value =
            inst->values[i];

        const std::string& name =
            paramDef.name;

        if (name == "positionX")
            outPos.x = value.fval;
        else if (name == "positionY")
            outPos.y = value.fval;
        else if (name == "positionZ")
            outPos.z = value.fval;
        else if (name == "rotationPitch")
            outRot.x = value.fval;
        else if (name == "rotationYaw")
            outRot.y = value.fval;
        else if (name == "rotationRoll")
            outRot.z = value.fval;
    }
}