#pragma once

#include <cstdint>
#include <vector>
#include <string>

struct TerrainMaterialDef
{
    uint8_t id;

    std::string name;
    std::string texturePath;

    int atlasX;
    int atlasY;
    int atlasW;
    int atlasH;

    float uvScaleX = 1.0f;
    float uvScaleY = 1.0f;

    float uvOffsetX = 0.0f;
    float uvOffsetY = 0.0f;

    float rotation = 0.0f;
};

struct TerrainCellPaint
{
    uint8_t materialID = 0;
};

class TerrainPaintMap
{
public:
    static TerrainPaintMap& GetActive();

    void Create(uint32_t width, uint32_t height);
    void Clear(uint8_t materialID);

    bool IsValidCell(int x, int y) const;

    void SetCell(int x, int y, uint8_t materialID);
    uint8_t GetCell(int x, int y) const;

    void PaintCircle(float centerX, float centerZ, float radius, float cellSize, uint8_t materialID);

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

    const std::vector<TerrainCellPaint>& GetCells() const;

    void SetAtlasTexture(const std::string& path);
    const std::string& GetAtlasTexture() const;

    void SetAtlasGridSize(uint32_t tilesX, uint32_t tilesY);
    uint32_t GetAtlasTilesX() const;
    uint32_t GetAtlasTilesY() const;

    void ClearMaterials();
    void AddMaterial(const TerrainMaterialDef& material);
    const TerrainMaterialDef* GetMaterial(uint8_t materialID) const;
    const std::vector<TerrainMaterialDef>& GetMaterials() const;

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<TerrainCellPaint> m_cells;

    std::string m_atlasTexturePath;
    uint32_t m_atlasTilesX = 1;
    uint32_t m_atlasTilesY = 1;
    std::vector<TerrainMaterialDef> m_materials;
};