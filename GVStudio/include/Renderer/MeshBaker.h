#pragma once

#include "Renderer/MeshTypes.h"
#include "MiniMath/MiniMath.h"


#include <vector>

struct BakedLight
{
    int type = 0;

    Vec3 position;
    Vec3 direction;

    Vec3 color = { 1.0f, 1.0f, 1.0f };

    float intensity = 1.0f;
    float range = 10.0f;
    float falloff = 2.0f;
};

namespace GV
{
    class MeshBaker
    {
    public:
        static void BakeMesh(
            Mesh& mesh,
            const Mat4& model,
            const std::vector<BakedLight>& bakedLights
        );

        static void ClearShadowCasters();
        static void AddShadowCaster(const Mesh& mesh, const Mat4& model);
    };
}