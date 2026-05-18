#pragma once

#include <cstdint>
#include <string>

struct LoadedHeightmap
{
    uint8_t* samples = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct TerrainParams
{
    float sampleSpacing = 128.0f;
    float heightScale = 1024.0f;
    float baseHeight = 0.0f;
    std::string heightmapPath;
    std::string texturePath;

    uint32_t tilesX = 0;
    uint32_t tilesY = 0;
};

struct TerrainTile
{
    uint32_t tileX = 0;
    uint32_t tileY = 0;
    uint8_t heights[33 * 33]{};
};

class TerrainGenerator
{
public:
    static LoadedHeightmap LoadHeightmap(const std::string& path);

    static bool GenerateTiles(
        const LoadedHeightmap& heightmap,
        TerrainTile*& outTiles,
        uint32_t& outTileCount,
        TerrainParams& params);

    static void FreeTiles(TerrainTile* tiles, uint32_t tileCount);
};

void FreeHeightmap(LoadedHeightmap& hm);