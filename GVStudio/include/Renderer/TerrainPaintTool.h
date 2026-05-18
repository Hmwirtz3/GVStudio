#pragma once

#include "Renderer/TerrainPaintMap.h"
#include "MiniMath/MiniMath.h"

class TerrainPaintTool
{
public:
    void SetMaterial(uint8_t materialID);
    void SetRadius(float radius);

    void Paint(TerrainPaintMap& map, float worldX, float worldZ, float cellSize);

    uint8_t GetMaterial() const;
    float GetRadius() const;

    bool BuildMouseRay(
        float mouseX,
        float mouseY,
        float viewportX,
        float viewportY,
        float viewportWidth,
        float viewportHeight,
        const Mat4& view,
        const Mat4& proj,
        Vec3& outRayOrigin,
        Vec3& outRayDir
    );

private:
    uint8_t m_materialID = 0;
    float m_radius = 128.0f;
};