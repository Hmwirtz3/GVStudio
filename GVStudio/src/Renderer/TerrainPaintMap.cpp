#include "Renderer/TerrainPaintMap.h"

#include <cmath>

void TerrainPaintMap::Create(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    m_cells.clear();
    m_cells.resize(width * height);
}

void TerrainPaintMap::Clear(uint8_t materialID)
{
    for (uint32_t i = 0; i < m_cells.size(); ++i)
        m_cells[i].materialID = materialID;
}

bool TerrainPaintMap::IsValidCell(int x, int y) const
{
    if (x < 0)
        return false;

    if (y < 0)
        return false;

    if (x >= (int)m_width)
        return false;

    if (y >= (int)m_height)
        return false;

    return true;
}

void TerrainPaintMap::SetCell(int x, int y, uint8_t materialID)
{
    if (!IsValidCell(x, y))
        return;

    m_cells[y * m_width + x].materialID = materialID;
}

uint8_t TerrainPaintMap::GetCell(int x, int y) const
{
    if (!IsValidCell(x, y))
        return 0;

    return m_cells[y * m_width + x].materialID;
}

void TerrainPaintMap::PaintCircle(float centerX, float centerZ, float radius, float cellSize, uint8_t materialID)
{
    if (cellSize <= 0.0f)
        return;

    const int minX = (int)std::floor((centerX - radius) / cellSize);
    const int maxX = (int)std::floor((centerX + radius) / cellSize);
    const int minY = (int)std::floor((centerZ - radius) / cellSize);
    const int maxY = (int)std::floor((centerZ + radius) / cellSize);

    const float radiusSq = radius * radius;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            const float cellCenterX = ((float)x + 0.5f) * cellSize;
            const float cellCenterZ = ((float)y + 0.5f) * cellSize;

            const float dx = cellCenterX - centerX;
            const float dz = cellCenterZ - centerZ;

            if ((dx * dx + dz * dz) <= radiusSq)
                SetCell(x, y, materialID);
        }
    }
}

uint32_t TerrainPaintMap::GetWidth() const
{
    return m_width;
}

uint32_t TerrainPaintMap::GetHeight() const
{
    return m_height;
}

const std::vector<TerrainCellPaint>& TerrainPaintMap::GetCells() const
{
    return m_cells;
}

TerrainPaintMap& TerrainPaintMap::GetActive()
{
    static TerrainPaintMap map;
    return map;
}

void TerrainPaintMap::SetAtlasTexture(const std::string& path)
{
    m_atlasTexturePath = path;
}

const std::string& TerrainPaintMap::GetAtlasTexture() const
{
    return m_atlasTexturePath;
}

void TerrainPaintMap::SetAtlasGridSize(uint32_t tilesX, uint32_t tilesY)
{
    m_atlasTilesX = tilesX;
    m_atlasTilesY = tilesY;
}

uint32_t TerrainPaintMap::GetAtlasTilesX() const
{
    return m_atlasTilesX;
}

uint32_t TerrainPaintMap::GetAtlasTilesY() const
{
    return m_atlasTilesY;
}

void TerrainPaintMap::ClearMaterials()
{
    m_materials.clear();
}

void TerrainPaintMap::AddMaterial(const TerrainMaterialDef& material)
{
    m_materials.push_back(material);
}

const TerrainMaterialDef* TerrainPaintMap::GetMaterial(uint8_t materialID) const
{
    if (materialID >= m_materials.size())
        return nullptr;

    return &m_materials[materialID];
}

const std::vector<TerrainMaterialDef>& TerrainPaintMap::GetMaterials() const
{
    return m_materials;
}