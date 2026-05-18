#pragma once

#include "Renderer/TerrainGenerator.h"

#include <cstdint>



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

    static uint32_t GetRevision();
};