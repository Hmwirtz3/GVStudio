#include "Renderer/TerrainPaintTool.h"

#include <iostream>

void TerrainPaintTool::SetMaterial(uint8_t materialID)
{
    std::cout
        << "MATERIAL CHANGE REQUEST: "
        << (int)materialID
        << "\n";

    m_materialID = materialID;

    std::cout
        << "ACTIVE MATERIAL NOW: "
        << (int)m_materialID
        << "\n";
}

void TerrainPaintTool::SetRadius(float radius)
{
    m_radius = radius;

    std::cout
        << "SET RADIUS: "
        << m_radius
        << "\n";
}

void TerrainPaintTool::Paint(
    TerrainPaintMap& map,
    float worldX,
    float worldZ,
    float cellSize)
{
    std::cout
        << "PAINT USING MATERIAL: "
        << (int)m_materialID
        << "\n";

    std::cout
        << "PAINT WORLD POS: "
        << worldX
        << ", "
        << worldZ
        << "\n";

    std::cout
        << "PAINT CELL SIZE: "
        << cellSize
        << "\n";

    map.PaintCircle(
        worldX,
        worldZ,
        m_radius,
        cellSize,
        m_materialID
    );
}

bool TerrainPaintTool::BuildMouseRay(
    float mouseX,
    float mouseY,
    float viewportX,
    float viewportY,
    float viewportWidth,
    float viewportHeight,
    const Mat4& view,
    const Mat4& proj,
    Vec3& outRayOrigin,
    Vec3& outRayDir)
{
    if (viewportWidth <= 0.0f ||
        viewportHeight <= 0.0f)
    {
        std::cout
            << "INVALID VIEWPORT SIZE\n";

        return false;
    }

    float localX =
        mouseX - viewportX;

    float localY =
        mouseY - viewportY;

    localY =
        viewportHeight - localY;

    float ndcX =
        (2.0f * localX) /
        viewportWidth - 1.0f;

    float ndcY =
        (2.0f * localY) /
        viewportHeight - 1.0f;

    Mat4 invViewProj =
        Inverse(proj * view);

    Vec4 nearPoint =
    {
        ndcX,
        ndcY,
        -1.0f,
        1.0f
    };

    Vec4 farPoint =
    {
        ndcX,
        ndcY,
        1.0f,
        1.0f
    };

    Vec4 nearWorld =
        invViewProj * nearPoint;

    Vec4 farWorld =
        invViewProj * farPoint;

    nearWorld =
        nearWorld / nearWorld.w;

    farWorld =
        farWorld / farWorld.w;

    outRayOrigin =
    {
        nearWorld.x,
        nearWorld.y,
        nearWorld.z
    };

    outRayDir =
        Normalize(
            Vec3
            {
                farWorld.x - nearWorld.x,
                farWorld.y - nearWorld.y,
                farWorld.z - nearWorld.z
            }
        );

    std::cout
        << "RAY ORIGIN: "
        << outRayOrigin.x
        << ", "
        << outRayOrigin.y
        << ", "
        << outRayOrigin.z
        << "\n";

    std::cout
        << "RAY DIR: "
        << outRayDir.x
        << ", "
        << outRayDir.y
        << ", "
        << outRayDir.z
        << "\n";

    return true;
}

uint8_t TerrainPaintTool::GetMaterial() const
{
    std::cout
        << "GET MATERIAL: "
        << (int)m_materialID
        << "\n";

    return m_materialID;
}

float TerrainPaintTool::GetRadius() const
{
    std::cout
        << "GET RADIUS: "
        << m_radius
        << "\n";

    return m_radius;
}