#pragma once

#include "GVFramework/LogicUnit/LogicUnit.h"
#include <vector>
#include <string>
#include "MiniMath/MiniMath.h"

struct SceneFolder;
class SceneObject;
struct BakedLight;
struct TerrainParams;

enum class RenderItemType
{
    Mesh,
    CameraGizmo,
    TexturedQuad
};

struct RenderItem
{
    SceneObject* object = nullptr;

    Mat4 model{};
    std::string modelPath;

    RenderItemType type = RenderItemType::Mesh;

    Vec3 camPos{ 0,0,0 };
    Vec3 camRot{ 0,0,0 };

    float posX = 0.0f;
    float posY = 0.0f;
    int width = 0;
    int height = 0;
    bool visible = true;
    std::string texturePath;
};

class GatherScene
{
public:
    static void Collect(SceneFolder& root,
        const std::string& resourceRoot,
        std::vector<RenderItem>& outItems);

    static const std::vector<BakedLight>& GetBakedLights();
    static const std::vector<TerrainParams>& GetTerrain();

private:
    static void CollectFolder(SceneFolder& folder,
        const std::string& resourceRoot,
        std::vector<RenderItem>& outItems);

    static Mat4 BuildModelFromLogicUnit(
        GV_Logic_Unit_Instance* inst,
        const std::string& resourceRoot,
        std::string& outModelPath);

    static void ExtractCameraFromLogicUnit(
        GV_Logic_Unit_Instance* inst,
        Vec3& outPos,
        Vec3& outRot);
};