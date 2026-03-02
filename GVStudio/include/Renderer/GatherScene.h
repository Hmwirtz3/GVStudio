#pragma once

#include "GVFramework/LogicUnit/LogicUnit.h"
#include <vector>
#include <string>
#include "MiniMath/MiniMath.h"

struct SceneFolder;
class SceneObject;

struct RenderItem
{
    Mat4 model;
    std::string modelPath;
    SceneObject* object;
};

class GatherScene
{
public:
    static void Collect(SceneFolder& root,
        const std::string& resourceRoot,
        std::vector<RenderItem>& outItems);

private:
    static void CollectFolder(SceneFolder& folder,
        const std::string& resourceRoot,
        std::vector<RenderItem>& outItems);

    static Mat4 BuildModelFromLogicUnit(GV_Logic_Unit_Instance* inst,
        const std::string& resourceRoot,
        std::string& outModelPath);
};