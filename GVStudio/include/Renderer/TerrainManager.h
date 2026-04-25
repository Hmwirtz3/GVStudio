#pragma once

#include "Renderer/TerrainGenerator.h"

struct TerrainParams
{
    float sampleSpacing = 128.0f;
    float heightScale = 1.0f;
    float baseHeight = 0.0f;
};

class TerrainManager
{
public:
    static void SetTerrain(
        TerrainTile* tiles,
        uint32_t tileCount,
        const TerrainParams& params);

    static void Clear();

    static bool HasTerrain();

    static TerrainTile* GetTiles();
    static const TerrainTile* GetTilesConst();

    static uint32_t GetTileCount();

    static const TerrainParams& GetParams();
};